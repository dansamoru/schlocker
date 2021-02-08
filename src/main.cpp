#include <Arduino.h>

#include <Settings.h>
#include <Constants.h>
#include <Models.h>

Cell cells[LOCKERS_QUANTITY];
Scanner scanner(PIN_SCANNER, 100);
const unsigned short
        greenButton = PIN_BUTTON_GREEN,
        redButton = PIN_BUTTON_RED;

void pinsSetup() {
    pinMode(greenButton, INPUT_PULLUP);
    pinMode(redButton, INPUT_PULLUP);
}

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

void scannerSetup() {
#if INPUT_DEBUG == 0
    scanner.begin();
    if (!scanner.getFirmwareVersion()) {
        DEBUG_error("!>Scanner failed");
    }
    scanner.SAMConfig();
#endif
}

ScannerAnswer get_scannerAnswer() {
    ScannerAnswer scannerAnswer;
    uint8_t uid[CARD_NUMBER_LENGTH];
#if INPUT_DEBUG == 0
    Debug_detail("Start scanning\n");
    scannerAnswer.success = scanner.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &scannerAnswer.uidLength);
#elif INPUT_DEBUG >= 1
    DEBUG_main("#Enter uid\n");
    DEBUG_detail("Length is: ");
    DEBUG_detail(CARD_NUMBER_LENGTH);
    DEBUG_detail("\n");
    for (int i = 0; i < CARD_NUMBER_LENGTH; ++i) {
        DEBUG_detail("    uid[");
        DEBUG_detail(i);
        DEBUG_detail("]?\n");
        while (!Serial.available()) { delay(INPUT_DELAY); }
        uid[i] = Serial.read() - '0';
    }
    scannerAnswer.success = true;
    scannerAnswer.uidLength = CARD_NUMBER_LENGTH;
#endif
    if (scannerAnswer.success) {
        if (scannerAnswer.uidLength == CARD_NUMBER_LENGTH) {
            for (int i = 0; i < scannerAnswer.uidLength; ++i) {
                scannerAnswer.uid[i] = uid[i];
            }
        } else {
            scannerAnswer.success = false;
        }
    }
    return scannerAnswer;
}

void openLocker(uint8_t pin) {
    DEBUG_detail("Locker open, pin=");
    DEBUG_detail(pin);
    DEBUG_detail("\n");
    digitalWrite(pin, HIGH);
    delay(LOCKER_DELAY);
    digitalWrite(pin, LOW);
}

bool userRegister(const uint8_t *uid) {
    for (auto &cell : cells) {
        if (cell.is_vacant) {
            DEBUG_main("#User registration\n")
            cell.is_vacant = false;
            for (int j = 0; j < CARD_NUMBER_LENGTH; ++j) {
                cell.card_uid[j] = uid[j];
                DEBUG_detail("   ");
                DEBUG_detail(uid[j]);
                DEBUG_detail("\n");
            }
            openLocker(cell.locker_pin);
            return true;
        }
    }
    return false;
}

void openCell(uint8_t *uid, bool unregister = false) {
    for (int i = 0; i < LOCKERS_QUANTITY; ++i) {
        if (!cells[i].is_vacant) {
            bool currentRegistration = true;
            for (int j = 0; j < CARD_NUMBER_LENGTH; ++j) {
                if (uid[j] != cells[i].card_uid[j]) {
                    currentRegistration = false;
                    break;
                }
            }
            if (currentRegistration) {
                openLocker(cells[i].locker_pin);
                if (unregister) {
                    DEBUG_main("#User deleted\n");
                    DEBUG_detail("    Locker:\n");
                    DEBUG_detail("        locker_pin=");
                    DEBUG_detail(cells[i].locker_pin);
                    DEBUG_detail("\n        id=");
                    DEBUG_detail(cells[i].id);
                    DEBUG_detail("\n    User uid:\n");
                    for (int j = 0; j < CARD_NUMBER_LENGTH; ++j) {
                        DEBUG_detail("        ");
                        DEBUG_detail(cells[i].card_uid[j]);
                        DEBUG_detail("\n");
                    }
                    cells[i].is_vacant = true;
                }
                return;
            }
        }
    }
    if (userRegister(uid)) {
        DEBUG_main("#New user registered\n")
    } else {
        DEBUG_error("!>User registration failed\n");
    }
}

State getCurrentState() {
    State currentState;
#if INPUT_DEBUG == 2
    DEBUG_main("#Input state\n");
    DEBUG_main("    greenButton? (0 - pressed, 1)\n");
    while (!Serial.available()) { delay(INPUT_DELAY); }
    currentState.greenButton = (Serial.read() == '0' ? LOW : HIGH);
    DEBUG_main("    redButton? (0 - pressed, 1)\n");
    while (!Serial.available()) { delay(INPUT_DELAY); }
    currentState.redButton = (Serial.read() == '0' ? LOW : HIGH);
#else
    currentState.greenButton = digitalRead(greenButton);
    currentState.redButton = digitalRead(redButton);
#endif
    return currentState;
}

void update() {
    State currentState = getCurrentState();
    if (currentState.greenButton == LOW && currentState.redButton == HIGH) {
        DEBUG_detail("Only greenButton pressed\n");
        ScannerAnswer scannerAnswer = get_scannerAnswer();
        if (scannerAnswer.success) {
            DEBUG_detail("Got success scannerAnswer\n");
            openCell(scannerAnswer.uid);
        }
    } else if (currentState.greenButton == HIGH && currentState.redButton == LOW) {
        DEBUG_detail("Only redButton pressed\n");
        ScannerAnswer scannerAnswer = get_scannerAnswer();
        if (scannerAnswer.success) {
            DEBUG_detail("Got success scannerAnswer\n");
            openCell(scannerAnswer.uid, true);
        }
    }
}

void setup() {
    // Turn on Serial for debugging
#if DEBUG != 0 || INPUT_DEBUG != 0
    Serial.begin(9600);
#endif
    DEBUG_main("#Serial connected\n")

    pinsSetup();
    cellsSetup();
    scannerSetup();
}

void loop() {
    update();
    delay(DELAY_TIME);
}