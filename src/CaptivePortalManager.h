#ifndef CAPTIVE_PORTAL_MANAGER_H
#define CAPTIVE_PORTAL_MANAGER_H

#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Preferences.h>

typedef void (*FollowerCountCallback)(int newCount);

class CaptivePortalManager
{
public:
    CaptivePortalManager(const char* apSSID = "ConfigPortal", 
                         const char* apPASS = "");

 
    void begin();

    void handle();

    int getFollowerCount() const { return _currentFollowerCount; }


    void setFetchInterval(unsigned long interval) { _fetchInterval = interval; }

    void onFollowerCountUpdate(FollowerCountCallback cb);

    enum ErrorState {
        ERROR_NONE,
        ERROR_WIFI_CONNECTION,
        ERROR_INSTAGRAM_FETCH,
        ERROR_JSON_PARSE
    };

    ErrorState getLastError() const { return _lastError; }

    void clearLastError() { _lastError = ERROR_NONE; }

private:

    void startCaptivePortal();
    void stopCaptivePortal();
    bool connectToWiFi(const String& ssid, const String& pass);
    bool fetchInstagramFollowers(const String& userName);

    void handleRoot();
    void handleSubmit();
    void handleNotFound();
    void handleGetStatus();

    DNSServer dnsServer;
    WebServer server;

    const char* apSSID;
    const char* apPASS;

    String wifiSSID;
    String wifiPASS;
    String instagramUser;

    bool configReceived;

    unsigned long lastFetchTime;
    unsigned long _fetchInterval;

    int  _currentFollowerCount;
    ErrorState _lastError;

    FollowerCountCallback followerCb;

    static const char MAIN_page[];

    Preferences prefs;
};

#endif
