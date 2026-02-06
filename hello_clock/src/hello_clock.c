#include "S32K144.h"            /* include peripheral declarations S32K144 */
#include "clocks_and_modes.h"

int lpit0_ch0_flag_counter = 0; /* LPIT0 timeout counter */

void PORT_init(void);
void LPIT0_init(void);
void WDOG_disable(void);

void PORT_init (void) {
  IP_PCC-> PCCn[PCC_PORTD_INDEX] = PCC_PCCn_CGC_MASK; /* Enable clock for PORT D */
  IP_PTD->PDDR |= 1<<0;            /* Port D0:  Data Direction= output */
  IP_PORTD->PCR[0] =  0x00000100;  /* Port D0:  MUX = ALT1, GPIO (to blue LED on EVB) */
}

void LPIT0_init (void) {
  IP_PCC->PCCn[PCC_LPIT_INDEX] = PCC_PCCn_PCS(6);    /* Clock Src = 6 (SPLL2_DIV2_CLK)*/
  IP_PCC->PCCn[PCC_LPIT_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clk to LPIT0 regs */
  IP_LPIT0->MCR = 0x00000001;    /* DBG_EN-0: Timer chans stop in Debug mode */
                              /* DOZE_EN=0: Timer chans are stopped in DOZE mode */
                              /* SW_RST=0: SW reset does not reset timer chans, regs */
                           	   /* M_CEN=1: enable module clk (allows writing other LPIT0 regs)*/
  IP_LPIT0->TMR[0].TVAL = 40000000;    /* Chan 0 Timeout period: 40M clocks */
  IP_LPIT0->TMR[0].TCTRL = 0x00000001; /* T_EN=1: Timer channel is enabled */
                              /* CHAIN=0: channel chaining is disabled */
                              /* MODE=0: 32 periodic counter mode */
                              /* TSOT=0: Timer decrements immediately based on restart */
                              /* TSOI=0: Timer does not stop after timeout */
                              /* TROT=0 Timer will not reload on trigger */
                              /* TRG_SRC=0: External trigger source */
                              /* TRG_SEL=0: Timer chan 0 trigger source is selected*/
}

void WDOG_disable (void){
  IP_WDOG->CNT=0xD928C520;    /*Unlock watchdog*/
  IP_WDOG->TOVAL=0x0000FFFF;  /*Maximum timeout value*/
  IP_WDOG->CS = 0x00002100;   /*Disable watchdog*/
}

int main(void) {
	WDOG_disable();
	PORT_init();
	SOSC_init_8MHz();
	SPLL_init_160MHz();
	/* Configure ports */
	/* Initialize system oscillator for 8 MHz xtal */
	/* Initialize sysclk to 160 MHz with 8 MHz SOSC */
	NormalRUNmode_80MHz();  /* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
	 LPIT0_init();
	/* Initialize PIT0 for 1 second timeout  */
	for (;;) {
		/* Toggle output to LED every LPIT0 timeout */
		while (0 == (IP_LPIT0->MSR & LPIT_MSR_TIF0_MASK)) {} /* Wait for LPIT0 CH0 Flag */
		lpit0_ch0_flag_counter++;
		/* Increment LPIT0 timeout counter */
		IP_PTD->PTOR |= 1<<0;
		/* Toggle output on port D0 (blue LED) */
		IP_LPIT0->MSR |= LPIT_MSR_TIF0_MASK; /* Clear LPIT0 timer flag 0 */
	}
}
