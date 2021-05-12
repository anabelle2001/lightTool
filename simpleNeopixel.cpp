//Code copied from
//https://github.com/bigjosh/SimpleNeoPixelDemo/blob/master/SimpleNeopixelDemo/SimpleNeopixelDemo.ino
// (Copyright © 2015 Josh Levine, adapted under MIT liscence)
//Modified Values to work with arduino nano and neopixels from 2019

#include "simpleNeopixel.h"
#include <Arduino.h>

#define PIXEL_PORT  PORTD  // Port of the pin the pixels are connected to
#define PIXEL_DDR   DDRD   // Port of the pin the pixels are connected to
#define PIXEL_BIT   6      // Bit of the pin the pixels are connected to

// These are the timing constraints taken mostly from the WS2812 datasheets 
// These are chosen to be conservative and avoid problems rather than for maximum throughput 
#define T0H  350    // Width of a 0 bit in ns
#define T1H  700    // Width of a 1 bit in ns
#define T0L  800    // Width of a 0 bit in ns
#define T1L  600    // Width of a 1 bit in ns

#define RES 250000    // Width of the low gap between bits to cause a frame to latch

// Note that this has to be SIGNED since we want to be able to check for 
//negative values of derivatives
#define NS_PER_SEC (1000000000L)          
#define CYCLES_PER_SEC (F_CPU)
#define NS_PER_CYCLE ( NS_PER_SEC / CYCLES_PER_SEC )
#define NS_TO_CYCLES(n) ( (n) / NS_PER_CYCLE )

void show() {
	delayMicroseconds( (RES / 1000UL) + 1);
	// Round up since the delay must be _at_least_ this long (too short might not work, too long not a problem)
}

void ledsetup() {
  bitSet( PIXEL_DDR , PIXEL_BIT );  
}

inline void sendBit( bool bitVal ) {
    if (  bitVal ) asm volatile (//1 bit
		// Set the output bit
		"sbi %[port], %[bit] \n\t"
		// Execute NOPs to delay exactly the specified number of cycles
		".rept %[onCycles] \n\t"
		"nop \n\t"
		".endr \n\t"
		// Clear the output bit
		"cbi %[port], %[bit] \n\t"
		// Execute NOPs to delay exactly the specified number of cycles
		".rept %[offCycles] \n\t"
		"nop \n\t"
		".endr \n\t"
		::
		[port]		"I" (_SFR_IO_ADDR(PIXEL_PORT)),
		[bit]		"I" (PIXEL_BIT),
		// 1-bit width less overhead  for the actual bit setting, note that 
		// this delay could be longer and everything would still work
		[onCycles]	"I" (NS_TO_CYCLES(T1H) - 2),		
		// Minimum interbit delay. Note that we probably don't need this at all
		// since the loop overhead will be enough, but here for correctness
		[offCycles] 	"I" (NS_TO_CYCLES(T1L) - 2)
	); else asm volatile (//0 bit
		// Set the output bit
		"sbi %[port], %[bit] \n\t"
		// Now timing actually matters. The 0-bit must be long enough to be 
		// detected but not too long or it will be a 1-bit
		".rept %[onCycles] \n\t" 
		// Execute NOPs to delay exactly the specified number of cycles
		"nop \n\t"
		".endr \n\t"
		// Clear the output bit
		"cbi %[port], %[bit] \n\t"
		// Execute NOPs to delay exactly the specified number of cycles
		".rept %[offCycles] \n\t"
		"nop \n\t"
		".endr \n\t"
		::
		[port]		"I" (_SFR_IO_ADDR(PIXEL_PORT)),
		[bit]		"I" (PIXEL_BIT),
		[onCycles]	"I" (NS_TO_CYCLES(T0H) - 2),
		[offCycles]	"I" (NS_TO_CYCLES(T0L) - 2)
	);



}

inline void sendByte( unsigned char byte ) {
    for( unsigned char bit = 0 ; bit < 8 ; bit++ ) {
      sendBit( bitRead( byte , 7 ) );// Neopixel wants bit in highest-to-lowest order
      byte <<= 1;
    }           
}


void sendPixel( unsigned char r, unsigned char g , unsigned char b )  {
  noInterrupts();
  sendByte(g);          // Neopixel wants colors in green then red then blue order
  sendByte(r);
  sendByte(b);
  interrupts();
}

