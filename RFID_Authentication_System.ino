/*
 * Multi-Factor RFID Authentication System
 *
 * Description: Security access control system using RFID card + PIN verification
 *              with timeout protection, 3-attempt lockout, and visual/audio feedback
 *
 * Author: Oleksandr Zarichnyy
 * Date: March 2026
 *
 * Hardware Requirements:
 *   - Arduino Mega 2560
 *   - MFRC522 RFID Reader Module
 *   - 4x4 Matrix Keypad
 *   - 16x2 LCD Display
 *   - Passive Buzzer
 *   - Red and Green LEDs
 *
 * Features:
 *   - Two-factor authentication (card + PIN)
 *   - User database with personalized greeting
 *   - Welcome screen displayed on idle
 *   - 3-attempt PIN lockout with live attempt counter
 *   - Session timeout protection (7.5 seconds)
 *   - Visual feedback (LEDs) and audio feedback (buzzer)
 *   - Real-time LCD display
 */

// ========== LIBRARIES ==========
#include <SPI.h>
#include <Keypad.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>

// ========== PIN DEFINITIONS ==========
const int SS_PIN = 10;
const int RST_PIN = 5;
const int BUZZER_PIN = 47;
const int RED_LED = 45;
const int GREEN_LED = 43;

// ========== CONSTANTS ==========
const unsigned long PIN_TIMEOUT = 7500;  // PIN entry timeout in milliseconds
const byte ROWS = 4;
const byte COLS = 4;
const int pinAmount = 4;                // Number of digits in PIN

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
MFRC522 RFID(SS_PIN, RST_PIN);
Keypad KEYPAD(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
LiquidCrystal lcd(13, 12, 9, 8, 7, 6);  // RS, EN, D4, D5, D6, D7

// ========== USER DATABASE ==========
struct User
{
  byte cardIDS[4];  // RFID card unique identifier (4-byte UID)
  char pins[5];     // 4-digit PIN + null terminator
  char* name;       // Display name shown on LCD greeting
};

User users[] =
{
  {{0xB3, 0x29, 0xD7, 0x5}, "1234", "Alice"},  // White card
  {{0x5E, 0xEE, 0x9C, 0x4}, "ABCD", "Bob"}     // Blue tag
};

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
// Called when card + PIN are both verified successfully
void accessGranted()
{
  Serial.println("Access granted");

  lcd.clear();
  lcd.print("ACCESS GRANTED");

  // Rising two-tone beep + green LED flash
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
// Compares entered PIN against stored PIN for the given user
// Returns true if they match, false otherwise
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

// ========== PIN ENTRY + 3-ATTEMPT SEQUENCE ==========
// Prompts the user to enter their PIN up to 3 times
// Returns true if correct PIN entered on any attempt, false if all 3 fail
// Also handles session timeout during PIN entry
bool pinSequence(int userIndex)
{
  for (int j = 0; j < 3; j++)  // j tracks current attempt (0, 1, 2)
  {
    char keypadEnters[5];  // Temp buffer to hold entered PIN + null terminator

    Serial.print("User ");
    Serial.print(users[userIndex].name);
    Serial.println(" prompted for PIN");

    // Display personalized greeting with PIN prompt
    lcd.clear();
    lcd.print("Hello ");
    lcd.print(users[userIndex].name);
    lcd.setCursor(0, 1);
    lcd.print("Enter Pin: ");

    // Start session timeout timer
    unsigned long startTime = millis();
    char key = NO_KEY;

    // Collect exactly 4 key presses from the keypad
    for (int i = 0; i < pinAmount; i++)
    {
      // Wait for a key press, checking for timeout each iteration
      while (key == NO_KEY)
      {
        if (millis() - startTime > PIN_TIMEOUT)
        {
          // User took too long — end the session
          Serial.println("Session timeout");

          lcd.clear();
          lcd.print("SESSION EXPIRED");

          tone(BUZZER_PIN, 200);
          digitalWrite(RED_LED, HIGH);
          delay(800);
          noTone(BUZZER_PIN);
          delay(1000);
          digitalWrite(RED_LED, LOW);
          lcd.clear();

          startMessage();
          return false;
        }

        key = KEYPAD.getKey();
      }

      // Key received — mask on LCD and give audio feedback
      lcd.print("*");
      tone(BUZZER_PIN, 1500);
      delay(50);
      noTone(BUZZER_PIN);

      keypadEnters[i] = key;
      key = NO_KEY;
    }
    keypadEnters[pinAmount] = '\0';  // Null-terminate for strcmp

    // Debug: print entered PIN to serial monitor
    Serial.print("PIN entered: ");
    for (int j = 0; j < pinAmount; j++)
    {
      Serial.print(keypadEnters[j]);
      Serial.print(" ");
    }
    Serial.println();

    // Check if entered PIN matches stored PIN
    if (pinVerify(userIndex, keypadEnters) == true)
    {
      return true;  // Correct PIN — exit immediately
    }

    // Wrong PIN — if this was the last attempt, exit silently
    if ((3 - (j + 1)) == 0)
    {
      return false;
    }

    // Attempts remaining — show count and loop again
    lcd.clear();
    lcd.print("Wrong pin");
    lcd.setCursor(0, 1);
    lcd.print(3 - (j + 1));
    lcd.print(" tries left");

    delay(1500);
  }

  return false;  // All 3 attempts exhausted
}

// ========== SETUP ==========
void setup()
{
  Serial.begin(9600);

  SPI.begin();
  RFID.PCD_Init();

  lcd.begin(16, 2);
  lcd.clear();

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  Serial.println();
  Serial.println("System is ready!");
  Serial.println();

  startMessage();
}

// ========== MAIN LOOP ==========
void loop()
{
  // Wait for a card to be presented
  if (!RFID.PICC_IsNewCardPresent())
  {
    return;
  }
  Serial.println("Card detected");

  // Attempt to read the card's serial data
  if (!RFID.PICC_ReadCardSerial())
  {
    return;
  }
  Serial.println("Card read successfully!");

  // Audio feedback for successful card scan
  tone(BUZZER_PIN, 1800);
  delay(100);
  noTone(BUZZER_PIN);

  // Copy scanned UID into local array
  byte scannedArray[4];
  for (int i = 0; i < 4; i++)
  {
    scannedArray[i] = RFID.uid.uidByte[i];
    Serial.print(scannedArray[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Search user database for a matching card UID
  int userIndex = -1;
  for (int i = 0; i < (sizeof(users) / sizeof(users[0])); i++)
  {
    if (memcmp(scannedArray, users[i].cardIDS, 4) == 0)
    {
      userIndex = i;  // Store index of matched user
      break;
    }
  }

  // Card not found in database — deny access
  if (userIndex == -1)
  {
    Serial.println("Unauthorized card");

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

  // Card authorized — run PIN entry sequence (3 attempts MAX)
  bool gotIn = pinSequence(userIndex);

  if (gotIn == true)
  {
    accessGranted();
    startMessage();
  }
  else
  {
    // All attempts failed — lock out
    Serial.println("Access denied - too many attempts");

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
