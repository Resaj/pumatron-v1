#include <lpc213x.h>
	   
// Ficheros de cabecera con las funciones para el LCD
//#include "s1d15g00.h"
//#include "font.h"		

// Frecuencia del cristal del LPC2138
#define Fpclk 19.6608e6*3/4

// Variables auxiliares
static unsigned int ADCresult[8];	// Array para la lectura de los sensores de línea
unsigned long r, ch;				// Variables para guardar los datos del ADC
//char cadena[30];					// Variable para guardar el texto a imprimir en el LCD
int i = 0;							// Variable para la escritura en el LCD
int distancia[8];					// Array para guardar la conversión de tensión a mm.
int channel;						// Variable para seleccionar el canal

/********************** FUNCIONES DE CONTROL **********************/
// Función que realiza la lectura de los sensores de distancia
void ADC1(void)__irq
{
	// Se toman los valores de los sensores y se guardan en un array
	r = AD1DR;

	// Selección del canal convertido en ADC1
	ch = (r>>24)&0x07;

	// Guardar resultado de la conversión
	ADCresult[ch] = (r>>6)&0x03FF;

	VICVectAddr = 0;
}

// Función que realiza la conversión de tensión de los sensores a distancia	GP2D120
void GP2D120_read(void)
{
	distancia[channel] = 15.1*((2914.0 / (ADCresult[channel] + 10) - 0.5)); // Fórmula de conversión de tensión a mm.

	if (distancia[channel] > 350)
		distancia[channel] = 350; //saturamos
	else if (distancia[channel] < 40)
		distancia[channel] = 40; //saturamos
}

// Función que realiza la conversión de tensión de los sensores a distancia	GP2D12
void GP2D12_read(void)
{
	distancia[channel] = 21.6*((3000 / (ADCresult[channel] - 53) + 0.3)); // Fórmula de conversión de tensión a mm.

	if (distancia[channel] > 800)
		distancia[channel] = 800; //saturamos
	else if (distancia[channel] < 100)
		distancia[channel] = 100; //saturamos
}

// Interrupción IRQ, que realiza lectura periódica de eventos externos
void TIMER0(void)__irq
{
	//LCDClearScreen();	// Limpiar pantalla

	channel = 0;
	GP2D120_read();
//	sprintf(cadena, "%4d", distancia[0]);   
//	LCDPutStr(cadena, 80, 100, SMALL, WHITE, BLACK);
//
	channel = 3;
	GP2D120_read();
//	sprintf(cadena, "%4d", distancia[3]);   
//	LCDPutStr(cadena, 80, 5, SMALL, WHITE, BLACK);
//
	channel = 7;
	GP2D12_read();
//	sprintf(cadena, "%4d", distancia[7]);   
//	LCDPutStr(cadena, 20, 55, SMALL, WHITE, BLACK);
	
	if(distancia[0] > 110)
	{
		IO0PIN |= (1<<23);
		IO0PIN &= ~(1<<20);
		IO1PIN |= (1<<31);
		IO1PIN &= ~(1<<30);
		PWMMR2 = 0.4 * 0.001*Fpclk;
		PWMMR4 = 0.5 * 0.001*Fpclk;
	}
	else
	{
		IO0PIN |= (1<<23);
		IO0PIN &= ~(1<<20);
		IO1PIN |= (1<<31);
		IO1PIN &= ~(1<<30);
		PWMMR2 = 0.5 * 0.001*Fpclk;
		PWMMR4 = 0.4 * 0.001*Fpclk;
	}

	PWMLER = (1<<0)|(1<<2)|(1<<4);	// Enable PWM Match 0, 2 & 4 Latch
		
	T0IR |= 1<<2;		// Activar flag de interrupción del Match0.2
	VICVectAddr = 0;	 
}


/************************ CONFIGURACIONES ************************/
// Configuración del display
//void config_LCD (void)
//{
//	LCD_BL_DIR();		// BL = Output
//	LCD_CS_DIR();		// CS = Output
//	LCD_SCLK_DIR();		// SCLK = Output
//	LCD_SDATA_DIR();	// SDATA = Output
//	LCD_RESET_DIR();	// RESET = Output
//	LCD_SCLK_LOW();		// Standby SCLK
//	LCD_CS_HIGH();		// Disable CS
//	LCD_SDATA_HIGH();	// Standby SDATA
//	LCD_BL_HIGH();		// Black Light ON = 100%
//
//	InitLcd(); 			// Initial LCD
//	LCDClearScreen();	// Limpiar pantalla
//}

// Configuración del ADC1 para los sensores de distancia
void config_ADC1 (void)
{
	PINSEL0 |= (3<<12)|(3<<24)/*|(3<<26)|(3<<30)*/;		// Habilitar ADC1[0,3,4,5]
	PINSEL1 |= (1<<12);								// Habilitar ADC1[7]

	VICVectAddr0 = (unsigned long) ADC1;
	VICVectCntl0 = (0x20|21);
	VICIntEnable |= (1<<21);

	//AD1CR = 0x2103B9;	// Activación de la conversión para los canales 0, 3, 4, 5 y 7
	AD1CR = 0x210389;
}

// Configuración de las salidas de PWM para los motores
void config_MOTORES (void)
{
	PINSEL0 |= (1<<15)|(1<<17);		// PWM2 y PWM4
	IO0DIR |= (1<<20)|(1<<23);		// Salidas para el sentido de los motores
	IO1DIR |= (1<<30)|(1<<31);
	PWMTCR = 0x02;					// Reset TIMER y PREESCALER										   
	PWMPR = 0;						// El PRESCALER no modifica la frecuencia
	PWMMR0 = 0.001 * Fpclk;			// Periodo PWM										   
	PWMLER = (1<<0)|(1<<2)|(1<<4);	// Habilitar MATCH0, MATCH2 y MATCH4
	PWMMCR = 0x02;					// Reset TIMER COUNTER REGISTER ON MATCH0							   
	PWMPCR = (1<<10)|(1<<12);		// Habilitar PWM2 y PWM4 en flanco
	PWMTCR = (1<<0)|(1<<3);			// Habilitar PWM y comenzar la cuenta
}
  
// Configuración del Match0.2
void config_TIMER0 (void)
{
	PINSEL1 |= 1<<1;
	T0TCR = 0x02;
	T0PR = 0;
	T0MCR = (1<<6)|(1<<7);
	T0MR2 = 0.3 * Fpclk;
	VICVectAddr1 = (unsigned long) TIMER0;
	VICVectCntl1 = (0x20|4);
	T0TCR = 0x01;
	VICIntEnable |= 1<<4;
}	


/************************** FUNCIÓN MAIN **************************/
int main (void)
{
	// Configuraciones previas a la iniciación de las respectivas funciones
	config_MOTORES();
	//config_LCD();
	config_ADC1();
	config_TIMER0();

	while(1) // Bucle infinito para la ejecución del programa
	{
	}
}
