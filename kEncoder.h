/*************************************************************
  Manage the encoders.
*************************************************************/

#pragma once

#include <Arduino.h>
#include <assert.h>
#include <kPin.h>

using namespace kPin;


namespace kEncoder{

	typedef volatile uint8_t RoReg; /**< Read only 8-bit register (volatile const unsigned int) */
	typedef volatile uint8_t RwReg; /**< Read-Write 8-bit register (volatile unsigned int) */
	typedef volatile RwReg * port_ptr_t;
	typedef RwReg port_t;

	uint8_t maskToShifter(uint8_t mask);

	

	/*Interface for the pin collections*/
	class PinCollectionInterface {
		public:
			uint8_t mPinCount;
			uint8_t pins[];
			virtual byte read();
			virtual void pinMode(uint8_t);

	};

	template<uint8_t... Pins> class _PinCollection : public PinCollectionInterface {
		public:
		  uint8_t mPinCount = sizeof...(Pins);
		  uint8_t pins[sizeof...(Pins)] = {uint8_t(Pins)...};
	};

	template<uint8_t... Pins> class PinBank : public _PinCollection<Pins...>{
		public:
			uint8_t mPinCount;
			uint8_t pins[];
			
		  RwReg sPinMask;
		  volatile RoReg *sInPort;
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
		      sInPort = portInputRegister(digitalPinToPort(arg));
		    }
		    else {
		      // Fail if any of the pins are on a diffrent port.
		      assert(sInPort == portInputRegister(digitalPinToPort(arg)));
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
		  		volatile RwReg *sDDRPort = portModeRegister(digitalPinToPort(pins[0]));
		  		volatile RwReg *sPort  = portOutputRegister(digitalPinToPort(pins[0]));

			  	*sDDRPort |= ~sPinMask; // Set mask pins to input
				*sPort |= sPinMask; // Set mask pins to pullup
			}
			else {
				assert(false); // "todo"
			}
		  };
	};

	class PortGroup : public PinCollectionInterface {
			PortID &mPort;
			uint8_t mMask;
			uint8_t mShift = 0;
			public:
				PortGroup(PortID port, uint8_t mask) : mPort(port), mMask(mask) {
					mShift = maskToShifter(mask);
				};

			  virtual byte read() override{
			  	return mPort.digitalRead(mMask, mShift);
			  };

			  virtual void pinMode(uint8_t mode) override{
			  	mPort.pinMode(mode, mMask);
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

		PinCollectionInterface *mPins;

		Encoder() : mPins(nullptr) {};
		Encoder(PinCollectionInterface &pins) : mPins(&pins) {};

		void setPins(PinCollectionInterface &pins){mPins = &pins;};  // We need all both pins next to each other for fast reads.
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


