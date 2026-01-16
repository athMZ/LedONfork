#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please enable Bluetooth in the ESP32 build
#endif

BluetoothSerial SerialBT;

// ====== CONFIGURATION ======
#define ESPNOW_CHANNEL 6

// ====== ESP-NOW PEERS ======
// Slaves MAC Addresses
uint8_t broadcastAddress1[] = {0xE8, 0xDB, 0x84, 0xDD, 0x45, 0x94}; 
uint8_t broadcastAddress2[] = {0xE8, 0xDB, 0x84, 0xDD, 0x53, 0xA1}; 

// ====== DATA STRUCTURE ======
// Must match the slave structure exactly
typedef struct __attribute__((packed)) {
  int id;
  int valuez;
  uint32_t timestamp;
} test_struct;

test_struct test;

// ====== GLOBALS ======
int incoming;
int id = -1;
int val_byte1 = -1;
int val_byte2 = -1;
bool newData = false;

// ====== FORWARD DECLARATIONS ======
bool initESPNow();
void reset_rx_BT();

// ====== CALLBACKS ======

void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    // Serial.println(">> Packet Sent Successfully");
  } else {
    // Serial.println(">> Packet Delivery Failed");
  }
}

// ====== SETUP ======
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== ESP32 MASTER STARTING ===");

  // 1. Init WiFi in Station Mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  // 2. Set Channel (CRITICAL: Must match Slave Channel)
  esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);
  
  Serial.print("WiFi Channel set to: ");
  Serial.println(ESPNOW_CHANNEL);
  
  // 3. Init Bluetooth
  SerialBT.begin("ESP32_LEDoN");
  Serial.println("Bluetooth Started! Connect to 'ESP32_LEDoN'");

  // 4. Init ESP-NOW
  if (initESPNow()) {
    Serial.println("ESP-NOW Initialized Successfully");
  } else {
    Serial.println("ESP-NOW Init FAILED");
    ESP.restart();
  }
}

// ====== LOOP ======
void loop() {
  // Check for Bluetooth Data
  if (SerialBT.available()) {
    incoming = SerialBT.read();
    
    // Protocol Decoder
    if (incoming > 127) {
      // New Command Start (ID detected)
      reset_rx_BT();
      id = incoming - 128;
    }
    else if (val_byte1 == -1) {
      val_byte1 = incoming;
    }
    else if (val_byte2 == -1) {
      val_byte2 = incoming;
      
      // Full message received
      int value = 128 * val_byte1 + val_byte2;
      
      test.id = id;
      test.valuez = value;
      test.timestamp = millis(); // Update timestamp
      
      Serial.printf("BT Recv: ID=%d Val=%d -> Sending to Slaves...\n", test.id, test.valuez);
      
      esp_err_t result = esp_now_send(NULL, (uint8_t *) &test, sizeof(test));
      
      if (result == ESP_OK) {
        Serial.println("ESP-NOW Send Requested");
      } else {
        Serial.print("ESP-NOW Send Error: ");
        Serial.println(result);
      }
      
      // Reset for next message
      reset_rx_BT();
    }
  }
  
  delay(10); // Stability delay
}

bool initESPNow() {
  if (esp_now_init() != ESP_OK) {
    return false;
  }
  
  // Register Send Callback
  esp_now_register_send_cb(OnDataSent);
  
  // Register Peers
  esp_now_peer_info_t peerInfo = {};
  peerInfo.channel = ESPNOW_CHANNEL;  
  peerInfo.encrypt = false;
  
  // Add Peer 1
  memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add Peer 1");
  } else {
    Serial.println("Peer 1 Added");
  }
  
  // Add Peer 2
  memcpy(peerInfo.peer_addr, broadcastAddress2, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add Peer 2");
  } else {
    Serial.println("Peer 2 Added");
  }
  
  return true;
}

void reset_rx_BT() {
  id = -1;
  val_byte1 = -1;
  val_byte2 = -1;
}
