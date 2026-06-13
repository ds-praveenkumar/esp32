#pragma once
#include <Arduino.h>
#include "secrets.h"

#ifndef WIFI_SSID
#define WIFI_SSID ""
#define WIFI_PASS ""
#endif

void wifi_init();
bool wifi_wait_connected(unsigned long timeout_ms);
bool wifi_is_connected();
