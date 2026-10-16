#include <WiFi.h>
#include <WebServer.h>

// ─── إعدادات الـ Wi-Fi (تم إخفاء البيانات الحساسة للرفع) ───────────────────
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// ─── تحديد البينات (Pin Definitions) ──────────────────────────────────────
const int trigPin   = 14;
const int echoPin   = 25;
const int redLed    = 26;
const int yellowLed = 27;
const int greenLed  = 33;
const int buzzer    = 32;

// ─── أبعاد الخزان (بالسم) ─────────────────────────────────────────────────
const int distFull  = 15;   // المسافة عندما يكون الخزان ممتلئاً
const int distEmpty = 75;   // المسافة عندما يكون الخزان فارغاً

WebServer server(80);

int waterLevel     = 0;
int lastWaterLevel = 0;
String statusStr      = "";
String alertStr       = "";
String refillTimeStr  = "No Refill Detected Yet";

unsigned long lastRefillMillis = 0;
bool firstRefill = false;

unsigned long lastReadTime = 0;
const unsigned long interval = 1000;

// ─── حساب الوقت بصيغة HH:MM:SS ───────────────────────────────────────────
String formatUptime(unsigned long ms) {
  unsigned long totalSeconds = ms / 1000;
  int s = totalSeconds % 60;
  int m = (totalSeconds / 60) % 60;
  int h = totalSeconds / 3600;

  char buf[12];
  sprintf(buf, "%02d:%02d:%02d", h, m, s);
  return String(buf);
}

// ─── حساب الوقت الفعلي منذ آخر إعادة ملء ─────────────────────────────────
String timeSinceRefill() {
  if (!firstRefill) return "No Refill Detected Yet";
  unsigned long elapsed = millis() - lastRefillMillis;
  return formatUptime(elapsed);
}

// ─── قياس المسافة بالسونار ────────────────────────────────────────────────
float getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  if (duration == 0) return -1;
  return (duration * 0.034) / 2;
}

// ─── التحكم بالـ LEDs والبازر ─────────────────────────────────────────────
void controlPeripherals(int percent) {
  digitalWrite(redLed,    LOW);
  digitalWrite(yellowLed, LOW);
  digitalWrite(greenLed,  LOW);
  noTone(buzzer);

  if      (percent >= 98) { digitalWrite(greenLed,  HIGH); tone(buzzer, 2000, 200); }
  else if (percent >= 75) { digitalWrite(greenLed,  HIGH); }
  else if (percent >= 25) { digitalWrite(yellowLed, HIGH); }
  else                    { digitalWrite(redLed,    HIGH); tone(buzzer, 1000, 400); }
}

// ─── صفحة الويب ───────────────────────────────────────────────────────────
void handleWebPage() {
  String barColor = (waterLevel >= 75) ? "#2ecc71" :
                    (waterLevel >= 25) ? "#f39c12" : "#e74c3c";

  String html = R"rawhtml(
<!DOCTYPE html>
<html lang='ar'>
<head>
  <meta charset='UTF-8'>
  <meta http-equiv='refresh' content='3'>
  <title>Water Tank Monitor</title>
  <style>
    body { font-family: 'Segoe UI', sans-serif; background: #eaf4fb;
           display: flex; flex-direction: column;
           align-items: center; padding: 40px 10px; margin: 0; }
    .card { background: white; border-radius: 16px; padding: 30px 40px;
            box-shadow: 0 4px 20px rgba(0,0,0,0.1); width: 380px; }
    h2 { margin-top: 0; color: #2c3e50; }
    .badge { font-size: 0.75rem; background: #3498db; color: white;
             border-radius: 20px; padding: 2px 10px; margin-left: 6px; }
    .divider { border: none; border-top: 1px solid #ddd; margin: 14px 0; }
    .row { display: flex; justify-content: space-between;
           align-items: center; margin: 10px 0; }
    .label { color: #7f8c8d; font-size: 0.95rem; }
    .value { font-weight: bold; color: #2c3e50; font-size: 1rem; }
    .bar-bg { background: #ecf0f1; border-radius: 10px;
              height: 22px; width: 100%; margin: 14px 0; overflow: hidden; }
    .bar-fill { height: 100%; border-radius: 10px;
                transition: width 0.4s ease; }
    .alert-box { margin-top: 14px; border-radius: 10px; padding: 10px 14px;
                 font-weight: bold; font-size: 0.95rem; }
    .alert-warn { background: #fdecea; color: #c0392b; }
    .alert-ok   { background: #eafaf1; color: #1e8449; }
    .refill-box { margin-top: 14px; background: #f0f4ff; border-left: 4px solid #3498db;
                  border-radius: 8px; padding: 10px 14px; }
    .refill-title { font-size: 0.82rem; color: #2980b9; margin-bottom: 4px; }
    .refill-val   { font-size: 1.3rem; font-weight: bold; color: #154360; font-family: monospace; }
  </style>
</head>
<body>
<div class='card'>
  <h2>💧 Water Tank Monitor <span class='badge'>Student Project</span></h2>
  <hr class='divider'>

  <div class='row'>
    <span class='label'>Water Level</span>
    <span class='value'>)rawhtml";

  html += String(waterLevel);
  html += R"rawhtml(%</span>
  </div>
  <div class='bar-bg'>
    <div class='bar-fill' style='width:)rawhtml";
  html += String(waterLevel);
  html += "%; background:";
  html += barColor;
  html += R"rawhtml(;'></div>
  </div>

  <div class='row'>
    <span class='label'>Tank Depth (Empty)</span>
    <span class='value'>81.2 cm</span>
  </div>
  <div class='row'>
    <span class='label'>Full Level Distance</span>
    <span class='value'>15 cm</span>
  </div>

  <hr class='divider'>

  <div class='row'>
    <span class='label'>Tank Status</span>
    <span class='value'>)rawhtml";
  html += statusStr;
  html += R"rawhtml(</span>
  </div>

  <div class='alert-box )rawhtml";
  html += (alertStr.startsWith("Warning") ? "alert-warn" : "alert-ok");
  html += "'>";
  html += alertStr;
  html += R"rawhtml(</div>

  <div class='refill-box'>
    <div class='refill-title'>⏱ Time Since Last Refill</div>
    <div class='refill-val'>)rawhtml";
  html += timeSinceRefill();
  html += R"rawhtml(</div>
  </div>

</div>
</body>
</html>)rawhtml";

  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);

  pinMode(trigPin,   OUTPUT);
  pinMode(echoPin,   INPUT);
  pinMode(redLed,    OUTPUT);
  pinMode(yellowLed, OUTPUT);
  pinMode(greenLed,  OUTPUT);
  pinMode(buzzer,    OUTPUT);

  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  server.on("/", handleWebPage);
  server.begin();
}

void loop() {
  server.handleClient();

  if (millis() - lastReadTime >= interval) {
    lastReadTime = millis();

    float distance = getDistance();
    if (distance < 0 || distance > 200) {
      alertStr = "Sensor Error! Check Connections.";
      Serial.println("Sensor Error!");
      return;
    }

    int rawLevel = map((int)distance, distEmpty, distFull, 0, 100);
    waterLevel   = constrain(rawLevel, 0, 100);

    if (waterLevel > lastWaterLevel + 5) {
      lastRefillMillis = millis();   
      firstRefill = true;
    }
    lastWaterLevel = waterLevel;

    if      (waterLevel >= 98) { statusStr = "OVERFLOW!"; alertStr = "Warning: Tank is Overflowing!"; }
    else if (waterLevel >= 75) { statusStr = "Full";      alertStr = "System Normal"; }
    else if (waterLevel >= 25) { statusStr = "Medium";    alertStr = "System Normal"; }
    else                       { statusStr = "Low";       alertStr = "Warning: Water Level Low! Please Refill."; }

    Serial.printf("Distance: %.1f cm | Level: %d%%\n", distance, waterLevel);
    controlPeripherals(waterLevel);
  }
}