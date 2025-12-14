#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please enable Bluetooth in the ESP32 build
#endif

BluetoothSerial SerialBT;

// ====== ESP-NOW PEERS ======
uint8_t broadcastAddress1[] = {0xFC, 0xF5, 0xC4, 0x96, 0x68, 0x0E};
uint8_t broadcastAddress2[] = {0xD8, 0xF1, 0x5B, 0x11, 0x5A, 0x66};

// ====== DATA ======
int incoming;
int id = -1;
int val_byte1 = -1;
int val_byte2 = -1;
bool newData = false;

typedef struct __attribute__((packed)) {
  int id;
  int valuez;
} test_struct;

test_struct test;

// ====== ESP-NOW SEND CALLBACK (NEW API) ======
void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  // info->mac_addr is the destination MAC (if you need it)

  /*
  Serial.print("ESP-NOW send status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
  */
}

// ====== SETUP ======
void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);

  SerialBT.begin("ESP32test");
  Serial.println("Bluetooth ready");

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo = {};
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Peer 1
  memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer 1");
  }

  // Peer 2
  memcpy(peerInfo.peer_addr, broadcastAddress2, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer 2");
  }

  Serial.println("ESP-NOW ready");
}

// ====== LOOP ======
void loop() {
  incomingData();
  showNewNumber();
}

// ====== BLUETOOTH RX ======
void incomingData() {
  if (!SerialBT.available()) return;

  incoming = SerialBT.read();

  if (incoming > 127) {
    reset_rx_BT();
    id = incoming - 128;
  }
  else if (val_byte1 == -1) {
    val_byte1 = incoming;
  }
  else if (val_byte2 == -1) {
    val_byte2 = incoming;

    int value = 128 * val_byte1 + val_byte2;

    test.id = id;
    test.valuez = value;

    esp_now_send(NULL, (uint8_t *)&test, sizeof(test));

    newData = true;
  }
}

// ====== DEBUG ======
void showNewNumber() {
  if (!newData) return;

  Serial.print("ID: ");
  Serial.print(id);
  Serial.print("  Value: ");
  Serial.println(test.valuez);

  newData = false;
  reset_rx_BT();
}

// ====== RESET RX STATE ======
void reset_rx_BT() {
  id = -1;
  val_byte1 = -1;
  val_byte2 = -1;
}
