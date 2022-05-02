#include <Arduino.h>
#ifndef IH2_MKR1010_CUSTOMSERIAL_H
#define IH2_MKR1010_CUSTOMSERIAL_H


class CustomSerial: public HardwareSerial {
public:
	static void clear();
};

#endif //IH2_MKR1010_CUSTOMSERIAL_H
