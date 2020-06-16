#pragma once
#include <Arduino.h>
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
  unsigned long userId;
  unsigned long lastOpenTime;
};
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
