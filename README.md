# CustomRGBWLight
Custom RGBW light that maps an RGBW smart light into a colorspace.


## Typical YAML configuration

Note that this particular smart bulb, the LOHAS bulb requires a hack to switch around channels.   Your bulb might not require this.

See old digiblur's repo 
https://github.com/digiblur/ESPHome_LOHAS_LED
and his video on the subject:
https://www.youtube.com/watch?v=fTb6n6flJIw

~~~
esphome:
  name: lohas_bulb01
  platform: ESP8266
  board: esp01_1m
  includes:
    - copychan.h
    - ColorTempRGBWLight.h

wifi:
  networks:
   - ssid: XXX
     password: YYY
     bssid: aaaaa
   - ssid: XXX
     password: YYY
     bssid: bbbb
   - ssid: XXX
     password: YYY
     bssid: ccccc
  ap:
    ssid: "lohas_bulb01"
    password: "EEEEE"

captive_portal:

api:

logger:
  level: INFO

ota:

sensor:
  - platform: wifi_signal
    name: "Julia Torchier WiFi"
    update_interval: 600s

my9231:
 data_pin: GPIO13  
 clock_pin: GPIO15  
 num_channels: 6
 num_chips: 2 
 
output:
  - platform: my9231
    id: output_blue
    channel: 3
  - platform: my9231
    id: output_red
    channel: 5
  - platform: my9231
    id: output_green
    channel: 4
  - platform: my9231
    id: output_cw1
    channel: 1
  - platform: my9231
    id: output_cw2
    channel: 2
  - platform: custom
    type: float
    lambda: |-
      auto *copy = new CopyOutput();
      copy->channel_a = id(output_cw1);
      copy->channel_b = id(output_cw2);
      return {copy}; 
    outputs:
    - id: output_white

light:
  - platform: custom
    lambda: |-
      auto light_out = new ColorTempRGBWLight(id(output_red), id(output_green), id(output_blue), id(output_white));
      App.register_component(light_out);
      return {light_out};
    lights:
      - name: "Julia Torchier"
        default_transition_length: 1s
~~~
