/*
 * BẢNG GHÉP NỐI CHÂN (PIN MAPPING)
 *
 * GPIO | Linh kiện     | Loại    | Chức năng
 * ------|---------------|---------|------------------------------
 * 2    | Đèn báo nhiệt | OUTPUT  | LED cảnh báo nhiệt độ cao
 * 5    | DHT11         | INPUT   | Cảm biến nhiệt độ & độ ẩm
 * 18   | MQ-2          | INPUT   | Cảm biến khí gas (THẤP = có gas)
 * 34   | Cảm biến mức nước | INPUT | Cảm biến mức nước (analog)
 * 21   | LCD SDA       | I2C     | Dữ liệu I2C (LCD 0x27)
 * 22   | LCD SCL       | I2C     | Xung nhịp I2C (LCD 0x27)
 * 26   | Đèn           | OUTPUT  | Relay đèn
 * 27   | Quạt          | OUTPUT  | Relay quạt
 */

#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define LCD_ADDRESS 0x27
#define LCD_COLS 16
#define LCD_ROWS 2
#define GPIO_OUTPUT_26 26
#define GPIO_OUTPUT_27 27
#define GPIO_WATER_SENSOR 34
#define GPIO_GAS_SENSOR 18
#define GPIO_TEMP_LED 2
#define GPIO_DHT_SENSOR 5  // DHT11 data pin
#define DHTPIN GPIO_DHT_SENSOR
#define DHTTYPE DHT11
#define SERIAL_BAUD_RATE 115200
#define WIFI_CONNECT_TIMEOUT_MS 10000
#define WIFI_CONNECT_TIMEOUT_DEFAULT_MS 15000
#define WIFI_RETRY_DELAY_MS 500
#define SERIAL_COMMAND_DELAY_MS 10
#define TEMP_THRESHOLD 35.0
#define WATER_LEVEL_THRESHOLD 50
#define LCD_UPDATE_INTERVAL_MS 2000
#define HTTP_OK 200
#define HTTP_PORT 80
#define CHART_MAX_DATA_POINTS 20
#define CHART_UPDATE_INTERVAL_MS 5000
#define CHART_HUMIDITY_MIN 30
#define CHART_HUMIDITY_MAX 100
#define CHART_TEMPERATURE_MIN 2
#define CHART_TEMPERATURE_MAX 70


const char* ssid = "Sy";
const char* password = "11111111";

const char STYLE_CSS[] PROGMEM = R"css(
:root{--bg:#0f1419;--surface:#1a2332;--border:#2d3a4d;--text:#e6edf3;--text-muted:#8b949e;--accent:#58a6ff;--accent-green:#3fb950;--accent-orange:#d29922;--accent-red:#f85149}
*{box-sizing:border-box}
body{font-family:'Outfit',sans-serif;background:var(--bg);color:var(--text);margin:0;padding:24px;min-height:100vh}
.banner{background:rgba(210,153,34,.15);border:1px solid var(--accent-orange);border-radius:8px;padding:12px 16px;margin-bottom:20px;font-size:.9rem;color:var(--accent-orange)}
.container{max-width:720px;margin:0 auto}
h1{font-size:1.75rem;font-weight:700;margin-bottom:24px;display:flex;align-items:center;gap:10px}
h1::before{content:'';width:8px;height:28px;background:linear-gradient(180deg,var(--accent),var(--accent-green));border-radius:4px}
.status-dot{width:8px;height:8px;border-radius:50%;background:var(--accent-green);animation:pulse 2s infinite}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.5}}
.cards{display:grid;grid-template-columns:repeat(auto-fit,minmax(160px,1fr));gap:16px;margin-bottom:24px}
.card{background:var(--surface);border:1px solid var(--border);border-radius:12px;padding:20px;text-align:center}
.card-title{font-size:.8rem;color:var(--text-muted);text-transform:uppercase;letter-spacing:.05em;margin-bottom:8px}
.card-value{font-family:'JetBrains Mono',monospace;font-size:1.75rem;font-weight:600}
.card-unit{font-size:.9rem;color:var(--text-muted)}
.badge{display:inline-block;padding:6px 14px;border-radius:20px;font-size:.85rem;font-weight:600}
.badge-safe{background:rgba(63,185,80,.2);color:var(--accent-green)}
.badge-danger{background:rgba(248,81,73,.2);color:var(--accent-red);animation:blink-red 0.8s ease-in-out infinite}
@keyframes blink-red{0%,100%{opacity:1}50%{opacity:0.3}}
.chart-card{background:var(--surface);border:1px solid var(--border);border-radius:12px;padding:20px;margin-bottom:24px}
.chart-card h3{font-size:1rem;font-weight:600;margin:0 0 16px 0;color:var(--text-muted)}
.chart-wrap{height:200px;position:relative}
.controls{background:var(--surface);border:1px solid var(--border);border-radius:12px;padding:24px}
.controls h3{font-size:1rem;font-weight:600;margin:0 0 20px 0;color:var(--text-muted)}
.gpio-row{display:flex;align-items:center;justify-content:space-between;padding:12px 0;border-bottom:1px solid var(--border)}
.gpio-row:last-of-type{border-bottom:none}
.gpio-label{font-family:'JetBrains Mono',monospace;font-size:.9rem}
.btn{display:inline-block;padding:10px 24px;border-radius:8px;font-family:'Outfit',sans-serif;font-size:.9rem;font-weight:600;text-decoration:none;cursor:pointer;border:none;transition:transform .15s,opacity .15s}
.btn:hover{opacity:.9;transform:translateY(-1px)}
.btn-on{background:var(--accent-green);color:#0d1117}
.btn-off{background:var(--border);color:var(--text);margin-left:8px}
.device-row{display:flex;align-items:center;gap:12px}
.device-icon{display:inline-flex;align-items:center;justify-content:center;color:var(--text-muted);transition:color .3s,filter .3s}
.device-icon svg{display:block}
.device-icon-bulb.lit{color:#ffd54f;filter:drop-shadow(0 0 8px rgba(255,213,79,.6))}
.device-icon-fan.spinning svg{animation:fan-spin 1s linear infinite}
@keyframes fan-spin{from{transform:rotate(0deg)}to{transform:rotate(360deg)}}
.device-icon-fan.spinning{color:var(--accent)}
)css";

struct KalmanFilter {
  float estimate;
  float errorEstimate;
  float errorMeasurement;
  float errorProcess;

  KalmanFilter() {
    estimate = 0;
    errorEstimate = 1;
    errorMeasurement = 4;
    errorProcess = 0.01;
  }

  float update(float measurement) {
    if (isnan(measurement) || measurement <= 0) {
      return estimate;
    }
    float kalmanGain = errorEstimate / (errorEstimate + errorMeasurement);
    estimate = estimate + kalmanGain * (measurement - estimate);
    errorEstimate = (1 - kalmanGain) * errorEstimate + errorProcess;
    return estimate;
  }
};

DHT dht(DHTPIN, DHTTYPE);
WebServer server(HTTP_PORT);
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);

KalmanFilter kalmanTemp;
KalmanFilter kalmanHumid;

String output26State = "off";
String output27State = "off";
String inputLine = "";
bool autoFanActive = false;

void handleGPIO26On() {
  output26State = "on";
  digitalWrite(GPIO_OUTPUT_26, HIGH);
  handleRoot();
}

void handleGPIO26Off() {
  output26State = "off";
  digitalWrite(GPIO_OUTPUT_26, LOW);
  handleRoot();
}

void handleGPIO27On() {
  output27State = "on";
  autoFanActive = false;
  digitalWrite(GPIO_OUTPUT_27, HIGH);
  handleRoot();
}

void handleGPIO27Off() {
  output27State = "off";
  autoFanActive = false;
  digitalWrite(GPIO_OUTPUT_27, LOW);
  handleRoot();
}

void handleStyle() {
  server.send_P(HTTP_OK, "text/css", STYLE_CSS);
}

void handleData() {
  float hRaw = dht.readHumidity();
  float tRaw = dht.readTemperature();
  float h = kalmanHumid.update(hRaw);
  float t = kalmanTemp.update(tRaw);
  int waterLevel = analogRead(GPIO_WATER_SENSOR);
  int gas = digitalRead(GPIO_GAS_SENSOR);

  String json = "{";
  json += "\"temperature\":" + String(t,1) + ",";
  json += "\"humidity\":" + String(h,1) + ",";
  json += "\"waterLevel\":" + String(waterLevel) + ",";
  json += "\"gas\":" + String(gas == LOW ? 1 : 0) + ",";
  json += "\"light\":" + String(digitalRead(GPIO_OUTPUT_26) == HIGH ? 1 : 0) + ",";
  json += "\"fan\":" + String(digitalRead(GPIO_OUTPUT_27) == HIGH ? 1 : 0);
  json += "}";

  server.send(HTTP_OK, "application/json", json);
}

void handleRoot() {
  int waterLevelValue = analogRead(GPIO_WATER_SENSOR);
  int gasValue = digitalRead(GPIO_GAS_SENSOR);
  String waterStateStr = (waterLevelValue < WATER_LEVEL_THRESHOLD) ? "Be Can " : "Be Day ";
  String gasStateStr = (gasValue == LOW) ? "PHAT HIEN gas " : "AN TOAN ";

  String html = R"rawliteral(
  <!DOCTYPE html>
  <html lang="vi">
  <head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 IoT Monitor</title>
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
  <link href="https://fonts.googleapis.com/css2?family=JetBrains+Mono:wght@400;600&family=Outfit:wght@400;500;600;700&display=swap" rel="stylesheet">
  <link href="/style.css" rel="stylesheet">
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  </head>
  <body>
  <div class="container">
    <h1><span class="status-dot"></span> TRẠM QUAN TRẮC ESP32</h1>

    <div class="cards">
      <div class="card">
        <div class="card-title">Nhiệt độ</div>
        <div class="card-value"><span id="temp">--</span><span class="card-unit"> °C</span></div>
      </div>
      <div class="card">
        <div class="card-title">Độ ẩm</div>
        <div class="card-value"><span id="hum">--</span><span class="card-unit"> %</span></div>
      </div>
      <div class="card">
        <div class="card-title">MỨC NƯỚC TRONG BỂ</div>
        <div><span class="card-value"><span id="waterLevel">--</span></span></div>
        <div><span id="waterStatus" class="badge )rawliteral" + String(waterLevelValue < WATER_LEVEL_THRESHOLD ? "badge-danger" : "badge-safe") + R"rawliteral("> )rawliteral" + waterStateStr + R"rawliteral(</span></div>
      </div>
      <div class="card">
        <div class="card-title">CẢM BIẾN GAS</div>
        <div><span id="gas" class="badge )rawliteral" + String(gasValue == LOW ? "badge-danger" : "badge-safe") + R"rawliteral("> )rawliteral" + gasStateStr + R"rawliteral(</span></div>
      </div>
    </div>

    <div class="chart-card">
      <h3>Độ ẩm (%)</h3>
      <div class="chart-wrap"><canvas id="chart-hum"></canvas></div>
    </div>
    <div class="chart-card">
      <h3>Nhiệt độ (°C)</h3>
      <div class="chart-wrap"><canvas id="chart-temp"></canvas></div>
    </div>

    <div class="controls">
      <h3>Điều khiển thiết bị đầu ra</h3>
      <div class="gpio-row">
        <span class="device-row">
          <span class="device-icon device-icon-bulb" id="light-icon" aria-hidden="true">
            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor" width="28" height="28"><path d="M9 21c0 .55.45 1 1 1h4c.55 0 1-.45 1-1v-1H9v1zm3-19C8.14 2 5 5.14 5 9c0 2.38 1.19 4.47 3 5.74V17c0 .55.45 1 1 1h6c.55 0 1-.45 1-1v-2.26c1.81-1.27 3-3.36 3-5.74 0-3.86-3.14-7-7-7z"/></svg>
          </span>
          <span class="gpio-label">Đèn (GPIO 26)</span>
        </span>
        <span>
          <a href="/26/on" class="btn btn-on">BẬT</a>
          <a href="/26/off" class="btn btn-off">TẮT</a>
        </span>
      </div>
      <div class="gpio-row">
        <span class="device-row">
          <span class="device-icon device-icon-fan" id="fan-icon" aria-hidden="true">
            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5" width="28" height="28"><circle cx="12" cy="12" r="9"/><path d="M12 3v4m0 10v4M3 12h4m10 0h4M5.64 5.64l2.83 2.83m5.06 5.06l2.83 2.83M5.64 18.36l2.83-2.83m5.06-5.06l2.83-2.83"/></svg>
          </span>
          <span class="gpio-label">Quạt (GPIO 27)</span>
        </span>
        <span>
          <a href="/27/on" class="btn btn-on">BẬT</a>
          <a href="/27/off" class="btn btn-off">TẮT</a>
        </span>
      </div>
    </div>
  </div>

  <script>
  (function(){
    var lightOn = )rawliteral" + String(digitalRead(GPIO_OUTPUT_26) == HIGH ? "true" : "false") + R"rawliteral(;
    var fanOn = )rawliteral" + String(digitalRead(GPIO_OUTPUT_27) == HIGH ? "true" : "false") + R"rawliteral(;
    var li = document.getElementById('light-icon'), fi = document.getElementById('fan-icon');
    if (li) li.classList.toggle('lit', lightOn);
    if (fi) fi.classList.toggle('spinning', fanOn);
  })();
  const ctxHum = document.getElementById('chart-hum').getContext('2d');
  const humGrad = ctxHum.createLinearGradient(0,0,0,200);
  humGrad.addColorStop(0,'rgba(88,166,255,.3)');
  humGrad.addColorStop(1,'rgba(88,166,255,0)');
  const chartHum = new Chart(ctxHum, {
    type: 'line',
    data: {
      labels: [],
      datasets: [{
        label: 'Độ ẩm (%)',
        borderColor: '#58a6ff',
        backgroundColor: humGrad,
        fill: true,
        data: [],
        tension: 0.35,
        pointRadius: 3,
        pointHoverRadius: 5
      }]
    },
    options: {
      animation: false,
      responsive: true,
      maintainAspectRatio: false,
      interaction: { intersect: false, mode: 'index' },
      plugins: { legend: { labels: { color: '#8b949e', font: { size: 12 } } } },
      scales: {
        x: { grid: { color: '#2d3a4d' }, ticks: { color: '#8b949e', maxTicksLimit: 8 } },
        y: { min: )rawliteral" + String(CHART_HUMIDITY_MIN) + R"rawliteral(, max: )rawliteral" + String(CHART_HUMIDITY_MAX) + R"rawliteral(, grid: { color: '#2d3a4d' }, ticks: { color: '#8b949e' } }
      }
    }
  });

  const ctxTemp = document.getElementById('chart-temp').getContext('2d');
  const tempGrad = ctxTemp.createLinearGradient(0,0,0,200);
  tempGrad.addColorStop(0,'rgba(248,81,73,.3)');
  tempGrad.addColorStop(1,'rgba(248,81,73,0)');
  const chartTemp = new Chart(ctxTemp, {
    type: 'line',
    data: {
      labels: [],
      datasets: [{
        label: 'Nhiệt độ (°C)',
        borderColor: '#f85149',
        backgroundColor: tempGrad,
        fill: true,
        data: [],
        tension: 0.35,
        pointRadius: 3,
        pointHoverRadius: 5
      }]
    },
    options: {
      animation: false,
      responsive: true,
      maintainAspectRatio: false,
      interaction: { intersect: false, mode: 'index' },
      plugins: { legend: { labels: { color: '#8b949e', font: { size: 12 } } } },
      scales: {
        x: { grid: { color: '#2d3a4d' }, ticks: { color: '#8b949e', maxTicksLimit: 8 } },
        y: { min: )rawliteral" + String(CHART_TEMPERATURE_MIN) + R"rawliteral(, max: )rawliteral" + String(CHART_TEMPERATURE_MAX) + R"rawliteral(, grid: { color: '#2d3a4d' }, ticks: { color: '#8b949e' } }
      }
    }
  });

  function updateData() {
    fetch('/data')
      .then(res => res.ok ? res.json() : Promise.reject())
      .then(data => {
        const time = new Date().toLocaleTimeString();
        const temp = (data.temperature != null && !isNaN(data.temperature)) ? data.temperature.toFixed(1) : '--';
        const hum = (data.humidity != null && !isNaN(data.humidity)) ? data.humidity.toFixed(1) : '--';

        document.getElementById('temp').textContent = temp;
        document.getElementById('hum').textContent = hum;

        const lightEl = document.getElementById('light-icon');
        const fanEl = document.getElementById('fan-icon');
        if (lightEl) lightEl.classList.toggle('lit', !!data.light);
        if (fanEl) fanEl.classList.toggle('spinning', !!data.fan);

        document.getElementById('waterLevel').textContent = data.waterLevel;
        const waterEl = document.getElementById('waterStatus');
        const waterLow = data.waterLevel < 50;
        waterEl.textContent = waterLow ? 'Be Can' : 'Be Day';
        waterEl.className = 'badge ' + (waterLow ? 'badge-danger' : 'badge-safe');

        const gasEl = document.getElementById('gas');
        gasEl.textContent = data.gas ? 'PHAT HIEN gas' : 'AN TOAN';
        gasEl.className = 'badge ' + (data.gas ? 'badge-danger' : 'badge-safe');

        if (temp !== '--' && hum !== '--') {
          if (chartHum.data.labels.length > )rawliteral" + String(CHART_MAX_DATA_POINTS) + R"rawliteral() {
            chartHum.data.labels.shift();
            chartHum.data.datasets[0].data.shift();
            chartTemp.data.labels.shift();
            chartTemp.data.datasets[0].data.shift();
          }
          chartHum.data.labels.push(time);
          chartHum.data.datasets[0].data.push(parseFloat(data.humidity));
          chartTemp.data.labels.push(time);
          chartTemp.data.datasets[0].data.push(parseFloat(data.temperature));
          chartHum.update('none');
          chartTemp.update('none');
        }
      })
      .catch(() => {});
  }
  updateData();
  setInterval(updateData, )rawliteral" + String(CHART_UPDATE_INTERVAL_MS) + R"rawliteral();
  </script>
  </body>
  </html>
  )rawliteral";

  server.send(HTTP_OK, "text/html", html);
}

void setupWebServer() {
  server.on("/style.css", handleStyle);
  server.on("/data", handleData);
  server.on("/", handleRoot);
  server.on("/26/on", handleGPIO26On);
  server.on("/26/off", handleGPIO26Off);
  server.on("/27/on", handleGPIO27On);
  server.on("/27/off", handleGPIO27Off);
  server.begin();
}

void setupLCD() {
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ESP32 IoT Monitor");
  lcd.setCursor(0, 1);
  lcd.print("Khoi tao...");
  delay(1500);
  lcd.clear();
}

void updateLCD(float temperature, float humidity, int waterLevel, int gasDetected) {
  bool lightOn = digitalRead(GPIO_OUTPUT_26) == HIGH;
  bool fanOn = digitalRead(GPIO_OUTPUT_27) == HIGH;
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("N:");
  lcd.print(temperature, 1);
  lcd.print((char)223);
  lcd.print("A:");
  lcd.print(humidity, 1);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("N:");
  lcd.print(waterLevel);
  lcd.print(" ");
  if (gasDetected) {
    lcd.print("G:!");
  } else {
    lcd.print("G:O");
  }

  lcd.setCursor(8, 1);
  lcd.print("D:");
  lcd.print(lightOn ? "1" : "0");
  lcd.print(" Q:");
  lcd.print(fanOn ? "1" : "0");
}

bool extractQuoted(const String& cmd, int index, String& result) {
  int count = 0;
  int start = -1;

  for (int i = 0; i < cmd.length(); i++) {
    if (cmd[i] == '"') {
      if (count == index * 2) {
        start = i + 1;
      } else if (count == index * 2 + 1) {
        result = cmd.substring(start, i);
        return true;
      }
      count++;
    }
  }
  return false;
}

void handleCommand(String cmd) {
  if (cmd.startsWith("WIFI ")) {
    String ssidStr, passStr;
    if (!extractQuoted(cmd, 0, ssidStr) || !extractQuoted(cmd, 1, passStr)) {
      Serial.println("Invalid format!");
      Serial.println("Use: WIFI \"SSID\" \"PASSWORD\"");
      return;
    }
    Serial.print("Connecting to: ");
    Serial.println(ssidStr);
    WiFi.begin(ssidStr.c_str(), passStr.c_str());
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_CONNECT_TIMEOUT_DEFAULT_MS) {
      delay(WIFI_RETRY_DELAY_MS);
      Serial.print(".");
    }
    Serial.println();
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("WiFi connected!");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("WiFi connection failed");
    }
  }
  else if (cmd == "STATUS") {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("WiFi Status: CONNECTED");
      Serial.print("SSID: ");
      Serial.println(WiFi.SSID());
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("WiFi Status: NOT CONNECTED");
    }
  }
  else if (cmd == "DISCONNECT") {
    WiFi.disconnect(true);
    Serial.println("WiFi disconnected");
  }
  else {
    Serial.println("Unknown command");
  }
}

bool tryConnect(const char* ssidConn, const char* passwordConn, unsigned long timeout) {
  Serial.print("Connecting to ");
  Serial.println(ssidConn);
  WiFi.begin(ssidConn, passwordConn);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < timeout) {
    delay(WIFI_RETRY_DELAY_MS);
    Serial.print(".");
  }
  Serial.println();
  return WiFi.status() == WL_CONNECTED;
}

void waitForWiFiCommand() {
  Serial.println("\n=== WiFi Connection Failed ===");
  Serial.println("Please enter WiFi credentials via serial:");
  Serial.println("Format: WIFI \"SSID\" \"PASSWORD\"");
  Serial.println("================================\n");

  bool connected = false;
  while (!connected) {
    while (Serial.available()) {
      char c = Serial.read();
      if (c == '\n') {
        inputLine.trim();
        if (inputLine.length() > 0) {
          handleCommand(inputLine);
          connected = (WiFi.status() == WL_CONNECTED);
        }
        inputLine = "";
      } else {
        inputLine += c;
      }
    }
    delay(SERIAL_COMMAND_DELAY_MS);
  }

  Serial.println("WiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  pinMode(GPIO_OUTPUT_26, OUTPUT);
  pinMode(GPIO_OUTPUT_27, OUTPUT);
  digitalWrite(GPIO_OUTPUT_26, LOW);
  digitalWrite(GPIO_OUTPUT_27, LOW);
  pinMode(GPIO_TEMP_LED, OUTPUT);
  digitalWrite(GPIO_TEMP_LED, LOW);
  pinMode(GPIO_GAS_SENSOR, INPUT);

  if (!tryConnect(ssid, password, WIFI_CONNECT_TIMEOUT_MS)) {
    waitForWiFiCommand();
  } else {
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

  setupWebServer();
  Serial.println("HTTP server started");

  setupLCD();
  Serial.println("LCD initialized");

  if (WiFi.status() == WL_CONNECTED) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("IP:");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP().toString());
    delay(3000);
    lcd.clear();
  }
}

void loop() {
  server.handleClient();

  int waterLevel = analogRead(GPIO_WATER_SENSOR);
  int gas = digitalRead(GPIO_GAS_SENSOR);
  bool gasDetected = (gas == LOW);
  bool waterHigh = (waterLevel > WATER_LEVEL_THRESHOLD);
  static unsigned long lastEventTime = 0;
  
  if (gasDetected) {
    lastEventTime = millis();
    
    if (digitalRead(GPIO_OUTPUT_27) == LOW) {
      digitalWrite(GPIO_OUTPUT_27, HIGH);
      output27State = "on";
      autoFanActive = true;
    }
  } else {
    if (digitalRead(GPIO_OUTPUT_27) == HIGH && autoFanActive) {
      if (millis() - lastEventTime >= 5000) {
        digitalWrite(GPIO_OUTPUT_27, LOW);
        output27State = "off";
        autoFanActive = false;
      }
    }
  }
  
  {
    float tNow = kalmanTemp.update(dht.readTemperature());
    digitalWrite(GPIO_TEMP_LED, tNow > TEMP_THRESHOLD ? HIGH : LOW);
  }

  unsigned long now = millis();
  static unsigned long lastLCDUpdate = 0;
  if (now - lastLCDUpdate >= LCD_UPDATE_INTERVAL_MS) {
    lastLCDUpdate = now;
    float hRaw = dht.readHumidity();
    float tRaw = dht.readTemperature();
    float t = kalmanTemp.update(tRaw);
    float h = kalmanHumid.update(hRaw);
    float temp = t;
    float hum = h;
    updateLCD(temp, hum, waterLevel, gasDetected ? 1 : 0);
  }
}
