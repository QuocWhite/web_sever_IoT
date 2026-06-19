# Pin Mapping Table

## ESP32 (`web_sever.ino` & `web_sever_lite.ino`)

| GPIO | Component     | Type    | Description                  | Active |
|------|---------------|---------|------------------------------|--------|
| 2    | Gas LED       | OUTPUT  | Gas detection indicator LED  | HIGH   |
| 5    | DHT11/DHT22   | INPUT   | Temperature & humidity sensor| N/A    |
| 18   | MQ-2          | INPUT   | Gas/smoke sensor             | LOW=detected |
| 21   | LCD SDA       | I2C     | I2C data line (LCD 0x27)    | N/A    |
| 22   | LCD SCL       | I2C     | I2C clock line (LCD 0x27)   | N/A    |
| 26   | Light         | OUTPUT  | Relay/bulb control           | HIGH   |
| 27   | Fan           | OUTPUT  | Relay/fan control            | HIGH   |

## ESP8266/NodeMCU (`web_sever_esp8266.ino`)

| GPIO | NodeMCU Pin | Component     | Type    | Description                  | Active |
|------|-------------|---------------|---------|------------------------------|--------|
| 2    | D4          | Gas LED       | OUTPUT  | Gas detection indicator LED  | HIGH   |
| 5    | D1          | DHT11         | INPUT   | Temperature & humidity sensor| N/A    |
| 12   | D6          | Light         | OUTPUT  | Relay/bulb control           | HIGH   |
| 13   | D7          | Fan           | OUTPUT  | Relay/fan control            | HIGH   |
| 14   | D5          | MQ-2          | INPUT   | Gas/smoke sensor             | LOW=detected |
| SDA  | D2 (GPIO4)  | LCD SDA       | I2C     | I2C data line (LCD 0x27)    | N/A    |
| SCL  | D1 (GPIO5)  | LCD SCL       | I2C     | I2C clock line (LCD 0x27)   | N/A    |

## Wiring Diagram Summary

```
ESP32                          ESP8266/NodeMCU
┌─────────────┐               ┌──────────────┐
│ GPIO 2 ─────┼── LED         │ D4 (GPIO 2) ─┼── LED
│ GPIO 5 ─────┼── DHT11/22    │ D1 (GPIO 5) ─┼── DHT11
│ GPIO 18 ────┼── MQ-2 DO     │ D5 (GPIO14) ─┼── MQ-2 DO
│ GPIO 21 ────┼── LCD SDA     │ D2 (GPIO 4) ─┼── LCD SDA
│ GPIO 22 ────┼── LCD SCL     │ D1 (GPIO 5) ─┼── LCD SCL (shared w/ DHT!)
│ GPIO 26 ────┼── Light Relay │ D6 (GPIO12) ─┼── Light Relay
│ GPIO 27 ────┼── Fan Relay   │ D7 (GPIO13) ─┼── Fan Relay
└─────────────┘               └──────────────┘
```

## Notes

- **LCD**: I2C address `0x27`, 16x2 character display
- **MQ-2**: Digital output (DO) — LOW = gas detected, HIGH = safe
- **Gas LED**: Blinks at 500ms intervals when gas detected
- **DHT11 vs DHT22**: `web_sever.ino` and `web_sever_esp8266.ino` use DHT11; `web_sever_lite.ino` uses DHT22
- **ESP8266 caveat**: LCD SCL (D1/GPIO5) shares the same pin as the DHT11 data line — verify wiring compatibility
