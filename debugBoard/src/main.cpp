#include "source.h"

DeviceFunctionMain mainFunction("MAIN_FUNCTION", 0);
DeviceFunctionRfid rfidFunction("RFID", 1);
DeviceFunctionLCD lcdFunction("LCD", 2, 1);
DeviceFunctionKeypad keypadFunction("KEYPAD", 3);
DeviceFunctionNtp ntpFunction("NTP", 4);

int resumeDelay = 4500;
int buzzerDelay = 200;
void idle();
void mainMenu();
void paymentMenu();
void presenceMenu();
void topupMenu();

void setup() {
    mainFunction.initEssential(true);
    mainFunction.initWifi(true);
    rfidFunction.initDevice();
    lcd1.init();
    lcd1.backlight();
    pinMode(buzzerPin, OUTPUT);

    idle();
}

void loop() {
    Serial.print("ERROR");
    delay(50);
}

//board code
void idle() {
    lcd1.noBacklight();
    
    bool loop = true;
    while (loop == true) {
        char keyDetect = keypadFunction.getButtonChar(false);
        if (keyDetect) {
            if(keyDetect == 'D') {
                loop = false;
            }
        }
    }
    mainMenu();
}

void mainMenu() {
    lcd1.backlight();

    lcd1.clear();
    lcd1.setCursor(0, 0);
    lcd1.print("1)Payment");

    lcd1.setCursor(11, 0);
    lcd1.print("3)Top");
    lcd1.setCursor(13, 1);
    lcd1.print("-up");

    lcd1.setCursor(0, 1);
    lcd1.print("2)Presence");

    bool loop = true;
    while (loop == true) {
        char keyDetect = keypadFunction.getButtonChar(false);
        if (keyDetect) {
            if(keyDetect == '1') {
                loop = false;
                paymentMenu();
            }
            if(keyDetect == '2') {
                loop = false;
                presenceMenu();
            }
            if(keyDetect == '3') {
                loop = false;
                topupMenu();
            }
        }
    }
}

void paymentMenu() {
    lcd1.clear();
    
    String price = keypadFunction.getButtonString(lcd1, true);
    
    lcd1.setCursor(0, 0);
    lcd1.print("Total:");
    lcd1.print(price);
    lcd1.setCursor(0, 1);
    lcd1.print("Scan your card");

    String uid = rfidFunction.getCardUid("HEX", true);

    //Wait
    lcd1.clear();
    lcd1.setCursor(0, 0);
    lcd1.print("Please wait...");

    rfidFunction.cardPayment(uid, price.toInt(), lcd1);

    //Buzzer
    digitalWrite(buzzerPin, 1);
    delay(buzzerDelay);
    digitalWrite(buzzerPin, 0);

    delay(resumeDelay);
    idle();
}

void presenceMenu() {
    lcd1.clear();

    lcd1.setCursor(0, 0);
    lcd1.print("Scan your card");

    String uid = rfidFunction.getCardUid("HEX", true);

    //Wait
    lcd1.clear();
    lcd1.setCursor(0, 0);
    lcd1.print("Please wait...");

    rfidFunction.cardPresence(uid, lcd1);

    //Buzzer
    digitalWrite(buzzerPin, 1);
    delay(buzzerDelay);
    digitalWrite(buzzerPin, 0);

    delay(resumeDelay);
    idle();
}

void topupMenu() {
    lcd1.clear();
    
    String value = keypadFunction.getButtonString(lcd1, true);
    
    lcd1.setCursor(0, 0);
    lcd1.print("Total:");
    lcd1.print(value);
    lcd1.setCursor(0, 1);
    lcd1.print("Scan your card");

    String uid = rfidFunction.getCardUid("HEX", true);

    //Wait
    lcd1.clear();
    lcd1.setCursor(0, 0);
    lcd1.print("Please wait...");

    rfidFunction.cardTopup(uid, value.toInt(), lcd1);

    //Buzzer
    digitalWrite(buzzerPin, 1);
    delay(buzzerDelay);
    digitalWrite(buzzerPin, 0);

    delay(resumeDelay);
    idle();
}