#include "scanner.h"

Scanner(pin){
    rfid = Adafruit_PN532(pin, 100);
    if (!rfid.getFirmwareVersion()) {
        Serial.print("Scanner didn't found");
    }
        scanner.SAMConfig();
}

bool Scanner::isReadable(){
    return rfid.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
}

unsigned long Scanner::read(){
    return *(unsigned long *) uid;
}

unsigned long Scanner::scan(){
      unsigned long startTime = millis();  //  Variable for timer
      delay(1);
      while (millis() - startTime <= SCANNER_WAIT_TIME) {
          if (isAvailable()) {
              unsigned long uid = read();
              return uid;
          }
      }
      return 0;
}
