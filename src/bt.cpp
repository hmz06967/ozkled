#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include "led.h"
#include <BLE2902.h>
#include <Preferences.h>

#include "bt.h"

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;

bool deviceConnected = false;
bool oldDeviceConnected = false;
bool startwifi = false;

String ssid = "";
String password = "";
static Preferences preferences;

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"  // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

//Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

int R = 0;
int G = 0;
int B = 0;
int I = 0;
int L = 0;
int NL = LED_COUNT;
float gamma_table[4];

int gamma_value = 1;
int brightness_value = 0;


bool detectSsidAndPassword(String cmd) {
    //"WIFI:$ssid,$password";

  int ssidIndex = cmd.indexOf("WIFI:");
  int passwordIndex = cmd.indexOf(",");

  if (ssidIndex != -1 && passwordIndex != -1) {
    ssid = (String)cmd.substring(ssidIndex + 5, passwordIndex);
    password = (String)cmd.substring(passwordIndex + 1);

    Serial.print("Detected SSID: ");
    Serial.println(ssid);
    Serial.print("Detected Password: ");
    Serial.println(password);

    saveWifiCredentials();
    startwifi = true;
    return true;
  }
  return false;
}

void loadWifiCredentials() {
  preferences.begin("wifi", true);
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");
  preferences.end();

  if (ssid.length() > 0 && password.length() > 0) {
    Serial.println("Loaded WiFi credentials from storage");
    Serial.print("SSID: ");
    Serial.println(ssid);
  }
}

void saveWifiCredentials() {
  preferences.begin("wifi", false);
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  preferences.end();
}

void parseCommand2(String cmd){
    
    cmd.trim();

    if(detectSsidAndPassword(cmd)) {
        return;
    }

    if (cmd.indexOf(',') != -1) {
        parseCommand(cmd);
    } else {

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
                led_setBrightness((uint8_t)brightness_value);
                break;
        }
    }
}

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

    r = cmd.substring(rPos + 1, gPos - 1).toInt();
    g = cmd.substring(gPos + 1, bPos - 1).toInt();
    b = cmd.substring(bPos + 1, iPos - 1).toInt();


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
            led_setLength(newL);
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
};

void bt_setup() {
  

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

  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_WRITE_NR);

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");


}

void bt_loop() {

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
}
