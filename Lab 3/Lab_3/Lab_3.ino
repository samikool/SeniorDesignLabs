#include "EEPROM.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "TouchScreen.h"

int CS = 10;
int DC = 9;
Adafruit_ILI9341 tft = Adafruit_ILI9341(CS, DC);

// These are the four touchscreen analog pins
int YP = A2;  // must be an analog pin, use "An" notation!
int XM = A3;  // must be an analog pin, use "An" notation!
int YM = 7;   // can be a digital pin
int XP = 8;   // can be a digital pin

// This is calibration data for the raw touch data to the screen coordinates
int TS_MINX = 150;
int TS_MINY = 120;
int TS_MAXX = 920;
int TS_MAXY = 940;

int MINPRESSURE = 10;
int MAXPRESSURE = 1000;

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

int acPin = 2; 
int heatPin = 3;
int powerPin = A1;


/*Time Struct*/
struct Time{
  int hour; //hour
  int minute; //minute
  double second; //seconds
  bool am;
};

enum Day {
  monday,
  tuesday,
  wednesday,
  thursday,
  friday,
  saturday,
  sunday,
};

/*LCD State*/
enum PossibleLCDStates {
  off,
  powersaving,
  main,
  setpointDisplay,
  setpointEdit,
  timedateEdit,
};
PossibleLCDStates LCDState = main;

/*HVAC Mode State*/
enum PossibleHVACModes{
  none,
  cool,
  heat,
  automatic
};
PossibleHVACModes HVACMode = cool;
PossibleHVACModes lastHVACMode = none;

/*HVAC Power State*/
enum PossibleHVACStatus{
  coolOn, 
  heatOn,
  nothingOn
};
PossibleHVACStatus HVACStatus = nothingOn;
PossibleHVACStatus lastHVACStatus = nothingOn;

/*Date Struct*/
struct Date{
  Day weekday;
  int day;
  int month;
  int year;
  bool isWeekday;
};

//Setpoint Struct
struct Setpoint{
  int temp; //temp of setpoint
  Time startTime; //start time of setpoint
  Time stopTime; //stop time of setpoint
  PossibleHVACModes HVACMode; //HVACMode
  bool isWeekday;
  bool enabled;
  bool isActive;
};

//Setpoint List Struct
struct SetpointList{
  Setpoint weekdayOne;
  Setpoint weekdayTwo;
  Setpoint weekdayThree;
  Setpoint weekdayFour;

  Setpoint weekendOne;
  Setpoint weekendTwo;
  Setpoint weekendThree;
  Setpoint weekendFour;
};

SetpointList setpointList = {{70,{0,0,0,true},{6,0,0,true},automatic,true,false,false}, 
  {70,{6,0,0,true},{12,0,0,false},none,true,false,false}, 
  {70,{12,0,0,false},{18,0,0,false},none,true,false,false}, 
  {70,{18,0,0,false},{0,0,0,true},none,true,false,false}, 
  {70,{0,0,0,true},{6,0,0,true},none,false,false,false}, 
  {70,{6,0,0,true},{12,0,0,false},none,false,false,false}, 
  {70,{12,0,0,false},{18,0,0,false},none,false,false,false}, 
  {70,{18,0,0,false},{0,0,0,true},none,false,false,false}};

//Hold
bool hold = true;

/*General Data*/
int currentTemp = 70;
int currentTargetTemp = 0;
int lastTargetTemp = 0;
int localTargetTemp = 60;
PossibleHVACModes localTargetHVACMode = cool;
int tempBuffer = 3;
Time currentTime = {23,59,55,false};
Time localTime = {0,0,0,false};
Time lastTouch = currentTime;
Date currentDate = {thursday, 12, 12, 2019, true};
Date localDate = {monday, 0, 0, 2019, true};
Setpoint localSetpoint = {70,{0,0,0,true},{6,0,0,true},automatic,true,false,false};

unsigned long loopStart = 0;
unsigned long loopTime = 0;

/*Bools for time to determine what part of time to update to keep performance up*/
bool updateSecond = true;
bool updateMinute = true;
bool updateHour = true;
bool updateWeekday = true;
bool updateDay= true;
bool updateMonth = true;
bool updateYear = true;
bool updateHold = true;
bool updateSetpoint = true;
bool updateHVACStatus = true;
bool updateHVACMode = true;
int previousSecond = currentTime.second;

/*Bools for set edit screen*/
bool updateSetStart = true;
bool updateSetMinute = true;
bool updateSetHour = true;
bool updateSetTemp = true;
bool updateSetMode = true;
bool updateSetEnable = true;

bool setStartWanted = true;
bool setStartSelected = true;
bool setMinuteSelected = false;
bool setHourSelected = false;
bool setModeSelected = false;
bool setEnableSelected = false;
bool setTempSelected = false;

bool setpointActive = false;
bool updateSetpointActive = true;
bool lastUpdateSetpointActive = updateSetpointActive;

int selectedSetpoint = 1;
bool setWeekdaySelected = true;
bool updateSetpointTable = false;

/*Bools for determining what part of date and time is selected on timedateEdit screen*/
bool minuteSelected = true;
bool hourSelected = false;
bool weekdaySelected = false;
bool daySelected = false;
bool monthSelected = false;
bool yearSelected = false;

TSPoint p = ts.getPoint();

char zero[3] = {'0', ' ', ':'};


void setup(){
    Serial.begin(2000000);
    pinMode(acPin, OUTPUT);
    pinMode(heatPin, OUTPUT);
    pinMode(powerPin, INPUT);

    tft.begin();
}

void loop(){
  loopStart = micros();
   
  while(analogRead(powerPin) > 500){
      tft.fillScreen(ILI9341_BLACK);
      digitalWrite(acPin, LOW);
      digitalWrite(heatPin, LOW);
      LCDState = off;

  }

  p = ts.getPoint();
    if(p.z > 0 && p.z < 1000 && p.x > 100){
        lastTouch = currentTime;
        if(LCDState == main){
            //go to timedateEdit
            if(140 < p.x && p.x < 240 && 120 < p.y && p.y < 400){
                LCDState = timedateEdit;         
                clearLCD();
                localTime = currentTime;
                localDate = currentDate;
                updateHold = true;
                updateSetpoint = true;
            }
            //go to setpointEdit
            else if( 400 < p.x && p.x < 530 && 125 < p.y && p.y < 310){
                LCDState = setpointDisplay;
                clearLCD();
                updateHold = true;
                updateSetpoint = true;
            }
            //Hold toggled
            else if(265 < p.x && p.x < 370 &&  130 < p.y && p.y < 315){
                if(hold == false){hold = true;}
                else{hold = false;}
                updateHold = true;
                updateSetpointActive = true;
            }
            //plus button
            else if(756 < p.x && p.x < 835 && 215 < p.y && p.y < 300 ){
                if(localTargetTemp < 99){
                    localTargetTemp += 1;
                    updateSetpoint = true;
                }
                
            }
            //minus button
            else if(750 < p.x && p.x < 850 && 120 < p.y && p.y < 220){
                if(localTargetTemp > 55){
                    localTargetTemp -= 1;
                    updateSetpoint = true;
                }   
            }

            //nextMode button
            else if(735 < p.x && p.x < 805 && 390 < p.y && p.y < 450){
                localTargetHVACMode = nextMode(localTargetHVACMode);
                updateHVACMode = true;
            }
            //previousMode button
            else if(700 < p.x && p.x < 805 && 570 < p.y && p.y < 680){
                for(int i=0; i<3; i++){
                    localTargetHVACMode = nextMode(localTargetHVACMode);
                }
                updateHVACMode = true;
            }
            
        }
       
        else if(LCDState == timedateEdit){
            //Back button
            if(120 < p.x && p.x < 240 && 660 < p.y && p.y < 900){
                LCDState = main;
                clearLCD();
                updateSetpointActive = true;
            }
            //select minute
            else if(248 < p.x && p.x < 305 && 267 < p.y && p.y < 670){
                clearTimeSelection();
                minuteSelected = true;
            } 
            //select hour
            else if(360 < p.x && p.x < 420 && 267 < p.y && p.y < 670){
                clearTimeSelection();
                hourSelected = true;
            }
            //select Weekday
            else if(460 < p.x && p.x < 530 && 267 < p.y && p.y < 670){
                clearTimeSelection();
                weekdaySelected = true;
            }
            //select Month
            else if(555 < p.x && p.x < 635 && 267 < p.y && p.y < 670){
                clearTimeSelection();
                monthSelected = true;
            }
            //select Day
            else if(655 < p.x && p.x < 745 && 267 < p.y && p.y < 670){
                clearTimeSelection();
                daySelected = true;
            }
            //select Year
            else if(760 < p.x && p.x < 840 && 267 < p.y && p.y < 670){
                clearTimeSelection();
                yearSelected = true;
            }
            
            
            //plus button
            else if(430 < p.x && p.x < 530 && 130 < p.y && p.y < 210){
                if(minuteSelected){
                    localTime.minute += 1;
                    if(localTime.minute == 60){
                        localTime.minute = 0;
                    }
                    updateMinute = true;
                }
                else if(hourSelected){
                    localTime.hour += 1;
                    if(localTime.hour == 24){
                        localTime.hour = 0;
                        localTime.am = true;
                    }

                    if(localTime.hour == 12){
                        localTime.am = false;
                    }
                    updateHour = true;
                }

                else if(weekdaySelected){
                    localDate.weekday = nextDay(localDate.weekday);
                    updateWeekday = true;
                }

                else if(monthSelected){
                    localDate.month += 1;
                    if(localDate.month == 13){
                        localDate.month = 1;              
                    }
                    
                    if((localDate.month % 2 == 0 && localDate.month != 2) && localDate.day > 30){
                        localDate.day = 30;
                        updateDay = true;
                    }
                    else if(localDate.month == 2 && localDate.day > 28){
                        localDate.day = 28;
                        updateDay = true;
                    }  
                    
                    updateMonth = true;
                }

                else if(daySelected){
                    localDate.day += 1;
                    if((localDate.month % 2 == 1 || localDate.month == 7) && localDate.day == 32){
                        localDate.day = 1;
                    }
                    else if((localDate.month % 2 == 0 && localDate.month != 2) && localDate.day == 31){
                        localDate.day = 1;
                    }
                    else if(localDate.month == 2 && localDate.day == 29){
                        localDate.day = 1;
                    }
                    updateDay = true;
                }

                else if(yearSelected){
                    localDate.year += 1;
                    if(localDate.year == 9999){
                        localDate.year = 1000;
                    }
                    updateYear = true;                
                }      
            }
            //minus button
            else if(415 < p.x && p.x < 550 && 750 < p.y && p.y < 810){
                if(minuteSelected){
                    localTime.minute -= 1;
                    if(localTime.minute == -1){
                        localTime.minute = 59;
                    }
                    updateMinute = true;
                }
                else if(hourSelected){
                    localTime.hour -= 1;
                    if(localTime.hour == -1){
                        localTime.hour = 23;
                        localTime.am = false;
                    } 

                    if(localTime.hour == 11){
                        localTime.am = true;
                    }
                    updateHour = true;
                }
                else if(weekdaySelected){
                    for(int i=0; i <6; i++){
                        localDate.weekday = nextDay(localDate.weekday);
                    }
                    updateWeekday = true;
                }
                else if(monthSelected){
                    localDate.month -= 1;
                    if(localDate.month == 0){
                        localDate.month = 12;                      
                    }
                    
                    if((localDate.month % 2 == 0 && localDate.month != 2) && localDate.day > 30){
                        localDate.day = 30;
                        updateDay = true;
                    }
                    else if(localDate.month == 2 && localDate.day > 28){
                        localDate.day = 28;
                        updateDay = true;
                    }  
                    
                    updateMonth = true;
                }
                 else if(daySelected){
                    localDate.day -= 1;
                    if((localDate.month % 2 == 1 || localDate.month == 7) && localDate.day == 0){
                        localDate.day = 31;
                    }
                    else if((localDate.month % 2 == 0 && localDate.month != 2) && localDate.day == 0){
                        localDate.day = 30;
                    }
                    else if(localDate.month == 2 && localDate.day == 0){
                        localDate.day = 28;
                    }
                    updateDay = true;
                }

                else if(yearSelected){
                    localDate.year -= 1;
                    if(localDate.year == 1000){
                        localDate.year = 9999;
                    }
                    updateYear = true; 
                }
            }
            else if(170 < p.x && p.x < 240 && 120 < p.y && p.y < 240){
                currentTime = localTime;
                currentDate = localDate;
                currentTime.second = 0;
                LCDState = main;  
                clearLCD();
            }
        }
        else if(LCDState == setpointEdit){
            //Back button
            if(120 < p.x && p.x < 240 && 660 < p.y && p.y < 900){
                LCDState = setpointDisplay;
                clearLCD();
            }
            //select time
            else if(248 < p.x && p.x < 305 && 267 < p.y && p.y < 670){
                clearSetSelection();
                setStartSelected = true;
            } 
            //select minute
            else if(360 < p.x && p.x < 420 && 267 < p.y && p.y < 670){
                clearSetSelection();
                setMinuteSelected = true;
            }
            //select hour
            else if(460 < p.x && p.x < 530 && 267 < p.y && p.y < 670){
                clearSetSelection();
                setHourSelected = true;
            }
            //select temp
            else if(555 < p.x && p.x < 635 && 267 < p.y && p.y < 670){
                clearSetSelection();
                setTempSelected = true;
            }
            //select mode
            else if(655 < p.x && p.x < 745 && 267 < p.y && p.y < 670){
                clearSetSelection();
                setModeSelected = true;
            }
            //select enabled
            else if(760 < p.x && p.x < 840 && 267 < p.y && p.y < 670){
                clearSetSelection();
                setEnableSelected = true;
            }
            
            
            //plus button
            else if(430 < p.x && p.x < 530 && 130 < p.y && p.y < 210){
                if(setStartSelected){
                    if(setStartWanted){
                        setStartWanted = false;
                    }else{
                        setStartWanted = true;
                    }
                    updateSetStart = true;
                    updateSetMinute = true;
                    updateSetHour = true;
                }
                else if(setMinuteSelected && setStartWanted){
                    localSetpoint.startTime.minute += 1;
                    if(localSetpoint.startTime.minute == 60){
                        localSetpoint.startTime.minute = 0;
                    }
                    updateSetMinute = true;
                }
                else if(setMinuteSelected && !setStartWanted){
                    localSetpoint.stopTime.minute += 1;
                    if(localSetpoint.stopTime.minute == 60){
                        localSetpoint.stopTime.minute = 0;
                    }
                    updateSetMinute = true;
                }
                
                else if(setHourSelected && setStartWanted){
                    localSetpoint.startTime.hour += 1;
                    if(localSetpoint.startTime.hour == 24){
                        localSetpoint.startTime.hour = 0;
                        localSetpoint.startTime.am = true;
                    }

                    if(localSetpoint.startTime.hour == 12){
                        localSetpoint.startTime.am = false;
                    }
                    updateSetHour = true;
                }
                else if(setHourSelected && !setStartWanted){
                    localSetpoint.stopTime.hour += 1;
                    if(localSetpoint.stopTime.hour == 24){
                        localSetpoint.stopTime.hour = 0;
                        localSetpoint.stopTime.am = true;
                    }

                    if(localSetpoint.stopTime.hour == 12){
                        localSetpoint.stopTime.am = false;
                    }
                    updateSetHour = true;
                }

                else if(setTempSelected){
                    localSetpoint.temp += 1;
                    if(localSetpoint.temp == 100){
                        localSetpoint.temp = 55;
                    }
                    updateSetTemp = true;
                }

                else if(setModeSelected){
                    localSetpoint.HVACMode = nextMode(localSetpoint.HVACMode);
                    updateSetMode = true;
                }

                else if(setEnableSelected){
                    if(localSetpoint.enabled){
                        localSetpoint.enabled = false;
                    }else{
                        localSetpoint.enabled = true;
                    }
                    updateSetEnable = true;
                }
            }
            //minus button
            else if(415 < p.x && p.x < 550 && 750 < p.y && p.y < 810){
                if(setStartSelected){
                    if(setStartWanted){
                        setStartWanted = false;
                    }else{
                        setStartWanted = true;
                    }
                    updateSetStart = true;
                    updateSetMinute = true;
                    updateSetHour = true;
                }
                else if(setMinuteSelected && setStartWanted){
                    localSetpoint.startTime.minute -= 1;
                    if(localSetpoint.startTime.minute == -1){
                        localSetpoint.startTime.minute = 59;
                    }
                    updateSetMinute = true;
                }
                else if(setMinuteSelected && !setStartWanted){
                    localSetpoint.stopTime.minute -= 1;
                    if(localSetpoint.stopTime.minute == -1){
                        localSetpoint.stopTime.minute = 59;
                    }
                    updateSetMinute = true;
                }
                
                else if(setHourSelected && setStartWanted){
                    localSetpoint.startTime.hour -= 1;
                    if(localSetpoint.startTime.hour == -1){
                        localSetpoint.startTime.hour = 23;
                        localSetpoint.startTime.am = false;
                    }

                    if(localSetpoint.startTime.hour == 11){
                        localSetpoint.startTime.am = true;
                    }
                    updateSetHour = true;
                }
                else if(setHourSelected && !setStartWanted){
                    localSetpoint.stopTime.hour -= 1;
                    if(localSetpoint.stopTime.hour == -1){
                        localSetpoint.stopTime.hour = 23;
                        localSetpoint.stopTime.am = true;
                    }

                    if(localSetpoint.stopTime.hour == 12){
                        localSetpoint.stopTime.am = false;
                    }
                    updateSetHour = true;
                }
                
                else if(setTempSelected){
                    localSetpoint.temp -= 1;
                    if(localSetpoint.temp == 54){
                        localSetpoint.temp = 99;
                    }
                    updateSetTemp = true;
                }

                else if(setModeSelected){
                    for(int i=0; i<3; i++){
                        localSetpoint.HVACMode = nextMode(localSetpoint.HVACMode);
                    }
                    updateSetMode = true;
                }

                else if(setEnableSelected){
                    if(localSetpoint.enabled){
                        localSetpoint.enabled = false;
                    }else{
                        localSetpoint.enabled = true;
                    }
                    updateSetEnable = true;
                }
            }
            //save button
            else if(170 < p.x && p.x < 240 && 120 < p.y && p.y < 240){
                if(setpointValid(localSetpoint)){
                    if(selectedSetpoint == 1 && setWeekdaySelected){
                        setpointList.weekdayOne = localSetpoint;
                    }
                    else if(selectedSetpoint == 2 && setWeekdaySelected){
                        setpointList.weekdayTwo = localSetpoint;
                    }
                    else if(selectedSetpoint == 3 && setWeekdaySelected){
                        setpointList.weekdayThree = localSetpoint;
                    }
                    else if(selectedSetpoint == 4 && setWeekdaySelected){
                        setpointList.weekdayFour = localSetpoint;
                    }
                    else if(selectedSetpoint == 1 && !setWeekdaySelected){
                        setpointList.weekendOne = localSetpoint;
                    }
                    else if(selectedSetpoint == 2 && !setWeekdaySelected){
                        setpointList.weekendTwo = localSetpoint;
                    }
                    else if(selectedSetpoint == 3 && !setWeekdaySelected){
                        setpointList.weekendThree = localSetpoint;
                    }
                    else if(selectedSetpoint == 4 && !setWeekdaySelected){
                        setpointList.weekendFour = localSetpoint;
                    }
                    LCDState = setpointDisplay;
                    clearLCD();
                }else{
                    tft.fillScreen(ILI9341_RED);
                    tft.setTextColor(ILI9341_BLACK);
                    tft.setRotation(3);
                    tft.setCursor(30,50);
                    tft.setTextSize(3);
                    tft.println("Setpoint Invalid");
                    tft.setTextSize(2);
                    tft.println("Setpoint conflicts with another setpoint");
                    delay(5000);
                    clearLCD();
                    tft.setRotation(0);
                }
              
                
            }
        }
        else if(LCDState == setpointDisplay){
            //Back button
            if(120 < p.x && p.x < 240 && 660 < p.y && p.y < 900){
                LCDState = main;
                clearLCD();
                updateSetpointActive = true;
            }
            
            else if(225 < p.x && p.x < 305 && 195 < p.y && p.y < 410){
                setWeekdaySelected = false;
                updateSetpointTable = true;
            }
             else if(230 < p.x && p.x < 290 && 455 < p.y && p.y < 700){
                setWeekdaySelected = true;
                updateSetpointTable = true;
            }
            else if(400 < p.x && p.x < 500 && 120 < p.y && p.y < 850){
                selectedSetpoint = 1;
            }
            else if(515 < p.x && p.x < 630 && 120 < p.y && p.y < 850){
                selectedSetpoint = 2;
            }
            else if(670 < p.x && p.x < 760 && 120 < p.y && p.y < 850){
                selectedSetpoint = 3;
            }
            else if(790 < p.x && p.x < 865 && 120 < p.y && p.y < 850){
                selectedSetpoint = 4;
            }
            else if(155 < p.x && p.x < 200 && 105 < p.y && p.y < 235){
                if(setWeekdaySelected){
                    if(selectedSetpoint == 1){
                        localSetpoint = setpointList.weekdayOne;
                    }
                    else if(selectedSetpoint == 2){
                        localSetpoint = setpointList.weekdayTwo;
                    }
                    else if(selectedSetpoint == 3){
                        localSetpoint = setpointList.weekdayThree;
                    }
                    else if(selectedSetpoint == 4){
                        localSetpoint = setpointList.weekdayFour;
                    }
                }else{
                    if(selectedSetpoint == 1){
                        localSetpoint = setpointList.weekendOne;
                    }
                    else if(selectedSetpoint == 2){
                        localSetpoint = setpointList.weekendTwo;
                    }
                    else if(selectedSetpoint == 3){
                        localSetpoint = setpointList.weekendThree;
                    }
                    else if(selectedSetpoint == 4){
                        localSetpoint = setpointList.weekendFour;
                    }
                }
                LCDState = setpointEdit;
                clearLCD();
            }
        }
        else if(LCDState == off || LCDState == powersaving){
            LCDState = main;
            clearLCD();
            updateHold = true;
            updateSetpoint = true;
            updateSetpointActive = true;
        }
        delay(200);
    }

    if(LCDState != off){
    if(hold){
      adjustTemp(localTargetTemp);
      adjustHVACMode(localTargetHVACMode);
      setpointActive = false;
    }
    else{
      if(currentDate.isWeekday){
        if(inSetpoint(currentTime, setpointList.weekdayOne) && setpointList.weekdayOne.enabled){
          adjustHVACMode(setpointList.weekdayOne.HVACMode);
          adjustTemp(setpointList.weekdayOne.temp);
          setpointActive = true;
        }
        else if(inSetpoint(currentTime, setpointList.weekdayTwo) && setpointList.weekdayTwo.enabled){
          adjustHVACMode(setpointList.weekdayTwo.HVACMode);
          adjustTemp(setpointList.weekdayTwo.temp);
          setpointActive = true;
        }
        else if(inSetpoint(currentTime, setpointList.weekdayThree) && setpointList.weekdayThree.enabled){
          adjustHVACMode(setpointList.weekdayThree.HVACMode);
          adjustTemp(setpointList.weekdayThree.temp);
          setpointActive = true;
        }
        else if(inSetpoint(currentTime, setpointList.weekdayFour) && setpointList.weekdayFour.enabled){
          adjustHVACMode(setpointList.weekdayFour.HVACMode);
          adjustTemp(setpointList.weekdayFour.temp);
          setpointActive = true;
        }
        else{
          adjustTemp(localTargetTemp);
          adjustHVACMode(localTargetHVACMode);
          setpointActive = false;
        }
      }
      else{
        if(inSetpoint(currentTime, setpointList.weekendOne) && setpointList.weekendOne.enabled){
          adjustHVACMode(setpointList.weekendOne.HVACMode);
          adjustTemp(setpointList.weekendOne.temp);
          setpointActive = true;
        }
        else if(inSetpoint(currentTime, setpointList.weekendTwo) && setpointList.weekendTwo.enabled){
          adjustHVACMode(setpointList.weekendTwo.HVACMode);
          adjustTemp(setpointList.weekendTwo.temp);
          setpointActive = true;
        }
        else if(inSetpoint(currentTime, setpointList.weekendThree) && setpointList.weekendThree.enabled){
          adjustHVACMode(setpointList.weekendThree.HVACMode);
          adjustTemp(setpointList.weekendThree.temp);
          setpointActive = true;
        }
        else if(inSetpoint(currentTime, setpointList.weekendFour) && setpointList.weekendFour.enabled){
          adjustHVACMode(setpointList.weekendFour.HVACMode);
          adjustTemp(setpointList.weekendFour.temp);
          setpointActive = true;
        }
        else{
            adjustTemp(localTargetTemp);
            adjustHVACMode(localTargetHVACMode);
            setpointActive = false;
        }
      }
    }
  }

    if(lastTouch.hour == currentTime.hour && currentTime.minute - lastTouch.minute == 1 && (int) currentTime.second - (int) lastTouch.second == 0){
        LCDState = powersaving;
    }
    
    redrawLCD();
    loopTime = micros() - loopStart;
    addTime(loopTime);
}

bool setpointValid(Setpoint setpoint){
    if(setWeekdaySelected){
        if((inSetpoint(setpoint.startTime,setpointList.weekdayOne) || inSetpoint(setpoint.stopTime,setpointList.weekdayOne)) 
        && (inSetpoint(setpointList.weekdayOne.startTime, setpoint) || inSetpoint(setpointList.weekdayOne.stopTime, setpoint))
        && setpointList.weekdayOne.enabled){
            return false;
        }
        else if((inSetpoint(setpoint.startTime,setpointList.weekdayTwo) || inSetpoint(setpoint.stopTime,setpointList.weekdayTwo)) 
        && (inSetpoint(setpointList.weekdayTwo.startTime, setpoint) || inSetpoint(setpointList.weekdayTwo.stopTime, setpoint))
        && setpointList.weekdayTwo.enabled){
            return false;
        }
        else if((inSetpoint(setpoint.startTime,setpointList.weekdayThree) || inSetpoint(setpoint.stopTime,setpointList.weekdayThree)) 
        && (inSetpoint(setpointList.weekdayThree.startTime, setpoint) || inSetpoint(setpointList.weekdayThree.stopTime, setpoint))
        && setpointList.weekdayThree.enabled){
            return false;
        }
        else if((inSetpoint(setpoint.startTime,setpointList.weekdayFour) || inSetpoint(setpoint.stopTime,setpointList.weekdayFour)) 
        && (inSetpoint(setpointList.weekdayFour.startTime, setpoint) || inSetpoint(setpointList.weekdayFour.stopTime, setpoint))
        && setpointList.weekdayFour.enabled){
            return false;
        }
    }else{
        if((inSetpoint(setpoint.startTime,setpointList.weekendOne) || inSetpoint(setpoint.stopTime,setpointList.weekendOne)) 
        && (inSetpoint(setpointList.weekendOne.startTime, setpoint) || inSetpoint(setpointList.weekendOne.stopTime, setpoint))
        && setpointList.weekendOne.enabled){
            return false;
        }
        else if((inSetpoint(setpoint.startTime,setpointList.weekendTwo) || inSetpoint(setpoint.stopTime,setpointList.weekendTwo)) 
        && (inSetpoint(setpointList.weekendTwo.startTime, setpoint) || inSetpoint(setpointList.weekendTwo.stopTime, setpoint))
        && setpointList.weekendTwo.enabled){
            return false;
        }
        else if((inSetpoint(setpoint.startTime,setpointList.weekendThree) || inSetpoint(setpoint.stopTime,setpointList.weekendThree)) 
        && (inSetpoint(setpointList.weekendThree.startTime, setpoint) || inSetpoint(setpointList.weekendThree.stopTime, setpoint))
        && setpointList.weekendThree.enabled){
            return false;
        }
        else if((inSetpoint(setpoint.startTime,setpointList.weekendFour) || inSetpoint(setpoint.stopTime,setpointList.weekendFour)) 
        && (inSetpoint(setpointList.weekendFour.startTime, setpoint) || inSetpoint(setpointList.weekendFour.stopTime, setpoint))
        && setpointList.weekendFour.enabled){
            return false;
        }
    }

   if(setpoint.startTime.hour > setpoint.stopTime.hour && !(!setpoint.startTime.am && setpoint.stopTime.am)){
      return false;
   }
   else if(setpoint.startTime.hour == setpoint.stopTime.hour){
      if(setpoint.startTime.minute >= setpoint.stopTime.minute){
          return false;
      }
      
   }

   return true;
}

bool inSetpoint(Time t,Setpoint setpoint){
    if(setpoint.startTime.hour < setpoint.stopTime.hour){
        if(t.hour > setpoint.startTime.hour && t.hour < setpoint.stopTime.hour){
            return true;
        }
    }
    if(setpoint.startTime.hour > setpoint.stopTime.hour){
        if(t.hour > setpoint.startTime.hour or t.hour < setpoint.stopTime.hour){
            return true;
        }
    }
    if(setpoint.startTime.hour == setpoint.stopTime.hour){
        if(t.minute >= setpoint.startTime.minute && t.minute <= setpoint.stopTime.minute){
            return true;
        }
    }
    if(setpoint.startTime.hour == t.hour){
        if(t.minute >= setpoint.startTime.minute){
            return true;
        }
    }
    if(setpoint.stopTime.hour == t.hour){
        if(t.minute <= setpoint.stopTime.minute){
            return true;
        }
    }
    return false;
}

void clearLCD(){
  tft.fillScreen(ILI9341_WHITE);
}

//this will completely redraw lcd with
void redrawLCD(){
    //read state draw state, start there then can get fancy with interrupt, maybe only update certain parts
    if(LCDState == off || LCDState == powersaving){
        tft.fillScreen(ILI9341_BLACK);
    }
    else if(LCDState == main){
        drawMain();
    }
    else if(LCDState == setpointDisplay){
        drawSetpointDisplay();
    }
    else if(LCDState == timedateEdit){
        drawTimedateEdit();
    }

    else if(LCDState == setpointEdit){
        drawSetpointEdit(localSetpoint);
    }
}

void drawSaveButton(){
    tft.fillRect(8, 8, 32, 65, ILI9341_GREEN);
    tft.drawRect(7, 7, 33, 66, ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_BLACK);
    tft.setRotation(3);
    tft.setCursor(258,15);
    tft.print("Save");
    tft.setRotation(0);
}

void drawBackButton(){
    tft.drawRect(7, 284, 33, 33, ILI9341_BLACK);
    tft.fillTriangle(
      22,311,
      12,291,
      32,291,
      ILI9341_BLACK);  
}


void drawMain(){
    /*Draw Boxes*/
    //hold box
    tft.drawRect(47,8,33,81, ILI9341_BLACK);
    //time box
    tft.drawRect(7,8,33, 117, ILI9341_BLACK);
    //program settings box
    tft.drawRect(95,8, 33,81, ILI9341_BLACK);
    if(updateSetpointActive){
      if(setpointActive){
          tft.fillRect(96,9,32,80, ILI9341_GREEN);
      }
      else{
          tft.fillRect(96,9,32,80, ILI9341_LIGHTGREY);
      }
      updateSetpointActive = false;
    }
    tft.drawRect(143, 8, 89, 81, ILI9341_BLACK);

    //HVAC Box
    tft.drawRect(143, 120, 89, 193, ILI9341_BLACK);
    //temp box
    tft.drawRect(7, 132, 129, 181, ILI9341_BLACK);
    //setpoint +-
    tft.drawRect(199,18,25,25,ILI9341_BLACK);
    tft.drawRect(199,54,25,25,ILI9341_BLACK);

    //drawStatus
    if(updateHVACStatus){
        tft.fillRect(155, 125, 25, 90, ILI9341_WHITE);
        updateHVACStatus = false;
    }
    
    tft.setRotation(3);
    tft.setCursor(15, 160);
    tft.setTextSize(2);
    tft.print("Status: ");
    if(HVACStatus == nothingOn){
        tft.setTextColor(ILI9341_BLACK);
        tft.print("off");
    }
    else if(HVACStatus == coolOn){
        tft.setTextColor(ILI9341_BLUE);
        tft.print("Cooling");
    }
    else if(HVACStatus == heatOn){
        tft.setTextColor(ILI9341_RED);
        tft.print("Heating");
    }
    tft.setRotation(0);

    
    //draw mode
    if(updateHVACMode){
        tft.fillRect(190, 148, 25, 55, ILI9341_WHITE);
        updateHVACMode = false;
    }

    //forward mode button
    tft.fillTriangle(202, 218, 195, 204, 209, 204, ILI9341_BLACK);

    //back button mode
    tft.fillTriangle(202, 132, 195, 146, 209, 146, ILI9341_BLACK);
    
    tft.setRotation(3);
    tft.setCursor(15, 195);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_BLACK);
    tft.print("Mode:    ");
    tft.print(modeToString(HVACMode, true));

    /*Draw Current Temp*/
    tft.setCursor(20, 45);
    tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(7);
    if(currentTemp / 10 < 1){
      tft.print(zero[2]);
    }
    tft.print(currentTemp);
    tft.println(" F");

    drawDateTime();

    /*Draw Hold*/
    if(updateHold){
        tft.setRotation(0);
        if(hold){
            tft.fillRect(47,8,33,81, ILI9341_GREEN);
        }
        else{
            tft.fillRect(47,8,33,81, ILI9341_LIGHTGREY);
        }
        tft.setRotation(3);
        tft.setCursor(250,55);
        tft.setTextSize(2);
        tft.println("Hold");
        
        updateHold = false;
    }

    /*Draw Setpoint*/
    if(updateSetpoint){
        //clear temperature
        tft.setRotation(0);
        tft.fillRect(161, 18, 31, 61, ILI9341_WHITE);
        /*Plus Button*/
        tft.fillRect(203,64,17,5, ILI9341_RED); //verticle
        tft.fillRect(209,57,5,19, ILI9341_RED); //horizontal
        /*Minue*/
        tft.fillRect(209,22,5,17, ILI9341_BLUE); //horizontal

        tft.setRotation(3);
        tft.setCursor(255,165);
        tft.setTextSize(3);
        tft.print(currentTargetTemp);
        
        updateSetpoint = false;
    }

    /*Draw Program*/
    
    tft.setRotation(3);
    tft.setCursor(252,101);
    tft.setTextSize(1);
    tft.print("Program");

    tft.setCursor(246,113);
    tft.setTextSize(1);
    tft.print("Setpoints");
    tft.setRotation(0);  
}

void drawSetpointEdit(Setpoint setpoint){
    drawEditGUI();

    /*Draw start/stop*/
    if(updateSetStart){
        tft.fillRect( 42, 95, 20, 130, ILI9341_WHITE);
        updateSetStart = false;  
    }
    
    
    tft.setRotation(3);
    tft.setTextSize(2);

    String start = "Start Time";
    String stopS = "Stop Time";
    if(setStartSelected){
        
        tft.setTextColor(ILI9341_BLACK);
        if(setStartWanted){
            tft.setCursor(100,45);
            tft.print(start);
        }else{
            tft.setCursor(107,45);
            tft.print(stopS);
        }
    }else{
        tft.setTextColor(ILI9341_LIGHTGREY);
        if(setStartWanted){
            tft.setCursor(100,45);
            tft.print(start);
        }else{
            tft.setCursor(107,45);
            tft.print(stopS);
        }
    }
    tft.setRotation(0);
    
    
    /*Draw minute Option*/
    if(updateSetMinute){
        tft.fillRect( 73, 100, 20, 25, ILI9341_WHITE);
        updateSetMinute = false;  
    }
    
    tft.setCursor(100,76);
    tft.setRotation(3);
    tft.setTextSize(2);
    if(setMinuteSelected){
        tft.setTextColor(ILI9341_BLACK);
        tft.print("Minute: ");
        if(setStartWanted){
            if(setpoint.startTime.minute / 10 < 1){
                tft.print(zero[0]);
            }
            tft.print(setpoint.startTime.minute);
        }else{
            if(setpoint.stopTime.minute / 10 < 1){
                tft.print(zero[0]);
            }
            tft.print(setpoint.stopTime.minute);
        }
       
    }else{
        tft.setTextColor(ILI9341_LIGHTGREY);
        tft.print("Minute: ");
        if(setStartWanted){
            if(setpoint.startTime.minute / 10 < 1){
                tft.print(zero[0]);
            }
            tft.print(setpoint.startTime.minute);
        }else{
            if(setpoint.stopTime.minute / 10 < 1){
                tft.print(zero[0]);
            }
            tft.print(setpoint.stopTime.minute);
        }
    }
    tft.setRotation(0);

    /*Draw hour Option*/
    if(updateSetHour){
        tft.fillRect( 104, 85, 20, 65, ILI9341_WHITE);
        updateSetHour = false;  
    }
    
    tft.setCursor(100,107);
    tft.setRotation(3);
    tft.setTextSize(2);
    if(setHourSelected){
        tft.setTextColor(ILI9341_BLACK);
        tft.print("Hour: ");
        if(setStartWanted){
            if(setpoint.startTime.hour  > 12){
                tft.print(setpoint.startTime.hour % 12);
            }else{
                tft.print(setpoint.startTime.hour);
            }
        }else{
            if(setpoint.stopTime.hour  > 12){
                tft.print(setpoint.stopTime.hour % 12);
            }else{
                tft.print(setpoint.stopTime.hour);
            }
        }    
       
    }else{
        tft.setTextColor(ILI9341_LIGHTGREY);
        tft.print("Hour: ");
        if(setStartWanted){
            if(setpoint.startTime.hour  > 12){
                tft.print(setpoint.startTime.hour % 12);
            }else{
                tft.print(setpoint.startTime.hour);
            }
        }else{
            if(setpoint.stopTime.hour  > 12){
                tft.print(setpoint.stopTime.hour % 12);
            }else{
                tft.print(setpoint.stopTime.hour);
            }
        }    
    }

    if(setStartWanted){
        if(setpoint.startTime.am){
            tft.print(" AM");
        }else{
            tft.print(" PM");
        }
    }else{
        if(setpoint.stopTime.am){
            tft.print(" AM");
        }else{
            tft.print(" PM");
        }
    }
    
    tft.setRotation(0);

    /*Temperature*/
    if(updateSetTemp){
        tft.fillRect(138, 100, 20, 35, ILI9341_WHITE);
        updateSetTemp = false;
    }

    
    tft.setCursor(95,140);
    tft.setRotation(3);
    tft.setTextSize(2);
    if(setTempSelected){
        tft.setTextColor(ILI9341_BLACK);
        tft.print("Target: ");
        tft.print(setpoint.temp);
        tft.print(" F");
    }else{
        tft.setTextColor(ILI9341_LIGHTGREY);
        tft.print("Target: ");
        tft.print(setpoint.temp);
        tft.print(" F");
    }
    tft.setRotation(0);

    /*HVAC Mode*/
    if(updateSetMode){
        tft.fillRect(170, 90, 20, 60, ILI9341_WHITE);
        updateSetMode = false;
    }
    
    tft.setCursor(105,173);
    tft.setRotation(3);
    tft.setTextSize(2);
    if(setModeSelected){
        tft.setTextColor(ILI9341_BLACK);
        tft.print("Mode: ");
    }else{
        tft.setTextColor(ILI9341_LIGHTGREY);
        tft.print("Mode: ");
    }
    tft.print(modeToString(setpoint.HVACMode,setModeSelected));
    
    tft.setRotation(0);
    
    /*Draw Year*/
    if(updateSetEnable){
        tft.fillRect(202, 105, 20, 110, ILI9341_WHITE);
        updateSetEnable = false;
    }
    
    
    tft.setRotation(3);
    tft.setTextSize(2);
    if(setEnableSelected){
        if(setpoint.enabled){
            tft.setCursor(120,205);
            tft.setTextColor(ILI9341_GREEN);
            tft.print("Enabled");
        }else{
            tft.setCursor(115,205);
            tft.setTextColor(ILI9341_RED);
            tft.print("Disabled");
        }
    }else{
        tft.setTextColor(ILI9341_LIGHTGREY);
        if(setpoint.enabled){
            tft.setCursor(120,205);
            tft.print("Enabled");
        }else{
            tft.setCursor(115,205);
            tft.print("Disabled");
        }
    }
    tft.setRotation(0);
    
    tft.setRotation(0);
}
    
void drawSetpointDisplay(){
    if(updateSetpointTable){
        tft.fillRect(80, 0, 180, 290, ILI9341_WHITE);
        updateSetpointTable = false;
    }

    tft.setRotation(3);
    //Draw Weekday/weekend button
    tft.setCursor(180,40);
    
    if(setWeekdaySelected){
        tft.setTextColor(ILI9341_LIGHTGREY);
    }else{
        tft.setTextColor(ILI9341_BLACK);
    } 
    tft.setTextSize(2);
    tft.print("Weekend");

    tft.setCursor(70,40);
    if(!setWeekdaySelected){
        tft.setTextColor(ILI9341_LIGHTGREY);
    }else{
        tft.setTextColor(ILI9341_BLACK);
    } 
    tft.print("Weekday");
    tft.setRotation(0);  
    
    tft.drawRect(63, 0, 193, 289, ILI9341_BLACK); 
    tft.drawRect(0, 0, 30, 71, ILI9341_BLACK);
  
    //Horizontal Lines
    tft.drawLine(79, 0, 79, 320, ILI9341_BLACK);
    tft.drawLine(120, 0, 120, 320, ILI9341_BLACK);
    tft.drawLine(161, 0, 161, 320, ILI9341_BLACK);
    tft.drawLine(202, 0, 202, 320, ILI9341_BLACK); 
    tft.drawLine(63, 289, 63 , 320, ILI9341_BLACK); 
    //Vertical Lines
    tft.drawLine(63, 216, 240, 216, ILI9341_BLACK);
    tft.drawLine(63, 145, 240, 145, ILI9341_BLACK);
    tft.drawLine(63, 73, 240, 73, ILI9341_BLACK);

    drawBackButton();


    //Draw headers
    tft.setCursor(37,68);
    tft.setTextColor(ILI9341_BLACK); 
    tft.setRotation(3);
    tft.setTextSize(1);
    tft.print("Start Time");
    tft.setRotation(0); 

    tft.setCursor(111,68);
    tft.setTextColor(ILI9341_BLACK); 
    tft.setRotation(3);
    tft.setTextSize(1);
    tft.print("Stop Time");
    tft.setRotation(0); 

    tft.setCursor(200,68);
    tft.setTextColor(ILI9341_BLACK); 
    tft.setRotation(3);
    tft.setTextSize(1);
    tft.print("Temp.");
    tft.setRotation(0); 

    tft.setCursor(272,68);
    tft.setTextColor(ILI9341_BLACK); 
    tft.setRotation(3);
    tft.setTextSize(1);
    tft.print("Mode");
    tft.setRotation(0);

    //Draw Edit Button
    tft.setCursor(262,10);
    tft.setTextColor(ILI9341_BLACK); 
    tft.setRotation(3);
    tft.setTextSize(2);
    tft.print("Edit");
    tft.setRotation(0); 

    if (setWeekdaySelected == true){
          drawSetpointTable(setpointList.weekdayOne, setpointList.weekdayTwo, setpointList.weekdayThree, setpointList.weekdayFour); 
    } else {
          drawSetpointTable(setpointList.weekendOne, setpointList.weekendTwo, setpointList.weekendThree, setpointList.weekendFour); 
    }
}

void drawSetpointTable(Setpoint setpointOne,Setpoint setpointTwo,Setpoint setpointThree,Setpoint setpointFour){
    if(setpointOne.enabled){
        tft.drawRect(89, 295, 20, 20, ILI9341_BLACK);
        tft.fillRect(90, 296, 18, 18, ILI9341_GREEN);
    }else{
        tft.drawRect(89, 295, 20, 20, ILI9341_BLACK);
        tft.fillRect(90, 296, 18, 18, ILI9341_RED);
    }

    //draw row 1
    if(selectedSetpoint == 1){
        tft.setTextColor(ILI9341_BLACK);
    }else{
        tft.setTextColor(ILI9341_LIGHTGREY);
    }
    tft.setRotation(3);
    tft.setCursor(40, 92);
    tft.setTextSize(2);
    tft.print(setpointOne.startTime.hour);
    tft.print(zero[2]);
    if(setpointOne.startTime.minute/10 < 1){
        tft.print(zero[0]);  
    }  
    tft.print(setpointOne.startTime.minute);
    tft.print(zero[1]);

    tft.setCursor(110, 92);
    tft.print(setpointOne.stopTime.hour);
    tft.print(zero[2]);
    if(setpointOne.stopTime.minute/10 < 1){
        tft.print(zero[0]);  
    } 
    tft.print(setpointOne.stopTime.minute);
    tft.print(zero[1]);
    
    tft.setCursor(200,92);
    tft.print(setpointOne.temp);

    tft.setCursor(267, 92);
    if(selectedSetpoint == 1){
        tft.print(modeToString(setpointOne.HVACMode, true));
    }else{
        tft.print(modeToString(setpointOne.HVACMode, false));
    }
    tft.setRotation(0);

    //draw row 2
    if(setpointTwo.enabled){
        tft.drawRect(131, 295, 20, 20, ILI9341_BLACK);
        tft.fillRect(132, 296, 18, 18, ILI9341_GREEN);
    }else{
        tft.drawRect(131, 295, 20, 20, ILI9341_BLACK);
        tft.fillRect(132, 296, 18, 18, ILI9341_RED);
    }
    
    tft.setRotation(3);
    if(selectedSetpoint == 2){
        tft.setTextColor(ILI9341_BLACK);
    }else{
        tft.setTextColor(ILI9341_LIGHTGREY);
    }
    tft.setCursor(40, 131);
    tft.print(setpointTwo.startTime.hour);
    tft.print(zero[2]);
    if(setpointTwo.startTime.minute/10 < 1){
        tft.print(zero[0]);  
    }  
    tft.print(setpointTwo.startTime.minute);
    tft.print(zero[1]);

    tft.setCursor(110, 131);
    tft.print(setpointTwo.stopTime.hour);
    tft.print(zero[2]);
    if(setpointTwo.stopTime.minute/10 < 1){
        tft.print(zero[0]);  
    }  
    tft.print(setpointTwo.stopTime.minute);
    tft.print(zero[1]);
    
    tft.setCursor(200,131);
    tft.print(setpointTwo.temp);

    tft.setCursor(267, 131);
    if(selectedSetpoint == 2){
        tft.print(modeToString(setpointTwo.HVACMode, true));
    }else{
        tft.print(modeToString(setpointTwo.HVACMode, false));
    }
    tft.setRotation(0);


    //draw row 3
    if(setpointThree.enabled){
        tft.drawRect(172, 295, 20, 20, ILI9341_BLACK);
        tft.fillRect(173, 296, 18, 18, ILI9341_GREEN);
    }else{
        tft.drawRect(172, 295, 20, 20, ILI9341_BLACK);
        tft.fillRect(173, 296, 18, 18, ILI9341_RED);
    }

    tft.setRotation(3);
    if(selectedSetpoint == 3){
        tft.setTextColor(ILI9341_BLACK);
    }else{
        tft.setTextColor(ILI9341_LIGHTGREY);
    }
    tft.setCursor(40, 172);
    tft.print(setpointThree.startTime.hour);
    tft.print(zero[2]);
    if(setpointThree.startTime.minute/10 < 1){
        tft.print(zero[0]);  
    }  
    tft.print(setpointThree.startTime.minute);
    tft.print(zero[1]);

    tft.setCursor(110, 172);
    tft.print(setpointThree.stopTime.hour);
    tft.print(zero[2]);
    if(setpointThree.stopTime.minute/10 < 1){
        tft.print(zero[0]);  
    }   
    tft.print(setpointThree.stopTime.minute);
    tft.print(zero[1]);
    
    tft.setCursor(200,172);
    tft.print(setpointThree.temp);

    tft.setCursor(267, 172);
    if(selectedSetpoint == 2){
        tft.print(modeToString(setpointThree.HVACMode, true));
    }else{
        tft.print(modeToString(setpointThree.HVACMode, false));
    }
 
    tft.setRotation(0);


    //draw row 4
    if(setpointFour.enabled){
        tft.drawRect(213, 295, 20, 20, ILI9341_BLACK);
        tft.fillRect(214, 296, 18, 18, ILI9341_GREEN);
    }else{
        tft.drawRect(213, 295, 20, 20, ILI9341_BLACK);
        tft.fillRect(214, 296, 18, 18, ILI9341_RED);
    }

    tft.setRotation(3);
    if(selectedSetpoint == 4){
        tft.setTextColor(ILI9341_BLACK);
    }else{
        tft.setTextColor(ILI9341_LIGHTGREY);
    }
    tft.setCursor(40, 213);
    tft.print(setpointFour.startTime.hour);
    tft.print(zero[2]);
    if(setpointFour.startTime.minute/10 < 1){
        tft.print(zero[0]);  
    }   
    tft.print(setpointFour.startTime.minute);
    tft.print(zero[1]);

    tft.setCursor(110, 213);
    tft.print(setpointFour.stopTime.hour);
    tft.print(zero[2]);
    if(setpointFour.stopTime.minute/10 < 1){
        tft.print(zero[0]);  
    }  
    tft.print(setpointFour.stopTime.minute);
    tft.print(zero[1]);
    
    tft.setCursor(200,213);
    tft.print(setpointFour.temp);

    tft.setCursor(267, 213);
    if(selectedSetpoint == 4){
        tft.print(modeToString(setpointFour.HVACMode, true));
    }else{
        tft.print(modeToString(setpointFour.HVACMode, false));
    }
    
    tft.setRotation(0);
}


void drawTimedateEdit(){
    drawEditGUI();


    /*Draw minute Option*/
    if(updateMinute){
        tft.fillRect( 42, 100, 20, 25, ILI9341_WHITE);
        updateMinute = false;  
    }
    
    tft.setCursor(100,45);
    tft.setRotation(3);
    tft.setTextSize(2);
    if(minuteSelected){
        tft.setTextColor(ILI9341_BLACK);
        tft.print("Minute: ");
        if(localTime.minute / 10 < 1){
            tft.print(zero[0]);
        }
        tft.print(localTime.minute);
    }else{
        tft.setTextColor(ILI9341_LIGHTGREY);
        tft.print("Minute: ");
        if(localTime.minute / 10 < 1){
            tft.print(zero[0]);
        }
        tft.print(localTime.minute);
    }
    tft.setRotation(0);

    /*Draw hour Option*/
    if(updateHour){
        tft.fillRect( 73, 85, 20, 65, ILI9341_WHITE);
        updateHour = false;  
    }
    
    tft.setCursor(100,76);
    tft.setRotation(3);
    tft.setTextSize(2);
    if(hourSelected){
        tft.setTextColor(ILI9341_BLACK);
        tft.print("Hour: ");
        if(localTime.hour  > 12){
            tft.print(localTime.hour % 12);
        }else{
            tft.print(localTime.hour);
        }
    }else{
        tft.setTextColor(ILI9341_LIGHTGREY);
        tft.print("Hour: ");
        if(localTime.hour  > 12){
            tft.print(localTime.hour % 12);
        }else{
            tft.print(localTime.hour);
        }
    }
    if(localTime.am){
        tft.print(" AM");
    }else{
        tft.print(" PM");
    }
    tft.setRotation(0); 

    /*Draw Weekday*/
    if(updateWeekday){
        tft.fillRect(105, 85, 20, 40, ILI9341_WHITE);
        updateWeekday = false;
    }
    
    tft.setCursor(90,107);
    tft.setRotation(3);
    tft.setTextSize(2);
    if(weekdaySelected){
        tft.setTextColor(ILI9341_BLACK);
        tft.print("Weekday: ");
        tft.print(dayToString(localDate.weekday));
    }else{
        tft.setTextColor(ILI9341_LIGHTGREY);
        tft.print("Weekday: ");
        tft.print(dayToString(localDate.weekday));
    }
    tft.setRotation(0);

    /*Draw Month*/
    if(updateMonth){
        tft.fillRect(138, 94, 20, 50, ILI9341_WHITE);
        updateMonth = false;
    }

    
    tft.setCursor(95,140);
    tft.setRotation(3);
    tft.setTextSize(2);
    if(monthSelected){
        tft.setTextColor(ILI9341_BLACK);
        tft.print("Month: ");
        tft.print(monthToString(localDate.month));
    }else{
        tft.setTextColor(ILI9341_LIGHTGREY);
        tft.print("Month: ");
        tft.print(monthToString(localDate.month));
    }
    tft.setRotation(0);

    /*Draw Day*/
    if(updateDay){
        tft.fillRect(170, 113, 20, 30, ILI9341_WHITE);
        updateDay = false;
    }
    
    tft.setCursor(123,173);
    tft.setRotation(3);
    tft.setTextSize(2);
    if(daySelected){
        tft.setTextColor(ILI9341_BLACK);
        tft.print("Day: ");
        if(localDate.day / 10 < 1){
            tft.print(zero[1]);
        }
        tft.print(localDate.day);
    }else{
        tft.setTextColor(ILI9341_LIGHTGREY);
        tft.print("Day: ");
        if(localDate.day / 10 < 1){
            tft.print(zero[1]);
        }
        tft.print(localDate.day);
    }
    tft.setRotation(0);
    

    /*Draw Year*/
    if(updateYear){
        tft.fillRect(202, 95, 20, 60, ILI9341_WHITE);
        updateYear = false;
    }
    
    tft.setCursor(100,205);
    tft.setRotation(3);
    tft.setTextSize(2);
    if(yearSelected){
        tft.setTextColor(ILI9341_BLACK);
        tft.print("Year: ");
        tft.print(localDate.year);
    }else{
        tft.setTextColor(ILI9341_LIGHTGREY);
        tft.print("Year: ");
        tft.print(localDate.year);
    }
    tft.setRotation(0);
    
    
}

void drawEditGUI(){
    tft.drawRect(39, 79, 25, 161, ILI9341_BLACK);
    tft.drawRect(71, 79, 25, 161, ILI9341_BLACK);
    tft.drawRect(103, 79, 25, 161, ILI9341_BLACK);
    tft.drawRect(135, 79, 25, 161, ILI9341_BLACK);
    tft.drawRect(167, 79, 25, 161, ILI9341_BLACK);
    tft.drawRect(199, 79, 25, 161, ILI9341_BLACK);

    drawSaveButton();
    drawBackButton();

    /*Draw Plus Box*/
    tft.drawRect(100, 20, 40, 40, ILI9341_BLACK);
    //horizontal
    tft.fillRect(117, 24, 5, 32, ILI9341_RED);
    //verticle
    tft.fillRect(104, 38, 32, 5, ILI9341_RED);

    /*Draw Minus Box*/
    tft.drawRect(100, 260, 40, 40, ILI9341_BLACK);
    //horizontal
    tft.fillRect(117, 264, 5, 32, ILI9341_BLUE);
}

void drawDateTime(){
    /*Draw Time*/
    tft.setRotation(3);
    tft.setCursor(210,13);
    tft.setTextSize(1);
    tft.print(dayToString(currentDate.weekday));
    tft.print(zero[1]);

    /*Draw Hour*/
    if(updateHour){
        tft.setRotation(0);
        tft.fillRect(8, 65, 14, 22, ILI9341_WHITE);
        tft.fillRect(8, 10, 14, 22, ILI9341_WHITE);
        tft.setRotation(3);
        updateHour = false;
    }
    
    if(currentTime.hour % 13 / 10 < 1){
      tft.print(zero[1]);
    }
    if(currentTime.am || currentTime.hour == 12){
        tft.print(currentTime.hour);
    }
    else{
        tft.print(currentTime.hour % 12);
    }
    tft.print(zero[2]);

    /*Draw Minute*/
    if(updateMinute){
        tft.setRotation(0);
        tft.fillRect(8, 45, 14, 25, ILI9341_WHITE);
        tft.setRotation(3);
        updateMinute = false;
    }
    if(currentTime.minute / 10 < 1){
        tft.print(zero[0]);
    }
    tft.print(currentTime.minute);
    
    tft.print(zero[2]);
    
    /*Draw Second*/
    if(updateSecond){
        tft.setRotation(0);
        tft.fillRect(8, 37, 15, 15, ILI9341_WHITE);
        tft.setRotation(3);
        updateSecond = false;
    }


    if(currentTime.second / 10 < 1){
        tft.print(zero[0]);
    }
    tft.print((int) currentTime.second);

    /*Draw AM*/
    if(currentTime.am){
        tft.print(" AM");
    }else{
        tft.print(" PM");
    }

    /*Draw Date*/
    if(updateWeekday){
        tft.setRotation(0);
        tft.fillRect(10,12, 25, 106, ILI9341_WHITE);
        tft.setRotation(3);
        updateWeekday = false;
    }
    tft.setRotation(3);
    tft.setCursor(210, 25);
    tft.print(monthToString(currentDate.month));
    tft.print(zero[1]);
    tft.print(currentDate.day);
    tft.print(", ");
    tft.print(currentDate.year);
    tft.setRotation(0);
    
}

//add time, gonna have a huge chain for potential roll over
void addTime(unsigned long microsToAdd){
    previousSecond = (int) currentTime.second;
    currentTime.second += (double) microsToAdd/1000000;
    if((int) currentTime.second - previousSecond != 0){
        updateSecond = true;
    }
    if(currentTime.second >= 60){
        currentTime.second -= 60;
        updateMinute = true;
        currentTime.minute += 1;
        if(currentTime.minute >= 60){
            currentTime.minute -= 60;
            currentTime.hour += 1;
            if(currentTime.hour == 12){
                currentTime.am = false;
            }
            updateHour = true;
            if(currentTime.hour >= 24){
                currentTime.hour -= 24;
                currentTime.am = true;
                currentDate.weekday = nextDay(currentDate.weekday);
                updateWeekday = true;

                currentDate.day += 1;
                updateDay = true;
                
                if((currentDate.month % 2 == 1 || currentDate.month == 7) && currentDate.day == 32){
                    currentDate.month += 1;
                    currentDate.day = 1;
                    updateMonth = true;
                }
                else if((currentDate.month % 2 == 0 && currentDate.month != 2) && currentDate.day == 31){
                    currentDate.month += 1;
                    currentDate.day = 1;
                    updateMonth = true;
                }
                else if(currentDate.month == 2 && currentDate.day == 29){
                    currentDate.month += 1;
                    currentDate.day = 1;
                    updateMonth = true;
                }

                if(updateMonth){
                    if(currentDate.month == 13){
                        currentDate.month = 1;
                        currentDate.year += 1;
                        updateYear = true;
                    }
                }
            }
        }
    }
}

String dayToString(Day d){
    switch(d){
        case monday: return "Mon";
        case tuesday: return "Tue";
        case wednesday: return "Wed";
        case thursday: return "Thu";
        case friday: return "Fri";
        case saturday: return "Sat";
        case sunday: return "Sun";
    }
}

String monthToString(int m){
    switch(m){
        case 1: return "Jan.";
        case 2: return "Feb.";
        case 3: return "Mar.";
        case 4: return "Apr.";
        case 5: return "May.";
        case 6: return "Jun.";
        case 7: return "Jul.";
        case 8: return "Aug.";
        case 9: return "Sep.";
        case 10: return "Oct.";
        case 11: return "Nov.";
        case 12: return "Dec."; 
    }
}

Day nextDay(Day d){
    switch(d){
        case monday: return tuesday;
        case tuesday: return wednesday;
        case wednesday: return thursday;
        case thursday: return friday;
        case friday: return saturday;
        case saturday: return sunday;
        case sunday: return monday;
    }
}

PossibleHVACModes nextMode(PossibleHVACModes mode){
    switch(mode){
        case none: return cool;
        case cool: return heat;
        case heat: return automatic;
        case automatic: return none;  
    }
}

void adjustTemp(int targetTemp){
    currentTargetTemp = targetTemp;
    if(lastTargetTemp != currentTargetTemp){
        updateSetpoint = true;
    }
    lastTargetTemp = currentTargetTemp;
    
    if(targetTemp > currentTemp + tempBuffer && (HVACMode == heat or HVACMode == automatic)){
        HVACStatus = heatOn;
        if(lastHVACStatus != heatOn){
            updateHVACStatus = true;
        }
        digitalWrite(acPin, LOW);
        digitalWrite(heatPin, HIGH);   
    }
    else if(targetTemp < currentTemp - tempBuffer && (HVACMode == cool or HVACMode == automatic)){
        HVACStatus = coolOn;
        if(lastHVACStatus != coolOn){
            updateHVACStatus = true;
        }
        digitalWrite(acPin, HIGH);
        digitalWrite(heatPin, LOW);
    }
    else{
        HVACStatus = nothingOn;
        if(lastHVACStatus != nothingOn){
            updateHVACStatus = true;
        }   
        digitalWrite(acPin, LOW);
        digitalWrite(heatPin, LOW);
    }
    lastHVACStatus = HVACStatus;
}

void adjustHVACMode(PossibleHVACModes targetMode){
    if(targetMode == none){
        HVACMode = none;
        if(lastHVACMode != none){
            updateHVACMode = true;
        }
    }

    else if(targetMode == heat){
        HVACMode = targetMode;
        if(lastHVACMode != heat){
            updateHVACMode = true;
        }
    }

    else if(targetMode == cool){
        HVACMode = cool;
        if(lastHVACMode != cool){
            updateHVACMode = true;  
        }  
    }
    else if(targetMode = automatic){
        HVACMode = automatic;
        if(lastHVACMode != automatic){
            updateHVACMode = true;  
        }
    }
    lastHVACMode = HVACMode;
}

void clearSetSelection(){
    setStartSelected = false;
    setMinuteSelected = false;
    setHourSelected = false;
    setTempSelected = false;
    setModeSelected = false;
    setEnableSelected = false;
}

void clearTimeSelection(){
    minuteSelected = false;
    hourSelected = false;
    weekdaySelected = false;
    daySelected = false;
    monthSelected = false;
    yearSelected = false;  
}

String modeToString(PossibleHVACModes mode, bool selected){
  switch (mode){
      case cool:
          if(selected){
              tft.setTextColor(ILI9341_BLUE);
          }else{
              tft.setTextColor(ILI9341_LIGHTGREY);
          }
          
          return "Cool";
      case heat:
          if(selected){
              tft.setTextColor(ILI9341_RED);
          }else{
              tft.setTextColor(ILI9341_LIGHTGREY);
          }
          return "Heat";
      case automatic:
          if(selected){
              tft.setTextColor(ILI9341_BLACK);
          }else{
              tft.setTextColor(ILI9341_LIGHTGREY);
          }
          return "Auto";
      case none:
          tft.setTextColor(ILI9341_LIGHTGREY);
          return "Off";
  }
}
