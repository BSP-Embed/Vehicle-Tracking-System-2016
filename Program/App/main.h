#ifndef MAIN_H
#define MAIN_H

#include "includes.h"

#define INTDDR					DDRD
#define INTPORT					PORTD
#define INT0_PIN				PD2
#define INT1_PIN				PD3

//DEFINE CONSTANT
#define MSG_WAIT_MSG			1
#define MSG_PH_NUM				2
#define MSG_COLL_MSG			3
#define MSG_RCV_MSG				4

#define LINE_FEED				0x0A


//DEFINE MACROS
#define StartTmr()			TCCR0  	|= _BV(CS01)
#define StopTmr()			TCCR0  	&= ~_BV(CS01)
							
#define EnUARTInt()					UCSRB |= _BV(RXCIE); UCSRA |= _BV(RXC)
#define DisUARTInt()				UCSRB &= ~_BV(RXCIE); UCSRA &= ~_BV(RXC)

//FUNCTION PROTOTYPES
static void		 init		(void);
static void 	 disptitl 	(void);
static void 	tmr1init	(void);
static void 	EXTINTinit	(void);
static int8u	checkmsg	(void);
static void		VehicLock	(void);
static void		VehicUnlock	(void);

#endif

