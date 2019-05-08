/* Parameters.h
 *
 * Contains all users customizable parameters
 */

#ifndef PARAMETERS_H
#define PARAMETERS_H

	/******
	 *	Comment or uncomment to activate or disable some optional parts.
	 ******/

#	define DEV_ONLY	/* Include developpment only code */
/*#define STATIC_IP*/	/* Use static IP when on home network */
/*#define LED_BUILTIN 2*/	/* Workaround for my ESP-12 */

	/* If enabled, LED is lighting during network establishment 
	 * NOTEZ-BIEN : on ESP-201 this is mutually exclusif vs serial output
	 */
#if 0		// Led is lightning during Wifi / Mqtt connection establishment
#	define LED(x)	{ digitalWrite(LED_BUILTIN, x); }
#else
#	define LED(x)	{ }
#	define SERIAL_ENABLED
#endif

#ifdef STATIC_IP
	/* Static IP to avoid DHCP querying delay */
IPAddress adr_ip(192, 168, 0, 17);
IPAddress adr_gateway(192, 168, 0, 10);
IPAddress adr_dns(192, 168, 0, 3);
#endif

	/* GPIOs */
#define AUXPWR_GPIO	0 /* Power auxiliaries, on prod 0 */
#define ONE_WIRE_BUS 2 // Where OW bus is connected to
#define pinDHT 5	// Where DHT is connected to


	/* MQTT */
#define MQTT_CLIENT "Poulailler"
String MQTT_Topic("Poulailler/");	// Topic's root
String MQTT_Error = MQTT_Topic + "Error";
String MQTT_Output = MQTT_Topic + "Message";
String MQTT_Command = MQTT_Topic + "Command";

	/* Delays */
#define DELAY_STARTUP	5		// Let a chance to enter in interactive mode at startup ( 5s )
#define DELAY_LIGHT 500			// Delay during light sleep (in ms - 0.5s )

#define INTERVAL_DEVICE 300		// Interval b/w samples for the device in S (5 minutes)
#define INTERVAL_PERCHOIR 300	// Interval b/w samples for the "perchoir" in S (5 minutes)

#define INTERVAL_AUX 900		// Interval b/w samples for "auxiliaires" in S (15 minutes)
#define DELAY_AUX 250			// Delay before light detector capacitor is loaded (in ms)

#define TIMEOUT_DOOR	5000	// Timeout for the door to reach stops in mS

	// Network related delays
	// Caution to respect delays if in interactive mode
#define FAILUREDELAY	900		// Delay b/w 2 network attempt in case of failure (15 minutes)
#define RETRYAFTERSWITCHING	12	// number of connections before trying the nominal network in case of degraded mode
#define RETRYAFTERFAILURE	3	// number of connections before trying the nominal network in case of faillure
#define MQTTRETRY	60			// Delay b/w MQTT retries (seconds)

#endif
