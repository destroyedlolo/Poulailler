/* Repeating class
 *
 * 23/12/2017 First version
 */
#ifndef REPEATER_H
#define REPEATER_H

#include "Context.h"

class Repeater : public Context::keepInRTC {
	struct {
		unsigned long int start;	// start of current run
		unsigned long int interval;
	} tokeep;

	Context &context;
public:
	virtual void action( void ) = 0;	// Which action to launch

	Repeater( Context &ctx, unsigned long int aint, bool now=false) : Context::keepInRTC( ctx, (uint32_t *)&this->tokeep, sizeof(tokeep) ), context( ctx ) {
	/* <- aint : interval b/w launches (ms)
	 * 	now : if the action has to be launched now
	 *
	 * 	Took in account only for the initial run
	 */
		if(!ctx.isValid()){	// 1st run, default values
			this->tokeep.start = (now) ? 0 : context.getTime();
			this->tokeep.interval = aint;
			this->save();
		}
	}

	unsigned long int next( void ){
		return this->tokeep.start + this->tokeep.interval;
	}

	void changeInterval( unsigned long int i ){
		this->tokeep.interval = i;
		this->save();
	}

	unsigned long int getInterval( void ){
		return this->tokeep.interval;
	}

	void loop( void ){
		if( context.getTime() > this->next() ){
			this->action();
			this->tokeep.start = context.getTime();
			this->save();
		}
	}

	unsigned long int remain(void){
	/* Return the duration before the next activation
	 * in mS
	 */
		if(context.getTime() > this->next())
			return 0;
		else
			return( this->next() - context.getTime() );
	}
};
#endif
