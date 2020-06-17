#pragma once


//  ===SETTTINGS===

//  ==INCLUDES==
#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include <SPI.h>
//  =LIBS=
#include <src/libs/scanner.h>

//  ==DEFINES==
//  =PROJECT=
#define CELL_QUANTITY 4
//#define USERID_LENGTH 4  //  default — 4, secure — 7
//  =PINS=
#define LOCKER_START_PIN 10
#define SENSOR_START_PIN 10
#define SCANNER_PIN 10
#define BUZZER_PIN 10
//  =CIRCUIT=
#define LOCKERSENSOR_CONFIG false  //  0 — l+s, 1 — l/s
//  =HARDWARE=
#define SOUND 400
#define PLAY_TIME 50
#define LOCKER_DEFAULT true  //  Value when closed
#define LOOP_DELAY 10
#define SERIAL_BAUDRATE 9600  //  Serial speed
#define SCANNER_WAITTIME 1000
//  =TIMES=
#define LOCKER_WAITTIME 1000
#define CELL_DAWNTIME 604800000
#define CELL_OPEN_TIME_LIMIT 120000

//  ==STRUCTS==
struct cell_data{
  unsigned long userId;
  unsigned long lastOpenTime;
};


//  ===HEADERS===
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
unsigned short regUser(unsigned long userId);
void unregUser(unsigned short cell_number);
void updateLastOpenTime(unsigned short cell_number);
void openCell(unsigned short cell_number);
bool checkOpenedCell();
void play();
