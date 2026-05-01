#include "MemoryGame.h"

void MemoryGame::init() {
  randomSeed(millis());

  sequenceLength = 0;
  showIndex = 0;
  playerIndex = 0;

  activeColor = -1;
  lastActiveColor = -2;
  pendingSound = 0;
  lastSequenceLength = -1;

  fullRedraw = true;
  hudDirty = true;
  padDirty = true;
  messageDirty = true;

  addRandomColor();
  startGameTurn();
}

void MemoryGame::update(const InputState& in) {
  unsigned long now = millis();

  if (state == GAME_TURN_TEXT) {
    if (now - stateStart > 900) {
      state = GAME_SHOW_SEQUENCE;
      showIndex = 0;
      activeColor = -1;
      colorStart = now;
      messageDirty = true;
      padDirty = true;
    }
  }

  else if (state == GAME_SHOW_SEQUENCE) {
    if (activeColor == -1 && now - colorStart > 250) {
      activateColor(sequence[showIndex]);
    }

    else if (activeColor != -1 && now - colorStart > 420) {
      activeColor = -1;
      showIndex++;
      colorStart = now;
      padDirty = true;

      if (showIndex >= sequenceLength) {
        startPlayerTurn();
      }
    }
  }

  else if (state == PLAYER_TURN_TEXT) {
    if (now - stateStart > 800) {
      state = PLAYER_INPUT;
      activeColor = -1;
      messageDirty = true;
      padDirty = true;
    }
  }

  else if (state == PLAYER_INPUT) {
    if (in.pressedUp()) handlePlayerColor(COLOR_UP);
    else if (in.pressedDown()) handlePlayerColor(COLOR_DOWN);
    else if (in.pressedLeft()) handlePlayerColor(COLOR_LEFT);
    else if (in.pressedRight()) handlePlayerColor(COLOR_RIGHT);

    if (activeColor != -1 && now - colorStart > 220) {
      activeColor = -1;
      padDirty = true;
    }
  }

  else if (state == GAME_OVER) {
    if (in.pressedA()) {
      init();
    }
  }
}

void MemoryGame::render(Adafruit_ST7735& tft, SoundManager& sound) {
  if (fullRedraw) {
    drawStatic(tft);
    fullRedraw = false;
    hudDirty = true;
    messageDirty = true;
    padDirty = true;
  }

  if (hudDirty || lastSequenceLength != sequenceLength) {
    drawHud(tft);
    lastSequenceLength = sequenceLength;
    hudDirty = false;
  }

  if (padDirty || lastActiveColor != activeColor) {
    if (lastActiveColor >= 0) redrawSingleButton(tft, lastActiveColor);
    if (activeColor >= 0) redrawSingleButton(tft, activeColor);

    lastActiveColor = activeColor;
    padDirty = false;
  }

  if (messageDirty) {
    drawMessage(tft);
    messageDirty = false;
  }

  if (pendingSound > 0) {
    sound.playNote(pendingSound, 60);
    pendingSound = 0;
  }
}

void MemoryGame::exit() {
}

void MemoryGame::addRandomColor() {
  if (sequenceLength < MAX_SEQUENCE) {
    sequence[sequenceLength] = random(0, 4);
    sequenceLength++;
    hudDirty = true;
  }
}

void MemoryGame::startGameTurn() {
  state = GAME_TURN_TEXT;
  stateStart = millis();
  activeColor = -1;
  messageDirty = true;
  padDirty = true;
}

void MemoryGame::startPlayerTurn() {
  state = PLAYER_TURN_TEXT;
  stateStart = millis();
  playerIndex = 0;
  activeColor = -1;
  messageDirty = true;
  padDirty = true;
}

void MemoryGame::handlePlayerColor(int color) {
  activateColor(color);

  if (color != sequence[playerIndex]) {
    state = GAME_OVER;
    activeColor = -1;
    pendingSound = 120;
    messageDirty = true;
    padDirty = true;
    return;
  }

  playerIndex++;

  if (playerIndex >= sequenceLength) {
    addRandomColor();
    startGameTurn();
  }
}

void MemoryGame::activateColor(int color) {
  activeColor = color;
  pendingSound = getSoundForColor(color);
  colorStart = millis();
  padDirty = true;
}

int MemoryGame::getSoundForColor(int color) {
  switch (color) {
    case COLOR_UP: return 523;
    case COLOR_DOWN: return 659;
    case COLOR_LEFT: return 784;
    case COLOR_RIGHT: return 988;
  }

  return 440;
}

void MemoryGame::drawStatic(Adafruit_ST7735& tft) {
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);

  tft.setCursor(20, 6);
  tft.print("MEMORIA COLOR");

  drawColorPad(tft);
}

void MemoryGame::drawHud(Adafruit_ST7735& tft) {
  tft.fillRect(0, 20, 128, 12, ST77XX_BLACK);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);

  tft.setCursor(8, 22);
  tft.print("Ronda: ");
  tft.print(sequenceLength);
}

void MemoryGame::drawMessage(Adafruit_ST7735& tft) {
  tft.fillRect(0, 132, 128, 28, ST77XX_BLACK);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);

  if (state == GAME_TURN_TEXT || state == GAME_SHOW_SEQUENCE) {
    tft.setCursor(18, 138);
    tft.print("Turno consola");
  }

  else if (state == PLAYER_TURN_TEXT || state == PLAYER_INPUT) {
    tft.setCursor(24, 138);
    tft.print("Tu turno");
  }

  else if (state == GAME_OVER) {
    tft.setCursor(30, 132);
    tft.print("Fallaste!");
    tft.setCursor(18, 145);
    tft.print("A: reiniciar");
  }
}

void MemoryGame::drawColorPad(Adafruit_ST7735& tft) {
  drawButton(tft, COLOR_UP,    44, 40, ST77XX_BLUE);
  drawButton(tft, COLOR_DOWN,  44, 92, ST77XX_GREEN);
  drawButton(tft, COLOR_LEFT,  14, 66, ST77XX_YELLOW);
  drawButton(tft, COLOR_RIGHT, 74, 66, ST77XX_RED);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_BLACK);

  tft.setCursor(55, 54);
  tft.print("UP");

  tft.setCursor(48, 106);
  tft.print("DOWN");

  tft.setCursor(20, 80);
  tft.print("LEFT");

  tft.setCursor(79, 80);
  tft.print("RIGHT");
}

void MemoryGame::redrawSingleButton(Adafruit_ST7735& tft, int color) {
  switch (color) {
    case COLOR_UP:
      drawButton(tft, COLOR_UP, 44, 40, ST77XX_BLUE);
      tft.setTextColor(ST77XX_BLACK);
      tft.setCursor(55, 54);
      tft.print("UP");
      break;

    case COLOR_DOWN:
      drawButton(tft, COLOR_DOWN, 44, 92, ST77XX_GREEN);
      tft.setTextColor(ST77XX_BLACK);
      tft.setCursor(48, 106);
      tft.print("DOWN");
      break;

    case COLOR_LEFT:
      drawButton(tft, COLOR_LEFT, 14, 66, ST77XX_YELLOW);
      tft.setTextColor(ST77XX_BLACK);
      tft.setCursor(20, 80);
      tft.print("LEFT");
      break;

    case COLOR_RIGHT:
      drawButton(tft, COLOR_RIGHT, 74, 66, ST77XX_RED);
      tft.setTextColor(ST77XX_BLACK);
      tft.setCursor(79, 80);
      tft.print("RIGHT");
      break;
  }
}

void MemoryGame::drawButton(Adafruit_ST7735& tft, int color, int x, int y, uint16_t baseColor) {
  tft.fillRect(x - 4, y - 4, 44, 32, ST77XX_BLACK);

  if (activeColor == color) {
    tft.fillRoundRect(x - 3, y - 3, 42, 30, 5, ST77XX_WHITE);
    tft.fillRoundRect(x, y, 36, 24, 4, baseColor);
  } else {
    tft.fillRoundRect(x, y, 36, 24, 4, baseColor);
    tft.drawRoundRect(x, y, 36, 24, 4, ST77XX_WHITE);
  }
}