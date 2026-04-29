#include "SnakeGame.h"
#include <Arduino.h>

// Colores RGB565
#define COL_BG        0x0000   // negro
#define COL_HUD_BG    0x2104   // gris oscuro
#define COL_HEAD      0x07FF   // cian
#define COL_BODY      0x07E0   // verde
#define COL_FOOD      0xF800   // rojo
#define COL_TEXT      0xFFFF   // blanco

// Convierte coordenada de celda a píxel
static inline int16_t px(int8_t gx) { return (int16_t)gx * SG_CELL; }
static inline int16_t py(int8_t gy) { return SG_HUD_H + (int16_t)gy * SG_CELL; }

// =========================
// PÚBLICO
// =========================

void SnakeGame::init() {
  randomSeed(millis());
  reset();
  Serial.println("SnakeGame init");
}

void SnakeGame::exit() {
  Serial.println("SnakeGame exit");
}

void SnakeGame::update(const InputState& in) {
  if (phase == PHASE_DEAD) {
    if (in.pressedA() || in.pressedB() || in.pressedC() || in.pressedD()) {
      reset();
    }
    return;
  }

  // Cambio de dirección (no se puede invertir)
  if (in.pressedA() && dir != SDIR_DOWN)  nextDir = SDIR_UP;
  if (in.pressedB() && dir != SDIR_UP)    nextDir = SDIR_DOWN;
  if (in.pressedC() && dir != SDIR_RIGHT) nextDir = SDIR_LEFT;
  if (in.pressedD() && dir != SDIR_LEFT)  nextDir = SDIR_RIGHT;

  uint32_t now = millis();
  if (now - lastMoveMs < moveInterval) return;
  lastMoveMs = now;

  dir = nextDir;

  // Nueva posición de la cabeza
  Pt nh = body[headIdx];
  switch (dir) {
    case SDIR_UP:    nh.y--; break;
    case SDIR_DOWN:  nh.y++; break;
    case SDIR_LEFT:  nh.x--; break;
    case SDIR_RIGHT: nh.x++; break;
  }

  // Colisión con pared
  if (nh.x < 0 || nh.x >= SG_COLS || nh.y < 0 || nh.y >= SG_ROWS) {
    phase = PHASE_DEAD;
    fullRedraw = needsRedraw = true;
    return;
  }

  bool eating = (nh.x == food.x && nh.y == food.y);

  // Colisión consigo mismo:
  // - Si come: la cola NO se mueve → revisar todos los segmentos actuales
  // - Si no come: la cola SÍ se mueve → no revisar el segmento de la cola
  {
    uint16_t skip  = eating ? 0 : 1;
    uint16_t start = (tailIdx + skip) % SG_MAXLEN;
    uint16_t count = length - skip;
    uint16_t idx   = start;
    for (uint16_t i = 0; i < count; i++) {
      if (body[idx].x == nh.x && body[idx].y == nh.y) {
        phase = PHASE_DEAD;
        fullRedraw = needsRedraw = true;
        return;
      }
      idx = (idx + 1) % SG_MAXLEN;
    }
  }

  // Guardar cola anterior (por si hay que borrarla al pintar)
  prevTail = body[tailIdx];

  // Avanzar cabeza en el buffer circular
  headIdx = (headIdx + 1) % SG_MAXLEN;
  body[headIdx] = nh;

  if (eating) {
    tailMoved = false;
    length++;
    score++;
    moveInterval = speedMs();
    spawnFood();
  } else {
    tailMoved = true;
    tailIdx = (tailIdx + 1) % SG_MAXLEN;
  }

  needsRedraw = true;
}

void SnakeGame::render(Adafruit_ST7735& tft) {
  if (!needsRedraw) return;
  needsRedraw = false;

  // --- Game Over ---
  if (phase == PHASE_DEAD) {
    if (fullRedraw) {
      renderGameOver(tft);
      fullRedraw = false;
    }
    return;
  }

  // --- Repintado completo (inicio o reinicio) ---
  if (fullRedraw) {
    tft.fillRect(0, SG_HUD_H, 160, 128 - SG_HUD_H, COL_BG);
    renderHUD(tft);

    uint16_t idx = tailIdx;
    for (uint16_t i = 0; i < length; i++) {
      fillCell(tft, body[idx], (i == length - 1) ? COL_HEAD : COL_BODY);
      idx = (idx + 1) % SG_MAXLEN;
    }
    fillCell(tft, food, COL_FOOD);
    fullRedraw = false;
    return;
  }

  // --- Repintado incremental (caso normal) ---

  // 1. Borrar celda de la cola anterior (si la cola avanzó)
  if (tailMoved) {
    fillCell(tft, prevTail, COL_BG);
  }

  // 2. La cabeza anterior pasa a ser cuerpo
  if (length > 1) {
    uint16_t neckIdx = (headIdx - 1 + SG_MAXLEN) % SG_MAXLEN;
    fillCell(tft, body[neckIdx], COL_BODY);
  }

  // 3. Dibujar nueva cabeza
  fillCell(tft, body[headIdx], COL_HEAD);

  // 4. Dibujar comida (puede ser nueva posición si acaba de comer)
  fillCell(tft, food, COL_FOOD);

  // 5. Actualizar HUD
  renderHUD(tft);
}

// =========================
// PRIVADO
// =========================

void SnakeGame::reset() {
  length  = 3;
  tailIdx = 0;
  headIdx = 2;

  int8_t cx = SG_COLS / 2;
  int8_t cy = SG_ROWS / 2;
  body[0] = { (int8_t)(cx - 2), cy };
  body[1] = { (int8_t)(cx - 1), cy };
  body[2] = { cx,               cy };

  dir = nextDir = SDIR_RIGHT;
  score        = 0;
  phase        = PHASE_PLAYING;
  lastMoveMs   = millis();
  moveInterval = speedMs();
  tailMoved    = false;
  fullRedraw   = true;
  needsRedraw  = true;

  spawnFood();
}

void SnakeGame::spawnFood() {
  // Intentos aleatorios; si falla, barrido lineal
  for (int i = 0; i < 500; i++) {
    Pt c = { (int8_t)random(0, SG_COLS), (int8_t)random(0, SG_ROWS) };
    if (!isOccupied(c)) { food = c; return; }
  }
  for (int8_t y = 0; y < SG_ROWS; y++) {
    for (int8_t x = 0; x < SG_COLS; x++) {
      Pt c = { x, y };
      if (!isOccupied(c)) { food = c; return; }
    }
  }
}

bool SnakeGame::isOccupied(Pt p) const {
  uint16_t idx = tailIdx;
  for (uint16_t i = 0; i < length; i++) {
    if (body[idx].x == p.x && body[idx].y == p.y) return true;
    idx = (idx + 1) % SG_MAXLEN;
  }
  return false;
}

void SnakeGame::fillCell(Adafruit_ST7735& tft, Pt p, uint16_t color) const {
  tft.fillRect(px(p.x), py(p.y), SG_CELL, SG_CELL, color);
}

void SnakeGame::renderHUD(Adafruit_ST7735& tft) const {
  tft.fillRect(0, 0, 160, SG_HUD_H, COL_HUD_BG);
  tft.setTextSize(1);
  tft.setTextColor(COL_TEXT, COL_HUD_BG);
  tft.setCursor(4, 4);
  tft.print("SNAKE");
  tft.setCursor(80, 4);
  tft.print("Pts:");
  tft.print(score);
}

void SnakeGame::renderGameOver(Adafruit_ST7735& tft) const {
  tft.fillScreen(COL_BG);

  tft.setTextSize(2);
  tft.setTextColor(COL_FOOD);
  tft.setCursor(22, 20);
  tft.print("GAME OVER");

  tft.setTextSize(1);
  tft.setTextColor(COL_TEXT);
  tft.setCursor(42, 55);
  tft.print("Puntos: ");
  tft.print(score);

  tft.setCursor(10, 75);
  tft.print("Pulsa para reiniciar");
}

uint32_t SnakeGame::speedMs() const {
  // 250ms inicial, -10ms cada 3 puntos, mínimo 80ms
  uint32_t r = (uint32_t)(score / 3) * 10;
  if (r > 170) r = 170;
  return 250 - r;
}