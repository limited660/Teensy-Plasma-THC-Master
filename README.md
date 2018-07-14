# Teensy Plasma THC Master

Loop
- <10 microsecond loop time

Screen
- RepRap Smart Controller (4x20 with Push Button Encoder)

Button
- One Press = Swap between Tip Target/Hysteresis & save value if changed
- Double Press = Recalls highlighted value from EEPROM
- Hold = Saves highlighted value to EEPROM, this value is restored on power up and using Double Press

Encoder
- Change highlighted value

Frequency Counter
- THCAD should be set to F/1
- Gate interval of 10 milliseconds
- Change mapfloat in Loop to match numbers provided on THCAD
