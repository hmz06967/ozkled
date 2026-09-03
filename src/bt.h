#pragma once

#include <Arduino.h>

void bt_setup();
void bt_loop();
void parseCommand(String cmd);
void parseCommand2(String cmd);
void loadWifiCredentials();
void saveWifiCredentials();