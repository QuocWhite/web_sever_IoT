# Web Server Lite - Hệ Thống Giám Sát IoT ESP32

## Tổng Quan Dự Án

Đây là **phiên bản rút gọn** của `web_sever.ino` gốc, đã loại bỏ hệ thống nhắn tin SMS. Phiên bản này giữ lại tất cả chức năng giám sát và điều khiển IoT cốt lõi — đọc cảm biến, bảng điều khiển web, màn hình LCD, tự động bật quạt khi phát hiện khí gas và giám sát mức nước.

### Khác Biệt So với Bản Gốc

| Tính năng | Gốc (`web_sever.ino`) | Lite (`web_sever_lite.ino`) |
|---|---|---|
| Giám sát nhiệt độ / độ ẩm | Có | Có |
| Phát hiện gas với quạt tự động | Có | Có |
| Giám sát mức nước | Không | **Đã thêm** |
| LED báo nhiệt độ cao | Không | **Đã thêm** |
| Màn hình LCD 16x2 | Có | Có |
| Bảng điều khiển web (Chart.js) | Có | Có |
| Điều khiển đèn / quạt từ xa | Có | Có |
| Cảnh báo SMS (SMSChef API) | Có | **Đã loại bỏ** |
| Lệnh `PHONE` qua serial | Có | **Đã loại bỏ** |
| Hàm `extractQuoted()` | Có | **Đã loại bỏ** |
| `TEMP_THRESHOLD` / `GAS_DETECTED` | Có | **Đã loại bỏ** |
| Xuất số điện thoại qua serial | Có | **Đã loại bỏ** |
| Cảnh báo serial dạng `PHONE:` | Có | **Đã loại bỏ** |

---

## Yêu Cầu Phần Cứng

| Linh kiện | Chân | Mô tả |
|---|---|---|
| ESP32 Dev Module | — | Vi điều khiển (WiFi + BLE) |
| DHT11 | GPIO 5 | Cảm biến nhiệt độ & độ ẩm |
| MQ-2 | GPIO 18 | Cảm biến khí gas (THẤP = có gas) |
| Cảm biến mức nước | GPIO 34 | Cảm biến mức nước (analog) |
| Đèn (Relay) | GPIO 26 | Ngõ ra — đèn hoặc relay chiếu sáng |
| Quạt (Relay) | GPIO 27 | Ngõ ra — quạt thông gió |
| LED báo nhiệt | GPIO 2 | Đèn báo nhiệt độ cao (>35°C) |
| LCD 16x2 I2C | I2C (0x27) | Hiển thị thông số cảm biến và trạng thái thiết bị |

---

## Kiến Trúc Hệ Thống

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
│  │  (Relay TẮT)     │                    │                    │   │
│  │                  ▼                    ▼                    │   │
│  │           ┌──────────┐       ┌───────────────┐            │   │
│  │           │ Thành    │       │ GET /, /data, │            │   │
│  │           │ công?    │       │ /26/on, /off, │            │   │
│  │           │  ┌─┐    │       │ /27/on, /off  │            │   │
│  │           │  └─┘    │       └───────────────┘            │   │
│  │           │   │     │                                     │   │
│  │           │ Không   │                                     │   │
│  │           │   ▼     │                                     │   │
│  │           │ Serial  │                                     │   │
│  │           │ Chờ lệnh│                                     │   │
│  │           │ WIFI    │                                     │   │
│  │  └───────────┴─────────┴─────────────────────────────────────┘   │
│  │                                                                  │
│  │  ┌──────────────── loop() ───────────────────────────────────┐   │
│  │  │                                                           │   │
│  │  │    server.handleClient()     ◄────── Yêu cầu HTTP         │   │
│  │  │         │                                                │   │
│  │  │         ▼                                                │   │
│  │  │    ┌──────────┐                                          │   │
│  │  │    │ MQ-2 ==  │── CAO ──▶ Kiểm tra cooldown quạt         │   │
│  │  │    │ THẤP?    │          Nếu quạt tự động &              │   │
│  │  │    │ (Có gas?)│          đã 5s → tắt quạt                 │   │
│  │  │    └────┬─────┘                                          │   │
│  │  │         │ THẤP (Có gas)                                  │   │
│  │  │         ▼                                                │   │
│  │  │    ┌──────────────────┐                                   │   │
│  │  │    │ Ghi thời điểm    │                                   │   │
│  │  │    │ Bật quạt (nếu    │                                   │   │
│  │  │    │ đang tắt)        │                                   │   │
│  │  │    └──────────────────┘                                   │   │
│  │  │                                                           │   │
│  │  │    ┌──────────────────┐                                   │   │
│  │  │    │ LED nhiệt: BẬT   │                                   │   │
│  │  │    │ nếu nhiệt > 35°C │                                   │   │
│  │  │    └──────────────────┘                                   │   │
│  │  │                                                           │   │
│  │  │    ┌──────────────────┐                                   │   │
│  │  │    │ Cập nhật LCD 2s  │                                   │   │
│  │  │    │ Đọc DHT11 +      │                                   │   │
│  │  │    │ mức nước         │                                   │   │
│  │  │    │ Bộ lọc Kalman    │                                   │   │
│  │  │    │ Hiển thị LCD     │                                   │   │
│  │  │    └──────────────────┘                                   │   │
│  │  └───────────────────────────────────────────────────────────┘   │
│  │                                                                  │
│  │  ┌─────────── CẢM BIẾN ────────────┐  ┌───── THIẾT BỊ ───────┐ │
│  │  │  DHT11 (GPIO 5)                 │  │  Đèn (GPIO 26)        │ │
│  │  │    Nhiệt độ + Độ ẩm            │  │  Quạt (GPIO 27)       │ │
│  │  │         │                       │  │  LED Nhiệt (GPIO 2)  │ │
│  │  │         ▼                       │  │  LCD 16x2 I2C (0x27)  │ │
│  │  │  Bộ lọc Kalman                  │  └───────────────────────┘ │
│  │  │         │                       │                            │
│  │  │         ▼                       │                            │
│  │  │  Dữ liệu cho LCD + API JSON     │                            │
│  │  │                                 │                            │
│  │  │  MQ-2 (GPIO 18)                 │                            │
│  │  │    THẤP = Có gas                │                            │
│  │  │                                 │                            │
│  │  │  Mức nước (GPIO 34, ADC)        │                            │
│  │  │    < 50 = Bể Cạn                │                            │
│  │  │    >= 50 = Bể đầy               │                            │
│  │  └─────────────────────────────────┘                            │
│  │                                                                  │
│  │  ┌──────────── GIAO TIẾP SERIAL (115200) ────────────────────┐  │
│  │  │  WIFI "SSID" "MẬT_KHẨU"  │  STATUS  │  DISCONNECT         │  │
│  │  └────────────────────────────────────────────────────────────┘  │
│  └──────────────────────────────────────────────────────────────────┘
```

---

## Luồng Xử Lý Code

### 1. `setup()` — Khởi tạo

```
Bật nguồn
    │
    ▼
Serial.begin(115200)
    │
    ▼
Khởi tạo GPIO:
  - GPIO 26, 27 → OUTPUT, LOW (tắt đèn & quạt)
  - GPIO 2      → OUTPUT, LOW (tắt LED nhiệt)
  - GPIO 18     → INPUT  (cảm biến gas MQ-2)
    │
    ▼
tryConnect(ssid, pass, 10s)
    │
    ├── THÀNH CÔNG → in địa chỉ IP
    │
    └── THẤT BẠI  → waitForWiFiCommand()
                      (vòng lặp chờ lệnh WIFI qua serial)
    │
    ▼
setupWebServer()  → đăng ký các route HTTP
    │
    ▼
setupLCD()       → khởi tạo I2C, hiện splash, xóa màn hình
    │
    ▼
Sẵn sàng (vào loop())
```

### 2. `loop()` — Vòng lặp chính

```
loop()
    │
    ├── server.handleClient()    ← Xử lý yêu cầu HTTP đến
    │
    ├── Logic phát hiện gas:
    │     │
    │     ├── MQ-2 == THẤP (có gas)?
    │     │     ├── Ghi lại thời điểm
    │     │     └── Nếu quạt TẮT → bật quạt (chế độ tự động)
    │     │
    │     └── MQ-2 == CAO (an toàn)?
    │           └── Nếu quạt tự động VÀ đã 5s → tắt quạt
    │
    ├── LED nhiệt độ:
    │     └── Nếu nhiệt > 35°C → BẬT LED, ngược lại → TẮT
    │
    └── Cập nhật LCD (mỗi 2000ms):
          ├── Đọc DHT11, cảm biến mức nước
          ├── Áp dụng bộ lọc Kalman
          └── updateLCD(temp, hum, waterLevel, gasDetected)
```

### 3. Xử Lý Yêu Cầu HTTP

| Route | Bộ xử lý | Mô tả |
|---|---|---|
| `GET /` | `handleRoot()` | Phục vụ bảng điều khiển HTML với Chart.js |
| `GET /data` | `handleData()` | Trả về JSON: `{temperature, humidity, waterLevel, gas, light, fan}` |
| `GET /style.css` | `handleStyle()` | Phục vụ CSS từ PROGMEM |
| `GET /26/on` | `handleGPIO26On()` | Bật GPIO 26 CAO, trả về HTML đã cập nhật |
| `GET /26/off` | `handleGPIO26Off()` | Tắt GPIO 26 THẤP, trả về HTML đã cập nhật |
| `GET /27/on` | `handleGPIO27On()` | Bật GPIO 27 CAO, xóa cờ auto, trả về HTML |
| `GET /27/off` | `handleGPIO27Off()` | Tắt GPIO 27 THẤP, xóa cờ auto, trả về HTML |

### 4. Giao Diện Người Dùng (JavaScript)

```
Tải trang
    │
    ├── Đọc trạng thái GPIO ban đầu → gán class CSS cho icon
    │
    ├── Tạo hai biểu đồ Chart.js dạng đường:
    │     - Biểu đồ độ ẩm (xanh dương, khoảng 30–100%)
    │     - Biểu đồ nhiệt độ (đỏ, khoảng 2–70°C)
    │
    └── updateData() gọi ngay, sau đó mỗi 5000ms:
          │
          ├── fetch('/data')
          │     │
          │     ├── Cập nhật giá trị (nhiệt, ẩm, mức nước, gas)
          │     ├── Chuyển class icon (đèn sáng, quạt quay)
          │     └── Thêm điểm dữ liệu vào biểu đồ (tối đa 20)
          │
          └── Lỗi: im lặng, thử lại sau 5s
```

---

## Giải Thuật Chính

### 1. Bộ Lọc Kalman cho Cảm Biến

**Mục đích:** Giảm nhiễu và loại bỏ các đột biến nhất thời từ cảm biến DHT11.

**Trạng thái:**
- `estimate` — giá trị đã lọc hiện tại
- `errorEstimate` — độ không chắc chắn của ước tính
- `errorMeasurement` — phương sai nhiễu của cảm biến (cố định 4.0)
- `errorProcess` — nhiễu quá trình (cố định 0.01)

**Giải thuật (mỗi lần cập nhật):**

```
function update(measurement):
    nếu measurement là NaN hoặc ≤ 0:
        trả về estimate  // bỏ qua giá trị không hợp lệ
    
    // Tính Kalman Gain (0 đến 1)
    K = errorEstimate / (errorEstimate + errorMeasurement)
    
    // Cập nhật ước tính với trọng số
    estimate = estimate + K × (measurement - estimate)
    
    // Cập nhật độ không chắc chắn
    errorEstimate = (1 - K) × errorEstimate + errorProcess
    
    trả về estimate
```

**Đặc điểm:**
- Ban đầu `K ≈ 0.2` (tin tưởng cảm biến chậm)
- Dần dần `K → 0.0025` (lọc mạnh, đầu ra mượt)
- Phục hồi nhanh nếu có thay đổi lớn kéo dài
- Giá trị không hợp lệ (`NaN`, 0, âm) được bỏ qua

### 2. Phát Hiện Gas và Tự Động Bật Quạt

**Kích hoạt:** Ngõ ra MQ-2 là active-low — `THẤP` = có gas, tự động bật quạt.

**Logic:**
```
nếu mq2 == THẤP:
    ghi thời điểm
    nếu quạt ĐANG TẮT:
        bật quạt (đánh dấu là tự động)

ngược lại (không có gas):
    nếu quạt tự động VÀ đã 5s:
        tắt quạt
        xóa cờ tự động
```

### 3. LED Báo Nhiệt Độ Cao

LED GPIO 2 báo hiệu nhiệt độ:
- BẬT khi nhiệt độ > 35°C
- TẮT khi nhiệt độ ≤ 35°C

### 4. Giám Sát Mức Nước

Cảm biến mức nước analog (GPIO 34, ADC):
- Giá trị < 50 → "Bể Cạn"
- Giá trị ≥ 50 → "Bể đầy"

### 5. Định Thời Không Chặn (`millis()`)

| Thao tác | Chu kỳ | Cơ chế |
|---|---|---|
| Cập nhật LCD | 2000 ms | `if (now - lastLCDUpdate >= LCD_UPDATE_INTERVAL_MS)` |
| Poll dữ liệu biểu đồ (JS) | 5000 ms | `setInterval(updateData, 5000)` |
| Tự động tắt quạt | 5000 ms | Tính từ lần cuối phát hiện gas |
| Kết nối WiFi | 500 ms | Delay giữa các dấu chấm thử lại |

### 6. Phân Tích Lệnh Serial

Các lệnh khả dụng trong phiên bản lite:
- `WIFI "SSID" "MẬT_KHẨU"` — kết nối WiFi
- `STATUS` — hiển thị trạng thái WiFi hiện tại
- `DISCONNECT` — ngắt kết nối WiFi

### 7. Bố Trí Màn Hình LCD (16×2)

```
┌──────────────────┐
│ N:25.0°C A:60.0% │  ← Hàng 0: nhiệt độ + độ ẩm
│ W:1024 GAS:OK D:0│  ← Hàng 1: mức nước, gas, thiết bị
└──────────────────┘
```

- `N:` nhiệt độ (Nhiệt độ) với ký hiệu độ (byte 223)
- `A:` độ ẩm (Ẩm)
- `W:` giá trị mức nước
- `GAS:!` khi phát hiện gas, `GAS:OK` khi an toàn
- `D:0/1` trạng thái relay đèn (Đèn)
- `Q:0/1` trạng thái relay quạt (Quạt)

---

## Bảng Điều Khiển Web

```
┌────────────────────────────────────────────┐
│  ● TRẠM QUAN TRẮC ESP32                    │
│                                            │
│  ┌────────┐ ┌────────┐ ┌────────────────┐  │
│  │ 24.5°C │ │  60%   │ │     1024       │  │
│  │Nhiệt độ│ │ Độ ẩm  │ │ MỨC NƯỚC TRONG │  │
│  │        │ │        │ │ BỂ: Bể đầy     │  │
│  └────────┘ └────────┘ └────────────────┘  │
│  ┌────────────────┐                        │
│  │  CẢM BIẾN GAS  │                        │
│  │  AN TOAN       │                        │
│  └────────────────┘                        │
│                                            │
│  ┌──────────────────────────────────────┐  │
│  │  Độ ẩm (%) — biểu đồ đường (xanh)    │  │
│  │  [Chart.js, cửa sổ 20 điểm]          │  │
│  └──────────────────────────────────────┘  │
│                                            │
│  ┌──────────────────────────────────────┐  │
│  │  Nhiệt độ (°C) — biểu đồ đường (đỏ)  │  │
│  └──────────────────────────────────────┘  │
│                                            │
│  ┌─ Điều khiển ──────────────────────────┐ │
│  │ 💡 Đèn (GPIO 26)          [BẬT] [TẮT] │ │
│  │ ⏳ Quạt (GPIO 27)         [BẬT] [TẮT] │ │
│  └───────────────────────────────────────┘ │
└────────────────────────────────────────────┘
```

---

## Lệnh Serial

Kết nối với ESP32 qua serial monitor ở tốc độ **115200 baud**.

| Lệnh | Mô tả |
|---|---|
| `WIFI "SSID" "MẬT_KHẨU"` | Kết nối đến mạng WiFi |
| `STATUS` | In trạng thái WiFi và địa chỉ IP |
| `DISCONNECT` | Ngắt kết nối WiFi hiện tại |

---

## Bảng Ghép Nối Chân

| GPIO | Linh kiện | Loại | Chức năng |
|------|-----------|------|-----------|
| 2 | Đèn báo nhiệt | OUTPUT | LED cảnh báo nhiệt độ cao (>35°C) |
| 5 | DHT11 | INPUT | Cảm biến nhiệt độ & độ ẩm |
| 18 | MQ-2 | INPUT | Cảm biến khí gas (THẤP = có gas) |
| 21 | LCD SDA | I2C | Đường dữ liệu I2C (LCD 0x27) |
| 22 | LCD SCL | I2C | Đường xung nhịp I2C (LCD 0x27) |
| 26 | Đèn | OUTPUT | Điều khiển relay đèn |
| 27 | Quạt | OUTPUT | Điều khiển relay quạt |
| 34 | Cảm biến mức nước | INPUT (ADC) | Cảm biến mức nước (analog) |

## Bắt Đầu Nhanh

```bash
# 1. Cài đặt hỗ trợ board ESP32 trong Arduino IDE
# 2. Cài đặt các thư viện cần thiết:
#    - WebServer (có sẵn trong ESP32 core)
#    - Thư viện DHT sensor của Adafruit
#    - LiquidCrystal_I2C của F. Malpartida / M. Schwartz
# 3. Mở web_sever_lite.ino trong Arduino IDE
# 4. Sửa thông tin WiFi ở đầu file:
#      const char* ssid = "TenWiFiCuaBan";
#      const char* password = "MatKhauWiFi";
# 5. Chọn board: ESP32 Dev Module
# 6. Chọn cổng COM phù hợp
# 7. Nạp code lên ESP32
# 8. Mở Serial Monitor (115200) để xem địa chỉ IP
# 9. Truy cập http://<IP_ESP32>/ trên trình duyệt
```

---

## Cấu Hình

Sửa ở đầu file `web_sever_lite.ino`:

```cpp
const char* ssid = "TenWiFiCuaBan";
const char* password = "MatKhauWiFi";
```

---

## Cấu Trúc Thư Mục

```
web_sever_lite/
├── web_sever_lite.ino    # File Arduino chính (một file duy nhất)
├── README.md             # Tài liệu tiếng Anh
└── README_vi.md          # Tài liệu tiếng Việt (file này)
```

---

## Giấy Phép

Giống như dự án gốc.
