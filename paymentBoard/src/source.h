#ifndef SOURCE
#define SOURCE

#include "include.h"
#include "net-prerequisite.h"

/*BOARD PARAMETER*/
#define BAUD_RATE 115200

/*Bus IO*/
#define SDA 3
#define SCL 8
#define RST_PIN 0 //9   the reset pin in rfid   
#define SS_PIN 5 //10   the fuckin sda pin in rfid

/*Standard IO*/
//TCSP3200
/*
#define S0 4 //output frequency scaling selection (s0, s1)
#define S1 5 
#define S2 6 //photodiode selection (s2, s3)
#define S3 7
#define sensorOut 8
*/ // Deprecated (Unused)

const uint8_t ROWS = 4;
const uint8_t COLS = 4;
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
uint8_t colPins[ROWS] = {40, 39, 38, 37}; // Pins used for the rows of the keypad
uint8_t rowPins[COLS] = {1, 2, 42, 41}; // Pins used for the columns of the keypad

int8_t buzzerPin = 45;

Keypad keypad4x4 = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

LiquidCrystal_I2C lcd1(0x26, 16, 2); //lcd1 16x4
LiquidCrystal_I2C lcd2(0x27, 20, 4); //lcd2 20x4

MFRC522 mfrc522(SS_PIN, RST_PIN); //rfid, only 1 instance at a time
MFRC522::MIFARE_Key key;
uint8_t nuidPICC[4];

WiFiUDP ntpUdp;
NTPClient timeClient(ntpUdp, ntpServerName, ntpOffset, ntpInterval);

class DeviceFunctionMain{
    public:
        DeviceFunctionMain(String name, uint32_t deviceId) {
            _name = name;
            _deviceId = deviceId;
        }

        void initEssential(bool serialOutput) {
            if (serialOutput == true) {
                Serial.begin(BAUD_RATE);
            }
            Wire.setPins(SDA, SCL);
            Wire.begin();
            SPI.begin();
        }

        void initWifi(bool ntpConnect) {
            WiFi.begin(SSID, PASSWORD);
            Serial.println("Connecting");
            while (WiFi.status() != WL_CONNECTED) {
                delay(500);
                Serial.print(".");
            }
            Serial.println("");
            Serial.print("Connected to WiFi network with IP Address: ");
            Serial.println(WiFi.localIP());

            if (ntpConnect == true) {
                timeClient.begin();
            }
            // timer is of importance, see the variables in 'net-prerequisite.h'

        }

        void postHttp() { // untested & editable
            String url = apiServerName; // modify this
            HTTPClient http;
            String response;

            StaticJsonDocument<200> buffer;
            String jsonParams;

            // below is example (editable) or make it return smth

            buffer["uid"] = "3A 2B 7F 28";

            serializeJson(buffer, jsonParams);
            
            http.begin(url);
            int statusCode = http.POST(jsonParams);
            response = http.getString();
            Serial.println(response);
            Serial.println(statusCode);
        }

        void getHttp() { // untested & editable
            String url = apiServerName; // modify this
            HTTPClient http;
            String response;

            http.begin(url);
            http.GET();
            
            response = http.getString();
            
            StaticJsonDocument<1024> document;
            deserializeJson(document, response);
            JsonObject object = document.as<JsonObject>();

            // below is example (editable) or make it returnable

            String data = object[String("data")];
            Serial.println(data);

            StaticJsonDocument<1024> document1;
            deserializeJson(document1, data);
            JsonObject object1 = document1.as<JsonObject>();

            String id = object1[String("id")];
            Serial.println(id);
        }

    private:
        String _name;
        uint32_t _deviceId;
};

class DeviceFunctionLCD : public DeviceFunctionMain{
    public:
        //type 1 for 16x2 type 2 for 20x4
        DeviceFunctionLCD(String name, uint32_t deviceId, uint8_t type) : DeviceFunctionMain(name, deviceId) {
            _type = type;
        }

        template <typename LCD>
        void bootLcd(LCD _lcd) {
            _lcd.init();
            _lcd.backlight();

            if (_type == 1) {
                _lcd.setCursor(0, 0);
                _lcd.print("LCD Initialized..");
                _lcd.setCursor(0, 1);
                _lcd.print("16x2");
            } else if (_type == 2) {
                _lcd.setCursor(0, 0);
                _lcd.print("LCD Initialized");
                _lcd.setCursor(0, 1);
                _lcd.print("20x4");
            }
        }

    private:
        uint8_t _type;

};

class DeviceFunctionRfid : public DeviceFunctionMain{ // this shit is fucked honestly
    public:
        DeviceFunctionRfid(String name, uint32_t deviceId) : DeviceFunctionMain(name, deviceId) {};

        void initDevice() {
            mfrc522.PCD_Init();
            for (byte i = 0; i < 6; i++) {
                key.keyByte[i] = 0xFF;
            }
        }

        void getCardUid() { // debug purpose

            if (!mfrc522.PICC_IsNewCardPresent()) {
                return;
            }
            if (!mfrc522.PICC_ReadCardSerial()) {
                return;
            }

            Serial.print(F("PICC type: "));
            MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
            Serial.println(mfrc522.PICC_GetTypeName(piccType));

            if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
                piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
                piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
                    Serial.println(F("Your tag/card type is not of MIFARE Classic."));
                    return;
                }
            
            if (mfrc522.uid.uidByte[0] != nuidPICC[0] ||
                mfrc522.uid.uidByte[1] != nuidPICC[1] ||
                mfrc522.uid.uidByte[2] != nuidPICC[2] ||
                mfrc522.uid.uidByte[3] != nuidPICC[3]) {
                    Serial.println(F("A new card has been detected."));
                    
                    // Store NUID into nuidPICC array (to save the previous card nuid)
                    for (byte i = 0; i < 4; i++) {
                        nuidPICC[i] = mfrc522.uid.uidByte[i];
                    }

                    Serial.println(F("++CARD/TAG NUID INFO++"));
                    Serial.print(F("NUID (HEX): "));
                    printHex(mfrc522.uid.uidByte, mfrc522.uid.size);
                    Serial.println();
                    Serial.print(F("NUID (DEC): "));
                    printDec(mfrc522.uid.uidByte, mfrc522.uid.size);
                    Serial.println();
                } else {
                    Serial.println(F("Card has been read previously. "));
                }

                // Halt PICC (idk why, but is prob to give mcu a break)
                mfrc522.PICC_HaltA();

                // Stop encryption on PCD
                mfrc522.PCD_StopCrypto1();
        }

        /*
        -type:
            dec = return decimal value
            hex = return hex value
        */
        String getCardUid(String type) {
            String uid;

            if (!mfrc522.PICC_IsNewCardPresent()) {
                return String(); // returning str because it doesnt want to return nothing
            }
            if (!mfrc522.PICC_ReadCardSerial()) {
                return String();
            }

            type.toUpperCase();

            if (type == "DEC") {
                uid = getUidDec(mfrc522.uid.uidByte, mfrc522.uid.size);
            } else if (type == "HEX") {
                uid = getUidHex(mfrc522.uid.uidByte, mfrc522.uid.size);
            } else return "invalid type param";
            return uid.substring(1); // this return the bare uid .substring(1)
        }

        String getCardUid(String type, bool loop) {
            String uid;

            while (loop == true) {
                if(!mfrc522.PICC_IsNewCardPresent()) {
                    continue;
                }
                if(!mfrc522.PICC_ReadCardSerial()) {
                    continue;
                }

                type.toUpperCase();

                if (type == "DEC") {
                    uid = getUidDec(mfrc522.uid.uidByte, mfrc522.uid.size);
                } else if (type == "HEX") {
                    uid = getUidHex(mfrc522.uid.uidByte, mfrc522.uid.size);
                } else return "invalid type param";

                loop = false;
                return uid.substring(1);
            }
        }

        void getCardDatabaseInformation(String uid) {
            String url = apiServerName + "/api/get-data";
            HTTPClient http;
            String response;

            StaticJsonDocument<200> buffer;
            String jsonParams;

            buffer["uid"] = uid;

            serializeJson(buffer, jsonParams);
            
            http.begin(url);
            http.addHeader("content-type", "application/json");
            int statusCode = http.POST(jsonParams);
            response = http.getString();
            Serial.println(response);
            Serial.println(statusCode);

            StaticJsonDocument<1024> docData;
            deserializeJson(docData, response);
            JsonObject objData = docData.as<JsonObject>();

            String data = docData[String("data")];

            StaticJsonDocument<1024> document;
            deserializeJson(document, data);
            JsonObject object = document.as<JsonObject>();

            String id = object[String("id")];
            String name = object[String("name")];
            bool superuser = object[String("superuser")];
            String nis = object[String("nis")];
            String email = object[String("email")];
            int balance = object[String("balance")];

            Serial.println(id);
            Serial.println(name);
            Serial.println(superuser);
            Serial.println(nis);
            Serial.println(email);
            Serial.println(balance);
        }

        template <typename LCD>
        void cardPayment(String uid, int price, LCD lcd) {
            String url = apiServerName + "/api/pay-card";
            HTTPClient http;
            String response;

            StaticJsonDocument<200> buffer;
            String jsonParams;

            buffer["uid"] = uid;
            buffer["price"] = price;

            serializeJson(buffer, jsonParams);
            
            http.begin(url);
            http.addHeader("content-type", "application/json");
            int statusCode = http.POST(jsonParams);
            response = http.getString();
            Serial.println(response);
            Serial.println(statusCode);

            StaticJsonDocument<1024> docData;
            deserializeJson(docData, response);
            JsonObject objData = docData.as<JsonObject>();

            String data = docData[String("data")];
            String message = docData[String("message")];

            StaticJsonDocument<1024> document;
            deserializeJson(document, data);
            JsonObject object = document.as<JsonObject>();

            String balance = document[String("balance")];

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(message);
            lcd.setCursor(0, 1);
            lcd.print("Balance:");
            lcd.print(balance);
        }

        template <typename LCD>
        void cardPresence(String uid, LCD lcd) {
            String url = apiServerName + "/api/presence";
            HTTPClient http;
            String response;

            StaticJsonDocument<200> buffer;
            String jsonParams;

            buffer["uid"] = uid;

            serializeJson(buffer, jsonParams);
            
            http.begin(url);
            http.addHeader("content-type", "application/json");
            int statusCode = http.POST(jsonParams);
            response = http.getString();
            Serial.println(response);
            Serial.println(statusCode);

            StaticJsonDocument<1024> docData;
            deserializeJson(docData, response);
            JsonObject objData = docData.as<JsonObject>();

            String data = docData[String("data")];

            StaticJsonDocument<1024> document;
            deserializeJson(document, data);
            JsonObject object = document.as<JsonObject>();

            String name = document[String("name")];
            bool presence = document[String("presence")];

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(name);
            lcd.setCursor(0, 1);
            if (presence == true) {
                lcd.print("Logged in");
            } else lcd.print("Logged out");

        }

        template <typename LCD>
        void cardTopup(String uid, int value, LCD lcd) {
            String url = apiServerName + "/api/topup-card";
            HTTPClient http;
            String response;

            StaticJsonDocument<200> buffer;
            String jsonParams;

            buffer["uid"] = uid;
            buffer["value"] = value;

            serializeJson(buffer, jsonParams);
            
            http.begin(url);
            http.addHeader("content-type", "application/json");
            int statusCode = http.POST(jsonParams);
            response = http.getString();
            Serial.println(response);
            Serial.println(statusCode);

            StaticJsonDocument<1024> docData;
            deserializeJson(docData, response);
            JsonObject objData = docData.as<JsonObject>();

            String data = docData[String("data")];
            String message = docData[String("message")];

            StaticJsonDocument<1024> document;
            deserializeJson(document, data);
            JsonObject object = document.as<JsonObject>();

            String balance = document[String("balance")];

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(message);
            lcd.setCursor(0, 1);
            lcd.print("Balance:");
            lcd.print(balance);
        }

        void printHex(uint8_t *buffer, uint8_t bufferSize) {
            for (byte i = 0; i < bufferSize; i++) {
                Serial.print(buffer[i] < 0x10 ? " 0" : " ");
                Serial.print(buffer[i], HEX);
            }
        }

        void printDec(uint8_t *buffer, uint8_t bufferSize) {
            for (byte i = 0; i < bufferSize; i++) {
                Serial.print(' ');
                Serial.print(buffer[i], DEC);
            }
        }

        String getUidHex(uint8_t *buffer, uint8_t bufferSize) {
            String content = "";
            for (byte i = 0; i < bufferSize; i++) {
                content.concat(String(buffer[i] < 0x10 ? " 0" : " "));
                content.concat(String(buffer[i], HEX));
            }
            content.toUpperCase();
            return content;
        }

        String getUidDec(uint8_t *buffer, uint8_t bufferSize) {
            String content = "";
            for (byte i = 0; i < bufferSize; i++) {
                content.concat(String(buffer[i] < 0x10 ? " 0" : " "));
                content.concat(String(buffer[i], DEC));
            }
            content.toUpperCase();
            return content;
        }

    private:

};

//specifically for TCSP3200
/*
class DeviceFunctionColorSensor : public DeviceFunctionMain{
    public:
        DeviceFunctionColorSensor(String name, uint32_t deviceId) : DeviceFunctionMain(name, deviceId) {};

        //frequencyScaling = 0=0%, 1=2%, 2=20%, 3=100%
        void initDevice(uint8_t frequencyScaling) {
            pinMode(S0, OUTPUT);
            pinMode(S1, OUTPUT);
            pinMode(S2, OUTPUT);
            pinMode(S3, OUTPUT);
            pinMode(sensorOut, INPUT);

            switch (frequencyScaling) {
                case 0:
                    digitalWrite(S0, LOW);
                    digitalWrite(S1, LOW);
                case 1:
                    digitalWrite(S0, LOW);
                    digitalWrite(S1, HIGH);
                case 2:
                    digitalWrite(S0, HIGH);
                    digitalWrite(S1, LOW);
                case 3:
                    digitalWrite(S0, HIGH);
                    digitalWrite(S1, HIGH);
            }
        }

        int getRedFrequency() {
            digitalWrite(S2, LOW);
            digitalWrite(S3, LOW);
            
            int redFrequency = pulseIn(sensorOut, LOW);
            
            return redFrequency;
        }
        
        int getGreenFrequency() {
            digitalWrite(S2, HIGH);
            digitalWrite(S3, HIGH);

            int greenFrequency = pulseIn(sensorOut, LOW);

            return greenFrequency;
        }

        int getBlueFrequency() {
            digitalWrite(S2, LOW);
            digitalWrite(S3, HIGH);

            int blueFrequency = pulseIn(sensorOut, LOW);

            return blueFrequency;
        }

        int getNoFilterFrequency() {
            digitalWrite(S2, HIGH);
            digitalWrite(S3, LOW);

            int clearFrequency = pulseIn(sensorOut, LOW);

            return clearFrequency;
        }

    private:

};
*/ //Deprecated (Unused)

class DeviceFunctionKeypad : public DeviceFunctionMain{
    public:
        DeviceFunctionKeypad(String name, uint32_t deviceId) : DeviceFunctionMain(name, deviceId) {};

        char getButtonChar(bool isDirectPrint) { // the param names are stupid, basically if true, it means that you wont be able to create a variable assigned to this function
            char button = keypad4x4.getKey();
            
            if (isDirectPrint == true) {
                if (button) {
                    return button;
                }
            } else return button;
        }

        template <typename LCD>
        String getButtonString(LCD lcd, bool paymentPrefix) {
            String word = "";
            lcd.backlight();

            bool loop = true;
            while (loop) {
                char character = getButtonChar(false);
                if (character) {
                    if (character == 'A') {} // accept
                    if (character == 'B') {}
                    if (character == 'C') {word = "";} // cancel/clear
                    if (character == 'D') {loop = false; return word;} // done
                    if (character == '*') {}
                    if (character == '#') {}
                    word += character;
                }

                lcd.clear();
                if (paymentPrefix == true) {lcd.print("Total:");}
                lcd.print(word);
                delay(50);
            }
        }

    private:
};

class DeviceFunctionNtp : public DeviceFunctionMain{ // will look back on this later
    public:
        DeviceFunctionNtp(String name, uint32_t deviceId) : DeviceFunctionMain(name, deviceId) {};

        int getNtpUnixtime() {
            timeClient.update();
            return timeClient.getEpochTime();
        }

        String getNtpFormattedTime() {
            timeClient.update();
            return timeClient.getFormattedTime();
        }
    private:
};

#endif