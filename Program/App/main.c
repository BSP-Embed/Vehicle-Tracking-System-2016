#include "main.h"

int8u phnum[15];
int8u sbuf[100];

struct  {
	volatile int8u vibr:1;
	volatile int8u sw:1;
	volatile int8u theft:1;
	volatile int8u msg:1;
}AppFlags;

extern int8u lcdptr;

int main(void)
{
	init();

	AppFlags.theft = 0;
	AppFlags.sw = 0;
	AppFlags.vibr = 0;
	AppFlags.msg = 0;

	while (TRUE) {

		if (AppFlags.vibr) {
			
			GICR &=  ~_BV(INT0);		/* Disable Vibration */	
			MCUCR &= ~_BV(ISC01);	
						
			AppFlags.vibr = 0;
			
			DisUARTInt();
			
			lcdclrr(1);
			lcdws("Vibra'n Occurred");
			beep(1,1000);
			sendloc(1);
			lcdclrr(1);
			
			EnUARTInt();
			
			AppFlags.theft = 1;
		}

		if (AppFlags.sw) {
			
			GICR &=  ~_BV(INT0);		/* Disable Vibration */
			MCUCR &= ~_BV(ISC01);
			
			AppFlags.sw = 0;
			
			beep(1,100);
			moton();
			lcdclrr(1);
			lcdws(" Vehicle Moving");
		}
		
		if (AppFlags.msg) {
				DisUARTInt();
				AppFlags.msg = 0;
				lcdclrr(1);
				lcdws("Message Receiv'd");
				beep(1,250);
				switch (checkmsg()){ 
					case 1:	sendloc(0); break;
					case 2: VehicLock(); break;
					case 3: VehicUnlock(); break;
					default: beep(1,500); lcdclrr(1); break;
				}
				EnUARTInt();
		}
	}
	return 0;
}

static void VehicLock(void)
{
	motoff();
	lcdclrr(1);
	lcdws("Vehicle Locked!");
		
	GICR &=  ~_BV(INT1);		/* Disable Switch */
	MCUCR &= ~_BV(ISC11);
}

static void VehicUnlock(void)
{
	lcdclrr(1);
	lcdws("Vehicle UnLocked");
	GICR |=  _BV(INT1);		/* Enable Switch */
	MCUCR |= _BV(ISC11);
}

static int8u checkmsg(void)
{
	if (!strcmp(sbuf, "track"))
		return 1;
	else if (!strcmp(sbuf, "lock"))
		return 2; 
	else if (!strcmp(sbuf, "unlock"))
		return 3;
	else
		return 0;
}

static void init(void)
{
	buzinit();
	ledinit();

	beep(2,100);
	lcdinit();
	motorinit();
	
	uartinit();
	EXTINTinit();
	GPSinit();
	GSMEn();
	GSMinit();
	tmr1init();
	disptitl();
	EnUARTInt();
	sei();
	StartTmr();
	beep(1,100);
	
}
static void EXTINTinit(void)
{
	INTDDR 	&= ~_BV(INT0_PIN);
	INTPORT |= _BV(INT0_PIN);

	INTDDR 	&= ~_BV(INT1_PIN);
	INTPORT |= _BV(INT1_PIN);

	GICR |= _BV(INT0) | _BV(INT1);			//ENABLE EXTERNAL INTERRUPT
	MCUCR |= _BV(ISC01) | _BV(ISC11);		//FALLING EDGE INTERRUPT

}

static void disptitl(void)
{
	lcdclr();
	lcdws("AntiTheft Vehic");
}
		
static void tmr1init(void)
{
	TCNT1H   = 0xD3;
	TCNT1L   = 0x00;
	TIMSK   |= _BV(TOIE1);			//ENABLE OVERFLOW INTERRUPT
	TCCR1A   = 0x00;					
	TCCR1B  |= _BV(CS10) | _BV(CS11); /* PRESCALAR BY 16 */
}

/* overflows at every 100msec */
ISR(TIMER1_OVF_vect) 
{ 
	static int8u i,j,k;

	TCNT1H = 0xD3;
	TCNT1L = 0x00;
	
	if (++i >= 50) 
		 i = 0;
	
	switch(i) {
		case 0: case 2: ledon(); break;
		case 1: case 3: ledoff(); break;
	} 
}
ISR(INT0_vect) 
{ 
	AppFlags.vibr = 1;
	GICR |= _BV(INT0);
}
ISR(INT1_vect) 
{ 
	AppFlags.sw = 1;
	GICR |= _BV(INT1);
}
ISR (USART_RXC_vect) 
{
	static int8u i;
	static int8u msgcnt,phcnt;
	static int8u state = MSG_WAIT_MSG;

	switch (state) {
			case MSG_WAIT_MSG:
				if ( UDR == '\"') state = MSG_PH_NUM;
				break;
			case MSG_PH_NUM:
				if (phcnt++ < 13)
					phnum[phcnt-1] = UDR;
				else
					state = MSG_COLL_MSG;
				break;
			case MSG_COLL_MSG:
				if (UDR == LINE_FEED)
					state = MSG_RCV_MSG;
				break;
			case MSG_RCV_MSG:
				if ((sbuf[msgcnt++] = UDR) == LINE_FEED) {
					sbuf[msgcnt-2] = '\0';
					for (i = 0 ; i < 10; i++)	/* eliminate +91 */
						phnum[i] = phnum[i+3];
					phnum[i] = '\0';
					state = MSG_WAIT_MSG;
					msgcnt = 0;
					phcnt = 0;
					AppFlags.msg = 1;
					DisUARTInt();
				}
		}
}
