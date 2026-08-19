#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <Adafruit_NeoPixel.h>

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"  // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define LED_PIN     13
#define LED_COUNT   200   // şerit LED sayısı

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

int R = 0;
int G = 0;
int B = 0;
int I = 0;
int L = 0;
int NL = LED_COUNT;
float gamma_table[4];

int gamma_value = 0;
int brightness_value = 0;

void clearPixel();
void setPixelColorSafe(int index, uint8_t r, uint8_t g, uint8_t b);
void applyColor(uint8_t r, uint8_t g, uint8_t b, int index);
void applyRGB(uint8_t *r, uint8_t *g, uint8_t *b);
uint8_t applyGammaBrightness(uint8_t value);

void parseCommand(String cmd);
void setup();
void loop();

void clearPixel() {
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
    strip.show();
}

void setPixelColorSafe(int index, uint8_t r, uint8_t g, uint8_t b) {
  if (index >= 0 && index < LED_COUNT) {
    strip.setPixelColor(index, strip.Color(r, g, b));
  }
}

uint8_t applyGammaBrightness(uint8_t value){
    uint16_t v = (uint16_t)value * brightness_value / 100;
    float normalized = (float)v / 255.0f;
    float corrected = powf(normalized, v);
    return (uint8_t)(corrected * 255.0f + 0.5f);
}

void applyRGB(uint8_t *r, uint8_t *g, uint8_t *b){
    *r = applyGammaBrightness(*r);
    *g = applyGammaBrightness(*g);
    *b = applyGammaBrightness(*b);
}

void applyColor(uint8_t r, uint8_t g, uint8_t b, int index) {
  
  applyRGB(&r, &g, &b);
    
  if (index == -1) {
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, strip.Color(r, g, b));
    }
  } else {
    strip.setPixelColor(index, strip.Color(r, g, b));
    
    //setPixelColorSafe(index, r, g, b);
  }

  strip.show();
}
void parseCommand2(String cmd)
{
    if (cmd.indexOf(',') != -1)
    {
        while (cmd.length() > 0)
        {
            int comma = cmd.indexOf(',');
            String token;

            if (comma == -1)
            {
                token = cmd;
                cmd = "";
            }
            else
            {
                token = cmd.substring(0, comma);
                cmd = cmd.substring(comma + 1);
            }

            if (token.length() < 2)
                continue;

            char type = token.charAt(0);
            int value = token.substring(1).toInt();

            switch (type)
            {
                case 'R':
                    R = value;
                    break;

                case 'G':
                    G = value;
                    break;

                case 'B':
                    B = value;
                    break;

                case 'I':
                    I = value;
                    break;

                case 'L':
                    L = value;
                    break;
            }
        }
    }
    else
    {
        if (cmd.length() < 2)
            return;

        char type = cmd.charAt(0);
        int value = cmd.substring(1).toInt();

        switch (type)
        {
            case 'G':
                gamma_value = value;
                break;

            case 'B':
                brightness_value = value;
                break;
        }
    }

    // LED sayısı değiştiyse
    if (L >= 0 && L <= 300)
    {
        if (L != NL)
        {
            clearPixel();

            NL = L;

            if (NL > 0)
            {
                strip.updateLength(NL);
            }
        }
    }

    // RGB uygula
    applyColor(R, G, B, I);
}
// R0,G0,B255,I-1,L20
void parseCommand(String cmd) {
  cmd.trim();

  int r = 0, g = 0, b = 0;
  int index = -2; // invalid default
  int newL = LED_COUNT;

  int rPos = cmd.indexOf("R");
  int gPos = cmd.indexOf("G");
  int bPos = cmd.indexOf("B");
  int iPos = cmd.indexOf("I");
  int lPos = cmd.indexOf("L");

  if (rPos >= 0 && gPos > rPos && bPos > gPos) {
    r = cmd.substring(rPos + 1, gPos - 1).toInt();
    g = cmd.substring(gPos + 1, bPos - 1).toInt();
    b = cmd.substring(bPos + 1, iPos - 1).toInt();
  }

  if (iPos >= 0) {
    String iStr;
    if (lPos > iPos)
      iStr = cmd.substring(iPos + 1, lPos - 1);
    else
      iStr = cmd.substring(iPos + 1);
    index = iStr.toInt();
  }

  if (lPos >= 0) {
    clearPixel();
    newL = cmd.substring(lPos + 1).toInt();
    if (newL > 0 && newL <= 300) {
      strip.updateLength(newL);
    }
  }

  applyColor(r, g, b, index);
}

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    Serial.println("Device connected");
  };

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    Serial.println("Device disconnected");
  }
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String rxValue = pCharacteristic->getValue();
    parseCommand2(rxValue);
    
    /*if (rxValue.length() > 0) {
      Serial.println("*********");
      Serial.print("Received Value: ");
      for (int i = 0; i < rxValue.length(); i++) {
        char c = rxValue[i];
        Serial.print(c);
      }
      Serial.println();
      Serial.println("*********");
    }

    Serial.flush();*/

  }
};

void setup() {
  Serial.begin(115200);

  strip.begin();
  strip.show();

  // Create the BLE Device
  BLEDevice::init("UART Service");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);

  // Descriptor 2902 is not required when using NimBLE as it is automatically added based on the characteristic properties
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");


}

void loop() {

  /*if (deviceConnected) {
    Serial.print("Notifying Value: ");
    Serial.println(txValue);
    pTxCharacteristic->setValue(&txValue, 1);
    pTxCharacteristic->notify();
    txValue++;
    delay(1000);  // Notifying every 1 second
  }*/

  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);                   // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising();  // restart advertising
    Serial.println("Started advertising again...");
    oldDeviceConnected = false;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = true;
  }

  delay(10);
}

