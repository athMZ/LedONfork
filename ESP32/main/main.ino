#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please enable Bluetooth in the ESP32 build
#endif

BluetoothSerial SerialBT;

// ====== ESP-NOW PEERS ======
uint8_t broadcastAddress1[] = {0xE8, 0xDB, 0x84, 0xDD, 0x45, 0x94}; //E8:DB:84:DD:45:94
uint8_t broadcastAddress2[] = {0xE8, 0xDB, 0x84, 0xDD, 0x53, 0xA1}; //E8:DB:84:DD:53:A1

// ====== CONNECTION MONITORING ======
unsigned long lastSendTime = 0;
unsigned long lastBTActivity = 0;
const unsigned long SEND_TIMEOUT = 5000;  // 5 seconds
const unsigned long BT_TIMEOUT = 30000;   // 30 seconds
bool peer1Connected = false;
bool peer2Connected = false;
int sendFailCount = 0;
const int MAX_FAIL_COUNT = 3;

// ====== DATA ======
int incoming;
int id = -1;
int val_byte1 = -1;
int val_byte2 = -1;
bool newData = false;

typedef struct __attribute__((packed)) {
  int id;
  int valuez;
  uint32_t timestamp;  // For tracking message age
} test_struct;

test_struct test;

// ====== ESP-NOW SEND CALLBACK (Compatible with ESP32 Core 3.x) ======
void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  lastSendTime = millis();
  
  if (status == ESP_NOW_SEND_SUCCESS) {
    sendFailCount = 0;
    
    // Check which peer acknowledged using info->src_addr
    if (memcmp(info->src_addr, broadcastAddress1, 6) == 0) {
      peer1Connected = true;
    } else if (memcmp(info->src_addr, broadcastAddress2, 6) == 0) {
      peer2Connected = true;
    }
    
    Serial.println("YES Send success");
  } else {
    sendFailCount++;
    Serial.print("Send failed (");
    Serial.print(sendFailCount);
    Serial.println("/3)");
    
    // Check which peer failed
    if (memcmp(info->src_addr, broadcastAddress1, 6) == 0) {
      peer1Connected = false;
    } else if (memcmp(info->src_addr, broadcastAddress2, 6) == 0) {
      peer2Connected = false;
    }
    
    if (sendFailCount >= MAX_FAIL_COUNT) {
      Serial.println("Too many failures, reinitializing ESP-NOW...");
      initESPNow();
      sendFailCount = 0;
    }
  }
}

// ====== ESP-NOW INITIALIZATION ======
bool initESPNow() {
  // Deinit if already initialized
  esp_now_deinit();
  delay(100);
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return false;
  }
  
  esp_now_register_send_cb(OnDataSent);
  
  esp_now_peer_info_t peerInfo = {};
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  
  // Add Peer 1
  memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer 1");
  } else {
    Serial.println("Peer 1 added");
  }
  
  // Add Peer 2
  memcpy(peerInfo.peer_addr, broadcastAddress2, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer 2");
  } else {
    Serial.println("Peer 2 added");
  }
  
  return true;
}

// ====== SETUP ======
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== ESP32 LED Controller ===");
  Serial.print("ESP32 Core Version: ");
  Serial.println(ESP_ARDUINO_VERSION_MAJOR);
  
  // Configure WiFi for better coexistence with Bluetooth
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  // Reduce WiFi power to minimize interference with BT
  esp_wifi_set_ps(WIFI_PS_NONE);  // Disable power saving
  
  // Set TX power (for ESP32 Core 3.x, values are in 0.25dBm units)
  // Range: 8 to 84 (2dBm to 21dBm), default is ~80 (20dBm)
  esp_wifi_set_max_tx_power(52);  // ~13dBm - reduced for BT coexistence
  
  // Initialize Bluetooth
  SerialBT.begin("ESP32_LEDoN");
  Serial.println("Bluetooth ready: ESP32_LEDoN");
  
  // Initialize ESP-NOW
  if (initESPNow()) {
    Serial.println("ESP-NOW ready");
  } else {
    Serial.println("ESP-NOW initialization failed!");
  }
  
  lastBTActivity = millis();
  Serial.println("=== Setup Complete ===\n");
}

// ====== LOOP ======
void loop() {
  incomingData();
  showNewNumber();
  monitorConnections();
  
  // Small delay to prevent watchdog issues
  delay(1);
}

// ====== BLUETOOTH RX ======
void incomingData() {
  if (!SerialBT.available()) return;
  
  lastBTActivity = millis();
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
    test.timestamp = millis();
    
    // Send to all peers (broadcast)
    esp_err_t result = esp_now_send(NULL, (uint8_t *)&test, sizeof(test));
    
    if (result != ESP_OK) {
      Serial.print("ESP-NOW send error: ");
      Serial.println(result);
      
      // Try to recover
      if (result == ESP_ERR_ESPNOW_NOT_INIT) {
        Serial.println("ESP-NOW not initialized, reinitializing...");
        initESPNow();
      }
    }
    
    newData = true;
  }
}

// ====== DEBUG ======
void showNewNumber() {
  if (!newData) return;
  
  Serial.print("ID: ");
  Serial.print(id);
  Serial.print("  Value: ");
  Serial.print(test.valuez);
  Serial.print("  [P1:");
  Serial.print(peer1Connected ? "OK" : "FAIL");
  Serial.print(" P2:");
  Serial.print(peer2Connected ? "OK" : "FAIL");
  Serial.println("]");
  
  newData = false;
  reset_rx_BT();
}

// ====== CONNECTION MONITORING ======
void monitorConnections() {
  static unsigned long lastCheck = 0;
  unsigned long now = millis();
  
  // Check every 5 seconds
  if (now - lastCheck < 5000) return;
  lastCheck = now;
  
  // Check if Bluetooth is still connected
  if (SerialBT.hasClient()) {
    if (now - lastBTActivity > BT_TIMEOUT) {
      Serial.println("[WARNING] Bluetooth timeout - no activity");
    }
  } else {
    Serial.println("[WARNING] Bluetooth client disconnected");
  }
  
  // Reset peer connection flags periodically
  // They'll be set to true when we get successful send callbacks
  static unsigned long lastPeerReset = 0;
  if (now - lastSendTime > 15000) {
    peer1Connected = false;
    peer2Connected = false;
  }

  // Status report
  Serial.print("Status: BT[");
  Serial.print(SerialBT.hasClient() ? "OK" : "FAIL");
  Serial.print("] P1[");
  Serial.print(peer1Connected ? "OK" : "FAIL");
  Serial.print("] P2[");
  Serial.print(peer2Connected ? "OK" : "FAIL");
  Serial.println("]");
}

// ====== RESET RX STATE ======
void reset_rx_BT() {
  id = -1;
  val_byte1 = -1;
  val_byte2 = -1;
}
