#ifndef WIFIHELPER_H
#define WIFIHELPER_H

#include "stdint.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"

struct WifiHelper
{
    uint8_t m_Initialized;
    EventGroupHandle_t m_WifiEventGroup;
};

uint8_t InitWifiSystem();

void HandleWifiEvent(esp_event_base_t eventBase, int32_t id, void* data);

#endif