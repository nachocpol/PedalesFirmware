#ifndef WIFIHELPER_H
#define WIFIHELPER_H

#include "stdint.h"

#include "esp_event_base.h"
#include "freertos/event_groups.h"

struct WifiHelper
{
    uint8_t m_Initialized;
    //EventGroupHandle_t m_WifiEventGroup;
} g_WifiHelper;

uint8_t InitWifiSystem();

void HandleWifiEvent(esp_event_base_t eventBase, int32_t id, void* data);

#endif