#ifndef SCREEN_H
#define SCREEN_H

#include "Definitions.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

class Screen {
 public:
  virtual ~Screen() {}

  virtual void enter() = 0;

  virtual ConsoleState update() = 0;

  virtual void draw(Adafruit_ST7735& tft) = 0;

  virtual void exit() = 0;

  virtual bool getNeedsRedraw() = 0;

};

#endif

/*
"= 0":
- Sirve para que de error si no se implementa el metodo en cada clase que herede
de Screen (lo cual es necesario ya que es una clase abstracta)
- La forma oficial y estándar de crear una clase abstracta en C++ es
precisamente declarar al menos una función como virtual pura (= 0)

virtual:
- Obliga al compilador a "comprobar" el objeto sucesor de Screen, ya que hace
que los metodos estáticos (se ejecutan en compilacion) a tener "Despacho
Dinámico" (se ejecuta en ejecucion si hay un objeto creado de ese metodo)
- Ejecuta los metodos de la clase del objeto a la que apunta el puntero (la
clase hijo), en vez de ejecutar los del propio puntero (la clase padre, que es
abstracta y sus metodos no estan implementados)
- Siendo mas especifico, se crea una VTable (Tabla Virtual) donde se guarda cada
virtual. Cuando llamas a currentScreen->draw(), el programa mira en ese índice
para ver a qué función de la clase hija debe saltar

"~Screen":
- Define un Destructor en C++, lo que nos ayuda a eliminar el objeto en la
memoria que ya no necesitemos.
- El Destructor se ejecuta solo si se sale de la funcion donde se crea el
Destructor (Ej: Una variable local Screen en una funcion) o manualmente si se
crea mediante un new, aplicando delete al puntero, eliminado asi el objeto
Detalle: Al igual que "Screen(){}" define el constructor, "~Screen(){}" define
el destructor. Uno se ejecuta al nacer y otro al morir.

*/