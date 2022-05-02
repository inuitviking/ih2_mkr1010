// Includes
// - Libraries
#include <DHT.h>			// Library for the DHT11
#include <DHT_U.h>			// Same as above
// - Custom
#include "CustomDHT.h"

float CustomDHT::dhtGetTemperature(DHT_Unified dht) {
	// Define a sensor event
	sensors_event_t event;
	// Get temperature
	dht.temperature().getEvent(&event);

	// If temperature is nan, then print something to the serial port
	if (isnan(event.temperature)) {
		Serial.print("Something is wrong. Temperature is ");
		Serial.println(event.temperature);
		// return -300, as that goes below the kelvin minimum.
		return -300.0;
	} else {
		// else return the temperature
		return event.temperature;
	}
}

float CustomDHT::dhtGetHumidity(DHT_Unified dht) {
	// Define a sensor event
	sensors_event_t event;
	// Get humidity
	dht.humidity().getEvent(&event);

	// If humidity is nan, then print something to the serial port
	if(isnan(event.relative_humidity)) {
		Serial.print("Something is wrong. Humidity is ");
		Serial.println(event.relative_humidity);
		// Return -1 as humidity is either 0 or more.
		return -1.0;
	} else {
		// else return the humidity
		return event.relative_humidity;
	}
}