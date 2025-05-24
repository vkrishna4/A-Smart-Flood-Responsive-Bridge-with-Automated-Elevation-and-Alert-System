#include <Servo.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// Initialize software serial for GSM module (adjust pins as needed)
SoftwareSerial gsmSerial(2, 3); // RX, TX
LiquidCrystal_I2C lcd(0x27, 16, 2); // Initialize LCD (address 0x27, 16 columns, 2 rows)
const int Buzzer = 12; // Buzzer pin
Servo servo1, servo2; // Servo objects

// Function declarations
void gsm_init();
void locationsend();

unsigned long lastSmsTime = 0; // Variable to store the last SMS sent time
const unsigned long smsDelay = 600000; // 10 minutes in milliseconds

void setup() {
    // Attach servos to pins
    servo1.attach(5);
    servo2.attach(6);
    
    // Initialize LCD
    lcd.init();
    lcd.backlight();
    
    // Initialize Serial for debugging
    Serial.begin(9600);
    gsmSerial.begin(9600);
    
    // Display initial message
    lcd.clear();
    lcd.print(F("GSM/IOT Based"));
    lcd.setCursor(0, 1);
    lcd.print(F("Flood Alert SYS"));
    delay(3000);
    lcd.clear();
    
    // Display saved mobile number
    lcd.setCursor(0, 0);
    lcd.print(F("Saved Mob.NO:"));
    lcd.setCursor(0, 1);
    lcd.print(F("+91"));
    for (int count = 0; count < 10; count++) {
        lcd.print((char)EEPROM.read(count));
    }
    delay(1000);
    
    // Initialize GSM module
    gsm_init();
}

void loop() {
    // Main loop code
    int level = analogRead(A0); // Read the water level
    lcd.setCursor(0, 0);
    lcd.print(F("LEVEL : "));
    lcd.print(level);
    lcd.print(F(" Mtrs  "));
    delay(1000);
    
    // Check if the water level is above a certain threshold
    if (level > 500) { // Adjust this threshold as needed
        // Move servo to the detected water level
        int servoPosition = map(level, 0, 1023, 0, 90); // Map the level to servo position
        servo1.write(servoPosition);
        servo2.write(servoPosition);
        delay(1000); // Allow time for the servo to reach the position

        // Check if 10 minutes have passed since the last SMS was sent
        if (millis() - lastSmsTime >= smsDelay) {
            locationsend(); // Send alert if water level is critical
            lastSmsTime = millis(); // Update the last SMS sent time
        }

        // Display critical alert
        for (int i = 0; i < 5; i++) {
            Serial.print(F("*<meta http-equiv=\"refresh\" content = \"3\" ><h1 style=\"color:red;text-align:center\">GSM/IOT BASED</h1><h1 style=\"color:red;text-align:center\">FLOOD ALERT SYSTEM</h1>"));
            Serial.print(F("<h2 style=\"color;text-align:center\">Critical Alert: Rising water levels detected! Automated bridge elevation in progress."));
            Serial.print(F("<h2 style=\"color;text-align:center\">PLZ AVOID CROSSING."));
            Serial.print(F("#"));
        }
    } else {
        // If water is not detected, return servo to normal position
        servo1.write(0); // Normal position
        servo2.write(0); // Normal position
        delay(1000); // Allow time for the servo to return

        // Display no alerts
        for (int i = 0; i < 5; i++) {
            Serial.print(F("*<meta http-equiv=\"refresh\" content = \"3\" ><h1 style=\"color:red;text-align:center\">GSM/IOT BASED</h1><h1 style=\"color:red;text-align:center\">FLOOD ALERT SYSTEM</h1>"));
            Serial.print(F("<h2 style=\"color;text-align:center\">!!!NO ALERTS!!!</h2>"));
            Serial.print(F("#"));
        }
    }
}

void gsm_init() {
    lcd.clear();
    lcd.print(F("Finding Gsm.."));
    boolean at_flag = true;
    while (at_flag) {
        gsmSerial.println(F("AT"));
        unsigned long startTime = millis();
        // Wait for response or timeout after 2 seconds
        while ((millis() - startTime) < 2000) {
            while (gsmSerial.available() > 0) {
                // Read response line from GSM and check for "OK"
                String response = gsmSerial.readStringUntil('\n');
                if (response.indexOf("OK") != -1) {
                    at_flag = false;
                    break;
                }
            }
            if (!at_flag)
                break;
        }
        delay(1000);
    }
    lcd.clear();
    lcd.print(F("Gsm Connected."));
    delay(1000);
    lcd.clear();
}

void locationsend() {
    int count;
    gsmSerial.println("AT");
    delay(1000);
    gsmSerial.println("AT+CMGF=1"); // Sets the GSM Module in Text Mode
    delay(1000); // Delay of 1000 milliseconds or 1 second
    gsmSerial.print("AT+CMGS=\"");
    gsmSerial.print("+91");
    for (count = 0; count < 10; count++) {
        gsmSerial.print((char)EEPROM.read(count));
    }
    gsmSerial.print("\"");
    gsmSerial.println("\r");
    delay(1000);
    gsmSerial.print("Emergency Alert! Water level critical at bridge area. Bridge elevation in progress. Avoid the area and stay safe.");
    gsmSerial.println((char)26); // Send Ctrl+Z to indicate end of message
    delay(1000);
    gsmSerial.end();
}