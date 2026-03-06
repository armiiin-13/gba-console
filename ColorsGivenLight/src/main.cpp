#include <Arduino.h>

// pin declaration
int button = 13;
int ledRed = 6;
int ledGreen = 5;
int ledBlue = 3;
int ldr = A0;

int valueRed = 0;
int valueGreen = 123;
int valueBlue = 0;

int umbral = 600;

// functions declaration
void setUpLed();
void upToRed();
void upToBlue();
void printRGBValues();

void setup() {
  pinMode(ledRed,OUTPUT);
  pinMode(ledGreen,OUTPUT);
  pinMode(ledBlue,OUTPUT);
  pinMode(button, INPUT);
  pinMode(ldr,INPUT);

  setUpLed();

  Serial.begin(9600);
}

void loop() {
  int ldrValue = analogRead(ldr);
  Serial.print("LDR Value = ");
  Serial.println(ldrValue);
  delay(500);
  int buttonValue = digitalRead(button);
  if (buttonValue == HIGH){ // button pressed
    if (ldrValue < umbral){ // tendency to blue
      for (int i = 0; i < 5; i++){
        upToBlue();
        printRGBValues();
        delay(500);
      }
    } else { // tendency to red
      for (int i = 0; i < 5; i++){
        upToRed();
        printRGBValues();
        delay(200);
      }
    }
  }
}

void setUpLed(){
  analogWrite(ledRed, valueRed);
  analogWrite(ledGreen, valueGreen);
  analogWrite(ledBlue, valueBlue);
}

void upToRed(){
  if (valueRed < 255){
    valueRed += 51;
    analogWrite(ledRed, valueRed);
  }
  if (valueBlue > 0){
    valueBlue -= 51;
    analogWrite(ledBlue, valueBlue);
  }
}

void upToBlue(){
  if (valueBlue < 255){
    valueBlue += 51;
    analogWrite(ledBlue, valueBlue);
  }
  if (valueRed > 0){
    valueRed -= 51;
    analogWrite(valueRed, valueRed);
  }
}

void printRGBValues(){
  Serial.print("RGB Led = (");
  Serial.print(valueRed);
  Serial.print(", ");
  Serial.print(valueGreen);
  Serial.print(", ");
  Serial.print(valueBlue);
  Serial.println(")");
}