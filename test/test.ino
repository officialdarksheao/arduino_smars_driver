// Drone
// flexible sensors
// rf module
// hbridge controls

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <DHT.h>
#include <ArduinoJson.h>

StaticJsonDocument<200> doc;

RF24 radio(48, 49); // CE, CSN
const uint64_t pipeOut = 0xE6E6E6E6E6E6;
const uint64_t pipeIn = 0xE6E6E6E6E6D6;

#define DHTTYPE    DHT11     // DHT 11
#define DHTPIN    22
DHT dht(DHTPIN, DHTTYPE);

void forward(){
  digitalWrite(2, HIGH);
  digitalWrite(3, LOW);
  digitalWrite(4, HIGH);
  digitalWrite(5, LOW);
}
void backward(){
  digitalWrite(2, LOW);
  digitalWrite(3, HIGH);
  digitalWrite(4, LOW);
  digitalWrite(5, HIGH);
}
void left(){
  digitalWrite(2, HIGH);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, HIGH);
}
void right(){
  digitalWrite(2, LOW);
  digitalWrite(3, HIGH);
  digitalWrite(4, HIGH);
  digitalWrite(5, LOW);
}
void stopMoving(){
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
}

const int delayBetweenCalls = 1000;
int count = 0;
char sensor[200] = {0};
const int pingPin = 24;
const int echoPin = 25;

void getUpdate(){
  // recieve from remote
  char text[10] = {0};
  if (radio.available())
  {
    radio.read(&text, sizeof(text));
    // Serial.print("Recieved: ");
    // Serial.println(text);
    if (!strcmp(text, "Forward")){
      forward();
      // Serial.println("Going Forward");
    }
    else if (!strcmp(text, "Right")){
      right();
      // Serial.println("Going Right");
    }
    else if (!strcmp(text, "Backward")){
      backward();
      // Serial.println("Going Backward");
    }
    else if (!strcmp(text, "Left")){
      left();
      // Serial.println("Going Left");
    }
    else{
      stopMoving();
      // Serial.println("Stopping");
    }
  }
  
}

long lastCm = 0;
long lastIn = 0;

long microsecondsToInches(long microseconds) {
   return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds) {
   return microseconds / 29 / 2;
}

void sendUpdateIfNeeded(){
  if (count > delayBetweenCalls){
    radio.stopListening();
    count = 0;
    float t = dht.readTemperature();
    float f = (t * 9.0) / 5.0 + 32.0;
    float h = dht.readHumidity();
    if (isnan(h) || isnan(t)) {
      f = -2;
      t = -2;
    }
    long duration, inches, cm;
    digitalWrite(pingPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(pingPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    inches = microsecondsToInches(duration);
    cm = microsecondsToCentimeters(duration);

    if (inches > 1000){
      inches = lastIn;
    }
    else{
      lastIn = inches;
    }
    if (cm > 3000){
      cm = lastCm;
    }
    else{
      lastCm = cm;
    }

    doc[0] = h;
    doc[1] = t;
    doc[2] = f;
    doc[3] = inches;
    doc[4] = cm;

    serializeJson(doc, sensor);

    // Serial.println(sensor);
    radio.write(&sensor, sizeof(sensor));
    radio.startListening();
  }
}

void setup() {
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);

  pinMode(pingPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(pingPin, LOW);
  
  stopMoving();

  dht.begin();
  count = 0;
  
  radio.begin();
  radio.openWritingPipe(pipeOut);
  radio.openReadingPipe(0, pipeIn);
  radio.startListening();

  // Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  count++;
  sendUpdateIfNeeded();
  getUpdate();
  delay(1);
}
