import requests
import serial
import time
import re
import os
import json
import subprocess
import platform

API_KEY = "1d16053493371ee2f3af0e04290fdb9a53e1f8d7"
DEVICE_UUID = "c522522de2020d78"
TEMP_THRESHOLD = 35.0
BAUD_RATE = 9600

SERIAL_TIMEOUT_SECONDS = 1
ESP_CONNECT_WAIT_SECONDS = 2
POLL_INTERVAL_SECONDS = 0.1
RETRY_WAIT_SECONDS = 2

SIM_SLOT = 1
PRIORITY_LEVEL = 1

QR_IMAGE = "QR.jpg"

config_cache = {}

ser = None
last_temp_alert_time = 0
last_gas_alert_time = 0
TEMP_COOLDOWN = 60
GAS_COOLDOWN = 60
GAS_DETECTED = 1

PHONE_NUMBER = None
SERIAL_PORT = None
USER_NAME = None
MAC_ADDRESS = None
user_info = {"name": "", "mac": "", "balance": 0, "state": ""}


# ================= MAC ADDRESS =================
def get_mac_address():
    if platform.system() == "Windows":
        result = subprocess.run(["getmac"], capture_output=True, text=True)
        for line in result.stdout.split("\n"):
            match = re.match(r'([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})', line)
            if match:
                return match.group().replace("-", ":").upper()
    else:
        mac = ""
        if platform.system() == "Linux":
            for iface in ["wlan0", "eth0", "wlan1", "en0"]:
                try:
                    with open(f"/sys/class/net/{iface}/address", "r") as f:
                        mac = f.read().strip().upper()
                        if mac and mac != "00:00:00:00:00:00":
                            return mac
                except:
                    pass
        if not mac:
            result = subprocess.run(["ip", "link"], capture_output=True, text=True)
            match = re.search(r'([0-9a-fA-F]{2}:){5}[0-9a-fA-F]{2}', result.stdout)
            if match:
                return match.group().upper()
    return "UNKNOWN"


# ================= CONFIG (Google Sheet) =================
def get_config(key, default=""):
    global config_cache
    if key in config_cache:
        return config_cache[key]
    
    result = call_script_api("getconfig", key=key)
    if result.get("success"):
        value = result.get("value", default)
        config_cache[key] = value
        return value
    return default


def set_config(key, value):
    global config_cache
    result = call_script_api("setconfig", key=key, value=value)
    if result.get("success"):
        config_cache[key] = value
        return True
    return False


# ================= GOOGLE SHEET API =================
def call_script_api(action, mac=None, name=None, messages=None):
    params = {"action": action}
    if mac:
        params["mac"] = mac
    if name:
        params["name"] = name
    if messages is not None:
        params["messages"] = str(messages)

    try:
        response = requests.get(SCRIPT_URL, params=params, timeout=10)
        return response.json()
    except Exception as e:
        print(f"[API ERROR] {e}")
        return {"success": False, "message": str(e)}


def check_user_exists(mac):
    result = call_script_api("check", mac=mac)
    return result.get("success", False)


def register_user(name, mac):
    result = call_script_api("register", mac=mac, name=name)
    if result.get("success"):
        global user_info
        user_info = {
            "name": result.get("name", name),
            "mac": result.get("mac", mac),
            "balance": result.get("balance", 10000),
            "state": result.get("state", "ng")
        }
        if result.get("state") == "ng":
            send_admin_email(name, mac, "New user registered -需要审核")
    return result


def get_user_info(mac):
    result = call_script_api("get", mac=mac)
    if result.get("success"):
        global user_info
        user_info = {
            "name": result.get("name", ""),
            "mac": result.get("mac", mac),
            "balance": result.get("balance", 0),
            "state": result.get("state", "")
        }
    return result


def deduct_balance(mac):
    result = call_script_api("deduct", mac=mac)
    if result.get("success"):
        global user_info
        user_info["balance"] = result.get("newBalance", 0)
    return result


def get_message_cost(balance):
    messages_available = balance // MESSAGE_COST_BULK
    if messages_available >= 10:
        return MESSAGE_COST_BULK
    else:
        return MESSAGE_COST_SINGLE


def can_send_message():
    cost = get_message_cost(user_info["balance"])
    return user_info["balance"] >= cost


def send_admin_email(name, mac, message):
    subject = f"[ESP Alert] User: {name} - {mac}"
    body = f"User: {name}\nMAC: {mac}\nMessage: {message}"
    print(f"[EMAIL] Would send to {ADMIN_EMAIL}: {subject}")
    print(f"[EMAIL] Body: {body}")


# ================= CONFIG =================
def load_config():
    global PHONE_NUMBER, SERIAL_PORT, USER_NAME, MAC_ADDRESS, TEMP_THRESHOLD

    phone = get_config("PhoneNumber")
    port = get_config("SerialPort")
    temp = get_config("TempThreshold")
    
    if phone:
        PHONE_NUMBER = phone
    if port:
        SERIAL_PORT = port
    if temp:
        try:
            TEMP_THRESHOLD = float(temp)
        except:
            pass

    if not PHONE_NUMBER:
        print("\n=== First-time Configuration ===")
        PHONE_NUMBER = input("Enter phone number (e.g., +84352480097): ").strip()
        set_config("PhoneNumber", PHONE_NUMBER)

    if not SERIAL_PORT:
        SERIAL_PORT = input("Enter COM port (e.g., COM3 or /dev/ttyUSB0): ").strip()
        set_config("SerialPort", SERIAL_PORT)

    set_config("TempThreshold", str(TEMP_THRESHOLD))
    print("Configuration saved to Google Sheet!\n")

    if not MAC_ADDRESS:
        MAC_ADDRESS = get_mac_address()
        print(f"[MAC] Detected: {MAC_ADDRESS}")

    if not check_user_exists(MAC_ADDRESS):
        print("\n=== First-time Registration ===")
        print(f"MAC Address: {MAC_ADDRESS}")
        USER_NAME = input("Enter your name: ").strip()

        set_config("UserName", USER_NAME)
        set_config("MAC", MAC_ADDRESS)

        print("\n[INFO] Registering user...")
        result = register_user(USER_NAME, MAC_ADDRESS)
        if result.get("success"):
            print(f"[SUCCESS] Registered!")
            print(f"  Name: {user_info['name']}")
            print(f"  MAC: {user_info['mac']}")
            print(f"  Balance: {user_info['balance']:,} VND (Free message)")
            print(f"  State: {user_info['state']} (ng=pending, ok=approved)")
        else:
            print(f"[ERROR] {result.get('message')}")
    else:
        stored_name = get_config("UserName")
        if stored_name:
            USER_NAME = stored_name
        get_user_info(MAC_ADDRESS)
        print(f"\n[INFO] Welcome back, {user_info['name']}!")
        print(f"[INFO] Balance: {user_info['balance']:,} VND")
        print(f"[INFO] State: {user_info['state']}")


# ================= BALANCE =================
def show_payment_info():
    print("\n" + "=" * 50)
    print("        INSUFFICIENT BALANCE - TOP UP REQUIRED")
    print("=" * 50)
    cost = get_message_cost(user_info["balance"])
    messages_available = user_info["balance"] // cost
    print(f"Current Cost: {cost:,} VND/message")
    print(f"Current Balance: {user_info['balance']:,} VND ({messages_available} messages)")
    print(f"Bank: {BANK_NAME}")
    print(f"Account Number: {BANK_ACCOUNT}")
    print(f"Account Name: {ACCOUNT_NAME}")
    print("=" * 50 + "\n")

    if os.path.exists(QR_IMAGE):
        print(f"[INFO] QR Code image available: {QR_IMAGE}")
    print("\n")


def top_up_balance():
    print("\n========== TOP UP MESSAGES ==========")
    cost = get_message_cost(user_info["balance"])
    messages_available = user_info["balance"] // cost
    print(f"Current cost: {cost:,} VND/message")
    print(f"Current balance: {messages_available} messages ({user_info['balance']:,} VND)")
    print("\nChoose number of messages to buy:")
    print("  1) 1 message   (10,000 VND)")
    print("  2) 5 messages  (50,000 VND)")
    print("  3) 10 messages (50,000 VND) - Best value!")
    print("  4) 20 messages (100,000 VND)")
    print("  5) 50 messages (250,000 VND)")
    print("  6) Custom amount")
    print("  q) Show QR Code")
    print("  c) Cancel")
    print("================================")
    print("Transfer to:")
    print(f"  Bank: {BANK_NAME}")
    print(f"  Account: {BANK_ACCOUNT}")
    print(f"  Name: {ACCOUNT_NAME}")
    print("\nAfter transfer, enter your choice to confirm: ", end="")

    choice = input().strip().lower()

    if choice == "c" or not choice:
        print("\n[CANCELLED]")
        return False

    if choice == "q":
        if os.path.exists(QR_IMAGE):
            if platform.system() == "Windows":
                os.startfile(QR_IMAGE)
            elif platform.system() == "Darwin":
                subprocess.run(["open", QR_IMAGE])
            else:
                subprocess.run(["xdg-open", QR_IMAGE])
            print("[INFO] Opened QR Code")
        else:
            print(f"[ERROR] QR image not found: {QR_IMAGE}")
        return top_up_balance()

    message_count = 0

    options = {
        "1": 1,
        "2": 5,
        "3": 10,
        "4": 20,
        "5": 50,
    }

    if choice == "6":
        try:
            print("\nEnter number of messages: ", end="")
            count = int(input().strip())
            if count > 0:
                message_count = count
            else:
                print("\n[ERROR] Invalid number")
                return False
        except ValueError:
            print("\n[ERROR] Invalid input")
            return False
    elif choice in options:
        message_count = options[choice]
    else:
        print("\n[ERROR] Invalid choice")
        return False

    cost_per_msg = MESSAGE_COST_SINGLE if message_count < 10 else MESSAGE_COST_BULK
    total_cost = message_count * cost_per_msg

    print(f"\n[INFO] Please transfer {total_cost:,} VND ({message_count} messages x {cost_per_msg:,} VND)")
    print("Press 'y' after transfer to confirm, 'c' to cancel: ", end="")
    confirm = input().strip().lower()

    if confirm == "y":
        result = call_script_api("topup", mac=MAC_ADDRESS, messages=message_count)
        if result.get("success"):
            user_info["balance"] = result.get("newBalance", user_info["balance"])
            print(f"\n[SUCCESS] Added {message_count} message credits ({total_cost:,} VND)")
            print(f"New balance: {user_info['balance']:,} VND ({user_info['balance'] // MESSAGE_COST_BULK} messages)")
            return True
        else:
            print(f"\n[ERROR] {result.get('message')}")
            return False
    else:
        print("\n[CANCELLED]")
        return False


# ================= SMS =================
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

    try:
        response = requests.post(url, data=data, timeout=10)
        return response.json()
    except Exception as e:
        return {"error": str(e)}


# ================= SENSOR PARSE =================
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
        print(f"[PHONE] Updated to: {PHONE_NUMBER}")
    return PHONE_NUMBER


# ================= ALERT =================
def check_and_alert(temperature, humidity, gas_detected):
    global last_temp_alert_time, last_gas_alert_time

    if not PHONE_NUMBER:
        print("[INFO] Phone number not set. Waiting for PHONE command...")
        return False

    current_time = time.time()
    messages = []

    if temperature > TEMP_THRESHOLD and (current_time - last_temp_alert_time >= TEMP_COOLDOWN):
        messages.append(f"OVERHEAT ALERT: {temperature:.1f}C (>{TEMP_THRESHOLD}C)")
        last_temp_alert_time = current_time

    if gas_detected == GAS_DETECTED and (current_time - last_gas_alert_time >= GAS_COOLDOWN):
        messages.append("GAS LEAK DETECTED! CHECK IMMEDIATELY!")
        last_gas_alert_time = current_time

    if messages:
        full_message = " | ".join(messages)
        print(f"[ALERT] {full_message}")

        get_user_info(MAC_ADDRESS)
        cost = get_message_cost(user_info["balance"])

        if not can_send_message():
            print(f"[BLOCKED] Insufficient balance! Need {cost:,} VND")
            show_payment_info()
            if os.path.exists(QR_IMAGE):
                print(f"[INFO] Press 'q' to view QR Code")
            result = top_up_balance()
            if not can_send_message():
                send_admin_email(user_info["name"], MAC_ADDRESS, "User needs more balance")
                print("[WARNING] Message NOT sent - no credit available")
                return False

        result = send_sms(PHONE_NUMBER, full_message)
        print(f"[SMS RESULT] {result}")

        result = deduct_balance(MAC_ADDRESS)
        if result.get("success"):
            print(f"[BALANCE] Remaining: {user_info['balance']:,} VND")

        return True

    return False


# ================= SERIAL CONNECT =================
def connect_serial():
    global SERIAL_PORT

    while True:
        try:
            print(f"Connecting to {SERIAL_PORT}...")
            ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=SERIAL_TIMEOUT_SECONDS)
            time.sleep(ESP_CONNECT_WAIT_SECONDS)
            print(f"Connected to {SERIAL_PORT}\n")
            return ser

        except serial.SerialException as e:
            print(f"[ERROR] Cannot open {SERIAL_PORT}: {e}")

            choice = input("Retry (r) / Change port (c) / Exit (e): ").lower()

            if choice == "c":
                SERIAL_PORT = input("Enter new COM port: ").strip()
                set_config("SerialPort", SERIAL_PORT)
            elif choice == "e":
                exit()

            time.sleep(RETRY_WAIT_SECONDS)


# ================= MAIN =================
def main():
    global ser

    load_config()

    print(f"\n[INFO] Message balance: {user_info['balance']:,} VND")
    cost = get_message_cost(user_info["balance"])
    print(f"[INFO] Current cost: {cost:,} VND/message\n")

    ser = connect_serial()

    try:
        while True:
            if ser.in_waiting:
                line = ser.readline().decode('utf-8', errors='ignore').strip()

                if line.startswith("PHONE:"):
                    parse_phone_number(line)
                elif line.startswith("T:"):
                    print(f"[RAW] {line}")

                    temperature, humidity, gas_detected = parse_sensor_data(line)

                    if temperature is not None:
                        print(f"[DATA] Temp: {temperature}C | Humidity: {humidity}% | Gas: {gas_detected}")
                        check_and_alert(temperature, humidity, gas_detected)

            time.sleep(POLL_INTERVAL_SECONDS)

    except serial.SerialException as e:
        print(f"[SERIAL ERROR] {e}")

    except KeyboardInterrupt:
        print("\nStopped by user")

    finally:
        if ser and ser.is_open:
            ser.close()
            print("Serial connection closed")


# ================= RUN =================
if __name__ == "__main__":
    main()
