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

#include "Context.h"

class Porte : public Context::keepInRTC {
public:
	enum Command {	// What is requested to the gate
		NONE,	// Stay at it is
		ERROR,	// Door can't move
		STOP,	// Stop the door
		OPEN,
		CLOSE
	};

private:
	enum GPIO {
		DOWN = 12,
		UP = 13,
		END = 15
	};

	struct { 
		enum Command command;
		unsigned long timeout;
		unsigned long startmvt;	// Start of the current movement
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
			this->data.timeout = TIMEOUT_DOOR;
			this->save();
		}
	}

	bool action( enum Command movement = Command::NONE ){
	/* Launch or stop motor movement
	 * -> Command::NONE : restore the last movement
	 * <- is the door moving ?
	 */
		if( this->data.command == Command::ERROR )	// Already in error
			return false;

		if( movement != Command::NONE ){
			this->data.command = movement;
			this->save();
		}
	
			// Remove potential previous request
		digitalWrite( GPIO::DOWN, 0);
		digitalWrite( GPIO::UP, 0);

		switch( this->data.command ){
		case Command::OPEN :
			context.Output("Ouverture porte");
			digitalWrite( GPIO::UP, 1);
			this->data.startmvt = context.getTime();
			this->save();
			return true;
		case Command::CLOSE :
			context.Output("Fermeture porte");
			digitalWrite( GPIO::DOWN, 1);
			this->data.startmvt = context.getTime();
			this->save();
			return true;
		case Command::ERROR :
			context.Output("Timeout sur le mouvement de la porte : PROBLEME !!!");
		default:
			return false;	// the door is not in movement
		}
	}

	void clearErrorCondition( void ){
		this->data.command = Command::NONE;	// Reset error condition
		this->action( Command::STOP );		// Force GPIO to a known condition
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
		else if( this->isMoving() ){
			if( context.getTime() > ( this->data.startmvt + this->data.timeout ) )	// Movment too long
				return this->action( Command::ERROR );
			else
				return true;	// movement on way
		} else
			return false;	// no movment on way
	}

	bool inError( void ){ return(this->data.command == Command::ERROR); }

	unsigned long getTimeout( void ){ return this->data.timeout; }
	void setTimeout( signed long v ){
		this->data.timeout = v;
		this->save();
	}
};
#endif

