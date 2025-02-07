#ifndef CAPTIVE_PORTAL_MANAGER_H
#define CAPTIVE_PORTAL_MANAGER_H

#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Preferences.h>

// Forward declaration for callback type
typedef void (*FollowerCountCallback)(int newCount);

class CaptivePortalManager
{
public:
    // Constructor with optional custom AP credentials
    CaptivePortalManager(const char* apSSID = "ConfigPortal", 
                         const char* apPASS = "");

    // Initialize the captive portal or connect to stored credentials
    void begin();

    // Main loop to handle DNS, HTTP, and follower updates
    void handle();

    // Get the current cached follower count
    int getFollowerCount() const { return _currentFollowerCount; }

    // Set how often to fetch the Instagram follower count (ms)
    void setFetchInterval(unsigned long interval) { _fetchInterval = interval; }

    // Register a callback function that fires when new follower data arrives
    void onFollowerCountUpdate(FollowerCountCallback cb);

    // Possible error codes/states
    enum ErrorState {
        ERROR_NONE,
        ERROR_WIFI_CONNECTION,
        ERROR_INSTAGRAM_FETCH,
        ERROR_JSON_PARSE
    };

    // Returns the latest error state
    ErrorState getLastError() const { return _lastError; }

    // Clears any stored error (optional user action)
    void clearLastError() { _lastError = ERROR_NONE; }

private:
    // Internal methods
    void startCaptivePortal();
    void stopCaptivePortal();
    bool connectToWiFi(const String& ssid, const String& pass);
    bool fetchInstagramFollowers(const String& userName);

    // Internal web server handlers
    void handleRoot();
    void handleSubmit();
    void handleNotFound();
    void handleGetStatus();

    // DNS and HTTP servers
    DNSServer dnsServer;
    WebServer server;

    // AP credentials
    const char* apSSID;
    const char* apPASS;

    // User-provided credentials
    String wifiSSID;
    String wifiPASS;
    String instagramUser;

    // Indicates that user has submitted new credentials
    bool configReceived;

    // Timestamps for repeated follower fetch
    unsigned long lastFetchTime;
    unsigned long _fetchInterval;

    // Follower count and error state
    int  _currentFollowerCount;
    ErrorState _lastError;

    // Callback function pointer
    FollowerCountCallback followerCb;

    // Web page contents (includes JS for live updates)
    static const char MAIN_page[];

    // Preferences for NVS-based storage
    Preferences prefs;
};

#endif
