#include <Arduino.h>
#include "User_Setup.h"

#define SECTIONQUANTITY 6

class Locker {
public:
    String userId = NULL;
    bool isUsed = false;

    bool isOpened() {
        return 0;
    }

    void openDoor() {
        return;
        0;
    }

    void deleteUser() {

    }
};



Locker lockers[SECTIONQUANTITY];
short lastLocker;
//String cardId = NULL;

bool buttonNumber() {
    /*
     * Getting number of pressed button
     * true -just open
     * false - open and delete
     *
     * DEBUG
    */
    Serial.print("Enter bvutton (y/n): ");
    while (Serial.available() == 0);
    char s = Serial.read();
    if (s == 'y') {
        return true;
    } else {
        return false;
    }
}

void registration(String userId) {

}

void openLocker(int lockerId) {
    if (buttonNumber()) {
        lockers[lockerId].openDoor();
    } else {
        lockers[lockerId].openDoor();
        lockers[lockerId].deleteUser();
    }
}

short cardRegistration(String cardId) {
    for (int i = 0; i < SECTIONQUANTITY; i++) {
        if (lockers[i].userId == cardId) {
            return i;
        }
    }
    return NULL;
}

String getCardId() {
    /*
     * Card identification
     * DEBUG
     */
    Serial.print("Enter card id: ");
    while (Serial.available() == 0);
    return Serial.readStringUntil(' ');
}

void cardInput() {
    /*
     * Acts after applying the card
     */
    String cardId = getCardId();
    short lockerId = cardRegistration(cardId);
    if (lockerId != NULL) {
        openLocker(lockerId);
    } else {
        registration(cardId);
    }
}
// ьоь тевирп проверка связи

bool isCardAvailable() {
    /*
     * Checking for availability of the card
     * DEBUG
     */
    Serial.print("Enter card availability (y/n): ");
    while (Serial.available() == 0);
    char s = Serial.read();
    if (s == 'y') {
        return true;
    } else {
        return false;
    }
}

void waitingForCard() {
    /*
     * Waiting while card is not near the scanner
     */
    while (!isCardAvailable());
}

int main() {
    waitingForCard();
    cardInput();
    return 0;
}

void setup() {

}

void loop() {
    main();
}
