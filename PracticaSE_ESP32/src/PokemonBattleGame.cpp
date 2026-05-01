#include "PokemonBattleGame.h"

void PokemonBattleGame::init() {
  player = {
    "PIKACHU",
    100,
    100,
    {
      {"RAYO", 24},
      {"PLACAJE", 16},
      {"CHISPA", 20},
      {"AT. RAPIDO", 18}
    }
  };

  enemy = {
    "CHARMANDER",
    100,
    100,
    {
      {"ARANAZO", 14},
      {"ASCUAS", 22},
      {"GRUÑIDO", 8},
      {"MORDISCO", 18}
    }
  };

  state = PLAYER_CHOOSE;
  selectedAttack = 0;
  message = "Elige ataque";
  messageStart = 0;
  needsRedraw = true;
}

void PokemonBattleGame::update(const InputState& in) {
  if (state == PLAYER_CHOOSE) {
    if (in.pressedLeft())  changeSelection(-1, 0);
    if (in.pressedRight()) changeSelection(1, 0);
    if (in.pressedUp())    changeSelection(0, -1);
    if (in.pressedDown())  changeSelection(0, 1);

    if (in.pressedA()) {
      playerAttack(selectedAttack);
    }
  }

  else if (state == MESSAGE) {
    if (in.pressedA() || millis() - messageStart > 900) {
      if (enemy.hp <= 0) {
        state = VICTORY;
        message = "Ganaste!";
      } else {
        state = ENEMY_TURN;
        enemyAttack();
      }
      needsRedraw = true;
    }
  }

  else if (state == ENEMY_TURN) {
    if (in.pressedA() || millis() - messageStart > 900) {
      if (player.hp <= 0) {
        state = DEFEAT;
        message = "Perdiste...";
      } else {
        state = PLAYER_CHOOSE;
        message = "Elige ataque";
      }
      needsRedraw = true;
    }
  }

  else if (state == VICTORY || state == DEFEAT) {
    if (in.pressedA()) {
      init();
    }
  }
}

void PokemonBattleGame::render(Adafruit_ST7735& tft, SoundManager& sound) {
  if (!needsRedraw) return;

  drawBattle(tft);

  if (state == MESSAGE || state == ENEMY_TURN) {
    sound.playSelect();
  }

  needsRedraw = false;
}

void PokemonBattleGame::exit() {
}

void PokemonBattleGame::changeSelection(int dx, int dy) {
  int col = selectedAttack % 2;
  int row = selectedAttack / 2;

  col += dx;
  row += dy;

  if (col < 0) col = 1;
  if (col > 1) col = 0;
  if (row < 0) row = 1;
  if (row > 1) row = 0;

  selectedAttack = row * 2 + col;
  needsRedraw = true;
}

void PokemonBattleGame::playerAttack(int index) {
  Attack atk = player.attacks[index];

  enemy.hp -= atk.power;
  if (enemy.hp < 0) enemy.hp = 0;

  static char buffer[32];
  snprintf(buffer, sizeof(buffer), "%s usa %s", player.name, atk.name);
  message = buffer;
  messageStart = millis();
  state = MESSAGE;
  needsRedraw = true;
}

void PokemonBattleGame::enemyAttack() {
  int index = random(0, 4);
  Attack atk = enemy.attacks[index];

  player.hp -= atk.power;
  if (player.hp < 0) player.hp = 0;

  static char buffer[32];
  snprintf(buffer, sizeof(buffer), "%s usa %s", enemy.name, atk.name);
  message = buffer;
  messageStart = millis();
  needsRedraw = true;
}

void PokemonBattleGame::drawBattle(Adafruit_ST7735& tft) {
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);

  // -------- ENEMIGO --------
  // Nombre y barra a la izquierda
  tft.setCursor(5, 4);
  tft.print(enemy.name);
  drawHpBar(tft, 5, 16, enemy.hp, enemy.maxHp);

  // Charmander arriba a la derecha
  int cx = 130;
  int cy = 28;
  tft.fillCircle(cx, cy, 12, ST77XX_ORANGE);
  tft.fillCircle(cx + 4, cy - 5, 3, ST77XX_BLACK);
  tft.fillTriangle(cx - 10, cy + 8, cx - 18, cy + 18, cx - 4, cy + 13, ST77XX_ORANGE);
  tft.fillCircle(cx - 19, cy + 17, 4, ST77XX_RED);
  tft.fillCircle(cx - 19, cy + 17, 2, ST77XX_YELLOW);

  // -------- JUGADOR --------
  // Pikachu más a la izquierda
  int px = 40;
  int py = 62;
  tft.fillCircle(px, py, 13, ST77XX_YELLOW);
  tft.fillCircle(px - 5, py - 5, 2, ST77XX_BLACK);
  tft.fillCircle(px + 6, py + 2, 3, ST77XX_RED);
  tft.fillTriangle(px - 8, py - 11, px - 15, py - 24, px - 3, py - 14, ST77XX_YELLOW);
  tft.fillTriangle(px + 8, py - 11, px + 15, py - 24, px + 3, py - 14, ST77XX_YELLOW);
  tft.fillRect(px - 20, py + 2, 10, 5, ST77XX_YELLOW);
  tft.fillRect(px - 26, py - 2, 8, 5, ST77XX_YELLOW);

  // Nombre y barra de Pikachu más a la derecha
  tft.setCursor(80, 60);
  tft.print(player.name);
  drawHpBar(tft, 80, 70, player.hp, player.maxHp);

  // -------- CAJA DE ATAQUES --------
  // Más ancha y menos alta
  tft.drawRect(7, 85, 140, 35, ST77XX_WHITE);

  if (state == PLAYER_CHOOSE) {
    for (int i = 0; i < 4; i++) {
      int x = (i % 2 == 0) ? 12 : 72;   // antes 1 / 61 -> +5 px derecha
      int y = (i < 2) ? 93 : 107;      // antes 95 / 109 -> -2 px arriba

      if (i == selectedAttack) {
        tft.setTextColor(ST77XX_YELLOW);
        tft.setCursor(x, y);
        tft.print(">");
      } else {
        tft.setTextColor(ST77XX_WHITE);
        tft.setCursor(x, y);
        tft.print(" ");
      }

      tft.setCursor(x + 6, y);
      tft.print(player.attacks[i].name);
    }
  } else {
    tft.setTextColor(ST77XX_WHITE);

    tft.setCursor(11, 92);   // antes 6,102 -> +5 derecha y más arriba
    tft.print(message);

    if (state == VICTORY || state == DEFEAT) {
      tft.setCursor(11, 106);
      tft.print("A: reiniciar");
    }
  }
}

void PokemonBattleGame::drawHpBar(Adafruit_ST7735& tft, int x, int y, int hp, int maxHp) {
  int width = 60;
  int height = 6;

  tft.drawRect(x, y, width, height, ST77XX_WHITE);

  int fillWidth = map(hp, 0, maxHp, 0, width - 2);

  uint16_t color = ST77XX_GREEN;
  if (hp < maxHp / 2) color = ST77XX_YELLOW;
  if (hp < maxHp / 4) color = ST77XX_RED;

  tft.fillRect(x + 1, y + 1, fillWidth, height - 2, color);
}