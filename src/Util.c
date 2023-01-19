#include "Util.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sys/time.h"

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