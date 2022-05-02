// Includes
// - Libraries
#include "WiFiNINA.h"        // Library to control the WiFi stuff
// - Custom
#include "CustomWifi.h"
#include "Serial/CustomSerial.h"

void CustomWifi::wifiStartup(char *ssid, char *pass, int status) {
	// Check for the WiFi module:
	if (WiFi.status() == WL_NO_MODULE) {
		Serial.println("Communication with WiFi module failed!");
		while (true);
	}
	// Check firmware version
	String fv = WiFiClass::firmwareVersion();
	if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
		Serial.println("Please upgrade the firmware");
	}

	// attempt to connect to WiFi network:
	while (status != WL_CONNECTED) {
		Serial.print("Attempting to connect to SSID: ");
		Serial.println(ssid);
		// Connect to WPA/WPA2 network. Change this line if using open or WEP network:
		status = WiFi.begin(ssid, pass);
		// wait 10 seconds for connection:
		for (int i = 0; i < 5; i++) {
			Serial.print(i);
			Serial.print(" ");
			delay(1000);
		}
		CustomSerial::clear();
	}
}

void CustomWifi::printWiFiStatus() {
	Serial.print("SSID: ");			// Print SSID (WiFi name)
	Serial.println(WiFi.SSID());
	Serial.print("IP Address: ");	// Print local IP address
	Serial.println(WiFi.localIP());
}
