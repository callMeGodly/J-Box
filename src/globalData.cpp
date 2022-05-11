#include "globalData.h"
int engRPM = 0;
int throttlePosition = 0;
int MAP = 0;
int intakeAirTemp = 0;
int oilPressure = 0;
int fuelPressure = 0;
int knockValue = 0;
int maxKnockValue = 0;
int AirFuelRatio = 0;
int turbinePressure = 0;

int hybridBatteryVoltage = 0;
int engLoad = 0;
int hybridBatteryCurrent = 0;
int hybridBatteryTemp = 0;
int hybridBatteryCharge = 0;
int ecuAFR = 0;
int ecuMAP = 0;
int ecuTiming = 0;
int rawEcuMapReading = 0;
int rawEcuIatReading = 0;
int rpmPerPulse = 20;
bool loggingActive = false;
bool loggingSuccessful = false;
loggingState loggingStatus = sdError;

unsigned int gpsUpdateTime = 0;
unsigned int serialExtractTime = 0;
unsigned int ioReadTime = 0;
unsigned int dataLogTime = 0;
unsigned int displayUpdateTime = 0;
unsigned int mainLoopTime = 0;

Draw::Draw() :
u8g2(U8G2_R2, 2, 14, 7, 8, 6, 20, 21, 5, /*enable/wr=*/ 27 , /*cs/ce=*/ 26, /*dc=*/ 25, /*reset=*/24) // Connect RD (orange) with +5V, FS0 and FS1 with GND
{
}
void Draw::clearBox(int x0, int y0, int w, int h){
    u8g2.setDrawColor(0);
    u8g2.drawBox(x0,y0,w,h);
    u8g2.setDrawColor(1);
}
