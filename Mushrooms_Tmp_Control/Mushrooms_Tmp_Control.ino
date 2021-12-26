#include <WiFi.h>

#include "Status.h"
#include "TmpSensorInterface.h"
#include "ControlWebServer.h"
#include "Settings.h"

void buttonIsPressed();
void DoMushroomFruitControl();
void TurnOnMist();
void TurnOffMist();
void TurnOnVent();
void TurnOffVent();
void doMist();
void doVent();
bool initDone = false;
//bool AP_MODE = true;


TempratureSensorModule* tmpSensor;
// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 13 as an output.
  
  Serial.begin(115200);
  WiFi.mode(WIFI_OFF);
  delay(1000);
  Settings.init(true); // TODO - just for 

  if (Settings.AP_MODE) {
	  Serial.printf("WIFI AP: %s, %s\n", AP_SSID, AP_SSID_PASS);
	  WiFi.mode(WIFI_AP);
	  WiFi.softAP(AP_SSID, AP_SSID_PASS);
  }
  else {
	  Serial.printf("WIFI: %s, %s\n", Settings.SSID, Settings.SSID_PASS);
	  WiFi.mode(WIFI_STA);
	  WiFi.begin(DEFAULT_SSID, DEFAULT_SSID_PASS);
	  delay(1000);
  }
  pinMode(HEATER_CONTROL_PIN, OUTPUT);
  digitalWrite(HEATER_CONTROL_PIN, HIGH);
  pinMode(VENT_CONTROL_PIN, OUTPUT);
  digitalWrite(VENT_CONTROL_PIN, HIGH);
  pinMode(HUMIDIFIER_CONTROL_PIN, OUTPUT);
  digitalWrite(HUMIDIFIER_CONTROL_PIN, HIGH);
  //pinMode(NEW_LED, OUTPUT);
  //digitalWrite(NEW_LED, LOW);


  //pinMode(NETWORK_STATUS_LED, OUTPUT);
  //pinMode(SENSOR_LED, OUTPUT);
  pinMode(LED, OUTPUT);

  doInit();
  Settings.printSettings(WiFi.getMode() == WIFI_AP);
  onVent = doVent;
  onMist = doMist;
}

void doInit() {
	if (initDone) return;
	initDone = true;
	tmpSensor = new TempratureSensorModule(TMP_SENSOR_PIN);
	tmpSensor->updateRead();
	delay(100);
	SetupWebServer();
}

void MysiliumGromTick() {
	if (!Settings.MysiliumGrowActive) return;
	tmpSensor->updateRead();

	if (Status.SensorValid) {
		//Serial.print("tmp sensor is OK, tmp ="); Serial.println(Status.tmp);
		if (Status.tmp > Settings.highTemp) {
			TurnOffHeat();
		}
		if (Status.tmp < Settings.lowTemp) {
			TurnOnHeat();
		}
	}
	else {
		TurnOffHeat();
	}
}
// the loop function runs over and over again forever
int i;
void loop() {
	
	if (digitalRead(button_pin) == 0) {
		buttonIsPressed();
	}

	/*if (Status.WifiStatus == WIFI_CONNECTED) {
		doInit();
	}*/
	/*
	if (i % 20 == 0) {
		digitalWrite(VENT_CONTROL_PIN, !digitalRead(VENT_CONTROL_PIN));
		digitalWrite(HUMIDIFIER_CONTROL_PIN, !digitalRead(VENT_CONTROL_PIN));
		digitalWrite(NEW_LED, !digitalRead(VENT_CONTROL_PIN));
		//Serial.println("hopa");
	}
	*/

	if(i++%50 == 0){		
		WifiStatus();

		MysiliumGromTick();

		Status.PrintStatus();
	}

	if (i % 10 == 0) {		DoMushroomFruitControl();	}
	
	if (i % 5 == 0) updateLEDs();

	if (initDone && Status.WifiStatus == WIFI_CONNECTED) {
		TickWebServer();
	}


	if (i >= 1000) {
		i = 0;
		Settings.printSettings(WiFi.getMode() == WIFI_AP);
	}

	delay(100);
}

void BlinkLed() {
	//digitalWrite(NETWORK_STATUS_LED, !digitalRead(NETWORK_STATUS_LED));
	digitalWrite(LED, !digitalRead(LED));
}

void updateLEDs() {
	//BlinkLed(); return;

	if (!Status.SensorValid || Status.WifiStatus != WIFI_CONNECTED) {
		BlinkLed();
	}
	else {
		digitalWrite(LED, HIGH);
	}
	/*
	if (Status.SensorValid) {
		digitalWrite(SENSOR_LED, LOW);
	}
	else {
		digitalWrite(SENSOR_LED, !digitalRead(SENSOR_LED));
	}
	if (Status.WifiStatus == WIFI_CONNECTED) {
		digitalWrite(NETWORK_STATUS_LED, LOW);
	}
	else {
		digitalWrite(NETWORK_STATUS_LED, !digitalRead(NETWORK_STATUS_LED));
	}
	*/
}




//bool StatusOfVent = false;
void doMist() {
	TurnOffVent();
	TurnOnMist();
	Status.MushroomStatus = MISTING_SHORT;
}

void doVent() {
	TurnOffMist();
	TurnOnVent();
	
}

void DoMushroomFruitControl() {
	if (!Settings.MushroomFruitActive) return;
	if(Status.MushroomStatus != STANDBY)
		Serial.printf("mushroom status = %d\n", Status.MushroomStatus);
	switch (Status.MushroomStatus) {
	case STANDBY:
		if (abs(millis()-Status.MillisOfLastVent) > (((uint32_t)60000) * Settings.MinsBetweenVent)
			|| Status.MillisOfLastVent==0) {
			TurnOnVent();
			return;
		}
		if (abs(millis()- Status.MilisLastMist) > (((uint32_t)60000) * Settings.MinsBetweenMist)) {
			TurnOnMist();
			Status.MushroomStatus = MISTING_SHORT;
			return;
		}
		return;
	case VENTING:
		if (abs(millis()- Status.MillisVentStatusChange) > (((uint32_t)1000) * Settings.SecsOfVent)) {
			TurnOffVent();
			TurnOnMist();
			Status.MushroomStatus = MISTING_AFTER_VENT;
		}
		return;
	case MISTING_AFTER_VENT:
		if (abs(millis()- Status.MillisMistStatusChange) > (((uint32_t)1000) * Settings.SecsHumidAfterVent)) {
			TurnOffMist();
		}
		return;
	case MISTING_SHORT:
		if (abs(millis()- Status.MillisMistStatusChange) > (((uint32_t)1000) * Settings.SecsOfShortMist)) {
			TurnOffMist();
		}
		return;
	}
}

void TurnOnMist() {
	Serial.println("Turning ON mist");
	digitalWrite(HUMIDIFIER_CONTROL_PIN, LOW);
	Status.MillisMistStatusChange = millis();
}

void TurnOffMist() {
	Serial.println("Turning OFF mist");
	digitalWrite(HUMIDIFIER_CONTROL_PIN, HIGH);
	Status.MilisLastMist = millis();
	Status.MillisMistStatusChange = millis();
	Status.MushroomStatus = STANDBY;
}

void TurnOnVent() {
	Serial.println("Turning ON vent");
	digitalWrite(VENT_CONTROL_PIN, LOW);
	Status.MillisVentStatusChange = millis();
	Status.MushroomStatus = VENTING;
}

void TurnOffVent() {
	Serial.println("Turning OFF vent");
	digitalWrite(VENT_CONTROL_PIN, HIGH);
	Status.MillisOfLastVent = millis();
	Status.MillisVentStatusChange = millis();
	Status.MushroomStatus = STANDBY;

}



int wifiCounter = 0;
void WifiStatus() {
	if (WiFi.getMode() == WIFI_AP) {
		Status.WifiStatus = WIFI_CONNECTED;
		sprintf(Status.Ip, WiFi.softAPIP().toString().c_str());
		return;
	}
	if (WiFi.status() == WL_CONNECT_FAILED || WiFi.status() == WL_DISCONNECTED
		|| WiFi.status() == WL_NO_SSID_AVAIL)
	{
		Status.WifiStatus = WIFI_NOT_CONNECTED;
		sprintf(Status.Ip, "0.0.0.0");
		Serial.printf("Network status = %d\n", WiFi.status());
		if (wifiCounter++ % 5 == 4) {
			Serial.println("Wifi trying to reconnect");
			WiFi.reconnect();
		}
	}
	else {
		Status.WifiStatus = WIFI_CONNECTED;
		sprintf(Status.Ip, WiFi.localIP().toString().c_str());
		wifiCounter = 0;
	}
	
}
int ttt = 0;
void TurnOnHeat() {
	//digitalWrite(HEATER_CONTROL_PIN,ttt++%2==0 ? HIGH:LOW );
	digitalWrite(HEATER_CONTROL_PIN, LOW); 
	Serial.println("turning on heat");
	Status.HeatIsOn = true;
}

void TurnOffHeat() {
	digitalWrite(HEATER_CONTROL_PIN, HIGH); 
	Serial.println("turning off heat");
	Status.HeatIsOn = false;
}


void buttonIsPressed() {
	Serial.println("start button pressed");
	unsigned long startPress = millis();
	while (true) {
		if (digitalRead(button_pin) != 0) {
			Status.PrintStatus();
			Settings.printSettings(WiFi.getMode() == WIFI_AP);
			return;
		}
		else {
			if (abs(millis() - startPress) > 5000) {
				for (int i = 0; i < 10; i++) { BlinkLed(); delay(100); }
				Settings.AP_MODE = !Settings.AP_MODE;
				if (Settings.AP_MODE) {
					Serial.printf("Switched to settings mode (AP: %s :: %s)\n", AP_SSID, AP_SSID_PASS);
					String IpStr = WiFi.localIP().toString();
					sprintf(Status.Ip, "%s", IpStr.c_str());
					Status.Ip[IpStr.length()] = 0;
					Serial.printf("LastKnowIP =%s\n", Status.Ip);
					WiFi.mode(WIFI_OFF);
					delay(100);
					WiFi.mode(WIFI_AP);
					WiFi.softAP(AP_SSID, AP_SSID_PASS);
					IPAddress myIP = WiFi.softAPIP();
					Serial.print("AP IP address: "); Serial.println(myIP);
					//ResetWebServer(80);
					Status.WifiStatus == WIFI_HOTSPOT;
				}
				else {
					Serial.println("Switched from settings mode to regular mode");
					///WiFi.mode(WIFI_OFF);
					delay(100);
					WiFi.mode(WIFI_STA);
					WiFi.begin("TalHome", "pixelismycat");
					Serial.print(Settings.SSID); Serial.print(" , ");
					Serial.println(Settings.SSID_PASS);
				}
				return;
			}

		}
		delay(30);
	}
}