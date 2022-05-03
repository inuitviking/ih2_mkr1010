//
// Created by angus on 5/3/22.
//

#ifndef IH2_MKR1010_CUSTOMSERVO_H
#define IH2_MKR1010_CUSTOMSERVO_H


class CustomServo {
public:
	static int open(Servo servo, int angle);
	static int close(Servo servo, int angle);
};


#endif //IH2_MKR1010_CUSTOMSERVO_H
