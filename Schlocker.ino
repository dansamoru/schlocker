/*
 * PROJECT: SCHLOCKER!
 * Developers: Dneprov Stepan, Samoilov Daniil
 *             vk: @pegaster   vk: @dansamoru
*/

#include "src/schlockerFunc.h"

void setup(){
    Serial.begin(SERIAL_BAUDRATE);
    pinModes();
    scannerSetup();
    cellSetup();
    memorySetup();
}

void loop(){
    update();
    delay(LOOP_DELAY);
}
