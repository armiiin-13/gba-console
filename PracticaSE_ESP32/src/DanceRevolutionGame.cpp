#include "DanceRevolutionGame.h"

static const int HIT_LINE_Y = 122;
static const int NOTE_START_Y = 28;
static const int TRAVEL_TIME = 2600;
static const int HIT_WINDOW = 260;

static const int melody[] = {
  659, 784, 880, 784,
  659, 587, 659, 784,
  988, 880, 784, 659,
  587, 659, 784, 659
};

static const int melodyLength = sizeof(melody) / sizeof(melody[0]);

void DanceRevolutionGame::init() {
  score = 0;
  combo = 0;
  songIndex = 0;

  playing = true;

  fullRedraw = true;
  hudDirty = true;
  feedbackDirty = true;
  notesDirty = true;
  endDirty = false;

  startTime = millis();
  lastMusicNoteTime = 0;

  lastFeedback = 0;
  lastScore = -1;
  lastCombo = -1;
  lastFeedbackDrawn = 999;

  for (int i = 0; i < MAX_NOTES; i++) {
    prevNoteY[i] = -999;
    prevNoteVisible[i] = false;
  }

  generateChart();
}

void DanceRevolutionGame::update(const InputState& in) {
  if (!playing) {
    if (in.pressedA()) {
      init();
    }
    return;
  }

  unsigned long songTime = millis() - startTime;

  if (in.pressedUp()) checkInput(NOTE_UP);
  if (in.pressedDown()) checkInput(NOTE_DOWN);
  if (in.pressedLeft()) checkInput(NOTE_LEFT);
  if (in.pressedRight()) checkInput(NOTE_RIGHT);
  if (in.pressedA()) checkInput(NOTE_A);
  if (in.pressedB()) checkInput(NOTE_B);

  updateMisses(songTime);

  if (songTime > notes[noteCount - 1].time + 2500) {
    playing = false;
    endDirty = true;
  }

  notesDirty = true;
}

void DanceRevolutionGame::render(Adafruit_ST7735& tft, SoundManager& sound) {
  unsigned long songTime = millis() - startTime;

  if (playing) {
    updateMusic(sound, songTime);
  } else {
    sound.stopTone();
  }

  if (fullRedraw) {
    drawStaticScreen(tft);
    fullRedraw = false;
    hudDirty = true;
    feedbackDirty = true;
    notesDirty = true;
  }

  if (hudDirty || score != lastScore || combo != lastCombo) {
    drawHUD(tft);
    lastScore = score;
    lastCombo = combo;
    hudDirty = false;
  }

  if (notesDirty && playing) {
    drawMovingNotes(tft, songTime);
    notesDirty = false;
  }

  if (feedbackDirty || lastFeedback != lastFeedbackDrawn) {
    drawFeedback(tft);
    lastFeedbackDrawn = lastFeedback;
    feedbackDirty = false;
  }

  if (endDirty) {
    drawEndScreen(tft);
    endDirty = false;
  }
}

void DanceRevolutionGame::exit() {
}

void DanceRevolutionGame::generateChart() {
  noteCount = 32;

  NoteType pattern[] = {
    NOTE_LEFT, NOTE_UP, NOTE_RIGHT, NOTE_DOWN,
    NOTE_A, NOTE_B, NOTE_UP, NOTE_RIGHT,
    NOTE_LEFT, NOTE_DOWN, NOTE_A, NOTE_UP,
    NOTE_RIGHT, NOTE_B, NOTE_DOWN, NOTE_LEFT
  };

  for (int i = 0; i < noteCount; i++) {
    notes[i].type = pattern[i % 16];
    notes[i].time = 1800 + i * 850;
    notes[i].hit = false;
    notes[i].missed = false;
  }
}

void DanceRevolutionGame::checkInput(NoteType input) {
  unsigned long songTime = millis() - startTime;

  int bestIndex = -1;
  unsigned long bestDiff = 99999;

  for (int i = 0; i < noteCount; i++) {
    if (notes[i].hit || notes[i].missed) continue;
    if (notes[i].type != input) continue;

    unsigned long diff = abs((long)songTime - (long)notes[i].time);

    if (diff < bestDiff) {
      bestDiff = diff;
      bestIndex = i;
    }
  }

  if (bestIndex != -1 && bestDiff <= HIT_WINDOW) {
    notes[bestIndex].hit = true;
    combo++;

    if (bestDiff < 70) {
      score += 300;
      lastFeedback = 3;
    } else if (bestDiff < 150) {
      score += 150;
      lastFeedback = 2;
    } else {
      score += 80;
      lastFeedback = 1;
    }

    hudDirty = true;
    feedbackDirty = true;
    notesDirty = true;
  } else {
    combo = 0;
    lastFeedback = -1;

    hudDirty = true;
    feedbackDirty = true;
  }
}

void DanceRevolutionGame::updateMisses(unsigned long songTime) {
  for (int i = 0; i < noteCount; i++) {
    if (!notes[i].hit && !notes[i].missed && songTime > notes[i].time + HIT_WINDOW) {
      notes[i].missed = true;
      combo = 0;
      lastFeedback = -1;

      hudDirty = true;
      feedbackDirty = true;
      notesDirty = true;
    }
  }
}

void DanceRevolutionGame::updateMusic(SoundManager& sound, unsigned long songTime) {
  const unsigned long beatDuration = 420;

  if (songTime - lastMusicNoteTime > beatDuration) {
    int note = melody[songIndex % melodyLength];

    sound.playToneAsync(note);

    songIndex++;
    lastMusicNoteTime = songTime;
  }

  if (songTime - lastMusicNoteTime > 260) {
    sound.stopTone();
  }
}

int DanceRevolutionGame::laneX(NoteType type) {
  switch (type) {
    case NOTE_LEFT: return 8;
    case NOTE_DOWN: return 28;
    case NOTE_UP: return 48;
    case NOTE_RIGHT: return 68;
    case NOTE_A: return 91;
    case NOTE_B: return 111;
  }

  return 0;
}

void DanceRevolutionGame::drawStaticScreen(Adafruit_ST7735& tft) {
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);

  tft.setCursor(4, 4);
  tft.print("DANCE BIT");

  drawLane(tft, laneX(NOTE_LEFT), "<");
  drawLane(tft, laneX(NOTE_DOWN), "v");
  drawLane(tft, laneX(NOTE_UP), "^");
  drawLane(tft, laneX(NOTE_RIGHT), ">");
  drawLane(tft, laneX(NOTE_A), "A");
  drawLane(tft, laneX(NOTE_B), "B");

  tft.drawFastHLine(0, HIT_LINE_Y, 128, ST77XX_WHITE);
}

void DanceRevolutionGame::drawHUD(Adafruit_ST7735& tft) {
  tft.fillRect(74, 4, 54, 10, ST77XX_BLACK);
  tft.fillRect(0, 14, 128, 10, ST77XX_BLACK);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);

  tft.setCursor(76, 4);
  tft.print(score);

  tft.setCursor(4, 15);
  tft.print("Combo:");
  tft.print(combo);
}

void DanceRevolutionGame::drawMovingNotes(Adafruit_ST7735& tft, unsigned long songTime) {
  for (int i = 0; i < noteCount; i++) {
    if (prevNoteVisible[i]) {
      int oldX = laneX(notes[i].type);
      tft.fillRect(oldX + 1, prevNoteY[i] - 1, 14, 10, ST77XX_BLACK);
      prevNoteVisible[i] = false;
    }
  }

  for (int i = 0; i < noteCount; i++) {
    if (notes[i].hit || notes[i].missed) continue;

    long timeUntilHit = (long)notes[i].time - (long)songTime;
    int y = HIT_LINE_Y - ((long)timeUntilHit * (HIT_LINE_Y - NOTE_START_Y) / TRAVEL_TIME);

    if (y >= NOTE_START_Y && y <= 150) {
      drawNote(tft, notes[i].type, y);

      prevNoteY[i] = y;
      prevNoteVisible[i] = true;
    }
  }

  tft.drawFastHLine(0, HIT_LINE_Y, 128, ST77XX_WHITE);
}

void DanceRevolutionGame::drawFeedback(Adafruit_ST7735& tft) {
  tft.fillRect(0, 134, 128, 12, ST77XX_BLACK);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);

  if (lastFeedback == 3) {
    tft.setCursor(42, 135);
    tft.print("PERFECT");
  } else if (lastFeedback == 2) {
    tft.setCursor(48, 135);
    tft.print("GOOD");
  } else if (lastFeedback == 1) {
    tft.setCursor(50, 135);
    tft.print("OK");
  } else if (lastFeedback == -1) {
    tft.setCursor(48, 135);
    tft.print("MISS");
  }
}

void DanceRevolutionGame::drawEndScreen(Adafruit_ST7735& tft) {
  tft.fillRect(10, 50, 108, 54, ST77XX_BLACK);
  tft.drawRect(10, 50, 108, 54, ST77XX_WHITE);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);

  tft.setCursor(34, 62);
  tft.print("FIN DEL TEMA");

  tft.setCursor(28, 78);
  tft.print("Score: ");
  tft.print(score);

  tft.setCursor(24, 92);
  tft.print("A: reiniciar");
}

void DanceRevolutionGame::drawLane(Adafruit_ST7735& tft, int x, const char* label) {
  tft.drawRect(x, 28, 16, 100, ST77XX_ORANGE);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setCursor(x + 5, 132);
  tft.print(label);
}

void DanceRevolutionGame::drawNote(Adafruit_ST7735& tft, NoteType type, int y) {
  int x = laneX(type);

  uint16_t color = ST77XX_WHITE;

  switch (type) {
    case NOTE_LEFT: color = ST77XX_YELLOW; break;
    case NOTE_DOWN: color = ST77XX_GREEN; break;
    case NOTE_UP: color = ST77XX_BLUE; break;
    case NOTE_RIGHT: color = ST77XX_RED; break;
    case NOTE_A: color = ST77XX_MAGENTA; break;
    case NOTE_B: color = ST77XX_CYAN; break;
  }

  tft.fillRect(x + 2, y, 12, 8, color);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_BLACK, color);
  tft.setCursor(x + 5, y + 1);

  switch (type) {
    case NOTE_LEFT: tft.print("<"); break;
    case NOTE_DOWN: tft.print("v"); break;
    case NOTE_UP: tft.print("^"); break;
    case NOTE_RIGHT: tft.print(">"); break;
    case NOTE_A: tft.print("A"); break;
    case NOTE_B: tft.print("B"); break;
  }
}