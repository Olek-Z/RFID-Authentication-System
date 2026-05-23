# Multi-Factor RFID Authentication System
A multi-factor security access control system built with Arduino that uses RFID card scanning combined with PIN verification and real-time timestamped access logging.

## Features
**Two-Factor Authentication**: RFID card + 4-digit custom PIN verification
**User Database**: Supports multiple users with personalized greetings
**Welcome Screen**: Idle display prompts user to scan their card
**3-Attempt Lockout**: Wrong PIN shows remaining attempts, locks out after 3 failures
**Session Timeout**: 7.5-second timeout protection during PIN entry
**Visual/Audio Feedback**: Green/Red LEDs and buzzer tones signal different outcomes
**LCD Display**: Real-time status messages and user prompts
**Timestamped Serial Logging**: Every access event logged in CSV format with date and time via DS3231 RTC
**Scalable Design**: Easy to add new users to the database

## Hardware
- Arduino Mega 2560
- MFRC522 RFID Reader Module
- DS3231 RTC Module (with CR2032 battery)
- 4x4 Matrix Keypad
- 16x2 LCD Display (Parallel mode)
- Passive Buzzer, 2x LEDs (Red & Green), 2x 220Ω Resistors (for LEDs)
- Jumper wires & Breadboard

## Software
### Arduino Libraries
- `SPI.h` (built-in)
- `MFRC522.h` - [RFID Library](https://github.com/miguelbalboa/rfid)
- `Keypad.h` - [Keypad Library](https://github.com/Chris--A/Keypad)
- `LiquidCrystal.h` (built-in)
- `RTClib.h` - [Adafruit RTClib](https://github.com/adafruit/RTClib)

All libraries can be installed via the Arduino IDE Library Manager.

## Pin Configuration
| Component | Arduino Pin |
|-----------|-------------|
| RFID SS   | 10          |
| RFID RST  | 5           |
| RFID MISO | 50          |
| RFID MOSI | 51          |
| RFID SCK  | 52          |
| RTC SDA   | 20          |
| RTC SCL   | 21          |
| Buzzer    | 47          |
| Red LED   | 45          |
| Green LED | 43          |
| LCD RS    | 13          |
| LCD E     | 12          |
| LCD D4-D7 | 9, 8, 7, 6  |
| Keypad Rows | 22, 24, 26, 28 |
| Keypad Cols | 30, 32, 34, 36 |

## Serial Log Format
Every access event is printed to Serial in CSV format:
- YYYY-MM-DD, HH:MM:SS, NAME, RESULT
# Example output:
- 2026-05-23, 14:32:05, Alice, GRANTED ACCESS
- 2026-05-23, 14:35:11, Bob, ACCESS DENIED: TOO MANY ATTEMPTS
- 2026-05-23, 14:37:42, UNKNOWN, UNAUTHORIZED

## How to Use
1. Upload the code to your Arduino Mega
2. Tap your RFID card on the reader
3. Enter your 4-digit PIN on the keypad within 7.5 seconds
4. System grants or denies access with visual, audio, and serial feedback

## Demo
[![Demo Video](https://img.youtube.com/vi/QOcPQ9NVaQI/0.jpg)](https://youtu.be/QOcPQ9NVaQI)

*Note: Demo shows v1.x — v2.0 footage coming soon.*

## Future Improvements
- SD card logging for permanent audit trail (hardware purchased, in progress)
- Master card mode for adding/removing users without reprogramming

## What I Learned
- Embedded systems programming (Arduino/C++)
- SPI and I2C communication protocols & multi-component hardware integration
- State machine design
- Security principles (two-factor authentication, timeout protection, lockout logic)
- Real-time data logging and CSV formatting on embedded hardware
