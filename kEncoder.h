/*************************************************************
  Manage the encoders.
*************************************************************/

#pragma once

#include <Arduino.h>
#include <assert.h>
#include <kPin.h>

using namespace kPin;

namespace kEncoder {

typedef volatile uint8_t RoReg; /**< Read only 8-bit register (volatile const unsigned int) */
typedef volatile uint8_t RwReg; /**< Read-Write 8-bit register (volatile unsigned int) */
typedef volatile RwReg * port_ptr_t;
typedef RwReg port_t;

uint8_t maskToShifter(uint8_t mask);



/*Interface for the pin collections*/
class PinCollectionInterface {
  public:
  	uint8_t mPinCount = 0;
  	PinID pins[] = {};

    virtual byte read();
    virtual void pinMode(uint8_t);

};


template<uint8_t... Pins> class PinGroup : public PinCollectionInterface {
  public:
    uint8_t mPinCount = sizeof...(Pins);
    PinID pins[sizeof...(Pins)] = {PinID(Pins)...};

    virtual byte read() override {
      byte value = 0;
      for (uint8_t i = 0; i < this->mPinCount; ++i) {
        value |= this->pins[i].digitalRead() << i;
      }
      return value;
    };

    virtual void pinMode(uint8_t mode) override {
      for (uint8_t i = 0; i < this->mPinCount; ++i) {
        this->pins[i].pinMode(mode);
      }
    };
};

class PinPort : public PinCollectionInterface {
  public:

    const PortID &mPort;
    uint8_t mMask;
    uint8_t mShift;

    PinPort(const PortID &port, uint8_t mask) : mPort(port), mMask(mask) {
      mShift = maskToShifter(mask);
    }

    inline virtual uint8_t read() override {
      return mPort.digitalRead(mMask, mShift);
    };
    virtual void pinMode(uint8_t mode) override {
    	mPort.pinMode(mode, mMask);
    };
};


class Encoder
{
  public:

    PinCollectionInterface *mPins;

    Encoder() : mPins(nullptr) {};
    Encoder(PinCollectionInterface &pins) : mPins(&pins) {};

    void setPins(PinCollectionInterface &pins) {
      mPins = &pins;
    };  // We need all both pins next to each other for fast reads.
    void setDebounceDelay(int delay);
    void setInteruptHandler(void (*interuptHandler)(void));

    void interputHandler();

    bool error(bool reset_flag = true);
    virtual void update(byte) {};

  protected:
    volatile bool error_flag = false;

    unsigned int _debounce_delay = 0;

    void (*_interupt_handler)(void);
};

class AbsoluteEncoder : public  Encoder
{
  public:

    AbsoluteEncoder() : Encoder() {};
    AbsoluteEncoder(PinCollectionInterface pins) : Encoder(pins) {};

    void setup();
    void setup(void (*interuptHandler)(void));

    volatile char position = 0;
    volatile char direction = 0;

  private:
    // Encoder state packed into a byte so we can use it as an index into the direction array.
    // bits 7-4 == previous values
    // bits 3-0 == current values
    volatile byte _absolute_encoder_state;

    void (*_interupt_handler)(void);

    void update(byte new_reading);

    typedef byte (AbsoluteEncoder::*fptr)(void);
    fptr read_values;
};


class RelativeEncoder : public Encoder {
  public:

    RelativeEncoder() : Encoder() {};
    RelativeEncoder(PinCollectionInterface pins) : Encoder(pins) {};

    void setup();
    void setup(void (*interuptHandler)(void));

    volatile char steps = 0;
    volatile char direction = 0;

  private:
    volatile byte _relative_encoder_state;
    volatile char _relative_steps_buffer = 0;

    void (*_interupt_handler)(void);

    void update(byte new_reading);
};
};


