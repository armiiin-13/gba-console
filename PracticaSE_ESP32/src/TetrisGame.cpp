#include "TetrisGame.h"

void TetrisGame::init() {
  // TODO: Inicializar el juego Tetris
}

void TetrisGame::update(const InputState& in) {
  // TODO: Actualizar lógica del juego Tetris
}

void TetrisGame::render(Adafruit_ST7735& tft) {
  // TODO: Renderizar el juego Tetris
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(30, 50);
  tft.println("Tetris Game");
  tft.setCursor(20, 80);
  tft.println("Proximamente...");
}

void TetrisGame::exit() {
  // TODO: Limpiar recursos del juego Tetris
}
