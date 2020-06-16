/*
 * PROJECT: SCHLOCKER!
*/
#include "src/schlockerFunc.h"

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
