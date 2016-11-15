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


/********************** FUNCIONES DE CONTROL **********************/
// Interrupción IRQ, que realiza lectura periódica de eventos externos
void TIMER0(void)__irq
{
	LCDClearScreen();	// Limpiar pantalla
	LCDPutStr(cadena, 80, 5, SMALL, WHITE, BLACK);
	sprintf(cadena, "null");
				
	T0IR |= 1<<2;		// Activar flag de interrupción del Match0.2
	VICVectAddr = 0;	 
}

// Interrupción IRQ asociada a EINT1
void EXTERNA1(void)__irq
{
	sprintf(cadena, "EINT1");

	EXTINT |= 1<<1;
	VICVectAddr = 0;
}

// Interrupción IRQ asociada a EINT3
void EXTERNA3(void)__irq
{
	sprintf(cadena, "EINT3");

	EXTINT |= 1<<3;
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
	T0MR2 = (1 * Fpclk);
	T0TCR = 0x01;
	VICVectAddr2 = (unsigned long) TIMER0;
	VICVectCntl2 = (0x20|4);
	VICIntEnable |= 1<<4;
}

// Configuración de EINT1
void config_EINT1(void)
{
	PINSEL0 |= 3<<6;
	EXTMODE |= 1<<1;
	EXTPOLAR &= ~(1<<1);
	VICVectAddr0 = (unsigned long) EXTERNA1;
	VICVectCntl0 = (0x20|15);
	VICIntEnable |= 1<<15;
}

// Configuración de EINT3
void config_EINT3(void)
{
	PINSEL0 |= 3<<18;
	EXTMODE = 1<<3;
	EXTPOLAR &= ~(1<<3);
	VICVectAddr1 = (unsigned long) EXTERNA3;
	VICVectCntl1 = (0x20|17);
	VICIntEnable |= 1<<17;
}


/************************** FUNCIÓN MAIN **************************/
int main (void)
{
	// Configuraciones previas a la iniciación de las respectivas funciones
	config_LCD();
	config_TIMER0();
	config_EINT1();
	config_EINT3();

	while(1) // Bucle infinito para la ejecución del programa
	{
	}
}
