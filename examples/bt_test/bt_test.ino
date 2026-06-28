#include <Arduino.h>
#include "Ozkled.h"

#define LED_PIN     13
#define LED_COUNT   200   // şerit LED sayısı

Ozkled ozkled;

void setup() {
  Serial.begin(115200);
  ozkled.init(LED_COUNT, LED_PIN);
}

void loop() {
  // Example: Turn on red pixel at index 0
  ozkled.setPixel(0, 255, 0, 0);
  
  delay(1000);

  // Send buffer (blinks red)
  if (ozkled.sendBuffer()) {
    Serial.println("LED buffer sent!");
  }
}
