#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "SNDT123";
const char* password = "sndt@123=\\";

// ========== Pins ==========
#define MQ_PIN          A0          // Gas / PM sensor
#define DT_PIN          D2          // HX710B Data
#define SCK_PIN         D1          // HX710B Clock

#define RELAY_FAN1      D5          // GPIO14 - Ventilation Fan
#define RELAY_FAN2      D6          // GPIO12 - Cleaning Fan
#define RELAY_SPRAY     D7          // GPIO13 - Spray pump
#define RELAY_REFILL    D0          // GPIO16 - Refill pump (Manual only)

// ========== Thresholds ==========
const int  gasThreshold      = 70;         // Adjust after testing
const long pressureThreshold = 1255215;    // Adjust after testing

AsyncWebServer server(80);

bool autoMode = true;

int  gasValue      = 0;
long pressureValue = 0;

String fan1Status  = "OFF";
String fan2Status  = "OFF";
String sprayStatus = "OFF";
String refillStatus = "OFF";

unsigned long lastSensorRead = 0;

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(RELAY_FAN1, OUTPUT);
  pinMode(RELAY_FAN2, OUTPUT);
  pinMode(RELAY_SPRAY, OUTPUT);
  pinMode(RELAY_REFILL, OUTPUT);

  // All relays OFF (Active LOW)
  digitalWrite(RELAY_FAN1, HIGH);
  digitalWrite(RELAY_FAN2, HIGH);
  digitalWrite(RELAY_SPRAY, HIGH);
  digitalWrite(RELAY_REFILL, HIGH);

  pinMode(DT_PIN, INPUT);
  pinMode(SCK_PIN, OUTPUT);
  digitalWrite(SCK_PIN, LOW);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // ===================== WEB INTERFACE =====================
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Air Quality Controller</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body{font-family:Arial;text-align:center;background:#0f1620;color:white;margin:0;padding:16px;}
    h1{color:#4fc3f7;}
    .card{background:#1e2a44;margin:12px auto;padding:16px;border-radius:12px;max-width:500px;}
    .val{font-size:1.6em;font-weight:bold;color:#4fc3f7;}
    button{padding:12px 22px;margin:6px;font-size:16px;border:none;border-radius:8px;cursor:pointer;min-width:120px;}
    .on{background:#2e7d32;} .off{background:#c62828;} .mode{background:#1565c0;}
    .status{font-size:18px;margin:8px 0;}
  </style>
</head>
<body>
  <h1>Air Quality + Cleaning System</h1>

  <div class="card">
    <div>Gas / PM : <span class="val" id="gas">0</span></div>
    <div>Pressure : <span class="val" id="press">0</span></div>
  </div>

  <div class="card">
    <div class="status">Mode: <span id="auto">AUTO</span></div>
    <button class="mode" onclick="toggleAuto()">Toggle Auto / Manual</button>
  </div>

  <div class="card">
    <h3>Fan 1 (Ventilation)</h3>
    <div class="status" id="f1">OFF</div>
    <button class="on" onclick="ctrl('fan1',1)">ON</button>
    <button class="off" onclick="ctrl('fan1',0)">OFF</button>
  </div>

  <div class="card">
    <h3>Fan 2 (Cleaning)</h3>
    <div class="status" id="f2">OFF</div>
    <button class="on" onclick="ctrl('fan2',1)">ON</button>
    <button class="off" onclick="ctrl('fan2',0)">OFF</button>
  </div>

  <div class="card">
    <h3>Spray Pump</h3>
    <div class="status" id="spray">OFF</div>
    <button class="on" onclick="ctrl('spray',1)">ON</button>
    <button class="off" onclick="ctrl('spray',0)">OFF</button>
  </div>

  <div class="card">
    <h3>Refill Pump (Manual Only)</h3>
    <div class="status" id="refill">OFF</div>
    <button class="on" onclick="ctrl('refill',1)">ON</button>
    <button class="off" onclick="ctrl('refill',0)">OFF</button>
  </div>

  <script>
    function ctrl(device, state) {
      fetch(`/control?device=${device}&state=${state}`);
    }
    function toggleAuto() {
      fetch('/auto');
    }

    setInterval(() => {
      fetch('/data').then(r => r.text()).then(d => {
        let v = d.split(',');
        document.getElementById('gas').innerText   = v[0];
        document.getElementById('press').innerText = v[1];
        document.getElementById('f1').innerText    = v[2];
        document.getElementById('f2').innerText    = v[3];
        document.getElementById('spray').innerText = v[4];
        document.getElementById('refill').innerText= v[5];
        document.getElementById('auto').innerText  = v[6];
      });
    }, 1000);
  </script>
</body>
</html>
    )rawliteral";
    request->send(200, "text/html", html);
  });

  // Manual control for all 4 devices
  server.on("/control", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("device") && request->hasParam("state")) {
      String device = request->getParam("device")->value();
      int state = request->getParam("state")->value().toInt();

      if (device == "fan1")   digitalWrite(RELAY_FAN1,   state ? LOW : HIGH);
      if (device == "fan2")   digitalWrite(RELAY_FAN2,   state ? LOW : HIGH);
      if (device == "spray")  digitalWrite(RELAY_SPRAY,  state ? LOW : HIGH);
      if (device == "refill") digitalWrite(RELAY_REFILL, state ? LOW : HIGH);
    }
    request->send(200);
  });

  server.on("/auto", HTTP_GET, [](AsyncWebServerRequest *request){
    autoMode = !autoMode;
    request->send(200);
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    String resp = String(gasValue) + "," +
                  String(pressureValue) + "," +
                  fan1Status + "," +
                  fan2Status + "," +
                  sprayStatus + "," +
                  refillStatus + "," +
                  (autoMode ? "AUTO" : "MANUAL");
    request->send(200, "text/plain", resp);
  });

  server.begin();
  Serial.println("Web Server Started");
}

void loop() {
  if (millis() - lastSensorRead >= 500) {
    lastSensorRead = millis();

    gasValue      = getGasAverage();
    pressureValue = readHX710BRaw();

    if (autoMode) {
      // ----- Gas / PM control -----
      if (gasValue > gasThreshold) {
        digitalWrite(RELAY_FAN1, LOW);     // Fan1 ON
        digitalWrite(RELAY_SPRAY, HIGH);   // Spray OFF
        fan1Status  = "ON (High Gas)";
        sprayStatus = "OFF";
      } else {
        digitalWrite(RELAY_FAN1, HIGH);    // Fan1 OFF
        digitalWrite(RELAY_SPRAY, LOW);    // Spray ON
        fan1Status  = "OFF";
        sprayStatus = "ON (Low Gas)";
      }

      // ----- Pressure → Cleaning Fan -----
      if (pressureValue > pressureThreshold) {
        digitalWrite(RELAY_FAN2, LOW);
        fan2Status = "ON (High Pressure)";
      } else {
        digitalWrite(RELAY_FAN2, HIGH);
        fan2Status = "OFF";
      }
    }
    else {
      // Manual mode - only update status text
      fan1Status   = (digitalRead(RELAY_FAN1)  == LOW) ? "ON (Manual)" : "OFF (Manual)";
      fan2Status   = (digitalRead(RELAY_FAN2)  == LOW) ? "ON (Manual)" : "OFF (Manual)";
      sprayStatus  = (digitalRead(RELAY_SPRAY) == LOW) ? "ON (Manual)" : "OFF (Manual)";
    }

    // Refill is always manual
    refillStatus = (digitalRead(RELAY_REFILL) == LOW) ? "ON (Manual)" : "OFF";
  }
  yield();
}

// ==================== Helper Functions ====================
int getGasAverage() {
  long sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(MQ_PIN);
    delayMicroseconds(500);
  }
  return sum / 10;
}

long readHX710BRaw() {
  long value = 0;
  unsigned long timeout = millis();
  while (digitalRead(DT_PIN) == HIGH) {
    if (millis() - timeout > 100) return pressureValue;
  }

  for (int i = 0; i < 24; i++) {
    digitalWrite(SCK_PIN, HIGH);
    value = (value << 1) | digitalRead(DT_PIN);
    digitalWrite(SCK_PIN, LOW);
  }
  digitalWrite(SCK_PIN, HIGH);
  digitalWrite(SCK_PIN, LOW);

  if (value & 0x800000) value |= ~0xFFFFFF;
  return value;
}