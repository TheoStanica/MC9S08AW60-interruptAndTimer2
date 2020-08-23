#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */

#define VNkeyboard 22 /* Interrupt vector for Keyboard */ 
#define VNTimer 11

typedef	unsigned char	uint8;
typedef	unsigned short	uint16;
typedef	unsigned long	uint32;

typedef	char			int8;
typedef	short			int16;
typedef	long			int32;

// To Clear or Set single bits in a byte variable.
#define	b_SetBit(bit_ID, varID)		(varID |= (uint8)(1<<bit_ID))
#define	b_ClearBit(bit_ID, varID)	(varID &= ~(uint8)(1<<bit_ID))
#define	b_XorBit(bit_ID, varID)		(varID ^= (uint8)(1<<bit_ID))

// To Clear or Set single bits in a word(16-bit) variable.
#define	w_SetBit(bit_ID, varID)		(varID |= (uint16)(1<<bit_ID))
#define	w_ClearBit(bit_ID, varID)	(varID &= ~(uint16)(1<<bit_ID))
#define	w_XorBit(bit_ID, varID)		(varID ^= (uint16)(1<<bit_ID))

// To check single bit status in a given variable in ZERO PAGE
#define	b_CheckBit(varID, bit_ID)	((varID & (uint8)(1<<bit_ID))?1:0)
//#define	b_CheckBit(varID, bit_ID)	(varID & (muint8)(1<<bit_ID))
#define	w_CheckBit(varID, bit_ID) ((varID & (uint16)(1<<bit_ID))?1:0)

// To check single bit status in a given variable in ZERO PAGE
#define		DummyRead(varID)			{__asm lda varID;}




int ctr = 0;				// counter variable used to count how many times timer interrupt has been triggered.
void init(void);




//         NOTE: Code works like this:
//				- when the program starts, we call the init() method where we configure registers. Timer is configured but timer does not trigger when overflow flag is set
//				- when button is pressed, F0 turns on, KBI interrupt is turned off and timer is set to be triggered when overflow flag is set
// 				- ctr variable counts how many times timer has been triggered.
// 				- after we've simulated 5 seconds (5 x 1s timer), we set timer to not trigger when overflow flag is set,
//				reset the counter, turn off F0, re-enable KBI interrupt



void main(void) {
  EnableInterrupts;			//enable global interrupts
  
  init();					//calling the method that configures the ports and registers
  

  while(1)
  	{
  		__RESET_WATCHDOG(); 
  	}
  /* please make sure that you never leave main */
}

interrupt VNTimer void TPM1_overflow()
{ 
	byte varTOF; 
	varTOF = TPM1SC_TOF; // clear TOF; first read and then write 0 to the flag
	TPM1SC_TOF = 0;
	
	if(ctr < 6){
			ctr += 1;			//increase counter
	}
	else{
		TPM1SC = 0b00001011;	// 	disable timer  (bit 6 disables it) 
		ctr = 0;				//	reset the counter
		PTFD = 0x00;			//	turn off all LEDs
		KBI1PE_KBIPE5 = 1;		//	re-enable kbi interrupt
	}
		
}



interrupt VNkeyboard void intKBI_SW()
{
	TPM1SC = 0b01001011; 	// enable timer1
	PTFD = 0x01;			//turn on F0
	KBI1PE_KBIPE5 = 0; 		//disable kbi 
	KBI1SC_KBACK = 1; 		/*acknowledge interrupt*/
}




void init(){
	SOPT = 0x00;   			//disable watchdog
	
	
	
	ICGC2 = 0b00000000; // Set up ICG control register 2
	ICGC1 = 0X78;
	//THESE 2 DO NOT WORK FOR SOME REASON?(in the simulation)
	//ICGS1 = 0b01101010;
	
	
	
	PTFDD = 0x01;  			//set F0 as output
	PTFD = 0x00; 			//turn off all LEDs
	
							//For pushbutton switches we can use either PTC2 PTC6 PTD3 or PTD2
							// However only PTD3 and PTD2 can trigger interrupts.
							//we will set KBIP5 *port D2 pin*
	
	
	PTDDD = 0xA9; 			//set port D pin 2 as input
							// note: we could have set it as 0b11111011 as well
	
	PTDPE_PTDPE2 = 1; 		//enabled port D pin 2 pull ups

	
	KBI1SC_KBEDG5 = 0; 		//falling edge trigger  for the pin (pull up pin)
	
	KBI1SC = 0b11110010;	//bit7:4 = 0 => trigger on rising edge for all KBI ports
							//bit3 = 0   => no KBI interrupt pending
							//bit2 = 0   => write-only
							//bit1 = 1   => KBI generates hardware interrupt
							//bit0 = 0   => edge-only detection
	
	KBI1PE_KBIPE5 = 1;  //enable keyboard interrupt on the pin
	
	
	TPM1SC = 0b00001011;		//b7- 0 
								//b6 - 0 => timer is not triggered when overflow flag is set.
								//b5 - 0
								//b4:3 = 01 (select bus rate clock)
								//b2:0 = 011 (in the course example we used 111 to simulate 1 second(62500 counter overflow) 
								//            but in the simulator, same config results in aprox 16seconds simulated
								//			 So i decided to use a smaller divisor.(128/16 = 8 ->b2:0= 011) 
				
		
	//set overflow value to 62500
	TPM1MODH = 0xF4;
	TPM1MODL = 0x24;
	
}
