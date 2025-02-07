/*
  CustomAP.ino
  Demonstrates how to customize the ESP32 Access Point credentials
  when starting the captive portal. This is useful for branding
  your setup or adding a password for the portal.
*/

#include <Arduino.h>
#include <WiFi.h>
#include "CaptivePortalManager.h"

// Create a CaptivePortalManager object with custom AP credentials 
// (SSID: "MyCustomPortal", Password: "12345678").
CaptivePortalManager captivePortal("MyCustomPortal", "12345678");

void setup() {
  Serial.begin(115200);
  delay(2000);

  // Attempt to connect with stored credentials; if none, 
  // start an AP named "MyCustomPortal" requiring "12345678" to join.
  captivePortal.begin();
}

void loop() {
  // Handle captive portal tasks (DNS redirection, web server endpoints)
  // and periodic Instagram fetch.
  captivePortal.handle();

  // If connected, print out the current follower count
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("Follower count: %d\n", captivePortal.getFollowerCount());
  }

  delay(2000);
}
