#ifndef POKEMON_BATTLE_GAME_H
#define POKEMON_BATTLE_GAME_H

#include <Arduino.h>
#include <Adafruit_ST7735.h>
#include <IGame.h>

struct Attack {
  const char* name;
  int power;
};

struct Pokemon {
  const char* name;
  int hp;
  int maxHp;
  Attack attacks[4];
};

class PokemonBattleGame : public IGame {
public:
  void init() override;
  void update(const InputState& in) override;
  void render(Adafruit_ST7735& tft, SoundManager& sound) override;
  void exit() override;

private:
  enum BattleState {
    PLAYER_CHOOSE,
    MESSAGE,
    ENEMY_TURN,
    VICTORY,
    DEFEAT
  };

  Pokemon player;
  Pokemon enemy;

  BattleState state;

  int selectedAttack;
  const char* message;
  unsigned long messageStart;
  bool needsRedraw;

  void playerAttack(int index);
  void enemyAttack();
  void changeSelection(int dx, int dy);
  void drawBattle(Adafruit_ST7735& tft);
  void drawHpBar(Adafruit_ST7735& tft, int x, int y, int hp, int maxHp);
};

#endif