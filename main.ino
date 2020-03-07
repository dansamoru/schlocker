/*
 * PROJECT SCHLOCKER
 * v.0.1.10
 *
 * Developer: Samoilov Daniil © 2020
 * VK: @dansamoru
 */



//  ===INCLUDE SETTINGS===

//  For Arduino IDE
#include <Wire.h>
#include <SPI.h>
#include <Servo.h>
#include <Adafruit_PN532.h>

//  For CLion
//#include <Wire/Wire.h>
//#include <SPI/SPI.h>
//#include <Servo.h>


//  ===PROJECT SETTINGS===
#define CELL_QUANTITY 4  //  Quantity of cells
#define DEFAULT_POSITION 0 // 0 - CLOSE, 1 - OPEN

//  ===CIRCUIT SETTINGS===
#define CELL_START_PIN 3  //  First pin for cells
#define SCANNER_PIN 2  //  Pin for RFID-scanner
#define OPEN_BUTTON_PIN 7  //  Pin for "Green" button
#define DELETE_BUTTON_PIN 8  //  Pin for "Red" button
#define CLOSE_BUTTON_PIN 9
#define GREEN_LED_PIN 13
#define YELLOW_LED_PIN 10
#define RED_LED_PIN 11
#define BUZZER_PIN A0

//  ===HARDWARE SETTINGS===
#define DEBUG true  //  Switch on debug (boolean)
#define OPEN_ANGLE 90  //  Cell opening angle
#define CLOSE_ANGLE 0  //  Cell closing angle
#define SCANNER_WAIT_TIME 1000  //  Total delay for scanning
#define LOOP_DELAY 100  //  Delay in "loop"
#define SERIAL_SPEED 9600
#define BUZZER_TON_1 4200
#define BUZZER_TON_2 8400


//  ===CODE===

//  Class for debugging messages
class Debugger {
    bool lastEnds = true;  //  Had last flag "end"
public:

    //  Free RAM on the board
#if DEBUG

    int freeRam() {
        extern int __heap_start, *__brkval;
        int v;
        return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    }

#endif

    //  First message
    void loaded() {
#if DEBUG
        if (DEBUG) {
            Serial.println("###LOADED###");
            Serial.println("#TECHNICAL INFORMATION#");
            Serial.print("Free RAM: ");
            Serial.println(freeRam());
            Serial.println("###END###\n");
        }
#endif
    }

    //  Acr logs
    void act(String object, String act, int number = -1, bool ending = true) {
#if DEBUG
        if (lastEnds) {
            Serial.println("#ACT#");
            Serial.print("Object: ");
            Serial.print(object);
            if (number != -1) {
                Serial.print(" (");
                Serial.print(number);
                Serial.print(")");
            }
            Serial.print("\n");
        }
        Serial.print("Act: ");
        Serial.println(act);
        lastEnds = false;
        if (ending) {
            lastEnds = true;
            Serial.println("#END#\n");
        }
#endif
    }

    //  Error logs
    void error(String object, String error, int number = -1) {
#if DEBUG
        Serial.println("#ERROR#");
        Serial.print("Object: ");
        Serial.print(object);
        if (number != -1) {
            Serial.print(" (");
            Serial.print(number);
            Serial.print(")");
        }
        Serial.print("\n");
        Serial.print("Error: ");
        Serial.println(error);
        Serial.println("#END#\n");
#endif
    }
};

Debugger debugger;

//  Basic class for all components
class Port {
protected:
    short number;

    Port(short _number) {
        number = _number;

    }
};

//  Class for ports with "INPUT" mode
class PortIn : protected Port {
protected:
    PortIn(short _number) : Port(_number) {
        pinMode(number, INPUT);
    }
};

//  Class for ports with "OUTPUT" mode
class PortOut : protected Port {
protected:
    PortOut(short _number) : Port(_number) {
        pinMode(number, OUTPUT);
    }

    void set(bool value) {
        digitalWrite(number, value);
    }
};

//  Class for ports with "INPUT_PULLUP" mode
class PortButton : protected Port {
protected:
    PortButton(short _number) : Port(_number) {
        pinMode(number, INPUT_PULLUP);
        digitalWrite(number, HIGH);
    }

    bool read() {
        return digitalRead(number);
    }
};

//  Class for ports with servos
class PortServo : protected Port {
    Servo *servo;
public:
    PortServo(short _number) : Port(_number) {

        servo = new Servo();
        servo->attach(number);
        servo->write(DEFAULT_POSITION==0?CLOSE_ANGLE:OPEN_ANGLE);

    }

    //  Read current position of servo
    short read() {
        return servo->read();
    }

    //  Write new position for servo
    void write(short angle) {
        servo->write(angle);
    }
};

// Class for ports with RFID-scanner
class PortScanner : protected Port {
    Adafruit_PN532 scanner = Adafruit_PN532(number, 100);
    uint8_t card;
    uint8_t uid[8];
    uint8_t uidLength;
protected:
    PortScanner(short _number) : Port(_number) {
        scanner.begin();
        if (!scanner.getFirmwareVersion()) {
            debugger.error("Scanner", "Didn't find");
        }
        scanner.SAMConfig();
    }

    bool isReadable() {
        card = scanner.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
        return card;
    }

    unsigned long get_uid() {
        return *(unsigned long *) uid;
    }
};

//  Class for user with identity key
class User {
    unsigned long uid=0;
public:
    unsigned long get_uid(){
        return uid;
    }
    void set_uid(unsigned long _uid){
        uid=_uid;
    }
};


class Buzzer :protected PortOut{
  public:
  Buzzer(unsigned short _number): PortOut(_number){}
  play(unsigned short mode){
    debugger.act("Buzzer", "Play");
    switch(mode){
      case 0:
      tone(BUZZER_TON_1,number, 500);
      break;
    }
  }
};

Buzzer buzzer =Buzzer(BUZZER_PIN);

//  Class for buttons
class Button : protected PortButton {
public:
    Button(unsigned short _number) : PortButton(_number) {}

    //  Is button pressed in this moment
    bool isPushed() {
        return !read();
    }
};

//  Class for indication bulbs
class Bulb : protected PortOut {
public:
    Bulb(short _number) : PortOut(_number){}

    void turnOn() {
        set(HIGH);

    }

    void turnOff() {
        set(LOW);
    }
};

//  Component of section
class Cell : protected PortServo {
    unsigned short identity;  //  Number of cell
    int lastOpenTime=0;  //  Time when cell was opened last time
    int registrationTime=0;  //  Time when cell was registered
    bool isOpen=false;
    User user;
public:
    Cell(short _number) : PortServo(_number + CELL_START_PIN) {
        identity = number - CELL_START_PIN;
    }

    //  Get unique user id
    unsigned long userId() {
        return user.get_uid();
    }

    //  Close cell
    void close() {
        debugger.act("Cell", "Close", identity);
        isOpen = false;
        write(CLOSE_ANGLE);
    }

    //  Open cell
    void open() {
        debugger.act("Cell", "Open", identity);
        isOpen = true;
        write(OPEN_ANGLE);
    }

    //  Unregister recorded user
    void unreg(bool doOpen = true) {
        debugger.act("Cell", "Unregistration", identity, false);
        user.set_uid(0);
        if (doOpen) {
            open();
        }
    }

    //  Register new user for cell
    void reg(unsigned long userIdentity, bool doOpen = true) {
        debugger.act("Cell", "Registration", identity, false);
        user.set_uid(userIdentity);
        if (doOpen) {
            open();
        }
    }

    bool get_isOpen(){
      return isOpen;
    }
};

//  Class for RFID-scanner
class Scanner : protected PortScanner {
public:
    Scanner() : PortScanner(SCANNER_PIN) {}

    bool isAvailable() {
        return isReadable();
    }

    //  Read current value
    unsigned long read() {
        return get_uid();
    }

    //  The cycle of scanning
    unsigned long scan() {
      buzzer.play(0);
        debugger.act("Scanner", "Start scanning", -1, false);
        unsigned long startTime = millis();  //  Variable for timer
        delay(1);
        while (millis() - startTime <= SCANNER_WAIT_TIME) {
            if (isAvailable()) {
                unsigned long uid = read();
                debugger.act("Scanner", "End scanning");
                return uid;
            }
            //delay(LOOP_DELAY / 10);
        }
        debugger.act("Scanner", "End scanning (timeout)");
        return 0;
    }

};

//  Class for all led indication
class Indication {
    Bulb green = Bulb(GREEN_LED_PIN);
    Bulb yellow = Bulb(YELLOW_LED_PIN);
    Bulb red = Bulb(RED_LED_PIN);
    short mode = 0;
public:

    //  Set indication mode
    void set_mode(short value) {
        mode = value;
    }

    void refresh() {
        switch (mode) {
            case 0:
                green.turnOn();
                yellow.turnOff();
                red.turnOff();
                break;
            case 1:
                green.turnOff();
                yellow.turnOn();
                red.turnOff();
                break;
            case 2:
                green.turnOff();
                yellow.turnOff();
                red.turnOn();
                break;
            case 3:
                green.turnOn();
                yellow.turnOn();
                red.turnOff();
                break;
            case 4:
                green.turnOff();
                yellow.turnOn();
                red.turnOn();
                break;
        }
    }
};

//  Main class for section
class Schlocker {
    Cell *cells[CELL_QUANTITY];
    Scanner scanner;
    Indication indication;
    Button greenButton = Button(OPEN_BUTTON_PIN);
    Button redButton = Button(DELETE_BUTTON_PIN);
    Button closeButton = Button(CLOSE_BUTTON_PIN);
    unsigned short stat =0;  //  Section status: 0 - green, 1 - yellow, 2 - red, 3 - green/yellow, 4 - red/yellow
public:
    Schlocker() {
        for (int i = 0; i < CELL_QUANTITY; ++i) {
            cells[i] = new Cell(i);
        }
        indication.refresh();
    }

    //  Search cell with userId
    unsigned short cellSearch(unsigned long userId) {
        for (int i = 0; i < CELL_QUANTITY; ++i) {
            if (cells[i]->userId() == userId) {
                return i+1;
            }
        }
        return 0;
    }

    void updateStat(){
            bool isGreen=false;
      for(int i = 0; i <CELL_QUANTITY; i++){
        if(cells[i]->userId()==0){
          isGreen = true;
        }
      }
      if(isGreen){
        stat = 0;
      }else{
        stat = 2;
      }
      indication.set_mode(stat);
    }

    //  Refresh system status
    void refresh() {
updateStat();
        if (greenButton.isPushed()) {
            debugger.act("Green button", "Pressed");
            unsigned long userId = scanner.scan();
            if (userId != 0) {
                unsigned short cellId = cellSearch(userId);
                if (cellId != 0) {
                    cells[cellId-1]->open();
                } else {
                    cells[cellSearch(0)-1]->reg(userId);

                }
            }
        }
        redButton.isPushed();
        if (redButton.isPushed()) {
            debugger.act("Red button", "Pressed");
            unsigned long userId = scanner.scan();
            if (userId != 0) {
                unsigned short cellId = cellSearch(userId);
                if (cellId != 0) {
                    cells[cellId-1]->unreg();
                }
            }
        }
        if (closeButton.isPushed()){
          debugger.act("Close button", "Pressed");
          for(int i = 0; i < CELL_QUANTITY; i++){
            if(cells[i]->get_isOpen()){
              cells[i]->close();
            }
          }
        }
        indication.refresh();
    }

};

Schlocker *schlocker;

void setup() {
    Serial.begin(SERIAL_SPEED);
    schlocker = new Schlocker();
    debugger.loaded();
}

void loop() {
    schlocker->refresh();
    delay(LOOP_DELAY);
}
