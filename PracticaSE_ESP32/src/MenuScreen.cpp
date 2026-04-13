#include "MenuScreen.h"

MenuScreen::MenuScreen(Button &up, Button &down, Button &a, GameCard **cardRef)
    : _btnUp(up), _btnDown(down), _btnA(a), _activeCardRef(cardRef) {}

void MenuScreen::enter() { 
  Serial.println("MenuScreen ENTER");
  _needsRedraw = true; 
}

ConsoleState MenuScreen::update()
{
  if (_btnUp.isPressed())
  {
    int old = _cursorMenu;

    _cursorMenu--;
    if (_cursorMenu < 0)
      _cursorMenu = 2;

    if (old != _cursorMenu)
    {
      _needsRedraw = true;
    }
  }

  if (_btnDown.isPressed())
  {
    int old = _cursorMenu;

    _cursorMenu++;
    if (_cursorMenu > 2)
      _cursorMenu = 0;

    if (old != _cursorMenu)
    {
      _needsRedraw = true;
    }
  }
  /*
      _cursorMenu:
      0: Juego
      1: Calendario
      2: Ajustes
  */
  if (_btnA.isPressed())
  {
    if (_cursorMenu == 0) {
      if (_activeCardRef != nullptr && *_activeCardRef != nullptr) {
        return STATE_LOADING_GAME;
      }
      return STATE_MENU;
    }
    if (_cursorMenu == 1)
      return STATE_CALENDAR;
    if (_cursorMenu == 2)
      return STATE_SETTINGS;
  }

  return STATE_MENU;
}

void MenuScreen::draw(Adafruit_ST7735 &tft)
{
  if (!_needsRedraw)
    return;

  Theme theme = consoleConfig.getTheme(); // Obtenemos colores dinámicos
  tft.fillScreen(theme.background);
  int langIdx = consoleConfig.isEnglish ? 1 : 0; // 0=ESP, 1=ENG
  tft.setTextSize(1);

  // Título dinámico
  const char *txt_main[] = {"MENU PRINCIPAL", "MAIN MENU"};

  tft.setCursor(10, 10);
  tft.setTextColor(theme.accent);
  tft.print(txt_main[langIdx]);

  // Opción JUGAR
  const char *txt_gameNull[] = {"[SIN JUEGO]", "[NO GAME]"};
  GameCard *current = (_activeCardRef != nullptr) ? *_activeCardRef : nullptr;
  String gameName =
      (current != nullptr) ? current->name : txt_gameNull[langIdx];

  tft.setTextSize(1);
  tft.setCursor(15, 30);
  if (_cursorMenu == 0)
  {
    tft.setTextColor(theme.accent); // Color resaltado (Cian/Azul)
    tft.print("> ");
  }
  else
  {
    tft.setTextColor(theme.text); // Color normal (Blanco/Negro)
    tft.print("  ");
  }
  tft.print(gameName);

  // Opción CALENDARIO
  tft.setCursor(15, 60);
  if (_cursorMenu == 1)
  {
    tft.setTextColor(theme.accent); // Color resaltado (Cian/Azul)
    tft.print("> ");
  }
  else
  {
    tft.setTextColor(theme.text); // Color normal (Blanco/Negro)
    tft.print("  ");
  }
  tft.print(consoleConfig.isEnglish ? "CALENDARY" : "CALENDARIO");

  // Opción CALENDARIO
  tft.setCursor(15, 90);
  if (_cursorMenu == 2)
  {
    tft.setTextColor(theme.accent); // Color resaltado (Cian/Azul)
    tft.print("> ");
  }
  else
  {
    tft.setTextColor(theme.text); // Color normal (Blanco/Negro)
    tft.print("  ");
  }
  tft.print(consoleConfig.isEnglish ? "SETTINGS" : "AJUSTES");

  Serial.println("MenuScreen DRAW -> false");
  _needsRedraw = false;
}

void MenuScreen::exit() {}

bool MenuScreen::getNeedsRedraw() { return _needsRedraw; }