//
// Created by angus on 5/2/22.
//

#ifndef IH2_MKR1010_CUSTOMWIFI_H
#define IH2_MKR1010_CUSTOMWIFI_H

class CustomWifi {
public:
	static void wifiStartup(char *ssid, char *pass, int status);
	static void printWiFiStatus();
};

#endif //IH2_MKR1010_CUSTOMWIFI_H
