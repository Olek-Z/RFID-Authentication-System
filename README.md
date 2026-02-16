# Multi-Factor RFID Authentication System

A multi-factor security access control system built with Arduino that uses RFID card scanning with PIN verification. Features personalized for optimal user experience and session timeout protection. 

## Features

**Two-Factor Authentication**: RFID card + 4-digit custom PIN verification
**User Database**: Supports multiple users with personalized greetings
**Session Timeout**: 7.5-second timeout protection during PIN entry
**Visual/Audio Feedback**: Green/Red LEDs and Buzzer tones to signal different features
**LCD Display**: Real-time status messages and user prompts
**Scalable Design**: Easy to add new users to the database

## Hardware

- Arduino Mega 2560
- MFRC522 RFID Reader Module
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

All libraries can be installed via the Arduino IDE

## Pin Configuration

| Component | Arduino Pin |
|-----------|-------------|
| RFID SS   | 10          |
| RFID RST  | 5           |
| RFID MISO | 50          |
| RFID MOSI | 51          |
| RFID SCK  | 52          |
| Buzzer    | 47          |
| Red LED   | 45          |
| Green LED | 43          |
| LCD RS    | 13          |
| LCD E     | 12          |
| LCD D4-D7 | 9, 8, 7, 6  |
| Keypad Rows | 22, 24, 26, 28 |
| Keypad Cols | 30, 32, 34, 36 |

## How to use

1. Upload the code to Arduino and tap your RFID card on the reader
2. Enter the 4-digit PIN on the keypad within the time frame
3. See the result

## Demo
[Video coming soon]

## Circuit Diagram
[Schematic coming soon]

## Future Improvements
- Attempt counter with progressive lockout (3 attempts → temporary ban)
- Real-time clock integration for access logging with time stamps and SD card logging for audit trail

## What I learned

- Embedded systems programming (Arduino/C++)
- SPI communication protocols & multi-component hardware integration
- State machine designing
- Simple Security principles (two-factor authentication & timeout protection), User experience (visual/audio feedback), and Data structure implementation (structs, user database)
