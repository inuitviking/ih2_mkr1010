// Includes
// - Libraries
#include <Servo.h>
// - Custom
#include "CustomServo.h"

int CustomServo::open(Servo servo, int angle) {
	if (angle != 180) {
		for(angle = 0; angle < 180; angle++) {
			servo.write(angle);
		}
	}
}

int CustomServo::close(Servo servo, int angle) {
	if (angle != 0) {
		for(angle = 180; angle > 0; angle--) {
			servo.write(angle);
		}
	}
}

