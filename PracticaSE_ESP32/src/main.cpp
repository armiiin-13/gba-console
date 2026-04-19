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

// --- Pines ---
#define BUZZER_PIN 4

#define POWER_BUTTON 15

#define BUTTON_UP 1
#define BUTTON_DOWN 2
#define BUTTON_RIGHT 5
#define BUTTON_LEFT 6
#define BUTTON_A 20
#define BUTTON_B 41

#define PIN_CART 35 // interruptor que simula cartucho insertado

#define TFT_CS 9
#define TFT_RST 16
#define TFT_DC 8

#define RF_CS 10
#define RF_RST 11

#define SPI_SCK 18
#define SPI_MISO 3
#define SPI_MOSI 17

#define LED_PIN 48

// Inicializacion botones
Button btnPower(POWER_BUTTON);
Button btnUp(BUTTON_UP);
Button btnDown(BUTTON_DOWN);
Button btnLeft(BUTTON_LEFT);
Button btnRight(BUTTON_RIGHT);
Button btnA(BUTTON_A);
Button btnB(BUTTON_B);

// --- Components ---
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
MFRC522 mfrc522(RF_CS, RF_RST);
Adafruit_NeoPixel pixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);
SoundManager sound(BUZZER_PIN);

// Lista de juegos
ColorGame colorGame;

GameCard gameList[] = {
    {"A5 03 EC 05", "ColorGame", &colorGame},
    {"62 09 20 07", "Snake"},
    {"51 F7 1F 07", "Tetris"}};

const int numGames = sizeof(gameList) / sizeof(gameList[0]);

// --- Dependencias---
ConsoleState consoleState = STATE_OFF;
Screen *currentScreen = nullptr;
GameCard *activeGameCard = nullptr;
IGame *activeGame = nullptr;
GlobalConfig consoleConfig;

int cursorMenu = 1;       // 0=Jugar 1=Settings 2=Reloj
bool gameInserted = true; // Por implementar boton que detecta cartucho

// ---Metodos---

/*
Screen* gameFactory(String uid) {
  if (uid == "62 09 20 07") return new SnakeGame();
  if (uid == "51 F7 1F 07") return new TetrisGame();
  return nullptr;
}
*/

// =========================
// FUNCIONES SPI
// =========================
void selectScreen()
{
  digitalWrite(RF_CS, HIGH);
  digitalWrite(TFT_CS, LOW);
}

void deselectScreen()
{
  digitalWrite(TFT_CS, HIGH);
}

void selectRFID()
{
  digitalWrite(TFT_CS, HIGH);
  digitalWrite(RF_CS, LOW);
}

void deselectRFID()
{
  digitalWrite(RF_CS, HIGH);
}

void setScreen(Screen *nextScreen)
{
  // Salimos y liberamos la pantalla en la que estamos
  if (currentScreen != nullptr)
  {
    currentScreen->exit();
    delete currentScreen;
  }

  // Cambiamos la pantalla en la que estamos y la cargamos
  currentScreen = nextScreen;
  if (currentScreen != nullptr)
    currentScreen->enter();
}

// =========================
// ISR POWER
// =========================
volatile bool powerIRQ = false;
bool systemOn = false;

void IRAM_ATTR onPowerButton()
{
  powerIRQ = true;
}

// =========================
// UI SISTEMA
// =========================
void drawOffScreen()
{
  selectScreen();
  tft.fillScreen(ST77XX_BLACK);
}

void drawWaitingRFIDScreen()
{
  selectScreen();
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(12, 30);
  tft.println("Cartucho detectado");

  tft.setCursor(16, 50);
  tft.println("Acerca RFID");
}

void drawLoadingScreen()
{
  selectScreen();
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(18, 50);
  tft.println("Cargando juego...");
}

// =========================
// LECTURA DE INPUT
// =========================
InputState input;

void readInput()
{
  input.previousA = input.currentA;
  input.previousB = input.currentB;
  input.previousC = input.currentC;
  input.previousD = input.currentD;

  input.currentA = btnUp.isPressed();
  input.currentB = btnDown.isPressed();
  input.currentC = btnLeft.isPressed();
  input.currentD = btnRight.isPressed();
}

bool cartInserted = false;
bool lastCartInserted = false;

// =========================
// LECTURA DE CARTUCHO
// =========================
String readRFIDUid();
void detectInsertedGameCard();

void readCartridgeSwitch()
{
  lastCartInserted = cartInserted;
  cartInserted = (digitalRead(PIN_CART) == LOW);
}

// =========================
// POWER
// =========================
void processPowerButton()
{
  static uint32_t lastPress = 0;

  if (!powerIRQ)
    return;

  powerIRQ = false;
  uint32_t now = millis();

  if (now - lastPress <= 200)
    return;
  lastPress = now;

  systemOn = !systemOn;

  if (systemOn)
  {
    Serial.println("Sistema ON");

    if (currentScreen)
    {
      setScreen(nullptr);
    }

    cartInserted = (digitalRead(PIN_CART) == LOW);
    lastCartInserted = cartInserted;

    if (cartInserted)
    {
      detectInsertedGameCard();
    }
    else
    {
      activeGameCard = nullptr;
    }

    consoleState = STATE_MENU;
  }
  else
  {
    Serial.println("Sistema OFF");

    if (activeGame)
    {
      activeGame->exit();
      activeGameCard = nullptr;
      activeGame = nullptr;
    }

    if (currentScreen)
    {
      setScreen(nullptr);
    }

    consoleState = STATE_OFF;
    drawOffScreen();
  }
}

// =========================
// CAMBIO DE CARTUCHO
// =========================
void processCartridgeChange()
{
  if (!systemOn)
    return;

  // pasó de insertado -> no insertado
  if (!cartInserted && lastCartInserted)
  {
    Serial.println("Cartucho retirado");

    if (activeGame)
    {
      activeGame->exit();
      activeGame = nullptr;
    }

    activeGameCard = nullptr;

    if (currentScreen)
    {
      setScreen(nullptr);
    }

    consoleState = STATE_MENU;
  }

  // pasó de no insertado -> insertado

  if (cartInserted && !lastCartInserted)
  {
    Serial.println("Cartucho insertado");

    detectInsertedGameCard();

    if (consoleState == STATE_MENU)
    {
      if (currentScreen)
      {
        setScreen(nullptr);
      }
      consoleState = STATE_MENU;
    }
  }
}

// =========================
// RFID
// =========================
bool isRFIDCardPresent()
{
  selectRFID();

  if (!mfrc522.PICC_IsNewCardPresent())
  {
    deselectRFID();
    return false;
  }

  if (!mfrc522.PICC_ReadCardSerial())
  {
    deselectRFID();
    return false;
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  deselectRFID();

  return true;
}

String readRFIDUid()
{
  const uint32_t timeoutMs = 500;
  uint32_t start = millis();

  while (millis() - start < timeoutMs)
  {
    selectRFID();

    bool cardDetected = mfrc522.PICC_IsNewCardPresent();
    bool cardRead = mfrc522.PICC_ReadCardSerial();

    if (cardDetected && cardRead)
    {
      String uid = "";

      for (byte i = 0; i < mfrc522.uid.size; i++)
      {
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

void detectInsertedGameCard()
{
  activeGameCard = nullptr;

  selectRFID();
  mfrc522.PCD_Init();
  deselectRFID();
  delay(50);

  String uid = readRFIDUid();

  if (uid != "")
  {
    for (int i = 0; i < numGames; i++)
    {
      if (uid == gameList[i].uid)
      {
        activeGameCard = &gameList[i];
        Serial.print("Juego detectado: ");
        Serial.println(activeGameCard->name);
        break;
      }
    }

    if (activeGameCard == nullptr)
    {
      Serial.print("Tarjeta no reconocida: ");
      Serial.println(uid);
    }
  }
  else
  {
    Serial.println("No se detecto RFID");
  }
}

// =========================
// CARGA DE JUEGO
// =========================
IGame *loadSelectedGame()
{
  if (activeGameCard != nullptr)
  {
    return activeGameCard->game;
  }
  return nullptr;
}

void gameCardRead()
{
  if (!gameInserted)
    return;

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
  {
    // 1. Convertimos el UID leído a String
    String uidInput = "";
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
      uidInput += (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
      uidInput += String(mfrc522.uid.uidByte[i], HEX);
      if (i < mfrc522.uid.size - 1)
        uidInput += " ";
    }
    uidInput.toUpperCase();

    // 2. Generamos nuestro puntero con gameFactory
    for (int i = 0; i < numGames; i++)
    {
      // Comprobamos que el cartucho insertado esta registrado
      if (uidInput == gameList[i].uid)
      {
        activeGameCard = &gameList[i];
        currentScreen->enter();
        Serial.print("Cargando: ");
        Serial.println(activeGameCard->name);
        break;
      }
    }

    if (activeGameCard == nullptr)
      Serial.println("Tarjeta no reconocida: " + uidInput);

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.flush();
  delay(2000);

  pixel.begin();
  pixel.setBrightness(5);
  pixel.setPixelColor(0, pixel.Color(255, 150, 0)); // Amarillo (Iniciando)
  pixel.show();

  // Configurar Bus SPI de Hardware
  // Orden: SCLK, MISO, MOSI, SS (el SS global no importa mucho aquí)
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  digitalWrite(TFT_CS, HIGH); // Forzamos a la pantalla a ignorar el bus
  mfrc522.PCD_Init();

  // Inicializar Pantalla
  tft.initR(INITR_BLACKTAB); // O INITR_REDTAB según tu modelo
  tft.setRotation(1);
  drawOffScreen();

  sound.begin();
  sound.playBootUp();

  // Inicializar RFID
  mfrc522.PCD_Init();

  // Inicializar Interruptor
  pinMode(PIN_CART, INPUT_PULLUP);
  cartInserted = (digitalRead(PIN_CART) == LOW);
  lastCartInserted = cartInserted;

  // Inicializar botones
  btnUp.begin();
  btnDown.begin();
  btnLeft.begin();
  btnRight.begin();
  btnA.begin();
  btnB.begin();
  btnPower.begin();
  attachInterrupt(digitalPinToInterrupt(POWER_BUTTON), onPowerButton, FALLING);

  // Inicializamos Screen
  currentScreen = new MenuScreen(btnUp, btnDown, btnA, &activeGameCard);

  pixel.setPixelColor(0, pixel.Color(0, 0, 255)); // Azul (Listo)
  pixel.show();
  Serial.println("Bus compartido configurado.");
  delay(2000);
}

void loop()
{
  readInput();
  readCartridgeSwitch();
  processPowerButton();
  processCartridgeChange();

  if (!systemOn)
    return;

  switch (consoleState)
  {
  case STATE_OFF:
    break;

  case STATE_MENU:
  {
    if (currentScreen == nullptr)
    {
      setScreen(new MenuScreen(btnUp, btnDown, btnA, &activeGameCard));
    }
    {
      ConsoleState nextState = currentScreen->update();

      if (currentScreen->getNeedsRedraw())
      {
        selectScreen();
        currentScreen->draw(tft);
        deselectScreen();
      }

      if (nextState != STATE_MENU)
      {
        setScreen(nullptr);

        if (nextState == STATE_CALENDAR)
        {
          consoleState = STATE_CALENDAR;
        }
        else if (nextState == STATE_SETTINGS)
        {
          consoleState = STATE_SETTINGS;
        }
        else if (nextState == STATE_LOADING_GAME)
        {
          if (cartInserted && activeGameCard != nullptr)
          {
            consoleState = STATE_LOADING_GAME;
            drawLoadingScreen();
          }
          else
          {
            consoleState = STATE_MENU;
          }
        }
      }
    }
    break;
  }

  case STATE_CALENDAR:
  {
    if (currentScreen == nullptr)
    {
      setScreen(new CalendarScreen(btnB));
    }

    ConsoleState nextState = currentScreen->update();

    if (currentScreen->getNeedsRedraw())
    {
      selectScreen();
      currentScreen->draw(tft);
      deselectScreen();
    }

    if (nextState != STATE_CALENDAR)
    {
      setScreen(nullptr);

      if (nextState == STATE_MENU)
      {
        consoleState = STATE_MENU;
      }
    }

    break;
  }

  case STATE_SETTINGS:
  {
    if (currentScreen == nullptr)
    {
      setScreen(new SettingsScreen(btnUp, btnDown, btnLeft, btnRight, btnA, btnB));
    }

    {
      ConsoleState nextState = currentScreen->update();

      if (currentScreen->getNeedsRedraw())
      {
        selectScreen();
        currentScreen->draw(tft);
        deselectScreen();
      }

      if (nextState != STATE_SETTINGS)
      {
        setScreen(nullptr);

        if (nextState == STATE_MENU)
        {
          consoleState = STATE_MENU;
        }
      }
    }
    break;
  }

  case STATE_WAITING_CART:
  {
    break;
  }

  case STATE_LOADING_GAME:
  {
    activeGame = loadSelectedGame();
    if (activeGame)
    {
      activeGame->init();
      consoleState = STATE_GAME_RUNNING;
    }
    else
    {
      if (currentScreen)
      {
        setScreen(nullptr);
      }
      consoleState = STATE_MENU;
    }
    break;
  }

  case STATE_GAME_RUNNING:
  {
    if (!cartInserted)
    {
      if (activeGame)
      {
        activeGameCard = nullptr;
        activeGame->exit();
        activeGame = nullptr;
      }

      if (currentScreen)
      {
        setScreen(nullptr);
      }

      consoleState = STATE_MENU;
    }
    else if (activeGame)
    {
      activeGame->update(input);
      selectScreen();
      activeGame->render(tft);
      deselectScreen();
    }
    break;
  }
  }
}