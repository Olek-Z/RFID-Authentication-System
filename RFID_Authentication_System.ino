/*
 * Multi-Factor RFID Authentication System
 *
 * Description: Security access control system using RFID card + PIN verification
 *              with timeout protection, 3-attempt lockout, visual/audio feedback,
 *              and real-time timestamped serial logging via DS3231 RTC module.
 *
 * Author: Oleksandr Zarichnyy
 * Version: 2.0.0
 * Date: May 2026
 *
 * Hardware:
 *   - Arduino Mega 2560
 *   - MFRC522 RFID Reader (SPI — SS pin 10, RST pin 5)
 *   - DS3231 RTC Module (I2C — SDA pin 20, SCL pin 21)
 *   - 4x4 Matrix Keypad (rows: 22,24,26,28 | cols: 30,32,34,36)
 *   - 16x2 LCD Display (RS:13, EN:12, D4:9, D5:8, D6:7, D7:6)
 *   - Passive Buzzer (pin 47)
 *   - Red LED (pin 45), Green LED (pin 43)
 *
 * Features:
 *   - Two-factor authentication (card + PIN)
 *   - User database with personalized greeting
 *   - 3-attempt PIN lockout with live attempt counter
 *   - Session timeout protection (7.5 seconds)
 *   - Real-time timestamped CSV serial logging
 *   - Visual (LEDs) and audio (buzzer) feedback
 */

// ========== LIBRARIES ==========
#include <SPI.h>
#include <Keypad.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>
#include "RTClib.h"

// ========== PIN DEFINITIONS ==========
const int SS_PIN     = 10;
const int RST_PIN    = 5;
const int BUZZER_PIN = 47;
const int RED_LED    = 45;
const int GREEN_LED  = 43;

// ========== CONSTANTS ==========
const unsigned long PIN_TIMEOUT = 7500;  // Session timeout in milliseconds
const byte ROWS      = 4;
const byte COLS      = 4;
const int pinAmount  = 4;               // Number of digits in PIN

// ========== KEYPAD CONFIGURATION ==========
char keys[ROWS][COLS] =
{
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {22, 24, 26, 28};
byte colPins[COLS] = {30, 32, 34, 36};

// ========== HARDWARE OBJECTS ==========
RTC_DS3231 rtc;
MFRC522 RFID(SS_PIN, RST_PIN);
Keypad KEYPAD(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
LiquidCrystal lcd(13, 12, 9, 8, 7, 6);  // RS, EN, D4, D5, D6, D7

// ========== USER DATABASE ==========
struct User
{
  byte  cardIDS[4];  // 4-byte RFID card UID
  char  pins[5];     // 4-digit PIN + null terminator
  char* name;        // Display name shown on LCD
};

User users[] =
{
  {{0xB3, 0x29, 0xD7, 0x5}, "1234", "Alice"},  // White card
  {{0x5E, 0xEE, 0x9C, 0x4}, "ABCD", "Bob"}     // Blue tag
};

// ========== HELPER: LEADING ZERO ==========
// Prints a leading zero for single-digit values to keep timestamps uniform
void addZero(int value)
{
  if (value < 10) Serial.print("0");
}

// ========== HELPER: CSV TIMESTAMP ==========
// Prints current time as YYYY-MM-DD, HH:MM:SS followed by a comma separator
void timeStamp(DateTime now)
{
  Serial.print(now.year());  Serial.print("-");
  addZero(now.month());      Serial.print(now.month());  Serial.print("-");
  addZero(now.day());        Serial.print(now.day());    Serial.print(", ");
  addZero(now.hour());       Serial.print(now.hour());   Serial.print(":");
  addZero(now.minute());     Serial.print(now.minute()); Serial.print(":");
  addZero(now.second());     Serial.print(now.second()); Serial.print(", ");
}

// ========== IDLE WELCOME SCREEN ==========
// Displayed on startup and after every completed authentication attempt
void startMessage()
{
  lcd.clear();
  lcd.print("Welcome!");
  lcd.setCursor(0, 1);
  lcd.print("Scan your card!");
}

// ========== ACCESS GRANTED SEQUENCE ==========
// Green LED flash + rising two-tone beep
void accessGranted()
{
  lcd.clear();
  lcd.print("ACCESS GRANTED");

  tone(BUZZER_PIN, 1500);
  digitalWrite(GREEN_LED, HIGH);
  delay(150);
  tone(BUZZER_PIN, 2000);
  delay(150);
  noTone(BUZZER_PIN);
  delay(1000);
  digitalWrite(GREEN_LED, LOW);
  lcd.clear();
}

// ========== PIN VERIFICATION ==========
// Returns true if entered PIN matches stored PIN for the given user
bool pinVerify(int userIndex, char* keypadEnters)
{
  if (strcmp(users[userIndex].pins, keypadEnters) != 0)
  {
    return false;
  }
  else
  {
    return true;
  }
}

// ========== PIN ENTRY SEQUENCE ==========
// Prompts user for PIN up to 3 times
// Returns: 0 = correct PIN, 1 = too many attempts, 2 = session timeout
int pinSequence(int userIndex)
{
  for (int j = 0; j < 3; j++)
  {
    char keypadEnters[5];

    Serial.print("User ");
    Serial.print(users[userIndex].name);
    Serial.println(" prompted for PIN");

    lcd.clear();
    lcd.print("Hello ");
    lcd.print(users[userIndex].name);
    lcd.setCursor(0, 1);
    lcd.print("Enter Pin: ");

    unsigned long startTime = millis();
    char key = NO_KEY;

    // Collect exactly pinAmount key presses
    for (int i = 0; i < pinAmount; i++)
    {
      while (key == NO_KEY)
      {
        if (millis() - startTime > PIN_TIMEOUT)
        {
          return 2;  // Session timeout
        }
        key = KEYPAD.getKey();
      }

      lcd.print("*");
      tone(BUZZER_PIN, 1500);
      delay(50);
      noTone(BUZZER_PIN);

      keypadEnters[i] = key;
      key = NO_KEY;
    }
    keypadEnters[pinAmount] = '\0';

    Serial.print("PIN entered: ");
    for (int j = 0; j < pinAmount; j++)
    {
      Serial.print(keypadEnters[j]);
      Serial.print(" ");
    }
    Serial.println();

    if (pinVerify(userIndex, keypadEnters) == true)
    {
      return 0;  // Correct PIN
    }

    // Wrong PIN — show remaining attempts and loop
    lcd.clear();
    lcd.print("Wrong pin");
    lcd.setCursor(0, 1);
    lcd.print(3 - (j + 1));
    lcd.print(" tries left");

    delay(1500);
  }

  return 1;  // All attempts exhausted
}

// ========== SETUP ==========
void setup()
{
  delay(1000);  // Suppress garbage characters on Serial startup
  Serial.begin(9600);

  SPI.begin();
  RFID.PCD_Init();

  lcd.begin(16, 2);
  lcd.clear();

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED,    OUTPUT);
  pinMode(GREEN_LED,  OUTPUT);

  rtc.begin();
  if (rtc.lostPower())
  {
    Serial.println("RTC lost power, resetting time...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  Serial.println();
  Serial.println("----------------------");
  Serial.println("System is ready!");
  Serial.println("----------------------");

  startMessage();
}

// ========== MAIN LOOP ==========
void loop()
{
  DateTime now = rtc.now();

  if (!RFID.PICC_IsNewCardPresent()) return;

  Serial.println();
  Serial.println("Card detected");

  if (!RFID.PICC_ReadCardSerial()) return;

  Serial.println("Card read successfully!");

  tone(BUZZER_PIN, 1800);
  delay(100);
  noTone(BUZZER_PIN);

  // Read scanned UID into local array
  byte scannedArray[4];
  for (int i = 0; i < 4; i++)
  {
    scannedArray[i] = RFID.uid.uidByte[i];
    Serial.print(scannedArray[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Search user database for matching UID
  int userIndex = -1;
  for (int i = 0; i < (sizeof(users) / sizeof(users[0])); i++)
  {
    if (memcmp(scannedArray, users[i].cardIDS, 4) == 0)
    {
      userIndex = i;
      break;
    }
  }

  // Unknown card — deny immediately
  if (userIndex == -1)
  {
    timeStamp(now);
    Serial.print("UNKNOWN");
    Serial.print(", ");
    Serial.println("UNAUTHORIZED");

    lcd.clear();
    lcd.print("UNAUTHORIZED");

    tone(BUZZER_PIN, 200);
    digitalWrite(RED_LED, HIGH);
    delay(800);
    noTone(BUZZER_PIN);
    delay(2500);
    digitalWrite(RED_LED, LOW);
    lcd.clear();

    startMessage();
    return;
  }

  // Known card — run PIN sequence
  int gotIn = pinSequence(userIndex);

  if (gotIn == 0)
  {
    accessGranted();

    timeStamp(now);
    Serial.print(users[userIndex].name);
    Serial.print(", ");
    Serial.println("GRANTED ACCESS");

    startMessage();
  }
  else if (gotIn == 2)
  {
    timeStamp(now);
    Serial.print(users[userIndex].name);
    Serial.print(", ");
    Serial.println("ACCESS DENIED: SESSION TIMEOUT");

    lcd.clear();
    lcd.print("ACCESS DENIED");
    lcd.setCursor(0, 1);
    lcd.print("SESSION EXPIRED");

    tone(BUZZER_PIN, 200);
    digitalWrite(RED_LED, HIGH);
    delay(800);
    noTone(BUZZER_PIN);
    delay(1000);
    digitalWrite(RED_LED, LOW);
    lcd.clear();

    startMessage();
  }
  else
  {
    timeStamp(now);
    Serial.print(users[userIndex].name);
    Serial.print(", ");
    Serial.println("ACCESS DENIED: TOO MANY ATTEMPTS");

    lcd.clear();
    lcd.print("ACCESS DENIED");
    lcd.setCursor(0, 1);
    lcd.print("Too many tries!");

    tone(BUZZER_PIN, 200);
    digitalWrite(RED_LED, HIGH);
    delay(800);
    noTone(BUZZER_PIN);
    delay(2500);
    digitalWrite(RED_LED, LOW);
    lcd.clear();

    startMessage();
  }
}
