#include "io.h"
#include "globalData.h"


// int *ecuData[] = 
// {&vehicleSpeed,&engRPM,
// &throttlePosition,&ecuMAP, // to do make this more modular since variables passed change
// &fuelInjectionPulseWidth,&AirFuelRatio,
// &intakeAirTemp,&knockValue};
#define ecuNUMprams 8
const int oilPressSensorPin = A18;

float OilPressureConvert(int ADCval);
float FuelPressureConvert(int ADCval);
float TurbinePressureConvert(int ADCval);
float MAPConvert(int ADCval);
float MAP2Boost(float myMAP);
float getFuelEnrichment(float boostPressure, float fuelPressure);
float getFmuGain(float boostPressure, float fuelPressure);

void initializeIO()
{
    pinMode(tachPin,INPUT);
    pinMode(tpsPin,INPUT);
    pinMode(oilPressSensorPin,INPUT);
    Serial1.begin(9600);
}

void readIO()
{
    oilPressure = OilPressureConvert(analogRead(oilPressSensorPin));
}
float OilPressureConvert(int ADCval)
{
  // this is for the 100PSI sensor
  if(ADCval > 1015)
  return(999);
  if (ADCval <= 101)
  {
    return (0);
  }
  float OilPressure = (.1272 * ADCval) - 12.068;
  return OilPressure;
}

float FuelPressureConvert(int ADCval)
{
  // this is for the 200PSI sensor
  if(ADCval > 1015)
  return(999);
  if (ADCval <= 101)
  {
    return (0);
  }
  float FuelPressure = (.2481 * ADCval) - 23.196;
  return FuelPressure;
}

float TurbinePressureConvert(int ADCval)
{
  // this is for the 30PSI sensor
  if(ADCval > 1015)
  return(999);
  if (ADCval <= 101)
  {
    return (0);
  }
  float TurbinePressure = (.0376 * ADCval) - 2.781;
  return TurbinePressure;
}

float MAPConvert(int ADCval)
{
  if(ADCval > 1015)
  {
  return(999);
  }
  float MAP = (ADCval + 3.611) / 3.253;
  return MAP;
}

float MAP2Boost(float myMAP)

{
  float boost = ((myMAP - 101.325) / 101.325) * 14.7;
  return boost;
}

float getFuelEnrichment(float boostPressure, float fuelPressure)
{
  float fuelEnrichment = (.1490712 * sqrt(fuelPressure - boostPressure)) 
  / ((boostPressure + 14.7) / 14.7);
  return fuelEnrichment;
}

float getFmuGain(float boostPressure, float fuelPressure)
{
  if (boostPressure > 0)
  {
    float fmuGain = ((fuelPressure - 45) / boostPressure) - 1;
    fmuGain = constrain(fmuGain,0,50);
    return fmuGain;
  }
  else
  {
    return 0;
  }
}

void extractSerialData()
{
  #define maxBufferSz 120
  #define maxDataElements 16
  //int *ecuData[] = {&vehicleSpeed,&engRPM,&throttlePosition,&ecuMAP,&fuelInjectionPulseWidth,&AirFuelRatio,&intakeAirTemp,&knockValue};
  if ((Serial1.available() > 40) && (Serial1.available() < 120))
  {
    char readBuffer[maxBufferSz + 1]; // store the information here and leave room for a termination char
    uint8_t bufferSize = Serial1.readBytesUntil('\r',readBuffer,maxBufferSz); // keep track of the buffer size for later
    readBuffer[bufferSize] = '\0'; // terminate the used portion of the read buffer with a null
    uint8_t bufferIndex = 0;
    uint8_t dataIndex = 0;
    int extractedData[maxDataElements];
    //Serial.println(readBuffer);
    while(bufferSize > bufferIndex)
    {
      if(readBuffer[bufferIndex] == ',') // if we find a comma
      {
        if (readBuffer[bufferIndex + 1] == ' ' || readBuffer[bufferIndex + 1] == '+' || readBuffer[bufferIndex + 1] == '-') 
        // and the next charactor is a space or a plus or a minus
        {
          // prepare an array of digits in anticipation for the number that may follow
          char myDigits[10] = "         ";
          myDigits[0] = readBuffer[bufferIndex + 1]; // store the preceding sign into the first place in my digits
          uint8_t digitIndex = 1;
           if(readBuffer[bufferIndex + 2] == ',')
           // and the charactor after that is another comma
           {
              bufferIndex += 3; // add 3 to the buffer index since we've alreadly looked ahead 2 charactors
              // at this point we have matched to our pattern
              while (isdigit(readBuffer[bufferIndex])) 
              // now start reading data until we hit another comma or some non numeric data
              {
                myDigits[digitIndex] = readBuffer[bufferIndex]; // copy the digit in the buffer to one of mydigits
                digitIndex++;
                bufferIndex++; // move on to the next digit in the buffer
              }
              if(digitIndex > 1) // check to make sure it actually captured some data before saving anything
              {
                extractedData[dataIndex] = atoi(myDigits); //convert my string of digits to a real number
                dataIndex++; // advance to the next piece of data
              }
              continue; // go to the next interation of the main loop since we've already found the next comma
              // and the current buffer index is now returning a non numeric charactor
           }
        }
      }
      // we didn't find a match to our pattern so... 
      bufferIndex++; // move on to the next charactor in the buffer
    }
    if(dataIndex == ecuNUMprams) 
    // if the number of data blocks we extracted excatly matches the number of paramaters we expect
    {
      // we've got good data so now save it to the appropriate variables
      for(dataIndex = 0; dataIndex < ecuNUMprams; dataIndex++ )
      {
        //*ecuData[dataIndex] = extractedData[dataIndex];
        // *ecuData points to the individual ecu paramater variables
        // in the exact order they are relayed over serial
        // so simply copy these extracted data blocks sequentially to the variables refrenced in ecuData
        //Serial.println(*ecuData[dataIndex]); // print the extracted data to the console for debugging
      }
    }
    //writeData();
  }
}