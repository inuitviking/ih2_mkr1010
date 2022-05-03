// Includes
// - Libraries
#include <Adafruit_SSD1306.h>	// Library for the specific OLED

#ifndef IH2_MKR1010_CUSTOMOLED_H
#define IH2_MKR1010_CUSTOMOLED_H


class CustomOLED: Adafruit_SSD1306 {

public:
	void clear();
	void println(String text);

	Adafruit_SSD1306 display;
};


#endif //IH2_MKR1010_CUSTOMOLED_H
