#include "TetrisGame.h"
#include <Arduino.h>

// ─── Piezas ────────────────────────────────────────────────────────
// Rejilla 4×4: bit (15 - fila*4 - col) ↔ celda (fila, col)
// 7 piezas × 4 rotaciones CW
static const uint16_t PIECES[7][4] = {
  { 0x0F00, 0x2222, 0x00F0, 0x4444 }, // I
  { 0x6600, 0x6600, 0x6600, 0x6600 }, // O
  { 0x4E00, 0x8C80, 0x0E40, 0x4C40 }, // T
  { 0x6C00, 0x8C40, 0x6C00, 0x8C40 }, // S
  { 0xC600, 0x4C80, 0xC600, 0x4C80 }, // Z
  { 0x8E00, 0xC880, 0xE200, 0x44C0 }, // J
  { 0x2E00, 0x88C0, 0xE800, 0xC440 }, // L
};

// RGB565: índice 0=vacío, 1-7=pieza
static const uint16_t COLORS[8] = {
  0x0000, // vacío
  0x07FF, // I  cian
  0xFFE0, // O  amarillo
  0xF81F, // T  magenta
  0x07E0, // S  verde
  0xF800, // Z  rojo
  0x001F, // J  azul
  0xFC00, // L  naranja
};

static const uint32_t FALL_TABLE[11] = {
  800, 720, 630, 550, 470, 380, 300, 220, 150, 100, 80
};

static inline bool pieceBit(uint16_t mask, int8_t r, int8_t c) {
  return (mask >> (15 - r * 4 - c)) & 1;
}

// ─── IGame ─────────────────────────────────────────────────────────

void TetrisGame::init() {
  randomSeed(millis());
  reset();
  Serial.println("TetrisGame init");
}

void TetrisGame::exit() {
  Serial.println("TetrisGame exit");
}

// ─── Lógica ────────────────────────────────────────────────────────

void TetrisGame::reset() {
  for (int r = 0; r < TG_ROWS; r++)
    for (int c = 0; c < TG_COLS; c++)
      board[r][c] = 0;

  score = 0; lines = 0; level = 0;
  gameOver    = false;
  fullRedraw  = true;
  hudDirty    = false;
  needsRedraw = true;
  pieceChanged = false;
  dasLeft = dasRight = false;
  dasTimer = dasRepeatTimer = 0;
  lastFallMs = millis();
  nxtType = random(7);
  spawnPiece();
}

void TetrisGame::spawnPiece() {
  curType = nxtType;
  curRot  = 0;
  curX    = (TG_COLS / 2) - 2;  // columna 3
  curY    = -1;                  // empieza una fila sobre el tablero
  nxtType = random(7);

  if (!fits(curType, curRot, curX, curY)) {
    if (!fits(curType, curRot, curX, 0)) {
      gameOver = true;
      fullRedraw = true;
      return;
    }
    curY = 0;
  }
  prevType = curType; prevRot = curRot; prevX = curX; prevY = curY;
  pieceChanged = false;
}

bool TetrisGame::fits(int8_t type, int8_t rot, int8_t bx, int8_t by) const {
  uint16_t mask = PIECES[type][rot];
  for (int8_t r = 0; r < 4; r++) {
    for (int8_t c = 0; c < 4; c++) {
      if (!pieceBit(mask, r, c)) continue;
      int8_t row = by + r;
      int8_t col = bx + c;
      if (col < 0 || col >= TG_COLS)  return false;
      if (row >= TG_ROWS)             return false;
      if (row >= 0 && board[row][col]) return false;
      // row < 0: por encima del tablero, válido
    }
  }
  return true;
}

void TetrisGame::doLock() {
  uint16_t mask = PIECES[curType][curRot];
  for (int8_t r = 0; r < 4; r++)
    for (int8_t c = 0; c < 4; c++)
      if (pieceBit(mask, r, c)) {
        int8_t row = curY + r, col = curX + c;
        if (row >= 0 && row < TG_ROWS && col >= 0 && col < TG_COLS)
          board[row][col] = (uint8_t)(curType + 1);
      }
}

int TetrisGame::countAndClear() {
  int count = 0;
  for (int8_t r = TG_ROWS - 1; r >= 0; r--) {
    bool full = true;
    for (int8_t c = 0; c < TG_COLS; c++)
      if (!board[r][c]) { full = false; break; }
    if (full) {
      count++;
      for (int8_t rr = r; rr > 0; rr--)
        for (int8_t c = 0; c < TG_COLS; c++)
          board[rr][c] = board[rr - 1][c];
      for (int8_t c = 0; c < TG_COLS; c++) board[0][c] = 0;
      r++; // recomprobar misma fila (nuevo contenido bajó)
    }
  }
  return count;
}

void TetrisGame::addScore(int cleared) {
  if (cleared == 0) return;
  static const uint16_t M[] = {0, 100, 300, 500, 800};
  score += (uint32_t)M[cleared] * (level + 1);
  lines += (uint16_t)cleared;
  uint8_t newLvl = (uint8_t)(lines / 10);
  if (newLvl > 10) newLvl = 10;
  level = newLvl;
  hudDirty = true;
}

uint32_t TetrisGame::fallMs() const {
  return FALL_TABLE[level < 11 ? level : 10];
}

// ─── Update ────────────────────────────────────────────────────────
// Botones:  A=Rotar  B=Bajada rápida  C=Izquierda  D=Derecha

void TetrisGame::update(const InputState& in) {
  if (gameOver) {
    if (in.pressedUp() || in.pressedDown() || in.pressedLeft() || in.pressedRight()) reset();
    return;
  }

  uint32_t now = millis();

  // ── Rotación (con wall-kicks básicos) ──
  if (in.pressedUp()) {
    int8_t nr = (curRot + 1) % 4;
    static const int8_t KICKS[] = {0, -1, 1, -2, 2};
    for (int k = 0; k < 5; k++) {
      if (fits(curType, nr, curX + KICKS[k], curY)) {
        curX += KICKS[k];
        curRot = nr;
        pieceChanged = needsRedraw = true;
        break;
      }
    }
  }

  // ── Movimiento lateral con DAS ──
  if (in.pressedLeft()) {
    if (fits(curType, curRot, curX - 1, curY)) { curX--; pieceChanged = needsRedraw = true; }
    dasLeft = true; dasRight = false;
    dasTimer = dasRepeatTimer = now;
  }
  if (in.pressedRight()) {
    if (fits(curType, curRot, curX + 1, curY)) { curX++; pieceChanged = needsRedraw = true; }
    dasRight = true; dasLeft = false;
    dasTimer = dasRepeatTimer = now;
  }
  if (!in.currentLeft) dasLeft  = false;
  if (!in.currentRight) dasRight = false;

  // Auto-repetición tras 150ms, cada 50ms
  if (dasLeft  && now - dasTimer >= 150 && now - dasRepeatTimer >= 50) {
    dasRepeatTimer = now;
    if (fits(curType, curRot, curX - 1, curY)) { curX--; pieceChanged = needsRedraw = true; }
  }
  if (dasRight && now - dasTimer >= 150 && now - dasRepeatTimer >= 50) {
    dasRepeatTimer = now;
    if (fits(curType, curRot, curX + 1, curY)) { curX++; pieceChanged = needsRedraw = true; }
  }

  // ── Gravedad (B = bajada rápida) ──
  uint32_t eff = in.currentDown ? 50 : fallMs();
  if (now - lastFallMs >= eff) {
    lastFallMs = now;
    if (fits(curType, curRot, curX, curY + 1)) {
      curY++;
      pieceChanged = needsRedraw = true;
    } else {
      // Bloquear pieza
      doLock();
      int cleared = countAndClear();
      addScore(cleared);
      spawnPiece();
      fullRedraw = needsRedraw = true;
    }
  }
}

// ─── Helpers de render ─────────────────────────────────────────────

uint16_t TetrisGame::pieceColor(int8_t idx) const {
  if (idx < 0 || idx > 7) return 0x0000;
  return COLORS[idx];
}

void TetrisGame::drawCell(Adafruit_ST7735& tft, int8_t bc, int8_t br, uint16_t color) const {
  // TG_CELL-1 deja un hueco de 1px entre celdas
  tft.fillRect(TG_BX + bc * TG_CELL, TG_BY + br * TG_CELL,
               TG_CELL - 1, TG_CELL - 1, color);
}

void TetrisGame::paintPiece(Adafruit_ST7735& tft,
                             int8_t type, int8_t rot, int8_t bx, int8_t by,
                             uint16_t color) const {
  uint16_t mask = PIECES[type][rot];
  for (int8_t r = 0; r < 4; r++)
    for (int8_t c = 0; c < 4; c++) {
      if (!pieceBit(mask, r, c)) continue;
      int8_t row = by + r, col = bx + c;
      if (row < 0 || row >= TG_ROWS || col < 0 || col >= TG_COLS) continue;
      drawCell(tft, col, row, color);
    }
}

void TetrisGame::drawBoard(Adafruit_ST7735& tft) const {
  for (int8_t r = 0; r < TG_ROWS; r++)
    for (int8_t c = 0; c < TG_COLS; c++)
      drawCell(tft, c, r, pieceColor(board[r][c]));
}

void TetrisGame::drawHUD(Adafruit_ST7735& tft) const {
  const uint16_t bg  = 0x0000;
  const uint16_t fg  = 0xFFFF;
  const uint16_t acc = 0xFFE0; // amarillo
  const uint16_t dim = 0x7BEF; // gris

  tft.fillRect(TG_HX, 0, 160 - TG_HX, 128, bg);

  tft.setTextSize(1);
  tft.setTextColor(acc, bg);
  tft.setCursor(TG_HX + 4, 4);
  tft.print("TETRIS");

  tft.drawFastHLine(TG_HX, 14, 160 - TG_HX, dim);

  tft.setTextColor(dim, bg);
  tft.setCursor(TG_HX + 4, 18);
  tft.print("SCORE");
  tft.setTextColor(fg, bg);
  tft.setCursor(TG_HX + 4, 28);
  tft.print(score);

  tft.setTextColor(dim, bg);
  tft.setCursor(TG_HX + 4, 46);
  tft.print("LEVEL");
  tft.setTextColor(fg, bg);
  tft.setCursor(TG_HX + 4, 56);
  tft.print(level);

  tft.setTextColor(dim, bg);
  tft.setCursor(TG_HX + 4, 70);
  tft.print("LINES");
  tft.setTextColor(fg, bg);
  tft.setCursor(TG_HX + 4, 80);
  tft.print(lines);

  tft.drawFastHLine(TG_HX, 91, 160 - TG_HX, dim);

  tft.setTextColor(dim, bg);
  tft.setCursor(TG_HX + 4, 95);
  tft.print("NEXT");

  // Preview 4×4 con celdas de 5px
  tft.fillRect(TG_HX + 4, 105, 20, 20, bg);
  uint16_t mask = PIECES[nxtType][0];
  uint16_t col  = pieceColor(nxtType + 1);
  for (int8_t r = 0; r < 4; r++)
    for (int8_t c = 0; c < 4; c++)
      if (pieceBit(mask, r, c))
        tft.fillRect(TG_HX + 4 + c * 5, 105 + r * 5, 4, 4, col);
}

void TetrisGame::drawGameOver(Adafruit_ST7735& tft) const {
  tft.fillScreen(0x0000);
  tft.setTextSize(2);
  tft.setTextColor(0xF800);
  tft.setCursor(20, 15);
  tft.print("GAME");
  tft.setCursor(20, 35);
  tft.print("OVER");

  tft.setTextSize(1);
  tft.setTextColor(0xFFFF);
  tft.setCursor(10, 65);
  tft.print("Puntuacion:");
  tft.setCursor(10, 77);
  tft.print(score);
  tft.setCursor(10, 100);
  tft.print("Pulsa para");
  tft.setCursor(10, 112);
  tft.print("reiniciar");
}

// ─── Render ────────────────────────────────────────────────────────

void TetrisGame::render(Adafruit_ST7735& tft, SoundManager& sound) {
  if (!needsRedraw && !fullRedraw && !hudDirty) return;

  // Pantalla de game over
  if (gameOver) {
    if (fullRedraw) {
      drawGameOver(tft);
      fullRedraw = needsRedraw = false;
    }
    return;
  }

  // Repintado completo: inicio o tras bloquear pieza
  if (fullRedraw) {
    tft.fillRect(0, 0, TG_HX, 128, 0x0000);
    tft.drawRect(TG_BX - 1, TG_BY - 1,
                 TG_COLS * TG_CELL + 2, TG_ROWS * TG_CELL + 2, 0x8410);
    drawBoard(tft);
    paintPiece(tft, curType, curRot, curX, curY, pieceColor(curType + 1));
    drawHUD(tft);
    prevType = curType; prevRot = curRot; prevX = curX; prevY = curY;
    fullRedraw = hudDirty = needsRedraw = false;
    return;
  }

  // Repintado incremental: solo la pieza que se movió/rotó
  if (needsRedraw && pieceChanged) {
    paintPiece(tft, prevType, prevRot, prevX, prevY, 0x0000); // borrar
    paintPiece(tft, curType,  curRot,  curX,  curY,  pieceColor(curType + 1));
    prevType = curType; prevRot = curRot; prevX = curX; prevY = curY;
    pieceChanged = needsRedraw = false;
  }

  if (hudDirty) { drawHUD(tft); hudDirty = false; }
}