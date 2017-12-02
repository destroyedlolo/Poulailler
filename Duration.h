/* Duration class
 *
 * 25/10/2017 First version
 */

class Duration {
	unsigned long int start, stop;

	public:
		Duration( void ) : stop(0) { reInit(); }
		void reInit( void ) { start = millis(); }
		unsigned long int Finished( void ){ stop = millis(); return (stop - start); }
		unsigned long int operator *( void ){ return( (stop ? stop : millis()) - start ); }
};
