
int PushPin = 2;   // choose the input pin (for a pushbutton)
int val = 0;     // variable for reading the pin status

void setup() {
  Serial.begin(9600);
  pinMode(PushPin, INPUT);    // declare pushbutton as input
}

void loop(){
  val = digitalRead(PushPin);  // read input value
  
  if (val == HIGH) {         // check if the input is HIGH (button pressed)
    Serial.println("Pressed");
    
  } else {
    Serial.println("----");  //Output if Button is released
  }

  delay(20);
}
 
