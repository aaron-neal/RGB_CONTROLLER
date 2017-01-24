const char RGB_SELECTOR_PAGE[] PROGMEM = R"=====(<!DOCTYPE html><html lang=en><meta content="width=device-width,initial-scale=1,user-scalable=no"name=viewport><title>Wifi RGB - {{}}</title><script src=https://cdnjs.cloudflare.com/ajax/libs/jscolor/2.0.4/jscolor.min.js></script><script>function sendCommand(e){alert(e);var n="command="+e,t=new XMLHttpRequest;t.open("POST","/",!0),t.setRequestHeader("Content-Type","application/x-www-form-urlencoded; charset=UTF-8"),t.send(n)}function update(e){sendRGB(Math.round(e.rgb[0]),Math.round(e.rgb[1]),Math.round(e.rgb[2])),sendCommand(command)}function updateSliders(){var e=document.getElementById("r").value,n=document.getElementById("g").value,t=document.getElementById("b").value;sendRGB(e,n,t)}function sendRGB(e,n,t){var o='{"command":"setRGB","r":';o+=e,o=o+',"g":'+n,o=o+',"b":'+t+"}",sendCommand(o),document.getElementById("r").value=e,document.getElementById("g").value=n,document.getElementById("b").value=t}function sendKelvin(e){var n='{"command":"setKelvin","kelvin":';n=n+e+"}",sendCommand(n)}function setBootColour(){var e=document.getElementById("jsbootColour"),n='{"command":"setBootColour","r":';n+=Math.round(e.jscolor.rgb[0]),n=n+',"g":'+Math.round(e.jscolor.rgb[1]),n=n+',"b":'+Math.round(e.jscolor.rgb[2])+"}",sendCommand(n)}function clearBootColour(){sendCommand('{"command":"clearBootColour"}')}function updateBrightness(e){var n='{"command":"setBrightness","brightness":'+e+"}";sendCommand(n)}function restartDevice(){sendCommand('{"command":"restart"}')}function randomFade(){timespan=document.getElementById("t").value,sendCommand('{"command":"startRandomColours","timespan":'+timespan+"}")}function rainbow(){timespan=document.getElementById("t").value,sendCommand('{"command":"startRainbow","timespan":'+timespan+"}")}function flash(){timespan=document.getElementById("t").value,sendCommand('{"command":"startFlash","timespan":'+timespan+"}")}function stopAll(){sendCommand('{"command":"off"}')}function resetSettings(){var e=confirm("Are you sure you want to reset all the settings?");1==e&&sendCommand(reset)}function openTab(e,n){var t,o,a;for(o=document.getElementsByClassName("tabcontent"),t=0;t<o.length;t++)o[t].style.display="none";for(a=document.getElementsByClassName("tablinks"),t=0;t<a.length;t++)a[t].className=a[t].className.replace(" active","");document.getElementById(n).style.display="block",e.currentTarget.className+=" active"}var off='{"command":"off"}',reset='{"command":"resetSettings"}'</script><style>.c{text-align:center}div,input{font-size:1em}input{width:95%}body{text-align:center;font-family:verdana}button{border:0;border-radius:.3rem;background-color:#1fa3ec;color:#fff;width:20%;line-height:2rem;font-size:1rem;cursor:pointer;margin:4px 2px;box-shadow:0 8px 16px 0 rgba(0,0,0,.2),0 6px 20px 0 rgba(0,0,0,.19)}.q{float:right;width:64px;text-align:right}button:hover{opacity:.75}.cl{display:inline-block;float:center;width:20%}sliderss>*{vertical-align:middle;display:inline-block}#tabs ul{margin:0;padding:0;list-style-type:none;position:relative;display:block;height:40px;font-size:15px;font-weight:700;border-bottom:1px solid #000;border-top:1px solid #000;text-align:center}#tabs li{display:inline-block;margin-left:5px;padding:0;margin-right:5px}#tabs li a{display:block;float:left;color:#000;height:32px;text-decoration:none;padding:8px 10px 0 10px;border-left:1px solid #000;border-right:1px solid #000}#tabs li a:hover{color:#FFF;height:32px;background-color:#ddd}#tabs li a:focus,.active{background-color:#ccc}.tabcontent{display:none;padding:6px 12px;border:1px solid #ccc;border-top:none;width:320px}</style><div style=text-align:center;display:inline-block;min-width:250px><h3>WiFi RGB Controller - {{}}</h3><div id=tabs><ul><li><a class=tablinks href=javascript:void(0) id=colours onclick='openTab(event,"Colours")'>Colours</a><li><a class=tablinks href=javascript:void(0) id=colours onclick='openTab(event,"Effects")'>Effects</a><li><a class=tablinks href=javascript:void(0) id=settings onclick='openTab(event,"Settings")'>Settings</a></ul></div><div id=Colours class=tabcontent><h4>Colour Select</h4><input value=FFFEFD class=jscolor onchange=update(this.jscolor)><br><br><button onclick=sendRGB(255,0,0) style=background-color:red class=cl>Red</button> <button onclick=sendRGB(0,255,0) style=background-color:green class=cl>Green</button> <button onclick=sendRGB(0,0,255) style=background-color:#00f class=cl>Blue</button> <button onclick=sendRGB(255,255,255) style=color:#000;background-color:#fff class=cl>White</button><br><br><sliderss><label>R:</label><input value=0 id=r max=255 min=0 name=r style=width:70% type=range onchange=updateSliders()><br><label>G:</label><input value=0 id=g max=255 min=0 name=g style=width:70% type=range onchange=updateSliders()><br><label>B:</label><input value=0 id=b max=255 min=0 name=b style=width:70% type=range onchange=updateSliders()></sliderss><h4>Kelvin Scale</h4><label>800</label><input value=0 id=r max=7000 min=800 name=r style=width:70% type=range onchange=sendKelvin(this.value)><label>7000</label><br><h4>Brightness</h4><input value=100 max=100 min=0 type=range onchange=updateBrightness(this.value/100)><br><br><button onclick=sendCommand(off) style=width:50% class=cl>Turn Off</button><br><h4>Set Boot Colour</h4><input value=FFFEFD id=jsbootColour class=jscolor> <button onclick=setBootColour() class=cl>Apply</button> <button onclick=clearBootColour() class=cl>Clear</button><br></div><div id=Effects class=tabcontent><h4>Speed (ms)</h4><label>100</label><input value=1000 id=t max=7000 min=100 name=r style=width:60% type=range><label>7000</label><br><br><button onclick=randomFade() style=width:50%>Random Fade</button><br><br><button onclick=rainbow() style=width:50%>Rainbow</button><br><br><button onclick=flash() style=width:50%>Flash</button><br><br><button onclick=stopAll() style=width:50%>Stop All</button><br><br></div><div id=Settings class=tabcontent><button onclick=restartDevice() style=width:50%>Restart</button><br><br><button onclick=resetSettings() style=width:50%>Reset Settings</button><br><br></div></div><script>document.getElementById("colours").click()</script>
)=====";
