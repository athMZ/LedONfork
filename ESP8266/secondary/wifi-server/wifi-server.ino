#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>

// ================== WIFI ==================
const char* AP_SSID = "LED_MASTER";
const char* AP_PASS = "12345678";

// ================== SERVER ==================
ESP8266WebServer server(80);

// ================== UDP ==================
WiFiUDP udp;
const uint16_t UDP_PORT = 4210;
IPAddress broadcastIP(192,168,4,255);

// ================== PACKET ==================
#pragma pack(push,1)
struct ControlPacket {
  uint8_t  id;
  uint16_t value;
};
#pragma pack(pop)

void sendCommand(uint8_t id, uint16_t value) {
  ControlPacket pkt{ id, value };

  udp.beginPacket(broadcastIP, UDP_PORT);
  udp.write((uint8_t*)&pkt, sizeof(pkt));
  udp.endPacket();

  Serial.printf("CMD -> ID:%d VAL:%d\n", id, value);
}

// ================== HTML PAGE ==================
const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>LED Controller</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: sans-serif; text-align: center; }
    button { font-size: 18px; margin: 6px; padding: 14px 22px; }
  </style>
</head>
<body>
  <h2>ESP8266 LED Control</h2>

  <button onclick="send(0,0)">OFF</button>
  <button onclick="send(1,0)">SOLID</button>
  <button onclick="send(26,0)">MOVING DOT</button>
  <button onclick="send(47,0)">BALL</button>

  <br><br>
  <button onclick="send(4,50)">Dim</button>
  <button onclick="send(4,128)">Normal</button>
  <button onclick="send(4,255)">Bright</button>

  <script>
    function send(id,val){
      fetch(`/cmd?id=${id}&val=${val}`);
    }
  </script>
</body>
</html>
)rawliteral";

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println("\n=== ESP8266 LED MASTER (WEB) ===");

  // WiFi AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);

  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  // UDP
  udp.begin(UDP_PORT);

  // Web routes
  server.on("/", []() {
    server.send_P(200, "text/html", PAGE);
  });

  server.on("/cmd", []() {
    if (!server.hasArg("id") || !server.hasArg("val")) {
      server.send(400, "text/plain", "Missing args");
      return;
    }

    uint8_t id = server.arg("id").toInt();
    uint16_t val = server.arg("val").toInt();

    sendCommand(id, val);
    server.send(200, "text/plain", "OK");
  });

  server.begin();
  Serial.println("Web server started");
}

// ================== LOOP ==================
void loop() {
  server.handleClient();
}
