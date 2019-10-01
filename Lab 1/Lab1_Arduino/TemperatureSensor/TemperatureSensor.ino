#include <OneWire.h>
#include <DallasTemperature.h>

const int tempDataPin = 4;
String testing;
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
  testing = Serial.readString();

    
    sensors.requestTemperatures();

    if (sensors.getTempCByIndex(0) == -127){
      Serial.print("Sensor not connected");
    }
    
    Serial.print("Celsius temperature: ");
    Serial.print(sensors.getTempCByIndex(0)); 
    Serial.print(" - Fahrenheit temperature: ");
    Serial.println(sensors.getTempFByIndex(0));

  delay(1000);

}
