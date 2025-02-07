
[![Check Arduino](https://github.com/atmarachchige0081/CaptivePortalManager/actions/workflows/check-arduino.yml/badge.svg)](https://github.com/atmarachchige0081/CaptivePortalManager/actions/workflows/check-arduino.yml)
[![Compile Examples](https://github.com/atmarachchige0081/CaptivePortalManager/actions/workflows/compile-examples.yml/badge.svg)](https://github.com/atmarachchige0081/CaptivePortalManager/actions/workflows/compile-examples.yml)
[![Spell Check](https://github.com/atmarachchige0081/CaptivePortalManager/actions/workflows/spell-check.yml/badge.svg)](https://github.com/atmarachchige0081/CaptivePortalManager/actions/workflows/spell-check.yml)


# CaptivePortalManager

CaptivePortalManager is an Arduino/ESP32 library that:
- **Creates a captive portal** to collect Wi-Fi credentials from users.
- **Fetches Instagram follower counts** for a specified username.
- Optionally **stores credentials in NVS**, providing persistent Wi-Fi logins across reboots.
- Includes **callbacks**, **error handling**, and a **JavaScript-powered dynamic web interface** for real-time updates.

## Features

1. **Captive Portal**  
   - Redirects users to a configuration page for Wi-Fi and Instagram details.
2. **Instagram Follower Count**  
   - Periodically fetches the follower count from Instagram's API using HTTPS.
3. **Callback Mechanism**  
   - Allows custom functions to trigger whenever a new follower count is fetched.
4. **Error Handling**  
   - Differentiates between Wi-Fi connection failures, Instagram fetch failures, JSON parse errors, etc.
5. **NVS Storage**  
   - Remembers credentials across device resets (ESP32 only).
6. **Dynamic Web Page**  
   - JavaScript polls for status/follower updates, displayed live on the captive portal page.

## Installation

1. **Using the Arduino Library Manager**  
   - *(Recommended if submitted to and accepted by the Arduino Library Manager)*  
   - In the Arduino IDE, go to **Tools > Manage Libraries...**  
   - Search for **"CaptivePortalManager"**, then click **Install**.
2. **Manual Installation**  
   - Download or clone this repository into your Arduino libraries folder (e.g., `~/Documents/Arduino/libraries` on most systems).  
   - Ensure the folder name is `CaptivePortalManager`.  
   - Restart the Arduino IDE.

## Dependencies

- ArduinoJson for JSON parsing.
- Preferences (built into ESP32 Arduino Core) if you want NVS storage.
- WiFiClientSecure (built into ESP32 Arduino Core) for HTTPS connections.


