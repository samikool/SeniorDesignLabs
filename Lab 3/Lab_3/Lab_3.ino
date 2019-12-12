#include "EEPROM.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "TouchScreen.h"

int CS = 10;
int DC = 9;
int RESET = 8;
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

/*Time Struct*/
struct Time{
  int hour; //hour
  int minute; //minute
  double second; //seconds
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
PossibleLCDStates LCDState = main;

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
Date currentDate = {monday, 1, 1, 2019, true};

unsigned long loopStart = 0;
unsigned long loopTime = 0;

TSPoint point = ts.getPoint();

void setup(){
   Serial.begin(2000000);
   pinMode(acPin, OUTPUT);
   pinMode(heatPin, OUTPUT);

   tft.begin();
   tft.fillScreen(ILI9341_WHITE);
   Serial.println("Loop starting");
   
}

void loop(){
  loopStart = micros();
  point = ts.getPoint();
  //delay(1000);
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
  
  /*Serial.print("Day: ");
  Serial.print(dayToString(currentDate.weekday));
  Serial.print(" Loop Time: ");
  Serial.print(loopTime);
  Serial.print(" Hour: ");
  Serial.print(currentTime.hour);
  Serial.print(" Minute: ");
  Serial.print(currentTime.minute);
  Serial.print(" Second: ");
  Serial.println(currentTime.second);*/

  
  if(point.z > 0 && point.x > 100){
    Serial.print("X = "); Serial.print(point.x);
    Serial.print("\tY = "); Serial.print(point.y);
    Serial.print("\tPressure = "); Serial.println(point.z);
    delay(200);
  }
  redrawLCD();
  loopTime = micros() - loopStart;
  addTime(loopTime);
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
  if(LCDState == off){
    
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

void drawMain(){
  tft.drawRect(47,8,33,81, ILI9341_BLACK);
  tft.drawRect(7,8,33, 117, ILI9341_BLACK);
  tft.drawRect(95,8, 33,81, ILI9341_BLACK);
  tft.drawRect(143, 8, 89, 81, ILI9341_BLACK);
  tft.drawRect(143, 124, 89, 189, ILI9341_BLACK);
  tft.drawRect(7, 132, 129, 181, ILI9341_BLACK);
  tft.drawRect(199, 18, 25, 25, ILI9341_BLACK);
  tft.drawRect(199, 54, 25, 25, ILI9341_BLACK);
  tft.drawRect(191,136,33,33,ILI9341_BLACK);
  tft.drawRect(191,180,33,33,ILI9341_BLACK);
  tft.drawRect(191,224,33,33,ILI9341_BLACK);
  tft.drawRect(191,268,33,33,ILI9341_BLACK);
  }
    
void drawSetpointEdit(){
  tft.drawRect(47, 15, 193, 289, ILI9341_BLACK); 
  tft.drawRect(7, 264, 33, 33, ILI9341_BLACK);
  tft.drawRect(7, 79, 25, 71, ILI9341_BLACK);
  tft.drawRect(7, 170, 25, 71, ILI9341_BLACK);
  tft.drawRect(215, 27, 17, 17, ILI9341_BLACK);
  tft.drawRect(215, 59, 17, 17, ILI9341_BLACK);
  tft.drawRect(215, 99, 17, 17, ILI9341_BLACK);
  tft.drawRect(215, 131, 17, 17, ILI9341_BLACK);
  tft.drawRect(215, 171, 17, 17, ILI9341_BLACK);
  tft.drawRect(215, 203, 17, 17, ILI9341_BLACK);
  tft.drawRect(215, 243, 17, 17, ILI9341_BLACK);
  tft.drawRect(215, 275, 17, 17, ILI9341_BLACK);
  //Horizontal Lines
  tft.drawLine(63, 15, 63, 303, ILI9341_BLACK);
  tft.drawLine(99, 15, 99, 303, ILI9341_BLACK);
  tft.drawLine(135, 15, 135, 303, ILI9341_BLACK);
  tft.drawLine(171, 15, 171, 303, ILI9341_BLACK);
  tft.drawLine(207, 15, 207, 303, ILI9341_BLACK); 
  //Vertical Lines
  tft.drawLine(47, 231, 240, 231, ILI9341_BLACK);
  tft.drawLine(47, 160, 240, 160, ILI9341_BLACK);
  tft.drawLine(47, 88, 240, 88, ILI9341_BLACK);
}


void drawTimedateEdit(){
  tft.drawRect(7, 264, 33, 33, ILI9341_BLACK);
  tft.drawRect(39, 79, 25, 161, ILI9341_BLACK);
  tft.drawRect(71, 79, 25, 161, ILI9341_BLACK);
  tft.drawRect(103, 79, 25, 161, ILI9341_BLACK);
  tft.drawRect(135, 79, 25, 161, ILI9341_BLACK);
  tft.drawRect(167, 79, 25, 161, ILI9341_BLACK);
  tft.drawRect(199, 79, 25, 161, ILI9341_BLACK);
}

//use this one with small part of screen, possible speed improvement
void updateLCD(){

}

//add time, gonna have a huge chain for potential roll over
void addTime(unsigned long microsToAdd){
  currentTime.second += (double) microsToAdd/1000;
  if(currentTime.second >= 60){
    currentTime.second -= 60;
    currentTime.minute += 1;
    if(currentTime.minute >= 60){
      currentTime.minute -= 60;
      currentTime.hour += 1;
      if(currentTime.hour >= 24){
        currentTime.hour -= 24;
        currentDate.weekday = nextDay(currentDate.weekday);
      }
    }
  }
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
