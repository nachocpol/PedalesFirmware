#ifndef WIFIHELPER_H
#define WIFIHELPER_H

#include "stdint.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_wifi.h"

static uint8_t k_MaxReconnectRetries = 6;

struct WifiHelper
{
    uint8_t m_Initialized;
    uint8_t m_NumRetries;
    EventGroupHandle_t m_WifiEventGroup;
};

typedef struct
{
    char m_SSID[33];
    int8_t m_RSSI;
    wifi_auth_mode_t m_AuthMode;
} AccessPointInfo;

uint8_t InitWifiSystem();

void ScanAccessPoints(AccessPointInfo* accessPoints, uint16_t* maxAccessPoints);

void ConnectToAccessPoint(AccessPointInfo* info, const char* password);

void HandleWifiEvent(esp_event_base_t eventBase, int32_t id, void* data);

uint8_t IsConnected();


#endif