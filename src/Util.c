#include "Util.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sys/time.h"

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
    if(output == NULL)
    {
        return;
    }
    memset(output, 0, outputMaxLen); // Reset output array

    if(input == NULL)
    {
        return;
    }

    uint8_t index = 0;
    char* cur = (char*)input;
    while(*cur != '0')
    {
        output[index++] = *cur;
        ++cur;
    }
}