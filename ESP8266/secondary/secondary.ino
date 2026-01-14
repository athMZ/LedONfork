#include <ESP8266WiFi.h>
#include <espnow.h>
#include <FastLED.h>

#define LED_PIN        5
#define NUM_LEDS       255
#define LED_TYPE       WS2812B
#define COLOR_ORDER    GRB
#define ESPNOW_CHANNEL 6
#define HEARTBEAT_ID   255

CRGB leds[NUM_LEDS];

// ================= LED STATE =================
uint16_t active_leds = NUM_LEDS;
uint8_t brightness = 127;
uint8_t h = 84, s = 255, v = 255;
uint8_t LEDmode = 1;

// ================= CONNECTION =================
unsigned long lastReceiveTime = 0;
bool connected = false;

// ================= DATA =================
typedef struct __attribute__((packed)) {
  int id;
  int valuez;
  uint32_t timestamp;
} test_struct;

// ================= ESP-NOW RX =================
void OnDataRecv(uint8_t *, uint8_t *data, uint8_t len) {
  test_struct pkt;
  memcpy(&pkt, data, sizeof(pkt));

  lastReceiveTime = millis();
  connected = true;

  if (pkt.id == HEARTBEAT_ID) return;

  switch (pkt.id) {
    case 0: LEDmode = 0; break;
    case 1: LEDmode = 1; break;
    case 2: active_leds = constrain(pkt.valuez, 1, NUM_LEDS); break;
    case 4: brightness = constrain(pkt.valuez, 0, 255); FastLED.setBrightness(brightness); break;
    case 6: h = constrain(pkt.valuez, 0, 255); break;
    case 7: s = constrain(pkt.valuez, 0, 255); break;
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  wifi_set_channel(ESPNOW_CHANNEL);

  if (esp_now_init() != 0) {
    ESP.restart();
  }

  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(brightness);
  FastLED.clear();
  FastLED.show();

  lastReceiveTime = millis();
  Serial.println("=== ESP8266 SLAVE READY ===");
}

// ================= LOOP =================
void loop() {
  if (millis() - lastReceiveTime > 3000) {
    connected = false;
  }

  switch (LEDmode) {
    case 0:
      fadeToBlackBy(leds, active_leds, 20);
      break;

    case 1:
      for (uint16_t i = 0; i < active_leds; i++) {
        leds[i] = CHSV(h, s, v);
      }
      break;
  }

  FastLED.show();
  yield();
}
