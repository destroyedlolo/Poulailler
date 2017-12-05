/* Network handling 
 *
 * 05/12/2017 First version
 */

#ifndef NETWORK_H
#define NETWORK_H

class Network {
public :
	enum NetworkMode { 
		SAFEDM, 	// Try Automation network first then home one
		SAFEMD,		// Try Home network first then automation one
		MAISON,		// Only one home network
		DOMOTIQUE	// Only one Automation network
	};

		/* Information to be persisted */
	struct keep {
		enum NetworkMode mode;	// which mode is in use
		bool first;				// first connection attempt

			/* Initial configuration */
		keep(){
			this->mode = NetworkMode::MAISON;
			this->first = true;
		}
	};
};

#endif
