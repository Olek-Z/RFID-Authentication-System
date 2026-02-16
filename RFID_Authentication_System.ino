/*
 * Multi-Factor RFID Authentication System
 * 
 * Description: Security access control system using a RFID card + PIN verification
 *              with timeout protection and visual/audio feedback
 * 
 * Author: Oleksandr Zarichnyy
 * Date: February 2026
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
#define SS_PIN 10
#define RST_PIN 5
#define BUZZER_PIN 47
#define RED_LED 45
#define GREEN_LED 43

// ========== CONSTANTS ==========
const unsigned long PIN_TIMEOUT = 7500;  // Timeout in milliseconds
const byte ROWS = 4;
const byte COLS = 4;
const char pinAmount = 4;

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
LiquidCrystal lcd(13, 12, 9, 8, 7, 6);

// ========== USER DATABASE ==========
struct User 
{
  byte cardIDS[4];  // RFID card unique identifier
  char pins[5];     // 4-digit PIN + null terminator
  char* name;       // User display name
};

User users[] = 
{
  {{0xB3, 0x29, 0xD7, 0x5}, "1234", "Alice"},
  {{0x5E, 0xEE, 0x9C, 0x4}, "ABCD", "Bob"}
};

// ========== SETUP ==========
void setup() 
{
  // Initialize serial communication
  Serial.begin(9600);
  
  // Initialize RFID reader
  SPI.begin();
  RFID.PCD_Init();
  
  // Initialize LCD
  lcd.begin(16, 2);
  lcd.clear();
  
  // Configure output pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  
  Serial.println();
  Serial.println("System is ready!");
  Serial.println();
}

// ========== MAIN AUTHENTICATION LOOP ==========
void loop() 
{
  // Check if a card present
  if (!RFID.PICC_IsNewCardPresent()) 
  {
    return;
  }
  Serial.println("Card detected");
  
  // Read cards data
  if (!RFID.PICC_ReadCardSerial()) 
  {
    return;
  }
  Serial.println("Card read successfully!");
  
  // Audio feedback for successful card read
  tone(BUZZER_PIN, 1800);
  delay(100);
  noTone(BUZZER_PIN);
  
  // Store the scanned card ID
  byte scannedArray[4];
  for (int i = 0; i < 4; i++) 
  {
    scannedArray[i] = RFID.uid.uidByte[i];
    Serial.print(scannedArray[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  
  // Search for the matching card in the user database
  int rowStorer = -1;
  for (int i = 0; i < 2; i++) 
  {
    if (memcmp(scannedArray, users[i].cardIDS, 4) == 0) 
    {
      rowStorer = i;
      break;
    }
  }
  
  // Handle unauthorized cards
  if (rowStorer == -1) 
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
    
    return;
  }
  
  // ========== PIN ENTRY ==========
  char keypadEnters[5];  // 4 digits + null terminator
  
  Serial.print("User ");
  Serial.print(users[rowStorer].name);
  Serial.println(" prompted for PIN");
  
  // Display personalized greeting
  lcd.clear();
  lcd.print("Hello ");
  lcd.print(users[rowStorer].name);
  lcd.setCursor(0, 1);
  lcd.print("Enter Pin: ");
  
  // Start timeout timer
  unsigned long startTime = millis();
  char key = NO_KEY;
  
  // Read 4-digit PIN with timeout protection
  for (int i = 0; i < pinAmount; i++) 
  {
    while (key == NO_KEY) 
    {
      // Check for timeout
      if (millis() - startTime > PIN_TIMEOUT) 
      {
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
        
        return;
      }
      
      key = KEYPAD.getKey();
    }
    
    // Visual and audio feedback for key press'
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
  
  // PIN verifier
  if (strcmp(users[rowStorer].pins, keypadEnters) != 0) 
  {
    Serial.println("Incorrect PIN - Access denied");
    
    lcd.clear();
    lcd.print("ACCESS DENIED");
    
    tone(BUZZER_PIN, 200);
    digitalWrite(RED_LED, HIGH);
    delay(800);
    noTone(BUZZER_PIN);
    delay(700);
    digitalWrite(RED_LED, LOW);
    delay(1000);
    lcd.clear();
    
    return;
  }
  
  // ========== ACCESS GRANTED ==========
  Serial.println("Access granted");
  
  lcd.clear();
  lcd.print("ACCESS GRANTED");
  
  // Success audio/visual feedback
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