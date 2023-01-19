#ifndef UTIL_H
#define UTIL_H

#include "stdint.h"

// Utility method to delay the current thread by 'x' miliseconds
void Delay(uint32_t ms);

// Returns current system time in miliseconds
uint64_t GetSystemMS();

#endif