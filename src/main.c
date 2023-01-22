// main.c

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_smartconfig.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "string.h"

#include "Config.h"
#include "Util.h"
#include "WifiHelper.h"

// Will hold the active state of the activity we are monitoring
struct State
{
    bool m_ActivityInProgress;
    int32_t m_TotalRevolutions;
    uint64_t m_InitialDetection;
    uint64_t m_LastTimeStampMS;
    int64_t m_PacketTimerMS;
} g_State;

// Sensor interrup will toggle this
bool g_SensorTriggered = false;

void IRAM_ATTR SensorInterruptHandler(void* arg);
void ResetState();
void Initialize();
void Loop(uint64_t deltaMS);

/////////////////////////////////////////////
void app_main() 
{
    Initialize();

    uint64_t dt = 0;
    uint64_t start = 0;
    uint64_t end = 0;

    while(true)
    {
        start = GetSystemMS();
        Loop(dt);
        Delay(10); // Wait before the next iteration
        end = GetSystemMS();
        dt = end - start;
    }
}
/////////////////////////////////////////////

static void SystemEventHandler(void* arg, esp_event_base_t eventBase, int32_t id, void* data)
{
    if(eventBase == WIFI_EVENT || eventBase == IP_EVENT || eventBase == SC_EVENT)
    {
        HandleWifiEvent(eventBase, id, data);
    }
    else
    {
        // Not handled
    }
}

void SensorInterruptHandler(void* arg)
{
    // Interrup code!
    g_SensorTriggered = true;
}

void ResetState()
{
    g_State.m_ActivityInProgress = false;
    g_State.m_LastTimeStampMS = 0;
    g_State.m_TotalRevolutions = 0;
    g_State.m_InitialDetection = 0;
    g_State.m_PacketTimerMS = 0;
}

void Initialize()
{
    ESP_LOGI(k_LogTag, "Initializing...");

    // Need to install this service before adding any interrupt handlers to the GPIOs
    gpio_install_isr_service(0);

    // Init the NVS system
    ESP_ERROR_CHECK(nvs_flash_init());

    // Init TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_init());

    // Setup event handling and register the events we care about
    {
        ESP_ERROR_CHECK(esp_event_loop_create_default());

        // Register the event handlers we care about.. we could add more
        esp_event_handler_instance_t wifiEventHandler;
        ESP_ERROR_CHECK(
            esp_event_handler_instance_register(
                WIFI_EVENT, ESP_EVENT_ANY_ID, &SystemEventHandler, NULL, &wifiEventHandler
            )
        );
        esp_event_handler_instance_t ipEventHanlder;
        ESP_ERROR_CHECK(
            esp_event_handler_instance_register(
                IP_EVENT, IP_EVENT_STA_GOT_IP, &SystemEventHandler, NULL, &ipEventHanlder
            )
        );
        esp_event_handler_instance_t scEventHandler;
        ESP_ERROR_CHECK(
            esp_event_handler_instance_register(
                SC_EVENT, ESP_EVENT_ANY_ID, &SystemEventHandler, NULL, &scEventHandler
            )
        );
    }

    InitWifiSystem();

    // Scan APs
    AccessPointInfo apInfos[10];
    uint16_t totalAps = 10;
    int targetAp = -1;
    ScanAccessPoints(apInfos, &totalAps);

    // Find the one we care about
    for(int apIndex = 0; apIndex < totalAps; ++apIndex)
    {
        if(strcmp(k_WifiSSID, apInfos[apIndex].m_SSID) == 0)
        {
            targetAp = apIndex;
            ESP_LOGI(k_LogTag, "\t >%s [%i]<", apInfos[apIndex].m_SSID, apInfos[apIndex].m_RSSI);
        }
        else
        {
            ESP_LOGI(k_LogTag, "\t %s [%i]", apInfos[apIndex].m_SSID, apInfos[apIndex].m_RSSI);
        }

    }

    // Connect to it
    if(targetAp != -1)
    {
        ConnectToAccessPoint(&apInfos[targetAp], k_WifiPassword);
    }
    else
    {
        ESP_LOGI(k_LogTag, "Could not find the dessired AP...");
    }

    // Configure LED pin
    gpio_set_direction(k_LedPin, GPIO_MODE_OUTPUT);

    // Configure sensor pin (used as interrupt source)
    gpio_set_direction(k_SensorPin, GPIO_MODE_INPUT);
    gpio_set_intr_type(k_SensorPin, GPIO_INTR_NEGEDGE); // Interrup on lowering edge
    gpio_isr_handler_add(k_SensorPin,SensorInterruptHandler, NULL);

    // Ensure we have a clean state before starting
    ResetState();
}

void Loop(uint64_t deltaMS)
{
    ESP_LOGI(k_LogTag, "Wifi connected = %i",IsConnected());

    // Utility code to toggle the led when the sensor is activated, this is useful to verify the sensor
    // is reading as expected. Note that the actual toggling is done thorugh an interrupt to ensure we never
    // miss it
    {
        // 1 = Detected magnet
        // 0 = Nothing
        int32_t sensorLevel = 1 - gpio_get_level(k_SensorPin);
        gpio_set_level(k_LedPin, sensorLevel);
    }

    const uint64_t currentTime = GetSystemMS();

    // Sensor was triggered by the interrupt
    if(g_SensorTriggered)
    {
        ESP_LOGI(k_LogTag, "Detected sensor");

        g_SensorTriggered = false;
        
        g_State.m_LastTimeStampMS = currentTime;
        
        // First rev, store tentative start
        if(g_State.m_TotalRevolutions == 0)
        {
            g_State.m_InitialDetection = g_State.m_LastTimeStampMS;
            ESP_LOGI(k_LogTag, "[%llu] Setting initial detection at:%llu", currentTime, g_State.m_InitialDetection);
        }
        
        ++g_State.m_TotalRevolutions;

        ESP_LOGI(k_LogTag, "[%llu] Total revolutions:%li", currentTime, g_State.m_TotalRevolutions);
    }

    // Confirm the activity has started
    if(!g_State.m_ActivityInProgress)
    {
        // If we have performed at least 'k_RevsBeforeStart' within the limited time 'k_TimeBeforeStart' lets now start the activity
        const uint64_t elapsedSinceTentative = currentTime - g_State.m_InitialDetection;
        if( g_State.m_TotalRevolutions >= k_RevsBeforeStart
             && elapsedSinceTentative <= k_TimeBeforeStart)
        {
            g_State.m_ActivityInProgress = true;
            ESP_LOGI(k_LogTag, "[%llu] Activity is now fully in progress", currentTime);
        }
        else if(elapsedSinceTentative > k_TimeBeforeStart && g_State.m_InitialDetection > 0u)
        {
            // Timeout
            ESP_LOGI(k_LogTag, "[%llu] Activity discarded", currentTime);
            ResetState();
        }
        else
        {
            // Activity start is in progress...  Wait
        }
    }
    else
    {
        // Activity is on-going
        const uint64_t elapsed = currentTime - g_State.m_LastTimeStampMS;
        if(elapsed >= k_ActivityTimeOut)
        {
            // Its been too long since the last sensor read. Activity has timed out
            ResetState();
            ESP_LOGI(k_LogTag, "[%llu] Activity timed out, re-setting", currentTime);
        }

        // Check if we need to send a new packet
        g_State.m_PacketTimerMS += deltaMS;
        if(g_State.m_PacketTimerMS >= k_SendPacketDelta)
        {
            g_State.m_PacketTimerMS = 0;
            float totalDistance = (float)g_State.m_TotalRevolutions * k_RevCircumference;
            ESP_LOGI(k_LogTag, "[%llu] Packet: %li, %f m.", currentTime, g_State.m_TotalRevolutions, totalDistance);
        }
    }
}