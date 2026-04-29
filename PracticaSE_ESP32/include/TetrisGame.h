#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H

#include <IGame.h>

class TetrisGame : public IGame {
    public:
        TetrisGame(){};
        void init() override;
        void update(const InputState& in) override;
        void render(Adafruit_ST7735& tft) override;
        void exit() override;
};

#endif