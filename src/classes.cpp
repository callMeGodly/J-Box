#include "classes.hpp"


int statusMessage::xEnd()
    {
        int xEnd = status_x0 + statusWidth;
        return(xEnd);
    }
void statusMessage::offsetStatus(int status_xOffset,int status_yOffset = 0){
        status_x0 += status_xOffset;
        status_y0 += status_yOffset;
}

void statusMessage::initialize(const char * message, const char * status, int status_xOffset = 0,int status_yOffset = 0) // performs all formatting calculations call this once
{
    // draw the message
    painter.u8g2.setFont(messageFont);
    int8_t messageAscent = painter.u8g2.getAscent();
    int8_t messageDecent = painter.u8g2.getDescent();
    messageWidth = painter.u8g2.getStrWidth(message);
    messageHeight = messageAscent - messageDecent; // the area the message can occupy
    y1 = y0 + messageDecent; // find the absolute bottom of the message
    y00 = y0 - messageAscent; // the upper corner of the message
    painter.clearBox(x0,y00,messageWidth,messageHeight); // clear the area just for good measure
    painter.u8g2.drawStr(x0,y0,message);// draw the message
    
    // draw the initial status
    painter.u8g2.setFont(statusFont); //switch to the status font
    int8_t statusDecent = painter.u8g2.getDescent();
    int8_t statusAscent = painter.u8g2.getAscent();
    statusWidth = painter.u8g2.getStrWidth(status);
    statusHeight = statusAscent - statusDecent; // the area the message can occupy
    statusTop = y0 - statusAscent; // the upper corner of the message
    status_x0 = x0 + messageWidth + status_xOffset; // start the status text at the end of the message
    status_y0 = y0 + status_yOffset;
    painter.u8g2.drawStr(status_x0,status_y0,status);
}
    
void statusMessage::display(const char * status)
{
    painter.clearBox(status_x0,statusTop,statusWidth,statusHeight);
    painter.u8g2.setFont(statusFont);
    painter.u8g2.drawStr(status_x0,status_y0,status);
}

void statusMessage::displayGPS_status()
{
    if(GPSconnected)
    display("Connected!");
    else
    display("Disconnected");
}

void statusMessage::displayLog_status()
{
    switch (loggingStatus)
    {
    case loggingOff:
        display("OFF");
        break;
    
    case logRunning:
        display("ON");
        break;

    case sdError:
        display("sdErr");
        break;

    default:
        break;
    }
    // if(loggingActive)
    // display("ON");
    // else
    // display("OFF");
}
void statusMessage::displayDate()
{
    //char dateTimeString [20] = constructDateTime(2).c_str();
    display(constructDateTime(3).c_str());
}






int digitalGauge::xEnd()
{
    painter.u8g2.setFont(unitFont); // set the font to the unit font so the appropriate width can be calculated
    int totalWidth = maxDigitWidth +    painter.u8g2.getStrWidth(unitText) + xUnitOffset;
    int x1 = x0 + totalWidth; 
    // or the max of the x value of xunit+strwidth or x0+digitwidth
    return(x1);
}

int digitalGauge::yEndBottom()
{
    painter.u8g2.setFont(digitFont); // set font to digit font to get values
    int y1 = y0 + painter.u8g2.getDescent(); // determine how much the digit hangs down below 
    painter.u8g2.setFont(unitFont); // set the font to the unit font
    y1 = max(y1,y0unit+painter.u8g2.getDescent()); // the bottom of the object is just max o
    return(y1);
}

void digitalGauge::unitLocation (int xOffset, int yOffset = 0)
{
    xUnitOffset = xOffset;
    yUnitOffset = yOffset;
}

void digitalGauge::findActiveArea()
{
    //(NOTE x position of update area cannot be changed with 6963 controller with current library, but this is a possible limitation of the controller itself)
    tx = x0 / 8; // find the smallest possible value for the beginning of the title block 
    tw = ceil(float(x0 + maxDigitWidth) / 8.0) - tx; // round up to find the minimum size box that needs to be updated 
    ty = 16 - ((digit_y0) / 8); // Because display is rotated the top of the screen is actually the bottom.
    th = ceil(float(digitHeight)/8.0);
}

void digitalGauge::updateRowArea()
{
    painter.u8g2.updateDisplayArea(0,ty,30,th); // only update the area required tile cordinates are 8 pixel blocks each
}

void digitalGauge::display(float val)
{
    if(isInitialized == false) // if the display has not yet been initialized
    {
        // initializing the display here is doing funny things
        // displays MPH twice on bootup of insight screen
        //initialize(val); // initialize the display
        return; // dont do anything more
    }
    if( abs(((lastVal - val) * 200) / maxVal) < 1) // if the diffrence between the values is less than .5% 
    {
        return; // don't do anything if no updates need to be made to save time
    }
    lastVal = val; // otherwise save the latest value;
    // if(val > maxVal)
    updateRequest = true;
    painter.clearBox(x0,digit_y0,maxDigitWidth,digitHeight);
    painter.u8g2.setFont(digitFont);
    char digitString [16] = "Error"; // make it error so its ovbious if theres a problem
    sprintf(digitString,printFormat,val); // generate a string with the digits in it
    #define currentWidth    painter.u8g2.getStrWidth(digitString) // use the current width to set the location of the digits
    #define digit_x0 x0 + maxDigitWidth - currentWidth // this always places the digit end in the same location
    painter.u8g2.drawStr(digit_x0,y0,digitString);
}

void digitalGauge::initializeFormatting() // perform the standard initialization calculations for formatting
{
    painter.u8g2.setFont(digitFont);
    char digitString [16] = "Error"; // make it error so its ovbious if theres a problem
    sprintf(digitString,printFormat,maxVal); // generate a string with the digits in it
    maxDigitWidth = painter.u8g2.getStrWidth(digitString);
    //digitHeight = painter.u8g2.getAscent() -   painter.u8g2.getDescent(); // the area the digit can occupy
    digitHeight =   painter.u8g2.getAscent(); // digits dont tend to extend below -    painter.u8g2.getDescent(); // the area the digit can occupy
    digit_y0 = y0 - painter.u8g2.getAscent(); // the upper corner of the digit
    painter.u8g2.setFont(unitFont);
    x0unit = x0 + maxDigitWidth + xUnitOffset;
    y0unit = y0 - yUnitOffset; 
    findActiveArea();
}

void digitalGauge::displayUnit()
{
    painter.u8g2.drawStr(x0unit,y0unit,unitText); // draw the unit text
}

void digitalGauge::initialize(float val) // preserves backwards compatibility
{
    initializeFormatting(); // perform the formatting calculations
    isInitialized = true;
    displayUnit(); // display the unit
    display(val);
}

void digitalGauge::initializeSmallGauge(int x, int y, float maximumVal, const char * format,const char * unit)
{
    x0 = x; y0 = y; maxVal = maximumVal;
    strcpy(printFormat,format);
    strcpy(unitText,unit);
    digitFont = u8g2_font_6x12_mn;
    unitFont =  u8g2_font_5x7_tr;
    initialize(maxVal);
}

void digitalGauge::initializeMediumGauge(int x, int y, float maximumVal, const char * format,const char * unit)
{
    x0 = x; y0 = y; maxVal = maximumVal;
    strcpy(printFormat,format);
    strcpy(unitText,unit);
    digitFont = u8g2_font_VCR_OSD_mf;
    unitFont =  u8g2_font_6x12_tr;
    initialize(maxVal);
}

void digitalGauge::initializeLargeGauge(float val)
{
    digitFont = u8g2_font_logisoso30_tr;
    unitFont =  u8g2_font_VCR_OSD_mf;
    initialize(val);
}

void digitalGauge::initializeLargeGauge(int x, int y, float maximumVal, const char * format,const char * unit)
{
    x0 = x; y0 = y; maxVal = maximumVal;
    strcpy(printFormat,format);
    strcpy(unitText,unit);
    initializeLargeGauge(maximumVal);
}

void digitalGauge::initializeLargeGauge(int x, int y, float maximumVal, const char * format,const char * unit,const uint8_t* font)
{
    x0 = x; y0 = y; maxVal = maximumVal;
    digitFont = u8g2_font_logisoso30_tr;
    unitFont = font;
    strcpy(printFormat,format);
    strcpy(unitText,unit);
    initialize(maximumVal);
}