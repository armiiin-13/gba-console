#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_ST7735.h>
#include <Arduino.h>
#include <MFRC522.h>
#include <SPI.h>

#include "Button.h"
#include "CalendarScreen.h"
#include "Definitions.h"
#include "LoadingScreen.h"
#include "MenuScreen.h"
#include "SettingsScreen.h"
#include "SoundManager.h"

// --- Pines ---
#define BUZZER_PIN 4
#define TFT_CS 7
#define TFT_DC 5
#define TFT_RST 9
#define TFT_SDA_RF_MOSI 11
#define TFT_SCK_RF_SCK 12

#define RF_CS 10
#define RF_RST 46
#define RF_MISO 13

#define LED_PIN 48

// Inicializacion botones
Button btnUp(16);
Button btnDown(17);
Button btnLeft(18);
Button btnRight(15);
Button btnA(8);
Button btnB(3);

// --- Components ---
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
MFRC522 mfrc522(RF_CS, RF_RST);
Adafruit_NeoPixel pixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);
SoundManager sound(BUZZER_PIN);

// Lista de juegos
GameCard gameList[] = {{"62 09 20 07", "Snake"}, {"51 F7 1F 07", "Tetris"}};

// --- Dependencias---
ConsoleState currentState = mainScreen;
Screen* currentScreen = nullptr;
GameCard* activeGameCard = nullptr;
GlobalConfig consoleConfig;

const int numGames = sizeof(gameList) / sizeof(gameList[0]);
int cursorMenu = 1;        // 0=Jugar 1=Settings 2=Reloj
bool gameInserted = true;  // Por implementar boton que detecta cartucho

// ---Metodos---

/*
Screen* gameFactory(String uid) {
  if (uid == "62 09 20 07") return new SnakeGame();
  if (uid == "51 F7 1F 07") return new TetrisGame();
  return nullptr;
}
*/

void setScreen(Screen* nextScreen) {
  // Salimos y liberamos la pantalla en la que estamos
  if (currentScreen != nullptr) {
    currentScreen->exit();
    delete currentScreen;
  }

  // Cambiamos la pantalla en la que estamos y la cargamos
  currentScreen = nextScreen;
  if (currentScreen != nullptr) currentScreen->enter();
}

void gameCardRead() {
  if (!gameInserted) return;

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // 1. Convertimos el UID leído a String
    String uidInput = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uidInput += (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
      uidInput += String(mfrc522.uid.uidByte[i], HEX);
      if (i < mfrc522.uid.size - 1) uidInput += " ";
    }
    uidInput.toUpperCase();

    // 2. Generamos nuestro puntero con gameFactory
    for (int i = 0; i < numGames; i++) {
      // Comprobamos que el cartucho insertado esta registrado
      if (uidInput == gameList[i].uid) {
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

void setup() {
  Serial.begin(115200);
  Serial.flush();
  delay(2000);

  pixel.begin();
  pixel.setBrightness(5);
  pixel.setPixelColor(0, pixel.Color(255, 150, 0));  // Amarillo (Iniciando)
  pixel.show();

  // Configurar Bus SPI de Hardware
  // Orden: SCLK, MISO, MOSI, SS (el SS global no importa mucho aquí)
  SPI.begin(TFT_SCK_RF_SCK, RF_MISO, TFT_SDA_RF_MOSI);

  digitalWrite(TFT_CS, HIGH);  // Forzamos a la pantalla a ignorar el bus
  mfrc522.PCD_Init();

  // Inicializar Pantalla
  tft.initR(INITR_BLACKTAB);  // O INITR_REDTAB según tu modelo
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(20, 40);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_GREEN);
  tft.println("BIENVENIDO");

  sound.begin();
  sound.playBootUp();

  // Inicializar RFID
  mfrc522.PCD_Init();

  // Inicializar botones
  btnUp.begin();
  btnDown.begin();
  btnLeft.begin();
  btnRight.begin();
  btnA.begin();
  btnB.begin();

  // Inicializamos Screen
  currentScreen = new MenuScreen(btnUp, btnDown, btnA, &activeGameCard);

  pixel.setPixelColor(0, pixel.Color(0, 0, 255));  // Azul (Listo)
  pixel.show();
  Serial.println("Bus compartido configurado.");
  delay(2000);
}

void loop() {
  gameCardRead();

  if (currentScreen != nullptr) {
    ConsoleState newState = currentScreen->update();
    if (newState != currentState) {
      currentState = newState;

      switch (currentState) {
        case (mainScreen):
          setScreen(new MenuScreen(btnUp, btnDown, btnA, &activeGameCard));
          break;
        case (activeGame):
          if (activeGameCard != nullptr) {
            setScreen(new LoadingScreen(activeGameCard));
            //currentScreen->update();
          }
          break;
        case (calendar):
          setScreen(new CalendarScreen(btnB));
          break;

        case (settings):
          setScreen(new SettingsScreen(btnUp, btnDown, btnLeft, btnRight, btnA, btnB));
          //currentScreen->update();
          break;
        default:
          // Exception?
          break;
      };
    }
  }

  if (currentScreen->getNeedsRedraw()) {
    currentScreen->draw(tft);
    sound.playSelect();
  }
}