/* Repeating class
 *
 * 23/12/2017 First version
 */
#ifndef REPEATER_H
#define REPEATER_H

#include "Context.h"

class Repeater : public Context::keepInRTC {
	unsigned long int next;	// next run
	unsigned long int interval;
	Context &context;

public:
	virtual void action( void ) = 0;	// Which action to launch

	Repeater( Context &ctx, unsigned long int aint, bool now=false) : Context::keepInRTC( ctx, (uint32_t *)&next, sizeof(next) ), interval( aint ), context( ctx ) {
	/* <- aint : interval b/w launches (ms)
	 * 	now : if the action has to be launched now
	 */
		if(!ctx.isValid()){	// 1st run, default values
			if(!now)
				this->next = context.getTime() + interval;
			else
				this->next = 0;
			this->save();
		}
	}

	void loop(void){
		if( context.getTime() > this->next ){
			this->action();
			this->next = context.getTime() + interval;
			this->save();
		}
	}

	unsigned long int remain(void){
	/* Return the duration before the next activation
	 * in mS
	 */
		if(context.getTime() > this->next)
			return 0;
		else
			return( this->next - context.getTime() );
	}
};

#endif
