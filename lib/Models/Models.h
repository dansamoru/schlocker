//
// Created by dansamoru on 06.02.2021.
//

#ifndef SCHLOCKER_MODELS_H
#define SCHLOCKER_MODELS_H

#include <Arduino.h>
#include "Constants.h"

struct Cell {
    unsigned short id = 0;
    unsigned short locker_pin = 0;
    unsigned short sensor_pin = 0;
    byte card_uid[CARD_NUMBER_LENGTH] = {};
};


#endif //SCHLOCKER_MODELS_H
