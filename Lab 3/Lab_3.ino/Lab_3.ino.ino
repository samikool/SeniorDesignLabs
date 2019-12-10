#include "EEPROM.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

int CS = 10;
int DC = 9;
int RESET = 8;
Adafruit_ILI9341 tft = Adafruit_ILI9341(CS, DC);

int acPin = 2; 
int heatPin = 3;

/*Time Struct*/
struct Time{
  int hour; //hour
  int minute; //minute
  float second; //seconds
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
  setpointEdit,
  timedateEdit,
};
PossibleLCDStates LCDState = powersaving;

/*HVAC Mode State*/
enum PossibleHVACModes{
  none,
  cool,
  heat,
  automatic
};
PossibleHVACModes HVACMode = none;

/*HVAC Power State*/
enum PossibleHVACStatus{
  coolOn, 
  heatOn,
  nothingOn
};
PossibleHVACStatus HVACStatus = nothingOn;

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

SetpointList setpointList = {{70,{0,0,0},{6,0,0},none,true,false}, 
  {70,{6,0,0},{12,0,0},none,true,false}, 
  {70,{12,0,0},{18,0,0},none,true,false}, 
  {70,{18,0,0},{24,0,0},none,true,false}, 
  {70,{0,0,0},{6,0,0},none,false,false}, 
  {70,{6,0,0},{12,0,0},none,false,false}, 
  {70,{12,0,0},{18,0,0},none,false,false}, 
  {70,{18,0,0},{24,0,0},none,false,false}};

//Hold
bool hold = false;

/*General Data*/
int currentTemp = 0;
int currentTargetTemp = 0;
int localTargetTemp = 0;
int tempBuffer = 3;
Time currentTime = {0,0,0};
Date currentDate = {monday, 1, 1, 2019,true};

int loopStart = 0;
int loopTime = 0;

void setup(){
   Serial.begin(9600);
   pinMode(acPin, OUTPUT);
   pinMode(heatPin, OUTPUT);

   tft.begin();
   tft.fillScreen(ILI9341_BLACK);
}

void loop(){
  int loopStart = micros();
  if(LCDState != off){
    if(hold){
      adjustTemp(localTargetTemp);
    }
    else{
      if(currentDate.isWeekday){
        if(setpointList.weekdayOne.isActive && inSetpoint(setpointList.weekdayOne)){
          HVACMode = setpointList.weekdayOne.HVACMode;
          adjustTemp(setpointList.weekdayOne.temp);
        }
        else if(setpointList.weekdayTwo.isActive && inSetpoint(setpointList.weekdayTwo)){
          HVACMode = setpointList.weekdayTwo.HVACMode;
          adjustTemp(setpointList.weekdayTwo.temp);
        }
        else if(setpointList.weekdayThree.isActive && inSetpoint(setpointList.weekdayThree)){
          HVACMode = setpointList.weekdayThree.HVACMode;
          adjustTemp(setpointList.weekdayThree.temp);
        }
        else if(setpointList.weekdayFour.isActive && inSetpoint(setpointList.weekdayFour)){
          HVACMode = setpointList.weekdayFour.HVACMode;
          adjustTemp(setpointList.weekdayFour.temp);
        }
        else{adjustTemp(localTargetTemp);}
      }
      else{
        if(setpointList.weekendOne.isActive && inSetpoint(setpointList.weekendOne)){
          HVACMode = setpointList.weekendOne.HVACMode;
          adjustTemp(setpointList.weekendOne.temp);
        }
        else if(setpointList.weekendTwo.isActive && inSetpoint(setpointList.weekendTwo)){
          HVACMode = setpointList.weekendTwo.HVACMode;
          adjustTemp(setpointList.weekendTwo.temp);
        }
        else if(setpointList.weekendThree.isActive && inSetpoint(setpointList.weekendThree)){
          HVACMode = setpointList.weekendThree.HVACMode;
          adjustTemp(setpointList.weekendThree.temp);
        }
        else if(setpointList.weekendFour.isActive && inSetpoint(setpointList.weekendFour)){
          HVACMode = setpointList.weekendFour.HVACMode;
          adjustTemp(setpointList.weekendFour.temp);
        }
        else{adjustTemp(localTargetTemp);}
      }
    }
  }
  loopTime = micros() - loopStart;
  addTime(loopTime);
  redrawLCD();
}

bool inSetpoint(Setpoint setpoint){
    if(setpoint.startTime.hour < setpoint.stopTime.hour){
      if(currentTime.hour > setpoint.startTime.hour && currentTime.hour < setpoint.stopTime.hour){
        return true;
      }
    }
    else if(setpoint.startTime.hour > setpoint.stopTime.hour){
      if(currentTime.hour > setpoint.startTime.hour or currentTime.hour < setpoint.stopTime.hour){
        return true;
      }
    }
    else if(setpoint.startTime.hour == setpoint.stopTime.hour){
      if(currentTime.minute >= setpoint.startTime.minute && currentTime.minute <= setpoint.stopTime.minute){
        return true;
      }
    }
    else if(setpoint.startTime.hour == currentTime.hour){
      if(currentTime.minute >= setpoint.startTime.minute){
        return true;
      }
    }
    else if(setpoint.stopTime.hour == currentTime.hour){
      if(currentTime.minute <= setpoint.stopTime.minute){
        return true;
      }
    }
    return false;
}

//this will completely redraw lcd with
void redrawLCD(){
  //read state draw state, start there then can get fancy with interrupt, maybe only update certain parts
  if(LCDState = off){
    
  }
  else if(LCDState == powersaving){
    drawPowersaving();
  }
  else if(LCDState == main){
    drawMain();
  }
  else if(LCDState == setpointEdit){
    drawSetpointEdit();
  }
  else if(LCDState == timedateEdit){
    drawTimedateEdit();
  }

}

void drawPowersaving(){}
void drawMain(){}
void drawSetpointEdit(){}
void drawTimedateEdit(){}

//use this one with small part of screen, possible speed improvement
void updateLCD(){

}

//add time, gonna have a huge chain for potential roll over
void addTime(int millisToAdd){
  
}

String dayToString(Day d){
  switch(d){
    case monday: return "Mon";
    case tuesday: return "Tue";
    case wednesday: return "Wed";
    case thursday: return "Thur";
    case friday: return "Fri";
    case saturday: return "Sat";
    case sunday: return "Sun";
  }
}

void adjustTemp(int targetTemp){
  if(targetTemp > currentTemp + tempBuffer && (HVACMode == heat or HVACMode == automatic)){
    HVACStatus = heatOn;
    digitalWrite(heatPin, HIGH);
    digitalWrite(acPin, LOW);  
  }
  else if(targetTemp < currentTemp - tempBuffer && (HVACMode == cool or HVACMode == automatic)){
    HVACStatus = coolOn;
    digitalWrite(acPin, HIGH);
    digitalWrite(heatPin, LOW);
  }
  else{
    HVACStatus = nothingOn;
    digitalWrite(heatPin, LOW);
    digitalWrite(acPin, LOW);  
  }
}
