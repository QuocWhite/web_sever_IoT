# ESP32 IoT Monitor

A web-based IoT monitoring system built on ESP32 with temperature/humidity sensing, gas detection, LCD display, and remote device control.

## Hardware

- **ESP32** microcontroller
- **DHT11** temperature & humidity sensor (GPIO 5)
- **MQ2** gas sensor (GPIO 18)
- **GPIO 26** - Light control output
- **GPIO 27** - Fan control output
- **GPIO 2** - Gas alert LED
- **LCD 16x2 I2C** - Display sensor readings (address: 0x27, SDA/SCL)
- Module sim a7680c

## Features

- Real-time temperature & humidity monitoring
- Gas leak detection with visual LED indicator and auto fan activation
- Auto fan turns on when gas detected, turns off 5 seconds after gas clears
- LCD 16x2 I2C display showing sensor data and actual hardware states
- Web dashboard with live charts (Chart.js)
- Remote control of light and fan via web interface
- SMS alerts for overheat and gas detection
- Serial command interface for WiFi configuration

## Web Interface

Access at `http://<ESP32_IP>/` to view:
- Current temperature and humidity
- Gas sensor status
- Interactive line charts (5s update interval)
- ON/OFF toggles for GPIO 26 and 27

## Files

| File | Description |
|------|-------------|
| `web_sever.ino` | Complete Arduino sketch (all code merged, includes Kalman filter) |
| `message_handler.py` | Python serial monitor with SMS alert integration |
| `message_handler_1.py` | Alternative Python SMS handler |
| `test_single_message.py` | Standalone SMS test script |
| `config.txt` | Configuration data |
| `run_handler.sh` | Script to launch the Python SMS handler |
| `style.css` | Standalone CSS for development preview |
| `preview.html` | Standalone HTML preview (no ESP32 required) |
| `PIN_MAPPING.md` | Hardware pin mapping reference |
| `Code.gs` | Google Apps Script for Google Sheets integration |
| `sim_arduino/` | SIM A7680C module firmware |
| `web_sever_esp8266/` | ESP8266 variant firmware |
| `web_sever_lite/` | Lite variant (water level, no SMS) |

**Obsolete (code merged into web_sever.ino):** `webserver.cpp/h`, `wifi.cpp/h`, `lcd.cpp/h`, `config.h`, `style.h`

## Configuration

Edit the top of `web_sever.ino` to set WiFi credentials:
```cpp
const char* ssid = "YourSSID";
const char* password = "YourPassword";
```

Edit `message_handler.py` to configure SMS alerts:
```python
API_KEY = "your_smschef_api_key"
DEVICE_UUID = "your_device_uuid"
PHONE_NUMBER = "+84..."
TEMP_THRESHOLD = 35.0  # Celsius
```

## Serial Commands

Connect to ESP32 serial at 115200 baud:
```
WIFI "SSID" "PASSWORD"   # Connect to WiFi
STATUS                    # Print connection status
DISCONNECT                # Disconnect WiFi
```

## SMS Alert System

The Python monitor (`message_handler.py`) reads sensor data from serial and sends SMS alerts via SMSChef API when:
- Temperature exceeds threshold (default: 35°C)
- Gas is detected

**Note:** ESP32 outputs sensor data to serial continuously every 2 seconds, so the Python script can monitor even without a browser connected.

Requires: `pip install requests pyserial`

## Quick Start

1. Install ESP32 board support in Arduino IDE
2. Install libraries: `WebServer`, `DHT`, `LiquidCrystal_I2C`
3. Open `web_sever.ino` in Arduino IDE
4. Update WiFi credentials
5. Select board: `ESP32 Dev Module`
6. Upload sketch to ESP32
7. Run `python message_handler.py` for SMS monitoring

## LCD Display (16x2)

```
Line 1: T:25.0°C H:60.0%
Line 2: GAS:OK  L:0 F:0
```

## Preview UI

Open `preview.html` in a browser to see the dashboard with simulated data (no ESP32 needed).
