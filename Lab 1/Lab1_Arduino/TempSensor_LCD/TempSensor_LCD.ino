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


//Variables and constants
int x, y;
String TextOut;

const int tempDataPin = 4;
String testing;
OneWire oneWire(tempDataPin);
DallasTemperature sensors(&oneWire);


void setup() {
  pinMode(tempDataPin, INPUT);

  Serial.begin(9600);
  sensors.begin();
  tft.begin();
  tft.clear();
  Serial.println("Starting Loop");
 
}

void loop() {
  testing = "";
  testing = Serial.readString();

  sensors.requestTemperatures();
  Serial.println("High");
  Serial.print("Celsius temperature: ");
  Serial.print(sensors.getTempCByIndex(0)); 
  Serial.print(" - Fahrenheit temperature: ");
  Serial.println(sensors.getTempFByIndex(0));

  out = sensors.getTempCByIndex(0);
  tft.setFont(Terminal12x16);
  tft.drawText(20, 60, "Temperature:", COLOR_RED);
  tft.drawText(45, 120, TextOut, COLOR_TOMATO);
  tft.drawText(110, 120, "*C", COLOR_TOMATO);

  delay(500);

}
