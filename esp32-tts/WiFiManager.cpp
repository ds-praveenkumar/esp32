#include "WiFiManager.h"
#include <WiFi.h>

void wifi_init()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.printf("WiFi: connecting to %s\n", WIFI_SSID);
}

bool wifi_wait_connected(unsigned long timeout_ms)
{
    unsigned long start = millis();
    while (!WiFi.isConnected() && millis() - start < timeout_ms) {
        delay(100);
    }
    if (WiFi.isConnected()) {
        Serial.printf("WiFi connected, IP: %s\n", WiFi.localIP().toString().c_str());
        return true;
    }
    Serial.println("WiFi timeout");
    return false;
}

bool wifi_is_connected()
{
    return WiFi.isConnected();
}
