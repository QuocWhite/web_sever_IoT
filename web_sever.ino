#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

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

// Function to handle turning GPIO 28 on
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

void readMQ2() {
  int mq2Value = digitalRead(MQ2_PIN);
  gasState = (mq2Value == LOW) ? "GAS DETECTED " : "SAFE ";
}

void handleData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int mq2 = digitalRead(MQ2_PIN);

  String json = "{";
  json += "\"temperature\":" + String(t,1) + ",";
  json += "\"humidity\":" + String(h,1) + ",";
  json += "\"gas\":" + String(mq2 == LOW ? 1 : 0);
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
  <html>
  <head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Monitor</title>

  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>

  <style>
    body {
      font-family: Helvetica;
      text-align: center;
    }
    .button {
      background-color: #4CAF50;
      border: none;
      color: white;
      padding: 14px 30px;
      font-size: 20px;
      cursor: pointer;
      margin: 4px;
    }
    .button2 {
      background-color: #555555;
    }
  </style>
  </head>

  <body>
  <h1>ESP32 Web Server</h1>

  <h2>DHT11 Sensor</h2>
  <p>Temperature: <strong><span id="temp">--</span> °C</strong></p>
  <p>Humidity: <strong><span id="hum">--</span> %</strong></p>

  <h2>MQ-2 Gas Sensor</h2>
  <p>Status: <strong><span id="gas">)" + gasState + R"rawliteral(</span></strong></p>

  <canvas id="chart" width="400" height="220"></canvas>

  <script>
  const ctx = document.getElementById('chart').getContext('2d');

  const chart = new Chart(ctx, {
    type: 'line',
    data: {
      labels: [],
      datasets: [
        {
          label: 'Temperature (°C)',
          borderColor: 'red',
          data: [],
          tension: 0.3,
          pointRadius: 3
        },
        {
          label: 'Humidity (%)',
          borderColor: 'blue',
          data: [],
          tension: 0.3,
          pointRadius: 3
        }
      ]
    },
    options: {
      animation: false,
      responsive: true,
      plugins: {
        legend: {
          labels: {
            font: { size: 16 }
          }
        },
        tooltip: {
          titleFont: { size: 16 },
          bodyFont: { size: 16 }
        }
      },
      scales: {
        x: {
          ticks: {
            font: { size: 14 }
          }
        },
        y: {
          min: 10,
          max: 90,
          ticks: {
            font: { size: 14 }
          }
        }
      }
    }
  });

  function updateData() {
    fetch('/data')
      .then(res => res.json())
      .then(data => {
        const time = new Date().toLocaleTimeString();

        document.getElementById('temp').textContent =
          data.temperature ?? '--';
        document.getElementById('hum').textContent =
          data.humidity ?? '--';

        document.getElementById('gas').textContent =
          data.gas ? 'GAS DETECTED ' : 'SAFE ';

        if (chart.data.labels.length > 20) {
          chart.data.labels.shift();
          chart.data.datasets[0].data.shift();
          chart.data.datasets[1].data.shift();
        }

        chart.data.labels.push(time);
        chart.data.datasets[0].data.push(data.temperature);
        chart.data.datasets[1].data.push(data.humidity);
        chart.update();
      });
  }

  setInterval(updateData, 2000);
  </script>

  <hr>

  <h2>LED Control</h2>

  <p>GPIO 26</p>
  <a href="/26/on"><button class="button">ON</button></a>
  <a href="/26/off"><button class="button button2">OFF</button></a>

  <p>GPIO 27</p>
  <a href="/27/on"><button class="button">ON</button></a>
  <a href="/27/off"><button class="button button2">OFF</button></a>

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
