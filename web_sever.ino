#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include "style.h"

// Replace with your network credentials
const char* ssid     = "Tu nguyen";
const char* password = "88888888";

// Assign output variables to GPIO pins
const int output26 = 26;
const int output27 = 27;
String output26State = "off";
String output27State = "off";

// DHT11 sensor settings
#define DHTPIN 5      // D5 (GPIO5)
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE);
float temperature = 0.0;
float humidity = 0.0;

// MQ-02 sensor
#define MQ2_PIN 18   // D18
String gasState = "SAFE";

// Create a web server object
WebServer server(80);

// Function to handle turning GPIO 26 on
void handleGPIO26On() {
  output26State = "on";
  digitalWrite(output26, HIGH);
  handleRoot();
}

// Function to handle turning GPIO 26 off
void handleGPIO26Off() {
  output26State = "off";
  digitalWrite(output26, LOW);
  handleRoot();
}

// Function to handle turning GPIO 27 on
void handleGPIO27On() {
  output27State = "on";
  digitalWrite(output27, HIGH);
  handleRoot();
}

// Function to handle turning GPIO 27 off
void handleGPIO27Off() {
  output27State = "off";
  digitalWrite(output27, LOW);
  handleRoot();
}

void handleStyle() {
  server.send_P(200, "text/css", STYLE_CSS);
}

void handleData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int mq2 = digitalRead(MQ2_PIN);

  String json = "{";
  json += "\"temperature\":" + String(t,1) + ",";
  json += "\"humidity\":" + String(h,1) + ",";
  json += "\"gas\":" + String(mq2 == LOW ? 1 : 0) + ",";
  json += "\"light\":" + String(output26State == "on" ? 1 : 0) + ",";
  json += "\"fan\":" + String(output27State == "on" ? 1 : 0);
  json += "}";

  server.send(200, "application/json", json);
}

// Function to handle the root URL and show the current states
void handleRoot() {
  // Read MQ-2 digital signal
  int mq2Value = digitalRead(MQ2_PIN);
  String gasState = (mq2Value == LOW) ? "GAS DETECTED " : "SAFE ";

  String html = R"rawliteral(
  <!DOCTYPE html>
  <html lang="en">
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
    <h1><span class="status-dot"></span> ESP32 IoT Monitor</h1>

    <div class="cards">
      <div class="card">
        <div class="card-title">Temperature</div>
        <div class="card-value"><span id="temp">--</span><span class="card-unit"> °C</span></div>
      </div>
      <div class="card">
        <div class="card-title">Humidity</div>
        <div class="card-value"><span id="hum">--</span><span class="card-unit"> %</span></div>
      </div>
      <div class="card">
        <div class="card-title">Gas Sensor</div>
        <div><span id="gas" class="badge )rawliteral" + String(mq2Value == LOW ? "badge-danger" : "badge-safe") + R"rawliteral("> )rawliteral" + gasState + R"rawliteral(</span></div>
      </div>
    </div>

    <div class="chart-card">
      <h3>Humidity (%)</h3>
      <div class="chart-wrap"><canvas id="chart-hum"></canvas></div>
    </div>
    <div class="chart-card">
      <h3>Temperature (°C)</h3>
      <div class="chart-wrap"><canvas id="chart-temp"></canvas></div>
    </div>

    <div class="controls">
      <h3>Output Control</h3>
      <div class="gpio-row">
        <span class="device-row">
          <span class="device-icon device-icon-bulb" id="light-icon" aria-hidden="true">
            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor" width="28" height="28"><path d="M9 21c0 .55.45 1 1 1h4c.55 0 1-.45 1-1v-1H9v1zm3-19C8.14 2 5 5.14 5 9c0 2.38 1.19 4.47 3 5.74V17c0 .55.45 1 1 1h6c.55 0 1-.45 1-1v-2.26c1.81-1.27 3-3.36 3-5.74 0-3.86-3.14-7-7-7z"/></svg>
          </span>
          <span class="gpio-label">Light (GPIO 26)</span>
        </span>
        <span>
          <a href="/26/on" class="btn btn-on">ON</a>
          <a href="/26/off" class="btn btn-off">OFF</a>
        </span>
      </div>
      <div class="gpio-row">
        <span class="device-row">
          <span class="device-icon device-icon-fan" id="fan-icon" aria-hidden="true">
            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5" width="28" height="28"><circle cx="12" cy="12" r="9"/><path d="M12 3v4m0 10v4M3 12h4m10 0h4M5.64 5.64l2.83 2.83m5.06 5.06l2.83 2.83M5.64 18.36l2.83-2.83m5.06-5.06l2.83-2.83"/></svg>
          </span>
          <span class="gpio-label">Fan (GPIO 27)</span>
        </span>
        <span>
          <a href="/27/on" class="btn btn-on">ON</a>
          <a href="/27/off" class="btn btn-off">OFF</a>
        </span>
      </div>
    </div>
  </div>

  <script>
  (function(){
    var lightOn = )rawliteral" + String(output26State == "on" ? "true" : "false") + R"rawliteral(;
    var fanOn = )rawliteral" + String(output27State == "on" ? "true" : "false") + R"rawliteral(;
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
        label: 'Humidity (%)',
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
        y: { min: 50, max: 120, grid: { color: '#2d3a4d' }, ticks: { color: '#8b949e' } }
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
        label: 'Temperature (°C)',
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
        y: { min: 2, max: 70, grid: { color: '#2d3a4d' }, ticks: { color: '#8b949e' } }
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

        const gasEl = document.getElementById('gas');
        gasEl.textContent = data.gas ? 'GAS DETECTED' : 'SAFE';
        gasEl.className = 'badge ' + (data.gas ? 'badge-danger' : 'badge-safe');

        if (temp !== '--' && hum !== '--') {
          if (chartHum.data.labels.length > 20) {
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
  setInterval(updateData, 5000);
  </script>
  </body>
  </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);

  // Initialize the output variables as outputs
  pinMode(output26, OUTPUT);
  pinMode(output27, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output26, LOW);
  digitalWrite(output27, LOW);
  pinMode(MQ2_PIN, INPUT);

  // Connect to Wi-Fi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Set up the web server to handle different routes
  server.on("/style.css", handleStyle);
  server.on("/data", handleData);
  server.on("/", handleRoot);
  server.on("/26/on", handleGPIO26On);
  server.on("/26/off", handleGPIO26Off);
  server.on("/27/on", handleGPIO27On);
  server.on("/27/off", handleGPIO27Off);

  // Start the web server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Handle incoming client requests
  server.handleClient();
}
