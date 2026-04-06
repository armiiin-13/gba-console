#include "LoadingScreen.h"

LoadingScreen::LoadingScreen(GameCard* game) : _game(game) {}

void LoadingScreen::enter() {
  _startTime = millis();
  _needsRedraw = true;
}

ConsoleState LoadingScreen::update() {
  if (millis() - _startTime > 2000) return activeGame;

  // Mientras "carga", devolvemos loading
  return loading;
}

void LoadingScreen::draw(Adafruit_ST7735& tft) {
  if (!_needsRedraw) return;

  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(20, 50);
  tft.print("CARGANDO...");

  if (_game != nullptr){
    tft.setCursor(20,70);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_GREEN);
    tft.print(_game->name);
  }

  _needsRedraw = false;
}

void LoadingScreen::exit() {}

bool LoadingScreen::getNeedsRedraw(){
  return _needsRedraw;
}