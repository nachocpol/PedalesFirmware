// Util.h
//  Implements utility methods that don't have a better place to be

#ifndef UTIL_H
#define UTIL_H

#include "stdint.h"

// Utility method to delay the current thread by 'x' miliseconds
void Delay(uint32_t ms);

// Returns current system time in miliseconds
uint64_t GetSystemMS();

void StrToArray(const char* input, uint8_t* output, uint8_t outputMaxLen);

void ArrayToStr(uint8_t* input, char* output, uint8_t outputMaxLen);

typedef struct
{
    const char* m_MAC;
    int32_t m_ActivityIndex;
    int32_t m_Revolutions;
    float m_TotalDistance;
    float m_Speed;
}InfluxData;
void BuildInfluxPacket(InfluxData data, char* outputData, uint32_t outputMaxSize);

// Used to return incrementing indices. Values stored in NVS so values increase every
// time we call this method
int32_t GetNextUniqueIndex();

#endif