/*
  ExtendedFeatures.ino
  Showcases advanced capabilities of CaptivePortalManager:
    1) Callback when a new follower count is fetched.
    2) Setting a custom fetch interval.
    3) Checking for error states (Wi-Fi or Instagram fetch issues).
    4) Use of stored credentials in NVS (if library has that feature enabled).
*/

#include <Arduino.h>
#include <WiFi.h>
#include "CaptivePortalManager.h"

// Example callback function for new follower count
void followerCountCallback(int newCount) {
  Serial.print("[Callback] New Follower Count: ");
  Serial.println(newCount);
}

// Create a CaptivePortalManager object with custom AP credentials.
CaptivePortalManager captivePortal("ExtendedPortal", "portal123");

void setup() {
  Serial.begin(115200);
  delay(2000);

  // Register our callback
  captivePortal.onFollowerCountUpdate(followerCountCallback);

  // (Optional) Set a custom fetch interval, e.g. 2 minutes
  captivePortal.setFetchInterval(120000);

  // Start the manager:
  //   - Tries stored credentials first.
  //   - If none, spins up a captive portal named "ExtendedPortal".
  captivePortal.begin();
}

void loop() {
  // Must be called often to handle DNS, HTTP requests, etc.
  captivePortal.handle();

  // Monitor errors (like Wi-Fi or fetch failures)
  auto lastErr = captivePortal.getLastError();
  if (lastErr != CaptivePortalManager::ERROR_NONE) {
    // Handle or log the error
    Serial.printf("Encountered error code: %d\n", lastErr);
    // Optionally clear the error state
    // captivePortal.clearLastError();
  }

  // If connected, print out the current follower count
  if (WiFi.status() == WL_CONNECTED) {
    int currentCount = captivePortal.getFollowerCount();
    Serial.printf("Current follower count: %d\n", currentCount);
  }

  delay(1000);
}
