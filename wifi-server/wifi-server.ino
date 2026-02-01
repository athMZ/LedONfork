#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>

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
    body { font-family: sans-serif; background:#111; color:#eee; text-align:center; max-width: 600px; margin: 0 auto; }
    h2 { margin-top: 10px; }
    .group { border:1px solid #333; margin:10px; padding:10px; border-radius:8px; }
    button {
      font-size:14px;
      padding:10px;
      margin:4px;
      min-width:120px;
      cursor: pointer;
      background-color: #333;
      color: #eee;
      border: 1px solid #555;
      border-radius: 4px;
    }
    button:active { background-color: #555; }
    input[type=range] {
      width: 90%;
      cursor: pointer;
    }
    .color-preview {
        width: 100%;
        height: 40px;
        border: 1px solid #555;
        border-radius: 4px;
        margin-bottom: 15px;
        background-color: hsl(118, 100%, 50%); /* Initial default */
    }
  </style>
</head>

<body>
<h2>ESP8266 LED Controller</h2>

<div class="group">
  <button onclick="send(0,0)" style="background-color: #500;">OFF</button>
  <button onclick="send(1,0)">SOLID</button>
</div>

<div class="group">
    <h3>Configuration</h3>
    <label>Active LEDs: <span id="ledCountDisp">85</span></label><br>
    <input type="range" min="1" max="85" value="85" 
           oninput="document.getElementById('ledCountDisp').innerText=this.value; send(2,this.value)">
</div>

<div class="group">
  <h3>Color</h3>
  <div id="colorPreview" class="color-preview"></div>
  
  <label>Hue</label><br>
  <input id="hueSlider" type="range" min="0" max="255" value="84"
         oninput="updateColor(); send(6,this.value)">
  <br>
  <label>Saturation</label><br>
  <input id="satSlider" type="range" min="0" max="255" value="255"
         oninput="updateColor(); send(7,this.value)">
</div>

<div class="group">
  <h3>Brightness</h3>
  <input type="range" min="0" max="255" value="127"
         oninput="send(4,this.value)">
</div>

<div class="group">
  <h3>Basic Effects</h3>
  <button onclick="send(26,0)">Moving Dot</button>
  <button onclick="send(47,0)">Bouncing Ball</button>
  <button onclick="send(48,0)">Mirrored Ball</button>
</div>

<div class="group">
  <h3>Waves / Beats</h3>
  <button onclick="send(21,0)">Adding Waves</button>
  <button onclick="send(23,0)">Blur Phase Beat</button>
  <button onclick="send(24,0)">Brightness Waves</button>
  <button onclick="send(25,0)">Gradient Beat</button>
  <button onclick="send(27,0)">Phase Beat</button>
  <button onclick="send(28,0)">Rainbow Beat</button>
</div>

<div class="group">
  <h3>Noise / Fire</h3>
  <button onclick="send(31,0)">Raw Noise</button>
  <button onclick="send(32,0)">Fire</button>
  <button onclick="send(33,0)">Noise 8</button>
  <button onclick="send(34,0)">Noise Moving</button>
  <button onclick="send(35,0)">Lava</button>
</div>

<div class="group">
  <h3>Demo / Special</h3>
  <button onclick="send(41,0)">Fire FastLED</button>
  <button onclick="send(42,0)">Cylon</button>
  <button onclick="send(43,0)">Pacifica</button>
  <button onclick="send(44,0)">Pride</button>
  <button onclick="send(45,0)">TwinkleFox</button>
  <button onclick="send(46,0)">Demo Reel</button>
</div>

<div class="group">
  <h3>New Effects</h3>
  <button onclick="send(80,0)">Breathing</button>
  <button onclick="send(81,0)">Matrix Rain</button>
  <button onclick="send(82,0)">Police Strobe</button>
  <button onclick="send(83,0)">Color Wipe</button>
</div>

<script>
function send(id, val) {
  fetch(`/cmd?id=${id}&val=${val}`);
}

function updateColor() {
    var h = document.getElementById('hueSlider').value;
    var s = document.getElementById('satSlider').value;
    
    // FastLED Hue 0-255 -> CSS HSL 0-360
    var deg = Math.floor(h * 360 / 255);
    // FastLED Sat 0-255 -> CSS HSL 0-100%
    var sat = Math.floor(s * 100 / 255);
    
    var colorString = 'hsl(' + deg + ', ' + sat + '%, 50%)';
    document.getElementById('colorPreview').style.backgroundColor = colorString;
}

// Init color on load
updateColor();
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

  MDNS.begin("led");
  Serial.println("APP URL: http://led.local");
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
  MDNS.update();
}
