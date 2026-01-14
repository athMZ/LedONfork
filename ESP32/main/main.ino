#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// ================= CONFIG =================
#define ESPNOW_CHANNEL 6
#define HEARTBEAT_ID   255

uint8_t peer1[] = {0xFC, 0xF5, 0xC4, 0x96, 0x88, 0x26};
uint8_t peer2[] = {0xD8, 0xF1, 0x5B, 0x11, 0x5A, 0x66};

// ================= DATA =================
typedef struct __attribute__((packed)) {
  int id;
  int valuez;
  uint32_t timestamp;
} test_struct;

test_struct packet;

// ================= STATE =================
unsigned long lastBTActivity = 0;
unsigned long lastHeartbeat  = 0;
bool newData = false;

int incoming;
int id = -1;
int v1 = -1;
int v2 = -1;

// ================= SEND CALLBACK =================
void OnDataSent(const wifi_tx_info_t *, esp_now_send_status_t status) {
  if (status != ESP_NOW_SEND_SUCCESS) {
    Serial.println("[WARN] ESP-NOW send failed");
  }
}

// ================= INIT ESP-NOW =================
void initESPNow() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_ps(WIFI_PS_NONE);
  esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("[ERR] ESP-NOW init failed");
    ESP.restart();
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peer = {};
  peer.channel = ESPNOW_CHANNEL;
  peer.encrypt = false;

  memcpy(peer.peer_addr, peer1, 6);
  esp_now_add_peer(&peer);

  memcpy(peer.peer_addr, peer2, 6);
  esp_now_add_peer(&peer);

  Serial.println("[OK] ESP-NOW ready");
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_LEDoN");

  initESPNow();
  lastBTActivity = millis();

  Serial.println("=== ESP32 MASTER READY ===");
}

// ================= BLUETOOTH RX =================
void handleBluetooth() {
  if (!SerialBT.available()) return;

  lastBTActivity = millis();
  incoming = SerialBT.read();

  if (incoming > 127) {
    id = incoming - 128;
    v1 = v2 = -1;
  } 
  else if (v1 == -1) v1 = incoming;
  else {
    v2 = incoming;
    packet.id = id;
    packet.valuez = v1 * 128 + v2;
    packet.timestamp = millis();
    esp_now_send(NULL, (uint8_t*)&packet, sizeof(packet));
    id = v1 = v2 = -1;
  }
}

// ================= HEARTBEAT =================
void sendHeartbeat() {
  if (millis() - lastHeartbeat < 1000) return;

  packet.id = HEARTBEAT_ID;
  packet.valuez = 0;
  packet.timestamp = millis();
  esp_now_send(NULL, (uint8_t*)&packet, sizeof(packet));

  lastHeartbeat = millis();
}

// ================= LOOP =================
void loop() {
  handleBluetooth();
  sendHeartbeat();
  delay(1);
}
