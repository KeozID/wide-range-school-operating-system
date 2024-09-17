#ifndef NETWORK
#define NETWORK

const char* SSID = "IOT-KeozID";
const char* PASSWORD = "keozdev-iot";

String serverName = ""; // e.g http://

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;

#endif