# CO2Sensor

CO2 sensor for Home Assistant using a MH-Z19 CO2 sensor on a ESP8266 NodeMCU

Note: superseded by [HomeMCU](https://github.com/Scrin/HomeMCU)

## Setup:

- Clone/download this repository
- Copy `include/config.h.example` as `include/config.h`
- Set your configuration in `config.h`
- Build/flash like any other PlatformIO project
- The RX pin of MH-Z19 should be connected to GPIO15 (pin labeled D8) and TX of MH-Z19 to GPIO13 (pin labeled D7)

For Home Assistant you'll want something like this in your configuration.yaml:

```
sensor:
  - platform: mqtt
    name: "CO2Sensor"
    state_topic: "CO2Sensor/state"
    availability_topic: "CO2Sensor/availability"
    unit_of_measurement: "ppm"
    value_template: "{{ value_json.co2 }}"
```

## Over The Air update:

Documentation: https://arduino-esp8266.readthedocs.io/en/latest/ota_updates/readme.html#web-browser

Basic steps:

- Use PlatformIO: Build
- Browse to http://IP_ADDRESS/update or http://hostname.local/update
- Select .pio/build/nodemcuv2/firmware.bin from work directory as Firmware and press Update Firmware
