#pragma once

#include <Arduino.h>

#define LED_COUNT 200

void led_setup();
void clearPixel();
void setPixelColorSafe(int index, uint8_t r, uint8_t g, uint8_t b);
void applyColor(uint8_t r, uint8_t g, uint8_t b, int index);
void led_setBrightness(uint8_t value);
void led_setLength(int newLength);
