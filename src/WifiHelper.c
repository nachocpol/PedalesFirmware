#include "WifiHelper.h"
#include "Config.h"

#include "esp_wifi.h"

struct WifiHelper g_WifiHelper;

uint8_t InitWifiSystem()
{
    if(g_WifiHelper.m_Initialized)
    {
        return 1;
    }

    g_WifiHelper.m_WifiEventGroup = xEventGroupCreate();

    esp_netif_create_default_wifi_sta();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

     wifi_config_t wifi_config = 
     {
        .sta = 
        {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORKD,
            .threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    return 1;
}

void HandleWifiEvent(esp_event_base_t eventBase, int32_t id, void* data)
{
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
}