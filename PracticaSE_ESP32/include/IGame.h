#ifndef IGAME_H
#define IGAME_H

#include <Arduino.h>
#include <Adafruit_ST7735.h>
#include <Definitions.h>

// Forward declaration
struct InputState;

class IGame {
public:
  virtual void init() = 0;
  virtual void update(const InputState& in) = 0;
  virtual void render(Adafruit_ST7735& tft) = 0;
  virtual void exit() = 0;
  virtual ~IGame() {}
};

#endif