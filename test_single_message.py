import requests

SECRET = "1d16053493371ee2f3af0e04290fdb9a53e1f8d7"
DEVICE = "c522522de2020d78"
PHONE = "+84373548547"
#PHONE = "+84352480097"

SIM_SLOT = 1
PRIORITY_LEVEL = 1
HTTP_OK = 200
SMS_STATUS_OK = 200

URL = "https://www.cloud.smschef.com/api/send/sms"
DATA = {
    "secret": SECRET,
    "mode": "devices",
    "device": DEVICE,
    "sim": SIM_SLOT,
    "priority": PRIORITY_LEVEL,
    "phone": PHONE,
    "message": "hehe"
}

response = requests.post(URL, data=DATA)

if response.status_code == HTTP_OK:
    result = response.json()
    if result.get('status') == SMS_STATUS_OK:
        print(f"Success! Message ID: {result['data']['messageId']}")
    else:
        print(f"Error: {result}")
else:
    print(f"HTTP Error: {response.status_code}")
