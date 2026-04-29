#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include <IGame.h>

#define SG_CELL 8
#define SG_COLS 20                     // 160 / 8
#define SG_ROWS 14                     // (128 - 16) / 8
#define SG_HUD_H 16                    // píxeles reservados para HUD
#define SG_MAXLEN (SG_COLS * SG_ROWS)  // 280 celdas máx

enum SnakeDir { SDIR_RIGHT, SDIR_LEFT, SDIR_UP, SDIR_DOWN };
enum SnakePhase { PHASE_PLAYING, PHASE_DEAD };

struct Pt {
  int8_t x, y;
};

class SnakeGame : public IGame {
 public:
  void init() override;
  void update(const InputState& in) override;
  void render(Adafruit_ST7735& tft) override;
  void exit() override;

 private:
  Pt body[SG_MAXLEN];  // buffer circular: body[tailIdx..headIdx]
  uint16_t headIdx, tailIdx;
  uint16_t length;

  SnakeDir dir, nextDir;
  Pt food;
  uint16_t score;
  SnakePhase phase;

  uint32_t lastMoveMs;
  uint32_t moveInterval;

  bool needsRedraw;  // algo cambió, hay que pintar
  bool fullRedraw;   // repintar pantalla entera
  bool tailMoved;    // la cola avanzó (hay que borrarla)
  Pt prevTail;       // posición anterior de la cola

  void reset();
  void spawnFood();
  bool isOccupied(Pt p) const;
  void fillCell(Adafruit_ST7735& tft, Pt p, uint16_t color) const;
  void renderHUD(Adafruit_ST7735& tft) const;
  void renderGameOver(Adafruit_ST7735& tft) const;
  uint32_t speedMs() const;
};

#endif