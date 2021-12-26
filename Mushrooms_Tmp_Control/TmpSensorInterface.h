#pragma once

#include <OneWire.h>			//    oneWire by Jim Studt, ..
#include <DallasTemperature.h>	//    DallasTemperature

#include "Status.h"

class TempratureSensorModule {
private:
	int pin;
	OneWire* DS18B20oneWire;
	DallasTemperature* TempProbe;
	uint8_t address = 77;
	bool addressFound = false;

public:
	//bool valid;
	//float tmp;
	void setTemp(bool validP, float tmpP) {
		//valid = validP; tmp = tmpP;
		Status.SensorValid = validP;
		Status.tmp = tmpP;
	}

	void init() {
		//if (TempProbe != NULL) delete(TempProbe);
		//if (DS18B20oneWire != NULL) delete(DS18B20oneWire);
		DS18B20oneWire = new OneWire(pin);// Setup a oneWire instance to communicate with any OneWire devices
		TempProbe = new DallasTemperature(DS18B20oneWire);// Pass our oneWire reference to Dallas Temperature sensor 
		addressFound = TempProbe->getAddress(&address, 0);
		
		TempProbe->begin();// Start the DS18B20 sensor
		if (addressFound) {
			Serial.println("Temprature Gauge is conneted");
			Serial.print("Temprature sensor address = ");
			Serial.println(address);
		}
		else {
			Serial.println("Temprature Gauge is NOT conneted");
		}

	}

	TempratureSensorModule(int pin) {
		this->pin = pin;
		DS18B20oneWire = NULL;// Setup a oneWire instance to communicate with any OneWire devices
		TempProbe = NULL;// Pass our oneWire reference to Dallas Temperature sensor 
		init();
	}


	virtual void updateRead() {
		if (!addressFound) addressFound = TempProbe->getAddress(&address, 0);
		//addressFound = true;
		if (addressFound) {
			TempProbe->requestTemperatures();
			float temperatureC = TempProbe->getTempCByIndex(0);
			//bool connected = TempProbe->isConnected(&address);
			//Serial.println(connected ? "Tmp sensor connected" : "Tmp sensor NOT connected");
			if (temperatureC < -10) {
				setTemp(false, temperatureC);
			}
			else {
				setTemp(true, temperatureC);
			}
		}
		else {
			Serial.println("Address not found");
			setTemp(false, -10);
		}
	}

};