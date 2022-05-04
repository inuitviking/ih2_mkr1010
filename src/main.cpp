// Includes
// - Libraries
//#include <Arduino.h>			// The main arduino library
#include <ArduinoBLE.h>			// Arduino Bluetooth LE library
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
#include <Encoder.h>			// Encoder stuff
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
// - Bluetooth
BLEService newService("180A");															// creating the service
BLEUnsignedCharCharacteristic randomReading("2A58", BLERead | BLENotify);	// creating the Analog Value characteristic
BLEByteCharacteristic switchChar("2A57", BLERead | BLEWrite);				// creating the LED characteristic
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
bool h12Flag;
bool pmFlag;
// - RFID
MFRC522 mfrc522(SS_PIN, RST_PIN);	// Create MFRC522 instance
byte accCards[][4] = {											// Accepted cards
		{0x40, 0x98, 0x6D, 0xA5},
		{0xE2, 0xFB, 0x50, 0x73}
};
// - Authentication
boolean authenticated = false;
// - Servo
Servo servo;
int servoAngle = 0;
// - Encoder
Encoder encoder(ENCODERPINA, ENCODERPINB);	// Configure the encoder
long oldPosition = -999;								// Just the old position.
int btnLastState = HIGH;								// The previous state from the input pin
int btnCurrentState;									// The current reading from the input pin

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
	String time = String(aClock.getHour(h12Flag, pmFlag), DEC);					// Construct time string
	time += ":";
	time += String(aClock.getMinute(), DEC);
	oled.display.setCursor(95,0);														// Set cursor position
	oled.display.print(time);																// Print time
	oled.display.display();																	// Display

	/**
	 * This does not work in parallel, you have to disable WiFi to activate BLE, ad vice versa.
	 * In other words, I need to do some channel hopping, so turn off WiFi when I want to use BLE,
	 * and turn off BLE when I want to use WiFi. This is not ideal in my use case, but could be
	 * used in situations, such as if you don't know what kind of connectivity you use on a
	 * specific field.
	 * See following links for ideas.
	 * General discussion:					https://forum.arduino.cc/t/bluetooth-and-wifi-in-parallel/878198/5
	 * Code that uses both WiFi and BLE:	https://forum.arduino.cc/t/switching-between-wifi-and-ble/629966/7
	 * About the NINA-W10 series:			https://www.u-blox.com/en/product/nina-w10-series-open-cpu
	 */
//	if (!BLE.begin()) {																		// Initialise BLE
//		Serial.println("starting BLE failed!");												// Loop forever if it's failing
//		while (1);
//	}
//	BLE.setLocalName("MKR WiFi 1010");														// Setting a name that will appear when scanning for bluetooth devices
//	BLE.setAdvertisedService(newService);
//	newService.addCharacteristic(switchChar);											// Add characteristics to a service
//	newService.addCharacteristic(randomReading);
//	BLE.addService(newService);															// adding the service
//	switchChar.writeValue(0);															//set initial value for characteristics
//	randomReading.writeValue(0);
//	BLE.advertise();																		//start advertising the service
//	Serial.println("Bluetooth device active, waiting for connections...");

	servo.attach(SERVOPIN);																// Attach servo
	servoAngle = CustomServo::close(servo, servoAngle);								// Close it if it is open

	pinMode(ENCODERBTN, INPUT_PULLUP);									// Button from encoder
}

/**
 * The loop function contains all the code that needs to be looped through.
 */
void loop() {
	/********************************
	************** MQTT *************
	********************************/

	int statusCode = 0;
	String door = ThingSpeak.readStringField(MQTT_CH_ID, 3, MQTT_READ_API_KEY);

	// Check the status of the read operation to see if it was successful
	statusCode = ThingSpeak.getLastReadStatus();
	if(statusCode == 200){
		if (door == "1" && !authenticated) {
			servoAngle = CustomServo::open(servo, servoAngle);
			Serial.println("Door Open");
		} else if (door == "0" && !authenticated) {
			servoAngle = CustomServo::close(servo, servoAngle);
			Serial.println("Door Closed");
		}
	}else{
		Serial.println("Problem reading channel. HTTP error code " + String(statusCode));
	}

	/********************************
	********** Long Millis **********
	********************************/
	if (millis() - dhtMillis > dhtInterval && !authenticated) {
		oled.clear();																													// Clear OLED
		dhtMillis = millis();																											// Set dhtMillis
		servoAngle = CustomServo::close(servo, servoAngle);																		// Close the servo
		oled.println("Scan RFID");																									// Tell the user what to do
		String time = String(aClock.getHour(h12Flag, pmFlag), DEC);															// Construct time string
		time += ":";
		time += String(aClock.getMinute(), DEC);
		oled.display.setCursor(95, 0);																							// Set the cursor to where we want to print the clock.
		oled.display.print(time);																										// Print the time.
		oled.display.display();																											// Put it on the display.

		float temperature = CustomDHT::dhtGetTemperature(dht);																			// Get the temperature.
		float humidity = CustomDHT::dhtGetHumidity(dht);																				// Get the humidity.

		if (temperature == -300.0 || humidity == -1) {																					// Check if temperature or humidity is wrong.
			Serial.println("Check sensors!");																							// Tell the user via terminal what to do.
		} else {																														// Else just send.
			Serial.print("Temperature: ");
			Serial.print(String(temperature).c_str());
			Serial.println("°C");
			Serial.print("Relative Humidity: ");
			Serial.print(String(humidity).c_str());
			Serial.println("%");

			ThingSpeak.setField(1, temperature);																				// Put temperature in field 1.
			ThingSpeak.setField(2, humidity);																				// Put humidity in field 2.
			int statusCode = ThingSpeak.writeFields((unsigned long) MQTT_CH_ID, (char *) MQTT_WRITE_API_KEY);	// Write to both fields to the service, and get the HTTP status code
			if (statusCode == 200) return;																								// If the status code is 200, all is good.
			Serial.println("Problem updating channel. HTTP error code " + String(statusCode));										// If not, print to serial that it isn't.
		}
	}

	/********************************
	********* Short Millis **********
	********************************/
	if (millis() - lastMillis >= interval) {
		if (!authenticated){
			oled.clear();																// Clear OLED.
			lastMillis = millis();														// Set lastMillis.
			servoAngle = CustomServo::close(servo, servoAngle);					// Close the servo.
			oled.println("Scan RFID");												// Tell the user what to do.
			String time = String(aClock.getHour(h12Flag, pmFlag), DEC);		// Construct clock string.
			time += ":";
			time += String(aClock.getMinute(), DEC);
			oled.display.setCursor(95, 0);										// Set the cursor to where we want to print the clock.
			oled.display.print(time);													// Print the time.
			oled.display.display();														// Put it on the display.
		} else {
			long newPosition = encoder.read();
			oled.clear();
			if (newPosition == 0) {
				oldPosition = newPosition;
				oled.display.setTextColor(BLACK, WHITE);
				oled.display.println("Show temperature");
				oled.display.setTextColor(WHITE, BLACK);
				oled.display.println("Show humidity.");
				oled.display.setTextColor(WHITE, BLACK);
				oled.display.println("Lock and disengage");
				oled.display.display();
				oled.println(String(newPosition));
			} else if (newPosition == 4) {
				oldPosition = newPosition;
				oled.display.setTextColor(WHITE, BLACK);
				oled.display.println("Show temperature");
				oled.display.setTextColor(BLACK, WHITE);
				oled.display.println("Show humidity.");
				oled.display.setTextColor(WHITE, BLACK);
				oled.display.println("Lock and disengage");
				oled.display.display();
				oled.println(String(newPosition));
			} else if (newPosition == 8) {
				oldPosition = newPosition;
				oled.display.setTextColor(WHITE, BLACK);
				oled.display.println("Show temperature");
				oled.display.setTextColor(WHITE, BLACK);
				oled.display.println("Show humidity.");
				oled.display.setTextColor(BLACK, WHITE);
				oled.display.println("Lock and disengage");
				oled.display.display();
				oled.println(String(newPosition));
			} else {
				oldPosition = newPosition;
				oled.display.setTextColor(BLACK, WHITE);
				oled.display.println("Show temperature");
				oled.display.setTextColor(WHITE, BLACK);
				oled.display.println("Show humidity.");
				oled.display.setTextColor(WHITE, BLACK);
				oled.display.println("Lock and disengage");
				oled.display.display();
				oled.println(String(newPosition));
			}

			btnCurrentState = digitalRead(ENCODERBTN);
			if (btnLastState == LOW && btnCurrentState == HIGH) {
				oled.clear();
				if (newPosition == 0) {
					String temperatureString = "Temperature: ";
					temperatureString += CustomDHT::dhtGetTemperature(dht);
					temperatureString += (char)247;
					temperatureString += "C";
					oled.println(temperatureString);
					delay(2000);
				} else if (newPosition == 4) {
					String humidityString = "Humidity: ";
					humidityString += CustomDHT::dhtGetHumidity(dht);
					humidityString += "%";
					oled.println(humidityString);
					delay(2000);
				} else if (newPosition == 8) {
					servoAngle = CustomServo::close(servo, servoAngle);
					authenticated = false;
				}
			}


			// save the last state
			btnLastState = btnCurrentState;
		}
	}

	/********************************
	******* WiFi Reconnection *******
	********************************/
	if (WiFi.status() != WL_CONNECTED) {														// If we're no longer connected
		Serial.print("Lost WiFi connection. Attempting to reconnect to ");						// Say that we are no longer connected (in serial) and that we're going to try again
		Serial.println((char *) SECRET_SSID);
		CustomWifi::wifiStartup((char *) SECRET_SSID, (char *) SECRET_PASS, status);	// Attempt reconnection of WiFi
	}

	/********************************
	*********** Bluetooth ***********
	********************************/
	//	BLEDevice central = BLE.central();																									// Wait for a BLE central
//	if (central) {																														// If a central is connected to the peripheral
//		Serial.print("Connected to central: ");
//
//		Serial.println(central.address());																							// print the central's BT address
//
//		digitalWrite(LED_BUILTIN, HIGH);																				// turn on the LED to indicate the connection
//
//		// Check the battery level every 200ms
//		// While the central is connected:
//		while (central.connected()) {
//			unsigned long currentMillis = millis();
//
//			if (currentMillis - lastMillis >= interval) {																				// If 200ms have passed, we check the battery level
//				lastMillis = currentMillis;
//
//				int randomValue = analogRead(A2);
//				randomReading.writeValue(randomValue);
//
//				if (switchChar.written()) {
//					if (switchChar.value()) {																							// any value other than 0
//						servoAngle = CustomServo::open(servo, servoAngle);
//					} else {																											// a 0 value
//						servoAngle = CustomServo::close(servo, servoAngle);
//					}
//				}
//
//			}
//		}
//
//		digitalWrite(LED_BUILTIN, LOW);																					// When the central disconnects, turn off the LED
//		Serial.print("Disconnected from central: ");
//		Serial.println(central.address());
//	}

	/********************************
	**** AUTHENTICATION OF CARDS ****
	********************************/
	if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {											// Look for new cards, and select one if present
		return;
	}

	int len = (sizeof(accCards) / sizeof(accCards[0]));																	// Get size of array
	for(int i = 0; i < len; i++) {																						// Loop through card array
		if (memcmp(accCards[0], mfrc522.uid.uidByte, len) == 0 || memcmp(accCards[1], mfrc522.uid.uidByte, len) == 0) {	// If card is in array
			lastMillis = millis();																						// Set previous millis
			authenticated = true;
			oled.clear();																								// Clear OLED
			oled.println("Card authorized. Welcome.");																// Print authorized text
			servoAngle = CustomServo::open(servo, servoAngle);													// Open "door"
		} else {																										// If card is NOT in array
			lastMillis = millis();																						// Set previous millis
			oled.clear();																								// Clear OLED
			servoAngle = CustomServo::close(servo, servoAngle);													// Close "Door"
			oled.println("CARD NOT AUTHORIZED.");																	// Print authorized text
			display.drawRect(45, 12, 40, 40, WHITE);													// Draw box with cross
			display.drawLine(45,12,83,50, WHITE);
			display.drawLine(45, 50, 83, 12, WHITE);
			display.display();																							// Display box.
		}
	}
}
