/* NetMQTT.h
 *
 * Handle MQTT's
 */

#ifndef NETMQTT_H 
#define NETMQTT_H 0.0100

#include <LFUtilities.h>	// Fake but needed to load LFUtilities
#include <LFUtilities/SafeMQTTClient.h>	// Network connection

class NetMQTT : public SafeMQTTClient {

public :
	NetMQTT( 
		Client &client,		// Network client
		const char *assid, const char *apwd,	// WiFi
		const char *abURL, int abport,	// MQTT Broker
		const char *aclientID,	// MQTT CLientID
		const char *atopic = NULL,	// root of topics to publish too (if empty, doesn't publish)
		bool clear_session = false
	) : SafeMQTTClient( client, assid, apwd, abURL, abport, aclientID, atopic, clear_session ) {}

	void begin( 
		const char *topic,	// Command topic to register to
		void (*func)( char*, byte* , unsigned int )
	){
		this->getClient().setCallback( func );
		this->getClient().subscribe(topic, 1);
	}

	void loop(){
		this->getClient().loop();
	}
};

#endif
