// Includes
// - Libraries
#include <Arduino.h>			// The main arduino library
#include <SPI.h>				// For reading the RFID
#include <MFRC522.h>			// For reading the RFID
#include <Servo.h>				// Library to control a servo
#include <WiFiNINA.h>			// Library to control the WiFi stuff
#include <DHT.h>				// Library for the DHT11
#include <DHT_U.h>				// Same as above
//#include <Adafruit_GFX.h>		// Graphics for the OLED
#include <Adafruit_SSD1306.h>	// Library for the specific OLED
#include <Wire.h>				// I2C stuff
#include <DS3231.h>				// Clock
//#include <Servo.h>				// Servo
#include <ThingSpeak.h>			// Library to speak to ThingSpeak
// - Custom
#include "Wifi/CustomWifi.h"	// Header file for a custom class to take care of the Wifi (uses WiFiNINA).
#include "DHT/CustomDHT.h"		// Header file for a custom class to take care of DHT.
#include "OLED/CustomOLED.h"	// Header file for a custom class to take care of OLED.
#include "Serial/CustomSerial.h"// Header file for a custom class to take care of Serial.
#include "Servo/CustomServo.h"	// Header file for a custom class to take care of Servo.
#include "secrets/mqtt.h"		// Secret header file containing MQTT credentials
#include "secrets/wifi.h"		// Secret header file containing WiFi credentials
#include "definitions.h"		// Header file containing various definitions
#include "pins.h"				// Header file containing pins

// Variables
// - WiFi
int keyIndex = 0;							// your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;				// The idle status of the WiFi
WiFiClient wifiClient;						// The WiFi client
// - DHT
DHT_Unified dht(DHTPIN, DHTTYPE);	// The DHT_unified object; this contains e.g. both humidity and temperature
uint32_t delayMS;							// Set the delay for the sensor
// - Millis
unsigned long dhtMillis = 0;
unsigned long lastMillis = 0;				// Last millis
const long dhtInterval = 20000;				// Millis interval to use. 20s for ThingSpeak.
const long interval = 5000;				// Millis interval to use. 20s for ThingSpeak.
// - OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
CustomOLED oled;
// - Clock
DS3231 aClock;
// - RFID
MFRC522 mfrc522(A1, RST_PIN);	// Create MFRC522 instance
byte accCards[][4] = {											// Accepted cards
		{0x40, 0x98, 0x6D, 0xA5},
		{0xE2, 0xFB, 0x50, 0x73}
};
// - Servo
Servo servo;
int servoAngle = 0;

/*
 **** Actual code ****
 * Where the actual fun happens
 */
/**
 * The function to setup your board.
 */
void setup() {
	// Initialisation
	Serial.begin(9600);															// Start serial
	Wire.begin();																			// Start I2C
	if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {						// Address for OLED, and start (SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally)
		Serial.println(F("SSD1306 allocation failed"));
		for(;;);																			// Don't proceed, loop forever
	}

	oled.display = display;																	// Set the display to the started OLED
	while (!Serial) {;}																		// Wait for a serial port before continuing.

	SPI.begin();																			// Init SPI bus
	mfrc522.PCD_Init();																		// Init MFRC522 card

	dht.begin();																			// Start the DHT11
	sensor_t sensor;																		// Define a sensor_t object
	delayMS = sensor.min_delay / 1000;														// Set the delay of that sensor

	oled.clear();																			// Clear OLED
	String wifiStartupText = "Connecting to \n";											// Tell the user what we're doing
	wifiStartupText += SECRET_SSID;
	oled.println(wifiStartupText);

	CustomWifi::wifiStartup((char *) SECRET_SSID, (char *) SECRET_PASS, status);	// Connect to WiFi
	CustomWifi::printWiFiStatus();															// Print Wifi Status (Just whihc SSID and the local IP)
	ThingSpeak.begin(wifiClient);														// Being talking to ThingSpeak

	oled.clear();																			// Clear the display
	oled.println("Scan RFID");															// Tell the user what to do

	// Construct time string
	bool h12Flag;
	bool pmFlag;
	String time = String(aClock.getHour(h12Flag, pmFlag), DEC);
	time += ":";
	time += String(aClock.getMinute(), DEC);
	oled.display.setCursor(95,0);
	oled.display.print(time);
	oled.display.display();

	servo.attach(SERVOPIN);																// Attach servo
	servoAngle = CustomServo::close(servo, servoAngle);
}

/**
 * The loop function contains all the code that needs to be looped through.
 */
void loop() {

	if (millis() - dhtMillis > dhtInterval) {
		oled.clear();
		dhtMillis = millis();
		servoAngle = CustomServo::close(servo, servoAngle);
		oled.println("Scan RFID");
		// Construct time string
		bool h12Flag;
		bool pmFlag;
		String time = String(aClock.getHour(h12Flag, pmFlag), DEC);
		time += ":";
		time += String(aClock.getMinute(), DEC);
		oled.display.setCursor(95, 0);
		oled.display.print(time);
		oled.display.display();

		// Print temperature and humidity
		float temperature = CustomDHT::dhtGetTemperature(dht);
		float humidity = CustomDHT::dhtGetHumidity(dht);

		// Check if temperature or humidity is wrong
		if (temperature == -300.0 || humidity == -1) {
			Serial.println("Check sensors!");
		} else {
			// Else just send.
			Serial.print("Temperature: ");
			Serial.print(String(temperature).c_str());
			Serial.println("°C");
			Serial.print("Relative Humidity: ");
			Serial.print(String(humidity).c_str());
			Serial.println("%");

			// Put values in their respective fields
			ThingSpeak.setField(1, temperature);
			ThingSpeak.setField(2, humidity);
			// Write to both fields to the service, and get the HTTP status code while we're at it.
			int statusCode = ThingSpeak.writeFields((unsigned long) MQTT_CH_ID, (char *) MQTT_WRITE_API_KEY);
			// If the status code is 200, all is good; else it isn't.
			if (statusCode == 200) return;
			Serial.println("Problem updating channel. HTTP error code " + String(statusCode));
		}
	}

	if (millis() - lastMillis >= interval) {
		oled.clear();
		lastMillis = millis();
		servoAngle = CustomServo::close(servo, servoAngle);
		oled.println("Scan RFID");
		// Construct time string
		bool h12Flag;
		bool pmFlag;
		String time = String(aClock.getHour(h12Flag, pmFlag), DEC);
		time += ":";
		time += String(aClock.getMinute(), DEC);
		oled.display.setCursor(95, 0);
		oled.display.print(time);
		oled.display.display();
	}

	// Reconnect Wifi if it dies.
	if (WiFi.status() != WL_CONNECTED) {
		Serial.print("Lost WiFi connection. Attempting to reconnect to ");
		Serial.println((char *) SECRET_SSID);
		CustomWifi::wifiStartup((char *) SECRET_SSID, (char *) SECRET_PASS, status);
	}

	// Look for new cards, and select one if present
	if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
		return;
	}

	int len = (sizeof(accCards) / sizeof(accCards[0]));																	// Get size of array
	for(int i = 0; i < len; i++) {																						// Loop through card array
		if (memcmp(accCards[0], mfrc522.uid.uidByte, len) == 0 || memcmp(accCards[1], mfrc522.uid.uidByte, len) == 0) {	// If card is in array
			lastMillis = millis();																						// Set previous millis
			oled.clear();																								// Clear OLED

			oled.println("Card authorized. Welcome.");																// Print authorized text

			servoAngle = CustomServo::open(servo, servoAngle);													// Open "door"
		} else {													// If card is NOT in array
			lastMillis = millis();																						// Set previous millis
			oled.clear();																								// Clear OLED

			servoAngle = CustomServo::close(servo, servoAngle);													// Close "Door"
			oled.println("CARD NOT AUTHORIZED.");																	// Print authorized text
			// Draw box with cross
			display.drawRect(45, 12, 40, 40, WHITE);
			display.drawLine(45,12,83,50, WHITE);
			display.drawLine(45, 50, 83, 12, WHITE);
			// Display box.
			display.display();
		}
	}
}
