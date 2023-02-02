// Config.h
//  General configuration settings for the program. Feel free to change
//  any value as required

#ifndef CONFIG_H
#define CONFIG_H

#include "driver/gpio.h"

// General settings
static const char* k_VersionStr = "1_3";
static const char* k_LogTag = "Firmware";
static const gpio_num_t k_LedPin = GPIO_NUM_2;
static const gpio_num_t k_SensorPin = GPIO_NUM_4;

// Meassurement of the tire circumference in meters
static const float k_RevCircumference = 1.75f;
// How many revolutions do we need to detect before considering a valid start
static const uint8_t k_RevsBeforeStart = 4;
// How much time do we have (in ms) to perform the initial revolutions
static const uint32_t k_TimeBeforeStart = 8000;
// Once the activity started, after how long should we stop the activity if we don't
// see any revolutions (in ms)
static const uint32_t k_ActivityTimeOut = 15000;
// How often should we send activity packets to the database (if the activity is running) in ms.
static const uint32_t k_SendPacketDelta = 3000;

// Network settings
static const char* k_WifiSSID = "VM2930766";
static const char* k_WifiPassword = "yy6nnSmcdgvj";
// RaspberryIP needed to send the InfluxDB meassurements
static const char* k_RaspberryIP = "192.168.0.29";

// InfluxDB configuration
static const char* k_InfluxOrg = "pedalesOrg";  
static const char* k_InfluxBucket = "pedales";
static const char* k_InfluxToken = "p27Ksj5h4Rm-PU_vuBvXtHyJxUyjhFqj6mCIR0Pkr5W4Z1xTp1Jxu6-amp6rAX23oQqVjwcXv8MRoMzuK2YtIw==";

#endif