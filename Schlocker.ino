/*
 *
 * PROJECT SCHLOCKER!
 *
 * Developer: Dneprov Stepan © 2020
 * VK: @pegaster
 *
*/
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
