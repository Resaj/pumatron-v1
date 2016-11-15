#include <lpc213x.h>
#include <stdio.h>
	   
// Ficheros de cabecera con las funciones para el LCD
#include "s1d15g00.h"
#include "font.h"		

// Frecuencia del cristal del LPC2138
#define Fpclk 19.6608e6*3/4


// Variables auxiliares
static unsigned int ADCresult[12];	// Array para la lectura de los sensores de línea
unsigned long r, ch;				// Variables para guardar los datos del ADC
char cadena[30];					// Variable para guardar el texto a imprimir en el LCD
int i = 0;							// Variable para la escritura en el LCD
int mux = 0;

/********************** FUNCIONES DE CONTROL **********************/
// Función que realiza la lectura de los sensores de línea
void ADC0(void)__irq
{
	// Se toman los valores de los sensores y se guardan en un array
	r = AD0DR;

	// Selección del canal convertido en ADC0
	ch = (r>>24)&0x07;

	// Guardar resultado de la conversión
	if(mux == 0)
	{
		if(ch == 1)
			ADCresult[7] = (r>>6)&0x03FF;
		else if(ch == 2)
			ADCresult[9] = (r>>6)&0x03FF;
		else if(ch == 3)
			ADCresult[11] = (r>>6)&0x03FF;
		else if(ch == 5)
			ADCresult[5] = (r>>6)&0x03FF;
		else if(ch == 6)
			ADCresult[3] = (r>>6)&0x03FF;
		else if(ch == 7)
			ADCresult[1] = (r>>6)&0x03FF;
	}
	else
	{
		if(ch == 1)
			ADCresult[6] = (r>>6)&0x03FF;
		else if(ch == 2)
			ADCresult[8] = (r>>6)&0x03FF;
		else if(ch == 3)
			ADCresult[10] = (r>>6)&0x03FF;
		else if(ch == 5)
			ADCresult[4] = (r>>6)&0x03FF;
		else if(ch == 6)
			ADCresult[2] = (r>>6)&0x03FF;
		else if(ch == 7)
			ADCresult[0] = (r>>6)&0x03FF;
	}

	VICVectAddr = 0;
}

// Interrupción IRQ, que realiza lectura periódica de eventos externos
void TIMER0(void)__irq
{
	//LCDClearScreen();	// Limpiar pantalla
	for(i = 0; i < 12; i++)
	{
		sprintf(cadena, "%4d", ADCresult[i]);   
		LCDPutStr(cadena, (120-(i*10)), 40, SMALL, WHITE, BLACK);
		if(ADCresult[i] > 800)
			LCDSetRect((120-(i*10)), 25, (125-(i*10)), 30, 1, RED);
		else
			LCDSetRect((120-(i*10)), 25, (125-(i*10)), 30, 1, WHITE);
	}
	
	if(mux == 0)
	{
		mux = 1;
		IO0PIN |= (1<<25)|(1<<27);
		IO1PIN &= ~(1<<23);
	}
	else
	{
		mux = 0;
		IO0PIN &= ~(1<<25);
		IO0PIN &= ~(1<<27);
		IO1PIN |= 1<<23;
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
	PINSEL1 |= (1<<20)|(1<<24)|(1<<26)|(1<<28);		// Habilitar ADC0[1,2,3,5]
	IO0DIR |= 1<<25; 								// Salida del multiplexor

	VICVectAddr0 = (unsigned long) ADC0;
	VICVectCntl0 = (0x20|18);
	VICIntEnable |= (1<<18);

	AD0CR = 0x2103EE;	// Activación de la conversión en modo burst
}

// Configuración del Match0.2
void config_TIMER0 (void)
{
	PINSEL1 |= 1<<1;
	T0TCR = 0x02;
	T0PR = 0;
	T0MCR = (1<<6)|(1<<7);
	T0MR2 = 0.1 * Fpclk;
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

	for(i = 0; i < 12; i++)
	{
		sprintf(cadena, "%d:", i);   
		LCDPutStr(cadena, (120-(i*10)), 5, SMALL, WHITE, BLACK);
	}

	config_ADC0();
	config_TIMER0();
	IO0DIR |= 1<<27;	// Led
	IO1DIR |= 1<<23;	// Led

	while(1) // Bucle infinito para la ejecución del programa
	{
	}
}
