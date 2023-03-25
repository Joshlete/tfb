// Elektor UNO R4 @1mhz (now @8mhz for digital temp probes)
// AVR Arduino 1.8.9
//   DallasTemperature 3.8.0
//   OneWire 2.3.5
// After install on a new ENV run this to make it build...
// git restore .pio/libdeps/ATmega328PB/FastLED/src/platforms/avr/fastpin_avr.h

// get d to around 404000, 403 - 404

// We call this a ProPod (TFB)
// 500 to 511 are 1v21 hand soldered resistor
// 512 to 531 (547-549,576,578) are 1v21a China soldered resistor (I think?)
// 532 to 575 plus 577 are 1v21b New LDO and resistor (a few A revs mixed in?)
// Special v22's 362 366 404 411 436 440 445 446 454
// 579 to 614, 630 to 672 are 1v22
// 615 to 625 have special JETPOD mode enabled (615 to 629 reserved)
// 674 to 699 will have on board LED (673 is free)
// PROPODS: 700 to 732
// JETPODS: 733 to 767
// PROPODS: 768 to 791
// 768, 776 - 791, have osccal, needs calibration
// 792 - 796, testing PB variant, osccal reads ~399 - ~400
// 797 - 868, JP, oscall reads ~397 - ~401
// PROPODS: 869 to 921
// next available ID: 922
#define UNIT_ID_IN 921
#define FIRMWARE_ID_IN 52
// #define JETPOD_MODE
// #define NEW_OSCCAL 0xA9
// #define NEW_OSCCAL 0xB2
// #define NEW_OSCCAL 0x98
// #define NEW_OSCCAL 0x43
// FW 39 added police mode on boot for temp probe errors
// FW 40 is for China made 1v21 boards with no PCT and WS2812B LED
// FW 41 changed battery dead voltage to 10v
// FW 42 more DS probe resets
// FW 43 ice fan limit set to 3500
// FW 44 JETPOD mode option and Ice Fan speed rewrite
// FW 45 Onboard LED mirrors external LED
// FW 46 Changes to IceFan speed tracking
// FW 47 More IceFan speed changes and slow down dead<->charging state changes
// FW 48 More graceful recovery from dead
// FW 49 OSCCAL option added
// FW 50 changed battery dead voltage to 11.2v
// FW 51 JETPOD mode PTC cool down time added
// FW 52 updated smart unit reporting ice fan to tft, slowed down stuck reporting 1 in 5

#define ECHO_DEBUG_COMMENTS
#define ECHO_DEBUG_COMMENTS1
//#define ECHO_DEBUG_COMMENTS2
//#define ECHO_DEBUG_COMMENTS3
//#define USE_PCT2075_PROBE_AMBIENT

#include <Arduino.h>
#include <avr/wdt.h>
#include <SPI.h>
#include <EEPROM.h>
#include <FastLED.h>
#include "main.h"

#define LED_WS 21
#define LED_WS2 20
#define FAN_MAIN 10
#define FAN_ICE 5
#define FAN_ICE_ALT 9
#define CHARGE_ENABLE 7
#define FAN_ICE_RPM 8
#define FAN_ICE_PWM 6
#define BAT_VOLTS A7
#define SAFE_VOLTS A6
#define CHARGER_VOLTS A1
#define TEMP_EXIT A3
#define TEMP_TOP A0
#define TEMP_RETURN A2
#define SPI_FLASH 2

#ifdef USE_PCT2075_PROBE_AMBIENT
#include <Wire.h>
#define PCT2075_ADDRESS 0x48
#endif

#include <OneWire.h> 
#include <DallasTemperature.h>
#define DS_PROBES_BAT 5
#define DS_PROBES_AMB 6
OneWire wireBat(DS_PROBES_BAT);
OneWire wireAmb(DS_PROBES_AMB);
DallasTemperature dsBattery(&wireBat);
DallasTemperature dsAmbient(&wireAmb);
DeviceAddress addr;

int VOLT_OFFSET=0;
unsigned int VOLT_SCALER=1945;
unsigned int VOLT_SCALER2=1950;
const int EEPROM_VOLT_OFFSET=2;
const int EEPROM_VOLT_OFFSET2=3;
const unsigned int UNIT_ID=UNIT_ID_IN;
const byte FIRMWARE_ID=FIRMWARE_ID_IN;
unsigned int TEMP_TARGET;
const int EEPROM_TEMP_TARGET=1;
const int EEPROM_OSCCAL_NEW=4;
const unsigned int BATTERY_FULL=14300;
const unsigned int BATTERY_SHIPABLE=13500;
const unsigned int BATTERY_DEAD=11200;
unsigned int BATTERY_CHANGING=14000;

// I think these are for either a 3435 or a 3470
// beta=3470
static const PROGMEM unsigned int degFMap20k[]={15362,15300,15238,15176,15116,15056,14996,14937,14879,14821,14764,14707,14651,14595,14540,14485,14431,14377,14324,14271,14219,14167,14116,14065,14014,13964,13914,13864,13815,13767,13719,13671,13623,13576,13529,13483,13437,13391,13345,13300,13256,13211,13167,13123,13080,13036,12994,12951,12909,12866,12825,12783,12742,12701,12660,12620,12580,12540,12500,12461,12422,12383,12344,12305,12267,12229,12191,12154,12116,12079,12042,12006,11969,11933,11897,11861,11825,11790,11755,11719,11684,11650,11615,11581,11547,11513,11479,11445,11412,11378,11345,11312,11279,11247,11214,11182,11150,11118,11086,11054,11023,10991,10960,10929,10898,10867,10836,10806,10775,10745,10715,10685,10655,10625,10596,10566,10537,10507,10478,10449,10420,10392,10363,10335,10306,10278,10250,10222,10194,10166,10138,10110,10083,10056,10028,10001,9974,9947,9920,9893,9867,9840,9813,9787,9761,9735,9708,9682,9657,9631,9605,9579,9554,9528,9503,9478,9452,9427,9402,9377,9352,9328,9303,9278,9254,9229,9205,9181,9156,9132,9108,9084,9060,9036,9013,8989,8965,8942,8918,8895,8871,8848,8825,8802,8779,8756,8733,8710,8687,8664,8642,8619,8596,8574,8551,8529,8507,8485,8462,8440,8418,8396,8374,8352,8330,8309,8287,8265,8244,8222,8201,8179,8158,8136,8115,8094,8073,8052,8031,8010,7989,7968,7947,7926,7905,7884,7864,7843,7823,7802,7781,7761,7741,7720,7700,7680,7660,7639,7619,7599,7579,7559,7539,7519,7499,7480,7460,7440,7420,7401,7381,7361,7342,7322,7303,7283,7264,7245,7225,7206,7187,7168,7149,7129,7110,7091,7072,7053,7034,7015,6996,6978,6959,6940,6921,6903,6884,6865,6847,6828,6810,6791,6773,6754,6736,6717,6699,6681,6662,6644,6626,6607,6589,6571,6553,6535,6517,6499,6481,6463,6445,6427,6409,6391,6373,6355,6338,6320,6302,6284,6267,6249,6231,6214,6196,6179,6161,6144,6126,6109,6091,6074,6056,6039,6022,6004,5987,5970,5952,5935,5918,5901,5883,5866,5849,5832,5815,5798,5781,5764,5747,5730,5713,5696,5679,5662,5645,5628,5611,5594,5578,5561,5544,5527,5510,5494,5477,5460,5444,5427,5410,5394,5377,5360,5344,5327,5311,5294,5278,5261,5244,5228,5212,5195,5179,5162,5146,5129,5113,5097,5080,5064,5048,5031,5015,4999,4982,4966,4950,4934,4917,4901,4885,4869,4853,4836,4820,4804,4788,4772,4756,4740,4724,4708,4691,4675,4659,4643,4627,4611,4595,4579,4563,4547,4531,4515,4499,4483,4467,4452,4436,4420,4404,4388,4372,4356,4340,4324,4309,4293,4277,4261,4245,4229,4214,4198,4182,4166,4150,4135,4119,4103,4087,4071,4056,4040,4024,4008,3993,3977,3961,3945,3930,3914,3898,3883,3867,3851,3835,3820,3804,3788,3773,3757,3741,3726,3710,3694,3679,3663,3647,3632,3616,3600,3585,3569,3553,3538,3522,3506,3491,3475,3459,3444,3428,3412,3397,3381,3366,3350,3334,3319,3303,3287,3272,3256,3240,3225,3209,3193,3178,3162,3146,3131,3115,3099,3084,3068,3052,3037,3021,3005,2989,2974,2958,2942,2927,2911,2895,2879,2864,2848,2832,2817,2801,2785,2769,2754,2738,2722,2706,2690,2675,2659,2643,2627,2611,2596,2580,2564,2548,2532,2516,2501,2485,2469,2453,2437,2421,2405,2389,2373,2357,2341,2325,2310,2294,2278,2262,2246,2230,2213,2197,2181,2165,2149,2133,2117,2101,2085,2069,2053,2036,2020,2004,1988,1972,1956,1939,1923,1907,1891,1874,1858,1842,1825,1809,1793,1776,1760,1743,1727,1711,1694,1678,1661,1645,1628,1612,1595,1579,1562,1545,1529,1512,1495,1479,1462,1445,1429,1412,1395,1378,1362,1345,1328,1311,1294,1277,1260,1243,1226,1209,1192,1175,1158,1141,1124,1107,1090,1073,1055,1038,1021,1004,986,969,952,934,917,899,882,864,847,829,812,794,777,759,741,724,706,688,670,652,634,617,599,581,563,545,527,508,490,472,454,436,418,399,381,362,344,326,307,289,270,251,233,214,195,177,158,139,120,101,82,63,44,25,6,0,0};
const unsigned long FLASH_MAX_SIZE=262144;

unsigned long batteryVoltsSum5=0,iceFanDutySum5=0,mainFanDutySum5=0,tempReturnSum5=0,tempExitSum5=0,tempTopSum5=0,tempBatSum5=0;
unsigned long batteryVoltsSum1=0,iceFanDutySum1=0,tempReturnSum1=0,tempExitSum1=0,tempTopSum1=0,tempBatSum1=0;
unsigned int batteryVoltsAve5=0,iceFanDutyAve5=0,mainFanDutyAve5=0,tempReturnAve5=0,tempExitAve5=0,tempTopAve5=0,tempBatAve5=0;
unsigned int batteryVoltsAve1=0,iceFanDutyAve1=0,tempReturnAve1=0,tempExitAve1=0,tempTopAve1=0,tempBatAve1=0;
unsigned int batteryVolts,iceFanDuty,mainFanDuty,tempReturn,tempExit,tempTop,tempBat;
unsigned int tempRetDigital,tempBatDigital;
byte dsBatteryAvailable=0,dsAmbientAvailable=0,dsBatteryAvailableLast=0,dsAmbientAvailableLast=0;
unsigned long lastSavedDS=100000000;
unsigned int counterAve1=0,counterAve5=0,batteryVoltsLastWrite=0;
unsigned int batteryVoltsHistory[25],tempBatHistory[25];
byte chargerHistory[25],unitHistory[25],sample;
unsigned long tftRead=0;
unsigned int tftID=0;
byte tftF1=0,tftF2=0,tftF3=0,tftF4=0,tftF5=0,tftStep=0,tftCRC;

byte exitTempLostCount=0, iceFanReadReady=0;

unsigned long chargerVoltsSum5=0,chargerVoltsSum1=0;
unsigned int chargerVoltsAve5=0,chargerVoltsAve1=0,chargerVolts;
unsigned long safeVoltsSum5=0,safeVoltsSum1=0;
unsigned int safeVoltsAve5=0,safeVoltsAve1=0,safeVolts;

//        0=idle  1=running  2=dead  3=charging
byte unitState=1, unitStateLast=255, failCount=0, ledState=0, ledFlash, liveData=0, eraseConfirm=0, voffsetpre=0, spibuf[16];
CRGB ledStatus[1],ledStatus2[1];
byte targetConfirm=0, activateConfirm=0, zeroConfirm=0, relogChange=0;
byte readJump,fasterFlash;
unsigned int temp_build;
unsigned long timeLastIncrement,timeLastAverages1,timeLastAverages5,timeLastFlash,timeStartRun,timeLastFlashWrite;
unsigned long timeLastDeadFlip,timeLastDeadFlipPrint,timeChargeEnableDelay;
unsigned long rpmSTime,rpmSTimeLast,rpmLast=0,rpmLastMax=0,rpmLastMaxTime=0,timeRPM,timeRPMwritten,timeHotBat;
unsigned long rpmLastReal=0, rpmLastRealTime=0;
byte rpmLastRealStuck=0;
byte rpmState,rpmStateLast,rpmLoops,rpmWaits;

unsigned long flashOffsetWrite=0,flashOffsetRead=0;
SPISettings flash(500000,MSBFIRST,SPI_MODE0);

unsigned long epochMillis, epochTime, epochInput;
byte epochType, epochRead;

byte firstIceFanSpeed=0;
byte iceFanLimitCounter;
#define ICE_FAN_LIMIT_COUNT 30

unsigned int intelTopID=0;            // Top ID if available
unsigned int intelTopIDWritten=65535; // Top ID last written
unsigned int intelTopIDWorking;       // Top ID builder
byte intelTopUnit=0;                  // 0=searching, 1=found top, 2=lost top
byte intelTopUnitFan=0;               // last ice fan request
byte intelTopDecode=0;                // last ice fan request
unsigned long intelTopTimeCheck=0;    // last time ID was sent (every 10 seconds?)
unsigned long intelTopTimeReplied=0;  // last time TFT replied
unsigned long intelTopTimeWritten=86400000; // last time TFT id written
// Every 10s look for top unit by sending packet: ii{UNIT_ID}I{rpmLast}I{TEMP_TARGET}II
//  ex: ii125I3250I3800II
// Top unit reply II{TOP_ID}i{ICE_REQUEST}ii)
//                01 2      2 3           34

unsigned long millisIceFanOnTime,millisIceFanMaxTime,millisIceFanSpeed,millisIceFanSpeedLast;
byte iceFanStateLast,iceFanCheckCount;

void TrackIceFanOn(){
  if(iceFanStateLast!=1){
    millisIceFanOnTime=millis();
    iceFanStateLast=1;
    iceFanCheckCount=0;
  }
  if(iceFanReadReady<100){
    iceFanReadReady++;
  }
}
void TrackIceFanOff(){
  if(iceFanStateLast!=0){
    millisIceFanOnTime=millis()-millisIceFanOnTime;
    iceFanStateLast=0;
  }
  iceFanCheckCount=0;
  iceFanReadReady=0;
}
void InitIceFanRPM(){
  millisIceFanOnTime=0;
  millisIceFanMaxTime=0;
  millisIceFanSpeed=0;
  millisIceFanSpeedLast=0;
  iceFanStateLast=0;
  iceFanCheckCount=0;
  iceFanReadReady=0;
}

void setup(){
  cli();
  wdt_reset();
  MCUSR &= ~(1<<WDRF);
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  WDTCSR = 0x00;
  sei();
  Serial.begin(62500);
  analogReference(EXTERNAL);
  pinMode(SPI_FLASH,OUTPUT);
  digitalWrite(SPI_FLASH,HIGH);
  pinMode(FAN_MAIN,OUTPUT);
  digitalWrite(FAN_MAIN,LOW);
  pinMode(FAN_ICE_ALT,OUTPUT);
  digitalWrite(FAN_ICE_ALT,LOW);
  pinMode(CHARGE_ENABLE,OUTPUT);
  digitalWrite(CHARGE_ENABLE,LOW);
  pinMode(FAN_ICE_PWM,INPUT);
  pinMode(FAN_ICE_RPM,INPUT_PULLUP);
  pinMode(LED_WS,OUTPUT);
  pinMode(LED_WS2,OUTPUT);
  FastLED.addLeds<WS2812B, LED_WS, GRB>(ledStatus, 1);
  FastLED.addLeds<WS2812B, LED_WS2, GRB>(ledStatus2, 1);
  OffLED();
  ReadVolts();
 #ifdef NEW_OSCCAL
  if(NEW_OSCCAL != EEPROM.read(EEPROM_OSCCAL_NEW)){
    EEPROM.write(EEPROM_OSCCAL_NEW,NEW_OSCCAL);
  }
  OSCCAL=NEW_OSCCAL;
 #else
  byte osccalSet=EEPROM.read(EEPROM_OSCCAL_NEW);
  if(osccalSet<=185 && osccalSet>=150){
    OSCCAL=osccalSet;
    delay(50);
    Serial.println("-Loaded High OSCCAL");
  }else if(osccalSet<=115 && osccalSet>=90){
    OSCCAL=osccalSet;
    delay(50);
    Serial.println("-Loaded Low OSCCAL");
  }else if(osccalSet>0){
    EEPROM.write(EEPROM_OSCCAL_NEW,0);
    delay(50);
    Serial.println("-OSCCAL cleared");
  }else{
    delay(50);
    Serial.println("-No OSCCAL set");
  }
 #endif
  while(batteryVolts<9000){
    if(batteryVolts<6000){
      BrownLED(); delay(50);
      OffLED();   delay(1150);
    }else{
      BrownLED(); delay(100);
      OffLED();   delay(900);
      RedLED();   delay(100);
      OffLED();   delay(100);
    }
    ReadVolts();
    Serial.print("-id");
    Serial.print(UNIT_ID);
    Serial.print("dead");
    Serial.print(batteryVolts);
    Serial.println("mv");
    delay(100);
    Serial.print("-OSCCALx");
    Serial.println(OSCCAL,HEX);
    delay(100);
    Serial.println("-U1U");
    delay(400);
    Serial.println("-U2U");
    delay(100);
    Serial.print("-OSCCALy");
    Serial.println(EEPROM.read(EEPROM_OSCCAL_NEW),HEX);
    delay(100);
    digitalWrite(CHARGE_ENABLE,HIGH);
  }
  DigitalTempInit();
  #ifdef USE_PCT2075_PROBE_AMBIENT
   Wire.begin();
   byte pct_errs=0;
   Wire.beginTransmission(PCT2075_ADDRESS); 
   Wire.write(0x01);  // PCT2075_CONF
   Wire.write(0);
   pct_errs=Wire.endTransmission();
   Wire.beginTransmission(PCT2075_ADDRESS); 
   Wire.write(0x04);  // PCT2075_TIDLE
   Wire.write(5);  // 500ms
   pct_errs+=Wire.endTransmission();
   Wire.beginTransmission(PCT2075_ADDRESS); 
   Wire.write(0x00);  // PCT2075_TEMP
   pct_errs+=Wire.endTransmission();
   Serial.print("-LM75 ");
   if(pct_errs!=0){
     Serial.print('?');
     safeVolts=8000;
     RedLED();
     delay(500);
     BlueLED();
     delay(500);
     RedLED();
     delay(500);
     BlueLED();
     delay(500);
     RedLED();
     delay(500);
     BlueLED();
     delay(500)
     RedLED();
     delay(500);
     BlueLED();
     delay(500);
     OffLED();
     delay(500);
     safeVolts=0;
   }else{
     ReadTempReturnDigital();
     Serial.print(tempReturn);
   }
   Serial.println();
  #endif
  SPI.begin();
  incrementAverages();
  Serial.println("Booted");
  incrementAverages();
  SPIFlashInit();
  incrementAverages();
  calculateAverages1();
  calculateAverages5();
  PrintAverages1();
  PrintAverages5();
  timeLastIncrement=millis();
  timeLastAverages1=timeLastIncrement;
  timeLastAverages5=timeLastAverages1;
  BATTERY_CHANGING=14000;
  byte temp_read=EEPROM.read(EEPROM_TEMP_TARGET);
  Serial.print(F("-Ttemp "));
  if(temp_read>=100 && temp_read<=199){
    TEMP_TARGET=3200;
    TEMP_TARGET+=(temp_read-100)*10;
    Serial.print(F("loaded "));
    Serial.println(TEMP_TARGET);
  }else{
    TEMP_TARGET=3500;
    Serial.print(temp_read);
    Serial.println('?');
  }
  ReadVCCTemp();
  ReadVCCTemp2();
  sample=0;
  while(sample<24){
    batteryVoltsHistory[sample]=0;
    tempBatHistory[sample]=0;
    chargerHistory[sample]=0;
    unitHistory[sample]=255;
    sample++;
  }
  batteryVoltsAve1=BATTERY_FULL;
  fasterFlash=0;
  failCount=0;
  ReadTempBatDigital();
  ReadTempReturnDigital();
  delay(100);
  DigitalTempsLog();
  IceFanInit();
  EpochStart();
  InitIceFanRPM();
  timeStartRun=millis();
}

void loop(){
  if(millis()-timeLastIncrement>=500){
    EpochTick();
    timeLastIncrement=millis();
    if(timeLastIncrement-rpmLastRealTime>=60000){  // If the RPMs have not been read for a minute assume it is zero...
      rpmLastReal=0;
      rpmLastRealTime=timeLastIncrement;
    }
    incrementAverages();
    
    if(chargerVolts>10000 && chargerVolts>=batteryVolts+200){
      fasterFlash=1;
      if(tempReturnAve1<9500 && tempReturnAve1>2500 && tempBatAve1<9500 && tempBatAve1>2500){
        digitalWrite(CHARGE_ENABLE,HIGH);
        timeChargeEnableDelay=timeLastIncrement;
      }else{
        digitalWrite(CHARGE_ENABLE,LOW);
        if(millis()-timeHotBat>60000){
          timeHotBat=millis();
         #ifdef ECHO_DEBUG_COMMENTS1
          Serial.print(F("-HotBatProtect:"));
          Serial.print(tempReturnAve5);
          Serial.print(',');
          Serial.println(tempBatAve5);
         #endif
        }
      }
      if(unitState==2){
       #ifdef ECHO_DEBUG_COMMENTS
        Serial.println(F("-Bcd: D2C"));  // -Battery charger detected: Dead to Charging
       #endif
        timeLastDeadFlip=millis();
        timeLastAverages5=timeLastDeadFlip-300000;
        unitState=3;
      }
    }else if(unitState==1){
      fasterFlash=0;
      digitalWrite(CHARGE_ENABLE,HIGH);  // Allow discharging...
      timeChargeEnableDelay=timeLastIncrement;
    }else if(unitState==3){
      fasterFlash=0;
      digitalWrite(CHARGE_ENABLE,HIGH);  // Hold MOSFET on till we fall back states
      timeChargeEnableDelay=timeLastIncrement;
    }else{
      fasterFlash=0;
      if(timeLastIncrement-timeChargeEnableDelay>=1000){  // Hold the MOSFET on for one second
        digitalWrite(CHARGE_ENABLE,LOW);
      }
    }
    switch(unitState){
      case 1: // Running
        if(batteryVoltsAve1<BATTERY_DEAD){
          // Battery died, shutdown
         #ifdef ECHO_DEBUG_COMMENTS
          Serial.println(F("-Lb: R2D"));  // -Low battery: Running to Dead
         #endif
          timeLastAverages5=millis()-300000;
          unitState=2;
        }else if(tempExit==0 && (tempTop==0 || tempTop>=10000)){
          failCount++;
          if(failCount>9){
            // 4 samples say the top unit is disconnected, die
           #ifdef ECHO_DEBUG_COMMENTS
            Serial.println(F("-Tfd: R2D"));  // -Top fans disconnected: Running to Dead
           #endif
            timeLastAverages5=millis()-300000;
            unitState=2;
          }
        }else{
          failCount=0;
        }
        if((millis()-timeStartRun)>3000 && (millis()-timeStartRun)<30000){
          if(millisIceFanSpeedLast>3500 && dsAmbientAvailable>0 && dsBatteryAvailable>0){
            ledState=1;
          }else{
            ledState=5;
          }
        }else if(unitState==1 && iceFanStateLast==1 && (millis()-millisIceFanOnTime)>3000){
          if(millisIceFanSpeedLast>3500 && dsAmbientAvailable>0 && dsBatteryAvailable>0){
            ledState=1;
          }else{
            ledState=5;
          }
        }else{
          if(dsAmbientAvailable>0 && dsBatteryAvailable>0){
            ledState=1;
          }else{
            ledState=5;
          }
        }
        if(millis()-tftRead>=300000){
          tftRead=millis()-240000;
          Serial.println('@');
        }
        if(intelTopUnit>0){
          // If we think a smart top unit is connected update every 10 seconds
          if(millis()-intelTopTimeCheck>=10000){
            intelTopTimeCheck=millis();
            Serial.print("ii");
            Serial.print(UNIT_ID);
            Serial.print("I");
            Serial.print(rpmLastReal);
            Serial.print("I");
            Serial.print(TEMP_TARGET);
            Serial.println("II");
          }
        }else if(millis()-timeStartRun<60000){
          // On a new run ping for the first 60 seconds
          if(millis()-intelTopTimeCheck>=5000){
            intelTopTimeCheck=millis();
            Serial.print("ii");
            Serial.print(UNIT_ID);
            Serial.print("I");
            Serial.print(rpmLastReal);
            Serial.print("I");
            Serial.print(TEMP_TARGET);
            Serial.println("II");
          }
        }
        break;
      case 2: // Dead
        failCount=0;
        if(batteryVoltsAve1>BATTERY_CHANGING){
          // Battery voltage is rising so we must be charging
         #ifdef ECHO_DEBUG_COMMENTS
          Serial.println(F("-B+d: D2C"));  // -Battery +delta: Dead to Charging
         #endif
          timeLastDeadFlip=millis();
          timeLastAverages5=timeLastDeadFlip-300000;
          unitState=3;
        }
        if(dsAmbientAvailable>0 && dsBatteryAvailable>0){
          ledState=2;
        }else{
          ledState=5;
        }
        intelTopID=0;
        intelTopUnit=0;
        intelTopUnitFan=0;
        break;
      case 3: // Charging
        failCount=0;
        exitTempLostCount=0;
        if(batteryVoltsAve1>BATTERY_FULL){
          // Long term average says we are full
         #ifdef ECHO_DEBUG_COMMENTS
          Serial.println(F("-Cc: C2I"));  // -Charge complete: Charging to Idle
         #endif
          timeLastAverages5=millis()-300000;
          unitState=0;
        }else if(fasterFlash==0 && batteryVoltsAve1>chargerVolts+100){
          if(millis()-timeLastDeadFlip>=30000){
           #ifdef ECHO_DEBUG_COMMENTS
            Serial.println(F("-Bcd: C2D"));  // -Battery charger disconnected: Charging to Dead
           #endif
            timeLastDeadFlip=millis();
            timeLastAverages5=timeLastDeadFlip-300000;
            unitState=2;
          }else if(millis()-timeLastDeadFlipPrint>10000){
            timeLastDeadFlipPrint=millis();
           #ifdef ECHO_DEBUG_COMMENTS
            Serial.println(F("-Bcd: Scd"));  // -Battery charger disconnected: State change delay...
           #endif
          }
        }
        if(dsAmbientAvailable>0 && dsBatteryAvailable>0){
          ledState=3;
        }else{
          ledState=5;
        }
        intelTopID=0;
        intelTopUnit=0;
        intelTopUnitFan=0;
        break;
      default: // Idle
        failCount=0;
        if(batteryVoltsAve1<BATTERY_SHIPABLE){
         #ifdef ECHO_DEBUG_COMMENTS
          Serial.println(F("-Bns: I2C"));  // -Battery not shipable: Idle to Charging
         #endif
          // Seems we are no longer full
          timeLastDeadFlip=millis();
          timeLastAverages5=timeLastDeadFlip-300000;
          unitState=3;
        }else if(tempExit>0){
          // Top unit was connected, start a new run
          BATTERY_CHANGING=14000;
         #ifdef ECHO_DEBUG_COMMENTS
          Serial.println(F("-Tfc: I2R"));  // -Top fans connected: Idle to Running
         #endif
          timeStartRun=millis();
          timeLastAverages5=timeStartRun-300000;
          unitState=1;
          exitTempLostCount=0;
          firstIceFanSpeed=0;
        }
        if(dsAmbientAvailable>0 && dsBatteryAvailable>0){
          ledState=4;
        }else{
          ledState=5;
        }
        intelTopID=0;
        intelTopUnit=0;
        intelTopUnitFan=0;
        break;
    }
  }
  if(millis()-timeLastAverages1>=5000){
    if(millis()-timeLastAverages1>8000){
      timeLastAverages1=millis();
    }else{
      timeLastAverages1+=5000;
    }
    calculateAverages1();
    if(liveData==1){
      PrintAverages1();
    }
  }
  if(millis()-timeLastAverages5>=300000){
    if(millis()-timeLastAverages5>400000){
      timeLastAverages5=millis();
    }else{
      timeLastAverages5+=300000;
    }
    calculateAverages5();
    byte possible=0;
    sample=24;
    while(sample>1){
      sample--;
      batteryVoltsHistory[sample]=batteryVoltsHistory[sample-1];
      tempBatHistory[sample]=tempBatHistory[sample-1];
      chargerHistory[sample]=chargerHistory[sample-1];
      unitHistory[sample]=unitHistory[sample-1];
    }
    batteryVoltsHistory[0]=batteryVoltsAve5;
    tempBatHistory[0]=tempBatAve5;
    chargerHistory[0]=fasterFlash;
    unitHistory[0]=unitState;
    if(unitHistory[0]>=2 && chargerHistory[0]>0 && batteryVoltsHistory[0]>=14000 && tempBatHistory[0]>=4500){
      // Only check if it is not active or idle
      // Unit must also be charging
      // Battery must be over 14 volts and warm
      possible=1;
    }
    sample=1;
    while(possible>0 && sample<11){
      if(unitHistory[sample]<2){
       #ifdef ECHO_DEBUG_COMMENTS3
        Serial.print(F("-HSf "));  // -History State fail 
        Serial.println(sample);
       #endif
        possible=0;
      }
      if(chargerHistory[sample]==0){
       #ifdef ECHO_DEBUG_COMMENTS3
        Serial.print(F("-HCf "));  // -History Charger fail 
        Serial.println(sample);
       #endif
        possible=0;
      }
      if(batteryVoltsHistory[sample]<=batteryVoltsHistory[0]){
       #ifdef ECHO_DEBUG_COMMENTS3
        Serial.print(F("-HVf "));  // -History Voltage fail 
        Serial.println(sample);
       #endif
        possible=0;
      }
      if(tempBatHistory[sample]+50<=tempBatHistory[sample-1]){
       #ifdef ECHO_DEBUG_COMMENTS3
        Serial.print(F("-HTf "));  // -History Temp fail 
        Serial.println(sample);
       #endif
        possible=0;
      }
      sample++;
    }
    if(possible>0 && batteryVoltsAve1>BATTERY_SHIPABLE){
     #ifdef ECHO_DEBUG_COMMENTS2
      Serial.println(F("-BpfA"));  // -Battery peak found, activating
     #endif
      unitState=0;
    }
    if(liveData==1){
      PrintAverages5();
    }
    if(unitState==1 || relogChange>0 || unitState!=unitStateLast){
      LogFlashStatus5();
      timeLastFlashWrite=millis();
      relogChange=0;
      batteryVoltsLastWrite=batteryVoltsAve5;
    }else if(unitState==0 && (batteryVoltsAve5>=batteryVoltsLastWrite+100 || batteryVoltsAve5+100<=batteryVoltsLastWrite)){
      // Only log state 0's if battery voltage has changed
      LogFlashStatus5();
      timeLastFlashWrite=millis();
      batteryVoltsLastWrite=batteryVoltsAve5;
    }else if(millis()-timeLastFlashWrite>=3600000){
      // Write to flash every 1 hour worse case so we have some data
      LogFlashStatus5();
      timeLastFlashWrite=millis();
      batteryVoltsLastWrite=batteryVoltsAve5;
    }else{
     #ifdef ECHO_DEBUG_COMMENTS1
      Serial.println(F("-Other State not logging"));
     #endif
    }
    unitStateLast=unitState;
  }
  if(millis()-timeLastFlash>100){
    timeLastFlash=millis();
    switch(ledState){
      case 1: // Running = Solid Green
        if(exitTempLostCount>0){
          BlueLED();
        }else{
          GreenLED();
        }
        break;
      case 2: // Dead = Flashing Red
        if(ledFlash>=10){
          ledFlash=0;
        }
        if(ledFlash<1){
          RedLED();
        }else if(fasterFlash>0 && ledFlash==3){
          RedLED();
        }else{
          // This is here to hold the blue light on for a bit so people notice it...
          if(exitTempLostCount>0){
            BlueLED();
          }else{
            OffLED();
          }
        }
        ledFlash++;
        break;
      case 3: // Charging = Solid Red
        if(fasterFlash>0 && tempBatAve5>8000 && tempBatAve5>tempReturnAve5+2000){
          if(ledFlash>=10){
            ledFlash=0;
          }
          if(ledFlash>=5){
            RedLED();
          }else{
            GreenLED();
          }
          ledFlash++;
        }else{
          RedLED();
        }
        break;
      case 4: // Idle = Flashing Green
        if(ledFlash>=10){
          ledFlash=0;
        }
        // If the battery is warm and connected to a charger then flash red and green
        if(fasterFlash>0 && tempBatAve5>8000 && tempBatAve5>tempReturnAve5+2000){
          if(ledFlash<1 || ledFlash==3){
            GreenLED();
          }else if(ledFlash==8){
            RedLED();
          }else{
            OffLED();
          }
        }else{
          if(ledFlash<1){
            GreenLED();
          }else if(fasterFlash>0 && ledFlash==3){
            GreenLED();
          }else{
            OffLED();
          }
        }
        ledFlash++;
        break;
      case 5: // Temp probe or icefan error = fast red flash
        if(ledFlash>=10){
          ledFlash=0;
        }
        if(ledFlash%2==0){
          RedLED();
        }else{
          OffLED();
        }
        ledFlash++;
        break;
      default: // LEDs off
        break;
    }
    if(iceFanReadReady>=10){
      CalcIceFanRPM();
      iceFanReadReady=5;
    }
    if(unitState==1 && iceFanStateLast==1){
      iceFanCheckCount++;
      if(iceFanCheckCount>=15){
        if(millis()-millisIceFanOnTime>1500){
          // Read RPM now
          iceFanCheckCount=10;  // Read the speed every half second now
          millisIceFanSpeedLast=IceFanReadRPM();
          if(millisIceFanSpeedLast>=millisIceFanSpeed){
            // If this RPM is faster set the max reading
            millisIceFanMaxTime=millis();
            millisIceFanSpeed=millisIceFanSpeedLast;
          }else if(millis()-millisIceFanMaxTime>=600000){
            // Every 20 minute reset the max reading
            millisIceFanMaxTime=millis();
            millisIceFanSpeed=millisIceFanSpeedLast;
          }
        }
      }
    }
  }
  if(Serial.available()>0){
    int cmd=Serial.read();
    switch(cmd){
      case 'L': liveData=1;
        Serial.println(F("-Live data enabled\r\nmillis(),mVBat,iceFan,fReturn,fExit,fTop,mainFan,fBat,mVCharger,fRetDup,safeVolts,dAmb,fAmb,dBat,fBat"));
        eraseConfirm=0; targetConfirm=0; activateConfirm=0; zeroConfirm=0; readJump=0; epochRead=0; voffsetpre=0; tftStep=0; intelTopDecode=0; break;
      case 'l': liveData=0; Serial.println(F("-Live data off")); eraseConfirm=0; targetConfirm=0; activateConfirm=0; zeroConfirm=0; readJump=0; epochRead=0; voffsetpre=0; tftStep=0; intelTopDecode=0; break;
      // case 's': PrintTargets(); eraseConfirm=0; targetConfirm=0; activateConfirm=0; zeroConfirm=0; readJump=0; epochRead=0; voffsetpre=0; tftStep=0; intelTopDecode=0; break;
      case 'D': liveData=0; DumpAllFlash(); eraseConfirm=0; targetConfirm=0; activateConfirm=0; zeroConfirm=0; readJump=0; epochRead=0; voffsetpre=0; tftStep=0; intelTopDecode=0; break;
      case 'E': eraseConfirm=1; targetConfirm=0; activateConfirm=0; zeroConfirm=0; readJump=0; epochRead=0; voffsetpre=0; tftStep=0; intelTopDecode=0; break;
      case 'r': if(eraseConfirm==1){ eraseConfirm=2; }else{ eraseConfirm=0; } targetConfirm=0; activateConfirm=0; zeroConfirm=0; readJump=0; epochRead=0; voffsetpre=0; tftStep=0; intelTopDecode=0; break;
      case 'a': if(eraseConfirm==2){ liveData=0; EraseAllFlash(); } eraseConfirm=0; targetConfirm=0; activateConfirm=0; zeroConfirm=0; readJump=0; epochRead=0; voffsetpre=0; tftStep=0; intelTopDecode=0; break;
      case 'p': liveData=0; PrintPositionStatus(); eraseConfirm=0; targetConfirm=0; activateConfirm=0; zeroConfirm=0; readJump=0; epochRead=0; voffsetpre=0; tftStep=0; intelTopDecode=0; break;
      // =UNIT_ID FIRMWARE_ID millis() flashOffsetRead flashOffsetWrite TEMP_TARGET
      case 'P': liveData=0; PrintPositionData(); eraseConfirm=0; targetConfirm=0; activateConfirm=0; zeroConfirm=0; readJump=0; epochRead=0; voffsetpre=0; tftStep=0; intelTopDecode=0; break;
      // if no new data available
      // ~UNIT_ID FIRMWARE_ID millis() flashOffsetRead flashOffsetWrite TEMP_TARGET
      // if data is available
      // |HEX DATA BLOCKS
      case 'R': liveData=0; RePrintPositionData(); eraseConfirm=0; targetConfirm=0; activateConfirm=0; zeroConfirm=0; readJump=0; epochRead=0; voffsetpre=0; tftStep=0; intelTopDecode=0; break;
      // if last read greater than zero go back one and print
      // |HEX DATA BLOCKS
      case 'N': liveData=0; PrintNowData(); eraseConfirm=0; targetConfirm=0; activateConfirm=0; zeroConfirm=0; readJump=0; epochRead=0; voffsetpre=0; tftStep=0; intelTopDecode=0; break;
      // \HEX DATA BLOCKS
      case 'b': liveData=0; eraseConfirm=0; targetConfirm=0; activateConfirm=0; zeroConfirm=0; readJump=0; epochRead=0; PrintNowCRC(); voffsetpre=0; tftStep=0; intelTopDecode=0; break;
      case 'f': liveData=0; eraseConfirm=0; targetConfirm=0; activateConfirm=0; zeroConfirm=0; readJump=0; epochRead=0; PrintFlashCRC(); voffsetpre=0; tftStep=0; intelTopDecode=0; break;
      case 'F': liveData=0; eraseConfirm=0; targetConfirm=0; activateConfirm=0; zeroConfirm=0; readJump=0; epochRead=0; RePrintFlashCRC(); voffsetpre=0; tftStep=0; intelTopDecode=0; break;
      case 'u': liveData=0; eraseConfirm=0; targetConfirm=0; activateConfirm=0; zeroConfirm=0; readJump=0; epochInput=0; epochRead=1; voffsetpre=0; tftStep=0; intelTopDecode=0; break;
      case 'U':
        if(epochRead==11 && epochInput>1538070000 && epochInput<1890000000){
          // Only accept dates from 2018 to 2029
         #ifdef ECHO_DEBUG_COMMENTS3
          Serial.print(F("-Update epoch from "));
          Serial.print(epochTime);
          Serial.print(F(" to "));
          Serial.println(epochInput);
         #endif
          epochTime=epochInput;
          EpochSet();
          relogChange=1;
          timeLastAverages5=millis()-300000;
        }else{
         #ifdef ECHO_DEBUG_COMMENTS3
          Serial.print(F("-Bad epoch time "));
          Serial.println(epochInput);
         #endif
        }
        liveData=0; eraseConfirm=0; targetConfirm=0; activateConfirm=0; zeroConfirm=0; readJump=0; epochRead=0; voffsetpre=0; tftStep=0; intelTopDecode=0; break;
      case 'T': eraseConfirm=0; targetConfirm=1; activateConfirm=0; zeroConfirm=0; readJump=0; epochRead=0; voffsetpre=0; tftStep=0; intelTopDecode=0; break;
      case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        if(targetConfirm==1){
          temp_build=cmd-'0';
          targetConfirm=2;
        }else if(targetConfirm==2){
          temp_build*=10;
          temp_build+=cmd-'0';
          EEPROM.write(EEPROM_TEMP_TARGET,temp_build+100);
         #ifdef ECHO_DEBUG_COMMENTS2
          Serial.print(F("-New TT set "));  // -New Temperature Target set 
          Serial.println(EEPROM.read(EEPROM_TEMP_TARGET));
         #endif
          temp_build*=10;
          temp_build+=3200;
          TEMP_TARGET=temp_build;
         #ifdef ECHO_DEBUG_COMMENTS2
          Serial.print(F("-TT is "));  // -Temperature Target is 
          Serial.println(TEMP_TARGET);
         #endif
          TargetTempSet();
          relogChange=1;
          targetConfirm=0;
        }else if(epochRead>0){
          epochInput*=10;
          epochInput+=cmd-'0';
          epochRead++;
        }else if(tftStep>0){
          switch(tftStep){
            case 1: case 2: case 3:
              tftID*=10; tftID+=cmd-'0'; tftStep++; break;
            case 5:
              tftF1=cmd; tftStep++; break;
            case 6:
              tftF2=cmd; tftStep++; break;
            case 7:
              tftF3=cmd; tftStep++; break;
            case 8:
              tftF4=cmd; tftStep++; break;
            case 9:
              tftF5=cmd; tftStep++; break;
            case 11:
              tftCRC=(tftID/100);
              tftCRC+=(tftID/10)%10;
              tftCRC+=tftID%10;
              tftCRC+=tftF1-'0';
              tftCRC+=tftF2-'0';
              tftCRC+=tftF3-'0';
              tftCRC+=tftF4-'0';
              tftCRC+=tftF5-'0';
              tftCRC%=10;
              tftCRC+='0';
              if(tftCRC==cmd){
                Serial.print(F("-TFT W"));
                Serial.print(tftID);
                Serial.print('O');
                Serial.write(tftF1);
                Serial.write(tftF2);
                Serial.write(tftF3);
                Serial.write(tftF4);
                Serial.write(tftF5);
                Serial.print('%');
                Serial.write(tftCRC);
                Serial.println();
                spibuf[1]='W';
                spibuf[2]=(tftID/100)+'0';
                spibuf[3]=((tftID/10)%10)+'0';
                spibuf[4]=(tftID%10)+'0';
                spibuf[5]='O';
                spibuf[6]=tftF1;
                spibuf[7]=tftF2;
                spibuf[8]=tftF3;
                spibuf[9]=tftF4;
                spibuf[10]=tftF5;
                spibuf[11]='%';
                spibuf[12]=tftCRC;
                spibuf[13]='_';
                spibuf[14]='_';
                LogFlashDebug();
                tftRead=millis();
              }else{
               #ifdef ECHO_DEBUG_COMMENTS3
                Serial.print(F("-TFT err W"));
                Serial.print(tftID);
                Serial.print('O');
                Serial.write(tftF1);
                Serial.write(tftF2);
                Serial.write(tftF3);
                Serial.write(tftF4);
                Serial.write(tftF5);
                Serial.print('%');
                Serial.write(tftCRC);
                Serial.print(' ');
                Serial.println(cmd);
               #endif
              }
              tftStep=0;
              break;
            default:
             #ifdef ECHO_DEBUG_COMMENTS3
              Serial.print(F("-TFT err "));
              Serial.println(tftStep);
             #endif
              tftStep=0;
          }
        }else if(intelTopDecode==3){
          if(cmd=='1'){
            intelTopUnitFan=1;
          }else{
            intelTopUnitFan=0;
          }
        }else if(intelTopDecode==2){
          intelTopIDWorking*=10;
          intelTopIDWorking+=cmd-'0';
        }else{
          targetConfirm=0;
          tftStep=0;
        }
        eraseConfirm=0; activateConfirm=0; zeroConfirm=0; readJump=0; voffsetpre=0; break;
      case 'A': eraseConfirm=0; targetConfirm=0; activateConfirm=1; zeroConfirm=0; readJump=0; epochRead=0; voffsetpre=0; tftStep=0; intelTopDecode=0; break;
      case 'c': eraseConfirm=0; targetConfirm=0; zeroConfirm=0; if(activateConfirm==1){ activateConfirm=2; }else{ activateConfirm=0; } readJump=0; epochRead=0; voffsetpre=0; tftStep=0; intelTopDecode=0; break;
      case '#': eraseConfirm=0; targetConfirm=0; zeroConfirm=0; if(activateConfirm==2 && batteryVoltsAve1>BATTERY_SHIPABLE){ unitState=0; Serial.println(F("-Manually switching to idle")); } activateConfirm=0; readJump=0; epochRead=0; voffsetpre=0; tftStep=0; break;
      case 'Z': eraseConfirm=0; targetConfirm=0; activateConfirm=0; zeroConfirm=1; readJump=0; epochRead=0; voffsetpre=0; tftStep=0; intelTopDecode=0; break;
      case 'o': eraseConfirm=0; targetConfirm=0; activateConfirm=0; if(zeroConfirm==1){ zeroConfirm=2; }else{ zeroConfirm=0; } readJump=0; epochRead=0; voffsetpre=0; tftStep=0; intelTopDecode=0; break;
      case '&': eraseConfirm=0; targetConfirm=0; activateConfirm=0; if(zeroConfirm==2){ flashOffsetRead=0; Serial.println(F("-Read offset zero'ed")); } zeroConfirm=0; readJump=0; epochRead=0; voffsetpre=0; tftStep=0; intelTopDecode=0; break;
      case 'j': eraseConfirm=0; targetConfirm=0; activateConfirm=0; epochRead=0; voffsetpre=0; tftStep=0; if(readJump==0){ readJump=1; }else{ readJump=0; } break;
      case 'J': eraseConfirm=0; targetConfirm=0; activateConfirm=0; epochRead=0; voffsetpre=0; tftStep=0; if(readJump==0){ readJump=2; }else{ readJump=0; } break;
      case 'k': eraseConfirm=0; targetConfirm=0; activateConfirm=0; epochRead=0; voffsetpre=0; tftStep=0; if(readJump==1){ readJump=0; JumpUp(100); }else if(readJump==2){ readJump=0; JumpDown(100); }else{ readJump=0; } break;
      case 'K': eraseConfirm=0; targetConfirm=0; activateConfirm=0; epochRead=0; voffsetpre=0; tftStep=0; if(readJump==1){ readJump=0; JumpUp(1000); }else if(readJump==2){ readJump=0; JumpDown(1000); }else{ readJump=0; } break;
      case 'g': eraseConfirm=0; targetConfirm=0; activateConfirm=0; zeroConfirm=0; readJump=0; epochRead=0; tftStep=0;
        if(voffsetpre==21){voffsetpre=22;}else{PrintVCCTemp(); voffsetpre=1;} break;
      case 'h': eraseConfirm=0; targetConfirm=0; activateConfirm=0; zeroConfirm=0; readJump=0; epochRead=0; tftStep=0;
        if(voffsetpre==1){voffsetpre=2;}else{PrintVCCTemp2(); voffsetpre=21;} break;
      case 'G': eraseConfirm=0; targetConfirm=0; activateConfirm=0; zeroConfirm=0; readJump=0; epochRead=0; tftStep=0;
        if(voffsetpre==2){DecVCCTemp(); PrintVCCTemp();}else if(voffsetpre==22){DecVCCTemp2(); PrintVCCTemp2();} voffsetpre=0; intelTopDecode=0; break;
      case 'H': eraseConfirm=0; targetConfirm=0; activateConfirm=0; zeroConfirm=0; readJump=0; epochRead=0; tftStep=0;
        if(voffsetpre==2){IncVCCTemp(); PrintVCCTemp();}else if(voffsetpre==22){IncVCCTemp2(); PrintVCCTemp2();} voffsetpre=0; intelTopDecode=0; break;
      case 'm':
        eraseConfirm=0; targetConfirm=0; activateConfirm=0; zeroConfirm=0; readJump=0; epochRead=0; tftStep=0; intelTopDecode=0;
        sample=0;
       #ifdef ECHO_DEBUG_COMMENTS3
        Serial.println(F("-History on Bat"));
        while(sample<24 && unitHistory[sample]<255){
          Serial.print('-');
          Serial.print(unitHistory[sample]);
          Serial.print(';');
          Serial.print(chargerHistory[sample]);
          Serial.print(';');
          Serial.print(batteryVoltsHistory[sample]);
          Serial.print(';');
          Serial.println(tempBatHistory[sample]);
          sample++;
        }
       #endif
        break;
      case 'W':
        tftStep=1;
        tftID=0;
        break;
      case 'O':
        if(tftStep==4){
          tftStep++;
        }else{
          tftStep=0;
        }
        break;
      case '%':
        if(tftStep==10){
          tftStep++;
        }else{
          tftStep=0;
        }
        break;
      case 'i':
        if(intelTopDecode==2){
          intelTopDecode=3;
        }else if(intelTopDecode==3){
          intelTopDecode=4;
        }else if(intelTopDecode==4){
          if(intelTopIDWorking>0){
            if(intelTopID!=intelTopIDWorking){
              intelTopID=intelTopIDWorking;
              Serial.print(F("-New Top ID: "));
              Serial.println(intelTopID);
            }
            SaveTFTid();
            intelTopUnit=1;
          }
          intelTopTimeReplied=millis();
          intelTopDecode=0;
        }else{
          intelTopDecode=0;
        }
        break;
      case 'I':
        if(intelTopDecode==0){
          intelTopDecode=1;
        }else if(intelTopDecode==1){
          intelTopDecode=2;
          intelTopIDWorking=0;
        }else{
          intelTopDecode=0;
        }
        break;
      default: eraseConfirm=0; targetConfirm=0; activateConfirm=0; zeroConfirm=0; readJump=0; epochRead=0; voffsetpre=0; tftStep=0; intelTopDecode=0; break;
    }
      //   B C                   M       Q   S     V   X Y  
      //       d e                 n     q     t   v w x y z
      // ! @   $   ^   * ( ) , . / < > ? : " ; ' { } [ ] = + ` ~ - +
  }
}

void SaveTFTid(){
  if(intelTopID!=intelTopIDWritten){
    if(millis()-intelTopTimeWritten>600000){
      spibuf[1]='T';
      spibuf[2]='F';
      spibuf[3]='T';
      spibuf[4]='i';
      spibuf[5]='d';
      spibuf[6]='=';
      unsigned int ttv=intelTopID;
      spibuf[11]=(ttv%10)+'0';
      ttv/=10;
      spibuf[10]=(ttv%10)+'0';
      ttv/=10;
      spibuf[9]=(ttv%10)+'0';
      ttv/=10;
      spibuf[8]=(ttv%10)+'0';
      ttv/=10;
      spibuf[7]=(ttv%10)+'0';
      spibuf[12]=' ';
      spibuf[13]=' ';
      spibuf[14]=' ';
      LogFlashDebug();
      intelTopIDWritten=intelTopID;
      intelTopTimeWritten=millis();
      Serial.print(F("-New Top ID write "));
      Serial.println(intelTopID);
    }
  }
}

void TargetTempSet(){
  spibuf[1]='T';
  spibuf[2]='e';
  spibuf[3]='m';
  spibuf[4]='p';
  spibuf[5]='=';
  unsigned int ttv=TEMP_TARGET;
  spibuf[9]=(ttv%10)+'0';
  ttv/=10;
  spibuf[8]=(ttv%10)+'0';
  ttv/=10;
  spibuf[7]=(ttv%10)+'0';
  ttv/=10;
  spibuf[6]=(ttv%10)+'0';
  spibuf[10]=' ';
  spibuf[11]=' ';
  spibuf[12]=' ';
  spibuf[13]=' ';
  spibuf[14]=' ';
  LogFlashDebug();
}

void RedLED(){
  if(safeVolts>=7000){
    ledStatus[0]=CRGB::Red;
    FastLED.show();
  }else{
    OffLED();
  }
  if(unitState==3){
    ledStatus2[0]=CRGB::Black;
  }else{
    ledStatus2[0]=CRGB::Red;
  }
  FastLED.show();
}

void GreenLED(){
  if(safeVolts>=7000){
    ledStatus[0]=CRGB::Green;
    FastLED.show();
  }else{
    OffLED();
  }
  if(unitState==3){
    ledStatus2[0]=CRGB::Black;
  }else{
    ledStatus2[0]=CRGB::Green;
  }
  FastLED.show();
}

void BlueLED(){
  if(safeVolts>=7000){
    ledStatus[0]=CRGB::Blue;
    FastLED.show();
  }else{
    OffLED();
  }
  if(unitState==3){
    ledStatus2[0]=CRGB::Black;
  }else{
    ledStatus2[0]=CRGB::Blue;
  }
  FastLED.show();
}

void WhiteLED(){
  ledStatus[0]=CRGB::White;
  ledStatus2[0]=CRGB::White;
  FastLED.show();
}

void OrangeLED(){
  ledStatus[0]=CRGB::Orange;
  ledStatus2[0]=CRGB::Orange;
  FastLED.show();
}

void YellowLED(){
  ledStatus[0]=CRGB::Yellow;
  ledStatus2[0]=CRGB::Yellow;
  FastLED.show();
}

void BrownLED(){
  ledStatus[0]=CRGB::Brown;
  ledStatus2[0]=CRGB::Brown;
  FastLED.show();
}

void OffLED(){
  ledStatus[0]=CRGB::Black;
  ledStatus2[0]=CRGB::Black;
  FastLED.show();
}

void ReadVolts(){
  unsigned long tempLong=analogRead(BAT_VOLTS);
  tempLong*=VOLT_SCALER;
  // This is a 3v3 ref and 100k/8k voltage divider
  tempLong*=37;
  tempLong/=1549;
  batteryVolts=tempLong+VOLT_OFFSET;
  tempLong=analogRead(CHARGER_VOLTS);
  tempLong*=VOLT_SCALER2;
  tempLong*=37;
  tempLong/=1549;
  chargerVolts=tempLong;
  tempLong=analogRead(SAFE_VOLTS);
  tempLong*=VOLT_SCALER2;
  tempLong*=37;
  tempLong/=1549;
  safeVolts=tempLong;
}
void ReadTempReturn(){
 #ifdef USE_PCT2075_PROBE_AMBIENT
  if(Wire.requestFrom(PCT2075_ADDRESS,2)==2){
    int tempWorking=Wire.read();
    tempWorking<<=8;
    tempWorking|=Wire.read();
    tempWorking>>=5;
    if(tempWorking>=-136){
      if(tempWorking<=520){
        tempWorking*=45;
        tempWorking/=2;
        tempWorking+=3200;
        tempReturn=tempWorking;
      }else{
        tempReturn=15000;
      }
    }else{
      tempReturn=0;
    }
  }
 #else
  tempReturn=tempRetDigital;
 #endif
}
void ReadTempExit(){
  tempExit=ADC2Temp(analogRead(TEMP_EXIT));
}
void ReadTempTop(){
  tempTop=ADC2Temp(analogRead(TEMP_TOP));
}
void ReadTempBat(){
  if(dsBatteryAvailable>0){
    tempBat=tempBatDigital;
  }else{
    tempBat=0;
  }
}
void ReadTempReturnDigital(){
  if(dsAmbient.getAddress(addr,0)){
    float tempC = dsAmbient.getTempC(addr);
    if(tempC == DEVICE_DISCONNECTED_C){
     #ifdef ECHO_DEBUG_COMMENTS1
      Serial.println(F("-Error: read dsAmbient temp"));
     #endif
      if(dsAmbientAvailable>0){
        dsAmbientAvailable=0;
      }
      dsAmbient.begin();
      dsAmbient.getDeviceCount();
      dsAmbient.setWaitForConversion(false);
    }else{
      if(tempC>=-17.0){
        if(tempC<=51.5){
          tempRetDigital=(tempC*180)+3200;
        }else{
          tempRetDigital=12550;
        }
      }else{
        tempRetDigital=0;
      }
      if(dsAmbientAvailable==0){
        dsAmbientAvailable=1;
      }
    }
  }else{
   #ifdef ECHO_DEBUG_COMMENTS1
    Serial.println(F("-Error: find dsAmbient temp"));
   #endif
    if(dsAmbientAvailable>0){
      dsAmbientAvailable=0;
    }
    dsAmbient.begin();
    dsAmbient.getDeviceCount();
    dsAmbient.setWaitForConversion(false);
  }
  dsAmbient.requestTemperatures();
}
void ReadTempBatDigital(){
  if(dsBattery.getAddress(addr,0)){
    float tempC = dsBattery.getTempC(addr);
    if(tempC == DEVICE_DISCONNECTED_C){
     #ifdef ECHO_DEBUG_COMMENTS1
      Serial.println(F("-Error: read dsBattery temp"));
     #endif
      if(dsBatteryAvailable>0){
        dsBatteryAvailable=0;
      }
      dsBattery.begin();
      dsBattery.getDeviceCount();
      dsBattery.setWaitForConversion(false);
    }else{
      if(tempC>=-17.0){
        if(tempC<=51.5){
          tempBatDigital=(tempC*180)+3200;
        }else{
          tempBatDigital=12550;
        }
      }else{
        tempBatDigital=0;
      }
      if(dsBatteryAvailable==0){
        dsBatteryAvailable=1;
      }
    }
  }else{
   #ifdef ECHO_DEBUG_COMMENTS1
    Serial.println(F("-Error: find dsBattery temp"));
   #endif
    if(dsBatteryAvailable>0){
      dsBatteryAvailable=0;
    }
    dsBattery.begin();
    dsBattery.getDeviceCount();
    dsBattery.setWaitForConversion(false);
  }
  dsBattery.requestTemperatures();
}
unsigned int ADC2Temp(int adc){
  if(adc>796){
    // Temp probe open or frozen
    return 0;
  }else if(adc<21){
    // Temp probe shorted
    return 23456;
  }else if(adc<107){
    return 19999;
  }else{
    return pgm_read_word_near(degFMap20k+(adc-107));
  }
}
void DigitalTempInit(){
  safeVolts=8000;
  byte found;
  dsBatteryAvailable=0;
  while(dsBatteryAvailable==0){
    dsBattery.begin();
    found=dsBattery.getDeviceCount();
    if(found!=1){
      RedLED();
      delay(700);
      BlueLED();
      delay(100);
      OffLED();
      delay(100);
      BlueLED();
      delay(100);
      RedLED();
      delay(700);
    }
    dsBattery.setWaitForConversion(false);
    dsBattery.requestTemperatures();
    if(dsBattery.getAddress(addr,0)){
      float tempC = dsBattery.getTempC(addr);
      if(tempC == DEVICE_DISCONNECTED_C){
       #ifdef ECHO_DEBUG_COMMENTS1
        Serial.println(F("-Error: Could not read dsBattery data"));
       #endif
      }else{
        dsBatteryAvailable=1;
      }
    }
  }
  dsAmbientAvailable=0;
  while(dsAmbientAvailable==0){
    dsAmbient.begin();
    found=dsAmbient.getDeviceCount();
    if(found!=1){
      OrangeLED();
      delay(700);
      BlueLED();
      delay(100);
      OffLED();
      delay(100);
      BlueLED();
      delay(100);
      OffLED();
      delay(100);
      BlueLED();
      delay(100);
      OrangeLED();
      delay(700);
    }
    dsAmbient.setWaitForConversion(false);
    dsAmbient.requestTemperatures();
    if(dsAmbient.getAddress(addr,0)){
      float tempC = dsAmbient.getTempC(addr);
      if(tempC == DEVICE_DISCONNECTED_C){
       #ifdef ECHO_DEBUG_COMMENTS1
        Serial.println(F("-Error: Could not read dsAmbient data"));
       #endif
      }else{
        dsAmbientAvailable=1;
      }
    }
  }
  OffLED();
  safeVolts=0;
}

void IceFanInit(){
  safeVolts=8000;
  digitalWrite(FAN_ICE_ALT,HIGH);
  byte r0=0;
  while(r0==0){
    WhiteLED();
    delay(700);
    unsigned int rpm=IceFanReadRPM();
    if(rpm>3500){
      r0=1;
    }
    if(r0==0){
      BlueLED(); delay(100); OffLED(); delay(100);
      BlueLED(); delay(100); OffLED(); delay(100);
      BlueLED(); delay(100); OffLED(); delay(100);
      BlueLED(); delay(100); OffLED(); delay(100);
      BlueLED(); delay(100); OffLED(); delay(100);
      BlueLED(); delay(100);
    }
    WhiteLED();
  }
  digitalWrite(FAN_ICE_ALT,LOW);
  delay(300);
  OffLED();
  safeVolts=0;
}

unsigned int IceFanReadRPM(){
  int ret=0;
  byte r1,r2;
  unsigned long ms1;
  ms1=micros();
  r1=digitalRead(FAN_ICE_RPM);
  r2=r1;
  while(r1==r2 && micros()-ms1<30000){
    r2=digitalRead(FAN_ICE_RPM);
  }
  if(r1!=r2){
    ms1=micros();
    while(r1!=r2 && micros()-ms1<20000){
      r2=digitalRead(FAN_ICE_RPM);
    }
    if(r1==r2){
      ms1=micros()-ms1;
      if(ms1>1000){
        ms1=15000000/ms1;
        ret=ms1;
      }
    }
  }
  rpmLastReal=ret;
  rpmLastRealTime=millis();
  return ret;
}

void DigitalTempsLog(){
  if(millis()-lastSavedDS>=21600000 || (millis()-lastSavedDS>=1200000 && (dsBatteryAvailableLast!=dsBatteryAvailable || dsAmbientAvailableLast!=dsAmbientAvailable))){
    // Save digital temp data every 6 hours or 20 minutes if a temp probe dies
    unsigned long working;
    spibuf[1]='T';
    spibuf[2]='D';
    if(dsAmbientAvailable){
      spibuf[3]='E';
    }else{
      spibuf[3]='F';
    }
    working=tempRetDigital/10;
    spibuf[4]=(working%10)+'0';
    working/=10;
    spibuf[5]=(working%10)+'0';
    working/=10;
    spibuf[6]=(working%10)+'0';
    working=tempReturnAve5/100;
    spibuf[7]=(working%10)+'0';
    working/=10;
    spibuf[8]=(working%10)+'0';
    if(dsBatteryAvailable){
      spibuf[9]='B';
    }else{
      spibuf[9]='C';
    }
    working=tempBatDigital/10;
    spibuf[10]=(working%10)+'0';
    working/=10;
    spibuf[11]=(working%10)+'0';
    working/=10;
    spibuf[12]=(working%10)+'0';
    working=tempBatAve5/100;
    spibuf[13]=(working%10)+'0';
    working/=10;
    spibuf[14]=(working%10)+'0';
    LogFlashDebug();
    lastSavedDS=millis();
    dsBatteryAvailableLast=dsBatteryAvailable;
    dsAmbientAvailableLast=dsAmbientAvailable;
  }
}

//epochMillis, epochTime, epochType

void EpochStart(){
  epochMillis=millis();
  epochType=0;  // Not a real time
  if(flashOffsetWrite==0){
    epochTime=1;
  }else{
    epochTime=flashOffsetWrite*4567;
    // Max flash write offset is 262143
    // 262143 * 4567 = 1197207081
    // Which goes back to 2007 Dec 9th at 8:31 AM
    // Long before ThorFan started
    // So if the date is before this we know it is fake
  }
}
void EpochTick(){
  if(millis()-epochMillis>1000){
    epochMillis+=1000;
    epochTime++;
  }
}
void EpochSet(){
  spibuf[1]='T';
  spibuf[2]='i';
  spibuf[3]='m';
  spibuf[4]='e';
  spibuf[14]=(epochInput%10)+'0';
  epochInput/=10;
  spibuf[13]=(epochInput%10)+'0';
  epochInput/=10;
  spibuf[12]=(epochInput%10)+'0';
  epochInput/=10;
  spibuf[11]=(epochInput%10)+'0';
  epochInput/=10;
  spibuf[10]=(epochInput%10)+'0';
  epochInput/=10;
  spibuf[9]=(epochInput%10)+'0';
  epochInput/=10;
  spibuf[8]=(epochInput%10)+'0';
  epochInput/=10;
  spibuf[7]=(epochInput%10)+'0';
  epochInput/=10;
  spibuf[6]=(epochInput%10)+'0';
  epochInput/=10;
  spibuf[5]=(epochInput%10)+'0';
  LogFlashDebug();
}

void FindFlashOffset(){
  if(CheckOffsetUsed(0)==0xff){
    // If the first cell is blank it seems like an empty chip
   #ifdef ECHO_DEBUG_COMMENTS2
    Serial.println(F("-Flash Blank"));  // -Blank flash it seems
   #endif
    flashOffsetWrite=0;
    flashOffsetRead=0;
  }else{
    unsigned long tempLong=FLASH_MAX_SIZE/2,workingFlashOffset=FLASH_MAX_SIZE/2;
    byte tempByte=19,tempByte2;
    while(tempByte>0){
      tempByte2=CheckOffsetUsed(workingFlashOffset);
      if(tempLong==1){tempLong=2;}
      tempLong/=2;
      if(tempByte2==0xff){
        workingFlashOffset-=tempLong;
      }else{
        workingFlashOffset+=tempLong;
      }
      tempByte--;
      //Serial.print('-');
      //Serial.print(workingFlashOffset);
      //Serial.print(' ');
      //Serial.print(tempLong);
      //Serial.print(' ');
      //Serial.println(tempByte2);
    }
    if(workingFlashOffset>=FLASH_MAX_SIZE){
     #ifdef ECHO_DEBUG_COMMENTS2
      Serial.println(F("-Full"));  // -Flash seems Full
     #endif
      flashOffsetWrite=FLASH_MAX_SIZE;
      flashOffsetRead=0;
      return;
    }
    if(CheckOffsetUsed(workingFlashOffset)==0xff){
     #ifdef ECHO_DEBUG_COMMENTS2
      Serial.print(F("-Flash Start "));
      Serial.println(workingFlashOffset);
     #endif
      flashOffsetWrite=workingFlashOffset;
      flashOffsetRead=0;
    }else{
      workingFlashOffset++;
      if(CheckOffsetUsed(workingFlashOffset)==0xff){
       #ifdef ECHO_DEBUG_COMMENTS2
        Serial.print(F("-Flash ++ Start "));
        Serial.println(workingFlashOffset);
       #endif
        flashOffsetWrite=workingFlashOffset;
        flashOffsetRead=0;
      }else{
       #ifdef ECHO_DEBUG_COMMENTS1
        Serial.print(F("-BAD FLASH? "));
        Serial.println(workingFlashOffset);
       #endif
        flashOffsetWrite=FLASH_MAX_SIZE;
        flashOffsetRead=0;
      }
    }
  }
}
void LogFlashStatus1(){
  if(flashOffsetWrite<FLASH_MAX_SIZE){
    spibuf[0]=unitState;
    unsigned long tempLong=millis();
    spibuf[1]=(tempLong>>24)&0xff;
    spibuf[2]=(tempLong>>16)&0xff;
    spibuf[3]=(tempLong>>8)&0xff;
    spibuf[4]=tempLong&0xff;
    spibuf[5]=batteryVoltsAve1/100;
    spibuf[6]=iceFanDutyAve1;
    spibuf[7]=tempReturnAve1>>8;
    spibuf[8]=tempReturnAve1&0xff;
    spibuf[9]=tempExitAve1>>8;
    spibuf[10]=tempExitAve1&0xff;
    spibuf[11]=mainFanDuty;
    spibuf[12]=tempBatAve1>>8;
    spibuf[13]=tempBatAve1&0xff;
    spibuf[14]=tempTopAve1/100;
    spibuf[15]=8;
    WriteSPIBuf();
  }else{
   #ifdef ECHO_DEBUG_COMMENTS
    Serial.println(F("-F2F"));  // -Flash chip is Full
   #endif
  }
}
void LogFlashStatus5(){
  if(flashOffsetWrite<FLASH_MAX_SIZE){
    spibuf[0]=0x10|unitState;
    unsigned long tempLong=epochTime;
    spibuf[1]=(tempLong>>24)&0xff;
    spibuf[2]=(tempLong>>16)&0xff;
    spibuf[3]=(tempLong>>8)&0xff;
    spibuf[4]=tempLong&0xff;
    spibuf[5]=batteryVoltsAve5/100;
    spibuf[6]=(mainFanDutyAve5&0xf0)|(iceFanDutyAve5>>4);
    tempLong=tempReturnAve5/50;
    if(tempLong>251){tempLong=253;}
    spibuf[7]=tempLong;
    tempLong=tempExitAve5/50;
    if(tempLong>251){tempLong=253;}
    spibuf[8]=tempLong;
    tempLong=tempBatAve5/50;
    if(tempLong>251){tempLong=253;}
    spibuf[9]=tempLong;
    tempLong=tempTopAve5/50;
    if(tempLong>251){tempLong=253;}
    spibuf[10]=tempLong;
    tempLong=millis();
    spibuf[11]=(tempLong>>24)&0xff; // Future use
    spibuf[12]=(tempLong>>16)&0xff; // Future use
    spibuf[13]=(tempLong>>8)&0xff; // Future use
    spibuf[14]=tempLong&0xff; // Future use
    spibuf[15]=0;
    spibuf[15]^=spibuf[0];
    spibuf[15]^=spibuf[1];
    spibuf[15]^=spibuf[2];
    spibuf[15]^=spibuf[3];
    spibuf[15]^=spibuf[4];
    spibuf[15]^=spibuf[5];
    spibuf[15]^=spibuf[6];
    spibuf[15]^=spibuf[7];
    spibuf[15]^=spibuf[8];
    spibuf[15]^=spibuf[9];
    spibuf[15]^=spibuf[10];
    spibuf[15]^=spibuf[11];
    spibuf[15]^=spibuf[12];
    spibuf[15]^=spibuf[13];
    spibuf[15]^=spibuf[14];
    WriteSPIBuf();
  }else{
   #ifdef ECHO_DEBUG_COMMENTS
    Serial.println(F("-F2F"));  // -Flash chip is Full
   #endif
  }
}
void LogFlashDebug(){
  if(flashOffsetWrite<FLASH_MAX_SIZE){
    spibuf[0]=0x20|unitState;
    // Bytes 1 to 14 are set before call
    spibuf[15]=0;
    spibuf[15]^=spibuf[0];
    spibuf[15]^=spibuf[1];
    spibuf[15]^=spibuf[2];
    spibuf[15]^=spibuf[3];
    spibuf[15]^=spibuf[4];
    spibuf[15]^=spibuf[5];
    spibuf[15]^=spibuf[6];
    spibuf[15]^=spibuf[7];
    spibuf[15]^=spibuf[8];
    spibuf[15]^=spibuf[9];
    spibuf[15]^=spibuf[10];
    spibuf[15]^=spibuf[11];
    spibuf[15]^=spibuf[12];
    spibuf[15]^=spibuf[13];
    spibuf[15]^=spibuf[14];
    WriteSPIBuf();
  }else{
   #ifdef ECHO_DEBUG_COMMENTS
    Serial.println(F("-F2F"));  // -Flash chip is Full
   #endif
  }
}
void DumpAllFlash(){
  //Serial.println(F("-Serial Data Dump"));
  //unsigned long offset=0;
  //while(offset<flashOffsetWrite){
  //  ReadSPIBuf(offset);
  //  PrintSPIBuf();
  //  offset++;
  //  if(Serial.available()>0) break;
  //}
  //Serial.println(F("-Dump Complete"));
}
void JumpUp(unsigned long jump_by){
  if(flashOffsetRead+jump_by<flashOffsetWrite){
    flashOffsetRead+=jump_by;
  }else{
    if(flashOffsetWrite>0){
      flashOffsetRead=flashOffsetWrite-1;
    }
  }
  PrintPositionStatus();
}
void JumpDown(unsigned long jump_by){
  if(flashOffsetRead>=jump_by){
    flashOffsetRead-=jump_by;
  }else{
    flashOffsetRead=0;
  }
  PrintPositionStatus();
}

void SPIFlashInit(){
  byte tempByte=1;
  safeVolts=8000;
  YellowLED();
  while(tempByte>0){
    SPI.beginTransaction(flash);
    digitalWrite(SPI_FLASH,LOW);
    SPI.transfer(0x66);
    digitalWrite(SPI_FLASH,HIGH);
    digitalWrite(SPI_FLASH,LOW);
    SPI.transfer(0x99);
    digitalWrite(SPI_FLASH,HIGH);
    delay(100);
    digitalWrite(SPI_FLASH,LOW);
    SPI.transfer(0x05);
    tempByte=SPI.transfer(0x05);
    digitalWrite(SPI_FLASH,HIGH);
    SPI.endTransaction();
    if(tempByte>0){
      YellowLED();
      delay(700);
      BlueLED(); delay(100); OffLED(); delay(100);
      BlueLED(); delay(100); OffLED(); delay(100);
      BlueLED(); delay(100); OffLED(); delay(100);
      BlueLED(); delay(100); OffLED(); delay(100);
      BlueLED(); delay(100);
      YellowLED();
      delay(700);
     #ifdef ECHO_DEBUG_COMMENTS2
      Serial.println(F("-Flash reset error"));
     #endif
    }
  }
 #ifdef ECHO_DEBUG_COMMENTS1
  Serial.println(F("-Flash reset ok"));
 #endif
  byte manuID;  // 0xEF
  byte typeID;  // 0x40
  byte capaID;  // 0x18
  SPI.beginTransaction(flash);
  digitalWrite(SPI_FLASH,LOW);
  SPI.transfer(0x9F);
  manuID=SPI.transfer(0);
  typeID=SPI.transfer(0);
  capaID=SPI.transfer(0);
  digitalWrite(SPI_FLASH,HIGH);
  SPI.endTransaction();
  if(manuID!=0xEF || typeID!=0x40 || capaID!=0x18){
    while(1){
      YellowLED();
      delay(700);
      BlueLED(); delay(100); OffLED(); delay(100);
      BlueLED(); delay(100); OffLED(); delay(100);
      BlueLED(); delay(100); OffLED(); delay(100);
      BlueLED(); delay(100); OffLED(); delay(100);
      BlueLED(); delay(100);
     #ifdef ECHO_DEBUG_COMMENTS2
      Serial.println(F("-Flash ID error"));
     #endif
    }
  }
 #ifdef ECHO_DEBUG_COMMENTS1
  Serial.println(F("-Flash ID ok"));
 #endif
  FindFlashOffset();
  ReadSPIBuf(0);
 #ifdef ECHO_DEBUG_COMMENTS2
  Serial.print(F("-Flash read "));
  Serial.print(flashOffsetWrite);
  Serial.print(' ');
  PrintSPIBuf();
 #endif
  if(spibuf[0]!=0x21 || spibuf[1]!='N' || spibuf[2]!='I' || spibuf[3]!='D'
     || spibuf[4]!=':' || spibuf[13]!='O' || spibuf[14]!='k'){
   #ifdef ECHO_DEBUG_COMMENTS1
    Serial.println(F("-Flash start miss match"));
   #endif
    if(spibuf[0]!=0xFF || spibuf[1]!=0xFF || spibuf[2]!=0xFF || spibuf[3]!=0xFF
       || spibuf[4]!=0xFF || spibuf[13]!=0xFF || spibuf[14]!=0xFF || spibuf[15]!=0xFF
       || flashOffsetWrite!=0){
      EraseAllFlash();
    }else{
      tempByte=1;
      SPI.beginTransaction(flash);
      digitalWrite(SPI_FLASH,LOW);
      SPI.transfer(0x4B);
      while(tempByte<=12){
        spibuf[tempByte]=SPI.transfer(0);
        tempByte++;
      }
      digitalWrite(SPI_FLASH,HIGH);
      SPI.endTransaction();
      spibuf[1]='N';
      spibuf[2]='I';
      spibuf[3]='D';
      spibuf[4]=':';
      spibuf[13]='O';
      spibuf[14]='k';
     #ifdef ECHO_DEBUG_COMMENTS2
      Serial.print(F("-Flash write "));
      Serial.print(flashOffsetWrite);
      Serial.print(' ');
      PrintSPIBuf();
     #endif
      LogFlashDebug();
      while(false){
        GreenLED(); delay(500); YellowLED(); delay(500);
      }
    }
  }
  OffLED();
  safeVolts=0;
}
void ReadSPIBuf(unsigned long workingFlashOffset){
  byte spibufa1=(workingFlashOffset>>12)&0xff;
  byte spibufa2=(workingFlashOffset>>4)&0xff;
  byte spibufa3=(workingFlashOffset<<4)&0xff;
  SPI.beginTransaction(flash);
  digitalWrite(SPI_FLASH,LOW);
  SPI.transfer(0x03);
  SPI.transfer(spibufa1);
  SPI.transfer(spibufa2);
  SPI.transfer(spibufa3);
  byte tempByte=0;
  while(tempByte<16){
    spibuf[tempByte]=SPI.transfer(0);
    tempByte++;
  }
  digitalWrite(SPI_FLASH,HIGH);
  SPI.endTransaction();
}
void WriteSPIBuf(){
  if(flashOffsetWrite<FLASH_MAX_SIZE){
    byte spibufa1=(flashOffsetWrite>>12)&0xff;
    byte spibufa2=(flashOffsetWrite>>4)&0xff;
    byte spibufa3=(flashOffsetWrite<<4)&0xff;
    SPI.beginTransaction(flash);
    digitalWrite(SPI_FLASH,LOW);
    SPI.transfer(0x06);
    digitalWrite(SPI_FLASH,HIGH);
    digitalWrite(SPI_FLASH,LOW);
    SPI.transfer(0x02);
    SPI.transfer(spibufa1);
    SPI.transfer(spibufa2);
    SPI.transfer(spibufa3);
    byte tempByte=0;
    while(tempByte<16){
      SPI.transfer(spibuf[tempByte]);
      tempByte++;
    }
    digitalWrite(SPI_FLASH,HIGH);
    SPI.endTransaction();
    flashOffsetWrite++;
  }else{
   #ifdef ECHO_DEBUG_COMMENTS1
    Serial.println(F("-Fsua"));  // -Flash space unavailable
   #endif
  }
}
byte CheckOffsetUsed(unsigned long workingFlashOffset){
  byte spibufa1=(workingFlashOffset>>12)&0xff;
  byte spibufa2=(workingFlashOffset>>4)&0xff;
  byte spibufa3=(workingFlashOffset<<4)&0xff;
  SPI.beginTransaction(flash);
  digitalWrite(SPI_FLASH,LOW);
  SPI.transfer(0x03);
  SPI.transfer(spibufa1);
  SPI.transfer(spibufa2);
  SPI.transfer(spibufa3);
  spibufa1=SPI.transfer(0);
  digitalWrite(SPI_FLASH,HIGH);
  SPI.endTransaction();
  return spibufa1;
}
void EraseAllFlash(){
  byte tempByte;
 #ifdef ECHO_DEBUG_COMMENTS2
  Serial.println(F("-Erasing flash"));
 #endif
  SPI.beginTransaction(flash);
  digitalWrite(SPI_FLASH,LOW);
  SPI.transfer(0x06);
  digitalWrite(SPI_FLASH,HIGH);
  digitalWrite(SPI_FLASH,LOW);
  SPI.transfer(0x60);
  digitalWrite(SPI_FLASH,HIGH);
  unsigned long ms1=millis();
  do{
    digitalWrite(SPI_FLASH,LOW);
    SPI.transfer(0x05);
    tempByte=SPI.transfer(0x05);
    digitalWrite(SPI_FLASH,HIGH);
    Serial.print(tempByte);
    RedLED(); delay(100); YellowLED(); delay(400);
  }while(tempByte>0 && millis()-ms1<30000);
  SPI.endTransaction();
 #ifdef ECHO_DEBUG_COMMENTS3
  Serial.println(F("\r\n-Erase complete"));
  delay(300);
  Serial.print(F("Rebooting"));
  delay(300);
 #endif
  Serial.print(F("."));
  delay(300);
  Serial.print(F("."));
  delay(300);
  Serial.print(F("."));
  cli();
  wdt_enable(WDTO_250MS);
  while(1);
}

void CalcFans(){
 #ifdef JETPOD_MODE
  if(unitState==1 && tempExit>0){
    if(tempExit>TEMP_TARGET){
      mainFanDuty=255;
    }else{
      mainFanDuty=0;
    }
    if(dsAmbientAvailable && tempRetDigital>2500){
      if(tempRetDigital>TEMP_TARGET){
        iceFanDuty=255;
      }else{
        iceFanDuty=0;
      }
    }else{
      if(tempExit>TEMP_TARGET){
        iceFanDuty=255;
      }else{
        iceFanDuty=0;
      }
    }
    if(exitTempLostCount<240){
      exitTempLostCount=0;
    }
  }else{
    iceFanDuty=0;
    mainFanDuty=0;
  }
 #else
  if(unitState==1 && tempExit>0){
    if(tempTopAve1>22222){
      // If the top temp sensor is shorted with a 50 ohm resistor then kill all fans
      iceFanDuty=0;
    }else if(tempExit>TEMP_TARGET){
      iceFanDuty=255;
    }else{
      iceFanDuty=0;
    }
    if(tempTopAve1>22222){
      // If the top temp sensor is shorted with a 50 ohm resistor then kill all fans
      mainFanDuty=0;
    }else if(iceFanDuty>0){
      // If we are dumping dry ice we always max out the top fans to help lift it quickly
      mainFanDuty=255;
    }else{
      CalcMainFain();
    }
    if(exitTempLostCount<240){
      exitTempLostCount=0;
    }
  }else if(unitState==1 && tempTop>0 && tempTop<10000){
    if(tempTop>TEMP_TARGET){
      iceFanDuty=255;
    }else{
      iceFanDuty=0;
    }
    if(iceFanDuty>0){
      // If we are dumping dry ice we always max out the top fans to help lift it quickly
      mainFanDuty=255;
    }else{
      CalcMainFain();
    }
    if(exitTempLostCount<245){
      exitTempLostCount++;
    }else{
      exitTempLostCount=250;
    }
  }else{
    iceFanDuty=0;
    mainFanDuty=0;
  }
 #endif
  iceFanLimitCounter++; if(iceFanLimitCounter>=40){ iceFanLimitCounter=0; }  // 40x500ms = 20 sec loop
  if(millis()-intelTopTimeReplied>60000){
    intelTopUnit=2;
  }
  if(intelTopUnit==1 || (unitState==1 && (millis()-timeStartRun)<30000)){
    digitalWrite(FAN_MAIN,HIGH); // Force the Main Fans on for the first 30 seconds too
  }else{
   #ifdef JETPOD_MODE
    if(iceFanLimitCounter<ICE_FAN_LIMIT_COUNT){
      digitalWrite(FAN_MAIN,mainFanDuty?HIGH:LOW);
    }else{
      digitalWrite(FAN_MAIN,LOW);  // Force down time for the fan to limit PTC trip
    }
   #else
    analogWrite(FAN_MAIN,mainFanDuty);
   #endif
  }
  if(intelTopUnit==1){
    if(intelTopUnitFan==1){
     #ifdef JETPOD_MODE
      if(iceFanLimitCounter<ICE_FAN_LIMIT_COUNT){
        digitalWrite(FAN_ICE_ALT,1);
        TrackIceFanOn();
      }else{
        digitalWrite(FAN_ICE_ALT,0);
        TrackIceFanOff();
      }
     #else
      digitalWrite(FAN_ICE_ALT,1);
      TrackIceFanOn();
     #endif
    }else{
      if((unitState==1 && (millis()-timeStartRun)<30000)){
        digitalWrite(FAN_ICE_ALT,1); // Force the Ice Fan on for the first 30 seconds
        TrackIceFanOn();
      }else{
        digitalWrite(FAN_ICE_ALT,0);
        TrackIceFanOff();
      }
    }
  }else if(iceFanDuty==0){
    if((unitState==1 && (millis()-timeStartRun)<30000)){
      digitalWrite(FAN_ICE_ALT,1); // Force the Ice Fan on for the first 30 seconds
      TrackIceFanOn();
    }else{
      digitalWrite(FAN_ICE_ALT,0);
      TrackIceFanOff();
    }
  }else{
   #ifdef JETPOD_MODE
    if(iceFanLimitCounter<ICE_FAN_LIMIT_COUNT){
      digitalWrite(FAN_ICE_ALT,1);
      TrackIceFanOn();
    }else{
      digitalWrite(FAN_ICE_ALT,0);
      TrackIceFanOff();
    }
   #else
    digitalWrite(FAN_ICE_ALT,1);
    TrackIceFanOn();
   #endif
  }
}
void CalcMainFain(){
  if(batteryVolts>10000){
    unsigned int delta=(batteryVolts/10)-1000;
    //delta*=19;
    //delta/=100;
    delta/=5;
    if(delta>85){
      mainFanDuty=170;
    }else{
      mainFanDuty=255-delta;
    }
  }else{
    mainFanDuty=255;
  }
}
void incrementAverages(){
  ReadVolts();
  batteryVoltsSum1+=batteryVolts;
  batteryVoltsSum5+=batteryVolts;
  chargerVoltsSum5+=chargerVolts;
  chargerVoltsSum1+=chargerVolts;
  safeVoltsSum5+=safeVolts;
  safeVoltsSum1+=safeVolts;
  ReadTempReturn();
  tempReturnSum1+=tempRetDigital;
  tempReturnSum5+=tempRetDigital;
  ReadTempExit();
  tempExitSum1+=tempExit;
  tempExitSum5+=tempExit;
  ReadTempTop();
  tempTopSum1+=tempTop;
  tempTopSum5+=tempTop;
  ReadTempBat();
  tempBatSum1+=tempBatDigital;
  tempBatSum5+=tempBatDigital;
  CalcFans();
  iceFanDutySum1+=iceFanDuty;
  iceFanDutySum5+=iceFanDuty;
  mainFanDutySum5+=mainFanDuty;
  counterAve1++;
  counterAve5++;
}
void calculateAverages1(){
  batteryVoltsSum1/=counterAve1;
  batteryVoltsAve1=batteryVoltsSum1;
  batteryVoltsSum1=0;
  chargerVoltsSum1/=counterAve1;
  chargerVoltsAve1=chargerVoltsSum1;
  chargerVoltsSum1=0;
  safeVoltsSum1/=counterAve1;
  safeVoltsAve1=safeVoltsSum1;
  safeVoltsSum1=0;
  iceFanDutySum1/=counterAve1;
  iceFanDutyAve1=iceFanDutySum1;
  iceFanDutySum1=0;
  tempExitSum1/=counterAve1;
  tempExitAve1=tempExitSum1;
  tempExitSum1=0;
  tempTopSum1/=counterAve1;
  tempTopAve1=tempTopSum1;
  tempTopSum1=0;
  ReadTempReturnDigital();
  ReadTempBatDigital();
  if(dsAmbientAvailable){
    tempReturnSum1/=counterAve1;
    tempReturnAve1=tempReturnSum1;
    tempReturnSum1=0;
  }else{
    tempReturnAve1=0;
    tempReturnSum1=0;
  }
  if(dsBatteryAvailable){
    tempBatSum1/=counterAve1;
    tempBatAve1=tempBatSum1;
    tempBatSum1=0;
  }else{
    tempBatAve1=0;
    tempBatSum1=0;
  }
  counterAve1=0;
}
void calculateAverages5(){
  batteryVoltsSum5/=counterAve5;
  batteryVoltsAve5=batteryVoltsSum5;
  batteryVoltsSum5=0;
  chargerVoltsSum5/=counterAve5;
  chargerVoltsAve5=chargerVoltsSum5;
  chargerVoltsSum5=0;
  safeVoltsSum5/=counterAve5;
  safeVoltsAve5=safeVoltsSum5;
  safeVoltsSum5=0;
  iceFanDutySum5/=counterAve5;
  iceFanDutyAve5=iceFanDutySum5;
  iceFanDutySum5=0;
  mainFanDutySum5/=counterAve5;
  mainFanDutyAve5=mainFanDutySum5;
  mainFanDutySum5=0;
  tempExitSum5/=counterAve5;
  tempExitAve5=tempExitSum5;
  tempExitSum5=0;
  tempTopSum5/=counterAve5;
  tempTopAve5=tempTopSum5;
  tempTopSum5=0;
  ReadTempReturnDigital();
  ReadTempBatDigital();
  if(dsAmbientAvailable){
    tempReturnSum5/=counterAve5;
    tempReturnAve5=tempReturnSum5;
    tempReturnSum5=0;
  }else{
    tempReturnAve5=0;
    tempReturnSum5=0;
  }
  if(dsBatteryAvailable){
    tempBatSum5/=counterAve5;
    tempBatAve5=tempBatSum5;
    tempBatSum5=0;
  }else{
    tempBatAve5=0;
    tempBatSum5=0;
  }
  DigitalTempsLog();  // Only logs every 50M millis
  counterAve5=0;
}

void CalcIceFanRPM(){
    // millisIceFanMaxTime is millis()
    // millisIceFanSpeed is newRPM
  if(unitState==1){
    if((intelTopUnit!=1 && iceFanDuty>0) || (intelTopUnit==1 && intelTopUnitFan==1)){
      // Seems we are active and the iceFan is requested
      if(firstIceFanSpeed==0                        // Only check on first spin
          || millis()-timeRPM>=600000               // or Every 10 minutes
          || millis()-intelTopTimeReplied<1500){    // Or if top fan is talking to us
        firstIceFanSpeed=1;
        // rpmLast=IceFanReadRPM();
        rpmLast=millisIceFanSpeed;
        if(rpmLast==0){
         #ifdef ECHO_DEBUG_COMMENTS
          Serial.println("-RPM stuck");
         #endif
          rpmLast/=16;
          rpmLastMax/=8;
          rpmLastMaxTime=millis();
        }else{
         #ifdef ECHO_DEBUG_COMMENTS
          Serial.print("-RPM ");
          Serial.println(rpmLast);
         #endif
          if(rpmLastMax<rpmLast){
            rpmLastMax=rpmLast;
          }
          rpmLastMaxTime=millis();
        }
        timeRPM=millis();
        if(millis()-timeRPMwritten>=600000){
          spibuf[1]='R';
          spibuf[2]='P';
          spibuf[3]='M';
          spibuf[4]='=';
          char cDelta='Z';
          if(iceFanCheckCount<10){
            cDelta='0'+iceFanCheckCount;
          }else if(iceFanCheckCount<25){
            cDelta='7'+iceFanCheckCount;
          }
          spibuf[11]=cDelta;
          cDelta='Z';
          unsigned long tDelta=millis()-millisIceFanMaxTime;
          tDelta/=10000;
          if(tDelta<10){
            cDelta='0'+tDelta;
          }else if(tDelta<31){
            cDelta='7'+tDelta;
          }
          spibuf[12]=cDelta;
          cDelta='Z';
          tDelta=millis()-timeLastFlashWrite;
          tDelta/=10000;
          if(tDelta<10){
            cDelta='0'+tDelta;
          }else if(tDelta<31){
            cDelta='7'+tDelta;
          }
          spibuf[13]=cDelta;
          if(rpmLast==0 && rpmLastReal==0 && rpmLastRealStuck>=5){  // Require two readings and 5 trys for a stuck
            spibuf[5]='s';
            spibuf[6]='t';
            spibuf[7]='u';
            spibuf[8]='c';
            spibuf[9]='k';
            spibuf[10]=' ';
            spibuf[14]=' ';
            rpmLastRealStuck=0;
          }else{
            if(rpmLast==0 && rpmLastReal==0){
              rpmLastRealStuck++;  // How many times have we hit this at zero RPM...
            }else{
              rpmLastRealStuck=0;
            }
            if(rpmLastReal>0){
              rpmSTimeLast=rpmLastReal;  // If the "real" reading is not zero perfer it
            }else{
              rpmSTimeLast=rpmLastMax;
            }
            spibuf[9]='0'+(rpmSTimeLast%10);
            rpmSTimeLast/=10;
            spibuf[8]='0'+(rpmSTimeLast%10);
            rpmSTimeLast/=10;
            spibuf[7]='0'+(rpmSTimeLast%10);
            rpmSTimeLast/=10;
            spibuf[6]='0'+(rpmSTimeLast%10);
            rpmSTimeLast/=10;
            spibuf[5]='0'+(rpmSTimeLast%10);
            spibuf[10]=' ';
            spibuf[14]=' ';
          }
          LogFlashDebug();
          timeRPMwritten=timeRPM;
          millisIceFanMaxTime=0;
          millisIceFanSpeed=0;
         #ifdef ECHO_DEBUG_COMMENTS1
          Serial.println(F("-logged"));
         #endif
        }
      }else{
        //Serial.print(F("-RPM sleep "));
        //Serial.print(millis()-timeRPM);  // >= 1200000
        //Serial.print(" ");
        //Serial.println(millis()-intelTopTimeReplied);  // < 1500
      }
    }else if(millis()-timeRPM>=1800000){
      // Been 30 minutes of active an no ICE fan requests
     #ifdef ECHO_DEBUG_COMMENTS
      Serial.println(F("-RPM zero"));
     #endif
      spibuf[1]='R';
      spibuf[2]='P';
      spibuf[3]='M';
      spibuf[4]='=';
      spibuf[5]='z';
      spibuf[6]='e';
      spibuf[7]='r';
      spibuf[8]='o';
      spibuf[9]=' ';
      spibuf[10]=' ';
      spibuf[11]=' ';
      spibuf[12]=' ';
      spibuf[13]=' ';
      spibuf[14]=' ';
      LogFlashDebug();
      timeRPM=millis();
      timeRPMwritten=timeRPM;
      rpmLast=0;
      rpmLastMax=0;
    }else{
     #ifdef ECHO_DEBUG_COMMENTS1
      Serial.println(F("-RPM off"));
     #endif
    }
  }else{
   #ifdef ECHO_DEBUG_COMMENTS1
    Serial.println(F("-RPM inactive"));
   #endif
    rpmLast=0;
    rpmLastMax=0;
  }
}

void PrintNowData(){
  Serial.print('\\');
  spibuf[0]=unitState;
  unsigned long tempLong=millis();
  spibuf[1]=(tempLong>>24)&0xff;
  spibuf[2]=(tempLong>>16)&0xff;
  spibuf[3]=(tempLong>>8)&0xff;
  spibuf[4]=tempLong&0xff;
  spibuf[5]=batteryVoltsAve1/100;
  spibuf[6]=iceFanDutyAve1;
  spibuf[7]=tempReturnAve1>>8;
  spibuf[8]=tempReturnAve1&0xff;
  spibuf[9]=tempExitAve1>>8;
  spibuf[10]=tempExitAve1&0xff;
  spibuf[11]=mainFanDuty;
  spibuf[12]=tempBatAve1>>8;
  spibuf[13]=tempBatAve1&0xff;
  byte tempByte1=0;
  if(tempReturnAve1==tempTopAve1){
    tempByte1=32;
  }else if(tempReturnAve1>tempTopAve1){
    if(tempReturnAve1-tempTopAve1>1500){
      tempByte1=1;
    }else{
      tempByte1=32-((tempReturnAve1-tempTopAve1)/50);
    }
  }else{
    if(tempTopAve1-tempReturnAve1>1500){
      tempByte1=63;
    }else{
      tempByte1=((tempTopAve1-tempReturnAve1)/50)+32;
    }
  }
  spibuf[14]=tempByte1<<2;
  spibuf[15]=3;
  PrintSPIBuf();
}
void PrintPositionStatus(){
  Serial.print('=');
  PrintStatus();
}
void RePrintPositionData(){
  if(flashOffsetRead>0){
    flashOffsetRead--;
    PrintPositionData();
  }else{
    Serial.print('~');
    PrintStatus();
  }
}
void PrintPositionData(){
  if(flashOffsetRead<flashOffsetWrite){
    ReadSPIBuf(flashOffsetRead);
    Serial.print('|');
    PrintSPIBuf();
    flashOffsetRead++;
  }else{
    Serial.print('~');
    PrintStatus();
  }
}
void PrintStatus(){
  Serial.print(UNIT_ID,HEX);
  Serial.print(' ');
  Serial.print(FIRMWARE_ID,HEX);
  Serial.print(' ');
  Serial.print(millis(),HEX);
  Serial.print(' ');
  Serial.print(flashOffsetRead,HEX);
  Serial.print(' ');
  Serial.print(flashOffsetWrite,HEX);
  Serial.print(' ');
  Serial.println(TEMP_TARGET,HEX);
}
void PrintSPIBuf(){
  byte tempByte=0;
  while(tempByte<15){
    if(spibuf[tempByte]<16){
      Serial.print('0');
    }
    Serial.print(spibuf[tempByte],HEX);
    Serial.print(' ');
    tempByte++;
  }
  if(spibuf[tempByte]<16){
    Serial.print('0');
  }
  Serial.println(spibuf[tempByte],HEX);
}
void PrintSPIBuf2(){
  byte tempByte=0;
  while(tempByte<15){
    if(spibuf[tempByte]<16){
      Serial.print('0');
    }
    Serial.print(spibuf[tempByte],HEX);
    Serial.print(' ');
    tempByte++;
  }
  if(spibuf[tempByte]<16){
    Serial.print('0');
  }
  Serial.print(spibuf[tempByte],HEX);
}
void PrintHelpMenu(int cmd){
 #ifdef ECHO_DEBUG_COMMENTS3
  Serial.print(F("-Help Menu: "));
  Serial.println(cmd);
  Serial.println(F("- L = Live data enabled"));
  Serial.println(F("- l = Live data off"));
  Serial.println(F("- D = Dump all flash data"));
  Serial.println(F("- E r a = Erase all flash logs"));
 #endif
}
void PrintAverages1(){
  Serial.print(millis());
  Serial.print(',');
  Serial.print(batteryVoltsAve1);
  Serial.print(',');
  Serial.print(iceFanDutyAve1);
  Serial.print(',');
  Serial.print(tempReturnAve1);
  Serial.print(',');
  Serial.print(tempExitAve1);
  Serial.print(',');
  Serial.print(tempTopAve1);
  Serial.print(',');
  Serial.print(mainFanDuty);
  Serial.print(',');
  Serial.print(tempBatAve1);
  Serial.print(',');
  Serial.print(chargerVoltsAve1);
  Serial.print(',');
  Serial.print(tempReturnAve1);
  Serial.print(',');
  Serial.print(safeVoltsAve1);
  Serial.print(',');
  Serial.print(dsAmbientAvailable);
  Serial.print(',');
  Serial.print(tempRetDigital);
  Serial.print(',');
  Serial.print(dsBatteryAvailable);
  Serial.print(',');
  Serial.println(tempBatDigital);
}
void PrintAverages5(){
  Serial.print(millis());
  Serial.print(',');
  Serial.print(batteryVoltsAve5);
  Serial.print(',');
  Serial.print(iceFanDutyAve5);
  Serial.print(',');
  Serial.print(tempReturnAve5);
  Serial.print(',');
  Serial.print(tempExitAve5);
  Serial.print(',');
  Serial.print(tempTopAve5);
  Serial.print(',');
  Serial.print(mainFanDuty);
  Serial.print(',');
  Serial.print(tempBatAve5);
  Serial.print(',');
  Serial.print(chargerVoltsAve5);
  Serial.print(',');
  Serial.print(tempReturnAve5);
  Serial.print(',');
  Serial.print(safeVoltsAve5);
  Serial.print(',');
  Serial.print(dsAmbientAvailable);
  Serial.print(',');
  Serial.print(tempRetDigital);
  Serial.print(',');
  Serial.print(dsBatteryAvailable);
  Serial.print(',');
  Serial.println(tempBatDigital);
}

void PrintNowCRC(){
  //^UNIT_ID,FIRMWARE_ID,millis(),flashOffsetRead,flashOffsetWrite,TEMP_TARGET,batteryVolts,batteryTemp,ambientTemp,unitState,epoch,...;checksum XOR
  unsigned long checksum=0;
  byte crcxor=0;
  Serial.print('^');
  Serial.print(UNIT_ID);
  checksum+=SumDigits(UNIT_ID); crcxor^=CRCDigits(UNIT_ID);
  Serial.print(',');
  Serial.print(FIRMWARE_ID);
  checksum+=SumDigits(FIRMWARE_ID); crcxor^=CRCDigits(FIRMWARE_ID);
  Serial.print(',');
  unsigned long mnow=millis();
  Serial.print(mnow);
  checksum+=SumDigits(mnow); crcxor^=CRCDigits(mnow);
  Serial.print(',');
  Serial.print(flashOffsetRead);
  checksum+=SumDigits(flashOffsetRead); crcxor^=CRCDigits(flashOffsetRead);
  Serial.print(',');
  Serial.print(flashOffsetWrite);
  checksum+=SumDigits(flashOffsetWrite); crcxor^=CRCDigits(flashOffsetWrite);
  Serial.print(',');
  Serial.print(TEMP_TARGET);
  checksum+=SumDigits(TEMP_TARGET); crcxor^=CRCDigits(TEMP_TARGET);
  Serial.print(',');
  Serial.print(batteryVoltsAve1);
  checksum+=SumDigits(batteryVoltsAve1); crcxor^=CRCDigits(batteryVoltsAve1);
  Serial.print(',');
  Serial.print(tempBatAve1);
  checksum+=SumDigits(tempBatAve1); crcxor^=CRCDigits(tempBatAve1);
  Serial.print(',');
  Serial.print(tempReturnAve1);
  checksum+=SumDigits(tempReturnAve1); crcxor^=CRCDigits(tempReturnAve1);
  Serial.print(',');
  Serial.print(unitState);
  checksum+=SumDigits(unitState); crcxor^=CRCDigits(unitState);
  Serial.print(',');
  Serial.print(epochTime);
  checksum+=SumDigits(epochTime); crcxor^=CRCDigits(epochTime);
  Serial.print(';');
  Serial.print(checksum);
  Serial.print(' ');
  Serial.println(crcxor,HEX);
}
void PrintFlashCRC(){
  //(offset,XX XX XX...XX;checksum XOR
  unsigned long checksum=0;
  byte crcxor=0;
  Serial.print('(');
  Serial.print(flashOffsetRead);
  checksum+=SumDigits(flashOffsetRead); crcxor^=CRCDigits(flashOffsetRead);
  Serial.print(',');
  ReadSPIBuf(flashOffsetRead);
  PrintSPIBuf2();
  
  byte tempByte=0;
  while(tempByte<16){
    checksum+=SumDigits(spibuf[tempByte]); crcxor^=CRCDigits(spibuf[tempByte]);
    tempByte++;
  }
  
  Serial.print(';');
  Serial.print(checksum);
  Serial.print(' ');
  Serial.println(crcxor,HEX);
  if(flashOffsetRead<flashOffsetWrite){
    flashOffsetRead++;
  }
}
void RePrintFlashCRC(){
  if(flashOffsetRead>0){
    flashOffsetRead--;
  }
  PrintFlashCRC();
}

void PrintVCCTemp(){
  long result;
  byte muxTemp=ADMUX; // Save ADMUX before hand
  Serial.print(F("-VCC reads "));
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(10); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while(bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  Serial.print(result);
  Serial.print(F("mv "));
  result*=3300;
  result/=5642;
  Serial.print(result);
  Serial.print(F("x"));
  Serial.print(VOLT_SCALER);
  Serial.print(F(" and DieTemp reads "));
  // Read temperature sensor against 1.1V reference
  ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(MUX3);
  delay(10); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while(bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = (result - 125) * 1075;
  result*=9;
  result/=5000;
  result+=320;
  ADMUX=muxTemp; // Restore ADMUX after
  delay(10);
  Serial.print(result);
  Serial.println(F("F x10"));
  //Serial.println(F("F x10 and VOLT_OFFSET is "));
  //Serial.print(VOLT_OFFSET);
  //Serial.println(F("mv"));
}

void ReadVCCTemp(){
  byte volt_read=EEPROM.read(EEPROM_VOLT_OFFSET);
  if(volt_read>=30 && volt_read<=230){
    VOLT_SCALER=1800;
    VOLT_SCALER+=volt_read;
    Serial.print(F("-VOLT_SCALER "));
    Serial.println(VOLT_SCALER);
  }else{
    Serial.print(F("-VOLT_SCALER not set "));
    Serial.print(volt_read);
    Serial.print(F(" using default "));
    VOLT_SCALER=1945;
    Serial.println(VOLT_SCALER);
  }
}
void DecVCCTemp(){
  byte volt_read=EEPROM.read(EEPROM_VOLT_OFFSET);
  if(volt_read>30 && volt_read<=230){
    volt_read--;
    Serial.print(F("--VOLT_SCALER "));
    Serial.println(volt_read);
    EEPROM.write(EEPROM_VOLT_OFFSET,volt_read);
  }else{
    EEPROM.write(EEPROM_VOLT_OFFSET,145);
  }
  ReadVCCTemp();
}

void IncVCCTemp(){
  byte volt_read=EEPROM.read(EEPROM_VOLT_OFFSET);
  if(volt_read>=30 && volt_read<230){
    volt_read++;
    Serial.print(F("-+VOLT_SCALER "));
    Serial.println(volt_read);
    EEPROM.write(EEPROM_VOLT_OFFSET,volt_read);
  }else{
    EEPROM.write(EEPROM_VOLT_OFFSET,145);
  }
  ReadVCCTemp();
}
void PrintVCCTemp2(){
  long result;
  byte muxTemp=ADMUX; // Save ADMUX before hand
  Serial.print(F("-VDD reads "));
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(10); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while(bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  Serial.print(result);
  Serial.print(F("mv "));
  result*=3300;
  result/=5642;
  Serial.print(result);
  Serial.print(F("x"));
  Serial.print(VOLT_SCALER2);
  Serial.print(F(" and DieTemp reads "));
  // Read temperature sensor against 1.1V reference
  ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(MUX3);
  delay(10); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while(bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = (result - 125) * 1075;
  result*=9;
  result/=5000;
  result+=320;
  ADMUX=muxTemp; // Restore ADMUX after
  delay(10);
  Serial.print(result);
  Serial.println(F("F x10"));
  //Serial.println(F("F x10 and VOLT_OFFSET is "));
  //Serial.print(VOLT_OFFSET);
  //Serial.println(F("mv"));
}

void ReadVCCTemp2(){
  byte volt_read=EEPROM.read(EEPROM_VOLT_OFFSET2);
  if(volt_read>=30 && volt_read<=230){
    VOLT_SCALER2=1800;
    VOLT_SCALER2+=volt_read;
    Serial.print(F("-VOLT_SCALER2 "));
    Serial.println(VOLT_SCALER2);
  }else{
    Serial.print(F("-VOLT_SCALER2 not set "));
    Serial.print(volt_read);
    Serial.print(F(" using default "));
    VOLT_SCALER2=1950;
    Serial.println(VOLT_SCALER2);
  }
}
void DecVCCTemp2(){
  byte volt_read=EEPROM.read(EEPROM_VOLT_OFFSET2);
  if(volt_read>30 && volt_read<=230){
    volt_read--;
    Serial.print(F("--VOLT_SCALER2 "));
    Serial.println(volt_read);
    EEPROM.write(EEPROM_VOLT_OFFSET2,volt_read);
  }else{
    EEPROM.write(EEPROM_VOLT_OFFSET2,150);
  }
  ReadVCCTemp2();
}

void IncVCCTemp2(){
  byte volt_read=EEPROM.read(EEPROM_VOLT_OFFSET2);
  if(volt_read>=30 && volt_read<230){
    volt_read++;
    Serial.print(F("-+VOLT_SCALER2 "));
    Serial.println(volt_read);
    EEPROM.write(EEPROM_VOLT_OFFSET2,volt_read);
  }else{
    EEPROM.write(EEPROM_VOLT_OFFSET2,150);
  }
  ReadVCCTemp2();
}
byte SumDigits(unsigned long input){
  byte ret=0;
  while(input>0){
    ret+=input%10;
    input/=10;
  }
  return ret;
}
byte CRCDigits(unsigned long input){
  byte ret=0;
  while(input>0){
    ret^=input%256;
    input/=256;
  }
  return ret;
}
