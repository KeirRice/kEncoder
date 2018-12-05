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
		
		void setDebounceDelay(int delay);
		void setInteruptHandler(void (*interuptHandler)(void));

		void interputHandler();
		void setPort(volatile uint8_t *ddr, volatile uint8_t *port, volatile uint8_t *pin, uint8_t mask);  // We need all both pins next to each other for fast reads.

		byte read();
		
		bool error(bool reset_flag = true);
		virtual void update(byte){};
		virtual byte readDigital(){return 0;};
		byte readPort();

	protected:
		volatile bool error_flag = false;
		
		volatile uint8_t *_ddr;
		volatile uint8_t *_port;
		volatile uint8_t *_pin;

		uint8_t _port_mask;
		uint8_t _port_shift_on_read = 0;
		unsigned int _debounce_delay = 0;

		void (*_interupt_handler)(void);

		typedef byte (Encoder::*fptr)(void);
  		fptr read_values;

	};

	class AbsoluteEncoder : public  Encoder
	{
	public:

		AbsoluteEncoder() : Encoder() {};

		void setPins(int pinA, int pinB, int pinC, int pinD);

		void setup();
		void setup(void (*interuptHandler)(void));
		
		byte readDigital();

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

		typedef byte (AbsoluteEncoder::*fptr)(void);
  		fptr read_values;

	};


	class RelativeEncoder : public  Encoder {
	public:

		RelativeEncoder() : Encoder() {};
		RelativeEncoder(int pinA, int pinB): Encoder(), _pinA(pinA), _pinB(pinB) {};

		void setPins(int pinA, int pinB);

		void setup();
		void setup(void (*interuptHandler)(void));
		
		byte readDigital();

		volatile char steps = 0;
		volatile char direction = 0;

	private:
		int _pinA, _pinB;

		volatile byte _relative_encoder_state;
		volatile char _relative_steps_buffer = 0;
		

		void (*_interupt_handler)(void);

		void update(byte new_reading);

		typedef byte (RelativeEncoder::*fptr)(void);
  		fptr read_values;
	};

};



