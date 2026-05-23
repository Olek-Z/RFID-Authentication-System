# Changelog

## [v2.0.0] - May 2026
### Added
- DS3231 RTC module integration for real-time timestamping
- `timeStamp()` function — prints current date and time in CSV format before every log entry
- `addZero()` helper function — pads single-digit values with a leading zero for clean timestamp formatting
- Timestamped serial logging for all access events (granted, denied, timeout, unauthorized)
- CSV log format: `YYYY-MM-DD, HH:MM:SS, NAME, RESULT`

### Changed
- `pinSequence()` return type changed from `bool` to `int` — now returns 0 (granted), 1 (too many attempts), or 2 (session timeout)
- Session timeout feedback moved from inside `pinSequence()` to `loop()` — all outcome handling now centralized in one place
- Removed redundant post-lockout `return false` inside `pinSequence()` for loop — falls through to bottom naturally
- Startup delay added before `Serial.begin()` to suppress garbage characters on boot
- Updated serial startup message with separator lines for cleaner output
- Header comment block updated to reflect new hardware and version

### Hardware Added
- DS3231 RTC Module (I2C — SDA pin 20, SCL pin 21)
- CR2032 coin battery for RTC power retention

---

## [v1.1.0] - March 2026
### Added
- Welcome screen displayed on idle and after every authentication attempt
- 3-attempt PIN lockout with live attempt counter shown on LCD
- `startMessage()` function for reusable welcome screen logic
- `accessGranted()` function extracted for cleaner code structure
- `pinVerify()` function for isolated PIN comparison logic
- `pinSequence()` function handling full PIN entry flow with retry logic

### Changed
- Refactored blocks of loop into separate focused functions
- Replaced `#define` pin declarations with `const int` for proper C++ convention
- Renamed `rowStorer` to `userIndex` for clarity
- Changed `pinAmount` type from `char` to `const int`
- User count in database loop now calculated dynamically with `sizeof` instead of hardcoded value

---

## [v1.0.0] - February 2026
### Initial Release
- Two-factor authentication (RFID card + 4-digit PIN)
- Multi-user database using struct with personalized greetings
- Session timeout protection (7.5 seconds)
- Visual feedback via Red and Green LEDs
- Audio feedback via passive buzzer
- Real-time LCD status messages
