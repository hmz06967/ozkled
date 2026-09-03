#include "led.h"

#include <FastLED.h>

#define LED_PIN 13
#define LED_COUNT 200

static CRGB leds[LED_COUNT];
static int currentLedCount = LED_COUNT;

static int red = 0;
static int green = 0;
static int blue = 0;

void led_setup()
{
    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LED_COUNT);
    FastLED.clear();
    FastLED.show();
}

void clearPixel()
{
    for (int i = 0; i < currentLedCount; i++) {
        leds[i] = CRGB::Black;
    }

    FastLED.show();
}

void setPixelColorSafe(int index, uint8_t r, uint8_t g, uint8_t b)
{
    if (index >= 0 && index < currentLedCount) {
        leds[index] = CRGB(r, g, b);
    }
}

void applyColor(uint8_t r, uint8_t g, uint8_t b, int index)
{
    red = r;
    green = g;
    blue = b;

    if (index == -1) {
        for (int i = 0; i < currentLedCount; i++) {
            leds[i] = CRGB(r, g, b);
        }
    } else {
        setPixelColorSafe(index, r, g, b);
    }

    FastLED.show();
}

void led_setBrightness(uint8_t value)
{
    FastLED.setBrightness(value);
    FastLED.show();
}

void led_setLength(int newLength)
{
    if (newLength < 1) {
        newLength = 1;
    }

    if (newLength > LED_COUNT) {
        newLength = LED_COUNT;
    }

    currentLedCount = newLength;
    clearPixel();
    applyColor(red, green, blue, -1);
}
