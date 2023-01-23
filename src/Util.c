// Util.c

#include "Util.h"
#include "Config.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "sys/time.h"
#include "esp_log.h"

#include "string.h"

void Delay(uint32_t ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

uint64_t GetSystemMS()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    int64_t timeUS = (int64_t)now.tv_sec * 1000000L + (int64_t)now.tv_usec;
    return (uint64_t)timeUS / (uint64_t)1000;
}

void StrToArray(const char* input, uint8_t* output, uint8_t outputMaxLen)
{
    if(output == NULL || input == NULL)
    {
        return;
    }
    memset(output, 0, outputMaxLen); 
    uint8_t index = 0;
    char* curChar = (char*)input;
    while(*curChar != '\0')
    {
        output[index++] = *curChar;
        if(index >= outputMaxLen)
        {
            return;
        }
        ++curChar;
    }
}

void ArrayToStr(uint8_t* input, char* output, uint8_t outputMaxLen)
{
    if(output == NULL || input == NULL)
    {
        return;
    }
    memset(output, 0, outputMaxLen); 
    uint8_t index = 0;
    uint8_t* curVal = (uint8_t*)input;
    while(*curVal != '\0')
    {
        output[index++] = (char)*curVal;
        if(index >= outputMaxLen)
        {
            return;
        }
        ++curVal;
    }
}

void BuildInfluxPacket(InfluxData data, char* outputData, uint32_t outputMaxSize)
{
    sprintf(outputData, "pedalesDatum,esp_id=%s,activity_index=%li revolutions=%li\i,totalDistance=%f,speed=%f",
        data.m_MAC, data.m_ActivityIndex, data.m_Revolutions, data.m_TotalDistance, data.m_Speed
    );
}

#define RESET_EVERY_TIME  0

int32_t GetNextUniqueIndex()
{
    nvs_handle_t handle;
    esp_err_t result = nvs_open("firmware", NVS_READWRITE, &handle);
    if(result != ESP_OK)
    {
        ESP_LOGI(k_LogTag, "Failed to open NVS!");
        return -1;
    }
    else
    {
        int32_t counter = 0;
        result = nvs_get_i32(handle, "counter", &counter);
        switch (result)
        {
            case ESP_OK:
            case ESP_ERR_NVS_NOT_FOUND:
                // This is all ok.
            break;
            default:
                ESP_LOGI(k_LogTag, "Failed to retrieve counter value NVS!");
                break;
        }

        counter++; // increment it

    #if RESET_EVERY_TIME
        counter = 0;
    #endif

        nvs_set_i32(handle, "counter", counter);

        nvs_commit(handle);
        nvs_close(handle);

        return counter;
    }
}