#pragma once  //  Хз что это, но оно нужно


//  ===SETTTINGS===
//  ==INCLUDES==
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
#define BUZZER_PIN 10
//  =CIRCUIT=
#define LOCKERSENSOR_CONF false  //  0 — l+s, 1 — l/s
//  =HARDWARE=
#define CELL_OPEN_TIME_LIMIT 120000
#define SOUND 400
#define PLAY_TIME 50
#define LOCKERSENSOR_MODE true
#define LOOP_DELAY 10
#define SERIAL_SPEED 9600
#define SCANNER_WAIT_TIME 1000
#define LOCKER_WAIT_TIME 1000
#define CELL_DONT_OPEN_TIME 604800000
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
void unregUser(unsigned short cell_number);
unsigned short regUser(unsigned long userId);
void rewriteLastOpenTime(unsigned short cell_number);
void open_cell(unsigned short cell_number);
bool checkOpenedCell();
void play();
