#ifndef CLASSES_HPP
#define CLASSES_HPP
#include <Arduino.h>
#include <U8g2lib.h>
#include "TouchScreen.h"
#include "display.h"
#include <string>
#include <math.h>
#include "globalData.h"
#include "gps.h"
#include "logging.h"

class dashItem{
public:
    Draw painter = Draw();
};

class statusMessage: public dashItem
{
    public:
    int x0 = 0; // the starting cordinates for the status message object
    int y0 = 0;
    const uint8_t* messageFont = u8g2_font_5x7_tr;
    const uint8_t* statusFont = messageFont;
    //enum displayItem {date,GPS_Status,loggingStatusMessage}; // maybe best to make these functions?
    int xEnd();
    void offsetStatus(int status_xOffset,int status_yOffset = 0);
    void initialize(const char * message, const char * status, int status_xOffset = 0,int status_yOffset = 0);
    void display(const char * status);
    void displayGPS_status();
    void displayLog_status();
    void displayDate();
    
    private:
    int x1; // stores the end location of the message: makes screen formating easier!
    int y00; // stores the top extent of the message
    int y1 = 0; // stores the bottom extent of the message
    int messageHeight = 0;
    int messageWidth = 0;
    int statusWidth = 0;
    int statusHeight = 0;
    int status_x0 = 0;
    int status_y0 = 0;
    int statusTop = 0;

};

class digitalGauge: public dashItem
{
    //todo 
    // consider adding extra fancy options like printing the units vertically to save space or having an indentifer precede the digits
    // Look at removing bloat by eliminating redundant variables instead using functions to modify key variables used in the printing process
    public:
    char printFormat [10] = "%1.0f";
    char unitText [10] = "mph";
    int x0 = 0;
    int y0 = screeny/2+20;
    float maxVal = 199;

    const uint8_t* digitFont = u8g2_font_logisoso30_tr;
    const uint8_t* unitFont = u8g2_font_VCR_OSD_mf;
    int xEnd();
    int yEndBottom();
    void unitLocation (int xOffset, int yOffset = 0);
    void findActiveArea();
    void updateRowArea();
    void display(float val);
    void displayUnit();
    void initialize(float val); // preserves backwards compatibility
    void initializeSmallGauge(int x, int y, float maximumVal, const char * format,const char * unit);
    void initializeMediumGauge(int x, int y, float maximumVal, const char * format,const char * unit);
    void initializeLargeGauge(float val);
    void initializeLargeGauge(int x, int y, float maximumVal, const char * format,const char * unit);
    void initializeLargeGauge(int x, int y, float maximumVal, const char * format,const char * unit,const uint8_t* font);
    
    private:
    bool isInitialized = false;
    float lastVal = 0;
    int maxDigitWidth = 0;
    int x0unit = 0;
    int y0unit = 0;
    int xUnitOffset = 0;
    int yUnitOffset = 0;
    int digitHeight = 0;
    int digit_y0 = 0;
    int digit_y1 = 0;
    uint8_t tx = 0; // used for the clear box function this is the x cordinate of the area that changes
    uint8_t ty = 0;
    uint8_t tw = 0;
    uint8_t th = 0;
    void initializeFormatting(); // perform the standard initialization calculations for formatting
};



#endif