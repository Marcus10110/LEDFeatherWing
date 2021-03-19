# LED Feather Wing

Mark's Living Room LED Light Control

The living room has 4 strips of WS2815 LEDs. Two ESP32s drive two strips each.

The lights are arranged in a square, with two opposite corners driven by the ESP32s.

Each strip has 150 LEDs, totaling 600.

To keep animations synchronized, both LEDs will keep time synchronization with ntp internet time.

## Ideas:

Use mDNS to broadcast the device name, and potentially do service discovery as well.
when the device powers on, it can check for siblings. If there are any, it can copy their current settings. Ohterwise, it can load defaults.
when it gets new settings from a web service, it can

Persisted Settings:
Device Name
TimeZone
DST enabled/disabled
On/Off/Brightness
For each strip - on/off, index, reversed, LED count (index used in multi-strip animations)

Web service:
Display all settings.
Expose buttons to change all interfaces.
