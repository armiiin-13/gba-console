#include "CalendarScreen.h"

String daysOfWeek[] = {"S", "M", "T", "W", "Th", "F", "S"};

CalendarScreen::CalendarScreen(Button& b) : _btnB(b) {};

void CalendarScreen::enter() { _needsRedraw = true; }

ConsoleState CalendarScreen::update() {
  if (_btnB.isPressed()) return mainScreen;
  return calendar;
}

void CalendarScreen::draw(Adafruit_ST7735& tft) {
  if (!_needsRedraw) return;

  Theme theme = consoleConfig.getTheme();
  tft.fillScreen(theme.background);

  // Mes dinámico
  tft.setTextColor(theme.accent);
  tft.setTextSize(2);
  tft.setCursor(20, 5);
  tft.print(consoleConfig.isEnglish ? "MARCH 2026" : "MARZO 2026");

  // Días de la semana dinámicos
  const char* daysESP[] = {"L", "M", "X", "J", "V", "S", "D"};
  const char* daysENG[] = {"M", "T", "W", "T", "F", "S", "S"};

  tft.setTextSize(1);
  for (int i = 0; i < 7; i++) {
    tft.setTextColor(theme.accent);  // Asegura visibilidad en ambos modos
    tft.setCursor(10 + (i * 22), 30);
    tft.print(consoleConfig.isEnglish ? daysENG[i] : daysESP[i]);
  }

  // Los números de los días
  tft.setTextColor(theme.text);
  int dayCounter = 1;
  for (int row = 0; row < 5; row++) {
    for (int col = 0; col < 7; col++) {
      if (dayCounter <= 31) {
        tft.setCursor(10 + (col * 22), 45 + (row * 15));
        tft.print(dayCounter);
        dayCounter++;
      }
    }
  }

  _needsRedraw = false;
}
void CalendarScreen::exit() {}
bool CalendarScreen::getNeedsRedraw() { return _needsRedraw; }