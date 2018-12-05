/*************************************************************
  Manage the encoders.
*************************************************************/
#pragma once

#define DEBOUNCE_DELAY 5

#define LIBCALL_ENABLEINTERRUPT 
#include <EnableInterrupt.h>


/*************************************************************
  State variables.
*************************************************************/


static char absolute_position;

// The encoder is using 4 bit grey codes. This means we need to map from the 4bit number to 
// the actual positions in sequence around the wheel.
static const char absolute_position_table[16] = {0, 15, 7, 8, 3, 12, 4, 11, 1, 14, 6, 9, 2, 13, 5, 10};

// Masks
static const byte absolute_port_mask = 0b00001111; // Only keep the four lowest bits
volatile uint8_t *absolute_port = PORTK;


static const byte absolute_lower_nibble_mask = 0b00001111; // Only keep the four lowest bits

/*************************************************************
  Interface
*************************************************************/

long GetAbsolutePosition() {
  return absolute_position;
}


/*************************************************************
  Encoder lookup tables.
*************************************************************/


char absoluteDirectionLookup(char previous_segment, char current_segment) {
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


/*************************************************************
  Functions
*************************************************************/

void UpdateAbsolutePosition(byte new_reading)
{
  // Encoder state packed into a byte so we can use it as an index into the direction array.
  // bits 7-4 == previous values
  // bits 3-0 == current values
  static byte absolute_encoder_state;

  byte previous_absolute_position_index = absolute_encoder_state & absolute_lower_nibble_mask;

  // Read the input data state of Bank B
  // Mask out the data keeping only pins 11-8
  // Shift the last data we got left four bits
  // OR the values togeather
  absolute_encoder_state = (absolute_encoder_state << 4) | new_reading;

  byte absolute_position_index = absolute_encoder_state & absolute_lower_nibble_mask;
  absolute_position = absolute_position_table[absolute_position_index];

  //absoluteDirectionLookup
  byte absolute_direction = absoluteDirectionLookup(previous_absolute_position_index, absolute_position_index);
  switch (absolute_direction) {
    case 0:
      // No change
      break;
    case 2:
      // Error
      DEBUG_PRINTLN("Absolute position error.");
      break;
    default:
      DEBUG_PRINT_VAR("We moved to ", absolute_position);
      break;
  }
}


/*************************************************************
  Setup and main loop.
*************************************************************/

void isr_handler(){
  static uint32_t last_interrupt_time = 0;
  uint32_t interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > DEBOUNCE_DELAY) {
    UpdateAbsolutePosition(PORTK & absolute_port_mask);
  }
  last_interrupt_time = interrupt_time; 
}

void encoder_absolute_setup()
{
  // Check the pins as we are hardcoded to them in the port_read_mask and UpdateAbsolutePosition.
  assert(PIN_G_IR == ARDUINO_A8);
  assert(PIN_H_IR == ARDUINO_A9);
  assert(PIN_I_IR == ARDUINO_A10);
  assert(PIN_J_IR == ARDUINO_A11);
  
  PIN_G_IR.pinMode(INPUT_PULLUP);
  PIN_H_IR.pinMode(INPUT_PULLUP);
  PIN_I_IR.pinMode(INPUT_PULLUP);
  PIN_J_IR.pinMode(INPUT_PULLUP);

  // Prime our values
  UpdateAbsolutePosition(PORTK & absolute_port_mask);

  // Interupts on
  attachPinChangeInterrupt(PIN_G_IR, isr_handler, CHANGE);
  attachPinChangeInterrupt(PIN_H_IR, isr_handler, CHANGE);
  attachPinChangeInterrupt(PIN_I_IR, isr_handler, CHANGE);
  attachPinChangeInterrupt(PIN_J_IR, isr_handler, CHANGE);
}

void encoder_absolute_loop() {
}

#endif // DISABLE_ENCODER_ABSOLUTE