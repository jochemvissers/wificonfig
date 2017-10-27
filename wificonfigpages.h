#include  "pgmspace.h"

const char HTTP_HEAD_[] PROGMEM =
  "<!DOCTYPE html><html lang=\"en\" class=\"\">"
  "<head>"
  "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1,user-scalable=no\"/>"
  "<meta http-equiv=\"Content-Type\" content=\"text/html\"; charset='utf-8' />"
  "<title>{v}</title>";

const char HTML_WIFICONF_STYLE[] PROGMEM = R"=====(
  body { color: #000000; font-family: avenir, helvetica, arial, sans-serif;  letter-spacing: 0.15em;}
  hr {    background-color: #eee;    border: 0 none;   color: #eee;    height: 1px; }
  .btn, .btn:link, .btn:visited {
  	border-radius: 0.3em;
  	border-style: solid;
  	border-width: 1px;
  color: #111;
  display: inline-block;
  	font-family: avenir, helvetica, arial, sans-serif;
  	letter-spacing: 0.15em;
  	margin-bottom: 0.5em;
  padding: 1em 0.75em;
  	text-decoration: none;
  	text-transform: uppercase;
  	-webkit-transition: color 0.4s, background-color 0.4s, border 0.4s;
  transition: color 0.4s, background-color 0.4s, border 0.4s;
  }
  .btn:hover, .btn:focus {
  color: #7FDBFF;
  border: 1px solid #7FDBFF;
  	-webkit-transition: background-color 0.3s, color 0.3s, border 0.3s;
  transition: background-color 0.3s, color 0.3s, border 0.3s;
  }
  	.btn:active {
  color: #0074D9;
  border: 1px solid #0074D9;
  		-webkit-transition: background-color 0.3s, color 0.3s, border 0.3s;
  transition: background-color 0.3s, color 0.3s, border 0.3s;
  	}
  	.btn--s
  	{
  		font-size: 12px;
  	}
  	.btn--m {
  		font-size: 14px;
  	}
  	.btn--l {
  		font-size: 20px;  border-radius: 0.25em !important;
  	}
  	.btn--full, .btn--full:link {
  		border-radius: 0.25em;
  display: block;
  			margin-left: auto;
  			margin-right: auto;
  			text-align: center;
  width: 100%;
  	}
  	.btn--blue:link, .btn--blue:visited {
  color: #fff;
  		background-color: #0074D9;
  	}
  	.btn--blue:hover, .btn--blue:focus {
  color: #fff !important;
  		background-color: #0063aa;
  		border-color: #0063aa;
  	}
  	.btn--blue:active {
  color: #fff;
  		background-color: #001F3F;  border-color: #001F3F;
  	}
  	@media screen and (min-width: 32em) {
  		.btn--full {
  			max-width: 16em !important; }
  	}
)=====";

const char HTTP_HEAD_END_[] PROGMEM        = R"=====(
  </head><body><center><div style='text-align:left;display:inline-block;min-width:260px;'>
  <link rel="stylesheet" href="style.css" type="text/css" />
)=====";

const char HTTP_END_[] PROGMEM             = "</div></center></body></html>";

const char HTML_WIFICONF_MAIN[] PROGMEM    = R"=====(
  <a href="/"  class="btn btn--s"><</a>&nbsp;&nbsp;<strong>Network Configuration</strong><hr>
)=====";

const char HTML_WIFICONF_FORMBEGIN[] PROGMEM = "<form action=\"\\wificonfig\\save\" method=\"post\">";
const char HTML_WIFICONF_FORMEND[] PROGMEM  = "</form>";
const char HTML_WIFICONF_FORM[] PROGMEM = R"=====(
  <table border="0"  cellspacing="0" cellpadding="3" style="width:310px" >
  <tr><td align="right">Accesspoint mode:</td><td><input type="checkbox" id="softap" name="softap"></td></tr>
  <tr><td align="right">Device Name:</td><td><input type="text" id="devicename" name="devicename" value=""></td></tr>
  <tr><td align="right">SSID:</td><td><input type="text" id="ssid" name="ssid" value=""></td></tr>
  <tr><td align="right">Password:</td><td><input type="text" id="pwd" name="pwd" value=""></td></tr>
  <tr><td align="right">DHCP:</td><td><input type="checkbox" id="dhcp" name="dhcp"></td></tr>
  <tr><td align="right">IP:     </td><td><input type="text" id="ip0" name="ip0" size="3">.<input type="text" id="ip1" name="ip1" size="3">.<input type="text" id="ip2" name="ip2" size="3">.<input type="text" id="ip3" name="ip3" value="" size="3"></td></tr>
  <tr><td align="right">Netmask:</td><td><input type="text" id="nm0" name="nm0" size="3">.<input type="text" id="nm1" name="nm1" size="3">.<input type="text" id="nm2" name="nm2" size="3">.<input type="text" id="nm3" name="nm3" size="3"></td></tr>
  <tr><td align="right">Gateway:</td><td><input type="text" id="gw0" name="gw0" size="3">.<input type="text" id="gw1" name="gw1" size="3">.<input type="text" id="gw2" name="gw2" size="3">.<input type="text" id="gw3" name="gw3" size="3"></td></tr>
  <tr><td align="right">MQTT Server:</td><td><input type="text" id="mqaddr" name="mqaddr" value=""></td></tr>
  <tr><td align="right">MQTT Password:</td><td><input type="text" id="mqpass" name="mqpass" value=""></td></tr>
  <tr><td colspan="2" align="center"><input type="submit" style="width:150px" class="btn btn--m btn--blue" value="Save"></td></tr>
  <tr><td colspan="2" align="center"><a href="/wificonfig/reset" style="width:150px" class="btn btn--m btn--blue">Reset all</a></td></tr>
  </table>
)=====";


const char HTML_WIFICONF_NETWORKSTATUS[] PROGMEM = R"=====(
  <hr>
  <strong>Connection State:</strong><div id="connectionstate">N/A</div>
  <hr>
  <strong>Networks:</strong><br>
  <table border="0"  cellspacing="3" style="width:310px" >
  <tr><td><div id="networks">Scanning...</div></td></tr>
  <tr><td align="center"><a href="javascript:GetState()" style="width:150px" class="btn btn--m btn--blue">Refresh</a></td></tr>
  </table>
)=====";

const char HTML_WIFICONF_SAVED[] PROGMEM = R"=====(
  <meta http-equiv="refresh" content="5; URL=/wifi">
  Saving and restarting. Please Wait...
)=====";
const char HTML_WIFICONF_SCRIPT[] PROGMEM = R"=====(
  <script>
  function microAjax(B,A){this.bindFunction=function(E,D){return function(){return E.apply(D,[D])}};this.stateChange=function(D){if(this.request.readyState==4){this.callbackFunction(this.request.responseText)}};this.getRequest=function(){if(window.ActiveXObject){return new ActiveXObject("Microsoft.XMLHTTP")}else{if(window.XMLHttpRequest){return new XMLHttpRequest()}}return false};this.postBody=(arguments[2]||"");this.callbackFunction=A;this.url=B;this.request=this.getRequest();if(this.request){var C=this.request;C.onreadystatechange=this.bindFunction(this.stateChange,this);if(this.postBody!==""){C.open("POST",B,true);C.setRequestHeader("X-Requested-With","XMLHttpRequest");C.setRequestHeader("Content-type","application/x-www-form-urlencoded");C.setRequestHeader("Connection","close")}else{C.open("GET",B,true)}C.send(this.postBody)}};

  function setValues(url)
  {
    microAjax(url, function (res)
    {
    	res.split(String.fromCharCode(10)).forEach(function(entry) {
    	fields = entry.split("|");
    	if(fields[2] == "input")
    	{
    			document.getElementById(fields[0]).value = fields[1];
    	}
    	else if(fields[2] == "div")
    	{
    			document.getElementById(fields[0]).innerHTML  = fields[1];
    	}
    	else if(fields[2] == "chk")
    	{
    			document.getElementById(fields[0]).checked  = fields[1];
    	}
      });
    });
  }

  function GetState()
  {
  	setValues("/wificonfig/connectionstate");
  }
  function selssid(value)
  {
  	document.getElementById("ssid").value = value;
  }

  window.onload = function ()
  {
    setValues("/wificonfig/values");
    setTimeout(GetState,3000);
  }

  </script>
)=====";
