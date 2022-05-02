//
// Created by angus on 5/2/22.
//

#ifndef IH2_MKR1010_CUSTOMDHT_H
#define IH2_MKR1010_CUSTOMDHT_H


class CustomDHT {
public:
	static float dhtGetTemperature (DHT_Unified dht);
	static float dhtGetHumidity (DHT_Unified dht);
};


#endif //IH2_MKR1010_CUSTOMDHT_H
