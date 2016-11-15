#include <lpc213x.h>
#include "Declaraciones.h"
#include "font.h"		
#include "s1d15g00.h"

/************************ CONFIGURACIONES ************************/
// Configuración de EINT1
void config_EINT1(void)
{
	PINSEL0 |= 1<<29;	//ó 3<<6
	EXTMODE |= 1<<1;
	EXTPOLAR |= 1<<1;
	VICVectAddr3 = (unsigned long) EXTERNA1;
	VICVectCntl3 = (0x20|15);
	VICIntEnable |= 1<<15;
}

// Configuración de EINT3
void config_EINT3(void)
{
	PINSEL0 |= 3<<18;
	EXTMODE |= 1<<3;
	EXTPOLAR &= ~(1<<3);
	VICVectAddr4 = (unsigned long) EXTERNA3;
	VICVectCntl4 = (0x20|17);
	VICIntEnable |= 1<<17;
}

// Configuración del ADC para los sensores
void config_ADC(void)
{
	PINSEL0 |= (3<<26)|(3<<30);	// Habilitar ADC1[4,5]
	PINSEL1 |= (1<<12);			// Habilitar ADC1[7]

	VICVectAddr2 = (unsigned long) ADC1;
	VICVectCntl2 = (0x20|21);
	VICIntEnable |= (1<<21);

	AD1CR = 0x2103B0;
}

// Configuración del Capture1.0 y Capture1.2
void config_TIMER1(void)
{
	PINSEL0 |= (1<<21);
	PINSEL1 |= (1<<2);
	T1TCR = 0x02;
	T1CTCR = 0;
	T1PR = 0;
	T1TCR = 0x01;
 	T1CCR = (3<<1)|(3<<7);
	VICVectCntl0 = (0x20|5);
	VICVectAddr0 = (unsigned long) TIMER1;
	VICIntEnable |= (1<<5);
}

// Configuración del Match0.2 del TIMER0
void config_TIMER0 (void)
{
	PINSEL1 |= 1<<1;
	T0TCR = 0x02;
	T0PR = 0;
	T0MCR = (1<<6)|(1<<7);
	T0MR2 = (unsigned long)(Periodo*Fpclk);
	VICVectAddr1 = (unsigned long) TIMER0;
	VICVectCntl1 = (0x20|4);
	T0TCR = 0x01;
	VICIntEnable |= 1<<4;
}

// Configuración de las salidas de PWM para los motores
void config_MOTORES (void)
{
	PINSEL0 |= (1<<15)|(1<<17);		// PWM2 y PWM4
	IO0DIR |= (1<<20)|(1<<23);		// Salidas para el sentido de los motores
	IO1DIR |= (1<<30)|(1<<31);
	PWMTCR = 0x02;					// Reset TIMER y PREESCALER										   
	PWMPR = 0;						// El PRESCALER no modifica la frecuencia
	PWMMR0 = Per_pwm * Fpclk;			// Periodo PWM									   
	PWMLER = (1<<0)|(1<<2)|(1<<4);	// Habilitar MATCH0, MATCH2 y MATCH4
	PWMMCR = 0x02;					// Reset TIMER COUNTER REGISTER ON MATCH0							   
	PWMPCR = (1<<10)|(1<<12);		// Habilitar PWM2 y PWM4 en flanco
	PWMTCR = (1<<0)|(1<<3);			// Habilitar PWM y comenzar la cuenta
}

// Configuración de los leds
void config_leds(void)
{
	IO0DIR |= (1<<31)|(1<<27);
	IO1DIR |= (1<<22)|(1<<23);
}

// Configuración del display
void config_LCD (void)
{
  LCD_BL_DIR();			// BL = Output
  LCD_CS_DIR();			// CS = Output
  LCD_SCLK_DIR();		// SCLK = Output
  LCD_SDATA_DIR();		// SDATA = Output
  LCD_RESET_DIR();		// RESET = Output
  LCD_SCLK_LOW();		// Standby SCLK
  LCD_CS_HIGH();		// Disable CS
  LCD_SDATA_HIGH();		// Standby SDATA
  LCD_BL_HIGH();		// Black Light ON = 100%

  InitLcd(); 			// Initial LCD
  LCDClearScreen();		// Limpiar pantalla
}
