#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <Arduino.h>
#include <Adafruit_ST7735.h>

class IGame;

enum ConsoleState {
  STATE_OFF,
  STATE_MENU,
  STATE_CALENDAR,
  STATE_SETTINGS,
  STATE_WAITING_CART,
  STATE_LOADING_GAME,
  STATE_GAME_RUNNING
};

struct InputState {
  bool currentUp = false;
  bool previousUp = false;

  bool currentDown = false;
  bool previousDown = false;

  bool currentLeft = false;
  bool previousLeft = false;

  bool currentRight = false;
  bool previousRight = false;

  bool currentA = false;
  bool previousA = false;

  bool currentB = false;
  bool previousB = false;

  bool pressedUp() const { return currentUp && !previousUp; }
  bool pressedDown() const { return currentDown && !previousDown; }
  bool pressedLeft() const { return currentLeft && !previousLeft; }
  bool pressedRight() const { return currentRight && !previousRight; }
  bool pressedA() const { return currentA && !previousA; }
  bool pressedB() const { return currentB && !previousB; }
};


struct GameCard {
  String uid;
  String name;
  IGame* game;
};

struct Theme {
  uint16_t background;
  uint16_t text;
  uint16_t accent;
};

struct GlobalConfig {
  bool isEnglish = false;
  bool isDarkMode = true;
  bool soundEnable = true;

  // Función para obtener los colores según el modo
  Theme getTheme() {
    if (isDarkMode) {
      return {ST77XX_BLACK, ST77XX_WHITE, ST77XX_CYAN};
    } else {
      return {ST77XX_WHITE, ST77XX_BLACK, ST77XX_BLUE};
    }
  }

};

extern GlobalConfig consoleConfig;
extern GameCard gameList[];
extern const int numGames;

#endif

/*
El compilador procesado cada .cpp de forma aislada (lo que suprime al #ifndef)
por lo que en cada .cpp que tenga un include Definitions.h se carga un
gameList[], lo cual es poco optimo ya que son bloques duplicados, y sobretodo,
tendriamos en memoria dos o mas variables con el mismo nombre, por lo que cuando
el Linker trate de unir los archivos para crear el programa final saltara un
error de "Multiple Definition"
(#ifndef solo funciona dentro del mismo .cpp). Para evitar esto usamos extern:

- extern:
Los atributos marcados con extern no son definidos cada vez que se lea el .h por
cada .cpp, solamente son declarados (se crean pero no se instancian). Esto
permite que podamos extraer y unificar la definicion de la variable en otro
fichero (en este caso main.cpp), dandonos un mayor control sobre donde queremos
nosotros como programadores definir cada variable (y evitando asi el problema
inicial de variables duplicadas).

Declaracion vs Definicion:
- Declaracion (extern en el .h): Se crea gameList, pero no se le asigna memoria
todavia.
- Definicion (en el .cpp): Se instancia el contenido de gameList.
*/