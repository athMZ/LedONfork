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

//callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&myData, incomingData, sizeof(test_struct));
  Serial.print("Id: "); Serial.print(myData.id);
  Serial.println();
  Serial.print(", valuez: "); Serial.print(myData.valuez);
  Serial.println();


void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  lastReceiveTime = millis();
  connectionActive = true;
  receiveCount++;
  
  memcpy(&myData, incomingData, sizeof(test_struct));
  
  Serial.print("RX ID: ");
  Serial.print(myData.id);
  Serial.print(" Value: ");
  Serial.print(myData.valuez);

  // Process commands
  switch (myData.id) {
    case 0:
      LEDmode = 0;
      Serial.println("OK Mode: OFF");
      break;
      
    case 2:
      FastLED.clear();
      FastLED.show();
      configured_leds = constrain(myData.valuez, 1, NUM_LEDS);
      FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, configured_leds);
      FastLED.setBrightness(brightness);
      Serial.print("OK LED count: ");
      Serial.println(configured_leds);
      break;
      
    case 4:
      brightness = constrain(myData.valuez, 0, 255);
      FastLED.setBrightness(brightness);
      Serial.print("OK Brightness: ");
      Serial.println(brightness);
      break;
      
    case 6:
      h = constrain(myData.valuez, 0, 255);
      Serial.print("OK Hue: ");
      Serial.println(h);
      break;
      
    case 7:
      s = constrain(myData.valuez, 0, 255);
      Serial.print("OK Saturation: ");
      Serial.println(s);
      break;
      
    case 8:
      LEDmode = 1;
      Serial.println("OK Mode: Solid color");
      break;
      
    default:
      LEDmode = myData.id;
      Serial.print("OK Mode: ");
      Serial.println(LEDmode);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== ESP8266 LED Receiver ===");
  
  // PRINT MAC ADDRESS - COPY THIS FOR MAIN BOARD
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.println(">>> Copy this MAC address to main board <<<");
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.setOutputPower(15);
  
  if (esp_now_init() != 0) {
    Serial.println("ERR ESP-NOW init failed");
    delay(1000);
    ESP.restart();
    return;
  }
  
  Serial.println("OK ESP-NOW initialized");
  
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
  
  // Initialize FastLED
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, configured_leds);
  FastLED.setBrightness(brightness);
  
  // Startup animation - blue sweep
  for(int i = 0; i < configured_leds; i++) {
    leds[i] = CRGB::Blue;
    FastLED.show();
    delay(20);
  }
  FastLED.clear();
  FastLED.show();
  
  lastReceiveTime = millis();
  Serial.println("=== Ready ===\n");
}

void monitorConnection() {
  unsigned long now = millis();
  
  if (now - lastReceiveTime > RECEIVE_TIMEOUT) {
    if (connectionActive) {
      Serial.println("WARN Connection timeout");
      connectionActive = false;
    }
  }
  
  if (now - lastStatusReport > 30000) {
    Serial.print("Status: ");
    Serial.print(connectionActive ? "Connected" : "Disconnected");
    Serial.print(" | Messages: ");
    Serial.print(receiveCount);
    Serial.print(" | Mode: ");
    Serial.print(LEDmode);
    Serial.print(" | LEDs: ");
    Serial.print(configured_leds);
    Serial.print(" | Heap: ");
    Serial.println(ESP.getFreeHeap());
    
    lastStatusReport = now;
  }
}


void loop() {
  BouncingBallEffect ballz(configured_leds, 3, 64, false);
  BouncingBallEffect ballzMirr(configured_leds, 3, 64, true);
  monitorConnection();


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
