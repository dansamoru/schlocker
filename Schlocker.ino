/*
 *
 * PROJECT SCHLOCKER!
 *
 * Developer: Samoilov Daniil © 2020
 * VK: @dansamoru
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

//  ==DEFINES==
//  =PROJECT=
#define CELL_QUANTITY 4
#define USERID_LENGTH 4  //  default — 4, secure — 7
//  =PINS=
#define LOCKER_START_PIN 10
#define SENSOR_START_PIN 10
#define SCANNER_PIN 10
//  =CIRCUIT=
#define LOCKERSENSOR_MODE false  //  0 — l+s, 1 — l/s
//  =HARDWARE=
#define LOOP_DELAY 10
#define SERIAL_SPEED 9600
//  =VARIABLES=
Adafruit_PN532 scanner(SCANNER_PIN, 100);
ushort
    lockers[CELL_QUANTITY],
    sensors[CELL_QUANTITY];
const ushort
    greenBtn 10,
    redBtn 10,
    greenLed 10,
    yellowLed 10,
    redLed 10;


//  ===CODE===
void cellSetup(){
#if LOCKERSENSOR_MODE == 0
    for (int i = 0; i < CELL_QUANTITY; i++) {
        lockers[i] = LOCKER_START_PIN + i;
        pinMode(lockers[i], OUTPUT);
        sensors[i] = SENSOR_START_PIN + i;
        pinMode(sensors[i], INPUT);
    }
#elif LOCKERSENSOR_MODE == 1
    for (int i = 0; i < CELL_QUANTITY; i++){
        lockers[i] = LOCKER_START_PIN + (i*2);
        pinMode(lockers[i], OUTPUT);
        sensors[i] = SENSOR_START_PIN + (i*2);
        pinMode(sensors[i], INPUT);
    }
#endif
}

void pinsMode(){
    cellSetup();
    pinMode(scanner, INPUT);
    pinMode(greenBtn, INPUT_PULLUP);
    pinMode(redBtn, INPUT_PULLUP);
    pinMode(greenLed, OUTPUT);
    pinMode(yellowLed, OUTPUT);
    pinMode(redLed, OUTPUT);
}

byte[USERID_LENGTH] getUserId(){
    uint8_t card{};
    uint8_t uid[USERID_LENGTH]{};
    uint8_t uidLength{};
    if (!scanner.getFirmwareVersion()) {
        return;
    }
    scanner.SAMConfig();
}

void update(){
    if(digitalRead(greenBtn) == LOW){

    } else
    if(digitalRead(redBtn) == LOW){
        byte[] userId = getUserId();
    }
}


void setup(){
    Serial.begin(SERIAL_SPEED);
    scanner.begin();
    pinsMode();
}

void loop(){
    update();
    delay(LOOP_DELAY);
}
