#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define MCU_SIM_BAUDRATE        115200
#define MCU_SIM_TX_PIN              3
#define MCU_SIM_RX_PIN              2

SoftwareSerial simSerial(MCU_SIM_RX_PIN, MCU_SIM_TX_PIN);

void lcdDebug(String line1, String line2 = "") {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1);
    if (line2 != "") {
        lcd.setCursor(0, 1);
        lcd.print(line2);
    }
}

// Please update number before test
#define PHONE_NUMBER                "+84345206315"

void sim_at_wait()
{
    delay(100);
    while (simSerial.available()) {
        Serial.write(simSerial.read());
    }
}

bool sim_at_cmd(String cmd){
    lcdDebug("Sending:", cmd);
    simSerial.println(cmd);
    sim_at_wait();
    lcdDebug("Sent:", cmd);
    delay(200);
    return true;
}

bool sim_at_send(char c){
    simSerial.write(c);
    return true;
}

void sent_sms()
{
    lcdDebug("Sending SMS...", "To: " + String(PHONE_NUMBER));
    delay(1000);
    
    sim_at_cmd("AT+CMGF=1");
    String temp = "AT+CMGS=\"";
    temp += (String)PHONE_NUMBER;
    temp += "\"";
    sim_at_cmd(temp);
    sim_at_cmd("this is the test mes");

    // End charactor for SMS
    sim_at_send(0x1A);
    lcdDebug("SMS Sent!", "Done");
}

void call()
{
    String temp = "ATD";
    temp += PHONE_NUMBER;
    temp += ";";
    sim_at_cmd(temp); 

    delay(20000);

    // Hang up
    sim_at_cmd("ATH"); 
}

void setup() 
{
    delay(20);
    Serial.begin(115200);
    Serial.println("\n\n\n\n-----------------------\nSystem started!!!!");

    lcd.init();
    lcd.backlight();
    lcdDebug("System started!", "Warming up...");

    delay(8000);
    simSerial.begin(MCU_SIM_BAUDRATE);

    // Check AT Command
    sim_at_cmd("AT");

    // Product infor
    sim_at_cmd("ATI");

    // Check SIM Slot
    sim_at_cmd("AT+CPIN?");

    // Check Signal Quality
    sim_at_cmd("AT+CSQ");

    sim_at_cmd("AT+CIMI");

    pinMode(2,OUTPUT); 
    digitalWrite(2,HIGH);

    sent_sms();

    // Delay 5s
    delay(5000);   

    //call();
}

void loop() 
{     
    if (Serial.available()){
        char c = Serial.read();
        simSerial.write(c);
    }
    sim_at_wait();
}
