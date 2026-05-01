#ifndef COLOR_GAME_H
#define COLOR_GAME_H

#include <IGame.h>

class ColorGame : public IGame {
private:
  uint16_t currentColor;
  bool needsRedraw;

public:
  void init() override;
  void update(const InputState& in) override;
  void render(Adafruit_ST7735& tft, SoundManager& sound) override;
  void exit() override;
};

#endif