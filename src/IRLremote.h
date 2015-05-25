/*
Copyright (c) 2014-2015 NicoHood
See the readme for credit to other people.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

// include guard
#pragma once

// software version
#define IRL_VERSION 181

#include <Arduino.h>

// include PinChangeInterrupt library if available
#ifdef PCINT_VERSION
#include "PinChangeInterrupt.h"
#endif

// delay_basic is only for avrs. With ARM sending is currently not possible
#ifdef ARDUINO_ARCH_AVR
#include <util/delay_basic.h>
#endif

//================================================================================
// Definitions
//================================================================================

// missing 1.0.6 definition workaround
#ifndef NOT_AN_INTERRUPT
#define NOT_AN_INTERRUPT -1
#endif

typedef enum IRType {
	IR_NO_PROTOCOL, // 0
	IR_USER, // 1
	IR_ALL, // 2
	IR_NEC, // ...
	IR_NEC_EXTENDED,
	IR_NEC_REPEAT,
	IR_PANASONIC,
	IR_SONY8,
	IR_SONY12,
	IR_SONY15,
	IR_SONY20,
	// add new protocols here
};

typedef struct IR_data_t {
	// variables to save received data
	uint8_t protocol;
	uint16_t address;
	uint32_t command;

	//TODO add nec struct/panasonic with id, checsum etc
};

// definition to convert an uint8_t array to an uint16_t/uint32_t at any position (thx timeage!)
#define UINT16_AT_OFFSET(p_to_8, offset)    ((uint16_t)*((const uint16_t *)((p_to_8)+(offset))))
#define UINT32_AT_OFFSET(p_to_8, offset)    ((uint32_t)*((const uint32_t *)((p_to_8)+(offset))))

#include "IRLprotocols.h"
#include "IRLkeycodes.h"

//================================================================================
// Receive
//================================================================================

// variadic template to choose the specific protocols that should be used
template <uint32_t debounce, IRType ...irProtocol>
class CIRLremote {
public:
	CIRLremote(void);

	// attach the interrupt so IR signals are detected
	bool begin(uint8_t pin);
	bool end(uint8_t pin);

	// user access to the libraries data
	static bool available(void) __attribute__((always_inline));
	static IR_data_t read(void); __attribute__((always_inline));

	// TODO get time functions
	static CIRL_NEC Nec;

protected:
	// interrupt function that is attached
	static void IRLinterrupt(void);

	// decode functions
	static uint8_t decodeNecOnly(const uint16_t duration) __attribute__((always_inline));
	static uint8_t decodeNec(const uint16_t duration) __attribute__((always_inline));
	static uint8_t decodePanasonicOnly(const uint16_t duration) __attribute__((always_inline));
	static uint8_t decodePanasonic(const uint16_t duration) __attribute__((always_inline));
	static uint8_t decodeSony12(const uint16_t duration) __attribute__((always_inline));
	static void decodeSony20(const uint16_t duration) __attribute__((always_inline));

	//// multifunctional template for receiving
	//template <uint8_t irLength, uint16_t timeoutThreshold, uint16_t markLeadThreshold, uint16_t spaceLeadThreshold,
	//	uint16_t spaceLeadHoldingThreshold, uint16_t markThreshold, uint16_t spaceThreshold,
	//	uint16_t markTimeout, uint16_t spaceTimeout>
	//	static bool IRLdecode(uint16_t duration, uint8_t data[], uint8_t &count) __attribute__((always_inline));

	// variables to save received data
	static uint8_t protocol;

	// time values for the last interrupt and the last valid protocol
	static uint32_t lastTime;
	static uint32_t lastEvent;

	// temporary buffer to hold bytes for decoding the protocols
	// not all of them are compiled, only the used ones
	static uint8_t dataNec[NEC_BLOCKS];
	static uint8_t countNec;
	static uint8_t dataPanasonic[PANASONIC_BLOCKS];
	static uint8_t countPanasonic;
	static uint8_t dataSony12[SONY_BLOCKS_12];
	static uint8_t countSony12;
	static uint8_t dataSony20[SONY_BLOCKS_20];
	static uint8_t countSony20;

	// todo move to hpp, change names
	// thanks to hs_ from the ##c++ channel on freenode
	template <IRType ...Ns>
	struct is_in;

	template <IRType N, IRType M, IRType ...Ns>
	struct is_in < N, M, Ns... >
	{
		static const bool value = (N == M) || is_in<N, Ns...>::value;
	};

	template <IRType N, IRType M>
	struct is_in < N, M > {
		static const bool value = (N == M);
	};

	template <IRType N>
	struct is_in < N > {
		// always return true (use any protocol) if no arguemnts were passed
		static const bool value = true;
	};

};

// implementation inline, moved to another .hpp file
#include "IRLremoteReceive.hpp"


//================================================================================
// Transmit
//================================================================================

// functions to send the protocol
template <IRType irType>
inline void IRLwrite(const uint8_t pin, uint16_t address, uint32_t command) __attribute__((always_inline));

template <uint8_t addressLength, uint8_t commandLength,
          uint16_t Hz, bool addressFirst,
          uint16_t markLead, uint16_t spaceLead,
          uint16_t markZero, uint16_t markOne,
          uint16_t spaceZero, uint16_t spaceOne>
inline void IRLsend(volatile uint8_t * outPort, uint8_t bitmask, uint16_t address, uint32_t command) __attribute__((always_inline));

// functions to set the pin high or low (with bitbang pwm)
inline void IRLmark(const uint16_t Hz, volatile uint8_t * outPort, uint8_t bitMask, uint16_t time) __attribute__((always_inline));
inline void IRLspace(volatile uint8_t * outPort, uint8_t bitMask, uint16_t time) __attribute__((always_inline));

// implementation inline, moved to another .hpp file
#include "IRLremoteTransmit.hpp"