#pragma once
#include "Settings.h"
#include <WebServer.h>

WebServer* webServer;

void setAddresses();
void notFoundResponse();
void rootResponse();
void respondStatus();
void respondSettings();
void storeSettings();
void restartController();


void SetupWebServer() {
	webServer = new WebServer(80);
	setAddresses();
	webServer->begin();
}

void notFoundResponse() {
	webServer->send(400, "text/html", "failed to find page");
}

void respondStatus() {
	char buff[300];
	Status.printJsonStatus(buff, 300);
	webServer->send(200, "text/html", buff);
}

void respondSettings() {
    char buff[300];
    Settings.printSettingsToBuff(buff, 300);
    webServer->send(200, "text/html", buff);
}

void storeSettings() {
    Settings.storeSettings();
    webServer->send(200, "text/html", "Settings Stored");
}

void restartController() {
    webServer->send(200, "text/html", "restarting controller");
    ESP.restart();
}

void setSettings() {
    Serial.println("responding to new settings");
    if (webServer->hasArg("plain") == false) { //Check if body received
        Serial.println("Body not received");
        webServer->send(400, "text/plain", "Body not received");
        return;
    }
    String body = webServer->arg("plain");
    Serial.println("Settings Body received:");
    Serial.println(body);
    Settings.updateSettings(body);
    webServer->send(200, "text/html", "OK");
}

void (*onVent)(void);
void (*onMist)(void);

void vent() {
    onVent();
    webServer->send(200, "text/html", "OK");
}

void mist() {
    onMist();
    webServer->send(200, "text/html", "OK");
}

void setAddresses() {
	webServer->onNotFound(notFoundResponse);
	webServer->on("/", rootResponse);
	webServer->on("/status", respondStatus);
    webServer->on("/settings", HTTP_GET, respondSettings);
    webServer->on("/settings", HTTP_POST, setSettings);
    webServer->on("/storesettings", HTTP_GET, storeSettings);
    webServer->on("/restart", HTTP_GET, restartController);
    webServer->on("/vent", HTTP_GET, vent);
    webServer->on("/mist", HTTP_GET, mist);
}

void TickWebServer() {
	webServer->handleClient();
}




const char IndexPage[] PROGMEM = R"=====( 
<!DOCTYPE html>
<html>
<head>
	<title>Mushroom TMP control</title>
	
	 <script>
       

         function getFromControllerAndDo(path, toDo) {
             var xhr = new XMLHttpRequest();

             xhr.onload = function () {
                 if (this.status == 200) toDo(xhr.responseText);
                 else alert("problem");
             }
             xhr.open("Get", path, false);
             xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
             xhr.send();
         }

         //const DEBUG = 1;
         function getData() {
             //if (DEBUG) {
             //    newDataStr('{ "WifiStatus": 1, "SensorValid": false, "tmp": -10, "HeatIsOn": false }');
             //    return;
             //}

             //newDataStr('{ "WifiStatus": 1, "SensorValid": false, "tmp": -10, "HeatIsOn": false, "SecsAlive": 46, "SecsSinceLastVent": 26, "SecsSinceLastMist": 1 }');
            // newSettingsStr('{"SSID":"TalHome","SSID_PASS":"pixelismycat","highTemp":30,"lowTemp":27,"AP_MODE":0,"MysiliumGrowActive":0,"MushroomFruitActive":1,"MinsBetweenVent":180,"SecsOfVent":15,"SecsHumidAfterVent":25,"MinsBetweenMist":20,"SecsOfShortMist":8}');
            // return;
             getFromControllerAndDo("/status", str => { newDataStr(str); });
             getFromControllerAndDo("/settings", str => { newSettingsStr(str); })
             
         }

        

         //function getDataTest() {
         //    newDataStr('{ "WifiStatus": 1, "SensorValid": false, "tmp": -10, "HeatIsOn": false }');
         //}

         function newDataStr(dataStr) {
             //alert("new data: " + dataStr);
             const obj = JSON.parse(dataStr);
             document.getElementById("SecsAlive_h3").innerHTML = "Seconds alive = "+obj.SecsAlive;
             if (obj.SensorValid) {
                 document.getElementById("tmp_h3").innerHTML = "tmp: " + obj.tmp + " C";
                 document.getElementById("tmp_h3").style.color = "blue";
             } else {
                 document.getElementById("tmp_h3").innerHTML = "Temprature sensor not connected";
                 document.getElementById("tmp_h3").style.color = "red";
             }
             document.getElementById("heat_h3").innerHTML = "Heat is " + (obj.HeatIsOn ? "ON" : "OFF");

             document.getElementById("SecsSinceLastVent_h3").innerHTML = "Seconds since Vent: "+obj.SecsSinceLastVent;
             document.getElementById("SecsSinceLastMist_h3").innerHTML = "Seconds since Mist: " +obj.SecsSinceLastMist;
             
         }

         function updateView() {
             document.getElementById("mushroomDiv").style.visibility = (document.getElementById("MushroomCheckbox").checked ? 'visible' : 'hidden'  );
             document.getElementById("mysiliumDiv").style.visibility = (document.getElementById("MysiliumCheckbox").checked ? 'visible' : 'hidden' );
         }

         function newSettingsStr(settingsStr) {
             const obj = JSON.parse(settingsStr);
             document.getElementById("low_tmp").value = obj.lowTemp;
             document.getElementById("high_tmp").value = obj.highTemp;
             document.getElementById("SSID").value = obj.SSID;
             document.getElementById("SSID_PASS").value = obj.SSID_PASS;
             document.getElementById("AP_MODE").checked = obj.AP_MODE != 0;
             document.getElementById("MinsBetweenVents").value = obj.MinsBetweenVent;
             document.getElementById("SecsOfVent").value = obj.SecsOfVent;
             document.getElementById("SecsOfMistAfterVent").value = obj.SecsHumidAfterVent;
             document.getElementById("MinsBetweenMist").value = obj.MinsBetweenMist;
             document.getElementById("SecsOfShortMist").value = obj.SecsOfShortMist;
             document.getElementById("MushroomCheckbox").checked = obj.MushroomFruitActive;
             document.getElementById("MysiliumCheckbox").checked = obj.MysiliumGrowActive;

             updateView();
         }

         function sendPostToUrl(url, obj) {
             let date = Date.now();
             var xhr = new XMLHttpRequest();
             xhr.onload = function () {
                 if (this.status == 200) {
                     alert("OK");
                 } else {
                     alert("ERROR");
                 }
             }
             xhr.open("Post", url, true);
             xhr.setRequestHeader("Content-type", "application/json");
             xhr.send(JSON.stringify(obj));
         }

         function updateSettings() {
             var obj = {};
             obj.lowTemp = document.getElementById("low_tmp").value;
             obj.highTemp = document.getElementById("high_tmp").value;
             obj.SSID = document.getElementById("SSID").value;
             obj.SSID_PASS = document.getElementById("SSID_PASS").value;
             obj.AP_MODE = (document.getElementById("AP_MODE").checked ? 1 : 0);
             obj.MysiliumGrowActive = document.getElementById("MysiliumCheckbox").checked;
             obj.MushroomFruitActive = document.getElementById("MushroomCheckbox").checked;
             obj.SecsOfShortMist = document.getElementById("SecsOfShortMist").value;
             obj.MinsBetweenMist = document.getElementById("MinsBetweenMist").value;
             obj.SecsHumidAfterVent = document.getElementById("SecsOfMistAfterVent").value;
             obj.SecsOfVent = document.getElementById("SecsOfVent").value;
             obj.MinsBetweenVent = document.getElementById("MinsBetweenVents").value;
             sendPostToUrl("/settings" ,obj);
         }

         function StoreSettings() {
             getFromControllerAndDo("/storesettings", str => { alert(str); });
         }
         function RestartController() {
             getFromControllerAndDo("/restart", str => { alert(str); });
         }
         function vent() {
             getFromControllerAndDo("/vent", str => { alert(str); });
         }
         function mist() {
             getFromControllerAndDo("/mist", str => { alert(str); });
         }
	 </script>
</head>

<body onload="getData()">
    <h3 id="SecsAlive_h3"></h3>
    <br />

    <input type="checkbox" id="MysiliumCheckbox" onchange="updateView()"/><br>
    <div id="mysiliumDiv">
        <h1>Mushrooms incubator state</h1>

        <h3 id="tmp_h3"></h3>
        <h3 id="heat_h3"></h3>

        <br />
        <button onclick="getData()"> Get Data</button>

        <h1>Mysilium incubator settings</h1>
        <label for="low_tmp">low tmp</label>
        <input id="low_tmp" type="number" min="10" max="40" />
        -
        <input id="high_tmp" type="number" min="10" max="40" />
        <label for="high_tmp">high tmp</label><br>
    </div>

    <input type="checkbox" id="MushroomCheckbox" onchange="updateView()"/><br>
    <div id="mushroomDiv">
        <h1>Mushrooms fruit chamber settings</h1>
        <label for="MinsBetweenVents">Mins between vents:</label>
        <input id="MinsBetweenVents" type="number" min="5" max="1440" />
        <label for="SecsOfVent">Secs of vents:</label>
        <input id="SecsOfVent" type="number" min="1" />
        <label for="SecsOfMistAfterVent">Secs mist after:</label>
        <input id="SecsOfMistAfterVent" type="number" min="1" />
        <br />
        <label for="MinsBetweenMist">Mins between mists:</label>
        <input id="MinsBetweenMist" type="number" min="2" />
        <label for="SecsOfShortMist">Secs of short mist:</label>
        <input id="SecsOfShortMist" type="number" min="1" max="200" />

        <h1>Mushrooms fruit chamber status</h1>
        <h3 id="SecsSinceLastVent_h3"></h3>
        <h3 id="SecsSinceLastMist_h3"></h3>
         <button onclick="vent()">VENT</button>
         <button onclick="mist()">MIST</button>
    </div>
    <h1>Network settings</h1>
    <label for="AP_MODE">Acces Point mode </label>
    <input type="checkbox" id="AP_MODE" /><br>

    <label for="SSID">SSID</label>
    <input type="text" id="SSID" /><br>
    <label for="SSID_PASS">SSID_PASS</label>
    <input type="text" id="SSID_PASS" /><br>
    <br>
    <button onclick="updateSettings()"> Update Settings</button>
    <button onclick="StoreSettings()"> Store Settings</button>
    <button onclick="RestartController()"> Restart</button>

</body>
</html>

)=====";

void rootResponse() {
	webServer->send(200, "text/html", IndexPage);
}