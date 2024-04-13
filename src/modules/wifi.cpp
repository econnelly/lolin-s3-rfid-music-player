//
// Created by eli on 4/2/2024.
//

#include "modules/wifi.h"

void init_wifi() {
  WiFiManager wm;
//  wm.resetSettings();

  // res = wm.autoConnect(); // auto generated AP name from chipid
  // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  bool res = wm.autoConnect("ESP32 MusicBox","songbird"); // password protected ap

  if(!res) {
    Serial.println("Failed to connect");
    while(true) {}
    // ESP.restart();
  }
}

