#include "esphome_api.h"

#include <string.h>
#include <led.h>

bool lightState = false;
float brightness = 1.0f;
float red = 1.0f;
float green = 1.0f;
float blue = 1.0f;

static const uint8_t MSG_HELLO_REQUEST             = 1;
static const uint8_t MSG_HELLO_RESPONSE            = 2;

static const uint8_t MSG_DEVICE_INFO_REQUEST       = 4;
static const uint8_t MSG_DEVICE_INFO_RESPONSE      = 5;

static const uint8_t MSG_LIST_ENTITIES_REQUEST     = 11;
static const uint8_t MSG_LIST_ENTITIES_LIGHT       = 15;
static const uint8_t MSG_LIST_ENTITIES_DONE        = 19;

static const uint8_t MSG_SUBSCRIBE_STATES_REQUEST  = 20;
static const uint8_t MSG_LIGHT_STATE               = 24;
static const uint8_t MSG_LIGHT_COMMAND             = 32;

ESPHomeAPI::ESPHomeAPI() :
    server(6053)
{
}


void ESPHomeAPI::begin()
{
    server.begin();

    Serial.println("ESPHome Native API");
    Serial.println("Port: 6053");
}


void ESPHomeAPI::loop()
{
    if (!client || !client.connected()) {

        if (client) {
            resetConnectionState();
        }

        WiFiClient newClient = server.accept();

        if (newClient) {

            client = newClient;
            connected = true;
            subscribed = false;

            Serial.println("ESPHome client connected");
        }

        return;
    }

    handleClient();
}


void ESPHomeAPI::handleClient()
{
    while (client.available()) {

        uint8_t messageType;
        uint8_t buffer[512];

        uint16_t length = 0;

        if (!readFrame(messageType, buffer, length)) {
            resetConnectionState();
            return;
        }

        processMessage(
            messageType,
            buffer,
            length
        );
    }
}


void ESPHomeAPI::resetConnectionState()
{
    subscribed = false;
    connected = false;
    lightState = false;
    brightness = 1.0f;
    red = 1.0f;
    green = 1.0f;
    blue = 1.0f;

    //clearPixel();

    if (client) {
        client.stop();
    }

    client = WiFiClient();
}


bool ESPHomeAPI::readFrame(
    uint8_t &messageType,
    uint8_t *data,
    uint16_t &length
)
{
    auto readByte = [&]() -> int {
        unsigned long startedAt = millis();

        while (client.connected() && !client.available()) {
            if (millis() - startedAt > 2000UL) {
                return -1;
            }
            delay(1);
        }

        if (!client.available()) {
            return -1;
        }

        return client.read();
    };

    auto readVarint = [&](uint32_t &value) -> bool {
        value = 0;
        uint8_t shift = 0;

        while (shift < 35) {
            int byte = readByte();
            if (byte < 0) {
                return false;
            }

            value |= (uint32_t)(byte & 0x7F) << shift;
            if ((byte & 0x80) == 0) {
                return true;
            }

            shift += 7;
        }

        return false;
    };

    int frameStart = readByte();
    if (frameStart != 0x00) {
        return false;
    }

    uint32_t messageLength = 0;
    if (!readVarint(messageLength)) {
        return false;
    }

    uint32_t typeValue = 0;
    if (!readVarint(typeValue)) {
        return false;
    }

    if (messageLength > 512) {
        return false;
    }

    length = (uint16_t)messageLength;
    messageType = (uint8_t)typeValue;

    unsigned long startedAt = millis();
    while (client.connected() && client.available() < length) {
        if (millis() - startedAt > 2000UL) {
            return false;
        }
        delay(1);
    }

    if (client.available() < length) {
        return false;
    }

    size_t readCount = client.readBytes((char *)data, length);
    if (readCount != length) {
        return false;
    }

    return true;
}


void ESPHomeAPI::processMessage(
    uint8_t messageType,
    uint8_t *data,
    uint16_t length
)
{
    switch (messageType) {

        case MSG_HELLO_REQUEST:
            handleHello(data, length);
            break;

        case MSG_DEVICE_INFO_REQUEST:
            handleDeviceInfo();
            break;

        case MSG_LIST_ENTITIES_REQUEST:
            handleListEntities();
            break;

        case MSG_SUBSCRIBE_STATES_REQUEST:
            handleSubscribeStates();
            break;

        case MSG_LIGHT_COMMAND:
            handleLightCommand(data, length);
            break;
    }
}


void ESPHomeAPI::sendFrame(
    uint8_t messageType,
    const uint8_t *data,
    uint16_t length
)
{
    uint8_t header[16];
    uint8_t headerLength = 0;

    header[headerLength++] = 0x00;
    headerLength += encodeVarint(header + headerLength, length);
    headerLength += encodeVarint(header + headerLength, messageType);

    client.write(header, headerLength);
    if (length > 0) {
        client.write(data, length);
    }
}


void ESPHomeAPI::handleHello(
    const uint8_t *data,
    uint16_t length
)
{
    (void)data;
    (void)length;

    uint8_t response[32];
    uint16_t responseLength = 0;

    responseLength += encodeVarint(response + responseLength, (1 << 3) | 0);
    responseLength += encodeVarint(response + responseLength, 1);

    responseLength += encodeVarint(response + responseLength, (2 << 3) | 0);
    responseLength += encodeVarint(response + responseLength, 10);

    responseLength += encodeString(response + responseLength, 3, "ESPHome Native API");
    responseLength += encodeString(response + responseLength, 4, "ozkled");

    sendFrame(MSG_HELLO_RESPONSE, response, responseLength);
}


void ESPHomeAPI::handleDeviceInfo()
{
    uint8_t response[64];
    uint16_t responseLength = 0;

    response[responseLength++] = 0;
    response[responseLength++] = 0;

    sendFrame(MSG_DEVICE_INFO_RESPONSE, response, responseLength);
}


void ESPHomeAPI::handleListEntities()
{
    uint8_t response[64];
    uint16_t responseLength = 0;

    response[responseLength++] = 0;
    response[responseLength++] = 0;

    sendFrame(MSG_LIST_ENTITIES_LIGHT, response, responseLength);
    sendFrame(MSG_LIST_ENTITIES_DONE, response, responseLength);
}


void ESPHomeAPI::handleSubscribeStates()
{
    subscribed = true;
    sendLightState();
}


void ESPHomeAPI::handleLightCommand(
    const uint8_t *data,
    uint16_t length
)
{
    bool hasState = false;
    bool stateValue = lightState;
    bool hasBrightness = false;
    float brightnessValue = brightness;
    bool hasColorBrightness = false;
    float colorBrightnessValue = 1.0f;
    bool hasRgb = false;
    float redValue = red;
    float greenValue = green;
    float blueValue = blue;

    uint16_t pos = 0;

    auto readVarint = [&](uint32_t &value) -> bool {
        value = 0;
        uint8_t shift = 0;

        while (pos < length && shift < 35) {
            uint8_t byte = data[pos++];
            value |= (uint32_t)(byte & 0x7F) << shift;
            if ((byte & 0x80) == 0) {
                return true;
            }
            shift += 7;
        }

        return false;
    };

    while (pos < length) {
        uint32_t tagValue = 0;
        if (!readVarint(tagValue)) {
            break;
        }

        uint8_t fieldNumber = (uint8_t)(tagValue >> 3);
        uint8_t wireType = (uint8_t)(tagValue & 0x07);

        switch (wireType) {
            case 0: {
                uint32_t value = 0;
                if (!readVarint(value)) {
                    pos = length;
                    break;
                }

                if (fieldNumber == 3) {
                    hasState = true;
                    stateValue = value != 0;
                }
                break;
            }

            case 5: {
                if (pos + 4 > length) {
                    pos = length;
                    break;
                }

                float value = readFloat(data + pos);
                pos += 4;

                switch (fieldNumber) {
                    case 5:
                        hasBrightness = true;
                        brightnessValue = value;
                        break;
                    case 21:
                        hasColorBrightness = true;
                        colorBrightnessValue = value;
                        break;
                    case 7:
                        hasRgb = true;
                        redValue = value;
                        break;
                    case 8:
                        hasRgb = true;
                        greenValue = value;
                        break;
                    case 9:
                        hasRgb = true;
                        blueValue = value;
                        break;
                    default:
                        break;
                }
                break;
            }

            case 2: {
                uint32_t stringLength = 0;
                if (!readVarint(stringLength)) {
                    pos = length;
                    break;
                }

                pos += (uint16_t)stringLength;
                if (pos > length) {
                    pos = length;
                }
                break;
            }

            default:
                pos = length;
                break;
        }
    }

    if (hasState) {
        lightState = stateValue;
    } else {
        lightState = true;
    }

    if (hasBrightness) {
        brightness = brightnessValue;
    } else if (hasColorBrightness) {
        brightness = colorBrightnessValue;
    }

    if (hasRgb) {
        red = redValue;
        green = greenValue;
        blue = blueValue;
    }

    if (brightness < 0.0f) {
        brightness = 0.0f;
    }
    if (brightness > 1.0f) {
        brightness = 1.0f;
    }

    if (red < 0.0f) red = 0.0f;
    if (red > 1.0f) red = 1.0f;
    if (green < 0.0f) green = 0.0f;
    if (green > 1.0f) green = 1.0f;
    if (blue < 0.0f) blue = 0.0f;
    if (blue > 1.0f) blue = 1.0f;

    if (!lightState) {
        clearPixel();
        sendLightState();
        return;
    }

    uint8_t brightnessLevel = (uint8_t)(brightness * 255.0f);
    uint8_t r = (uint8_t)(red * brightness * 255.0f);
    uint8_t g = (uint8_t)(green * brightness * 255.0f);
    uint8_t b = (uint8_t)(blue * brightness * 255.0f);

    led_setBrightness(brightnessLevel);
    applyColor(r, g, b, -1);

    sendLightState();
}


void ESPHomeAPI::sendLightState()
{
    if (!subscribed) {
        return;
    }

    uint8_t response[32];
    uint16_t responseLength = 0;

    responseLength += encodeVarint(response + responseLength, (1 << 3) | 5);
    writeFixed32(response + responseLength, 1);
    responseLength += 4;

    responseLength += encodeVarint(response + responseLength, (2 << 3) | 0);
    responseLength += encodeVarint(response + responseLength, lightState ? 1 : 0);

    responseLength += encodeVarint(response + responseLength, (3 << 3) | 0);
    responseLength += encodeVarint(response + responseLength, lightState ? 1 : 0);

    responseLength += encodeVarint(response + responseLength, (4 << 3) | 0);
    responseLength += encodeVarint(response + responseLength, 1);

    responseLength += encodeVarint(response + responseLength, (5 << 3) | 5);
    writeFloat(response + responseLength, brightness);
    responseLength += 4;

    responseLength += encodeVarint(response + responseLength, (6 << 3) | 0);
    responseLength += encodeVarint(response + responseLength, 1);

    responseLength += encodeVarint(response + responseLength, (7 << 3) | 5);
    writeFloat(response + responseLength, red);
    responseLength += 4;

    responseLength += encodeVarint(response + responseLength, (8 << 3) | 5);
    writeFloat(response + responseLength, green);
    responseLength += 4;

    responseLength += encodeVarint(response + responseLength, (9 << 3) | 5);
    writeFloat(response + responseLength, blue);
    responseLength += 4;

    sendFrame(MSG_LIGHT_STATE, response, responseLength);
}


uint32_t ESPHomeAPI::readFixed32(
    const uint8_t *data
)
{
    return (uint32_t)data[0]
        | ((uint32_t)data[1] << 8)
        | ((uint32_t)data[2] << 16)
        | ((uint32_t)data[3] << 24);
}


float ESPHomeAPI::readFloat(
    const uint8_t *data
)
{
    union {
        uint32_t u32;
        float f;
    } value;

    value.u32 = readFixed32(data);
    return value.f;
}


void ESPHomeAPI::writeFixed32(
    uint8_t *data,
    uint32_t value
)
{
    data[0] = (uint8_t)(value & 0xFF);
    data[1] = (uint8_t)((value >> 8) & 0xFF);
    data[2] = (uint8_t)((value >> 16) & 0xFF);
    data[3] = (uint8_t)((value >> 24) & 0xFF);
}


void ESPHomeAPI::writeFloat(
    uint8_t *data,
    float value
)
{
    union {
        uint32_t u32;
        float f;
    } raw;

    raw.f = value;
    writeFixed32(data, raw.u32);
}


uint8_t ESPHomeAPI::encodeVarint(
    uint8_t *buffer,
    uint32_t value
)
{
    uint8_t index = 0;

    while (value >= 0x80) {
        buffer[index++] = (uint8_t)((value & 0x7F) | 0x80);
        value >>= 7;
    }

    buffer[index++] = (uint8_t)value;
    return index;
}


uint8_t ESPHomeAPI::encodeString(
    uint8_t *buffer,
    uint8_t field,
    const char *str
)
{
    uint8_t index = 0;
    uint8_t header = (uint8_t)((field << 3) | 2);
    uint16_t textLength = (uint16_t)strlen(str);

    index += encodeVarint(buffer + index, header);
    index += encodeVarint(buffer + index, textLength);

    for (uint16_t i = 0; i < textLength; i++) {
        buffer[index++] = (uint8_t)str[i];
    }

    return index;
}
