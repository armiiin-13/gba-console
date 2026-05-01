#ifndef MEMORY_COLOR_GAME_H
#define MEMORY_COLOR_GAME_H

#include <Arduino.h>
#include <Adafruit_ST7735.h>
#include <IGame.h>

#define MAX_SEQUENCE 64

class MemoryGame : public IGame {
public:
  void init() override;
  void update(const InputState& in) override;
  void render(Adafruit_ST7735& tft, SoundManager& sound) override;
  void exit() override;

private:
  enum GameState {
    GAME_TURN_TEXT,
    GAME_SHOW_SEQUENCE,
    PLAYER_TURN_TEXT,
    PLAYER_INPUT,
    GAME_OVER
  };

  enum ColorButton {
    COLOR_UP = 0,
    COLOR_DOWN = 1,
    COLOR_LEFT = 2,
    COLOR_RIGHT = 3
  };

  GameState state;

  int sequence[MAX_SEQUENCE];
  int sequenceLength;
  int showIndex;
  int playerIndex;

  int activeColor;
  int lastActiveColor;
  int pendingSound;

  int lastSequenceLength;

  unsigned long stateStart;
  unsigned long colorStart;

  bool fullRedraw;
  bool hudDirty;
  bool padDirty;
  bool messageDirty;

  void addRandomColor();
  void startGameTurn();
  void startPlayerTurn();
  void handlePlayerColor(int color);
  void activateColor(int color);
  int getSoundForColor(int color);

  void drawStatic(Adafruit_ST7735& tft);
  void drawHud(Adafruit_ST7735& tft);
  void drawMessage(Adafruit_ST7735& tft);
  void drawColorPad(Adafruit_ST7735& tft);
  void redrawSingleButton(Adafruit_ST7735& tft, int color);
  void drawButton(Adafruit_ST7735& tft, int color, int x, int y, uint16_t baseColor);
};

#endif