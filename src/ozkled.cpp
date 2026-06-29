#include <ozkled.h>
#include <Arduino.h>

Ozkled ozkled;

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"  // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"


class CONNCallbacks : public BLEServerCallbacks {
  bool deviceConnected = false;
  
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    Serial.println("Device connected");
  };

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    Serial.println("Device disconnected");
  }
};

class RXCallbacks : public BLECharacteristicCallbacks {
  

  bool debug = false;
  
  void (*currentParseCallback)(String) = nullptr;

  void setParseCommandCallback(void (*callback)(String)) {
      currentParseCallback = callback;
  }

  void setDebug(bool dbgmode){
    debug = dbgmode;
  }

  void printUart(String rxValue){
    
    if (!debug)return;

    if (rxValue.length() > 0) {
      Serial.println("*********");
      Serial.print("Received Value: ");
      for (int i = 0; i < rxValue.length(); i++) {
        char c = rxValue[i];
        Serial.print(c);
      }
      Serial.println();
      Serial.println("*********");
    }
    Serial.flush();
  }

  void onWrite(BLECharacteristic *pCharacteristic) {
    String rxValue = pCharacteristic->getValue();

    if (currentParseCallback != nullptr) {
        currentParseCallback(rxValue);
        printUart(rxValue);
    } else {
        // Varsayılan işlem: boş komutu logla ya da ignore et
        Serial.println("No parse callback set.");
    }
    
    /**/

  }
};


void Ozkled::init(int led_size, int led_pin){
  
  _led_count = led_size;
  _led_pin = led_pin;

  pdevice.init("Ozkled_LED");

  pServer = pdevice.createServer();
  pService = pServer->createService(SERVICE_UUID);
  pServer->setCallbacks(new CONNCallbacks());

  BLECharacteristic * pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
  pTxCharacteristic->addDescriptor(new BLE2902());
  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
  pRxCharacteristic->setCallbacks(this);

  pService->start();
  pServer->getAdvertising()->start();
  
  neopixel = new Adafruit_NeoPixel(_led_count, _led_pin, NEO_GRB + NEO_KHZ800);

  Serial.println("Waiting a client connection to notify...");

}

void Ozkled::setNeoPixel(Adafruit_NeoPixel *_neopixel) {
  if (_neopixel == nullptr) return;
  neopixel = _neopixel;
  if (neopixel->numPixels() <= 0 || neopixel->getPin() < 0) {
      Serial.println("Invalid NeoPixel setup!");
      return;
  }
}

void Ozkled::clearPixel() {
  for (int i = 0; i < _led_count; i++) {
    neopixel->setPixelColor(i, neopixel->Color(0, 0, 0));
  }
  neopixel->show();
}

void Ozkled::setPixelColorSafe(int index, uint8_t r, uint8_t g, uint8_t b) {
  if (index >= 0 && index < _led_count) {
    neopixel->setPixelColor(index, neopixel->Color(r, g, b));
  }
}

void Ozkled::applyColor(uint8_t r, uint8_t g, uint8_t b, int index) {
  if (index == -1) {
    for (int i = 0; i < _led_count; i++) {
      neopixel->setPixelColor(i, neopixel->Color(r, g, b));
    }
  } else {
    neopixel->setPixelColor(index, neopixel->Color(r, g, b));
    //setPixelColorSafe(index, r, g, b);
  }
  neopixel->show();
}

void Ozkled::parseCommand(String cmd) {
  cmd.trim();

  int r = 0, g = 0, b = 0;
  int index = -2; // invalid default
  int newL = _led_count;

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
      neopixel->updateLength(newL);
    }
  }

  applyColor(r, g, b, index);
}
