#include <lpc213x.h>
#include "Declaraciones.h"
#include "font.h"		
#include "s1d15g00.h"

/************************ CONFIGURACIONES ************************/
// Configuración del pulsador de load
void config_load(void)
{
	IO0DIR &= ~(1<<14);
}

// Configuración de EINT3
void config_EINT3(void)
{
	PINSEL0 |= 3<<18;
	EXTMODE |= 1<<3;
	EXTPOLAR &= ~(1<<3);
	VICVectAddr3 = (unsigned long) EXTERNA3;
	VICVectCntl3 = (0x20|17);
	VICIntEnable |= 1<<17;
}

// Configuración del ADC0 para los sensores de línea
void config_ADC0 (void)
{
	PINSEL0 |= (1<<8)|(1<<9)|(1<<10)|(1<<11);		// Habilitar ADC0[6,7]
	PINSEL1 |= (1<<20)|(1<<24)|(1<<26)|(1<<28);		// Habilitar ADC0[1,2,3,5]
	IO0DIR |= 1<<25; 								// Salida del multiplexor

	AD0CR = 0x200000;
}

// Configuración de los potenciómetros para regular el PID
void config_ADC1(void)
{
	PINSEL0 |= (3<<12)|(3<<24)|(3<<30);		// Habilitar ADC1[0,3,5]

	AD1CR = 0x200000;
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
	VICVectAddr2 = (unsigned long) TIMER0;
	VICVectCntl2 = (0x20|4);
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
	LCD_SCLK_DIR();			// SCLK = Output
	LCD_SDATA_DIR();		// SDATA = Output
	LCD_RESET_DIR();		// RESET = Output
	LCD_SCLK_LOW();			// Standby SCLK
	LCD_CS_HIGH();			// Disable CS
	LCD_SDATA_HIGH();		// Standby SDATA
	LCD_BL_HIGH();			// Black Light ON = 100%
	
	InitLcd(); 				// Initial LCD
	LCDClearScreen();		// Limpiar pantalla
}

// Configuración de la UART0
void config_UART0(void)
{
	PINSEL0 |= (1<<0)|(1<<2);	// Activar RxD0 y TxD0
	U0FCR=0x00;					// Deshabilitar FIFO's
	U0LCR = 0x83;				// 8 bits, sin bit de paridad y 1 bit de stop

    Fdiv = ( Fpclk / 16 ) / baudrate ;
    U0DLM = Fdiv / 256;							
    U0DLL = Fdiv % 256;	   

	U0LCR = 0x3;				// DLAB = 0
	U0IER = 0x03;				// Activar recepción y transmisión de datos

	VICVectAddr1 = (unsigned long) UART0;
	VICVectCntl1 = 0x20|6;
	VICIntEnable = 1<<6;
}
