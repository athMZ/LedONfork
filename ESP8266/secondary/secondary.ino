#include <ESP8266WiFi.h>
#include <espnow.h>
#include <FastLED.h>

#define NUM_LEDS  255
#define LED_PIN   5
#define DITHER 1

//CRGB leds[NUM_LEDS];
CRGBArray<NUM_LEDS> leds;
CRGB ledsLeft[NUM_LEDS];
CRGB ledsRight[NUM_LEDS];


uint8_t configured_leds = 20;
uint8_t configured_leds_ALL = 255;

const uint8_t v = 255;

uint8_t h = 84;
uint8_t s = 255;
uint8_t  brightness = 127;
int LEDmode = 0;


#include "palette.h"
#include "wavesandblurs.h"
#include "noises.h"
#include "FLeffects.h"
#include "pride2015.h"
#include "twinkleFox.h"
#include "demoReel.h"
#include "davesFX.h"
#include "atulineFX.h"

//Structure example to receive data
//Must match the sender structure
typedef struct __attribute__((packed)) {
  int id;
  int valuez;
  uint32_t timestamp;  // MUST MATCH ESP32 structure!
} test_struct;

//Create a struct_message called myData
test_struct myData;

volatile bool newData = false;
volatile int receivedId = -1;
volatile int receivedValue = 0;

//callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&myData, incomingData, sizeof(test_struct));
  receivedId = myData.id;
  receivedValue = myData.valuez;
  newData = true;
}

// Debug timer
unsigned long lastDebugPrint = 0;

void processReceivedData() {
  if (!newData) return;

  // Create atomic snapshot of data to prevent race conditions
  noInterrupts();
  int _id = receivedId;
  int _val = receivedValue;
  newData = false; 
  interrupts();

  Serial.print("\n>>> RECV ID: "); Serial.print(_id);
  Serial.print(" VAL: "); Serial.println(_val);

  switch (_id) {
    case 0:      
      LEDmode = 0;
      Serial.println("    -> LEDmode = 0 (OFF)");
      break;
    case 2:
      if (_val > 0 && _val <= NUM_LEDS) {
        FastLED.clear();
        FastLED.show();
        configured_leds = _val;
        FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, configured_leds);
        FastLED.setBrightness(brightness);
        Serial.print("    -> configured_leds = "); Serial.println(configured_leds);
      } else {
        Serial.print("    -> INVALID LED count: "); Serial.println(_val);
      }
      break;
    case 4:
      brightness = _val;
      FastLED.setBrightness(brightness);
      Serial.print("    -> brightness = "); Serial.println(brightness);
      break;
    case 6:  
      h = _val;
      Serial.print("    -> h (hue) = "); Serial.println(h);
      break;
    case 7:  
      s = _val;
      Serial.print("    -> s (sat) = "); Serial.println(s);
      break;
    case 8:
      LEDmode = 1;
      Serial.println("    -> LEDmode = 1 (SOLID COLOR)");
      break;
    default:
      LEDmode = _id;
      Serial.print("    -> LEDmode = "); Serial.println(LEDmode);
      break;
  }
}

void setup() {
  //Initialize Serial Monitor
  Serial.begin(115200);

  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  //Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  configured_leds = 20;
  
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, configured_leds);
  FastLED.setBrightness(brightness);

  Serial.println("\n=== SETUP COMPLETE ===");
  Serial.print("configured_leds: "); Serial.println(configured_leds);
  Serial.print("brightness: "); Serial.println(brightness);
  Serial.print("h (hue): "); Serial.println(h);
  Serial.print("s (sat): "); Serial.println(s);
  Serial.print("v (val): "); Serial.println(v);
  Serial.print("LEDmode: "); Serial.println(LEDmode);
  Serial.println("======================\n");

  paletteBlendingIndex();

  chooseNextColorPalette(gTargetPalette);
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);

}





void loop() {
  processReceivedData();
  yield(); // Important for WiFi/ESP-NOW stack

  // Debug print every 3 seconds
  if (millis() - lastDebugPrint > 3000) {
    Serial.print("[STATUS] Mode:"); Serial.print(LEDmode);
    Serial.print(" LEDs:"); Serial.print(configured_leds);
    Serial.print(" H:"); Serial.print(h);
    Serial.print(" S:"); Serial.print(s);
    Serial.print(" V:"); Serial.print(v);
    Serial.print(" Bright:"); Serial.println(brightness);
    lastDebugPrint = millis();
  }

  BouncingBallEffect ballz(configured_leds, 3, 64, false);
  BouncingBallEffect ballzMirr(configured_leds, 3, 64, true);

  switch (LEDmode) {
    case 0:      
      fadeToBlackBy(leds, configured_leds_ALL, 10);
      break;
    case 1:
      // SOLID COLOR MODE
      for (int i = 0; i < configured_leds; i++) {
        leds[i] = CHSV(h, s, v);
      }
      break;
    case 2:
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
      movingDot();
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
      while (LEDmode == 47) {
        ballz.Draw();
      }
      break;
    case 48:
      while (LEDmode == 48) {
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


  }
  FastLED.show();
}
