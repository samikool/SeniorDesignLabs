#include <OneWire.h>
#include <DallasTemperature.h>

const int buttonPin = 2;
const int tempDataPin = 4;
String testing;
int buttonState = 0;
OneWire oneWire(tempDataPin);
DallasTemperature sensors(&oneWire);


void setup() {
  pinMode(tempDataPin, INPUT);

  Serial.begin(9600);
  sensors.begin();
  Serial.println("Starting Loop");
 
}

void loop() {
  testing = "";
  buttonState = digitalRead(buttonPin);
  testing = Serial.readString();

    
  if(buttonState == LOW){
    sensors.requestTemperatures();
    Serial.println("High");
    
    Serial.print("Celsius temperature: ");
    Serial.print(sensors.getTempCByIndex(0)); 
    Serial.print(" - Fahrenheit temperature: ");
    Serial.println(sensors.getTempFByIndex(0));

  }else{

    Serial.println("-");
  }

  delay(100);

}
