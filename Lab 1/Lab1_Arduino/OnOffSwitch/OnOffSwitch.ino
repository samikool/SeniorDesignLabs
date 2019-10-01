
int RockerSwitch = 7;   // choose the input pin (for a pushbutton)
int val = 0;     // variable for reading the pin status

void setup() {
  Serial.begin(9600);
  pinMode(RockerSwitch, INPUT);    // declare pushbutton as input
}

void loop(){
  val = digitalRead(RockerSwitch);  // read input value
  
  if (val == HIGH) {         // check if the input is HIGH (button pressed)
    Serial.println("ON");
    
  } else {
    Serial.println("----");  //Output if Button is released
  }

  delay(100);
}
 
