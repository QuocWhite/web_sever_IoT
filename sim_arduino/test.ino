#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include <SoftwareSerial.h>

#define DHTPIN 5
#define DHTTYPE DHT11

#define WATER_PIN A0
#define IRPIN A1

#define LEDRED A2
#define BUZZER 12

#define BTN_POS 7
#define BTN_UP 8
#define BTN_DOWN 9
#define BTN_OK 10
#define BTN_CANCEL 11

#define SIM_RX 2
#define SIM_TX 3

LiquidCrystal_I2C lcd(0x27,16,2);
DHT dht(DHTPIN,DHTTYPE);
SoftwareSerial sim(SIM_RX,SIM_TX);

char phone[11] = "0000000000";
int pos = 0;

#define PHONE_DIGITS 10

bool smsSent = false;

void sendSMS()
{
  sim.println("AT");
  delay(1000);

  sim.println("AT+CMGF=1");
  delay(1000);

  sim.print("AT+CMGS=\"");
  sim.print(phone);
  sim.println("\"");
  delay(1000);

  sim.println("CANH BAO CHAY !!!");
  delay(500);

  sim.write(26);
}

void handlePhoneInput() {
  if (digitalRead(BTN_POS) == LOW) {
    pos++;
    if (pos >= PHONE_DIGITS) pos = 0;
    delay(200);
  }

  if (digitalRead(BTN_UP) == LOW) {
    if (phone[pos] < '9')
      phone[pos]++;
    else
      phone[pos] = '0';
    delay(200);
  }

  if (digitalRead(BTN_DOWN) == LOW) {
    if (phone[pos] > '0')
      phone[pos]--;
    else
      phone[pos] = '9';
    delay(200);
  }
}

void setup()
{
  Serial.begin(9600);
  sim.begin(9600);

  dht.begin();

  pinMode(BUZZER,OUTPUT);
  pinMode(LEDRED,OUTPUT);

  pinMode(BTN_POS,INPUT_PULLUP);
  pinMode(BTN_UP,INPUT_PULLUP);
  pinMode(BTN_DOWN,INPUT_PULLUP);
  pinMode(BTN_OK,INPUT_PULLUP);
  pinMode(BTN_CANCEL,INPUT_PULLUP);

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0,0);
  lcd.print("DANG KET NOI....");
  delay(2000);
}

void loop()
{
  handlePhoneInput();

  float t = dht.readTemperature();
  int waterLevel = analogRead(WATER_PIN);
  int irValue = analogRead(IRPIN);

  bool fire = (irValue > 960);
  bool highTemp = (t > 60);
  bool waterHigh = (waterLevel > 700);

  bool alarm = fire || highTemp || waterHigh;

  Serial.print("T:");
  Serial.print(t, 1);
  Serial.print("|H:");
  Serial.print(dht.readHumidity(), 1);
  Serial.print("|W:");
  Serial.println(waterLevel);
  delay(50);

  lcd.setCursor(0,0);
  lcd.print("T:");
  lcd.print(t,1);
  lcd.print("C ");

  lcd.print("W:");
  lcd.print(waterLevel);
  lcd.print("   ");

  lcd.setCursor(0,1);
  lcd.print("sdt: ");
  lcd.print(phone);

  if(alarm)
  {
    if(!smsSent)
    {
      sendSMS();
      smsSent = true;
    }

    if(digitalRead(BTN_CANCEL)==LOW)
    {
      digitalWrite(BUZZER,LOW);
      digitalWrite(LEDRED,LOW);
      return;
    }

    digitalWrite(BUZZER,HIGH);

    digitalWrite(LEDRED,HIGH);
    delay(200);
    digitalWrite(LEDRED,LOW);
    delay(200);
  }
  else
  {
    smsSent=false;

    digitalWrite(BUZZER,LOW);
    digitalWrite(LEDRED,LOW);

    delay(1000);
  }
}
