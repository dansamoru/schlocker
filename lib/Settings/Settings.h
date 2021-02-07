//
// Created by dansamoru on 07.02.2021.
//

#ifndef SCHLOCKER_SETTINGS_H
#define SCHLOCKER_SETTINGS_H

/*
 * Set up a level of debugging
 *
 * Debug levels:
 *     0 - Debug off
 *     1 - Print main events [main]
 *     2 - Print everything [detail]
 *
 */
#define DEBUG 2

//  PROJECT CONFIGURATIONS
#define LOCKERS_QUANTITY 4U

// Pins
#define PIN_LOCKER_START 1U
#define PIN_SENSOR_START 6U


// === END OF SETTINGS===

// System debug configuration
#if DEBUG == 2
#define DEBUG_detail(val) Serial.print(val);
#define DEBUG_main(val) Serial.print(val);
#elif DEBUG == 1
#define DEBUG_detail(val)
#define DEBUG_main(val) Serial.print(val);
#else
#define DEBUG_detail(val)
#define DEBUG_main(val)
#endif

#endif //SCHLOCKER_SETTINGS_H
