#ifndef OZKLED_H
#define OZKLED_H

#include <Adafruit_NeoPixel.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

class Ozkled: public BLECharacteristicCallbacks {
    public:
        void onWrite(BLECharacteristic *pCharacteristic) override {
            String rxValue = pCharacteristic->getValue();

            if (rxValue.length() == 0 || rxValue == "null") return;

            Serial.printf("Gelen veri: %s\n", rxValue.c_str());

            parseCommand(rxValue);
        }

    public :

      bool deviceConnected = false;
      
      void init(int led_size, int led_pin);
      void setNeoPixel(Adafruit_NeoPixel *_neopixel);
      void applyColor(uint8_t r, uint8_t g, uint8_t b, int index);
      void setPixelColorSafe(int index, uint8_t r, uint8_t g, uint8_t b);
      void clearPixel(void);


    private:
      int _led_count = 200;
      int _led_pin = 13;
      Adafruit_NeoPixel *neopixel;

      BLEDevice pdevice;
      BLEServer *pServer;
      BLEService *pService;
      BLECharacteristic *pChar;

      uint8_t _chunkSize;           // max bytes per packet
      uint8_t _buffer[256];         // internal buffer (size based on chunking)
      int _currentIndex = 0;        // current index in buffer
      size_t _bufferLength = 0;

      void addToBuffer(uint8_t r, uint8_t g, uint8_t b);

      void parseCommand(String cmd);
 
};

// Global instance (optional) - use as: Ozkled myOzkled;
extern Ozkled ozkled;

#endif
