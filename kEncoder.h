/*************************************************************
  Manage the encoders.
*************************************************************/

#pragma once

#include <Arduino.h>

namespace kEncoder{

	class Encoder
	{
	public:

		Encoder() {};
		
		void setPort(volatile uint8_t *port, uint8_t port_mask);  // We all both pins next to each other for fast reads.
		void setDebounceDelay(int delay);
		void setInteruptHandler(void (*interuptHandler)(void));

		void interputHandler();

		bool error(bool reset_flag = true);

	protected:
		volatile bool error_flag = false;
		
		volatile uint8_t *_port;
		uint8_t _port_mask;
		uint8_t _port_shift_on_read = 0;
		unsigned int _debounce_delay = 0;

		void (*_interupt_handler)(void);

		virtual void update(byte){};

	};

	class AbsoluteEncoder : public  Encoder
	{
	public:

		AbsoluteEncoder() : Encoder() {};
		AbsoluteEncoder(int pinA, int pinB, int pinC, int pinD): Encoder(), _pinA(pinA), _pinB(pinB), _pinC(pinC), _pinD(pinD) {};

		void setPins(int pinA, int pinB, int pinC, int pinD);

		void setup();
		void setup(void (*interuptHandler)(void));

		volatile char position = 0;
		volatile char direction = 0;

	private:
		int _pinA, _pinB, _pinC, _pinD;

		// Encoder state packed into a byte so we can use it as an index into the direction array.
		// bits 7-4 == previous values
		// bits 3-0 == current values
		volatile byte _absolute_encoder_state;

		void (*_interupt_handler)(void);

		void update(byte new_reading);
	};


	class RelativeEncoder : public  Encoder {
	public:

		RelativeEncoder() : Encoder() {};
		RelativeEncoder(int pinA, int pinB): Encoder(), _pinA(pinA), _pinB(pinB) {};

		void setPins(int pinA, int pinB);

		void setup();
		void setup(void (*interuptHandler)(void));

		volatile char steps = 0;
		volatile char direction = 0;

	private:
		int _pinA, _pinB;

		volatile byte _relative_encoder_state;
		volatile char _relative_steps_buffer = 0;
		

		void (*_interupt_handler)(void);

		void update(byte new_reading);
	};

};



