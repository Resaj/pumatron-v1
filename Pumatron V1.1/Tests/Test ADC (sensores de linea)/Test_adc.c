#include <lpc213x.h>
#include <stdio.h>
	   
// Ficheros de cabecera con las funciones para el LCD
#include "s1d15g00.h"
#include "font.h"		

// Frecuencia del cristal del LPC2138
#define Fpclk 19.6608e6*3/4


// Variables auxiliares
static unsigned int ADCresult[8];	// Array para la lectura de los sensores de línea
unsigned long r, ch;				// Variables para guardar los datos del ADC
char cadena[30];					// Variable para guardar el texto a imprimir en el LCD
int i = 0;							// Variable para la escritura en el LCD

/********************** FUNCIONES DE CONTROL **********************/
// Función que realiza la lectura de los sensores de línea
void ADC0(void)__irq
{
	// Se toman los valores de los sensores y se guardan en un array
	r = AD0DR;

	// Selección del canal convertido en ADC0
	ch = (r>>24)&0x07;

	// Guardar resultado de la conversión
	ADCresult[ch] = (r>>6)&0x03FF;
	VICVectAddr = 0;
}

// Interrupción IRQ, que realiza lectura periódica de eventos externos
void TIMER0(void)__irq
{
	LCDClearScreen();	// Limpiar pantalla
	for(i = 0; i < 8; i++)
	{
		sprintf(cadena, "%d", ADCresult[i]);   
		LCDPutStr(cadena, (120-(i*10)), 5, SMALL, WHITE, BLACK);
	}
			
	T0IR |= 1<<2;		// Activar flag de interrupción del Match0.2
	VICVectAddr = 0;	 
}


/************************ CONFIGURACIONES ************************/
// Configuración del display
void config_LCD (void)
{
	LCD_BL_DIR();		// BL = Output
	LCD_CS_DIR();		// CS = Output
	LCD_SCLK_DIR();		// SCLK = Output
	LCD_SDATA_DIR();	// SDATA = Output
	LCD_RESET_DIR();	// RESET = Output
	LCD_SCLK_LOW();		// Standby SCLK
	LCD_CS_HIGH();		// Disable CS
	LCD_SDATA_HIGH();	// Standby SDATA
	LCD_BL_HIGH();		// Black Light ON = 100%

	InitLcd(); 			// Initial LCD
	LCDClearScreen();	// Limpiar pantalla
}

// Configuración del ADC0 para los sensores de línea
void config_ADC0 (void)
{
	PINSEL0 |= (1<<8)|(1<<9)|(1<<10)|(1<<11);		// Habilitar ADC0[6,7]
	PINSEL1 |= (1<<18)|(1<<20)|(1<<22)|(1<<24)|(1<<26)|(1<<28);	// Habilitar ADC0[0..5]

	VICVectAddr0 = (unsigned long) ADC0;
	VICVectCntl0 = (0x20|18);
	VICIntEnable |= (1<<18);

	AD0CR = 0x2103FF;	// Activación de la conversión en modo burst para los 8 canales de ADC0
}

// Configuración del Match0.2
void config_TIMER0 (void)
{
	PINSEL1 |= 1<<1;
	T0TCR = 0x02;
	T0PR = 0;
	T0MCR = (1<<6)|(1<<7);
	T0MR2 = (0.5 * Fpclk);
	VICVectAddr1 = (unsigned long) TIMER0;
	VICVectCntl1 = (0x20|4);
	T0TCR = 0x01;
	VICIntEnable |= 1<<4;
}	


/************************** FUNCIÓN MAIN **************************/
int main (void)
{
	// Configuraciones previas a la iniciación de las respectivas funciones
	config_LCD();
	config_ADC0();
	config_TIMER0();

	while(1) // Bucle infinito para la ejecución del programa
	{
	}
}
