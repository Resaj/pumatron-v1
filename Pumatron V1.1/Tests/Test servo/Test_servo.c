#include <lpc213x.h>
#include <stdio.h>
#include "font.h"		
#include "s1d15g00.h"
#include "mando.h"

// Frecuencia del cristal del LPC2138
#define Fpclk 19.6608e6*3/4
								 
// Variables auxiliares 
char cadena[30];			// Variable auxiliar para guardar el texto a imprimir en el LCD
float pwm_servo = 0.0016;	// Variable para el control del servo


/********************** FUNCIONES DE CONTROL **********************/
// Interrupción IRQ para el TIMER0
void TIMER0(void)__irq
{
	if(dato == BUTTON_CH_plus && pwm_servo != 1)
		pwm_servo += 0.0001;
	else if (dato == BUTTON_CH_minus && pwm_servo != 0)
		pwm_servo -= 0.0001;
	dato = 0;

	PWMMR5 = (float) pwm_servo * Fpclk;	// de 0.0006*Fpclk a 0.0026*Fpclk

	LCDClearScreen();	// Limpiar pantalla
	sprintf(cadena, "%f", pwm_servo);
	LCDPutStr(cadena, 80, 5, SMALL, WHITE, BLACK);

	PWMLER = (1<<0)|(1<<5);	// Enable PWM Match 0 & 5 Latch

	T0IR |= (1<<2);		// Activar flag de interrupción del Match0.2
	VICVectAddr = 0;
}


/************************ CONFIGURACIONES ************************/
// Configuración del Match0.2 del TIMER0
void config_TIMER0 (void)
{
	PINSEL1 |= 1<<1;
	T0TCR = 0x02;
	T0PR = 0;
	T0MCR = (1<<6)|(1<<7);
	T0MR2 = 0.5 * Fpclk;
	VICVectAddr1 = (unsigned long) TIMER0;
	VICVectCntl1 = (0x20|4);
	T0TCR = 0x01;
	VICIntEnable |= 1<<4;
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

// Configuración de las salidas de PWM para los motores
void config_SERVO (void)
{
	PINSEL1 |= (1<<10);		// PWM5
	PWMTCR = 0x02;			// Reset TIMER y PREESCALER										   
	PWMPR = 0;				// El PRESCALER no modifica la frecuencia
	PWMMR0 = 0.02 * Fpclk;	// Periodo PWM										   
	PWMLER = (1<<0)|(1<<5);	// Habilitar MATCH0 y MATCH5
	PWMMCR = 0x02;			// Reset TIMER COUNTER REGISTER ON MATCH0							   
	PWMPCR = (1<<13);		// Habilitar PWM5 en flanco
	PWMTCR = (1<<0)|(1<<3);	// Habilitar PWM y comenzar la cuenta
}
 

/************************** FUNCIÓN MAIN **************************/
int main (void)
{
	// Configuraciones previas a la iniciación de las respectivas funciones
	config_LCD();
	config_SERVO();
	config_TIMER1();
	config_TIMER0();

	while(1) // Bucle infinito para la ejecución del programa
	{
	}
}
