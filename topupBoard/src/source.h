#ifndef SOURCE
#define SOURCE

#include "include.h"

/*Bus IO*/
#define SDA 3
#define SCL 8
#define RST_PIN 9
#define SS_PIN 10

/*Standard IO*/
//TCSP3200
#define S0 4 //output frequency scaling selection (s0, s1)
#define S1 5 
#define S2 6 //photodiode selection (s2, s3)
#define S3 7
#define sensorOut 8

class AssignedGpioPin{
    public:
};

LiquidCrystal_I2C lcd1(0x26, 16, 2); //lcd1 16x4
LiquidCrystal_I2C lcd2(0x27, 20, 4); //lcd2 20x4
MFRC522 mfrc522(SS_PIN, RST_PIN); //rfid, only 1 instance at a time

class DeviceFunctionMain{
    public:
        DeviceFunctionMain(String name, uint32_t deviceId) {
            _name = name;
            _deviceId = deviceId;
        }

        void initEssential() {
            SPI.begin();
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

class DeviceFunctionRfid : public DeviceFunctionMain{
    public:
        DeviceFunctionRfid(String name, uint32_t deviceId) : DeviceFunctionMain(name, deviceId) {};

        void initDevice() {
            mfrc522.PCD_Init();
        }

    private:

};

//specifically for TCSP3200
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
        }

    private:

};

#endif