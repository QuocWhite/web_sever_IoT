# ESP32 IoT Monitor

Web-based IoT monitoring on ESP32 with temperature/humidity sensing, gas detection, LCD display, and web dashboard.

## Hardware

- **ESP32** microcontroller
- **DHT11** temperature & humidity sensor (GPIO 5)
- **MQ-2** gas sensor (GPIO 18)
- **GPIO 26** - Light control output
- **GPIO 27** - Fan control output
- **GPIO 2** - Gas alert LED
- **LCD 16x2 I2C** - Display sensor readings (address 0x27)

## Features

- Real-time temperature & humidity monitoring
- Gas leak detection with visual LED indicator and auto fan activation
- LCD 16x2 I2C display showing sensor data and GPIO states
- Web dashboard with live Chart.js charts
- Remote control of light and fan via web interface
- Serial command interface for WiFi configuration

## Files

| File | Description |
|------|-------------|
| `web_sever.ino` | Complete Arduino sketch (all code merged: server, HTML, CSS) |
| `style.css` | Standalone CSS for development |
| `preview.html` | Standalone HTML preview with simulated data (no ESP32 needed) |
| `style.h` | Header version of CSS (legacy) |

## Quick Start

1. Install ESP32 board support in Arduino IDE
2. Install libraries: `WebServer`, `DHT sensor library`, `LiquidCrystal_I2C`
3. Open `web_sever.ino`, update WiFi credentials
4. Select board: `ESP32 Dev Module`, upload sketch
5. Open Serial Monitor at 115200 baud to see IP address
6. Navigate to `http://<ESP32_IP>/`

## Serial Commands

| Command | Description |
|---------|-------------|
| `WIFI "SSID" "PASSWORD"` | Connect to WiFi |
| `STATUS` | Print connection status |
| `DISCONNECT` | Disconnect WiFi |

## LCD Display

```
Line 1: T:25.0C H:60.0%
Line 2: GAS:OK  L:0 F:0
```
