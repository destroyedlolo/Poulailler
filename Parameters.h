/* Parameters.h
 *
 * Contains all users customizable parameters
 */

#ifndef PARAMETERS_H
#define PARAMETERS_H

	/******
	 *	Comment or uncomment to activate or disable some optional parts.
	 ******/

#define DEV	// Development mode

#ifndef DEV

	/****
	 * Production settings
	 ****/

#	define MQTT_CLIENT "Poulailler"

	/* LED brights during network and MQTT acquisition
	 */
#	define LED(x)	{ digitalWrite(LED_BUILTIN, x); }
#	define DEF_NESTSLEEP 300	// Default sleep b/w nest sample (in seconds)
#	define DEF_WAKED	240			// Default waked time in interactive mode (seconds)
#	define DEF_VERBOSITY false	// low level of information send (by default, no to save some energies)

#else

	/****
	 * Development settings
	 ****/

#	define MQTT_CLIENT "Poulailler-Dev"

	/* Debug message on tx
	 * Notez-bien : LED disabled as on the same GPIO
	 */
#	define LED(x)	{ }
#	define SERIAL_ENABLED

#	define DEF_NESTSLEEP 90		// Sleep b/w nest sample (short as we're debugging)
#	define DEF_WAKED	240
#	define DEF_VERBOSITY true	// Hight level of information send
#endif

	/* MQTT */
String MQTT_Topic(MQTT_CLIENT);	// Topic's root
String MQTT_Error = MQTT_Topic + "/Error";
String MQTT_Command = MQTT_Topic + "/Command";

	/* GPIOs */
#define AUXPWR_GPIO	0 /* Power auxiliaries, on prod 0 */
#define ONE_WIRE_BUS 2 // Where OW bus is connected to
#define pinDHT 5	// Where DHT is connected to

#endif
