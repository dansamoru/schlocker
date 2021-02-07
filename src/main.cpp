#include <Arduino.h>

#include <Settings.h>
#include <Constants.h>
#include <Models.h>

Cell cells[LOCKERS_QUANTITY];
Scanner scanner(PIN_SCANNER, 100);
const unsigned short
        greenButton = PIN_BUTTON_GREEN,
        redButton = PIN_BUTTON_RED;

void cellsSetup() {
    DEBUG_main("#Pin setup\n");
    // Set up lockers
    DEBUG_detail("Set up lockers:\n");
    for (short i = 0; i < LOCKERS_QUANTITY; ++i) {
        cells[i].id = i;
        cells[i].locker_pin = PIN_LOCKER_START + i;
        cells[i].sensor_pin = PIN_SENSOR_START + i;

        pinMode(cells[i].locker_pin, OUTPUT);
        pinMode(cells[i].sensor_pin, INPUT);


        DEBUG_detail("   locker[");
        DEBUG_detail(i);
        DEBUG_detail("]: id=");
        DEBUG_detail(cells[i].id);
        DEBUG_detail(" locker_pin=");
        DEBUG_detail(cells[i].locker_pin);
        DEBUG_detail(" sensor_pin=");
        DEBUG_detail(cells[i].sensor_pin);
        DEBUG_detail("\n");
    }
}

void scannerSetup(){
    scanner.begin();
    if (!scanner.getFirmwareVersion()){
        DEBUG_error("!>Scanner failed");
    }
    scanner.SAMConfig();
}

void setup() {
    // Turn on Serial for debugging
#if DEBUG != 0
    Serial.begin(9600);
#endif
    DEBUG_main("#Serial connected\n")

    cellsSetup();
    scannerSetup();
}

void loop() {
    delay(DELAY_TIME);
}