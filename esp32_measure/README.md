# Web Server Lite - ESP32 IoT Monitoring System

## Project Overview

This is a **lightweight version** of the original `web_sever.ino` with the SMS/messaging subsystem removed. It retains all core IoT monitoring and control functionality — sensor reading, web dashboard, LCD display, gas-triggered fan control, and water level monitoring.

### Differences from the Original

| Feature | Original (`web_sever.ino`) | Lite (`web_sever_lite.ino`) |
|---|---|---|
| Temperature/Humidity monitoring | Yes | Yes |
| Gas detection with auto fan | Yes | Yes |
| Water level monitoring | No | **Added** |
| Temp threshold LED indicator | No | **Added** |
| LCD 16x2 display | Yes | Yes |
| Web dashboard (Chart.js) | Yes | Yes |
| Remote light/fan control | Yes | Yes |
| SMS alerts (SMSChef API) | Yes | **Removed** |
| `PHONE` serial command | Yes | **Removed** |
| `extractQuoted()` helper | Yes | **Removed** |
| `TEMP_THRESHOLD` / `GAS_DETECTED` | Yes | **Removed** |
| Serial output of phone number | Yes | **Removed** |
| `PHONE:` prefix serial alerts | Yes | **Removed** |

---

## Hardware Requirements

| Component | Pin | Description |
|---|---|---|
| ESP32 Dev Module | — | Microcontroller (WiFi + BLE) |
| DHT11 | GPIO 5 | Temperature & humidity sensor |
| MQ-2 | GPIO 18 | Gas/smoke sensor (LOW = gas detected) |
| Water level sensor | GPIO 34 | Analog water level sensor |
| Light (Relay) | GPIO 26 | Output — LED or relay-controlled light |
| Fan (Relay) | GPIO 27 | Output — fan for gas ventilation |
| Temp alert LED | GPIO 2 | LED indicator (on when temp > 35°C) |
| LCD 16x2 I2C | I2C (0x27) | Displays sensor readings and device states |

---

## System Architecture

```
                          ┌─────────────────────────────┐
                          │       TRÌNH DUYỆT (Browser) │
                          │  ┌───────────────────────┐  │
                          │  │  Chart.js Dashboard   │  │
                          │  │  fetch('/data') 5s    │  │
                          │  │  Bật/Tắt GPIO         │  │
                          │  └───────────┬───────────┘  │
                          └──────────────┼──────────────┘
                                         │ HTTP
                                         ▼
┌──────────────────────────────────────────────────────────────────┐
│                       ESP32 - Web Server                         │
│                                                                  │
│  ┌───────────────── setup() ─────────────────────────────────┐   │
│  │  GPIO Init ──▶ WiFi Connect ──▶ setupWebServer() ──▶ loop│   │
│  │  (Relay OFF)     │                    │                    │   │
│  │                  ▼                    ▼                    │   │
│  │           ┌──────────┐       ┌───────────────┐            │   │
│  │           │ Thành    │       │ GET /, /data, │            │   │
│  │           │ công?    │       │ /26/on, /off, │            │   │
│  │           │  ┌─┐    │       │ /27/on, /off  │            │   │
│  │           │  └─┘    │       └───────────────┘            │   │
│  │           │   │     │                                     │   │
│  │           │ Có│     │                                     │   │
│  │           │   ▼     │                                     │   │
│  │           │ Serial  │                                     │   │
│  │           │ Chờ lệnh│                                     │   │
│  │           │ WIFI    │                                     │   │
│  │  └───────────┴─────────┴─────────────────────────────────────┘   │
│  │                                                                  │
│  │  ┌──────────────── loop() ───────────────────────────────────┐   │
│  │  │                                                           │   │
│  │  │    server.handleClient()     ◄────── HTTP requests        │   │
│  │  │         │                                                │   │
│  │  │         ▼                                                │   │
│  │  │    ┌──────────┐                                          │   │
│  │  │    │ MQ-2 ==  │── CAO ──▶ check auto fan cooldown        │   │
│  │  │    │ THẤP?    │          if 5s elapsed → turn fan off    │   │
│  │  │    └────┬─────┘                                          │   │
│  │  │         │ THẤP (Gas detected)                            │   │
│  │  │         ▼                                                │   │
│  │  │    ┌──────────────────┐                                   │   │
│  │  │    │ Record timestamp │                                   │   │
│  │  │    │ Turn fan ON      │                                   │   │
│  │  │    └──────────────────┘                                   │   │
│  │  │                                                           │   │
│  │  │    ┌──────────────────┐                                   │   │
│  │  │    │ Temp LED: ON if  │                                   │   │
│  │  │    │ temp > 35°C      │                                   │   │
│  │  │    └──────────────────┘                                   │   │
│  │  │                                                           │   │
│  │  │    ┌──────────────────┐                                   │   │
│  │  │    │ LCD update 2s   │                                   │   │
│  │  │    │ Read DHT11 +    │                                   │   │
│  │  │    │ water level     │                                   │   │
│  │  │    │ Kalman Filter   │                                   │   │
│  │  │    │ Display on LCD  │                                   │   │
│  │  │    └──────────────────┘                                   │   │
│  │  └───────────────────────────────────────────────────────────┘   │
│  │                                                                  │
│  │  ┌─────────── SENSORS ────────────┐  ┌───── DEVICES ─────────┐ │
│  │  │  DHT11 (GPIO 5)                │  │  Light (GPIO 26)       │ │
│  │  │    Temp + Humidity             │  │  Fan (GPIO 27)         │ │
│  │  │         │                      │  │  Temp LED (GPIO 2)    │ │
│  │  │         ▼                      │  │  LCD 16x2 I2C (0x27)  │ │
│  │  │  Kalman Filter                 │  └───────────────────────┘ │
│  │  │         │                      │                            │
│  │  │         ▼                      │                            │
│  │  │  Data for LCD + JSON API       │                            │
│  │  │                                │                            │
│  │  │  MQ-2 (GPIO 18)                │                            │
│  │  │    LOW = Gas detected          │                            │
│  │  │                                │                            │
│  │  │  Water Level (GPIO 34, ADC)    │                            │
│  │  │    < 50 = Empty (Bể Cạn)      │                            │
│  │  │    >= 50 = Full (Bể đầy)      │                            │
│  │  └────────────────────────────────┘                            │
│  │                                                                  │
│  │  ┌─────────── SERIAL (115200) ──────────────────────────────┐  │
│  │  │  WIFI "SSID" "PASS"  │  STATUS  │  DISCONNECT              │  │
│  │  └────────────────────────────────────────────────────────────┘  │
│  └──────────────────────────────────────────────────────────────────┘
```

---

## Code Flow

### 1. `setup()` — Initialization Sequence

```
Power On
    │
    ▼
Serial.begin(115200)
    │
    ▼
Initialize GPIOs:
  - GPIO 26, 27 → OUTPUT, LOW (light & fan off)
  - GPIO 2      → OUTPUT, LOW (temp LED off)
  - GPIO 18     → INPUT  (MQ-2 gas sensor)
    │
    ▼
tryConnect(ssid, pass, 10s)
    │
    ├── SUCCESS → print IP
    │
    └── FAIL    → waitForWiFiCommand()
                  (blocking serial input loop for WIFI command)
    │
    ▼
setupWebServer()  → register HTTP route handlers
    │
    ▼
setupLCD()       → init I2C, show splash, clear
    │
    ▼
Ready (enters loop())
```

### 2. `loop()` — Main Execution Cycle

```
loop()
    │
    ├── server.handleClient()    ← Handle incoming HTTP requests
    │
    ├── Gas Detection Logic:
    │     │
    │     ├── MQ-2 == LOW (gas detected)?
    │     │     ├── Record timestamp
    │     │     └── If fan OFF → turn fan ON (auto mode)
    │     │
    │     └── MQ-2 == HIGH (safe)?
    │           └── If fan was auto-activated AND 5s since last
    │               gas detection → turn fan OFF
    │
    ├── Temperature LED:
    │     └── If temp > 35°C → LED ON, else LED OFF
    │
    └── LCD Update (every 2000ms):
          ├── Read DHT11 (raw temp & humidity), water level (analog)
          ├── Apply Kalman filter to temp & humidity
          └── updateLCD(temp, hum, waterLevel, gasDetected)
```

### 3. HTTP Request Handling

| Route | Handler | Description |
|---|---|---|
| `GET /` | `handleRoot()` | Serves full HTML dashboard with embedded Chart.js |
| `GET /data` | `handleData()` | Returns JSON: `{temperature, humidity, waterLevel, gas, light, fan}` |
| `GET /style.css` | `handleStyle()` | Serves embedded CSS from PROGMEM |
| `GET /26/on` | `handleGPIO26On()` | Sets GPIO 26 HIGH, serves updated HTML |
| `GET /26/off` | `handleGPIO26Off()` | Sets GPIO 26 LOW, serves updated HTML |
| `GET /27/on` | `handleGPIO27On()` | Sets GPIO 27 HIGH, clears auto-flag, serves HTML |
| `GET /27/off` | `handleGPIO27Off()` | Sets GPIO 27 LOW, clears auto-flag, serves HTML |

### 4. Frontend (Client-Side JavaScript)

```
Page Load
    │
    ├── Initial state: read GPIO states → set icon CSS classes
    │
    ├── Create two Chart.js line charts:
    │     - Humidity chart (blue, 30–100% range)
    │     - Temperature chart (red, 2–70°C range)
    │
    └── updateData() called immediately, then every 5000ms:
          │
          ├── fetch('/data')
          │     │
          │     ├── Update card values (temp, hum, water level, gas)
          │     ├── Toggle icon classes (light bulb lit, fan spinning)
          │     └── Push new data point to charts (max 20 points)
          │
          └── On error: silently ignore, retry next interval
```

---

## Key Algorithms

### 1. Kalman Filter for Sensor Smoothing

**Purpose:** Reduce noise and transient spikes from the DHT11 sensor readings.

**State:**
- `estimate` — current filtered value
- `errorEstimate` — uncertainty of the estimate
- `errorMeasurement` — inherent sensor noise variance (fixed at 4.0)
- `errorProcess` — process noise (fixed at 0.01)

**Algorithm (per update):**

```
function update(measurement):
    if measurement is NaN or ≤ 0:
        return estimate  // reject invalid readings
    
    // Compute Kalman Gain (0 to 1)
    K = errorEstimate / (errorEstimate + errorMeasurement)
    
    // Update estimate with weighted measurement
    estimate = estimate + K × (measurement - estimate)
    
    // Update error estimate
    errorEstimate = (1 - K) × errorEstimate + errorProcess
    
    return estimate
```

**Characteristics:**
- `K ≈ 0.2` initially (slowly trusts the sensor)
- Over time `K → 0.0025` (heavily filters, smooth output)
- The filter recovers quickly from large deviations if they persist
- Invalid readings (`NaN`, zero, negative) are silently ignored

### 2. Gas Detection with Auto Fan Control

**Trigger:** MQ-2 sensor output is active-low — `LOW` means gas detected.

**Logic:**
```
if mq2 == LOW:
    timestamp = now()
    if fan is OFF:
        fan = ON  (forced, mark as auto-activated)

else (no gas):
    if fan is ON AND fan was auto-activated:
        if now() - timestamp ≥ 5000ms:
            fan = OFF
            clear auto-flag
```

**Key detail:** The 5-second debounce prevents the fan from toggling rapidly when gas concentration hovers near the threshold. The `autoFanActive` flag distinguishes auto-activation from manual user control — if the user manually turned the fan on, the system will NOT turn it off automatically.

### 3. Temperature Threshold LED

GPIO 2 LED indicates temperature status:
- ON when temperature > 35°C (overheat warning)
- OFF when temperature ≤ 35°C

### 4. Water Level Monitoring

The analog water level sensor on GPIO 34 (ADC) is read and displayed:
- Reading < 50 → "Bể Cạn" (empty tank)
- Reading ≥ 50 → "Bể đầy" (full tank)

### 5. Non-Blocking Timing (`millis()`)

All timed operations use `millis()` difference checks instead of `delay()` to keep the system responsive:

| Operation | Interval | Mechanism |
|---|---|---|
| LCD update | 2000 ms | `if (now - lastLCDUpdate >= LCD_UPDATE_INTERVAL_MS)` |
| Chart data poll (JS side) | 5000 ms | `setInterval(updateData, 5000)` |
| Fan auto turn-off | 5000 ms | From last gas detection timestamp |
| WiFi connect retry | 500 ms | Delay between retry dots |

### 6. Serial Command Parsing

The `extractQuoted()` helper extracts N-th quoted argument from a command string:

```
Input:  WIFI "MySSID" "MyPassword"
Index 0: MySSID
Index 1: MyPassword

Algorithm:
  count quote characters in string
  opening quote at count = index × 2
  closing quote at count = index × 2 + 1
  return substring between them
```

Available commands in this lite version:
- `WIFI "SSID" "PASSWORD"` — connect to WiFi network
- `STATUS` — display current WiFi connection status
- `DISCONNECT` — disconnect from WiFi

### 7. LCD Display Layout (16×2)

```
┌──────────────────┐
│ N:25.0°C A:60.0% │  ← Row 0: temperature + humidity
│ W:1024 GAS:OK D:0│  ← Row 1: water level, gas, outputs
└──────────────────┘
```

- `N:` temperature in Celsius with degree symbol (byte 223)
- `A:` relative humidity percentage
- `W:` water level analog reading
- `GAS:!` when gas detected, `GAS:OK` when safe
- `D:0/1` light relay state
- `Q:0/1` fan relay state

---

## Web Dashboard

The dashboard is served as a single HTML page with embedded CSS and JavaScript:

```
┌────────────────────────────────────────────┐
│  ● TRẠM QUAN TRẮC ESP32                    │
│                                            │
│  ┌────────┐ ┌────────┐ ┌────────────────┐ │
│  │  24.5°C │ │  60%   │ │     1024      │ │
│  │ Nhiệt độ│ │ Độ ẩm  │ │ MỨC NƯỚC TRONG│ │
│  │         │ │        │ │ BỂ: Bể đầy    │ │
│  └────────┘ └────────┘ └────────────────┘ │
│  ┌────────────────┐                        │
│  │  CẢM BIẾN GAS   │                        │
│  │  AN TOAN (xanh) │                        │
│  └────────────────┘                        │
│                                            │
│  ┌──────────────────────────────────────┐  │
│  │  Humidity (%) — line chart (blue)    │  │
│  │  [Chart.js, 20-point rolling window] │  │
│  └──────────────────────────────────────┘  │
│                                            │
│  ┌──────────────────────────────────────┐  │
│  │  Temperature (°C) — line chart (red) │  │
│  └──────────────────────────────────────┘  │
│                                            │
│  ┌─ Output Control ─────────────────────┐ │
│  │ 💡 Light (GPIO 26)        [ON] [OFF] │ │
│  │ ⏳ Fan  (GPIO 27)         [ON] [OFF] │ │
│  └──────────────────────────────────────┘ │
└────────────────────────────────────────────┘
```

**Chart.js Features:**
- Two separate line charts for temperature and humidity
- Gradient fill under the line
- Rolling 20-data-point window
- 5-second auto-refresh interval
- Dark theme matching the dashboard design

---

## Serial Commands

Connect to the ESP32 via serial monitor at **115200 baud**.

| Command | Description |
|---|---|
| `WIFI "SSID" "PASSWORD"` | Connect to a WiFi network |
| `STATUS` | Print current WiFi status and IP address |
| `DISCONNECT` | Disconnect from current WiFi network |

If initial WiFi auto-connect fails, the system enters an interactive serial mode prompting for credentials.

---

## Quick Start

```bash
# 1. Install ESP32 board support in Arduino IDE
# 2. Install required libraries:
#    - WebServer (built-in ESP32 core)
#    - DHT sensor library by Adafruit
#    - LiquidCrystal_I2C by F. Malpartida / M. Schwartz
# 3. Open web_sever_lite.ino in Arduino IDE
# 4. Edit WiFi credentials at top of file:
#      const char* ssid = "YourSSID";
#      const char* password = "YourPassword";
# 5. Select board: ESP32 Dev Module
# 6. Select correct COM port
# 7. Upload to ESP32
# 8. Open Serial Monitor (115200) to see IP address
# 9. Navigate to http://<ESP32_IP>/ in a browser
```

---

## Configuration

Edit the top of `web_sever_lite.ino`:

```cpp
const char* ssid = "YourSSID";
const char* password = "YourPassword";
```

---

## Pin Mapping Summary / Bảng Ghép Nối Chân

| GPIO | Component / Linh kiện        | Type / Loại | Description / Chức năng                 |
|------|------------------------------|-------------|-----------------------------------------|
| 2    | Temp LED / Đèn báo nhiệt     | OUTPUT      | Temp threshold indicator (>35°C) / LED cảnh báo nhiệt độ cao |
| 5    | DHT11                        | INPUT       | Temperature & humidity sensor / Cảm biến nhiệt độ & độ ẩm |
| 18   | MQ-2                         | INPUT       | Gas/smoke sensor (LOW = gas detected) / Cảm biến khí gas (THẤP = có gas) |
| 34   | Water level / Mức nước       | INPUT (ADC) | Analog water level sensor / Cảm biến mức nước (analog) |
| 21   | LCD SDA                      | I2C         | I2C data line / Đường dữ liệu I2C (LCD 0x27) |
| 22   | LCD SCL                      | I2C         | I2C clock line / Đường xung nhịp I2C (LCD 0x27) |
| 26   | Light / Đèn                  | OUTPUT      | Relay/bulb control / Điều khiển relay đèn |
| 27   | Fan / Quạt                   | OUTPUT      | Relay/fan control / Điều khiển relay quạt |

---

## File Structure

```
web_sever_lite/
├── web_sever_lite.ino    # Main Arduino sketch (single file)
└── README.md             # This documentation
```

All code is contained in a single `.ino` file — no separate header or source files needed.

---

## License

Same as original project.
