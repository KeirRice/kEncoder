/*************************************************************
  Manage the relative encoder.

  This circuit expects the two sensors to have a digital pin each on the same port.
  Their outputs are also XORed and sent to an interupt pin.

  The data is collected on interupt and batched togeather until
  the main loop comes around and syncs the data.

  The idea is that the interupts should spin really fast and just grab all
  data including bounces. The loop will be slower and the data should have
  setteled a bit.

*************************************************************/
#pragma once

#ifdef DISABLE_ENCODER_RELATIVE

void encoder_relative_setup() {}
void encoder_relative_loop() {}

#else

#define LIBCALL_ENABLEINTERRUPT 
#include <EnableInterrupt.h>

#include "Helpers.h"
#include <util/atomic.h> // this library includes the ATOMIC_BLOCK macro.

/*************************************************************
  State variables.
*************************************************************/

static const byte relative_lower_nibble_mask = 0b00001111;  // Only keep the four lowest bits
static const byte relative_port_read_mask = 0b00000011;     // ARDUINO_D9, ARDUINO_D8

static volatile byte relative_encoder_state;
static volatile char relative_steps_buffer = 0;
static volatile boolean relative_error_flag = false;

static char relative_direction = 1;
static long relative_steps = 0;

/*************************************************************
  Encoder lookup tables.
*************************************************************/

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


/*************************************************************
  Functions
*************************************************************/

void set_home_position() {
  // Reset current steps to 0
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    relative_steps_buffer = 0;
    relative_steps = 0;
  }
}

long get_relative_position() {
  return relative_steps;
}


/*************************************************************
  Interupt
*************************************************************/

void process_new_relative_data(byte new_data) {
  relative_encoder_state = (relative_encoder_state << 2) | new_data;
  byte new_direction = relativeDirectionLookup[relative_encoder_state & relative_lower_nibble_mask];
  if (new_direction == 2) {
    // Don't do anything for errors
    relative_error_flag = true;
  }
  else {
    relative_steps += new_direction;
  }
}

/*************************************************************
  Internal calls
*************************************************************/

void sync_interupt_counts() {
  // Sync the values across the gap
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    relative_steps += relative_steps_buffer;
    relative_direction = sign(relative_steps);
    relative_steps_buffer = 0;
  }
}

boolean encoder_error(boolean reset_flag = true) {
  // Check for encoder errors
  if (relative_error_flag) {
    relative_error_flag = !reset_flag;
    return true;
  }
  return false;
}

/*************************************************************
  Setup and main loop.
*************************************************************/

void relativeISR(){
  process_new_relative_data((PORTK >> 4) & 0b00000011);
}

void encoder_relative_setup()
{
  // Check the pins as relativeISR is hardcoded to them.
  assert(PIN_E_SWITCH == ARDUINO_D66);
  assert(PIN_F_SWITCH == ARDUINO_D67);

  PIN_E_SWITCH.pinMode(INPUT_PULLUP);
  PIN_F_SWITCH.pinMode(INPUT_PULLUP);

  // Read the encoder state to get us started. On init current and previous can be the same.
  relative_encoder_state = (PIN_F_SWITCH.digitalRead() << 1) | PIN_E_SWITCH.digitalRead();
  relative_encoder_state = (relative_encoder_state << 2) | relative_encoder_state;
  
  attachPinChangeInterrupt(PIN_E_SWITCH, relativeISR, CHANGE);
  attachPinChangeInterrupt(PIN_F_SWITCH, relativeISR, CHANGE);
}



void encoder_relative_loop() {
  sync_interupt_counts();
  if (encoder_error()) {
    evtManager.trigger(ERROR_REL_DIRECTION);
  }
}
#endif // DISABLE_ENCODER_RELATIVE