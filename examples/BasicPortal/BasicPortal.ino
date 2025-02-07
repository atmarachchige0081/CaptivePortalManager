/*
  BasicPortal.ino
  Demonstrates the simplest usage of the CaptivePortalManager library.
  - Starts a captive portal using default AP credentials ("ConfigPortal").
  - Waits for user to input Wi-Fi + Instagram credentials.
  - Connects to Wi-Fi and automatically fetches Instagram follower count.
*/

#include <Arduino.h>
#include <WiFi.h>
#include "CaptivePortalManager.h"

// Create a CaptivePortalManager object with default AP credentials.
CaptivePortalManager captivePortal;

void setup() {
  Serial.begin(115200);
  delay(2000);

  // Initialize the manager. 
  // If valid Wi-Fi credentials are already stored, it will try to connect directly.
  // Otherwise, it starts the captive portal.
  captivePortal.begin();
}

void loop() {
  // Must be called regularly to handle DNS, HTTP requests, 
  // and to perform periodic follower count checks.
  captivePortal.handle();

  // If the device is online, you can retrieve the last follower count.
  if (WiFi.status() == WL_CONNECTED) {
    int count = captivePortal.getFollowerCount();
    Serial.printf("Current follower count: %d\n", count);
  }

  delay(1000);
}
