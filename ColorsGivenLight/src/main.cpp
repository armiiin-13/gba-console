#include <Arduino.h>

// PINs declaration
#define LIGHT A0
#define RED 6
#define GREEN 5
#define BLUE 3
#define BUTTON 13

int valueRed = 0;
int valueGreen = 0;
int valueBlue = 0;

int umbral = 600;

// Functions declaration
void setUpLed();
void upToRed();
void upToBlue();
void printRGBValues();

void setup() {
  pinMode(RED,OUTPUT);
  pinMode(GREEN,OUTPUT);
  pinMode(BLUE,OUTPUT);

  pinMode(BUTTON, INPUT_PULLUP);

  pinMode(LIGHT,INPUT);

  setUpLed();

  Serial.begin(9600);
}

void loop() {
  // Lectura LDR (fotoresistor)
  int ldrValue = analogRead(LIGHT);
  Serial.print("LDR Value = ");
  Serial.println(ldrValue);
  delay(500);

  if (digitalRead(BUTTON) == LOW){ // button pressed
    if (ldrValue < umbral){ // tendency to blue
      for (int i = 0; i < 5; i++){
        upToBlue();
        printRGBValues();
        delay(200);
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
  analogWrite(RED, valueRed);
  analogWrite(GREEN, valueGreen);
  analogWrite(BLUE, valueBlue);
}

void upToRed(){
  if (valueRed < 255){
    valueRed += 51;
    analogWrite(RED, valueRed);
  }
  if (valueBlue > 0){
    valueBlue -= 51;
    analogWrite(BLUE, valueBlue);
  }
}

void upToBlue(){
  if (valueBlue < 255){
    valueBlue += 51;
    analogWrite(BLUE, valueBlue);
  }
  if (valueRed > 0){
    valueRed -= 51;
    analogWrite(RED, valueRed);
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