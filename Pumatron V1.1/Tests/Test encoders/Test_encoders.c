#include <lpc213x.h>
#include <stdio.h>
	   
// Ficheros de cabecera con las funciones para el LCD
#include "s1d15g00.h"
#include "font.h"		

// Frecuencia del cristal del LPC2138
#define Fpclk 19.6608e6*3/4

// Variables auxiliares
char cadena[30];	// Variable para guardar el texto a imprimir en el LCD
int i = 0;			// Variable para la escritura en el LCD
int encoder[2];		// Variable para guardar el número de cuentas de los capture


/********************** FUNCIONES DE CONTROL **********************/
// Interrupción IRQ, que realiza lectura periódica de eventos externos
void TIMER0(void)__irq
{
	//LCDClearScreen();	// Limpiar pantalla
	for(i = 0; i < 2; i++)
	{
		sprintf(cadena, "%d", encoder[i]);   
		LCDPutStr(cadena, (100-(i*20)), 5, SMALL, WHITE, BLACK);
	}
			
	T0IR |= 1<<2;		// Activar flag de interrupción del Match0.2
	VICVectAddr = 0;	 
}

// Interrupción para el capture1.2 y capture1.3
void TIMER1(void)__irq
{
	if(T1IR&(1<<4))
	{
	 	encoder[0]++;
		T1IR |= 1<<0;		// Activar flag de interrupción del Capture1.0
	}
	else if(T1IR&(1<<6))
	{
		encoder[1]++;
		T1IR |= 1<<2;		// Activar flag de interrupción del Capture1.2
	}

	VICVectAddr = 0;	 
}


/************************ CONFIGURACIONES ************************/
// Configuración del display
void config_LCD(void)
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
   
// Configuración del Match0.2
void config_TIMER0(void)
{
	PINSEL1 |= 1<<1;
	T0TCR = 0x02;
	T0PR = 0;
	T0MCR = (1<<6)|(1<<7);
	T0MR2 = (100e-3 * Fpclk);
	VICVectAddr1 = (unsigned long) TIMER0;
	VICVectCntl1 = (0x20|4);
	T0TCR = 0x01;
	VICIntEnable |= 1<<4;
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


/************************** FUNCIÓN MAIN **************************/
int main (void)
{
	// Configuraciones previas a la iniciación de las respectivas funciones
	config_LCD();
	config_TIMER1();
	config_TIMER0();

	while(1) // Bucle infinito para la ejecución del programa
	{
	}
}
