#pragma once

#include <Arduino.h>
#include <WiFi.h>

class ESPHomeAPI {

public:

    ESPHomeAPI();

    void begin();
    void loop();

private:

    WiFiServer server;
    WiFiClient client;

    bool connected = false;
    bool subscribed = false;

    void handleClient();
    void resetConnectionState();
    bool readFrame(uint8_t &messageType, uint8_t *data,uint16_t &length);
    void processMessage(uint8_t messageType, uint8_t *data, uint16_t length);
    void sendFrame(uint8_t messageType, const uint8_t *data, uint16_t length);

    void handleHello(const uint8_t *data, uint16_t length);

    void handleDeviceInfo();
    void handleListEntities();
    void handleSubscribeStates();
    void handleLightCommand(const uint8_t *data, uint16_t length);
    void sendLightState();

    uint32_t readFixed32(const uint8_t *data);
    float readFloat(const uint8_t *data);
    void writeFixed32(uint8_t *data, uint32_t value);
    void writeFloat(uint8_t *data, float value);
    uint8_t encodeVarint(uint8_t *buffer, uint32_t value);
    uint8_t encodeString(uint8_t *buffer, uint8_t field, const char *str);
};


