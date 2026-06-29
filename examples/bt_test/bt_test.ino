#include <Arduino.h>
#include <ozkled.h>

#define LED_PIN     13
#define LED_COUNT   200   // şerit LED sayısı

void setup() {
  Serial.begin(115200);
  ozkled.init(LED_COUNT, LED_PIN);
}

void loop() {
  delay(1000);
}
