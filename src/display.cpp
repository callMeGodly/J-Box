#include "display.h"
#include "gps.h"
#include "logging.h"
#include "globalData.h"
#include "classes.hpp"

U8G2_T6963_240X128_F_8080 u8g2(U8G2_R2, 2, 14, 7, 8, 6, 20, 21, 5, /*enable/wr=*/ 27 , /*cs/ce=*/ 26, /*dc=*/ 25, /*reset=*/24); // Connect RD (orange) with +5V, FS0 and FS1 with GND
TSPoint p;
void modifyPointToScreen()
{
p.x =  map(p.x,100,944,0,240);
p.y =  map(p.y,190,860,0,128);
if(p.z < 5)
{
    return; // ignore values below 5: they show up when not pressed
}
p.z = abs(map(p.z,900,250,0,255));
}

TouchScreen ts = TouchScreen(XP, YP, XM, YM , 730);
enum screens {menu,settings,gps,naughtTo60Timer};
screens currentScreen = menu;

class touchEvent
{
    public:
    int x = 0;
    int y = 0;
    uint8_t minimumDuration = 50;
    uint8_t staleDuration = 5; // rework this method of debouncing or de
    int duration = 0;
    int minPressure = 10;
    int maxPressure = 255;

    private:
    elapsedMillis touchDuration;
    elapsedMillis lastEvent;
    bool isScreenPressed = false;
    int touchEventDuration = 0;
    #define numTraces 4
    struct trace // this stores a history of a "trace" - a touch event where pressed and moved
    {
        int x = 0;
        int y = 0;
        int z = 0;
        int t = 0;
    } traceData[numTraces];
    uint8_t traceIndex = 0;
    

    public:
    void detect() // the detect touch function serves to identify if the screen has been touched, if it meets the minimum duration
    {
        p = ts.getPoint();
        modifyPointToScreen(); // maybe avoid use of MAP function for this process intensive
        // Serial.print(p.x);
        // Serial.print(",");
        // Serial.print(p.y);
        // Serial.print(",");
        // Serial.println(p.z);
        if (p.z > minPressure && p.z < maxPressure) // if the pressure meets our criteria
        {
            if(isScreenPressed == false) // if the screen was previously not pressed
            {
                touchDuration = 0; // reset the touch duration counter
            }
            isScreenPressed = true; // note that the screen is now being pressed
            x = p.x;
            y = p.y;
            // record the trace data
            traceData[traceIndex].x = x;
            traceData[traceIndex].y = y;
            traceData[traceIndex].z = p.z;
            traceData[traceIndex].t = touchDuration;
            traceIndex++;
            if(traceIndex >= numTraces) { traceIndex = 0;} // if we exceed the maximum number of traces reset it
            
            // Serial.print(x);
            // Serial.print(",");
            // Serial.print(y);
            // Serial.print(",");
            // Serial.println(touchDuration);
        }
        else
        {
            if(isScreenPressed == true) // if the screen was previously being pressed
            {
                touchEventDuration = touchDuration; // record the duration of the touch event
                duration = touchEventDuration;
                for(int i = traceIndex + 1; i < numTraces; i++)
                // go through the trace loop and fill in any remaining spots with the last available data
                {
                  traceData[i].x = traceData[traceIndex].x;
                  traceData[i].y = traceData[traceIndex].y;
                  traceData[i].z = traceData[traceIndex].z;
                  //traceData[i].t = traceData[traceIndex].t;
                  traceData[i].t = duration; // use the current duration to reflect the total time of press

                }
                //traceData[traceIndex].t = duration;
                lastEvent = 0; // record the time since the last press
                traceIndex = 0; // reset the trace index
            }
            isScreenPressed = false; // note that the screen is no longer being pressed

        }
    }
    bool isPressed()
    {
        return(isScreenPressed);
    }
    
    bool isAreaPressed(int xCenter, int yCenter, int width, int height)
    {
       if(isScreenPressed == false)
       {
           return(false);
       }
       // test for a faulty case and return false if found
       if(abs(x - xCenter) > width)
       {
          return(false);
       }

       if(abs(y - yCenter) > height)
       {
          return(false);
       }
       // if we pass all tests then our area must have been tapped
       return(true);
    }

    bool isTapped()
    {
        
        return( (touchEventDuration > minimumDuration) && (lastEvent < staleDuration) && (isScreenPressed == false)  );
        // all of these conditions must be true for a valid touch press
    }

    bool isAreaTapped(int xCenter, int yCenter, int width, int height)
    {
        // a better way to do this might be to go from the last trace value back to the first
        // basically checking that the button has been pressed
        // might be able just to run the loop backwards from the trace index
        if((lastEvent > staleDuration)){ // check to make sure the event did not occur too long ago
            return(false);
        }
        int i = 0;
        #define minHoldDuration 80
        while( (traceData[i].t < minHoldDuration) && (i < numTraces)) //go though the trace data until the hold duration is exceeded or the index expries
        {
            // test for a faulty case and return false if found
            // see if the diffrence between the location of known touch points and the center is greater than the allowable tolerance
            if(abs(traceData[i].x - xCenter) > width)
            {
                Serial.print("xFailed @");
                Serial.print(traceData[i].x);
                Serial.print(" ! ");
                Serial.print(xCenter);
                Serial.print(" @index: ");
                Serial.print(i);
                Serial.print(" @dur: ");
                Serial.println(traceData[i].t);

                Serial.print("xFailed @");
                Serial.print(traceData[i-1].x);
                Serial.print(" ! ");
                Serial.print(xCenter);
                Serial.print(" @index: ");
                Serial.print(i-1);
                Serial.print(" @dur: ");
                Serial.println(traceData[i-1].t);

                return(false); // return the failed result
            }
            if(abs(traceData[i].y - yCenter) > height) 
            {
                Serial.print("yFailed @");
                Serial.print(traceData[i].y);
                Serial.print(" ! ");
                Serial.print(yCenter);
                Serial.print(" @index: ");
                Serial.print(i);
                Serial.print(" @dur: ");
                Serial.println(traceData[i].t);

                Serial.print("yFailed @");
                Serial.print(traceData[i-1].y);
                Serial.print(" ! ");
                Serial.print(yCenter);
                Serial.print(" @index: ");
                Serial.print(i-1);
                Serial.print(" @dur: ");
                Serial.println(traceData[i-1].t);
                return(false);
            }
            
            i++; //increment the index
        }
        // if we pass all tests then our area must have been tapped
        // clear the area
        
        return(true);
    }

};

touchEvent tap;

void initializeDisplay()
{
    pinMode(backlightPin,OUTPUT);
    analogWrite(backlightPin,100);
    u8g2.begin();
    //u8g2.setFlipMode(1); //6963 does not seem to support rotation of internal buffer
    u8g2.setContrast(255);
    u8g2.clearBuffer();
    // u8g2.setFont(u8g2_font_logisoso58_tf);
    // u8g2.drawStr(25,80,"J-Box");
    u8g2.drawXBM( 40, 10, JboxIcon_width, JboxIcon_height, JboxIcon_bits);
    //u8g2.drawXBM( 0, 30, ScRacing_width, ScRacing_height, ScRacing_bits);
    u8g2.setFont(u8g2_font_courR10_tr);
    u8g2.setFontPosBaseline(); // Set the font position to the bottom
    u8g2.drawStr(160,10,"J-Box");
    u8g2.drawStr(160,23,"Bootup");
    u8g2.sendBuffer();
    delay(2000);
    u8g2.clearBuffer();
}

void displayScreen()
{
  updateRequest = false;
  tap.detect();
  screenPointer(); // pull up the screen pointed to by the screen pointer
  
  if(updateRequest || millis() - lastDisplayUpdate > 500) // update the display atleast twice per second
    {
    u8g2.sendBuffer();
    lastDisplayUpdate = millis();
    }
}

class boxGauge
    {
    public:
    int padding = 2;
    int xStart = padding;
    int yStart = padding;
    int width = screenx-2*padding-40;
    int height = screeny/5;
    int max = 12000; int cutoff = 4000; int redLine = 10000;
    bool shiftText = false;

    private: // hidden variables
    int lastValue = 0; // use this to save the last value
    public: // functions

    void display(int current){ 
    if(current == lastValue) // if there is no change in the reading
        return; // dont do anything to save time
    lastValue = current;
    updateRequest = true;
     painter.clearBox(xStart,yStart,screenx-xStart,height);
     u8g2.drawFrame(xStart,yStart,width,height);
     int cutoffWidth = int(width*(float(cutoff/2.0)/float(max)));
     if (current <= cutoff) { // this part draws and clears the filled in bars
        u8g2.drawBox(xStart,yStart,int(width*(float(current/2.0)/float(max))),height);
        } else {
        u8g2.drawBox(xStart,yStart,cutoffWidth,height);
        u8g2.drawBox(xStart+cutoffWidth,yStart,int((width-cutoffWidth)*(float(current - cutoff)/float(max - cutoff))),height);
        } 
    }

    void drawBoxGauge(int current) { // idea: numbers that exist within the boxgauge showing the current value / shift
    painter.clearBox(xStart,yStart,screenx-xStart,height);
    u8g2.drawFrame(xStart,yStart,width,height);
    int cutoffWidth = int(width*(float(cutoff/2.0)/float(max)));
    if (current <= cutoff) { // this part draws and clears the filled in bars
        u8g2.drawBox(xStart,yStart,int(width*(float(current/2.0)/float(max))),height);
    } else {
        u8g2.drawBox(xStart,yStart,cutoffWidth,height);
        u8g2.drawBox(xStart+cutoffWidth,yStart,int((width-cutoffWidth)*(float(current - cutoff)/float(max - cutoff))),height);
    } 
    for (int i = 0; i<=max; i+=1000) {
        int offset;
        if (i<=cutoff) {
            offset=int((float(i)/float(max))*width)/2;
        } else {
            offset=cutoffWidth+int((float(i-cutoff)/float(max-cutoff))*(width-cutoffWidth));
        }
        if (i==redLine) {
            u8g2.drawVLine(xStart + offset, yStart, 5+height);
        } else {
            u8g2.drawVLine(xStart + offset, yStart+height, 5);
        }
        if (i%2000!=0 && i<=cutoff) {
            continue;
        }
        u8g2.setFont(u8g2_font_bitcasual_tn);
        int xfontoff = (i<10000) ? 2 : 5;
        u8g2.drawStr(xStart + offset - xfontoff, yStart+height+5+8,String(i/1000).c_str());
    }
    u8g2.setFont(u8g2_font_logisoso18_tf);
    int newFontx = 11;
    int yOff = 2;
    int xOff = 2;
    if (current<redLine) {
        //painter.clearBox(screenx/2+25,screeny/2+29-32,screenx/2,33);
        u8g2.drawStr(xStart+width+xOff,yStart+height-yOff,"0");
        u8g2.drawStr(xStart+width+newFontx+xOff,yStart+height-yOff,String(current/1000).c_str());
    } else {
        u8g2.drawStr(xStart+width+xOff,yStart+height-yOff,String(current/1000).c_str());
        if(shiftText){
        u8g2.setFont(u8g2_font_logisoso32_tf);
        u8g2.drawStr(screenx/2+25,screeny/2+29,"SHIFT");
        }
        u8g2.setFont(u8g2_font_logisoso18_tf);
    }
    u8g2.drawStr(xStart+width+2*newFontx+xOff,yStart+height-yOff,".");
    u8g2.drawStr(xStart+width+3*newFontx+xOff-6,yStart+height-yOff,String((current%1000)/100).c_str());
    //u8g2.sendBuffer();
 }
};


class button
{
    public:
    int x0 = 0;
    int y0 = 0;
    int height = 0;
    int width = 0;
    const uint8_t* textFont = u8g2_font_VCR_OSD_mf; // note const uint8_t* is a pointer to a variable of const uint_8t and can still be modified
    char message[32] = "DEFAULT";
    bool useTopCordinate = false;

    private:
    bool isActive = false;
    bool actionAssigned = false; // use this to avoid performing actions that wernt asigned.
    int text_y0 = 0; // top of the text message
    int box_y0 = 0;
    int box_y1 = 0;
    int box_x0 = 0;
    int box_x1 = 0;
    
    void (*actionFunction)(); // use this to point to the function we want called when the button is pressed

    // possible additions might be to shade the button if it is being pressed

    public:
    void setText(const char* myMessage)
    {
        strcpy(message,myMessage);
    }
    //void initialize
    void initialize()
    {
        // calculate the appropriate values to draw and format the button


        // draw the button
        u8g2.setFont(textFont);
        width = u8g2.getStrWidth(message);
        height = u8g2.getAscent() - u8g2.getDescent(); // the area the digit can occupy

        int text_y0 = y0 - u8g2.getAscent(); // the upper corner of the digit
        if(useTopCordinate)
        {
            text_y0 = y0;
            y0 = y0 + u8g2.getAscent(); // shift the start of the text down by the rise in the text
        }
        // todo there should be a variable to adjust padding or the amount of space on the edge
        int xPadding = 1;
        box_x0 = x0 - xPadding;
        box_x1 = x0 + width + xPadding;

        int yPadding = 2;
        box_y0 = text_y0 - yPadding;
        box_y1 = text_y0 + height + yPadding;
        //draw the button
        u8g2.drawStr(x0,y0,message);
        u8g2.drawFrame(box_x0,box_y0,box_x1 - box_x0,box_y1 - box_y0);

        // assign an action function to the button

    }
    void assignAction(void (*myFunction)())
    {
        actionFunction = myFunction;
        actionAssigned = true;
    }
    void draw()
    {
        painter.clearBox(box_x0,box_y0,box_x1 - box_x0,box_y1 - box_y0);
        u8g2.setFont(textFont);
        u8g2.drawStr(x0,y0,message);
        u8g2.drawFrame(box_x0,box_y0,box_x1 - box_x0,box_y1 - box_y0);
    }

    void fillButton()
    {
        u8g2.setFontMode(1);
        u8g2.setDrawColor(1);
        u8g2.drawBox(box_x0,box_y0,box_x1 - box_x0,box_y1 - box_y0);
        u8g2.setDrawColor(2); // make sure the message will stay visable
        u8g2.setFont(textFont);
        u8g2.drawStr(x0,y0,message);
        u8g2.setDrawColor(1);
        u8g2.setFontMode(0);
    }
    
    void read() // maybe call this read
    {
        if((tap.x >= box_x0) && (tap.x <= box_x1) && (tap.y >= box_y0) && (tap.y <= box_y1)) {
        // check to see if the tap is inside the button box
            if(tap.isPressed()){ // if the user is pressing the button
            //if(tap.isAreaPressed(((box_x0 + box_x1)/2), ((box_y0 + box_y1)/2),40,20)){
                    fillButton();
                //Serial.println("pressed");
                updateRequest = true;
            }
            if(tap.isTapped()){ // if the user tapped the button
             //if(tap.isAreaTapped(((box_x0 + box_x1)/2), ((box_y0 + box_y1)/2),40,20)){ // checking area taped should be more accurate but it is in gamma stage
                Serial.println("we got it!");
                draw();
                updateRequest = true;
                if(actionAssigned){ // make sure there is an action assigned before runing anything
                    actionFunction(); // Run the function that we previously associated with the button
                }
            }
        }
        else{
            if(tap.isPressed()){ // if we are outside the button box but still pressing the screen
            draw();
            }
        }

    }

};


void changeLogging_state()
{
    loggingActive = !loggingActive;
}

// objects used for menu screen
button M3ScreenButton;
button scRacingScreenButton;
button insightScreenButton;
void initializeInsightScreen(); // need function declaration before being called in menu screen
void initializeMenuScreen()
{
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_VCR_OSD_mf); //u8g2_font_9x15B_mr
    u8g2.drawStr(32,18,"Menu");
    u8g2.drawLine(0,19,240,19);
    ///u8g2.setFontMode(0); // shoulnt need to change this as it is the default
    M3ScreenButton.x0 = 10;
    M3ScreenButton.y0 = 40;
    M3ScreenButton.setText("M3 Screen");
    M3ScreenButton.initialize();
    M3ScreenButton.assignAction(&initializeEaganM3_Screen);
    insightScreenButton.x0 = 10;
    insightScreenButton.y0 = 70;
    insightScreenButton.setText("Insight Screen");
    insightScreenButton.initialize();
    insightScreenButton.assignAction(&initializeInsightScreen);
    //insightScreenButton.assignAction(&initializeEaganInsightScreen);
    scRacingScreenButton.x0 = 10;
    scRacingScreenButton.y0 = 100;
    scRacingScreenButton.setText("USC Racing Screen");
    scRacingScreenButton.initialize();
    // u8g2.setDrawColor(0); // set this to zero so that we get a black background against the font
    // u8g2.drawStr(0,40,"Device Settings");
    // u8g2.drawStr(0,60,"Option 2");
    // u8g2.drawStr(0,80,"Acceleration Tests");
    u8g2.sendBuffer();
    // u8g2.setDrawColor(1); // reset the draw color back to the default
    screenPointer = &menuScreen; // switch to the menu screen from now on
}

void menuScreen()
{
    tap.detect();
    M3ScreenButton.read();
    insightScreenButton.read();
    scRacingScreenButton.read();
}

//objects used for M3 screen
boxGauge tachometer;
digitalGauge speed;
digitalGauge xAcel;
digitalGauge yAcel;
digitalGauge lat;
digitalGauge lon;
statusMessage GPS_status;
statusMessage loggingStatusMessage;
statusMessage date;
button logButton;
button menuButton;
void initializeEaganM3_Screen()
{
    u8g2.clearBuffer();
    rpmPerPulse = 20; // the M3 uses 20 pulses per RPM
    GPS_status.y0 = 8;
    GPS_status.initialize("GPS: ","Disconnected");
    loggingStatusMessage.y0 = 8;
    loggingStatusMessage.x0 = GPS_status.xEnd() + 8;
    loggingStatusMessage.initialize("LOG:","sdErr",2);
    date.y0 = 8;
    date.x0 = loggingStatusMessage.xEnd() + 5;
    date.initialize("",constructDateTime(3).c_str());
    //boxGauge tachometer;
    tachometer.yStart = 12;
    tachometer.redLine = 7500;
    tachometer.max = 8500;
    tachometer.cutoff = 2000;
    tachometer.height = 25;
    //u8g2.print("gps not functional");
    tachometer.drawBoxGauge(engRPM);
    //digitalGauge speed;
    speed.y0 = 84;
    strcpy(speed.printFormat,"%3.0f\0");
    speed.initializeLargeGauge(88);
    //digitalGauge xAcel;
    // xAcel.maxVal = -99.0;
    // xAcel.y0 = 84;
    // xAcel.x0 = speed.xEnd();
    // //xAcel.y0 = 120; 
    // strcpy(xAcel.unitText,"xgs\0");
    // strcpy(xAcel.printFormat,"%+2.0f\0");
    // xAcel.unitFont = u8g2_font_t0_12_tf;
    xAcel.unitLocation(1,10);
    xAcel.initializeLargeGauge(speed.xEnd(),84,-99,"%+2.0f","xgs",u8g2_font_t0_12_tf);
    //digitalGauge yAcel;
    // yAcel.maxVal = -99;
    // strcpy(yAcel.printFormat,"%+2.0f\0");
    // yAcel.y0 = 84;
    // yAcel.x0 = xAcel.xEnd();
    // strcpy(yAcel.unitText,"ygs\0");
    // yAcel.unitFont = u8g2_font_t0_12_tf;
    yAcel.unitLocation(1,10);
    yAcel.unitLocation(1,10);
    yAcel.initializeLargeGauge(xAcel.xEnd(),84,-99,"%+2.0f","ygs",u8g2_font_t0_12_tf);
    lat.y0 = speed.yEndBottom() + 15;
    lat.digitFont = u8g2_font_6x12_mn;
    lat.unitFont = u8g2_font_5x7_tr;
    strcpy(lat.unitText,"*\0");
    strcpy(lat.printFormat,"%6.4f\0"); // google maps uses 6 digits (.6f)
    lat.maxVal = -100.1234;
    lat.initialize(-100.1234);
    lon.y0 = speed.yEndBottom() + 15;
    lon.digitFont = u8g2_font_6x12_mn;
    lon.unitFont = u8g2_font_5x7_tr;
    lon.x0 = lat.xEnd() + 30;
    strcpy(lon.unitText,"*\0");
    strcpy(lon.printFormat,"%6.4f\0");
    lon.maxVal = -100.1234;
    lon.initialize(-100.1234);
    logButton.x0 = 190;
    logButton.y0 = 120;
    logButton.setText("LOG");
    logButton.initialize();
    logButton.assignAction(&changeLogging_state);
    menuButton.x0 = 20;
    menuButton.y0 = 120;
    menuButton.setText("MENU");
    menuButton.initialize();
    menuButton.assignAction(&initializeMenuScreen);
    u8g2.sendBuffer();
    screenPointer = &EaganM3_Screen; // change the screen pointer to display the M3screen now
}

void EaganM3_Screen()
{
    // updateRequest = false; // now in the main display screen function
    // tap.detect();
    logButton.read();
    tachometer.display(engRPM);
    GPS_status.displayGPS_status();
    loggingStatusMessage.displayLog_status();
    date.displayDate();
    speed.display(gpsSpeed);
    xAcel.display(xAccel/10); // display acceleration in 10ths of a G
    yAcel.display(yAccel/10);
    lat.display(latitude);
    lon.display(longitude);
    menuButton.read(); // note if this is placed earlier when pressed the menu screen will load and then the m3 screen function will run where it left off
    
    //speed.updateArea();
}

// additional objects used by the insight screen
digitalGauge oilPressGauge;
digitalGauge AFR;
digitalGauge TPSval;
digitalGauge MAPval;
digitalGauge turbinePressureGauge;
digitalGauge knock;
digitalGauge fuelPressureGauge;
digitalGauge BatteryCurrent;
digitalGauge BatteryVoltage;

void insightScreen()
{
    // updateRequest = false; // now in the main display screen function
    // tap.detect();
    logButton.read();
    tachometer.display(engRPM);
    GPS_status.displayGPS_status();
    loggingStatusMessage.displayLog_status();
    date.displayDate();
    speed.display(gpsSpeed);
    AFR.display(AirFuelRatio/100.0);
    knock.display(knockValue/10.0);
    oilPressGauge.display(oilPressure);
    TPSval.display(throttlePosition);
    MAPval.display(MAP);
    turbinePressureGauge.display(turbinePressure);
    fuelPressureGauge.display(fuelPressure);
    BatteryCurrent.display((hybridBatteryCurrent/100));
    BatteryVoltage.display(hybridBatteryVoltage);
    xAcel.display(xAccel/10); // display acceleration in 10ths of a G
    yAcel.display(yAccel/10);
    //lat.display(latitude);
    //lon.display(longitude);
    menuButton.read(); // note if this is placed earlier when pressed the menu screen will load and then the current screen function will run where it left off
    
    //speed.updateArea();
}
void initializeInsightScreen()
{
    u8g2.clearBuffer();
    rpmPerPulse = 40; // the insight uses 36 pulses per RPM
    GPS_status.y0 = 8;
    GPS_status.initialize("GPS: ","Disconnected");
    loggingStatusMessage.y0 = 8;
    loggingStatusMessage.x0 = GPS_status.xEnd() + 8;
    loggingStatusMessage.initialize("LOG:","sdErr",2);
    date.y0 = 8;
    date.x0 = loggingStatusMessage.xEnd() + 5;
    date.initialize("",constructDateTime(3).c_str());
    tachometer.yStart = 12;
    tachometer.redLine = 5800;
    tachometer.max = 6500;
    tachometer.cutoff = 2000;
    tachometer.height = 15;
    tachometer.drawBoxGauge(engRPM);
    //strcpy(speed.printFormat,"%3.0f\0");
    speed.y0 = 74;
    speed.initializeLargeGauge(88);
    AFR.initializeMediumGauge(speed.xEnd()+5,60,22.0,"%3.1f","AFR");
    knock.initializeMediumGauge(speed.xEnd()+5,78,22.0,"%3.1f","KNK");
    xAcel.initializeMediumGauge(190,50,-99,"%+2.0f","xg");
    yAcel.initializeMediumGauge(190,75,-99,"%+2.0f","yg");
    oilPressGauge.initializeMediumGauge(0,92,99,"%2.0f","psi");
    TPSval.initializeMediumGauge(oilPressGauge.xEnd()+5,92,101,"%2.0f","%");
    MAPval.initializeMediumGauge(speed.xEnd()+15,96,300,"%2.0f","kPa");
    turbinePressureGauge.initializeMediumGauge(MAPval.xEnd()+5,96,15,"%3.0f","psi");
    fuelPressureGauge.initializeMediumGauge(speed.xEnd()+15,114,200,"%3.0f","psi");
    BatteryCurrent.initializeMediumGauge(0,110,-140,"%+2.0f","A");
    BatteryVoltage.initializeMediumGauge(BatteryCurrent.xEnd()+5,110,200,"%2.0f","V");
    logButton.x0 = 200;
    logButton.y0 = 127;
    logButton.setText("LOG");
    logButton.initialize();
    logButton.assignAction(&changeLogging_state);
    menuButton.x0 = 1;
    menuButton.y0 = 127;
    menuButton.setText("MENU");
    menuButton.initialize();
    menuButton.assignAction(&initializeMenuScreen);
    u8g2.sendBuffer();
    screenPointer = &insightScreen; // change the screen pointer to display the insightscreen now
}

class myInsightScreen
{
    private:
        bool initialized = false; // track if the screen has been initialized
        boxGauge tachometer;
        digitalGauge speed;
        digitalGauge xAcel;
        digitalGauge yAcel;
        digitalGauge lat;
        digitalGauge lon;
        digitalGauge oilPressGauge;
        digitalGauge AFR;
        //digitalGauge TPSval{99,0,100}; // must use braces otherwise treats as a function
        statusMessage GPS_status;
        statusMessage loggingStatusMessage;
        statusMessage date;
        button logButton;
        //button menuButton;
    public:
        void initialize()
        {
            u8g2.clearBuffer();
            GPS_status.y0 = 8;
            GPS_status.initialize("GPS: ","Disconnected");
            loggingStatusMessage.y0 = 8;
            loggingStatusMessage.x0 = GPS_status.xEnd() + 8;
            loggingStatusMessage.offsetStatus(2);
            loggingStatusMessage.initialize("LOG:","sdErr");
            date.y0 = 8;
            date.x0 = loggingStatusMessage.xEnd() + 5;
            date.initialize("",constructDateTime(3).c_str());
            tachometer.yStart = 12;
            tachometer.redLine = 5800;
            tachometer.max = 6500;
            tachometer.cutoff = 2000;
            tachometer.drawBoxGauge(1000);
            strcpy(speed.printFormat,"%3.0f\0");
            speed.initialize(88);
            xAcel.maxVal = -99.0;
            xAcel.x0 = speed.xEnd();
            strcpy(xAcel.unitText,"xgs\0");
            strcpy(xAcel.printFormat,"%+2.0f\0");
            xAcel.unitFont = u8g2_font_t0_12_tf;
            xAcel.unitLocation(1,10);
            xAcel.initialize(99);
            yAcel.maxVal = -99;
            strcpy(yAcel.printFormat,"%+2.0f\0");
            yAcel.x0 = xAcel.xEnd();
            strcpy(yAcel.unitText,"ygs\0");
            yAcel.unitFont = u8g2_font_t0_12_tf;
            yAcel.unitLocation(1,10);
            yAcel.initialize(99);
            lat.y0 = speed.yEndBottom() + 15;
            lat.digitFont = u8g2_font_6x12_mn;
            lat.unitFont = u8g2_font_5x7_tr;
            strcpy(lat.unitText,"*\0");
            strcpy(lat.printFormat,"%6.4f\0"); // google maps uses 6 digits (.6f)
            lat.maxVal = -100.1234;
            lat.initialize(-100.1234);
            lon.y0 = speed.yEndBottom() + 15;
            lon.digitFont = u8g2_font_6x12_mn;
            lon.unitFont = u8g2_font_5x7_tr;
            lon.x0 = lat.xEnd() + 30;
            strcpy(lon.unitText,"*\0");
            strcpy(lon.printFormat,"%6.4f\0");
            lon.maxVal = -100.1234;
            lon.initialize(-100.1234);
            logButton.x0 = 190;
            logButton.y0 = 120;
            logButton.setText("LOG");
            logButton.initialize();
            logButton.assignAction(&changeLogging_state);
            // menuButton.x0 = 20;
            // menuButton.y0 = 120;
            // menuButton.setText("MENU");
            // menuButton.initalize();
            u8g2.sendBuffer();
            initialized = true;
        }
        
        void display()
        {
            if(initialized) // check if we have been initialized
            {
                updateRequest = false;
                tap.detect();
                logButton.read();
                // menuButton.read();
                tachometer.display(engRPM);
                GPS_status.displayGPS_status();
                loggingStatusMessage.displayLog_status();
                date.displayDate();
                speed.display(gpsSpeed);
                xAcel.display(xAccel/10); // display acceleration in 10ths of a G
                yAcel.display(yAccel/10);
                lat.display(latitude);
                lon.display(longitude);
                TPSval.display(10);
                
                
                //speed.updateArea();
                if(updateRequest || millis() - lastDisplayUpdate > 500) // update the display atleast twice per second
                {
                    u8g2.sendBuffer();
                    lastDisplayUpdate = millis();
                }
            }
            else // initialize the display
            {
                initialize();
            }
        }

};
