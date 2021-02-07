//
// Created by dansamoru on 06.02.2021.
//

#ifndef SCHLOCKER_MODELS_H
#define SCHLOCKER_MODELS_H

#include <Arduino.h>

#include <Adafruit_PN532.h>
#include <Constants.h>

struct Cell {
    unsigned short id = 0;
    unsigned short locker_pin = 0;
    unsigned short sensor_pin = 0;
    byte card_uid[CARD_NUMBER_LENGTH] = {};
};

class Scanner : public Adafruit_PN532 {
public:
    Scanner(uint8_t irq, uint8_t reset):Adafruit_PN532(irq, reset){}
};

#endif //SCHLOCKER_MODELS_H
