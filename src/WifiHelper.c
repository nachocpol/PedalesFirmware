#include "WifiHelper.h"
#include "Config.h"
#include "Util.h"

#include "esp_log.h"
#include "esp_wifi.h"

#include "string.h"

struct WifiHelper g_WifiHelper;

uint8_t InitWifiSystem()
{
    if(g_WifiHelper.m_Initialized)
    {
        return 1;
    }

    g_WifiHelper.m_WifiEventGroup = xEventGroupCreate();

    // 1) Init general STA systems
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    uint8_t ssidArray[32];
    StrToArray(k_WifiSSID, ssidArray, 32);
    uint8_t passwordArray[64];
    StrToArray(k_WifiPassworkd, passwordArray, 64);

    // 2) Lets start by finding the default SSID (for settings and so on)
    int32_t foundNetwork = -1;
    wifi_ap_record_t foundRecords[10];
    {
        ESP_LOGI(k_LogTag, "Searching network: %s", k_WifiSSID);
        esp_wifi_scan_start(NULL, true);
        uint16_t maxRecords = 10;
        esp_wifi_scan_get_ap_records(&maxRecords, foundRecords);

        for(int i=0; i < maxRecords;++i)
        {
            char name[33];
            for(int c = 0; c < 33; ++c)
            {
                name[c] = foundRecords[i].ssid[c];
            }
            if(strcmp(name, k_WifiSSID) == 0)
            {
                foundNetwork = i;
                break;
            }
        }
        
        if(foundNetwork != -1)
        {
            ESP_LOGI(k_LogTag, "Wifi scan succeded, net strenght is: %i", foundRecords[foundNetwork].rssi);
        }
        else
        {
            ESP_LOGI(k_LogTag, "Wifi scan failed, could not find requested network");            
            return 0;// Exit with failed error
        }
    }
    esp_wifi_stop();

    // 3) Finally, just go ahead and connect to it
    esp_wifi_start();
    wifi_config_t wifi_config;
    memcpy(wifi_config.sta.ssid, ssidArray, 32);
    memcpy(wifi_config.sta.password, passwordArray, 64);
    wifi_config.sta.threshold.authmode = foundRecords[foundNetwork].authmode;
    wifi_config.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;
    
    esp_err_t resultCode = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    ESP_LOGI(k_LogTag, "Wifi config set result [%i]", resultCode);
    resultCode = esp_wifi_connect();
    ESP_LOGI(k_LogTag, "Wifi connect results [%i]", resultCode);
    ESP_ERROR_CHECK(resultCode);

    g_WifiHelper.m_Initialized = 1; // Tag as initialized... Need to reboot to try again (if needed)

    return resultCode == ESP_OK ? 1 : 0;
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