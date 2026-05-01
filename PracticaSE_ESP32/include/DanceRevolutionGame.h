#ifndef DANCE_GAME_H
#define DANCE_GAME_H

#include <Arduino.h>
#include <Adafruit_ST7735.h>
#include <IGame.h>

#define MAX_NOTES 64

class DanceRevolutionGame : public IGame {
public:
  void init() override;
  void update(const InputState& in) override;
  void render(Adafruit_ST7735& tft, SoundManager& sound) override;
  void exit() override;

private:
  enum NoteType {
    NOTE_UP,
    NOTE_DOWN,
    NOTE_LEFT,
    NOTE_RIGHT,
    NOTE_A,
    NOTE_B
  };

  struct DanceNote {
    NoteType type;
    unsigned long time;
    bool hit;
    bool missed;
  };

  DanceNote notes[MAX_NOTES];

  int noteCount;
  int score;
  int combo;
  int songIndex;

  bool playing;

  bool fullRedraw;
  bool hudDirty;
  bool feedbackDirty;
  bool notesDirty;
  bool endDirty;

  unsigned long startTime;
  unsigned long lastMusicNoteTime;

  int lastFeedback;
  int lastScore;
  int lastCombo;
  int lastFeedbackDrawn;

  int prevNoteY[MAX_NOTES];
  bool prevNoteVisible[MAX_NOTES];

  void generateChart();
  void checkInput(NoteType input);
  void updateMisses(unsigned long songTime);
  void updateMusic(SoundManager& sound, unsigned long songTime);

  void drawStaticScreen(Adafruit_ST7735& tft);
  void drawHUD(Adafruit_ST7735& tft);
  void drawMovingNotes(Adafruit_ST7735& tft, unsigned long songTime);
  void drawFeedback(Adafruit_ST7735& tft);
  void drawEndScreen(Adafruit_ST7735& tft);

  void drawLane(Adafruit_ST7735& tft, int x, const char* label);
  void drawNote(Adafruit_ST7735& tft, NoteType type, int y);
  int laneX(NoteType type);
};

#endif