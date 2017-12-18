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

	void connect( void ){
#ifdef SERIAL_ENABLED
		Serial.println("Connecting to MQTT");
#endif

		while(!clientMQTT.connected()){
			if(clientMQTT.connect(MQTT_CLIENT)){
#ifdef SERIAL_ENABLED
				Serial.println("connected");
#endif
				break;
			} else {
#ifdef SERIAL_ENABLED
				Serial.print("Failure, rc:");
				Serial.println(clientMQTT.state());
#endif
				delay(1000);	// Test dans 1 seconde
			}
		}
	}

public:
	void publish( const char *topic, const char *msg ){
		if(!clientMQTT.connected())
			this->connect();
		clientMQTT.publish( topic, msg );
	}

	void publish( String &topic, String &msg ){
		this->publish( topic.c_str(), msg.c_str() );
	}

	void publish( String &topic, const char *msg ){
		this->publish( topic.c_str(), msg );
	}

};
#endif