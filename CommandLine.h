/* Command line handling
 *
 * 28/10/2017 First version
 */

class CommandLine {
	bool active;	// Are we in command line mode ?

public:
	CommandLine() : active(false) {};

	void prompt() { Serial.println("ok >"); };

	void enter() { active = true; prompt(); };
	void finished() { active = false; };
	bool isactive() { return active; };

	void loop();	// implemented in the main file
};
