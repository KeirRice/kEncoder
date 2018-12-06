/*************************************************************
  Manage the encoders.
*************************************************************/

#pragma once

#include <Arduino.h>
#include <assert.h>


namespace kEncoder{

	typedef volatile uint8_t RoReg; /**< Read only 8-bit register (volatile const unsigned int) */
	typedef volatile uint8_t RwReg; /**< Read-Write 8-bit register (volatile unsigned int) */
	typedef volatile RwReg * port_ptr_t;
	typedef RwReg port_t;

	

	/* Return the number of sifts needed to put a 1 in the LSBit*/
	uint8_t maskToShifter(uint8_t mask){
	  for(unsigned int i = 0; i < 8; ++i){
	  	if(mask & 1){
	  		return i;
	  	}
	  	mask = mask >> 1;
	  }
	  return 0;
	}

	/*Interface for the pin collections*/
	class PinCollection {
		public:
			uint8_t mPinCount;
			uint8_t pins[];

			virtual byte read(){return 0;};
			virtual void pinMode(uint8_t){};

	};

	template<uint8_t... Pins> class _PinCollection : public PinCollection {
		public:
		  uint8_t mPinCount = sizeof...(Pins);
		  uint8_t pins[sizeof...(Pins)] = {uint8_t(Pins)...};
	};

	template<uint8_t... Pins> class PinBank : public _PinCollection<Pins...>{
		public:
		  RwReg sPinMask;
		  volatile RwReg *sPort;
		  volatile RoReg *sInPort;
		  volatile RwReg *sDDRPort;
		  uint8_t sPortShiftOnRead;

		  /* Compile time assert that pins are contiguous. */
		  template<class T, class... Args>
		  constexpr static bool check(T arg1, T arg2){
		    return (bool)(arg1 + 1 == arg2);
		  };
		  template<class T, class... Args>
		  constexpr static bool check(T arg1, T arg2, Args... args){
		    return (bool) ((arg1 + 1 == arg2) & check(arg2, args...));
		  };
		  static_assert(check(Pins...), "Invalid pins specified. We need pins in a contigous range on the same port.");

		  PinBank() {
		    _init(Pins...);
		    sPortShiftOnRead = maskToShifter(sPinMask);
		  }

		  /* Build up port data. */
		  void _init(int counter=0){};
		  template<class T, class... Args>
		  void _init(T arg, Args... args){
		  	int counter=0;
		    if(counter == 0){
		      // Set these values once, then we can just check the reset of the pins match.
		      sPort = portOutputRegister(digitalPinToPort(arg));
		      sInPort = portInputRegister(digitalPinToPort(arg));
		      sDDRPort = portModeRegister(digitalPinToPort(arg));
		    }
		    else {
		      // Fail if any of the pins are on a diffrent port.
		      assert(sPort == portOutputRegister(digitalPinToPort(arg)));
		    }
		    // Accumulate the bit mask
		    sPinMask |= digitalPinToBitMask(arg);
		    _init(args..., ++counter);
		  };

		  virtual byte read() override {
		  	return (*sInPort & sPinMask) << sPortShiftOnRead;
		  };
		  virtual void pinMode(uint8_t mode) override {
		  	if(mode == INPUT_PULLUP){
			  	*sDDRPort |= ~sPinMask; // Set mask pins to input
				*sPort |= sPinMask; // Set mask pins to pullup
			}
			else {
				assert(false); // "todo"
			}
		  };
	};

	template<uint8_t... Pins> class PinGroup : public _PinCollection<Pins...> {
		public:

		  virtual byte read() override{
		    byte value = 0;
		    for(uint8_t i=0; i < this->mPinCount; ++i){
		      value |= ::digitalRead(this->pins[i]) << i;
		    }
		    return value;
		  };

		  virtual void pinMode(uint8_t mode) override{
		    for(uint8_t i=0; i < this->mPinCount; ++i){
		    	::pinMode(this->pins[i], mode);
		    }
		  };
	};


	class Encoder
	{
	public:

		PinCollection *mPins;

		Encoder() : mPins(nullptr) {};
		Encoder(PinCollection &pins) : mPins(&pins) {};

		void setPins(PinCollection &pins){mPins = &pins;};  // We need all both pins next to each other for fast reads.
		void setDebounceDelay(int delay);
		void setInteruptHandler(void (*interuptHandler)(void));

		void interputHandler();

		bool error(bool reset_flag = true);
		virtual void update(byte){};
		
	protected:
		volatile bool error_flag = false;
		
		unsigned int _debounce_delay = 0;

		void (*_interupt_handler)(void);
	};

	class AbsoluteEncoder : public  Encoder
	{
	public:

		AbsoluteEncoder() : Encoder() {};
		AbsoluteEncoder(PinCollection pins) : Encoder(pins) {};

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
		RelativeEncoder(PinCollection pins) : Encoder(pins) {};

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



