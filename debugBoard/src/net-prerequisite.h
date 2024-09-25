#ifndef NETWORK
#define NETWORK

const char* SSID = "IOT-KeozID";
const char* PASSWORD = "keozdev-iot";

String apiServerName = "https://backend.wrsos.us.kg"; // e.g http://

const char* ntpServerName = "id.pool.ntp.org";
long ntpOffset = 0 * 3600; // offset(in hours) * 1 hours(in seconds)
unsigned long ntpInterval = 60000; // in millis

#endif