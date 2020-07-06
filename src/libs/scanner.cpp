#include "scanner.h"
#include <Adafruit_PN532.h>
Scanner::Scanner(unsigned int pin, unsigned long wait_time){
    _wait_time = wait_time;
    rfid = Adafruit_PN532(pin, 100);
    rfid.begin();
    if (!rfid.getFirmwareVersion()) {
        Serial.print("Scanner didn't found");
    }
    rfid.SAMConfig();
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
      while (millis() - startTime <= _wait_time) {
      if (isReadable()) {
          unsigned long uid = read();
          return uid;
          }
      }
      return 0;
}
