#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_ST7735.h>
#include <MFRC522.h>
#include <SPI.h>

#include "Button.h"
#include "CalendarScreen.h"
#include "Definitions.h"
#include "LoadingScreen.h"
#include "MenuScreen.h"
#include "SettingsScreen.h"
#include "SoundManager.h"
#include "ColorGame.h"
#include "SnakeGame.h"
#include "TetrisGame.h"
#include "PokemonBattleGame.h"
#include "MemoryGame.h"
#include "DanceRevolutionGame.h"

// ---- PINS ----
#define BUZZER_PIN 4

#define POWER_BUTTON 15

#define BUTTON_UP 1
#define BUTTON_DOWN 2
#define BUTTON_RIGHT 5
#define BUTTON_LEFT 6
#define BUTTON_A 20
#define BUTTON_B 41

#define PIN_CART 35

#define TFT_CS 9
#define TFT_RST 16
#define TFT_DC 8

#define RF_CS 10
#define RF_RST 11

#define SPI_SCK 18
#define SPI_MISO 3
#define SPI_MOSI 17

#define LED_PIN 48

// ---- DEFINE BUTTONS ----
Button btnPower(POWER_BUTTON);
Button btnUp(BUTTON_UP);
Button btnDown(BUTTON_DOWN);
Button btnLeft(BUTTON_LEFT);
Button btnRight(BUTTON_RIGHT);
Button btnA(BUTTON_A);
Button btnB(BUTTON_B);

// ---- COMPONENTS ----
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
MFRC522 mfrc522(RF_CS, RF_RST);
Adafruit_NeoPixel pixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);
SoundManager sound(BUZZER_PIN);

// ---- GAMELIST ----
ColorGame colorGame;
SnakeGame snakeGame;
TetrisGame tetrisGame;
PokemonBattleGame pokeBattleGame;
MemoryGame memoryGame;
DanceRevolutionGame danceRev;

GameCard gameList[] = {
    {"A5 03 EC 05", "Color Game",  &colorGame},
    {"62 09 20 07", "Snake", &snakeGame},
    {"51 F7 1F 07", "Tetris", &tetrisGame},
    {"5D E0 B1 41", "Snake", &snakeGame},
    {"FC 2C B2 41", "Tetris", &tetrisGame},
    {"E0 EA B1 41", "Pokemon Battle", &pokeBattleGame},
    {"15 71 B2 41", "Memory Game", &memoryGame},
    {"4B A0 B1 41", "Dance Rev.", &danceRev}
  };  

const int numGames = sizeof(gameList) / sizeof(gameList[0]);

// ---- CONSOLE STATE VARIABLES ----
ConsoleState consoleState = STATE_OFF;
Screen *currentScreen = nullptr;
GameCard *activeGameCard = nullptr;
IGame *runningGame = nullptr;
GlobalConfig consoleConfig;

int cursorMenu = 1;       // 0=Play 1=Settings 2=Clock
bool gameInserted = true;

// ---- SPI CONTROLLERS ----
void selectScreen() {
  digitalWrite(RF_CS, HIGH);
  digitalWrite(TFT_CS, LOW);
}

void deselectScreen() {
  digitalWrite(TFT_CS, HIGH);
}

void selectRFID() {
  digitalWrite(TFT_CS, HIGH);
  digitalWrite(RF_CS, LOW);
}

void deselectRFID() {
  digitalWrite(RF_CS, HIGH);
}

// ---- POWER + ISR ----
volatile bool powerIRQ = false;
bool systemOn = false;

void IRAM_ATTR onPowerButton() {
  powerIRQ = true;
}

void processPowerButton() {
  static uint32_t lastPress = 0;

  if (!powerIRQ)
    return;

  powerIRQ = false;
  uint32_t now = millis();

  if (now - lastPress <= 200)
    return;
  lastPress = now;

  systemOn = !systemOn;

  if (systemOn) {
    Serial.println("System ON");

    if (currentScreen) {
      setScreen(nullptr);
    }

    cartInserted = (digitalRead(PIN_CART) == LOW);
    lastCartInserted = cartInserted;

    if (cartInserted) {
      detectInsertedGameCard();
    } else {
      activeGameCard = nullptr;
    }

    consoleState = STATE_MENU;
  } else {
    Serial.println("System OFF");

    if (runningGame) {
      runningGame->exit();
      activeGameCard = nullptr;
      runningGame = nullptr;
    }

    if (currentScreen) {
      setScreen(nullptr);
    }

    consoleState = STATE_OFF;
    drawOffScreen();
  }
}

// ---- SYSTEM SCREENS ----
void drawOffScreen() {
  selectScreen();
  tft.fillScreen(ST77XX_BLACK);
}

void drawLoadingScreen() {
  selectScreen();
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(18, 50);
  tft.println("Loading Game...");
}

void setScreen(Screen *nextScreen) {
  // exit and free screen
  if (currentScreen != nullptr) {
    currentScreen->exit();
    delete currentScreen;
  }

  // change screen and load
  currentScreen = nextScreen;
  if (currentScreen != nullptr)
    currentScreen->enter();
}

// ---- RFID ----
bool isRFIDCardPresent() {
  selectRFID();

  if (!mfrc522.PICC_IsNewCardPresent()) {
    deselectRFID();
    return false;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    deselectRFID();
    return false;
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  deselectRFID();

  return true;
}

String readRFIDUid() {
  const uint32_t timeoutMs = 500;
  uint32_t start = millis();

  while (millis() - start < timeoutMs) {
    selectRFID();

    bool cardDetected = mfrc522.PICC_IsNewCardPresent();
    bool cardRead = mfrc522.PICC_ReadCardSerial();

    if (cardDetected && cardRead) {
      String uid = "";

      for (byte i = 0; i < mfrc522.uid.size; i++) {
        if (mfrc522.uid.uidByte[i] < 0x10)
          uid += "0";
        uid += String(mfrc522.uid.uidByte[i], HEX);
        if (i < mfrc522.uid.size - 1)
          uid += " ";
      }

      uid.toUpperCase();

      Serial.print("UID: ");
      Serial.println(uid);

      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
      deselectRFID();

      return uid;
    }

    deselectRFID();
    delay(20);
  }

  return "";
}

void detectInsertedGameCard(){
  activeGameCard = nullptr;

  selectRFID();
  mfrc522.PCD_Init();
  deselectRFID();
  delay(50);

  String uid = readRFIDUid();

  if (uid != "") {
    for (int i = 0; i < numGames; i++) {
      if (uid == gameList[i].uid) {
        activeGameCard = &gameList[i];
        Serial.print("Juego detectado: ");
        Serial.println(activeGameCard->name);
        break;
      }
    }

    if (activeGameCard == nullptr) {
      Serial.print("Card not recognised: ");
      Serial.println(uid);
    }
  } else {
    Serial.println("No se detecto RFID");
  }
}

// ---- READ INPUT ----
InputState input;

void readInput() {
  input.previousUp = input.currentUp;
  input.previousDown = input.currentDown;
  input.previousLeft = input.currentLeft;
  input.previousRight = input.currentRight;
  input.previousA = input.currentA;
  input.previousB = input.currentB;

  input.currentUp = btnUp.isPressed();
  input.currentDown = btnDown.isPressed();
  input.currentLeft = btnLeft.isPressed();
  input.currentRight = btnRight.isPressed();
  input.currentA = btnA.isPressed();
  input.currentB = btnB.isPressed();
}

// ---- CARTRIDGE SWITCH ----
bool cartInserted = false;
bool lastCartInserted = false;

void readCartridgeSwitch() {
  lastCartInserted = cartInserted;
  cartInserted = (digitalRead(PIN_CART) == HIGH);
}

void processCartridgeChange() {
  if (!systemOn)
    return;

  // inserted to not inserted
  if (!cartInserted && lastCartInserted) {
    Serial.println("Card exited");

    if (runningGame) {
      runningGame->exit();
      runningGame = nullptr;
    }

    activeGameCard = nullptr;

    if (currentScreen){
      setScreen(nullptr);
    }

    consoleState = STATE_MENU;
  }

  // not inserted to inserted
  if (cartInserted && !lastCartInserted) {
    Serial.println("Card inserted");

    detectInsertedGameCard();

    if (consoleState == STATE_MENU) {
      if (currentScreen) {
        setScreen(nullptr);
      }
      consoleState = STATE_MENU;
    }
  }
}


// ---- LOAD GAME ----
IGame *loadSelectedGame() {
  if (activeGameCard != nullptr) {
    return activeGameCard->game;
  }
  return nullptr;
}

void gameCardRead() {
  if (!gameInserted)
    return;

  selectRFID();
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String uidInput = readRFIDUid(); 

    // Generate pointer with gameFactory
    for (int i = 0; i < numGames; i++) {
      if (uidInput == gameList[i].uid) {
        activeGameCard = &gameList[i];
        currentScreen->enter();
        Serial.print("Loading: ");
        Serial.println(activeGameCard->name);
        break;
      }
    }

    if (activeGameCard == nullptr)
      Serial.println("Card not recognized: " + uidInput);

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }
  deselectRFID();
}

// ---- SETUP & LOOP ----
void setup(){
  Serial.begin(115200);
  Serial.flush();
  delay(2000);

  pixel.begin();
  pixel.setBrightness(5);
  pixel.setPixelColor(0, pixel.Color(255, 150, 0)); // Yellow (init)
  pixel.show();

  // Configure SPI bus of the hardware
  // Order: SCLK, MISO, MOSI, SS (el SS global do not matter here)
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  digitalWrite(TFT_CS, HIGH); // force screen to ignore the bus 
  mfrc522.PCD_Init();

  // Init screen
  tft.initR(INITR_BLACKTAB); // or INITR_REDTAB (depends on the model)
  tft.setRotation(3);
  drawOffScreen();

  sound.begin();

  // Init RFID
  mfrc522.PCD_Init();

  // Init switch
  pinMode(PIN_CART, INPUT_PULLUP);
  cartInserted = (digitalRead(PIN_CART) == LOW);
  lastCartInserted = cartInserted;

  // Init buttons
  btnUp.begin();
  btnDown.begin();
  btnLeft.begin();
  btnRight.begin();
  btnA.begin();
  btnB.begin();
  btnPower.begin();
  attachInterrupt(digitalPinToInterrupt(POWER_BUTTON), onPowerButton, FALLING);

  // Init screen
  currentScreen = new MenuScreen(btnUp, btnDown, btnA, &activeGameCard, sound);

  pixel.setPixelColor(0, pixel.Color(0, 0, 255)); // Blue (ready)
  pixel.show();
  Serial.println("Shared bus configurated.");
  delay(2000);
}

void loop() {
  readInput();
  readCartridgeSwitch();
  processPowerButton();
  processCartridgeChange();

  if (!systemOn)
    return;

  switch (consoleState){
    case STATE_OFF:
      break;

    case STATE_MENU: {
      if (currentScreen == nullptr) {
        setScreen(new MenuScreen(btnUp, btnDown, btnA, &activeGameCard, sound));
      }
      {
        ConsoleState nextState = currentScreen->update();

        if (currentScreen->getNeedsRedraw()){
          selectScreen();
          currentScreen->draw(tft);
          deselectScreen();
        }

        if (nextState != STATE_MENU){
          setScreen(nullptr);

          if (nextState == STATE_CALENDAR){
            consoleState = STATE_CALENDAR;
          } else if (nextState == STATE_SETTINGS){
            consoleState = STATE_SETTINGS;
          } else if (nextState == STATE_LOADING_GAME){
            if (cartInserted && activeGameCard != nullptr){
              consoleState = STATE_LOADING_GAME;
              drawLoadingScreen();
            } else {
              consoleState = STATE_MENU;
            }
          }
        }
      }
      break;
    }

    case STATE_CALENDAR: {
      if (currentScreen == nullptr) {
        setScreen(new CalendarScreen(btnB));
      }

      ConsoleState nextState = currentScreen->update();

      if (currentScreen->getNeedsRedraw()) {
        selectScreen();
        currentScreen->draw(tft);
        deselectScreen();
      }

      if (nextState != STATE_CALENDAR){
        setScreen(nullptr);

        if (nextState == STATE_MENU) {
          consoleState = STATE_MENU;
        }
      }

      break;
    }

    case STATE_SETTINGS: {
      if (currentScreen == nullptr) {
        setScreen(new SettingsScreen(btnUp, btnDown, btnLeft, btnRight, btnA, btnB, sound));
      }

      {
        ConsoleState nextState = currentScreen->update();

        if (currentScreen->getNeedsRedraw()){
          selectScreen();
          currentScreen->draw(tft);
          deselectScreen();
        }

        if (nextState != STATE_SETTINGS){
          setScreen(nullptr);

          if (nextState == STATE_MENU) {
            consoleState = STATE_MENU;
          }
        }
      }
      break;
    }

    case STATE_LOADING_GAME: {
      runningGame = loadSelectedGame();
      if (runningGame) {
        runningGame->init();
        consoleState = STATE_GAME_RUNNING;
      } else {
        if (currentScreen) {
          setScreen(nullptr);
        }
        consoleState = STATE_MENU;
      }
      break;
    }

    case STATE_GAME_RUNNING: {
      if (!cartInserted) {
        if (runningGame) {
          activeGameCard = nullptr;
          runningGame->exit();
          runningGame = nullptr;
        }

        if (currentScreen) {
          setScreen(nullptr);
        }

        consoleState = STATE_MENU;
      } else if (runningGame) {
        runningGame->update(input);
        selectScreen();
        runningGame->render(tft, sound);
        deselectScreen();
      }
      break;
    }
  }
}