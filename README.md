# CO2Sensor

CO2 sensor for Home Assistant using a MH-Z19 CO2 sensor on a ESP8266 NodeMCU

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
