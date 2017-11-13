# RGB_CONTROLLER
Platformio project for ESP8266 RGB Controller

Commands are shared between the self hosted website and over MQTT. These are in a JSON form i.e.

{"command":"setRGB","r":'255', "g":'255', "b":'255'}
{"command":"setKelvin","kelvin":'1400'}
{"command":"setBrightness","brightness":'0.5'}

Relevant MQTT channels are:

esp/rgblight/%d/status
esp/rgblight/%d/command   <--- %d should be replaced by the unique ID of the device
esp/rgblight/%d/debug

to find devices, subscribe and save to the debug console:

esp/rgblight/+/status 
