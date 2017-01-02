const char RGB_SELECTOR_PAGE[] PROGMEM = R"=====(
  <!DOCTYPE html>
<html>
<body>

<script src='https://cdnjs.cloudflare.com/ajax/libs/jscolor/2.0.4/jscolor.min.js'></script>

  <h1> WiFi RGB Controller </h1>
  <h2> Select a colour: </h2>
  <p><input class='jscolor' onchange='update(this.jscolor)' value='FFFEFD'>
  <p><button onclick="sendCommand(off)">Turn Off</button>
  <p><button onclick="sendCommand(red)">Red</button>
  <p><button onclick="sendCommand(green)">Green</button>
  <p><button onclick="sendCommand(blue)">Blue</button>
  <p><button onclick="sendCommand(fullBrightness)">100% Brightness</button>
  <p><button onclick="sendCommand(halfBrightness)">50% Brightness</button>

  <script>
	var off = '{"command":"off"}'
    var red = '{"command":"set_rgb","r":255,"g":0,"b":0}'
    var green = '{"command":"set_rgb","r":0,"g":255,"b":0}'
    var blue = '{"command":"set_rgb","r":0,"g":0,"b":255}'
    var halfBrightness = '{"command":"set_brightness", "b":0.5}'
    var fullBrightness = '{"command":"set_brightness", "b":1}'
    function sendCommand(command) {
        var data = 'command=' + command;
        var request = new XMLHttpRequest();
        request.open('POST', '/', true);
        request.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded; charset=UTF-8');
        request.send(data);
    }
    function update(jscolor) {
        var command = '{"command":"set_rgb","r":';
        command = command + Math.round(jscolor.rgb[0]);
        command = command +',"g":' + Math.round(jscolor.rgb[1]);
        command = command +',"b":' + Math.round(jscolor.rgb[2]) + '}';
        sendCommand(command);
    }

  </script>
</body>
</html>

)=====";
