#pragma once
#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
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
#define SCANNER_WAIT_TIME 1000
#define LOCKER_WAIT_TIME 1000
#define CELL_DONT_OPEN_TIME 604800000
//  =STRUCT TYPE=
struct cell_data{
  long userId;
  unsigned long lastOpenTime;
};
//  =VARIABLES=
const unsigned short cell_data_size = sizeof(cell_data);
Adafruit_PN532 scanner(SCANNER_PIN, 100);
unsigned short
    lockers[CELL_QUANTITY],
    sensors[CELL_QUANTITY];
const unsigned short
    greenBtn = 10,
    redBtn = 10,
    greenLed = 10,
    yellowLed = 10,
    redLed = 10;
uint8_t uid[USERID_LENGTH];
uint8_t uidLength{};
//  ===INITIALIZATION===
void pinModes();
void scannerSetup();
void cellSetup();
void memorySetup();
void update();
bool isReadable();
unsigned long scan();
void indicate(unsigned short status);
unsigned short update_status();
unsigned short findCellNumber(unsigned long userId);
void unregUser(unsigned short cell_number);
unsigned short regUser(unsigned long userId);
void rewriteLastOpenTime(unsigned short cell_number);
void open_cell(unsigned short cell_number);

//  ===REALIZATION===
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

void pinModes(){
    pinMode(greenBtn, INPUT_PULLUP);
    pinMode(redBtn, INPUT_PULLUP);
    pinMode(greenLed, OUTPUT);
    pinMode(yellowLed, OUTPUT);
    pinMode(redLed, OUTPUT);
}

void scannerSetup(){
    scanner.begin();
    if (!scanner.getFirmwareVersion()) {
        Serial.println("Scaner didn't found");
    }
    scanner.SAMConfig();
}

void memorySetup(){
    if(EEPROM.read(0) != CELL_QUANTITY){
        EEPROM.write(0, CELL_QUANTITY);
        for(unsigned short i = 1; i < CELL_QUANTITY * cell_data_size + 1; i++){
            EEPROM.update(i, 255);
        }
    }
}

bool isReadable(){
    return scanner.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
}

unsigned long scan(){
    unsigned long startTime = millis();  //  Variable for timer
    delay(1);
    while (millis() - startTime <= SCANNER_WAIT_TIME) {
        if (isReadable()) {
            return *(unsigned long *) uid;
        }
    }
}

unsigned short findCellNumber(unsigned long userId){
    cell_data buffer;
    for(unsigned short i = 0; i < CELL_QUANTITY; i++){
       EEPROM.get(i * cell_data_size + 1, buffer);
       if(buffer.userId == userId){
          return i;
       }
    }
    return CELL_QUANTITY;
}

unsigned short regUser(unsigned long userId){
    cell_data buffer;
    for(unsigned short i = 0; i < CELL_QUANTITY; i++){
       EEPROM.get(i * cell_data_size + 1, buffer);
       if(buffer.userId == 4294967295){
          buffer.userId = userId;
          EEPROM.put(i * cell_data_size + 1, buffer);
          return i;
       }
    }
}

void unregUser(unsigned short cell_number){
    cell_data buffer;
    EEPROM.get(cell_number * cell_data_size + 1, buffer);
    buffer.userId = 4294967295;
    EEPROM.put(cell_number * cell_data_size + 1, buffer);
}

void rewriteLastOpenTime(unsigned short cell_number){
    cell_data buffer;
    EEPROM.get(cell_number * cell_data_size + 1, buffer);
    buffer.lastOpenTime = millis();
    EEPROM.put(cell_number * cell_data_size + 1, buffer);
}

void indicate(unsigned short status){
    switch(status){
        case 0:
            digitalWrite(greenLed, HIGH);
            digitalWrite(redLed, LOW);
            digitalWrite(yellowLed, LOW);
            break;
        case 1:
            digitalWrite(redLed, LOW);
            digitalWrite(greenLed, HIGH);
            digitalWrite(yellowLed, HIGH);
            break;
        case 2:
            digitalWrite(yellowLed, LOW);
            digitalWrite(greenLed, LOW);
            digitalWrite(redLed, HIGH);
            break;
        case 3:
            digitalWrite(yellowLed, LOW);
            digitalWrite(greenLed, LOW);
            digitalWrite(redLed, HIGH);
            break;
    }
}

unsigned short update_status(){
    bool isGreen = false;
    bool isYellow = false;
    cell_data buffer;
    for(unsigned short i = 0; i < CELL_QUANTITY; i++){
        EEPROM.get(i * cell_data_size + 1, buffer);
        if(millis() - buffer.lastOpenTime > CELL_DONT_OPEN_TIME){
            isYellow = true;
        }
        if(buffer.userId == 4294967295){
            isGreen = true;
        }
        if(isGreen && isYellow){
            break;
        }
    }
    if(isGreen && isYellow){
        return 1;
    }
    else if(isGreen && !isYellow){
        return 0;
    }
    else if(!isGreen && isYellow){
        return 2;
    }
    else if(!isGreen && !isYellow){
        return 3;
    }
}


void open_cell(unsigned short cell_number){
    digitalWrite(lockers[cell_number], HIGH);
    delay(LOCKER_WAIT_TIME);
    digitalWrite(lockers[cell_number], LOW);
}

void update(){
    unsigned short status = update_status();
    unsigned short cell_number;
    indicate(status);
    unsigned long userId;
    if(digitalRead(greenBtn) == LOW && digitalRead(redBtn) == HIGH){

        if(isReadable()){
            userId = scan();
            cell_number = findCellNumber(userId);
            if(cell_number == CELL_QUANTITY && status < 2){
                cell_number = regUser(userId);
            }
            if(cell_number != CELL_QUANTITY){
                open_cell(cell_number);
                rewriteLastOpenTime(cell_number);
            }
        }
    }
    else if(digitalRead(redBtn) == LOW && digitalRead(greenBtn) == HIGH){
        if(isReadable){
        userId = scan();
        cell_number = findCellNumber(userId);
        if(cell_number != CELL_QUANTITY){
            unregUser(cell_number);
            open_cell(cell_number);
        }
        }
    }
}
