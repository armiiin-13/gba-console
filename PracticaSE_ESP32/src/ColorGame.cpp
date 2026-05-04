#include <ColorGame.h>
#include <Adafruit_ST7735.h>
#include <Arduino.h>

void ColorGame::init() {
  currentColor = ST77XX_BLACK;
  needsRedraw = true;
  Serial.println("ColorGame init");
}

void ColorGame::update(const InputState& in) {
  if (in.pressedUp()) {
    currentColor = ST77XX_RED;
    needsRedraw = true;
    Serial.println("UP -> ROJO");
  }

  if (in.pressedDown()) {
    currentColor = ST77XX_GREEN;
    needsRedraw = true;
    Serial.println("DOWN -> VERDE");
  }

  if (in.pressedLeft()) {
    currentColor = ST77XX_BLUE;
    needsRedraw = true;
    Serial.println("LEFT -> AZUL");
  }

  if (in.pressedRight()) {
    currentColor = ST77XX_YELLOW;
    needsRedraw = true;
    Serial.println("RIGHT -> AMARILLO");
  }
}

void ColorGame::render(Adafruit_ST7735& tft, SoundManager& sound) {
  if (!needsRedraw) return;

  tft.fillScreen(currentColor);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(8, 10);
  tft.println("Color Game");

  tft.setCursor(8, 25);
  tft.println("UP rojo");

  tft.setCursor(8, 35);
  tft.println("DOWN verde");

  tft.setCursor(8, 45);
  tft.println("LEFT azul");

  tft.setCursor(8, 55);
  tft.println("RIGHT amarillo");

  needsRedraw = false;
}

void ColorGame::exit() {
  Serial.println("ColorGame exit");
}