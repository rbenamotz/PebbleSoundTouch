var conf = {};
function unescape(str) {
  if (str===null)
    return str;
  str = str.replace("&apos;","'");
  str = str.replace("&amp;","&");
  str = str.replace("&quot;","\"");
  return str;
}
function readElement(xml, re) {
  var m = re.exec(xml);
  if (m===null)
    return "";
  return unescape(m[1]);
}
var apiCall = function (cmd,  method, body, onload, ontimeout) {  
  var url = "http://" + conf.speakerIp + ":8090/" + cmd;
  var xhr = new XMLHttpRequest();
  if (onload)
    xhr.onload = function(e) {onload(xhr.responseText);};
  xhr.onerror = function(e) {
    Pebble.showSimpleNotificationOnPebble("Error",JSON.stringify(e));
  };
  if (ontimeout===null)
    xhr.ontimeout = function(e) {
      console.log("timeout");
      Pebble.showSimpleNotificationOnPebble("Timeout",JSON.stringify(e));
    };
  else {
    xhr.ontimeout = ontimeout;
  }
  try {
    console.log("Opening " + url + " (" + method + "):" + body);
    xhr.open(method, url);
    xhr.timeout = 2000;
    xhr.send(body);
  }
  catch (err) {
    console.log(JSON.stringify(err));
    Pebble.showSimpleNotificationOnPebble("Error 41","Received status " + JSON.stringify(err) + "\n" + err);
  }
};


// Function to send a message to the Pebble using AppMessage API
function sendMessage(cmd, value) {
  console.log("Sending Message " + cmd + "," + value);
  Pebble.sendAppMessage({"cmd": cmd, "value": value});
}


// Called when JS is ready
Pebble.addEventListener("ready",
							function(e) {
                sendMessage(2,1);
							});
												
// Called when incoming message from the Pebble is received
Pebble.addEventListener("appmessage",
  function(e) {
    console.log("Received: " + JSON.stringify(e.payload));
    var cmd = e.payload.cmd;
    console.log("cmd=" + cmd);
    if (cmd===0 || cmd==3)
      setIp(cmd,e.payload.ip);
    if (cmd===1)
      pushButton(e.payload.buttonId);
    if (cmd===4)
      getNowPlaying();
    if (cmd===5)
      setVolume(e.payload.newVolume);
    if (cmd===6)
      getVolume();
    if (cmd===7)
      getPresets();
    if (cmd===8)
      shutOff();
  });

function getNowPlaying() {
  apiCall("now_playing","get",null,
          function(p) {
            var msg = {};
            msg["10"] = readElement(p,/<artist>(.+?)<\/artist>/m);
            msg["11"] = readElement(p,/<track>(.+?)<\/track>/m);
            msg["12"] = readElement(p,/<album>(.+?)<\/album>/m);
            msg["13"] = readElement(p,/<itemName>(.+?)<\/itemName>/m);
            msg["14"] = readElement(p,/source=\"(.+?)\"/);
            msg["15"] = readElement(p,/<stationLocation>(.+?)<\/stationLocation>/m);
            Pebble.sendAppMessage(msg);
          }, null);
}


function getPresets() {
  apiCall("presets","get",null,
          function(p) {
            var msg = {};
            var re = /<itemName>(.+?)<\/itemName>/g;
            var m;
            var i = 0;
            while ((m = re.exec(p)) !== null) {
              if (m.index === re.lastIndex) {
                re.lastIndex++;
              }
              msg[(1000 + i).toString()] = unescape(m[1]);
              i++;
            }
            Pebble.sendAppMessage(msg);
          }, null);
}

function setIp(cmd, newIp) {
  console.log("Setting ip to " + newIp);
  conf.speakerIp = newIp;
  apiCall("info","get",null,
         function(p) {
           getPresets();
           sendMessage(cmd,1);
         },
         function(p) {
           sendMessage(cmd,0);
         });
}
function pushButton(buttonId) {
  var body = "<key state=\"release\" sender=\"Gabbo\">PRESET_" + (buttonId + 1) + "</key>";
  apiCall("key","post",body,
         function(p) {
           //sendMessage(1);
         });
}
function getVolume() {
            apiCall("volume","get",null,
                    function (p) {
                      var msg = {};
                      msg["20"]= parseInt(readElement(p,/<actualvolume>(.+?)<\/actualvolume>/m));
                      Pebble.sendAppMessage(msg);
                    }, null);
  
}
function setVolume(newVolume) {
  console.log("Setting volume to  " + newVolume);
  var body = "<volume>" + newVolume + "</volume>";
  apiCall("volume","post",body,null, null);
}

function shutOff() {
  var body = "<key state=\"release\" sender=\"Gabbo\">POWER</key>";
  apiCall("key","post",body);
}


