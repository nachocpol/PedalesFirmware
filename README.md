# Pedales Firmware

This is the firmware for a project that tracks static bike activities using a hall sensor and an ESP32 micro.

The ESP32 will be running the main firmware, tracking the active activity and communicating with a Raspberry over HTTPS to store the information in an Influx DB (using MQTTP). This will be later parsed by a Grafana instance so the data can be displayed in a screen.
