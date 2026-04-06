#include "SettingsScreen.h"

// "Diccionario" sencillo para los textos
const char* txt_title[] = {"AJUSTES", "SETTINGS"};
const char* txt_lang[]  = {"IDIOMA: ", "LANG: "};
const char* txt_theme[] = {"TEMA: ", "THEME: "};
const char* txt_sound[] = {"SONIDO: ", "SOUND: "};

SettingsScreen::SettingsScreen(Button& up, Button& down, Button& l, Button& r, Button& a, Button& b) 
    : _btnUp(up), _btnDown(down), _btnLeft(l), _btnRight(r), _btnA(a), _btnB(b) {}

void SettingsScreen::enter() { _needsRedraw = true; }

ConsoleState SettingsScreen::update() {
    if (_btnB.isPressed()) return mainScreen;

    // 1. Navegación Vertical (Seleccionar opción)
    if (_btnDown.isPressed()) { 
        _selectedOption = (_selectedOption + 1) % 3; 
        _needsRedraw = true; 
    }
    if (_btnUp.isPressed()) { 
        _selectedOption = (_selectedOption - 1 + 3) % 3; 
        _needsRedraw = true; 
    }

    // 2. Navegación Horizontal (Cambiar valores en consoleConfig)
    bool changed = false;
    if (_btnLeft.isPressed() || _btnRight.isPressed() || _btnA.isPressed()) {
        if (_selectedOption == 0) consoleConfig.isEnglish = !consoleConfig.isEnglish;
        if (_selectedOption == 1) consoleConfig.isDarkMode = !consoleConfig.isDarkMode;
        if (_selectedOption == 2) consoleConfig.soundEnable = !consoleConfig.soundEnable;
        changed = true;
    }

    if (changed) _needsRedraw = true;
    return settings;
}

void SettingsScreen::draw(Adafruit_ST7735& tft) {
    if (!_needsRedraw) return;

    Theme theme = consoleConfig.getTheme();
    int langIdx = consoleConfig.isEnglish ? 1 : 0;

    tft.fillScreen(theme.background);
    tft.setTextColor(theme.text);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print(txt_title[langIdx]);

    tft.setTextSize(1);
    
    // Dibujamos las 3 opciones. Si está seleccionada, usamos el color de acento.
    // Opción 0: Idioma
    tft.setCursor(20, 50);
    tft.setTextColor(_selectedOption == 0 ? theme.accent : theme.text);
    tft.print(_selectedOption == 0 ? "> " : "  ");
    tft.print(txt_lang[langIdx]);
    tft.print(consoleConfig.isEnglish ? "ENG" : "ESP");

    // Opción 1: Tema
    tft.setCursor(20, 70);
    tft.setTextColor(_selectedOption == 1 ? theme.accent : theme.text);
    tft.print(_selectedOption == 1 ? "> " : "  ");
    tft.print(txt_theme[langIdx]);
    tft.print(consoleConfig.isDarkMode ? "DARK" : "LIGHT");

    // Opción 2: Sonido
    tft.setCursor(20, 90);
    tft.setTextColor(_selectedOption == 2 ? theme.accent : theme.text);
    tft.print(_selectedOption == 2 ? "> " : "  ");
    tft.print(txt_sound[langIdx]);
    tft.print(consoleConfig.soundEnable ? "ON" : "OFF");

    _needsRedraw = false;
}

bool SettingsScreen::getNeedsRedraw() { return _needsRedraw; }
void SettingsScreen::exit() {}