#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <FastLED.h>

// ================= LED CONFIG =================
#define LED_PIN        5
#define NUM_LEDS       255
#define LED_TYPE       WS2812B
#define COLOR_ORDER    GRB

// ================= WIFI =================
const char* WIFI_SSID = "LED_MASTER";
const char* WIFI_PASS = "12345678";

// ================= UDP =================
WiFiUDP udp;
const uint16_t UDP_PORT = 4210;

// ================= LED GLOBALS =================
CRGBArray<NUM_LEDS> leds;
CRGB ledsLeft[NUM_LEDS];
CRGB ledsRight[NUM_LEDS];

uint16_t configured_leds = NUM_LEDS;
uint16_t configured_leds_ALL = NUM_LEDS;

// Initial Colors
uint8_t h = 84;
uint8_t s = 255;
const uint8_t v = 255;
uint8_t brightness = 127;

// Mode Control
int LEDmode = 0;

// ================= EFFECT HEADERS =================
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

// ================= PACKET =================
#pragma pack(push,1)
struct ControlPacket {
  uint8_t  id;
  uint16_t value;
};
#pragma pack(pop)

// ================= EFFECT OBJECTS (FIXED) =================
BouncingBallEffect ballz(NUM_LEDS, 3, 64, false);
BouncingBallEffect ballzMirr(NUM_LEDS, 3, 64, true);

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println("\n=== ESP8266 LED SLAVE (UDP) ===");

  // WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nConnected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // UDP
  udp.begin(UDP_PORT);
  Serial.println("UDP ready");

  // FastLED
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(brightness);

  // Test flash
  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.show();
  delay(300);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

// ================= UDP RX =================
void handleUDP() {
  int size = udp.parsePacket();
  if (size != sizeof(ControlPacket)) return;

  ControlPacket pkt;
  udp.read((uint8_t*)&pkt, sizeof(pkt));

  Serial.printf("[RX] ID:%d VAL:%d\n", pkt.id, pkt.value);

  switch (pkt.id) {
    case 0:
      LEDmode = 0;
      break;

    case 2:
      configured_leds = constrain(pkt.value, 1, NUM_LEDS);
      fill_solid(leds + configured_leds,
                 NUM_LEDS - configured_leds,
                 CRGB::Black);
      FastLED.show();
      break;

    case 4:
      brightness = constrain(pkt.value, 0, 255);
      FastLED.setBrightness(brightness);
      FastLED.show();
      break;

    case 6:
      h = constrain(pkt.value, 0, 255);
      break;

    case 7:
      s = constrain(pkt.value, 0, 255);
      break;

    case 8:
      LEDmode = 1;
      break;

    default:
      LEDmode = pkt.id;
      break;
  }
}

// ================= LOOP =================
void loop() {
  handleUDP();

  switch (LEDmode) {
    case 0:
      fadeToBlackBy(leds, configured_leds_ALL, 10);
      break;

    case 1:
      for (int i = 0; i < configured_leds; i++) {
        leds[i] = CHSV(h, s, v);
      }
      break;

    case 26:
      movingDot();
      break;

    case 47:
      ballz.Draw();
      break;

    case 48:
      ballzMirr.Draw();
      break;

    // === keep ALL your other cases unchanged ===
    // (I trimmed here for readability)
  }

  if (LEDmode != 47 && LEDmode != 48) {
    FastLED.show();
  }

  yield(); // ESP8266 stability
}
