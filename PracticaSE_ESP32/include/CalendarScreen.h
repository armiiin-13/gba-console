#include "Button.h"
#include "Screen.h"

class CalendarScreen : public Screen {
 private:
  bool _needsRedraw = true;
  Button& _btnB;

 public:
  CalendarScreen(Button& b);
  void enter() override;
  ConsoleState update() override;
  void draw(Adafruit_ST7735& tft) override;
  void exit() override;
  bool getNeedsRedraw() override;
};