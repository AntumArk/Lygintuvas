// this example is public domain. enjoy!
// https://learn.adafruit.com/thermocouple/

#include "max6675.h"

int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;

// Rotary Encoder Inputs
#define CLK 2
#define DT 3
#define POWER 8

#define LED 11 // 9

#define STOPLED 13

int counter = 0;
int currentStateCLK;
int lastStateCLK;
String currentDir = "";
#define ENCODER_SPEED 5

int powerOn = 0;
int currentStatePower = 0;
int lastStatePower = 0;

// THermal
long lastRead = 0;
long readInterval = 1000;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
float temperature = 0;

#define MAXTEMP 350
#define MINTEMP 15

// Outputs
#define RELAY 7
long lastControl = 0;
long controlInterval = 3000;
long impulseLength = 1200;

/* OLED
/**************************************************************************
 This is an example for our Monochrome OLEDs based on SSD1306 drivers

 Pick one up today in the adafruit shop!
 ------> http://www.adafruit.com/category/63_98

 This example is for a 128x64 pixel display using I2C to communicate
 3 pins are required to interface (two I2C and one reset).

 Adafruit invests time and resources providing this open
 source code, please support Adafruit and open-source
 hardware by purchasing products from Adafruit!

 Written by Limor Fried/Ladyada for Adafruit Industries,
 with contributions from the open source community.
 BSD license, check license.txt for more information
 All text above, and the splash screen below must be
 included in any redistribution.
 **************************************************************************/

#include <SPI.h
#include <Wire.h>
#include <Adafruit_GFX.h> 
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
long lastDisplay = 0;
long displayInterval = 100;

void setupOLED()
{
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
}

void screenLoop()
{
  if ((millis() - lastDisplay) > displayInterval)
  {

    // Clear the buffer
    display.clearDisplay();

    drawValues();

    lastDisplay = millis();
  }
}

void drawValues(void)
{
  display.clearDisplay();
  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
  display.print("Goal: ");
  display.print(counter); // 1 digit after the decimal point
  display.println(" C");
  display.setTextSize(2); // Draw 2X-scale text
  display.print("T: ");
  display.print(temperature, 1); // ditto
  display.println("C");
  display.setTextSize(1); // Draw 2X-scale text
  display.print("Power ");
  display.print(powerOn);
  display.display(); // Show initial text
}

void setup()
{
  // Set encoder pins as inputs
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(POWER, INPUT);

  // Read the initial state of CLK
  lastStateCLK = digitalRead(CLK);
  // Call updateEncoder() when any high/low changed seen
  // on interrupt 0 (pin 2), or interrupt 1 (pin 3)
  attachInterrupt(0, updateEncoder, CHANGE);
  attachInterrupt(1, updateEncoder, CHANGE);

  Serial.begin(9600);

  // wait for MAX chip to stabilize
  delay(500);
  pinMode(LED, OUTPUT);

  pinMode(STOPLED, OUTPUT);

  // Outputs
  pinMode(RELAY, OUTPUT);

  setupOLED();
}

void loop()
{

  thermoLoop();
  controlLoop();
  screenLoop();
  powerBtnLoop();
  delay(1);
}

// Simulate rising edge
void powerBtnLoop()
{

  // Read the current state of POWER . Inverted due to pull up
  currentStatePower = !digitalRead(POWER);

  // If last and current state of POWER are different, then pulse occurred
  // React to only 1 state change to avoid double count
  if (currentStatePower != lastStatePower && currentStatePower == 1)
  {

    powerOn = !powerOn;
  }

  // Remember last CLK state
  lastStatePower = currentStatePower;
}

void thermoLoop()
{
  // For the MAX6675 to update, you must delay AT LEAST 250ms between reads!
  if ((millis() - lastRead) > readInterval)
  {
    temperature = thermocouple.readCelsius();
    lastRead = millis();

    Serial.print(counter);
    Serial.print('\t');
    Serial.println(temperature);
  }
}
void updateEncoder()
{
  // Read the current state of CLK
  currentStateCLK = digitalRead(CLK);

  // If last and current state of CLK are different, then pulse occurred
  // React to only 1 state change to avoid double count
  if (currentStateCLK != lastStateCLK && currentStateCLK == 1)
  {

    // If the DT state is different than the CLK state then
    // the encoder is rotating CCW so decrement
    if (digitalRead(DT) == currentStateCLK)
    {
      counter -= ENCODER_SPEED;
      currentDir = "CCW";
    }
    else
    {
      // Encoder is rotating CW so increment
      counter += ENCODER_SPEED;
      currentDir = "CW";
    }
  }

  // Remember last CLK state
  lastStateCLK = currentStateCLK;
  counter=limitCounter(counter);
  updateLED(counter);
}

int limitCounter(int currentCounter)
{
  if (currentCounter > MAXTEMP)
  {
    currentCounter = MAXTEMP;
    digitalWrite(STOPLED, HIGH);
  }
  else
  {

    digitalWrite(STOPLED, LOW);
  }

  return currentCounter = currentCounter < MINTEMP ? MINTEMP : currentCounter;
}

void updateLED(int currentCounter)
{
  int mappedValue = map(currentCounter, MINTEMP, MAXTEMP, 0, 254);
  analogWrite(LED, mappedValue);
}

void controlLoop()
{
  if ((millis() - lastControl) > controlInterval)
  {

    float difference = temperature - counter;

    if (temperature < counter && powerOn)
    {
      digitalWrite(RELAY, HIGH);
      delay(impulseLength);
      digitalWrite(RELAY, LOW);
    }
    else
    {
      digitalWrite(RELAY, LOW);
    }
    lastControl = millis();
  }
}
