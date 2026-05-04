#include "PokemonBattleGame.h"

// ---- COLOR DEFINITIONS ----
#define C_TRANSPARENT 0x0000
#define C_LIGHT_YELLOW 0xFFF2
#define C_DARK_GREY 0x2104
#define C_GREY 0x4A69
#define C_BROWN 0x69C2
#define C_LIGHT_BROWN 0xAC2C
#define C_DARK_BROWN 0x4101

// ---- FUNCTION DECLARATION ----
void drawTorchicSprite(Adafruit_ST7735& tft, int x, int y, int s);
void drawZigzagoonSprite(Adafruit_ST7735& tft, int x, int y, int s);

void PokemonBattleGame::init() {
  player = {
    "TORCHIC",
    100,
    100,
    {
      {"EMBER", 20},
      {"SCRATCH", 16},
      {"GROWL", 0},
      {"PECK", 10}
    }
  };

  enemy = {
    "ZIGZAGOON",
    100,
    100,
    {
      {"TAIL WHIP", 0},
      {"TACKLE", 17},
      {"GROWL", 0},
      {"HEADBUTT", 24}
    }
  };

  state = PLAYER_CHOOSE;
  selectedAttack = 0;
  message = "Choose attack";
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
        message = "You won!";
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
        message = "You lose...";
      } else {
        state = PLAYER_CHOOSE;
        message = "Choose attack";
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
  snprintf(buffer, sizeof(buffer), "%s used %s", player.name, atk.name);
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
  snprintf(buffer, sizeof(buffer), "%s used %s", enemy.name, atk.name);
  message = buffer;
  messageStart = millis();
  needsRedraw = true;
}

void PokemonBattleGame::drawBattle(Adafruit_ST7735& tft) {
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);

  // -------- ENEMY --------
  // health bar + name
  tft.setCursor(5, 4);
  tft.print(enemy.name);
  drawHpBar(tft, 5, 16, enemy.hp, enemy.maxHp);

  // drawing
  int cx = 87;
  int cy = 9;
  drawZigzagoonSprite(tft, cx, cy, 3);

  // -------- PLAYER --------
  // health bar + name
  tft.setCursor(80, 60);
  tft.print(player.name);
  drawHpBar(tft, 80, 70, player.hp, player.maxHp);

  // drawing
  int px = 28;
  int py = 29;
  drawTorchicSprite(tft, px, py, 3);

  // -------- ATTACK BOX --------
  tft.drawRect(7, 85, 150, 35, ST77XX_WHITE);

  if (state == PLAYER_CHOOSE) {
    for (int i = 0; i < 4; i++) {
      int x = (i % 2 == 0) ? 12 : 72;   
      int y = (i < 2) ? 93 : 107;

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

    tft.setCursor(11, 92);
    tft.print(message);

    if (state == VICTORY || state == DEFEAT) {
      tft.setCursor(11, 106);
      tft.print("A: restart");
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

// ---- DRAWING FUNCTIONS ----
void drawTorchicSprite(Adafruit_ST7735& tft, int x, int y, int s) {
  static const char* sprite[] = {
    "..NNN........",
    "..NYYN.N.....",
    ".NNOYYNYN....",
    ".NYYOYYYN....",
    ".NOYYYYYN....",
    "..NNOYRYONN..",
    ".NRROROROOON.",
    ".NOOOOOOOOOON",
    "NROOOOOOOO.WN",
    "NROOOOOOORRNN",
    "NRRONWOORLLRN",
    ".NRRNNRRLLON.",
    "NYNRRRRRRRN..",
    "NOYYORYRLYN..",
    "NROYYRRRRN...",
    ".NRRRRRRN....",
    ".NRRRNLN.....",
    "..NLN.NN.....",
    "...NN........"
  };

  for (int j = 0; j < 19; j++) {
    for (int i = 0; sprite[j][i] != '\0'; i++) {

      uint16_t c = C_TRANSPARENT;

      switch (sprite[j][i]) {
        case 'Y': c = ST77XX_YELLOW; break;
        case 'O': c = ST77XX_ORANGE; break;
        case 'R': c = ST77XX_RED; break;
        case 'L': c = C_LIGHT_YELLOW; break;
        case 'N': c = C_DARK_GREY; break;
        case 'W': c = ST77XX_WHITE; break;
      }

      if (c != C_TRANSPARENT) {
        tft.fillRect(x + i * s, y + j * s, s, s, c);
      }
    }
  }
}

void drawZigzagoonSprite(Adafruit_ST7735& tft, int x, int y, int s) {
  static const char* const sprite[] = {
    ".................NN...",
    "........NN..NN..NLN...",
    "...N.N.NLDNNLN.NLLNNN.",
    "..NBNBNLLLDBLLNGBLLLN.",
    "..NBDGGLDGGBBBLLGBBLNN",
    "..NDBBDGGBGGBBLLGGDLLN",
    ".NBBBBBGBBBGBDDLLGDNN.",
    ".NGGBBBBBBGLLDDLGDN...",
    ".NGGGGBBBGLLDDLGDN....",
    "NBBGGWGBBBGLDDDLN.....",
    "NGBGGRGBBGLLLGGBN.....",
    "NBBBGGBBBBGLBNLBN.....",
    ".NNDBBBDGGDNN.NN......",
    "...NNNNGLLN...........",
    "......NLLN............",
    ".......NN............."
  };

  for (int j = 0; j < 16; j++) {
    for (int i = 0; sprite[j][i] != '\0'; i++) {
      uint16_t c = C_TRANSPARENT;

      switch (sprite[j][i]) {
        case 'B': c = C_BROWN; break;
        case 'N': c = C_DARK_GREY; break;
        case 'D': c = C_DARK_BROWN; break;
        case 'L': c = C_LIGHT_BROWN; break;
        case 'G': c = C_GREY; break;
        case 'W': c = ST77XX_WHITE; break;
        case 'R': c = ST7735_RED; break;
      }

      if (c != C_TRANSPARENT) {
        tft.fillRect(x + i * s, y + j * s, s, s, c);
      }
    }
  }
}