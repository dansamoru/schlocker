//
// Created by dansamoru on 06.02.2021.
//

#ifndef SCHLOCKER_MODELS_H
#define SCHLOCKER_MODELS_H

#include <Arduino.h>
#include "Constants.h"

struct Locker{
    short id;
    byte card_uid[CARD_NUMBER_LENGTH];
};


#endif //SCHLOCKER_MODELS_H
