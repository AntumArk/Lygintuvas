// this example is public domain. enjoy!
// https://learn.adafruit.com/thermocouple/

#include "max6675.h"

int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;

// Rotary Encoder Inputs
#define CLK 2
#define DT 3

#define LED 11//9

#define STOPLED 13

int counter = 0;
int currentStateCLK;
int lastStateCLK;
String currentDir ="";
#define ENCODER_SPEED 5


// THermal
long lastRead=0;
long readInterval=1000;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
float temperature=0;

#define MAXTEMP 350
#define MINTEMP 15


// Outputs
#define RELAY 7
long lastControl=0;
long controlInterval=3000;
long impulseLength=1200;

void setup() {
  // Set encoder pins as inputs
  pinMode(CLK,INPUT);
  
  pinMode(DT,INPUT);
  // Read the initial state of CLK
  lastStateCLK = digitalRead(CLK);
  // Call updateEncoder() when any high/low changed seen
  // on interrupt 0 (pin 2), or interrupt 1 (pin 3)
  attachInterrupt(0, updateEncoder, CHANGE);
  attachInterrupt(1, updateEncoder, CHANGE);
  
  Serial.begin(9600);

  // wait for MAX chip to stabilize
  delay(500); 
  pinMode(LED,OUTPUT);
  
  pinMode(STOPLED,OUTPUT);

  // Outputs
  pinMode(RELAY,OUTPUT);
}

void loop() { 

thermoLoop();
controlLoop();
delay(1);
}

void thermoLoop(){
 // For the MAX6675 to update, you must delay AT LEAST 250ms between reads!
if((millis()-lastRead)>readInterval){
  temperature=thermocouple.readCelsius();
    lastRead=millis();
    
   Serial.print(counter);
   Serial.print('\t');
   Serial.println(temperature);
}

}
void updateEncoder(){
  // Read the current state of CLK
  currentStateCLK = digitalRead(CLK);

  // If last and current state of CLK are different, then pulse occurred
  // React to only 1 state change to avoid double count
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){

    // If the DT state is different than the CLK state then
    // the encoder is rotating CCW so decrement
    if (digitalRead(DT) == currentStateCLK) {
      counter -=ENCODER_SPEED;
      currentDir ="CCW";
    } else {
      // Encoder is rotating CW so increment
      counter +=ENCODER_SPEED;
      currentDir ="CW";
    }

//    Serial.print("Direction: ");
//    Serial.print(currentDir);
//    Serial.print(" | Counter: ");
//    Serial.println(counter);
  }

  // Remember last CLK state
  lastStateCLK = currentStateCLK;
  limitCounter(counter);
  updateLED(counter);
}

int limitCounter(int currentCounter){
  if(currentCounter>MAXTEMP){
    currentCounter= MAXTEMP;
    digitalWrite(STOPLED,HIGH);
  }
  else{
    
    digitalWrite(STOPLED,LOW);
  }
 
 currentCounter= currentCounter<MINTEMP?MINTEMP:currentCounter;
}

void updateLED(int currentCounter){
  int mappedValue=map(currentCounter, MINTEMP, MAXTEMP, 0, 254);
   analogWrite(LED, mappedValue); 
}

void controlLoop(){
  if((millis()-lastControl)>controlInterval){

 float difference=temperature-counter;
    
  if(temperature<counter){
    digitalWrite(RELAY,HIGH);
    delay(impulseLength);
    digitalWrite(RELAY,LOW);
  }
  else
  {
    digitalWrite(RELAY,LOW);
  }
  lastControl=millis();
  }
}
