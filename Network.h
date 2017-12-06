/* Network handling 
 *
 * 05/12/2017 First version
 */

#ifndef NETWORK_H
#define NETWORK_H

	/*****
	 * Notez-bien : All fields will be persisted in RTC memory
	 *****/
class Network {
	enum NetworkMode { 
		SAFEDM, 	// Try Automation network first then home one
		SAFEMD,		// Try Home network first then automation one
		MAISON,		// Only one home network
		DOMOTIQUE	// Only one Automation network
	} mode;	// which mode is in use
	bool first;	// first connection attempt

protected:
	void init( void ){ /* Initial configuration */
		this->mode = NetworkMode::MAISON;
		this->first = true;
	}

public:
	bool isfirstrun( void ){ return this->first; }
	void hasrun( void ){ this->first = false; }
	
	void setMode( enum NetworkMode n ){ this->mode = n; }
	enum NetworkMode getMode( void ){ return this->mode; }
};

#endif
