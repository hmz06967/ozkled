#include <Arduino.h>
#include <WiFi.h>
#include <led.h>
#include <bt.h>

#include "esphome_api.h"

ESPHomeAPI api;

extern String ssid;
extern String password;
extern bool startwifi;

void wifi_setup() {

    loadWifiCredentials();

    if (ssid.length() == 0 || password.length() == 0) {
        Serial.println("WiFi credentials not set. Please set them via Bluetooth.");
        return;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());

    Serial.print("Connecting to WiFi");
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());

    Serial.println("ESPHome API started on port 6053");
    api.begin();

}

void setup(){

    Serial.begin(115200);
    while(!Serial) {
        delay(10);
    }
  
    Serial.println("Starting ESP32...");

    bt_setup();
    wifi_setup();
    led_setup();
}

void loop(){

    bt_loop();
    api.loop();

    if (startwifi) {
        wifi_setup();
        startwifi = false;
    }
}
