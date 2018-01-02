/* Command line handling
 *
 * 28/10/2017 First version
 */
#ifndef COMMANDLINE_H
#define COMMANDLINE_H	0.0100

class CommandLine {
	bool active;	// Are we in command line mode ?

public:
	CommandLine() : active(false) {};

	void prompt() {
#		ifdef SERIAL_ENABLED
		Serial.println("ok >"); 
#		endif
	};

	void enter() { active = true; prompt(); context.Output("Waiting for commands ..."); };
	void finished() { active = false; context.Output("Automatic mode"); };
	bool isActive() { return active; };

	void loop();	// implemented in the main file
};
#endif
