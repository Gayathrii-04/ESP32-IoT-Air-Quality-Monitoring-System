
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

// -------- WIFI SETTINGS --------
// Enter your Wi-Fi hotspot name and password
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// -------- SENSOR PINS --------
#define DHTPIN 4
#define DHTTYPE DHT11
#define MQ135_PIN 34

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

// -------- SENSOR VALUES --------
float temperature = 0;
float humidity = 0;
float mqVoltage = 0;
int mqRaw = 0;
bool dhtValid = false;

unsigned long lastRead = 0;

// -------- DASHBOARD HTML --------
const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Smart Air Quality Monitoring</title>
<style>
body { margin:0; font-family:Arial,sans-serif; background:#f1f5f9; color:#172033; }
header { background:#123b5d; color:white; text-align:center; padding:22px 10px; }
header h2 { margin:0; }
main { max-width:1000px; margin:auto; padding:15px; }
.cards { display:grid; grid-template-columns:repeat(auto-fit,minmax(145px,1fr)); gap:12px; }
.card,.chart { background:white; padding:17px; border-radius:12px; box-shadow:0 2px 8px #00000012; margin-bottom:14px; }
.card h3 { font-size:14px; color:#64748b; }
.value { font-size:28px; font-weight:bold; }
.unit { font-size:14px; color:#64748b; }
.chart h3 { margin-top:0; }
canvas { width:100%; height:210px; display:block; }
.status { padding:12px; background:#dbeafe; border-radius:8px; margin:4px 0 16px; }
footer { text-align:center; color:#64748b; padding:15px; }
</style>
</head>
<body>
<header><h2>Smart Air Quality Monitoring</h2></header>
<main>
<div class="cards">
  <div class="card"><h3>Temperature</h3><div class="value"><span id="temp">--</span> <span class="unit">C</span></div></div>
  <div class="card"><h3>Humidity</h3><div class="value"><span id="hum">--</span> <span class="unit">%</span></div></div>
  <div class="card"><h3>MQ-135 Gas Reading</h3><div class="value" id="gas">--</div><div class="unit">ADC value (0-4095)</div></div>
  <div class="card"><h3>MQ-135 Voltage</h3><div class="value"><span id="volt">--</span> <span class="unit">V</span></div></div>
</div>
<div class="status"><b>Sensor status:</b> <span id="status">Waiting for data...</span>
<div class="unit" style="margin-top:6px">Relative sensor indication, not certified AQI or gas ppm.</div></div>
<div class="chart"><h3>Temperature Graph (C)</h3><canvas id="tempChart"></canvas></div>
<div class="chart"><h3>Humidity Graph (%)</h3><canvas id="humChart"></canvas></div>
<div class="chart"><h3>MQ-135 Gas Reading Graph</h3><canvas id="gasChart"></canvas></div>
</main>
<footer>Live readings update every 2 seconds</footer>
<script>
const historyData = { temp: [], hum: [], gas: [] };
const maxPoints = 30;

function drawGraph(id, values, fixedMax) {
  const canvas = document.getElementById(id);
  const width = canvas.clientWidth;
  if (width <= 0) return;
  const height = 210;
  const dpr = window.devicePixelRatio || 1;
  canvas.width = width * dpr;
  canvas.height = height * dpr;
  const ctx = canvas.getContext("2d");
  ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
  const left = 45, right = 10, top = 15, bottom = 25;
  const plotWidth = width - left - right;
  const plotHeight = height - top - bottom;
  ctx.clearRect(0, 0, width, height);
  ctx.font = "11px Arial";
  ctx.strokeStyle = "#dbe3ec";
  ctx.fillStyle = "#64748b";
  ctx.lineWidth = 1;
  let min = 0, max = 100;
  if (fixedMax !== null) {
    max = fixedMax;
  } else if (values.length > 0) {
    min = Math.min(...values);
    max = Math.max(...values);
    if (min === max) { min -= 1; max += 1; }
    const padding = (max - min) * 0.15;
    min = Math.max(0, min - padding);
    max += padding;
  }
  for (let i = 0; i <= 4; i++) {
    const y = top + plotHeight * i / 4;
    ctx.beginPath(); ctx.moveTo(left, y); ctx.lineTo(width - right, y); ctx.stroke();
    const label = max - (max - min) * i / 4;
    ctx.fillText(label.toFixed(0), 3, y + 4);
  }
  if (values.length < 2) {
    ctx.fillText("Collecting readings...", left + 10, height / 2);
    return;
  }
  ctx.strokeStyle = "#1677d2"; ctx.lineWidth = 2; ctx.beginPath();
  values.forEach((value, index) => {
    const x = left + plotWidth * index / (maxPoints - 1);
    const y = top + plotHeight - ((value - min) / (max - min)) * plotHeight;
    if (index === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
  });
  ctx.stroke();
}

async function updateData() {
  try {
    const response = await fetch("/data");
    if (!response.ok) throw new Error("Server error");
    const data = await response.json();
    document.getElementById("temp").textContent = data.temperature === null ? "--" : data.temperature.toFixed(1);
    document.getElementById("hum").textContent = data.humidity === null ? "--" : data.humidity.toFixed(1);
    document.getElementById("gas").textContent = data.mqRaw;
    document.getElementById("volt").textContent = data.mqVoltage.toFixed(3);
    document.getElementById("status").textContent = data.status;
    if (data.temperature !== null) historyData.temp.push(data.temperature);
    if (data.humidity !== null) historyData.hum.push(data.humidity);
    historyData.gas.push(data.mqRaw);
    for (const key in historyData) {
      if (historyData[key].length > maxPoints) historyData[key].shift();
    }
    drawGraph("tempChart", historyData.temp, null);
    drawGraph("humChart", historyData.hum, null);
    drawGraph("gasChart", historyData.gas, 4095);
  } catch (error) {
    document.getElementById("status").textContent = "Waiting for connection...";
  }
}
window.addEventListener("resize", () => {
  drawGraph("tempChart", historyData.temp, null);
  drawGraph("humChart", historyData.hum, null);
  drawGraph("gasChart", historyData.gas, 4095);
});
updateData();
setInterval(updateData, 2000);
</script>
</body>
</html>
)rawliteral";

// -------- READ SENSORS --------
void readSensors() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  dhtValid = !isnan(t) && !isnan(h);
  if (dhtValid) {
    temperature = t;
    humidity = h;
  }

  long total = 0;
  for (int i = 0; i < 10; i++) {
    total += analogRead(MQ135_PIN);
    delay(2);
  }
  mqRaw = total / 10;
  mqVoltage = analogReadMilliVolts(MQ135_PIN) / 1000.0;
}

// -------- SEND DASHBOARD DATA --------
void handleData() {
  String status;
  if (mqRaw < 1200) status = "Lower relative sensor reading";
  else if (mqRaw < 2400) status = "Moderate relative sensor reading";
  else status = "Higher relative sensor reading";

  String json = "{";
  json += "\"temperature\":";
  json += dhtValid ? String(temperature, 1) : "null";
  json += ",\"humidity\":";
  json += dhtValid ? String(humidity, 1) : "null";
  json += ",\"mqRaw\":" + String(mqRaw);
  json += ",\"mqVoltage\":" + String(mqVoltage, 3);
  json += ",\"status\":\"" + status + "\"";
  json += "}";

  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", json);
}

void handleRoot() {
  server.send_P(200, "text/html", webpage);
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  analogReadResolution(12);
  analogSetPinAttenuation(MQ135_PIN, ADC_11db);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 20000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected!");
    Serial.print("Open dashboard: http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWi-Fi connection failed.");
    Serial.println("Check Wi-Fi name and password, then reset.");
  }

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  server.handleClient();

  if (millis() - lastRead >= 2000) {
    lastRead = millis();
    readSensors();

    Serial.print("Temperature: ");
    if (dhtValid) {
      Serial.print(temperature, 1);
      Serial.print(" C | Humidity: ");
      Serial.print(humidity, 1);
      Serial.print(" %");
    } else {
      Serial.print("DHT11 read error");
    }
    Serial.print(" | MQ-135 ADC: ");
    Serial.print(mqRaw);
    Serial.print(" | Voltage: ");
    Serial.print(mqVoltage, 3);
    Serial.println(" V");
  }
}
