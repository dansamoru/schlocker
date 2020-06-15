/*
 *
 * PROJECT SCHLOCKER!
 *
 * Developer: Dneprov Stepan © 2020
 * VK: @pegaster
 *
*/


//  ===SETTINGS===

//  ==INCLUDES==
//  =SYSTEM LIBRARIES=
#include <EEPROM.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
//  =USER LIBRARIES=
#include "src/schlockerFunc.h"

//  ===USE===
void setup(){
    Serial.begin(SERIAL_SPEED);
    pinModes();
    scannerSetup();
    cellSetup();
    memorySetup();
}

void loop(){
    update();
    delay(LOOP_DELAY);
}
