#include <ESP8266WiFi.h>
#include <espnow.h>
#include <FastLED.h>

#define NUM_LEDS 255
#define LED_PIN 5

CRGBArray<NUM_LEDS> leds;

uint8_t configured_leds = 20;
const uint8_t v = 255;
uint8_t h = 84;
uint8_t s = 255;
uint8_t brightness = 127;
int LEDmode = 0;

// CONNECTION MONITORING
unsigned long lastReceiveTime = 0;
const unsigned long RECEIVE_TIMEOUT = 30000;
unsigned long lastStatusReport = 0;
bool connectionActive = false;
int receiveCount = 0;

// Structure to receive data - MUST match sender
typedef struct test_struct {
  int id;
  int valuez;
  uint32_t timestamp;
} test_struct;

test_struct myData;

// ESP-NOW RECEIVE CALLBACK
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

// SETUP
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

// LOOP
void loop() {
  monitorConnection();
  runLEDMode();
  FastLED.show();
  yield();
}

// CONNECTION MONITORING
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

// LED MODE HANDLER - SIMPLIFIED FOR NOW
void runLEDMode() {
  switch (LEDmode) {
    case 0:
      // OFF - fade to black
      fadeToBlackBy(leds, configured_leds, 10);
      break;
      
    case 1:
      // Solid color
      for (int i = 0; i < configured_leds; i++) {
        leds[i] = CHSV(h, s, v);
      }
      break;
      
    case 2:
      // Reserved
      break;
      
    default:
      // For now, just show a cycling rainbow for unknown modes
      static uint8_t hue = 0;
      fill_rainbow(leds, configured_leds, hue++, 7);
      break;
  }
}
