#include "WifiHelper.h"
#include "Config.h"
#include "Util.h"

#include "esp_log.h"
#include "esp_wifi.h"

#include "string.h"

struct WifiHelper g_WifiHelper;

uint8_t InitWifiSystem()
{
    g_WifiHelper.m_WifiEventGroup = xEventGroupCreate();
    
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t wifiConfig = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifiConfig));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Scan for APs and match the one we are looking for
    wifi_ap_record_t targetAP;
    {
        esp_wifi_scan_start(NULL, true);
        wifi_ap_record_t accessPoints[12];
        uint16_t totalAccessPoints = 12;
        ESP_LOGI(k_LogTag, "Start scanning for APs...");
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&totalAccessPoints, accessPoints));
        ESP_LOGI(k_LogTag, "Found:%u APs", totalAccessPoints);
        for(int i = 0; i < totalAccessPoints; ++i)
        {
            char ssidName[33];
            ArrayToStr(accessPoints[i].ssid, ssidName, 33);
            if(strcmp(ssidName, k_WifiSSID) == 0)
            {
                // Keep a copy around
                targetAP = accessPoints[i];
                ESP_LOGI(k_LogTag, "\t >%s[%i]<", ssidName, accessPoints[i].rssi); 
            }
            else
            {
                ESP_LOGI(k_LogTag, "\t %s[%i]", ssidName, accessPoints[i].rssi); 
            }
        }
    }

    // Connect to AP we care about now
    {
        uint8_t staPassword[64];
        StrToArray(k_WifiPassworkd, staPassword, 64);
        uint8_t staSSID[33];
        StrToArray(k_WifiSSID, staSSID, 33);

        wifi_config_t staConfig = {
            .sta = {
                .threshold.authmode = targetAP.authmode,
                .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
            },
        };
        memcpy(staConfig.sta.ssid, staSSID, 33);
        memcpy(staConfig.sta.password, staPassword, 64);

        ESP_LOGI(k_LogTag, "Connecting to:%s", staSSID);
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &staConfig));
        ESP_ERROR_CHECK(esp_wifi_connect());
    }

    return 1;
}

uint8_t IsConnected()
{
    return (xEventGroupGetBits(g_WifiHelper.m_WifiEventGroup) & BIT0);
}

void HandleWifiEvent(esp_event_base_t eventBase, int32_t id, void* data)
{
    if(eventBase == WIFI_EVENT)
    {
        if(id == WIFI_EVENT_STA_DISCONNECTED)
        {
            ESP_LOGI(k_LogTag, "Wifi disconnected");
            xEventGroupClearBits(g_WifiHelper.m_WifiEventGroup, BIT0);
        }
        else if(id == WIFI_EVENT_STA_START)
        {
            ESP_LOGI(k_LogTag, "Wifi started");
        }
        else 
        {
            ESP_LOGI(k_LogTag, "Wifi event [%li]", id);
        }
    }
    else if(eventBase == IP_EVENT)
    {
        if(id == IP_EVENT_STA_GOT_IP)
        {
            ip_event_got_ip_t* event = (ip_event_got_ip_t*)data;
            ESP_LOGI(k_LogTag, "Device IP:" IPSTR, IP2STR(&event->ip_info.ip));

            // We are now connected to the station
            xEventGroupSetBits(g_WifiHelper.m_WifiEventGroup, BIT0);
        }
        else
        {
            ESP_LOGI(k_LogTag, "IP event [%li]", id);
        }
    }
}