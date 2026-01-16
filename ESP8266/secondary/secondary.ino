#include <ESP8266WiFi.h>
#include <espnow.h>
#include <FastLED.h>

#define LED_PIN        5
#define NUM_LEDS       255
#define LED_TYPE       WS2812B
#define COLOR_ORDER    GRB
#define ESPNOW_CHANNEL 6

// ====== GLOBALS ======
CRGBArray<NUM_LEDS> leds;
CRGB ledsLeft[NUM_LEDS];  // Used by some effects? Kept from snippet
CRGB ledsRight[NUM_LEDS]; // Used by some effects? Kept from snippet

uint16_t configured_leds = 255;
uint16_t configured_leds_ALL = NUM_LEDS;

// Initial Colors
uint8_t h = 84;
uint8_t s = 255;
const uint8_t v = 255;
uint8_t brightness = 127;

// Mode Control
int LEDmode = 0;
bool connected = false;

// Effect Headers
#include "palette.h"
#include "wavesandblurs.h"
#include "noises.h"
#include "FLeffects.h"
#include "pride2015.h"
#include "twinkleFox.h"
#include "demoReel.h"
#include "davesFX.h"
#include "atulineFX.h"
#include "newEffects.h"

// ====== DATA STRUCTURE ======
// Must match Master exactly
typedef struct __attribute__((packed)) {
  int id;
  int valuez;
  uint32_t timestamp;
} test_struct;

test_struct myData;

// ====== ESP-NOW CALLBACK ======
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  if (len != sizeof(test_struct)) {
    Serial.print("Data Size Mismatch: Expected ");
    Serial.print(sizeof(test_struct));
    Serial.print(" got ");
    Serial.println(len);
    return;
  }

  memcpy(&myData, incomingData, sizeof(test_struct));
  
  // Debug
  Serial.printf("[RX] ID:%d Val:%d\n", myData.id, myData.valuez);

  switch (myData.id) {
    case 0:
      LEDmode = 0; // Off
      break;
      
    case 2:
      // Configure LED count
      configured_leds = constrain(myData.valuez, 1, NUM_LEDS);
      // Clear the rest of the strip to avoid artifacts
      if (configured_leds < NUM_LEDS) {
        fill_solid(leds + configured_leds, NUM_LEDS - configured_leds, CRGB::Black);
      }
      FastLED.show();
      break;
      
    case 4:
      // Brightness
      brightness = constrain(myData.valuez, 0, 255);
      FastLED.setBrightness(brightness);
      FastLED.show();
      break;
      
    case 6:  
      // Hue
      h = constrain(myData.valuez, 0, 255);
      break;
      
    case 7:  
      // Saturation
      s = constrain(myData.valuez, 0, 255);
      break;
      
    case 8:
      // Solid Color Mode
      LEDmode = 1;
      break;
      
    default:
      // Effect Mode
      LEDmode = myData.id;
      break;
  }
}

// ====== SETUP ======
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  delay(100);

  Serial.println("\n=== ESP8266 SLAVE STARTING ===");

  // 1. Wifi Init
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  // 2. ESP-NOW Init
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();
  }
  
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("ESP-NOW Ready");

  // 3. FastLED Init
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(brightness);
  
  // Test Flash
  fill_solid(leds, configured_leds, CRGB::Red);
  FastLED.show();
  delay(500);
  fill_solid(leds, configured_leds, CRGB::Black);
  FastLED.show();
}

// ====== MAIN LOOP ======
void loop() {
  BouncingBallEffect ballz(configured_leds, 3, 64, false);
  BouncingBallEffect ballzMirr(configured_leds, 3, 64, true);

  // Handle Logic
  switch (LEDmode) {
    case 0: // OFF      
      fadeToBlackBy(leds, configured_leds_ALL, 10);
      break;

    case 1: // SOLID COLOR
      for (int i = 0; i < configured_leds; i++) {
        leds[i] = CHSV(h, s, v);
      }
      break;
      
      // === Effects ===
      
    case 2:
      // NOP (Config placeholder)
      break;
      
    case 9:
      movingDots();
      break;
    case 12:
      heatMap();
      break;
    case 13:
      paletteBlending();
      break;
    case 14:
      seaGradient();
      break;
    case 15:
      blackened();
      break;
    case 16:
      paletteKnife();
      break;


    case 21:
      addingWaves();
      break;
    case 23:
      blurPhaseBeat();
      break;
    case 24:
      brightnessWaves();
      break;
    case 25:
      gradientBeat();
      break;
    case 26:
      movingDot(); // user log showed ID 26
      break;
    case 27:
      phaseBeat();
      break;
    case 28:
      rainbowBeat();
      break;
    case 29:
      sawTooth();
      break;

    case 31:
      fillRawNoise8();
      break;
    case 32:
      fire();
      break;
    case 33:
      inoiseEight();
      break;
    case 34:
      inoiseEightMoving();
      break;
    case 35:
      lava();
      break;
    case 36:
      movingPixel();
      break;
    case 37:
      prettyFill();
      break;
    case 38:
      ripple();
      break;
    case 39:
      comet();
      break;

    case 41:
      fireFastLed();
      break;
    case 42:
      cylon();
      break;
    case 43:
      runPacifica();
      break;
    case 44:
      runPride();
      break;
    case 45:
      runTwinkleFox();
      break;
    case 46:
      runDemoReel();
      break;
      
    case 47: 
      if (LEDmode == 47) {
        ballz.Draw();
      }
      break;
      
    case 48:
      if (LEDmode == 48) {
        ballzMirr.Draw();
      }
      break;
      
    case 49:
      DrawMarquee();
      break;
    case 50:
      DrawMarqueeMirrored();
      break;

    case 60:
      runConfetti();
      break;
    case 61:
      runConfetti2();
      break;
    case 62:
      runDotBeat();
      break;
    case 63:
      runEase();
      break;
    case 64:
      Lightning();
      break;
    case 65:
      runPlasma();
      break;
    case 66:
      RainbowMarch();
      break;
    case 67:
      RainbowMarch2();
      break;
    case 68:
      runSerendipitous();
      break;
    case 69:
      ThreeSinDemo();
      break; 
    case 70:
      runNoise16_1();
      break;
    case 71:
      runNoise16_2();
      break;
    case 72:
      runNoise16_3();
      break;

    // === NEW EFFECTS ===
    case 80:
      breathingEffect();
      break;
    case 81:
      matrixRainEffect();
      break;
    case 82:
      policeStrobe();
      break;
    case 83:
      colorWipeEffect();
      break;
  }
  
  if (LEDmode != 47 && LEDmode != 48) {
    FastLED.show();
  }
  
  // Essential for ESP8266 stability
  yield();
}
