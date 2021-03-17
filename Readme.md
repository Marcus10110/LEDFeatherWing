# LED Feather Wing

Mark's Living Room LED Light Control

The living room has 4 strips of WS2815 LEDs. Two ESP32s drive two strips each.

The lights are arranged in a square, with two opposite corners driven by the ESP32s.

Each strip has 150 LEDs, totaling 600.

To keep animations synchronized, both LEDs will keep time synchronization with ntp internet time.
