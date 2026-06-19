# Bảng Ghép Nối Chân (Pin Mapping)

## ESP32 (`web_sever_lite.ino`)

| GPIO | Linh kiện            | Loại       | Chức năng                     | Kích hoạt     |
|------|----------------------|------------|-------------------------------|---------------|
| 2    | Đèn còi báo nhiệt    | OUTPUT     | LED cảnh báo nhiệt độ cao     | CAO (>35°C)   |
| 5    | DHT11                | INPUT      | Cảm biến nhiệt độ & độ ẩm     | N/A           |
| 18   | MQ-2                 | INPUT      | Cảm biến khí gas              | THẤP = có gas |
| 34   | Cảm biến mức nước    | INPUT (ADC)| Cảm biến mức nước (analog)    | N/A           |
| 21   | LCD SDA              | I2C        | Đường dữ liệu I2C (LCD 0x27)  | N/A           |
| 22   | LCD SCL              | I2C        | Đường xung nhịp I2C (LCD 0x27)| N/A           |
| 26   | Đèn                  | OUTPUT     | Điều khiển relay đèn          | CAO           |
| 27   | Quạt                 | OUTPUT     | Điều khiển relay quạt         | CAO           |

## Sơ Đồ Đấu Nối

```
ESP32
┌─────────────────────┐
│ GPIO 2  ────────── LED báo nhiệt (>35°C)
│ GPIO 5  ────────── DHT11
│ GPIO 18 ────────── MQ-2 DO
│ GPIO 21 ────────── LCD SDA
│ GPIO 22 ────────── LCD SCL
│ GPIO 26 ────────── Relay đèn
│ GPIO 27 ────────── Relay quạt
│ GPIO 34 ────────── Cảm biến mức nước (analog)
└─────────────────────┘
```

## Ghi Chú

- **LCD**: Địa chỉ I2C `0x27`, màn hình 16x2 ký tự
- **MQ-2**: Ngõ ra số (DO) — THẤP = phát hiện gas, CAO = an toàn. Khi phát hiện gas, quạt tự động bật.
- **LED báo nhiệt**: Bật khi nhiệt độ > 35°C, tắt khi ≤ 35°C
- **Cảm biến mức nước**: Đọc analog (0-4095). < 50 = "Bể Cạn", ≥ 50 = "Bể đầy"
