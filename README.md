# Teensy Plasma THC Master

Loop
- Teensy 3.1
  - CPU Speed : 96MHz
  - Optimize : Fastest with LTO
  - Loop Time : 5.5 microsecond average
  - Cost : $19.80 (7/15/2018)
- Teensy LC
  - CPU Speed : 48MHz
  - Optimize : Faster with LTO
  - Loop Time : 15.7 microsecond average
  - Cost : $11.65 (7/15/2018)

Screen
- RepRap Smart Controller (4x20 with Push Button Encoder)

Button
- One Press = Swap between Tip Target/Hysteresis & save value if changed
- Double Press = Recalls highlighted value from EEPROM
- Hold(Editing Value) = Saves highlighted value to EEPROM, this value is restored on power up and using Double Press
- Hold(Not Editing Value) = Disables output and voltage comparisions, still prints current tip voltage to screen (tip voltage is always printed with small delay) 

Encoder
- Change highlighted value

Frequency Counter
- THCAD should be set to F/1
- Gate interval of 10 milliseconds
- Change mapfloat in Loop to match numbers provided on THCAD
