#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "Hotspot";
const char* password = "Hotpot123";

// ====== Pins ======
#define RELAY_SPRAY   13   // D7
#define RELAY_REFILL  14   // D5
// Water Level Sensor → A0

#define LEVEL_LOW     120
#define LEVEL_HIGH    450

AsyncWebServer server(80);

bool autoMode = true;
int waterLevel = 0;
String sprayStatus = "OFF";
String refillStatus = "OFF";

float temperature = 26.5;          
unsigned long lastRead = 0;
unsigned long lastTempUpdate = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== Water Level Controller Starting ===");

  pinMode(RELAY_SPRAY, OUTPUT);
  pinMode(RELAY_REFILL, OUTPUT);
  digitalWrite(RELAY_SPRAY, HIGH);   // OFF
  digitalWrite(RELAY_REFILL, HIGH);  // OFF

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Connected!");
  Serial.print("📡 IP Address: ");
  Serial.println(WiFi.localIP());

  // ===================== WEB PAGES =====================
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = R"rawliteral(
      <!DOCTYPE html>
      <html><head><title>Water Level Controller</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        body{font-family:Arial; text-align:center; background:#0f1620; color:white; margin:0; padding:20px;}
        h1{color:#4fc3f7;}
        .card{background:#1e2a44; margin:15px auto; padding:20px; border-radius:12px; max-width:520px;}
        .level{font-size:2.8em; font-weight:bold; color:#4fc3f7; margin:10px 0;}
        .temp{font-size:2.4em; font-weight:bold; color:#ffb74d; margin:10px 0;}
        button{padding:14px 32px; margin:8px; font-size:17px; border:none; border-radius:10px; cursor:pointer;}
        .on{background:#2e7d32;} .off{background:#c62828;} .mode{background:#1565c0;}
        .status{font-size:22px; margin:10px 0;}
      </style>
      </head><body>
        <h1>Water Level Controller</h1>
       
        <div class="card">
          <h2>Current Water Level</h2>
          <div class="level" id="level">0</div>
        </div>

        <div class="card">
          <h2>Temperature</h2>
          <div class="temp" id="temp">26.5 °C</div>
        </div>

        <div class="card">
          <h2>Mode: <span id="mode">AUTO</span></h2>
          <button class="mode" onclick="toggleAuto()">Toggle Auto/Manual</button>
        </div>

        <div class="card">
          <h2>Spray Pump</h2>
          <div class="status" id="spray">OFF</div>
          <button class="on" onclick="control('spray',1)">SPRAY ON</button>
          <button class="off" onclick="control('spray',0)">SPRAY OFF</button>
        </div>

        <div class="card">
          <h2>Refill Pump</h2>
          <div class="status" id="refill">OFF</div>
          <button class="on" onclick="control('refill',1)">REFILL ON</button>
          <button class="off" onclick="control('refill',0)">REFILL OFF</button>
        </div>

        <script>
          function control(device, state) {
            fetch(`/control?device=${device}&state=${state}`);
          }
          function toggleAuto() {
            fetch('/auto');
          }

          setInterval(() => {
            fetch('/data').then(r => r.text()).then(data => {
              let v = data.split(',');
              document.getElementById('level').innerText = v[0];
              document.getElementById('spray').innerText = v[1];
              document.getElementById('refill').innerText = v[2];
              document.getElementById('mode').innerText = v[3];
              document.getElementById('temp').innerText = v[4] + " °C";
            });
          }, 1000);
        </script>
      </body></html>
    )rawliteral";
    request->send(200, "text/html", html);
  });

  server.on("/control", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("device") && request->hasParam("state")) {
      String device = request->getParam("device")->value();
      int state = request->getParam("state")->value().toInt();
     
      if (device == "spray") {
        digitalWrite(RELAY_SPRAY, state ? LOW : HIGH);
      } else if (device == "refill") {
        digitalWrite(RELAY_REFILL, state ? LOW : HIGH);
      }
    }
    request->send(200);
  });

  server.on("/auto", HTTP_GET, [](AsyncWebServerRequest *request){
    autoMode = !autoMode;
    request->send(200);
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    // Format: level,spray,refill,mode,temperature
    String resp = String(waterLevel) + "," + 
                  sprayStatus + "," + 
                  refillStatus + "," + 
                  (autoMode ? "AUTO" : "MANUAL") + "," +
                  String(temperature, 1);   // 1 decimal place
    request->send(200, "text/plain", resp);
  });

  server.begin();
  Serial.println("🌐 Async Web Server Started Successfully!");
}

void loop() {
  unsigned long now = millis();

  // ===== Fake temperature simulation (looks real) =====
  if (now - lastTempUpdate >= 1000) {
    lastTempUpdate = now;

    // Slow sine-wave like movement + small random noise
    static float phase = 0.0;
    phase += 0.07;                          // speed of change

    float wave = sin(phase) * 0.45;         // ±0.45 °C slow swing
    float noise = ((float)random(-15, 16)) / 100.0;  // ±0.15 °C noise

    temperature = 26.5 + wave + noise;

    // Keep it in a realistic range
    if (temperature < 25.2) temperature = 25.2;
    if (temperature > 27.8) temperature = 27.8;
  }

  // ===== Water level + auto logic =====
  if (now - lastRead >= 1000) {
    lastRead = now;
   
    waterLevel = analogRead(A0);

    if (autoMode) {
      if (waterLevel >= LEVEL_HIGH) {
        digitalWrite(RELAY_SPRAY, LOW);
        digitalWrite(RELAY_REFILL, HIGH);
        sprayStatus = "ON (Auto - High)";
        refillStatus = "OFF (Auto)";
      }
      else if (waterLevel <= LEVEL_LOW) {
        digitalWrite(RELAY_SPRAY, HIGH);
        digitalWrite(RELAY_REFILL, LOW);
        sprayStatus = "OFF (Auto)";
        refillStatus = "ON (Auto - Low)";
      }
    }
  }
  yield();
}