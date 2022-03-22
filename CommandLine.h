/* Command line handling
 *
 * 28/10/2017 First version
 * 21/03/2022 redesign for v2
 */
#ifndef COMMANDLINE_H
#define COMMANDLINE_H	0.000

#include <string>
#include "NetMQTT.h"

class CommandLine {
	bool active;	// Are we in command line mode ?

public:
	CommandLine() : active(false) {};

	void prompt() {
#		ifdef SERIAL_ENABLED
		Serial.println("ok >"); 
#		endif
	};

	void enter() { active = true; nMQTT.logMsg("Waiting for commands ..."); prompt(); };
	void finished() { active = false; nMQTT.logMsg("Automatic mode"); };
	bool isActive() { return active; };

	void readSerial( void ){
		String cmd = Serial.readString();
		this->exec( cmd );
	}

	void exec( String & );	// implemented in the main file
};
#endif
