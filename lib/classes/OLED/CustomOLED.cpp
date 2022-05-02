// Includes
// - Libraries
#include <Adafruit_SSD1306.h>	// Library for the specific OLED
// - Custom
#include "CustomOLED.h"

void CustomOLED::clear() {
	display.clearDisplay();
	display.setTextSize(1); // Normal 1:1 pixel scale
	display.setTextColor(WHITE); // Draw white text
	display.setCursor(0,0); // Start at top-left corner
	display.display();
}

void CustomOLED::println(String text) {
	display.println(text.c_str());
	display.display();
}