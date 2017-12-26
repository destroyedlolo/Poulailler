/* Handle gate
 *
 * 06/12/2017 First version
 * 26/12/2017 Suit new app design
 *
 * GPIOs are put inactive during DeepSleep.
 * So the motor has to be commanded again if 'command' is not NONE
 * at startup : it's probably due to a crash while the gate was moving.
 */

#ifndef PORTE_H
#define PORTE_H

class Porte : public Context::keepInRTC {
	enum Command {	// What is requested to the gate
		NONE,	// Stay at it is
		STOP,	// Stop the door
		OPEN,
		CLOSE
	};

	enum GPIO {
		DOWN = 12,
		UP = 13,
		END = 15
	};

	struct { 
		enum Command command;
	} data;	// data to be kept

public:
	/*
	 * Hardware setup
	 */
	static void setup( void ){
		pinMode(GPIO::DOWN, OUTPUT);
		pinMode(GPIO::UP, OUTPUT);
		pinMode(GPIO::END, INPUT);
	}

	Porte( Context &ctx ) : Context::keepInRTC( ctx, (uint32_t *)&data, sizeof(data) ) {
		if( !ctx.isValid() ){	// Default value
			this->data.command = Command::STOP;
			this->save();
		}
	}

	bool action( enum Command movement = Command::NONE ){
	/* Launch or stop motor movement
	 * -> Command::NONE : restore the last movement
	 * <- is the door moving ?
	 */
		if( movement != Command::NONE ){
			this->data.command = movement;
			this->save();
		}
	
			// Remove potential previous request
		digitalWrite( GPIO::DOWN, 0);
		digitalWrite( GPIO::UP, 0);

		switch( this->data.command ){
		case Command::OPEN :
#			ifdef SERIAL_ENABLED
			Serial.println("Door : opening");
#			endif
			digitalWrite( GPIO::UP, 1);
			return true;
		case Command::CLOSE :
#			ifdef SERIAL_ENABLED
			Serial.println("Door : closing");
#			endif
			digitalWrite( GPIO::DOWN, 1);
			return true;
		default:
			return false;	// the door is not in movement
		}
	}

	bool isMoving( void ){
		switch( this->data.command ){
		case Command::OPEN :
		case Command::CLOSE :
			return true;
		default:
			return false;
		}
	}

	bool isStillMoving( void ){
	/* check if the door finished its movement.
	 * Notez-bien : thanks to electronic behind, GPIO::END is 'hight'
	 * 	only if the motor is in movement as linked to UP and DOWN GPIOs
	 * <-  
	 */
		if(digitalRead( GPIO::END ))
			return this->action( Command::STOP );
		else
			return this->isMoving();
	}
};
#endif

