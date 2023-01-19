#include "WifiHelper.h"

#include "esp_event.h"
#include "esp_wifi.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

uint8_t InitWifiSystem()
{
    if(g_WifiHelper.m_Initialized)
    {
        return 1;
    }

    //g_WifiHelper.m_WifiEventGroup = xEventGroupCreate();

    esp_netif_create_default_wifi_sta();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    return 1;
}

void HandleWifiEvent(esp_event_base_t eventBase, int32_t id, void* data)
{
    /*
    if(eventBase == WIFI_EVENT)
    {
        if(id == WIFI_EVENT_STA_START)
        {
            esp_wifi_connect();
        }
        else if(id == WIFI_EVENT_STA_DISCONNECTED)
        {
            xEventGroupClearBits(g_WifiHelper.m_WifiEventGroup, BIT0);
        }
    }
    else if(eventBase == IP_EVENT)
    {
        if(id == IP_EVENT_STA_GOT_IP)
        {
            // We are now connected to the station
            xEventGroupSetBits(g_WifiHelper.m_WifiEventGroup, BIT0);
        }
    }
    */
}