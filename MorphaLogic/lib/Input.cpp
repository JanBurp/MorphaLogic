#ifndef Input_h
#define Input_h

#include "Arduino.h"

class Input {
	
	public:
		
		/** 
		 * Setup the input
		 */
		void init(byte pin,  bool invert = false, bool internalPullup = false) {
			
			this->pin = pin;
			this->invert = invert;
			
			pinMode(this->pin, internalPullup ? INPUT_PULLUP : INPUT);
		}
		
		/** 
		 * Get the input state, TRUE if the pin is HIGH.
		 * Immediately reads presses, but the release can be delayed according to debouncing.
		 */
		bool read() {
			
			bool reading = digitalRead(this->pin);
			if (this->invert) reading = !reading;
			
			if (reading) {
				return true;
			}

			return false;

		}

		
	private:
		byte pin;
		bool invert;
		
};

#endif
