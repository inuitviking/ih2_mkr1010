// Includes
// - Libraries
#include <Arduino.h>		// The main arduino library
// - Custom
#include "CustomSerial.h"

void CustomSerial::clear() {
	Serial.write(27);		// ESC command
	Serial.print("[2J");	// clear screen command
	Serial.write(27);		// ESC command
	Serial.print("[H");		// cursor to home command
}
