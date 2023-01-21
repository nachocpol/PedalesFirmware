#include "WifiHelper.h"
#include "Config.h"
#include "Util.h"

#include "esp_log.h"
#include "esp_wifi.h"

#include "string.h"

struct WifiHelper g_WifiHelper;

uint8_t InitWifiSystem()
{
    if(g_WifiHelper.m_Initialized == 1)
    {
        return 1;
    }

    g_WifiHelper.m_WifiEventGroup = xEventGroupCreate();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t wifiConfig = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifiConfig));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    g_WifiHelper.m_Initialized = 1;

    return 1;
}

void ScanAccessPoints(AccessPointInfo* accessPoints, uint16_t* maxAccessPoints)
{
    if(g_WifiHelper.m_Initialized == 0)
    {
        *maxAccessPoints = 0;
        return;
    }

    esp_wifi_scan_start(NULL, true);
    wifi_ap_record_t scannedAccessPoints[12];
    uint16_t totalAccessPoints = 12;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&totalAccessPoints, scannedAccessPoints));

    if(totalAccessPoints < *maxAccessPoints)
    {
        *maxAccessPoints = totalAccessPoints;
    }

    // Store found APs in the array (figure out array size based on the user input)
    for(int i = 0; i < *maxAccessPoints; ++i)
    {
        char ssidName[33];
        ArrayToStr(scannedAccessPoints[i].ssid, ssidName, 33);
        
        // Copy to the output array
        memcpy(accessPoints[i].m_SSID, ssidName, 33);
        accessPoints[i].m_RSSI = scannedAccessPoints[i].rssi;
        accessPoints[i].m_AuthMode = scannedAccessPoints[i].authmode;        
    }
}

void ConnectToAccessPoint(AccessPointInfo* info, const char* password)
{
    if(info == NULL)
    {
        return;
    }

    uint8_t staPassword[64];
    StrToArray(password, staPassword, 64);

    wifi_config_t staConfig = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };

    ESP_LOGI(k_LogTag, "Mode %i", (int)staConfig.sta.threshold.authmode);
    ESP_LOGI(k_LogTag, "Password: %s", password);
    ESP_LOGI(k_LogTag, "SSID: %s", info->m_SSID);

    memcpy(staConfig.sta.ssid, info->m_SSID, 33);
    memcpy(staConfig.sta.password, k_WifiPassword, 64);

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &staConfig));
    ESP_ERROR_CHECK(esp_wifi_connect());
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