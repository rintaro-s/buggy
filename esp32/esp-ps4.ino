#include <PS4Controller.h>

const int in1Pin = 22;
const int in2Pin = 23;
const int pwmPin = 21;
const int trigPin = 8;
const int echoPin = 9;
const int distanceThreshold = 30;

void setup() {
  Serial.begin(115200);
  PS4.begin("90:38:0c:ed:83:e6");
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(pwmPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

long readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH);
  long distance = (duration / 2) / 29.1;
  return distance;
}

void loop() {
  if (!PS4.isConnected()) {
    PS4.begin("c8:f0:9e:a0:77:ae");
    delay(1000);
  } else {
    int x = PS4.LStickY();
    int speed = map(x, -128, 127, -255, 255);
    long distance = readDistance();

    if (distance > distanceThreshold) {
      if (speed > 0) {
        digitalWrite(in1Pin, HIGH);
        digitalWrite(in2Pin, LOW);
      } else {
        digitalWrite(in1Pin, LOW);
        digitalWrite(in2Pin, HIGH);
        speed = -speed;
      }
      analogWrite(pwmPin, speed);
    } else {
      digitalWrite(in1Pin, LOW);
      digitalWrite(in2Pin, LOW);
      analogWrite(pwmPin, 0);
    }
  }
}
