#include <lpc213x.h>
#include "Declaraciones.h"

/************************ CONFIGURACIONES ************************/
// Configuraci�n de EINT1
void config_EINT1(void)
{
	PINSEL0 |= 1<<29;	//� 3<<6
	EXTMODE |= 1<<1;
	EXTPOLAR |= 1<<1;
	VICVectAddr0 = (unsigned long) EXTERNA1;
	VICVectCntl0 = (0x20|15);
	VICIntEnable |= 1<<15;
}

// Configuraci�n de EINT3
void config_EINT3(void)
{
	PINSEL0 |= 3<<18;
	EXTMODE |= 1<<3;
	EXTPOLAR &= ~(1<<3);
	VICVectAddr3 = (unsigned long) EXTERNA3;
	VICVectCntl3 = (0x20|17);
	VICIntEnable |= 1<<17;
}

// Configuraci�n del ADC0 para los sensores de l�nea
void config_ADC0 (void)
{
	PINSEL0 |= (1<<8)|(1<<9)|(1<<10)|(1<<11);					// Habilitar ADC0[6,7]
	PINSEL1 |= (1<<18)|(1<<20)|(1<<22)|(1<<24)|(1<<26)|(1<<28);	// Habilitar ADC0[0..5]

	VICVectAddr2 = (unsigned long) ADC0;
	VICVectCntl2 = (0x20|18);
	VICIntEnable |= (1<<18);

	AD0CR = 0x2103FF;	// Activaci�n de la conversi�n en modo burst para los 8 canales de ADC0
}

// Configuraci�n del Match0.2 del TIMER0
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

// Configuraci�n de las salidas de PWM para los motores
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

// Configuraci�n de los leds
void config_leds(void)
{
	IO0DIR |= 1<<31;
	IO1DIR |= 1<<22;
}
