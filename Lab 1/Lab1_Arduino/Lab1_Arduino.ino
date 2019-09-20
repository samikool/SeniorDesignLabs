#include <OneWire.h>
#include <DallasTemperature.h>

const int buttonPin = 2;
const int tempDataPin = 4;
String testing;
int buttonState = 0;
OneWire oneWire(tempDataPin);
DallasTemperature sensors(&oneWire);

int sensetest = 0;

void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  //delay(100);
  //pinMode(tempDataPin, INPUT);

  Serial.begin(9600);
  sensors.begin();
  Serial.println("Starting Loop");
 
  //delay(1000);
}

void loop() {
 // delay(100);
  testing = "";
  buttonState = digitalRead(buttonPin);
  testing = Serial.readString();
  //delay(250);
  //delay(100);
    
  if(buttonState == HIGH){
    sensors.requestTemperatures();
    Serial.println("High");
    
    Serial.print("Celsius temperature: ");
    // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
    //Serial.print(sensors.getTempCByIndex(0)); 
    Serial.print(" - Fahrenheit temperature: ");
    //Serial.println(sensors.getTempFByIndex(0));
  }else{
    //testing = Serial.read();
    Serial.print(testing);
    Serial.flush();
    //delay(250);
    if(testing == 1){
        digitalWrite(LED_BUILTIN, HIGH);
    }
    //Serial.print("Testing: ");
    //Serial.println(testing);
    //Serial.println("-");
  }

  delay(1000);

}
