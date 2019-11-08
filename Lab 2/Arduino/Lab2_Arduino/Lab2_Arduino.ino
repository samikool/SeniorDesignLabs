int analogPin = A0;
int ledPin = 2; 
int val = 0;
int average = 0;
long sum = 0;
void setup(){
   Serial.begin(9600);
   pinMode(ledPin, OUTPUT);
}

void loop(){
  for(int i=0; i<100; i++){
    val = analogRead(analogPin);
    sum = sum + val;
  }
  average = sum / 100;


  if(average == 0){
    digitalWrite(ledPin, HIGH);
    Serial.println("1");
  }else{
    digitalWrite(ledPin, LOW);
    Serial.println("0");  
  }
  sum = 0;
  delay(30);
}
