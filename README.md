# ESP32 IoT Monitor

Web-based IoT monitoring system with temperature/humidity sensing, gas detection, LCD display, SMS alerts, and web dashboard.

## Hardware

- **ESP32** or **ESP8266** microcontroller
- **DHT11** temperature & humidity sensor (GPIO 5 / D1)
- **MQ-2** gas sensor (GPIO 18 / D5)
- **GPIO 26/12** - Light control output
- **GPIO 27/13** - Fan control output
- **GPIO 2** - Gas alert LED / temp warning LED
- **LCD 16x2 I2C** (address 0x27)
- **SIM A7680C** (for SMS alerts in full versions)

## Project Structure

Three independent folders for different purposes:

```
esp32_full/        ESP32 with full measurement + SMS alert system
esp32_measure/     ESP32 measurement only (no SMS, adds water level sensor)
esp8266_full/      ESP8266 port with full measurement + SMS
```

### `esp32_full/` — Full System (ESP32)

Complete monitoring system with SMS alerts via Python handler or direct SIM module.

| File | Description |
|------|-------------|
| `web_sever.ino` | ESP32 firmware (Kalman filter, gas/fan/light control, LCD, web UI) |
| `message_handler.py` | Python serial monitor with SMSChef SMS alert integration |
| `message_handler_1.py` | Alternative Python SMS handler |
| `run_handler.sh` | Script to launch the Python SMS handler |
| `config.txt` | Device configuration |
| `sim_arduino/sim_arduino.ino` | Standalone SIM A7680C module firmware |
| `Code.gs` | Google Apps Script for Google Sheets integration |
| `PIN_MAPPING.md` | Hardware pin mapping reference |
| `preview.html` / `style.css` | Web dashboard preview (no ESP32 needed) |
| `QR.jpg` | QR code for quick access |

### `esp32_measure/` — Measurement Only (ESP32)

Lightweight version without SMS. Adds water level monitoring.

| File | Description |
|------|-------------|
| `web_sever.ino` | ESP32 firmware (DHT11, MQ-2, water level, LCD, web UI) |
| `README.md` / `README_vi.md` | English / Vietnamese documentation |
| `BANG_CHAN_PIN.md` | Pin mapping (Vietnamese) |

### `esp8266_full/` — Full System (ESP8266)

ESP8266 (NodeMCU) port with same full feature set as esp32_full.

| File | Description |
|------|-------------|
| `web_sever.ino` | ESP8266 firmware (full measurement + SMS support) |

## Quick Start

1. Install board support (ESP32 or ESP8266) in Arduino IDE
2. Install libraries: `WebServer`, `DHT sensor library`, `LiquidCrystal_I2C`
3. Open the desired `web_sever.ino`, update WiFi credentials
4. Select board, upload sketch
5. For SMS: run `python message_handler.py` (requires `requests`, `pyserial`) or flash `sim_arduino/sim_arduino.ino`

## Serial Commands

```
WIFI "SSID" "PASSWORD"   Connect to WiFi
STATUS                    Print connection status
DISCONNECT                Disconnect WiFi
```

## LCD Display

```
Line 1: T:25.0C H:60.0%
Line 2: GAS:OK  L:0 F:0
```
