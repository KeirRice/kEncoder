/*************************************************************
  Manage the encoders.
*************************************************************/

#include "kEncoder.h"

#define LIBCALL_ENABLEINTERRUPT 
#include <EnableInterrupt.h>

namespace kEncoder{

	/* Return the number of sifts needed to put a 1 in the LSBit*/
	unsigned int maskToShiftCount(byte mask){
	  for(unsigned int i = 0; i < 8; ++i){
	  	if(mask & 1){
	  		return i;
	  	}
	  	mask = mask >> 1;
	  }
	  return 0;
	}


	/*
	*	Encoder
	*/


	void Encoder::setPort(volatile uint8_t *port, const uint8_t port_mask)
	{
		_port = port;
		_port_mask = port_mask;
	}
	void Encoder::setDebounceDelay(int delay)
	{
		_debounce_delay = delay;
	}
	void Encoder::setInteruptHandler(void (*interuptHandler)(void)){
		_interupt_handler = interuptHandler;
	}

	void Encoder::interputHandler()
	{
		  static uint32_t last_interrupt_time = 0;
		  uint32_t interrupt_time = millis();

		  // TODO: Check this algorithm works with events...
		  if (interrupt_time - last_interrupt_time > _debounce_delay) {
		    update((*_port & _port_mask) >> _port_shift_on_read);
		  }
		  last_interrupt_time = interrupt_time; 
	}


	bool Encoder::error(bool reset_flag /*= true*/) {
		// Check for encoder errors
		if (error_flag) {
			error_flag = !reset_flag;
			return true;
		}
		return false;
	}

	/*
	*	AbsoluteEncoder
	*/

	static const char absolute_position_table[16] = {0, 15, 7, 8, 3, 12, 4, 11, 1, 14, 6, 9, 2, 13, 5, 10};
	static const byte absolute_lower_nibble_mask = 0b00001111; // Only keep the four lowest bits

	void AbsoluteEncoder::setPins(int pinA, int pinB, int pinC, int pinD)
	{
		_pinA = pinA;
		_pinB = pinB;
		_pinC = pinC;
		_pinD = pinD;
	}

	void AbsoluteEncoder::setup(void (*interuptHandler)(void))
	{
		setInteruptHandler(interuptHandler);
		AbsoluteEncoder::setup();
	}
	void AbsoluteEncoder::setup()
	{
		pinMode(_pinA, INPUT_PULLUP);
		pinMode(_pinB, INPUT_PULLUP);
		pinMode(_pinC, INPUT_PULLUP);
		pinMode(_pinD, INPUT_PULLUP);

	 	_port_shift_on_read = maskToShiftCount(_port_mask);
		// Prime our values
		update((*_port >> _port_shift_on_read) & _port_mask);

		if(_interupt_handler){
			// Interupts on
			attachPinChangeInterrupt( (uint8_t) _pinA | PINCHANGEINTERRUPT, _interupt_handler, (uint8_t) CHANGE);
			attachPinChangeInterrupt( (uint8_t) _pinB | PINCHANGEINTERRUPT, _interupt_handler, (uint8_t) CHANGE);
			attachPinChangeInterrupt( (uint8_t) _pinC | PINCHANGEINTERRUPT, _interupt_handler, (uint8_t) CHANGE);
			attachPinChangeInterrupt( (uint8_t) _pinD | PINCHANGEINTERRUPT, _interupt_handler, (uint8_t) CHANGE);
		}
	}



	byte absoluteDirectionLookup(char previous_segment, char current_segment) {
	  /* Turns out a giant set of if statements is more memory effient than a 2D lookup table.
	  It's also compiled down to the same size as doing less comparisions by using the
	  raw absolute_encoder_state. 
	  
	  I guess sometimes super dumb and simple code works best.
	  */
	  
	  // No change
	  if (previous_segment == current_segment) {
	    return 0;
	  }

	  // Foward
	  if (previous_segment == 0 && current_segment == 1)
	  {
	    return 1;
	  }
	  if (previous_segment == 1 && current_segment == 3)
	  {
	    return 1;
	  }
	  if (previous_segment == 2 && current_segment == 6)
	  {
	    return 1;
	  }
	  if (previous_segment == 3 && current_segment == 2)
	  {
	    return 1;
	  }
	  if (previous_segment == 4 && current_segment == 12)
	  {
	    return 1;
	  }
	  if (previous_segment == 5 && current_segment == 4)
	  {
	    return 1;
	  }
	  if (previous_segment == 6 && current_segment == 7)
	  {
	    return 1;
	  }
	  if (previous_segment == 7 && current_segment == 5)
	  {
	    return 1;
	  }
	  if (previous_segment == 8 && current_segment == 0)
	  {
	    return 1;
	  }
	  if (previous_segment == 9 && current_segment == 8)
	  {
	    return 1;
	  }
	  if (previous_segment == 10 && current_segment == 11)
	  {
	    return 1;
	  }
	  if (previous_segment == 11 && current_segment == 9)
	  {
	    return 1;
	  }
	  if (previous_segment == 12 && current_segment == 13)
	  {
	    return 1;
	  }
	  if (previous_segment == 13 && current_segment == 15)
	  {
	    return 1;
	  }
	  if (previous_segment == 14 && current_segment == 10)
	  {
	    return 1;
	  }
	  if (previous_segment == 15 && current_segment == 14)
	  {
	    return 1;
	  }

	  // Back
	  if (previous_segment == 0 && current_segment == 8)
	  {
	    return -1;
	  }
	  if (previous_segment == 1 && current_segment == 0)
	  {
	    return -1;
	  }
	  if (previous_segment == 2 && current_segment == 3)
	  {
	    return -1;
	  }
	  if (previous_segment == 3 && current_segment == 1)
	  {
	    return -1;
	  }
	  if (previous_segment == 4 && current_segment == 5)
	  {
	    return -1;
	  }
	  if (previous_segment == 5 && current_segment == 7)
	  {
	    return -1;
	  }
	  if (previous_segment == 6 && current_segment == 2)
	  {
	    return -1;
	  }
	  if (previous_segment == 7 && current_segment == 6)
	  {
	    return -1;
	  }
	  if (previous_segment == 8 && current_segment == 9)
	  {
	    return -1;
	  }
	  if (previous_segment == 9 && current_segment == 11)
	  {
	    return -1;
	  }
	  if (previous_segment == 10 && current_segment == 14)
	  {
	    return -1;
	  }
	  if (previous_segment == 11 && current_segment == 10)
	  {
	    return -1;
	  }
	  if (previous_segment == 12 && current_segment == 4)
	  {
	    return -1;
	  }
	  if (previous_segment == 13 && current_segment == 12)
	  {
	    return -1;
	  }
	  if (previous_segment == 14 && current_segment == 15)
	  {
	    return -1;
	  }
	  if (previous_segment == 15 && current_segment == 13)
	  {
	    return -1;
	  }
	  return 2; // Error
	}



	void AbsoluteEncoder::update(byte new_reading)
	{
	  byte previous_absolute_position_index = _absolute_encoder_state & absolute_lower_nibble_mask;

	  _absolute_encoder_state = (_absolute_encoder_state << 4) | new_reading;
	  byte absolute_position_index = _absolute_encoder_state & absolute_lower_nibble_mask;
	  byte absolute_position = absolute_position_table[absolute_position_index];

	  //absoluteDirectionLookup
	  byte absolute_direction = absoluteDirectionLookup(previous_absolute_position_index, absolute_position_index);
	  switch (absolute_direction) {
	    case 0:
			// No change
			break;
			
	    case 2:
	      	// Error
	    	error_flag = true;
	      	// DEBUG_PRINTLN("Absolute position error.");
	      break;

	    default:
			// DEBUG_PRINT_VAR("We moved to ", absolute_position);
	  		direction = absolute_direction;
			break;
	  }
	  position = absolute_position;
	}



	/*
	*	RelativeEncoder
	*/


	static const char relativeDirectionLookup[16] = {
	  0,  // 00 00 // no change
	  -1, // 00 01
	  1,  // 00 10
	  2,  // 00 11 // error

	  1,  // 01 00
	  0,  // 01 01 // no change
	  2,  // 01 10 // error
	  -1, // 01 11

	  -1, // 10 00
	  2,  // 10 01 // error
	  0,  // 10 10 // no change
	  1,  // 10 11

	  2,  // 11 00 // error
	  1,  // 11 01
	  -1,  // 11 10
	  0  // 11 11 // no change
	};

	static const byte relative_lower_nibble_mask = 0b00001111;  // Only keep the four lowest bits

	void RelativeEncoder::setPins(int pinA, int pinB)
	{
		_pinA = pinA;
		_pinB = pinB;
	}

	void RelativeEncoder::setup(void (*interuptHandler)(void))
	{
		setInteruptHandler(interuptHandler);
		setup();
	}

	void RelativeEncoder::setup()
	{
		pinMode(_pinA, INPUT_PULLUP);
		pinMode(_pinB, INPUT_PULLUP);

		// Prime our values
		update((*_port & _port_mask) >> _port_shift_on_read);

		if(_interupt_handler){
			// Interupts on
			attachPinChangeInterrupt( (uint8_t) _pinA | PINCHANGEINTERRUPT, _interupt_handler, (uint8_t) CHANGE);
			attachPinChangeInterrupt( (uint8_t) _pinB | PINCHANGEINTERRUPT, _interupt_handler, (uint8_t) CHANGE);
		}
	  _port_shift_on_read = maskToShiftCount(_port_mask);
	 
	}


	void RelativeEncoder::update(byte new_data) {
	  _relative_encoder_state = (_relative_encoder_state << 2) | new_data;
	  byte new_direction = relativeDirectionLookup[_relative_encoder_state & relative_lower_nibble_mask];
	  if (new_direction == 2) {
	    // Don't do anything for errors
	    error_flag = true;
	  }
	  else {
	    steps += new_direction;
	  }
	}

}