# LED Feather Wing

Mark's Living Room LED Light Control

The living room has 4 strips of WS2815 LEDs. Two ESP32s drive two strips each.

The lights are arranged in a square, with two opposite corners driven by the ESP32s.

Each strip has 150 LEDs, totaling 600.

To keep animations synchronized, both LEDs will keep time synchronization with ntp internet time.

## Getting Started:

Requires Arduino

Install the ArduinoJson library
Install the Adafruit NeoPixel library

Install the ESP32 board package:
https://github.com/espressif/arduino-esp32#using-through-arduino-ide
version 1.0.6+

Select the Adafruit ESP32 Feather board

in this project's src directory, copy src/wifi_config.h.template into a new file, wifi_config.h. Update this with your wifi SSID and password, as well as optional OTA details. Do not check in!

Use the Arduino IDE to either upload via serial port or OTA.

## Simulator

Each animation added to AllAnimations in animations.cpp can be simulated, to make animation development faster.

to use:

```
cd simulator
yarn dev
```

This will start a webserver, by default localhost:8080, and it will build the animation source code with web assembly.

Open a browser to view the simulator. After making any change to the source code, simply wait for the web assembly complication to complete, then refresh the browser.

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

Endpoints:
index - static html page.
/status - returns json blob with all data
/setBrightness - just sets the brightness
/setAnimation - just sets the animation settings
/setSettings - set everything else!

Boot state machine.

1. attempt to load settings. if empty, load default. (keep track if we've defaulted or not)
   1.5. initialize time.
2. use DNS SD / mDNS to queryService, to locate other connected LED Feather Wings.
3. if found, query settings from the nearest one. Load brightness & animation number.

LED Animation Ideas:

- Solid white
- Daylight mode. Use wall clock time (and optionally sunrise/sunset) to create 24 basic animation. Could go nuts with it later.
- color strobe mode
- twinkling stars (colored or uncolored)
-

The boards can be manually discovered using:
dns-sd -B \_arduino.\_tcp
or
dns-sd -B \_LEDFeatherWing.\_tcp

## TODO:

- add Off / 1% 25% 50% 100% buttons
- actually implement some more animations, index support, etc
- when a device gets a request from someone, add it to the Neighbor list if it's not already there.
- when a request fails, remove from neighbor list.

## FW Issues

## LED Layout

The couch wall had 49 LEDs removed, leaving 101.
The bar wall has all 150 LEDs

## PCB Errata

- Label the connectors (somehow)
- feather GND on pin 16 should not be there, that's an IO pin.
- connector does not work with stranded - TODO - test with Ferrule, if that doesn't work, get a different connector
