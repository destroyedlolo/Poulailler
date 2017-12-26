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

	void prompt() { Serial.println("ok >"); };

	void enter() { active = true; prompt(); };
	void finished() { active = false; };
	bool isActive() { return active; };

	void loop();	// implemented in the main file
};
#endif
