import requests
import serial
import time
import re
import serial.tools.list_ports

API_KEY = "1d16053493371ee2f3af0e04290fdb9a53e1f8d7"
DEVICE_UUID = "c522522de2020d78"
PHONE_NUMBER = None
TEMP_THRESHOLD = 35.0
GAS_DETECTED = 1
BAUD_RATE = 115200

SERIAL_TIMEOUT_SECONDS = 1
ESP_CONNECT_WAIT_SECONDS = 2
POLL_INTERVAL_SECONDS = 0.1
RETRY_WAIT_SECONDS = 2

SIM_SLOT = 1
PRIORITY_LEVEL = 1

ser = None
last_temp_alert_time = 0
last_gas_alert_time = 0
TEMP_COOLDOWN = 60
GAS_COOLDOWN = 60

HTTP_OK = 200
SMS_STATUS_OK = 200

def send_sms(phone, message):
    url = "https://www.cloud.smschef.com/api/send/sms"
    data = {
        "secret": API_KEY,
        "mode": "devices",
        "device": DEVICE_UUID,
        "sim": SIM_SLOT,
        "priority": PRIORITY_LEVEL,
        "phone": phone,
        "message": message
    }
    response = requests.post(url, data=data)
    return response.json()

def parse_sensor_data(line):
    match = re.match(r'T:([\d.]+)\|H:([\d.]+)\|G:(\d+)', line.strip())
    if match:
        temperature = float(match.group(1))
        humidity = float(match.group(2))
        gas_detected = int(match.group(3))
        return temperature, humidity, gas_detected
    return None, None, None

def parse_phone_number(line):
    global PHONE_NUMBER
    match = re.match(r'PHONE:([^|]+)', line.strip())
    if match:
        PHONE_NUMBER = match.group(1).strip()
        print(f"Phone number updated: {PHONE_NUMBER}")
    return PHONE_NUMBER

def check_and_alert(temperature, humidity, gas_detected):
    global last_temp_alert_time, last_gas_alert_time
    
    if not PHONE_NUMBER:
        print("Phone number not set. Waiting for PHONE command...")
        return
    
    current_time = time.time()
    messages = []
    
    if temperature > TEMP_THRESHOLD and (current_time - last_temp_alert_time >= TEMP_COOLDOWN):
        messages.append(f"OVERHEAT ALERT: Temperature is {temperature:.1f}C (threshold: {TEMP_THRESHOLD}C)")
        last_temp_alert_time = current_time
    
    if gas_detected == GAS_DETECTED and (current_time - last_gas_alert_time >= GAS_COOLDOWN):
        messages.append("GAS LEAK DETECTED! Please check immediately!")
        last_gas_alert_time = current_time
    
    if messages:
        full_message = " | ".join(messages)
        print(f"Sending alert: {full_message}")
        result = send_sms(PHONE_NUMBER, full_message)
        print(f"SMS Result: {result}")

def find_esp32_port():
    ports = list(serial.tools.list_ports.comports())
    
    print("Available serial ports:")
    for p in ports:
        print(f"  {p.device} - {p.description}")
    
    for p in ports:
        if "USB" in p.device or "ACM" in p.device:
            print(f"Using {p.device}")
            return p.device
    
    if ports:
        print(f"Using {ports[0].device}")
        return ports[0].device

    return None

def main():
    global ser
    
    port = find_esp32_port()
    if not port:
        print("No serial port found. Connect ESP32 and try again.")
        while True:
            port = find_esp32_port()
            if port:
                break
            print("Waiting for ESP32 connection...")
            time.sleep(RETRY_WAIT_SECONDS)
    
    try:
        ser = serial.Serial(port, BAUD_RATE, timeout=SERIAL_TIMEOUT_SECONDS)
        time.sleep(ESP_CONNECT_WAIT_SECONDS)
        print(f"Connected to {port}")
        
        while True:
            if ser.in_waiting:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line.startswith("PHONE:"):
                    parse_phone_number(line)
                elif line.startswith("T:"):
                    print(f"Received: {line}")
                    temperature, humidity, gas_detected = parse_sensor_data(line)
                    if temperature is not None:
                        print(f"Temperature: {temperature}C, Humidity: {humidity}%, Gas: {gas_detected}")
                        check_and_alert(temperature, humidity, gas_detected)
            time.sleep(POLL_INTERVAL_SECONDS)
            
    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except KeyboardInterrupt:
        print("\nStopped by user")
    finally:
        if ser and ser.is_open:
            ser.close()

if __name__ == "__main__":
    main()
