/* Repeating class
 *
 * 23/12/2017 First version
 */
#ifndef REPEATER_H
#define REPEATER_H

class Repeater : public Context::keepInRTC {
	unsigned long int next;	// next run
	unsigned long int interval;

public:
	virtual void action( void ) = 0;	// Which action to launch

	Repeater( Context &ctx, unsigned long int aint, bool now=false) : Context::keepInRTC( ctx, (uint32_t *)&next, sizeof(next) ), interval( aint ) {
	/* <- aint : interval b/w launches (ms)
	 * 	now : if the action has to be launched now
	 */
		if(!ctx.isValid()){	// 1st run, default values
			if(!now)
				this->next = millis() + interval;
			else
				this->next = 0;
		}
	}

	void loop(void){
		if( millis() > this->next ){
			this->action();
			this->next = millis() + interval;
		}
	}
};

#endif
