// Drone Remote
// 4 button
// rf module
// oled display
// arduino nano

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <ArduinoJson.h>

StaticJsonDocument<200> doc;

Adafruit_SSD1306 display(-1);

RF24 radio(10, 9); // CE, CSN
const uint64_t pipeIn = 0xE6E6E6E6E6E6;
const uint64_t pipeOut = 0xE6E6E6E6E6D6;

const int upButton = 8; // blue
const int rightButton = 7; // green
const int downButton = 6; // orange
const int leftButton = 5; // brown

const int communicationLed = A1;
const int goLed = A2;
const int stopLed = A3;

const int ledBrightness = 200;
int directionGoing = 0; // 0=stop, 1=up, 2=right, 3=down, 4=left
float humid = 0;
float tempC = 0;
float tempF = 0;
long distanceIn = 0;
long distanceCm = 0;

char sensor[200] = {0};

void getRadio(){
  if (radio.available())
  {
    analogWrite(communicationLed, ledBrightness);
    radio.read(&sensor, sizeof(sensor));
    Serial.println(sensor);
    DeserializationError error = deserializeJson(doc, sensor);
    if (error) {
      humid = -1;
      tempC = -1;
      tempF = -1;
      distanceIn = -1;
      distanceCm = -1;
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
    }
    else{
      humid = doc[0];
      tempC = doc[1];
      tempF = doc[2];
      distanceIn = doc[3];
      distanceCm = doc[4];
    }
    analogWrite(communicationLed, 0);
  }
}

void sendUpdate() {
  radio.stopListening();
  char text[10] = {0};
  strcpy(text, "Stop");
  switch (directionGoing){
    case 1:
      strcpy(text, "Forward");
      break;
    case 2:
      strcpy(text, "Right");
      break;
    case 3:
      strcpy(text, "Backward");
      break;
    case 4:
      strcpy(text, "Left");
      break;
  }
  radio.write(&text, sizeof(text));
  radio.startListening();
}

void updateScreen(){
  display.clearDisplay();
  display.setTextSize(0.5);
  display.setTextColor(WHITE);
  display.setCursor(1,0);
  display.print("Direction: ");
  switch (directionGoing){
    case 0:
      display.println("Stopped");
      break;
    case 1:
      display.println("Forward");
      break;
    case 2:
      display.println("Right");
      break;
    case 3:
      display.println("Backward");
      break;
    case 4:
      display.println("Left");
      break;
  }
  
  display.setCursor(0,8);
  display.print("(C)");
  display.print(tempC);
  display.print(" (F)");
  display.println(tempF);
  
  display.print("Humidity: ");
  display.print(humid);
  display.println("%");

  display.print(distanceIn);
  display.print("in ");
  display.print(distanceCm);
  display.println("cm");
  
  display.display();
  display.clearDisplay();
}

void updateGoingLeds(){
  if (directionGoing != 0){
    analogWrite(goLed, ledBrightness);
    analogWrite(stopLed, 0);
  }
  else
  {
    analogWrite(goLed, 0);
    analogWrite(stopLed, ledBrightness);
  }
}

void checkButtons(){
  bool upPushed = digitalRead(upButton) == HIGH;
  bool rightPushed = digitalRead(rightButton) == HIGH;
  bool downPushed = digitalRead(downButton) == HIGH;
  bool leftPushed = digitalRead(leftButton) == HIGH;

  if (upPushed && !(rightPushed || downPushed || leftPushed)){
    directionGoing = 1;
  }
  else if (rightPushed && !(upPushed || downPushed || leftPushed)){
    directionGoing = 2;
  }
  else if (downPushed && !(rightPushed || upPushed || leftPushed)){
    directionGoing = 3;
  }
  else if (leftPushed && !(rightPushed || downPushed || upPushed)){
    directionGoing = 4;
  }
  else{
    directionGoing = 0;
  }
}

void setup() {
  pinMode(upButton, INPUT);
  pinMode(rightButton, INPUT);
  pinMode(downButton, INPUT);
  pinMode(leftButton, INPUT);
  pinMode(communicationLed, OUTPUT);
  pinMode(goLed, OUTPUT);
  pinMode(stopLed, OUTPUT);
  analogWrite(communicationLed, 0);
  analogWrite(goLed, 0);
  analogWrite(stopLed, 0);
  directionGoing = 0;

  radio.begin();
  radio.openWritingPipe(pipeOut);
  radio.openReadingPipe(0, pipeIn);
  radio.startListening();
  
  
  // put your setup code here, to run once:
 // initialize with the I2C addr 0x3C
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  
  // Clear the buffer.
  display.clearDisplay();

  // Display Text
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,10);
  display.println("Hello world!");
  display.display();
  delay(1000);
  display.clearDisplay();
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  checkButtons();
  updateGoingLeds();
  getRadio();
  updateScreen();
  sendUpdate();
  delay(1);
}
