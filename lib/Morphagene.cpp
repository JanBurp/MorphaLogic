#ifndef Morphagene_h
#define Morphagene_h

#include "Arduino.h"

class Morphagene {

	public:

		/**
		 * Setup state, inputs & outputs
		 */
		void init() {
            this->play = true;



			this->pin = pin;
			this->thresholdLow = thresholdLow;
			this->thresholdHigh = thresholdHigh;

		}

		/**
		 * Return the raw reading, as returned by analogRead()
		 */
		int readRaw() {
			return analogRead(this->pin);
		}

		/**
		 * Return the reading as a float number between 0 and 1, included.
		 * Optional thresholds are used map the raw values into the returned 0..1 range.
		 */
		float read() {

			int r = this->readRaw();

			if (r <= this->thresholdLow) {
				return 0.0;
			} else if (r >= this->thresholdHigh) {
				return 1.0;
			} else {
				return float(r - this->thresholdLow) / float(this->thresholdHigh - this->thresholdLow);
			}

		}

	private:
		bool play;

};

#endif
