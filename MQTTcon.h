/* MQTT handling 
 *
 * 16/12/2017 First version
 */

#ifndef MQTTCON_H
#define MQTTCON_H

#include <PubSubClient.h>

WiFiClient clientWiFi;
PubSubClient clientMQTT(clientWiFi);

	/*****
	 * Notez-bien : All fields will be persisted in RTC memory
	 *****/
class MQTTcon {
protected:
	void init( void ){} /* Initial configuration */

	void setup( void ){	/* Configuration */
		clientMQTT.setServer(BROKER_HOST, BROKER_PORT);
	}
};
#endif
