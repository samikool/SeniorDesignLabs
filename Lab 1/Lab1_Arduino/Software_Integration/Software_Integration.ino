#include <OneWire.h>
#include <DallasTemperature.h>
#include "SPI.h"
#include "TFT_22_ILI9225.h"

#if defined (ARDUINO_ARCH_STM32F1)
#define TFT_RST PA1
#define TFT_RS  PA2
#define TFT_CS  PA0 // SS
#define TFT_SDI PA7 // MOSI
#define TFT_CLK PA5 // SCK
#define TFT_LED 0 // 0 if wired to +5V directly
#elif defined(ESP8266)
#define TFT_RST 4   // D2
#define TFT_RS  5   // D1
#define TFT_CLK 14  // D5 SCK
//#define TFT_SDO 12  // D6 MISO
#define TFT_SDI 13  // D7 MOSI
#define TFT_CS  15  // D8 SS
#define TFT_LED 2   // D4     set 0 if wired to +5V directly -> D3=0 is not possible !!
#elif defined(ESP32)
#define TFT_RST 26  // IO 26
#define TFT_RS  25  // IO 25
#define TFT_CLK 14  // HSPI-SCK
//#define TFT_SDO 12  // HSPI-MISO
#define TFT_SDI 13  // HSPI-MOSI
#define TFT_CS  15  // HSPI-SS0
#define TFT_LED 0 // 0 if wired to +5V directly
#else
#define TFT_RST 8
#define TFT_RS  9
#define TFT_CS  10  // SS
#define TFT_SDI 11  // MOSI
#define TFT_CLK 13  // SCK
#define TFT_LED 3   // 0 if wired to +5V directly
#endif

#define TFT_BRIGHTNESS 200 // Initial brightness of TFT backlight (optional)

// Use hardware SPI (faster - on Uno: 13-SCK, 12-MISO, 11-MOSI)
TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_LED, TFT_BRIGHTNESS);

boolean tempProbeConnected();
void displayDisconnected();

//Variables and constants
int x, y;
String TextOut;

int RockerSwitch = 7;   // choose the input pin (for a pushbutton)
int valRocker = 0;     // variable for reading the pin status of RockerSwitch
int PushPin = 2;   // choose the input pin (for a pushbutton)
int valPush = 0;     // variable for reading the pushbutton pin status
const int tempDataPin = 4;
String input = "0";
int state = 0;
OneWire oneWire(tempDataPin);
DallasTemperature sensors(&oneWire);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(RockerSwitch, INPUT);    // declare Switch as input
  pinMode(PushPin, INPUT);    // declare pushbutton as input
  pinMode(tempDataPin, INPUT);
  
  sensors.begin();

  //initialize display
  tft.begin();
  tft.clear();
  tft.setFont(Terminal12x16);
}

void loop() {
  // put your main code here, to run repeatedly
  valRocker = digitalRead(RockerSwitch);  // read input value

  
  
//If Switch is turned on
  if (valRocker == LOW) { // check if the Switch is (- On)[Wired backwards]
      sensors.requestTemperatures();
      if (tempProbeConnected()){//check if probe connected
        //send data
        //Serial.println(sensors.getTempCByIndex(0));
        
        /*if(Serial.available() != 0){
          input = Serial.readString();
          state = input.toInt();
        }
        //Serial.println(Serial.available());
        //Serial.println(input); 
        if(state == 1){
          Serial.println(sensors.getTempCByIndex(0));
        }else{
          Serial.println("0");
        }*/
        
        while(Serial.available() == 0){
   
        }

        if(Serial.available() > 0){
          input = Serial.readString();
          state = input.toInt();
        }
       

        if(state == 1){
          Serial.println(sensors.getTempCByIndex(0));
        }else{
          Serial.println("0");
        }
        Serial.flush();

        //read uboyt value of pushbutton
        valPush = digitalRead(PushPin);  
        delay(10);

        //if button is pressed
        if (valPush == HIGH) {      
          //Output to LCD if Pushbutton is Pressed
          tft.setFont(Terminal12x16);
          tft.drawText(20, 60, "Temperature:", COLOR_RED);
          
          TextOut = sensors.getTempCByIndex(0);
      
          tft.drawText(45, 120, TextOut, COLOR_TOMATO);
          tft.drawText(110, 120, "*C", COLOR_TOMATO);
        }else{
          //if button is low erase text
          tft.clear();
          tft.setBackgroundColor(COLOR_BLACK);
        }
      }else {
        //if probe disconnected display error on screen
        displayDisconnected();
      }    
    }
    else{ 
      //If power switch is off
      tft.clear();
      tft.setBackgroundColor(COLOR_BLACK);
     }
  delay(10);
}

boolean tempProbeConnected(){
    if (sensors.getTempCByIndex(0) == -127){
        //Serial.print("Sensor not connected");
        return false;
    }
    return true;
}

void displayDisconnected(){
      tft.setFont(Terminal12x16);
      tft.drawText(0, 60, "Temperature Probe", COLOR_RED);    
      tft.drawText(0, 90, "Not Connected", COLOR_RED); 
}
