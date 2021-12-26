#pragma once
#include <EEPROM.h>
#include <ArduinoJson.h>

#define button_pin	0

#define TMP_SENSOR_PIN		18
#define HEATER_CONTROL_PIN	22
#define VENT_CONTROL_PIN	23
#define HUMIDIFIER_CONTROL_PIN	21

#define LED	32
//#define NEW_LED	13


#define DEFAULT_HIGH_TMP			29
#define DEFAULT_LOW_TMP				25
#define DEFAULT_SSID				"TalHome"
#define DEFAULT_SSID_PASS			"pixelismycat"
#define AP_SSID	"Tal_shroom_01"
#define AP_SSID_PASS	"12345678"


class ControllerSettings {
public:
	float highTemp;
	float lowTemp;
	int AP_MODE;
	int MysiliumGrowActive;

	int MushroomFruitActive;
	int MinsBetweenVent;
	int SecsOfVent;
	int SecsHumidAfterVent;
	int MinsBetweenMist;
	int SecsOfShortMist;

	int dummy[12];
	char SSID[20];
	char SSID_PASS[30];

	void printSettings(bool ap) {
		Serial.println("=========== Settings ============");
		if (MysiliumGrowActive) {
			Serial.println("Mysilium grow control:");
			Serial.printf("\tlow tmp:%f - high tmp: %f\n", lowTemp, highTemp);
		}
		else {
			Serial.println("Mysilium grow is not active");
		}
		if (MushroomFruitActive) {
			Serial.println("Mushroom fruit control:");
			Serial.printf("\tMinsBetweenVent:%d, SecsOfVent:%d, SecsMistAfterVent: %d\n", MinsBetweenVent, SecsOfVent, SecsHumidAfterVent);
			Serial.printf("\tMinsBetweenMist:%d - SecsOfShortMist: %d\n", MinsBetweenMist, SecsOfShortMist);
		}
		else {
			Serial.println("Mushroom fruit control is not active");
		}
		
		if(ap)
			Serial.printf("WIFI AP: %s, %s\n", AP_SSID, AP_SSID_PASS);
		else
			Serial.printf(" SSID: \"%s\" --Pass: \"%s\"\n", SSID, SSID_PASS);
		Serial.println("===================================");
	}

	void init(bool initFromEEPROM) {
		highTemp = DEFAULT_HIGH_TMP;
		lowTemp = DEFAULT_LOW_TMP;
		sprintf(SSID, DEFAULT_SSID);
		sprintf(SSID_PASS, DEFAULT_SSID_PASS);
		AP_MODE = 0;
		MysiliumGrowActive = 0;
		MushroomFruitActive = 1;
		MinsBetweenVent = 180;
		SecsOfVent = 15;
		SecsHumidAfterVent = 25;
		MinsBetweenMist = 20;
		SecsOfShortMist = 8;

		delay(1000);
		EEPROM.begin(512);  //Initialize EEPROM
		delay(100);
		if (digitalRead(button_pin) == 0 || !initFromEEPROM) {
			Serial.printf("Starting without EEPROM \n");
		}
		else {
			Serial.printf("Getting conf... sizeof(Settings) = %d \n", sizeof(ControllerSettings));
			EEPROM.get(0, *this); //EEPROM.get(0, conf);
		}


	}

	ControllerSettings() {
		//init();
	}	

	void storeSettings() {
		EEPROM.put(0, *this);
		EEPROM.commit();
	}

	int printSettingsToBuff(char* buff, int len) {
		DynamicJsonDocument doc(len);//StaticJsonDocument<CAPACITY> doc;
	// create an object
		JsonObject jsonObject = doc.to<JsonObject>();
		jsonObject["SSID"] = SSID;
		jsonObject["SSID_PASS"] = SSID_PASS;
		jsonObject["highTemp"] = highTemp;
		jsonObject["lowTemp"] = lowTemp;
		jsonObject["AP_MODE"] = AP_MODE;
		jsonObject["MysiliumGrowActive"] = MysiliumGrowActive;
		jsonObject["MushroomFruitActive"] = MushroomFruitActive;
		jsonObject["MinsBetweenVent"] = MinsBetweenVent;
		jsonObject["SecsOfVent"] = SecsOfVent;
		jsonObject["SecsHumidAfterVent"] = SecsHumidAfterVent;
		jsonObject["MinsBetweenMist"] = MinsBetweenMist;
		jsonObject["SecsOfShortMist"] = SecsOfShortMist;
		return serializeJson(doc, buff, len);
	}

	void updateSettings(String newSettingsStr) {
		DynamicJsonDocument doc(400);
		deserializeJson(doc, newSettingsStr);

		JsonObject obj = doc.as<JsonObject>();
		if (!obj.containsKey("SecsOfShortMist")) {
			Serial.printf("new settings does not contain 'SecsOfShortMist'\n");
		}
		else {
			SecsOfShortMist = obj["SecsOfShortMist"];
		}
		if (!obj.containsKey("MinsBetweenMist")) {
			Serial.printf("new settings does not contain 'MinsBetweenMist'\n");
		}
		else {
			MinsBetweenMist = obj["MinsBetweenMist"];
		}
		if (!obj.containsKey("SecsHumidAfterVent")) {
			Serial.printf("new settings does not contain 'SecsHumidAfterVent'\n");
		}
		else {
			SecsHumidAfterVent = obj["SecsHumidAfterVent"];
		}
		if (!obj.containsKey("SecsOfVent")) {
			Serial.printf("new settings does not contain 'SecsOfVent'\n");
		}
		else {
			SecsOfVent = obj["SecsOfVent"];
		}
		if (!obj.containsKey("MysiliumGrowActive")) {
			Serial.printf("new settings does not contain 'MysiliumGrowActive'\n");
		}
		else {
			MysiliumGrowActive = obj["MysiliumGrowActive"];
		}
		if (!obj.containsKey("MushroomFruitActive")) {
			Serial.printf("new settings does not contain 'MushroomFruitActive'\n");
		}
		else {
			MushroomFruitActive = obj["MushroomFruitActive"];
		}
		if (!obj.containsKey("MinsBetweenVent")) {
			Serial.printf("new settings does not contain 'MinsBetweenVent'\n");
		}
		else {
			MinsBetweenVent = obj["MinsBetweenVent"];
		}

		if (!obj.containsKey("highTemp")) {
			Serial.printf("new settings does not contain 'highTemp'\n");
		}
		else {
			highTemp = obj["highTemp"];
		}
		if (!obj.containsKey("lowTemp")) {
			Serial.printf("new settings does not contain 'lowTemp'\n");
		}
		else {
			lowTemp = obj["lowTemp"];
		}
		if (!obj.containsKey("SSID")) {
			Serial.printf("new settings does not contain 'SSID'\n");
		}
		else {
			String s = obj["SSID"];
			sprintf(SSID, s.c_str());
		}
		if (!obj.containsKey("SSID_PASS")) {
			Serial.printf("new settings does not contain 'SSID_PASS'\n");
		}
		else {
			String s = obj["SSID_PASS"];
			sprintf(SSID_PASS, s.c_str());
		}
		if (!obj.containsKey("AP_MODE")) {
			Serial.printf("new settings does not contain 'AP_MODE'\n");
		}
		else {
			AP_MODE = obj["AP_MODE"];
		}

	}
};

ControllerSettings Settings;