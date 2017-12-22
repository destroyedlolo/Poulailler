/* Duration class
 *
 * 25/10/2017 First version
 */
#ifndef DURATION_H
#define DURATION_H

class Duration {
	unsigned long int start, stop;

	public:
		Duration( void ) { reInit(); }
		void reInit( void ) { start = millis(); stop = 0; }
		unsigned long int Finished( void ){ stop = millis(); return (stop - start); }
		unsigned long int operator *( void ){ return( (stop ? stop : millis()) - start ); }
};

#endif
