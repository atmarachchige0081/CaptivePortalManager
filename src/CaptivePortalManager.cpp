#include "CaptivePortalManager.h"

// HTML/JS Page:
//   1) Basic form for entering Wi-Fi & Instagram credentials
//   2) JavaScript that periodically fetches follower count & status from /status
//      and updates the page dynamically.
const char CaptivePortalManager::MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 Captive Portal</title>
  <style>
    body {
      margin: 0; padding: 0; font-family: 'Arial', sans-serif;
      background: linear-gradient(to right, #4facfe, #00f2fe);
      display: flex; align-items: center; justify-content: center;
      height: 100vh; animation: fadeBg 1.5s ease-in-out;
    }
    .container {
      background: #ffffff; width: 90%; max-width: 400px;
      border-radius: 10px; padding: 20px;
      box-shadow: 0 4px 10px rgba(0, 0, 0, 0.2);
      text-align: center; animation: fadeIn 0.8s ease-in-out;
    }
    h2 { color: #007bff; font-size: 24px; margin-bottom: 10px; }
    p { color: #333; margin-bottom: 20px; }
    input {
      width: 90%; padding: 12px; margin: 8px 0;
      border: 1px solid #ccc; border-radius: 5px; font-size: 16px;
    }
    .button {
      width: 90%; padding: 12px; border: none;
      border-radius: 5px; background-color: #007bff;
      color: #fff; font-size: 18px; cursor: pointer;
      transition: background-color 0.3s;
    }
    .button:hover { background-color: #0056b3; }
    #status {
      margin-top: 20px; font-size: 14px; color: #555;
    }
    @keyframes fadeIn {
      from { opacity: 0; transform: scale(0.95); }
      to { opacity: 1; transform: scale(1); }
    }
    @keyframes fadeBg {
      from { background-position: 0% 50%; }
      to { background-position: 100% 50%; }
    }
  </style>
</head>
<body>
  <div class="container">
    <h2>ESP32 Captive Portal</h2>
    <p>Enter Wi-Fi & Instagram details:</p>
    <form action="/submit" method="POST">
      <input type="text" name="wifi_ssid" placeholder="Wi-Fi SSID" required><br>
      <input type="password" name="wifi_password" placeholder="Wi-Fi Password" required><br>
      <input type="text" name="instagram_username" placeholder="Instagram Username" required><br>
      <button class="button" type="submit">Connect</button>
    </form>

    <div id="status"></div>
  </div>

  <script>
    // Periodically poll the /status endpoint for updates
    setInterval(async () => {
      try {
        const response = await fetch('/status');
        if (!response.ok) return;
        const data = await response.json();
        let statusDiv = document.getElementById('status');

        let html = `<strong>Wi-Fi Status:</strong> ${data.wifi_status}<br>`;
        html += `<strong>Follower Count:</strong> ${data.follower_count}<br>`;
        if (data.last_error && data.last_error !== "None") {
          html += `<strong style="color:red;">Last Error:</strong> ${data.last_error}<br>`;
        }
        statusDiv.innerHTML = html;
      } catch(e) {
        console.log("Error fetching status:", e);
      }
    }, 5000);
  </script>
</body>
</html>
)rawliteral";

CaptivePortalManager::CaptivePortalManager(const char* apSSID, const char* apPASS)
    : server(80),
      apSSID(apSSID),
      apPASS(apPASS),
      configReceived(false),
      lastFetchTime(0),
      _fetchInterval(60000), // Default 60s
      _currentFollowerCount(-1),
      _lastError(ERROR_NONE),
      followerCb(nullptr) { }

void CaptivePortalManager::begin()
{
    Serial.begin(115200);

    // Attempt to load stored Wi-Fi credentials from Preferences (NVS)
    prefs.begin("captive", false);
    String storedSSID = prefs.getString("ssid", "");
    String storedPASS = prefs.getString("pass", "");
    prefs.end();

    // If stored credentials exist, try to connect directly
    if (!storedSSID.isEmpty() && !storedPASS.isEmpty()) {
        Serial.println("Found stored Wi-Fi credentials. Attempting to connect...");
        WiFi.mode(WIFI_STA);
        if (connectToWiFi(storedSSID, storedPASS)) {
            // Save them as current if successful
            wifiSSID = storedSSID;
            wifiPASS = storedPASS;
            return; // We skip captive portal if connected
        }
        // If connection fails, we fall back to Captive Portal
        _lastError = ERROR_WIFI_CONNECTION;
    }

    // If no valid credentials, start captive portal
    startCaptivePortal();
}

void CaptivePortalManager::handle()
{
    // Process DNS and web server requests
    dnsServer.processNextRequest();
    server.handleClient();

    // If user has submitted new credentials
    if (configReceived) {
        stopCaptivePortal();
        if (connectToWiFi(wifiSSID, wifiPASS)) {
            // Store these credentials securely in NVS
            prefs.begin("captive", false);
            prefs.putString("ssid", wifiSSID);
            prefs.putString("pass", wifiPASS);
            prefs.end();

            // Initial fetch
            if (!fetchInstagramFollowers(instagramUser)) {
                Serial.println("Initial Instagram fetch failed.");
            }
            lastFetchTime = millis();
        } else {
            Serial.println("Wi-Fi connection failed after config submission.");
            _lastError = ERROR_WIFI_CONNECTION;
        }
        configReceived = false;
    }

    // If already connected, fetch followers periodically
    if (WiFi.status() == WL_CONNECTED) {
        unsigned long now = millis();
        if (now - lastFetchTime >= _fetchInterval) {
            if (!fetchInstagramFollowers(instagramUser)) {
                Serial.println("Instagram fetch failed during loop.");
            }
            lastFetchTime = now;
        }
    }
}

void CaptivePortalManager::onFollowerCountUpdate(FollowerCountCallback cb)
{
    followerCb = cb;
}

void CaptivePortalManager::startCaptivePortal()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSSID, apPASS);
    delay(1000);

    IPAddress AP_IP = WiFi.softAPIP();
    Serial.println("Captive Portal started.");
    Serial.print("AP IP address: ");
    Serial.println(AP_IP);

    // Start DNS to redirect all requests
    dnsServer.start(53, "*", AP_IP);

    // Set up web server routes
    server.on("/", HTTP_GET, [this]() { handleRoot(); });
    server.on("/submit", HTTP_POST, [this]() { handleSubmit(); });
    server.on("/status", HTTP_GET, [this]() { handleGetStatus(); });
    server.onNotFound([this]() { handleNotFound(); });

    server.begin();
}

void CaptivePortalManager::stopCaptivePortal()
{
    dnsServer.stop();
    server.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    Serial.println("Captive portal stopped.");
}

bool CaptivePortalManager::connectToWiFi(const String& ssid, const String& pass)
{
    Serial.print("Connecting to Wi-Fi: ");
    Serial.println(ssid);
    WiFi.begin(ssid.c_str(), pass.c_str());

    unsigned long startTime = millis();
    const unsigned long TIMEOUT = 30000; // 30s
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < TIMEOUT) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Wi-Fi connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        _lastError = ERROR_NONE;
        return true;
    } else {
        Serial.println("Wi-Fi connection failed!");
        _lastError = ERROR_WIFI_CONNECTION;
        return false;
    }
}

bool CaptivePortalManager::fetchInstagramFollowers(const String& userName)
{
    if (userName.isEmpty()) {
        Serial.println("No Instagram username provided. Skipping fetch.");
        return false;
    }

    const char* HOST = "i.instagram.com";
    const int   PORT = 443;

    WiFiClientSecure client;
    // We use setInsecure() to skip certificate verification. 
    // For real security, consider certificate pinning or a valid root CA.
    client.setInsecure(); 

    Serial.printf("Connecting to Instagram at %s:%d\n", HOST, PORT);
    if (!client.connect(HOST, PORT)) {
        Serial.println("Connection failed.");
        _lastError = ERROR_INSTAGRAM_FETCH;
        return false;
    }

    String urlPath = "/api/v1/users/web_profile_info/?username=" + userName;

    // Prepare the request
    client.printf("GET %s HTTP/1.1\r\n", urlPath.c_str());
    client.printf("Host: %s\r\n", HOST);
    client.println("User-Agent: Instagram 76.0.0.15.395");
    client.println("Accept: application/json");
    client.println("X-IG-App-ID: 936619743392459");
    client.println("Connection: close");
    client.println();

    // Read the full response
    String rawResponse;
    while (client.connected() || client.available()) {
        if (client.available()) {
            String line = client.readStringUntil('\n');
            rawResponse += line + "\n";
        }
    }
    client.stop();

    // The JSON body typically begins after "\r\n\r\n"
    int jsonIndex = rawResponse.indexOf("\r\n\r\n");
    if (jsonIndex == -1) {
        jsonIndex = rawResponse.indexOf("\n\n");
    }
    if (jsonIndex == -1) {
        jsonIndex = 0; 
    } else {
        jsonIndex += 4;
    }
    String jsonBody = rawResponse.substring(jsonIndex);

    // Parse JSON
    StaticJsonDocument<4096> doc;
    DeserializationError error = deserializeJson(doc, jsonBody);
    if (error) {
        Serial.print("JSON parse error: ");
        Serial.println(error.f_str());
        _lastError = ERROR_JSON_PARSE;
        return false;
    }

    int followerCount = doc["data"]["user"]["edge_followed_by"]["count"] | -1;
    if (followerCount < 0) {
        followerCount = 0;
    }
    // Clip to a max for demonstration
    if (followerCount > 99999) {
        followerCount = 99999; 
    }

    _currentFollowerCount = followerCount;
    Serial.println("-----------------------");
    Serial.printf("Instagram @%s => %d followers\n", userName.c_str(), followerCount);
    Serial.println("-----------------------");

    // If we have a callback set, fire it
    if (followerCb) {
        followerCb(_currentFollowerCount);
    }

    _lastError = ERROR_NONE;
    return true;
}

void CaptivePortalManager::handleRoot()
{
    server.send_P(200, "text/html", MAIN_page);
}

void CaptivePortalManager::handleSubmit()
{
    // Extract form fields
    if (server.hasArg("wifi_ssid") && 
        server.hasArg("wifi_password") &&
        server.hasArg("instagram_username")) 
    {
        wifiSSID = server.arg("wifi_ssid");
        wifiPASS = server.arg("wifi_password");
        instagramUser = server.arg("instagram_username");
        configReceived = true;

        // Acknowledge receipt to the client
        String response = "<html><body><h2>Credentials Received!</h2>"
                          "<p>SSID: " + wifiSSID + "</p>"
                          "<p>Password: " + wifiPASS + "</p>"
                          "<p>Instagram: " + instagramUser + "</p>"
                          "<p>Attempting to connect...</p>"
                          "</body></html>";
        server.send(200, "text/html", response);
    } else {
        server.send(400, "text/plain", "Missing form fields.");
    }
}

void CaptivePortalManager::handleNotFound()
{
    // In a captive portal, redirect unknown pages back to root
    server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString(), true);
    server.send(302, "text/plain", "");
}

void CaptivePortalManager::handleGetStatus()
{
    // Return JSON with the Wi-Fi status, follower count, and last error
    String wifiStatus = (WiFi.status() == WL_CONNECTED) ? "Connected" : "Not Connected";

    // Convert error enum to human-readable strings
    String errorStr;
    switch(_lastError) {
        case ERROR_NONE:              errorStr = "None"; break;
        case ERROR_WIFI_CONNECTION:   errorStr = "Wi-Fi Connection Failed"; break;
        case ERROR_INSTAGRAM_FETCH:   errorStr = "Instagram Fetch Failed"; break;
        case ERROR_JSON_PARSE:        errorStr = "JSON Parse Error"; break;
        default:                      errorStr = "Unknown";
    }

    StaticJsonDocument<200> doc;
    doc["wifi_status"] = wifiStatus;
    doc["follower_count"] = _currentFollowerCount;
    doc["last_error"] = errorStr;

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}
