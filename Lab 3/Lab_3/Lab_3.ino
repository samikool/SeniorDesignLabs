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
  {70,{6,0,0,true},{12,0,0,false},none,true,false}, 
  {70,{12,0,0,false},{18,0,0,false},none,true,false}, 
  {70,{18,0,0,false},{0,0,0,true},none,true,false}, 
  {70,{0,0,0,true},{6,0,0,true},none,false,false}, 
  {70,{6,0,0,true},{12,0,0,false},none,false,false}, 
  {70,{12,0,0,false},{18,0,0,false},none,false,false}, 
  {70,{18,0,0,false},{0,0,0,true},none,false,false}};

//Hold
bool hold = true;

/*General Data*/
int currentTemp = 70;
int currentTargetTemp = 0;
int localTargetTemp = 60;
PossibleHVACModes localTargetHVACMode = cool;
int tempBuffer = 3;
Time currentTime = {23,59,55,false};
Date currentDate = {thursday, 12, 12, 2019, true};

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

/*Bools for determining what part of date and time is selected on timedateEdit screen*/
bool minuteSelected = true;
bool hourSelected = false;
bool weekdaySelected = false;
bool daySelected = false;
bool monthSelected = false;
bool yearSelected = false;

TSPoint p = ts.getPoint();

void setup(){
    Serial.begin(2000000);
    pinMode(acPin, OUTPUT);
    pinMode(heatPin, OUTPUT);

    tft.begin();
    clearLCD();
    Serial.println("Loop starting");
}

void loop(){
  loopStart = micros();
  p = ts.getPoint();
  //delay(1000);
  if(LCDState != off){
    
    if(hold){
      adjustTemp(localTargetTemp);
      adjustHVACMode(localTargetHVACMode);
    }
    else{
      if(currentDate.isWeekday){
        if(setpointList.weekdayOne.isActive && inSetpoint(setpointList.weekdayOne)){
          adjustHVACMode(setpointList.weekdayOne.HVACMode);
          adjustTemp(setpointList.weekdayOne.temp);
        }
        else if(setpointList.weekdayTwo.isActive && inSetpoint(setpointList.weekdayTwo)){
          adjustHVACMode(setpointList.weekdayTwo.HVACMode);
          adjustTemp(setpointList.weekdayTwo.temp);
        }
        else if(setpointList.weekdayThree.isActive && inSetpoint(setpointList.weekdayThree)){
          adjustHVACMode(setpointList.weekdayThree.HVACMode);
          adjustTemp(setpointList.weekdayThree.temp);
        }
        else if(setpointList.weekdayFour.isActive && inSetpoint(setpointList.weekdayFour)){
          adjustHVACMode(setpointList.weekdayFour.HVACMode);
          adjustTemp(setpointList.weekdayFour.temp);
        }
        else{adjustTemp(localTargetTemp);}
      }
      else{
        if(setpointList.weekendOne.isActive && inSetpoint(setpointList.weekendOne)){
          adjustHVACMode(setpointList.weekendOne.HVACMode);
          adjustTemp(setpointList.weekendOne.temp);
        }
        else if(setpointList.weekendTwo.isActive && inSetpoint(setpointList.weekendTwo)){
          adjustHVACMode(setpointList.weekendTwo.HVACMode);
          adjustTemp(setpointList.weekendTwo.temp);
        }
        else if(setpointList.weekendThree.isActive && inSetpoint(setpointList.weekendThree)){
          adjustHVACMode(setpointList.weekendThree.HVACMode);
          adjustTemp(setpointList.weekendThree.temp);
        }
        else if(setpointList.weekendFour.isActive && inSetpoint(setpointList.weekendFour)){
          adjustHVACMode(setpointList.weekendFour.HVACMode);
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

  
    if(p.z > 0 && p.z < 1000 && p.x > 100){
        Serial.print("X = "); Serial.print(p.x);
        Serial.print("\tY = "); Serial.print(p.y);
        Serial.print("\tPressure = "); Serial.println(p.z);
        if(LCDState == main){
            //go to timedateEdit
            if(140 < p.x && p.x < 240 && 120 < p.y && p.y < 400){
                LCDState = timedateEdit;
                clearLCD();
                Serial.println("Moving to timedateEdit");
                updateHold = true;
                updateSetpoint = true;
            }
            //go to setpointEdit
            else if( 400 < p.x && p.x < 530 && 125 < p.y && p.y < 310){
                LCDState = setpointDisplay;
                clearLCD();
                Serial.println("Moving to setpointEdit");
                updateHold = true;
                updateSetpoint = true;
            }
            //Hold toggled
            else if(265 < p.x && p.x < 370 &&  130 < p.y && p.y < 315){
                if(hold == false){hold = true;}
                else{hold = false;}
                updateHold = true;
                Serial.println("Hold toggeled");
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
            else if(750 < p.x && p.x < 805 && 570 < p.y && p.y < 625){
                for(int i=0; i<3; i++){
                    localTargetHVACMode = nextMode(localTargetHVACMode);
                }
                updateHVACMode = true;
            }
            
        }
        
        else if(LCDState == setpointDisplay){
          //Back button
            if(120 < p.x && p.x < 240 && 660 < p.y && p.y < 900){
                LCDState = main;
                clearLCD();
                Serial.println("Moving to main");
            }
        }
        
        else if(LCDState == timedateEdit){
            //Back button
            if(120 < p.x && p.x < 240 && 660 < p.y && p.y < 900){
                LCDState = main;
                clearLCD();
                Serial.println("Moving to main");
            }
            //select minute
            else if(248 < p.x && p.x < 305 && 267 < p.y && p.y < 670){
                minuteSelected = true;
                hourSelected = false;
                weekdaySelected = false;
                daySelected = false;
                monthSelected = false;
                yearSelected = false;
                Serial.println("Minute selected");
            } 
            //select hour
            else if(360 < p.x && p.x < 420 && 267 < p.y && p.y < 670){
                minuteSelected = false;
                hourSelected = true;
                weekdaySelected = false;
                daySelected = false;
                monthSelected = false;
                yearSelected = false;
                Serial.println("Hour selected");
            }
            //select Weekday
            else if(460 < p.x && p.x < 530 && 267 < p.y && p.y < 670){
                minuteSelected = false;
                hourSelected = false;
                weekdaySelected = true;
                daySelected = false;
                monthSelected = false;
                yearSelected = false;
                Serial.println("Weekday selected");
            }
            //select Month
            else if(555 < p.x && p.x < 635 && 267 < p.y && p.y < 670){
                minuteSelected = false;
                hourSelected = false;
                weekdaySelected = false;
                daySelected = false;
                monthSelected = true;
                yearSelected = false;
                Serial.println("Month selected");
            }
            //select Day
            else if(655 < p.x && p.x < 745 && 267 < p.y && p.y < 670){
                minuteSelected = false;
                hourSelected = false;
                weekdaySelected = false;
                daySelected = true;
                monthSelected = false;
                yearSelected = false;
                Serial.println("Day selected");
            }
            //select Year
            else if(760 < p.x && p.x < 840 && 267 < p.y && p.y < 670){
                minuteSelected = false;
                hourSelected = false;
                weekdaySelected = false;
                daySelected = false;
                monthSelected = false;
                yearSelected = true;
                Serial.println("Year selected");
            }
            
            
            //plus button
            else if(430 < p.x && p.x < 530 && 130 < p.y && p.y < 210){
                if(minuteSelected){
                    currentTime.minute += 1;
                    if(currentTime.minute == 60){
                        currentTime.minute = 0;
                    }
                    updateMinute = true;
                    currentTime.second = 0;
                }
                else if(hourSelected){
                    currentTime.hour += 1;
                    if(currentTime.hour == 24){
                        currentTime.hour = 0;
                        currentTime.am = true;
                    }

                    if(currentTime.hour == 12){
                        currentTime.am = false;
                    }
                    updateHour = true;
                }

                else if(weekdaySelected){
                    currentDate.weekday = nextDay(currentDate.weekday);
                    updateWeekday = true;
                }

                else if(monthSelected){
                    currentDate.month += 1;
                    if(currentDate.month == 13){
                        currentDate.month = 1;              
                    }
                    
                    if((currentDate.month % 2 == 0 && currentDate.month != 2) && currentDate.day > 30){
                        currentDate.day = 30;
                        updateDay = true;
                    }
                    else if(currentDate.month == 2 && currentDate.day > 28){
                        currentDate.day = 28;
                        updateDay = true;
                    }  
                    
                    updateMonth = true;
                }

                else if(daySelected){
                    currentDate.day += 1;
                    if((currentDate.month % 2 == 1 || currentDate.month == 7) && currentDate.day == 32){
                        currentDate.day = 1;
                    }
                    else if((currentDate.month % 2 == 0 && currentDate.month != 2) && currentDate.day == 31){
                        currentDate.day = 1;
                    }
                    else if(currentDate.month == 2 && currentDate.day == 29){
                        currentDate.day = 1;
                    }
                    updateDay = true;
                }

                else if(yearSelected){
                    currentDate.year += 1;
                    if(currentDate.year == 9999){
                        currentDate.year = 1000;
                    }
                    updateYear = true;                
                }      
            }
            //minus button
            else if(415 < p.x && p.x < 550 && 750 < p.y && p.y < 810){
                if(minuteSelected){
                    currentTime.minute -= 1;
                    if(currentTime.minute == -1){
                        currentTime.minute = 59;
                    }
                    updateMinute = true;
                    currentTime.second = 0;
                }
                else if(hourSelected){
                    currentTime.hour -= 1;
                    if(currentTime.hour == -1){
                        currentTime.hour = 23;
                        currentTime.am = false;
                    } 

                    if(currentTime.hour == 11){
                        currentTime.am = true;
                    }
                    updateHour = true;
                }
                else if(weekdaySelected){
                    for(int i=0; i <6; i++){
                        currentDate.weekday = nextDay(currentDate.weekday);
                    }
                    updateWeekday = true;
                }
                else if(monthSelected){
                    currentDate.month -= 1;
                    if(currentDate.month == 0){
                        currentDate.month = 12;                      
                    }
                    
                    if((currentDate.month % 2 == 0 && currentDate.month != 2) && currentDate.day > 30){
                        currentDate.day = 30;
                        updateDay = true;
                    }
                    else if(currentDate.month == 2 && currentDate.day > 28){
                        currentDate.day = 28;
                        updateDay = true;
                    }  
                    
                    updateMonth = true;
                }
                 else if(daySelected){
                    currentDate.day -= 1;
                    if((currentDate.month % 2 == 1 || currentDate.month == 7) && currentDate.day == 0){
                        currentDate.day = 31;
                    }
                    else if((currentDate.month % 2 == 0 && currentDate.month != 2) && currentDate.day == 0){
                        currentDate.day = 30;
                    }
                    else if(currentDate.month == 2 && currentDate.day == 0){
                        currentDate.day = 28;
                    }
                    updateDay = true;
                }

                else if(yearSelected){
                    currentDate.year -= 1;
                    if(currentDate.year == 1000){
                        currentDate.year = 9999;
                    }
                    updateYear = true; 
                }
            }
        }
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

void clearLCD(){
  tft.fillScreen(ILI9341_WHITE);
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
    else if(LCDState == setpointDisplay){
        drawSetpointDisplay();
    }
    else if(LCDState == timedateEdit){
        drawTimedateEdit();
    }
}

void drawPowersaving(){}

void drawBackButton(){
    tft.fillTriangle(
      22,291,
      12,271,
      32,271,
      ILI9341_BLACK);  
}

void drawMain(){
    /*Draw Boxes*/
    //program settings box
    tft.drawRect(47,8,33,81, ILI9341_BLACK);
    //time box
    tft.drawRect(7,8,33, 117, ILI9341_BLACK);
    //hold box
    tft.drawRect(95,8, 33,81, ILI9341_BLACK);
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
        tft.fillRect(155, 120, 25, 90, ILI9341_WHITE);
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
      
    tft.setRotation(3);
    tft.setCursor(15, 195);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_BLACK);
    tft.print("Mode:    ");
    if(HVACMode == none){
        tft.setTextColor(ILI9341_LIGHTGREY);
        tft.print("off");
    }
    else if(HVACMode == cool){
        tft.setTextColor(ILI9341_BLUE);
        tft.print("Cool");
        
    }
    else if(HVACMode == heat){
        tft.setTextColor(ILI9341_RED);
        tft.print("Heat");
    }
    else if(HVACMode == automatic){
        tft.setTextColor(ILI9341_BLACK);
        tft.print("Auto");
    }
    tft.setRotation(0);
    
    //forward mode button
    tft.fillTriangle(202, 218, 195, 204, 209, 204, ILI9341_BLACK);

    //back button mode
    tft.fillTriangle(202, 132, 195, 146, 209, 146, ILI9341_BLACK);

    /*Draw Current Temp*/
    tft.setRotation(3);
    tft.setCursor(20, 45);
    tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(7);
    if(currentTemp / 10 < 1){
      tft.print(" ");
    }
    tft.print(currentTemp);
    tft.println(" F");
    tft.setRotation(0);

    drawDateTime();

    /*Draw Hold*/
    if(updateHold){
        tft.setCursor(250,55);
        if(hold){
            tft.fillRect(47,8,33,81, ILI9341_GREEN);
        }
        else{
            tft.fillRect(47,8,33,81, ILI9341_LIGHTGREY);
        }
        tft.setRotation(3);
        tft.setTextSize(2);
        tft.println("Hold");
        tft.setRotation(0);  
        updateHold = false;
    }

    /*Draw Setpoint*/
    if(updateSetpoint){
        //clear temperature
        tft.fillRect(161, 18, 31, 61, ILI9341_WHITE);
        
        tft.setCursor(255,165);
        tft.setRotation(3);
        tft.setTextSize(3);
        tft.print(localTargetTemp);
        tft.setRotation(0);
        /*Plus Button*/
        tft.fillRect(203,64,17,5, ILI9341_RED); //verticle
        tft.fillRect(209,57,5,19, ILI9341_RED); //horizontal
        /*Minue*/
        tft.fillRect(209,22,5,17, ILI9341_BLUE); //horizontal
        updateSetpoint = false;
    }

    /*Draw Program*/
    tft.setCursor(252,101);
    tft.setRotation(3);
    tft.setTextSize(1);
    tft.print("Program");
    tft.setRotation(0);  

    tft.setCursor(246,113);
    tft.setRotation(3);
    tft.setTextSize(1);
    tft.print("Setpoints");
    tft.setRotation(0);  
    


    
}
    
void drawSetpointDisplay(){
    tft.drawRect(47, 15, 193, 289, ILI9341_BLACK); 
    tft.drawRect(7, 264, 33, 33, ILI9341_BLACK);
    tft.drawRect(7, 79, 25, 71, ILI9341_BLACK);
    tft.drawRect(7, 170, 25, 71, ILI9341_BLACK);
    tft.drawRect(215, 27, 17, 17, ILI9341_BLACK);
    tft.drawRect(215, 59, 17, 17, ILI9341_BLACK);
   // tft.drawRect(215, 99, 17, 17, ILI9341_BLACK);
   // tft.drawRect(215, 131, 17, 17, ILI9341_BLACK);
   // tft.drawRect(215, 171, 17, 17, ILI9341_BLACK);
   // tft.drawRect(215, 203, 17, 17, ILI9341_BLACK);
   // tft.drawRect(215, 243, 17, 17, ILI9341_BLACK);
   // tft.drawRect(215, 275, 17, 17, ILI9341_BLACK);
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

    drawBackButton();


    //Draw Weekday/weekend button
    tft.setCursor(90,20);
    tft.setTextColor(ILI9341_BLACK); 
    tft.setRotation(3);
    tft.setTextSize(2);
    tft.print("Weekday");
    tft.setRotation(0);  

    tft.setCursor(200 ,20);
    tft.setTextColor(ILI9341_BLACK); 
    tft.setRotation(3);
    tft.setTextSize(2);
    tft.print("Weekend");
    tft.setRotation(0);  


    //Draw headers
    tft.setCursor(40,52);
    tft.setTextColor(ILI9341_BLACK); 
    tft.setRotation(3);
    tft.setTextSize(1);
    tft.print("Hour");
    tft.setRotation(0); 

    tft.setCursor(105,52);
    tft.setTextColor(ILI9341_BLACK); 
    tft.setRotation(3);
    tft.setTextSize(1);
    tft.print("Minute");
    tft.setRotation(0); 

    tft.setCursor(185,52);
    tft.setTextColor(ILI9341_BLACK); 
    tft.setRotation(3);
    tft.setTextSize(1);
    tft.print("Temp.");
    tft.setRotation(0); 

    tft.setCursor(257,52);
    tft.setTextColor(ILI9341_BLACK); 
    tft.setRotation(3);
    tft.setTextSize(1);
    tft.print("Mode");
    tft.setRotation(0);

    //Draw hour buttons 2251
    tft.fillRect(216,249,16,4, ILI9341_BLACK); //verticle
    tft.fillRect(222,243,4,16, ILI9341_BLACK); //horizontal
        /*Minue*/
    tft.fillRect(222,275,4,16, ILI9341_BLACK); //horizontal

    //Draw min buttons
    tft.fillRect(216,178,16,4, ILI9341_BLACK); //verticle
    tft.fillRect(222,172,4,16, ILI9341_BLACK); //horizontal
    tft.fillRect(222,203,4,16, ILI9341_BLACK); //horizontal


    //Draw Temp buttons
    tft.fillRect(216,105,16,4, ILI9341_BLACK); //verticle
    tft.fillRect(222,99, 4,16, ILI9341_BLACK); //horizontal
    tft.fillRect(222,131,4,16, ILI9341_BLACK); //horizontal

    //Draw Mode buttons
}


void drawTimedateEdit(){
    tft.drawRect(7, 264, 33, 33, ILI9341_BLACK);
    tft.drawRect(39, 79, 25, 161, ILI9341_BLACK);
    tft.drawRect(71, 79, 25, 161, ILI9341_BLACK);
    tft.drawRect(103, 79, 25, 161, ILI9341_BLACK);
    tft.drawRect(135, 79, 25, 161, ILI9341_BLACK);
    tft.drawRect(167, 79, 25, 161, ILI9341_BLACK);
    tft.drawRect(199, 79, 25, 161, ILI9341_BLACK);

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
        if(currentTime.minute / 10 < 1){
            tft.print("0");
        }
        tft.print(currentTime.minute);
    }else{
        tft.setTextColor(ILI9341_LIGHTGREY);
        tft.print("Minute: ");
        if(currentTime.minute / 10 < 1){
            tft.print("0");
        }
        tft.print(currentTime.minute);
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
        if(currentTime.hour  > 12){
            tft.print(currentTime.hour % 12);
        }else{
            tft.print(currentTime.hour);
        }
    }else{
        tft.setTextColor(ILI9341_LIGHTGREY);
        tft.print("Hour: ");
        if(currentTime.hour  > 12){
            tft.print(currentTime.hour % 12);
        }else{
            tft.print(currentTime.hour);
        }
    }
    if(currentTime.am){
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
        tft.print(dayToString(currentDate.weekday));
    }else{
        tft.setTextColor(ILI9341_LIGHTGREY);
        tft.print("Weekday: ");
        tft.print(dayToString(currentDate.weekday));
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
        tft.print(monthToString(currentDate.month));
    }else{
        tft.setTextColor(ILI9341_LIGHTGREY);
        tft.print("Month: ");
        tft.print(monthToString(currentDate.month));
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
        if(currentDate.day / 10 < 1){
            tft.print("0");
        }
        tft.print(currentDate.day);
    }else{
        tft.setTextColor(ILI9341_LIGHTGREY);
        tft.print("Day: ");
        if(currentDate.day / 10 < 1){
            tft.print("0");
        }
        tft.print(currentDate.day);
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
        tft.print(currentDate.year);
    }else{
        tft.setTextColor(ILI9341_LIGHTGREY);
        tft.print("Year: ");
        tft.print(currentDate.year);
    }
    tft.setRotation(0);
    
    
    drawBackButton();
}

void drawDateTime(){
    /*Draw Time*/
    tft.setRotation(3);
    tft.setCursor(210,13);
    tft.setTextSize(1);
    tft.print(dayToString(currentDate.weekday));
    tft.print(" ");

    /*Draw Hour*/
    if(updateHour){
        tft.setRotation(0);
        tft.fillRect(8, 65, 14, 22, ILI9341_WHITE);
        tft.fillRect(8, 10, 14, 22, ILI9341_WHITE);
        tft.setRotation(3);
        updateHour = false;
    }
    
    if(currentTime.hour % 12 / 10 < 1){
      tft.print(" ");
    }
    if(currentTime.am){
        tft.print(currentTime.hour);
    }
    else{
        tft.print(currentTime.hour % 12);
    }
    tft.print(":");

    /*Draw Minute*/
    if(updateMinute){
        tft.setRotation(0);
        tft.fillRect(8, 45, 14, 25, ILI9341_WHITE);
        tft.setRotation(3);
        updateMinute = false;
    }
    if(currentTime.minute / 10 < 1){
        tft.print("0");
    }
    tft.print(currentTime.minute);
    
    tft.print(":");

    /*Draw Second*/
    if(updateSecond){
        tft.setRotation(0);
        tft.fillRect(8, 24, 15, 28, ILI9341_WHITE);
        tft.setRotation(3);
        updateSecond = false;
    }

    if(currentTime.second / 10 < 1){
        tft.print("0");
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
    tft.print(" ");
    tft.print(currentDate.day);
    tft.print(", ");
    tft.print(currentDate.year);
    tft.setRotation(0);
    
}

//use this one with small part of screen, possible speed improvement
void updateLCD(){

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
    if(targetTemp > currentTemp + tempBuffer && (HVACMode == heat or HVACMode == automatic)){
        HVACStatus = heatOn;
        if(lastHVACStatus != heatOn){
            updateHVACStatus = true;
        }   
        digitalWrite(heatPin, HIGH);
        digitalWrite(acPin, LOW);
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
        digitalWrite(heatPin, LOW);
        digitalWrite(acPin, LOW);  
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
