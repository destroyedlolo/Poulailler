/* Handle gate
 *
 * 06/12/2017 First version
 *
 * GPIOs are put inactive during DeepSleep.
 * So the motor has to be commanded again if 'command' is not NONE
 * at startup : it's probably due to a crash while the gate was moving.
 */

#ifndef PORTE_H
#define PORTE_H

	/*****
	 * Notez-bien : All fields will be persisted in RTC memory
	 *****/
class Porte {
	enum Command {	// What is requested to the gate
		NONE,	// Stay at it is
		STOP,	// Stop the door
		OPEN,
		CLOSE
	} command;

	enum GPIO {
		DOWN = 12,
		UP = 13,
		END = 15
	};

protected:
	virtual void save( void ) = 0;	// Needed to call context's one

	void init( void ){	/* Initial configuration */
		this->command = Command::STOP;
	}

public:
	static void setup( void ){
		pinMode(GPIO::DOWN, OUTPUT);
		pinMode(GPIO::UP, OUTPUT);
		pinMode(GPIO::END, INPUT);
	}

	bool action( enum Command movement = Command::NONE ){	// move the door
		if( movement != Command::NONE ){
			this->command = movement;
			this->save();
		}
	
		digitalWrite( GPIO::DOWN, 0);
		digitalWrite( GPIO::UP, 0);

		switch( this->command ){
		case Command::OPEN :
			digitalWrite( GPIO::UP, 1);
			return true;
		case Command::CLOSE :
			digitalWrite( GPIO::DOWN, 1);
			return true;
		default:
			return false;	// the door is not in movement
		}
	}

	bool isFinished( void ){	// is the door arrived to stop position
		return digitalRead( GPIO::END );
	}
};
#endif

