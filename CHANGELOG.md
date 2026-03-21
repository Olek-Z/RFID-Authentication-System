# Changelog

## [v1.1] - [March 2026]
### Added
- Welcome screen displayed on idle and after every authentication attempt
- 3-attempt PIN lockout with live attempt counter shown on LCD
- `startMessage()` function for reusable welcome screen logic
- `accessGranted()` function extracted for cleaner code structure
- `pinVerify()` function for isolated PIN comparison logic
- `pinSequence()` function handling full PIN entry flow with retry logic

### Changed
- Refactored monolithic loop into separate focused functions
- Replaced `#define` pin declarations with `const int` for proper C++ convention
- Renamed `rowStorer` to `userIndex` for clarity
- Changed `pinAmount` type from `char` to `const int`
- User count in database loop now calculated dynamically with `sizeof` instead of hardcoded value

---

## [v1.0] - [February 2026]
### Initial Release
- Two-factor authentication (RFID card + 4-digit PIN)
- Multi-user database using struct with personalized greetings
- Session timeout protection (7.5 seconds)
- Visual feedback via Red and Green LEDs
- Audio feedback via passive buzzer
- Real-time LCD status messages
