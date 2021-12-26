#pragma once

#include <ArduinoJson.h>
enum WIFI_STATUS { WIFI_NOT_CONNECTED = 0, WIFI_CONNECTED =1,  WIFI_HOTSPOT = 2 };
enum MushroomFruitSpaceStatus { STANDBY = 0, VENTING=1, MISTING_AFTER_VENT=2, MISTING_SHORT=3 };

class ControllerStatus {
public:
	bool SensorValid;
	float tmp;
	WIFI_STATUS WifiStatus;
	bool HeatIsOn;
	char Ip[50];
	MushroomFruitSpaceStatus MushroomStatus;
	uint32_t MilisLastMist;
	uint32_t MillisOfLastVent ;
	uint32_t MillisVentStatusChange ;
	uint32_t MillisMistStatusChange ;

	ControllerStatus() {
		WifiStatus = WIFI_NOT_CONNECTED;
		tmp = -10;
		SensorValid = false;
		HeatIsOn = false;
		sprintf(Ip, "0.0.0.0");
		MushroomStatus = STANDBY;
		 MilisLastMist = 0;
		 MillisOfLastVent = 0;
		 MillisVentStatusChange = 0;
		 MillisMistStatusChange = 0;
	}

	void PrintStatus() {
		Serial.println("========== status ==========");
		Serial.printf("Sensor valid = %s, tmp =%f\n", SensorValid ? "OK" : "FAULT", tmp);
		if (WifiStatus == WIFI_CONNECTED) {
			Serial.printf("Wifi Connected, IP=%s\n", Ip);
		}
		else {
			Serial.printf("Wifi DIS-Connected\n");
		}
		Serial.printf("Heat status = %s\n", HeatIsOn ? "ON" : "OFF");
		Serial.println("==========================");
	
	}

	void printJsonStatus(char* buff, int len) {
		DynamicJsonDocument doc(len);//StaticJsonDocument<CAPACITY> doc;
	// create an object
		JsonObject object = doc.to<JsonObject>();
		object["WifiStatus"] = WifiStatus;
		object["SensorValid"] = SensorValid;
		object["tmp"] = tmp;
		object["HeatIsOn"] = HeatIsOn;
		object["SecsAlive"] = ((uint32_t)millis() / 1000);
		object["SecsSinceLastVent"] = (uint32_t)((millis()- MillisOfLastVent) / 1000);
		object["SecsSinceLastMist"] = (uint32_t)((millis() - MilisLastMist) / 1000);
		int res = serializeJson(doc, buff, len);
	}
};


ControllerStatus Status;