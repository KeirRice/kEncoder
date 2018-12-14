/*************************************************************
  Manage the encoders.
*************************************************************/

#pragma once

#include <Arduino.h>
#include <assert.h>
#include <kPin.h>

using namespace kPin;


namespace kEncoder{

	class Encoder
	{
		public:
			Group::PinsInterface *mPins;

			// Pass pin group into constuctor or later using setPins
			Encoder() : mPins(nullptr) {};
			void setPins(Group::PinsInterface &pins){mPins = &pins;};  // We need all both pins next to each other for fast reads.
			
			// Debounce the signals
			void setDebounceDelay(int delay);

			// Set the call back handler
			void setInteruptHandler(void (*interuptHandler)(void));
			void interputHandler();

			bool error(bool reset_flag = true);

			virtual void update(byte){};
			
		protected:
			volatile bool error_flag = false;
			
			unsigned int _debounce_delay = 0;

			void (*_interupt_handler)(void) = NULL;
	};

	class AbsoluteEncoder : public  Encoder
	{
		public:
			AbsoluteEncoder() : Encoder() {};

			void setup();
			void setup(Group::PinsInterface &pins, void (*interuptHandler)(void));
			
			volatile char position = 0;
			volatile char direction = 0;

		private:
			// Encoder state packed into a byte so we can use it as an index into the direction array.
			// bits 7-4 == previous values
			// bits 3-0 == current values
			volatile byte _absolute_encoder_state;

			void (*_interupt_handler)(void);

			void update(byte new_reading) override;

			typedef byte (AbsoluteEncoder::*fptr)(void);
	  		fptr read_values;
	};


	class RelativeEncoder : public Encoder {
		public:
			RelativeEncoder() : Encoder() {};

			void setup();
			void setup(Group::PinsInterface &pins, void (*interuptHandler)(void));

			volatile char steps = 0;
			volatile char direction = 0;

		private:
			volatile byte _relative_encoder_state;
			volatile char _relative_steps_buffer = 0;
			
			void (*_interupt_handler)(void);

			void update(byte new_reading) override;
	};
};


