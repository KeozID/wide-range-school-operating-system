#include "source.h"

DeviceFunctionMain mainFunction("MAIN_FUNCTION", 0);
DeviceFunctionRfid rfidFunction("RFID", 1);
DeviceFunctionLCD lcdFunction("LCD", 2, 1);
DeviceFunctionKeypad keypadFunction("KEYPAD", 3);

void setup() {
    mainFunction.initEssential(true);
    rfidFunction.initDevice();
    lcdFunction.bootLcd(lcd1);
}

void loop() {
	rfidFunction.getCardUid();
    char SK = keypadFunction.getButtonChar(false);

    if (SK) {
        Serial.print(SK);
    }

}