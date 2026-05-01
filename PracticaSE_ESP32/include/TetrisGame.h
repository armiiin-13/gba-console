#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H

#include <IGame.h>

#define TG_CELL  6
#define TG_COLS  10
#define TG_ROWS  18
#define TG_BX    2     // board píxel X (left)
#define TG_BY    4     // board píxel Y (top)
#define TG_HX    65    // HUD píxel X

class TetrisGame : public IGame {
public:
  void init()                       override;
  void update(const InputState& in) override;
  void render(Adafruit_ST7735& tft, SoundManager& sound) override;
  void exit()                       override;

private:
  uint8_t  board[TG_ROWS][TG_COLS]; // 0=vacío, 1-7=color

  int8_t   curType, curRot, curX, curY;
  int8_t   nxtType;
  int8_t   prevType, prevRot, prevX, prevY;
  bool     pieceChanged;

  uint32_t score;
  uint16_t lines;
  uint8_t  level;

  bool     gameOver;
  bool     fullRedraw;
  bool     hudDirty;
  bool     needsRedraw;

  uint32_t lastFallMs;
  bool     dasLeft, dasRight;
  uint32_t dasTimer, dasRepeatTimer;
  
  int      lastClearedLines = 0;  // para sonidos de líneas
  uint32_t lastSoundMs = 0;       // para evitar sonidos muy seguidos

  void     reset();
  void     spawnPiece();
  bool     fits(int8_t type, int8_t rot, int8_t bx, int8_t by) const;
  void     doLock();
  int      countAndClear();
  void     addScore(int cleared);

  void     drawCell(Adafruit_ST7735& tft, int8_t bc, int8_t br, uint16_t color) const;
  void     paintPiece(Adafruit_ST7735& tft, int8_t type, int8_t rot,
                      int8_t bx, int8_t by, uint16_t color) const;
  void     drawBoard(Adafruit_ST7735& tft) const;
  void     drawHUD(Adafruit_ST7735& tft) const;
  void     drawGameOver(Adafruit_ST7735& tft) const;

  uint32_t fallMs() const;
  uint16_t pieceColor(int8_t idx) const;
};

#endif