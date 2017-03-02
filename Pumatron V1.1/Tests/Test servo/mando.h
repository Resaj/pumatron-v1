//
// FICHERO DE CABECERA QUE INCLUYE LAS FUNCIONES NECESARIAS PARA EL EMPLEO DEL MANDO
//
//
// VIVANCO. UR 2 UNIVERSAL CONTROLLER
// Código: TV, Sony, 077
//
// CAPTURE1.3, P0.18
//


#include <lpc213x.h>
#include <stdio.h>


// Frecuencia del cristal del LPC2138
#define Fpclk 19.6608e6*3/4


// Codificación de los botones del mando
#define BUTTON_power      0x95	  // Secuencia: 4T 2T 1T 2T 1T 2T 1T 1T 2T 1T 1T 1T 1T // Código: 000010010101 //
#define BUTTON_teletext1  0xB8	  // Secuencia: 4T 1T 1T 1T 2T 2T 2T 1T 2T 1T 1T 1T 1T // Código: 000010111000 //
#define BUTTON_teletext2  0xBF	  // Secuencia: 4T 2T 2T 2T 2T 2T 2T 1T 2T 1T 1T 1T 1T // Código: 000010111111 //
#define BUTTON_teletext3  0x1CA	  // Secuencia: 4T 1T 2T 1T 2T 1T 1T 2T 2T 2T 1T 1T 1T // Código: 000111001010 //
#define BUTTON_teletext4  0x9D	  // Secuencia: 4T 2T 1T 2T 2T 2T 1T 1T 2T 1T 1T 1T 1T // Código: 000010011101 //
#define BUTTON_1		  0x80	  // Secuencia: 4T 1T 1T 1T 1T 1T 1T 1T 2T 1T 1T 1T 1T // Código: 000010000000 //
#define BUTTON_2		  0x81	  // Secuencia: 4T 2T 1T 1T 1T 1T 1T 1T 2T 1T 1T 1T 1T // Código: 000010000001 //
#define BUTTON_3		  0x82	  // Secuencia: 4T 1T 2T 1T 1T 1T 1T 1T 2T 1T 1T 1T 1T // Código: 000010000010 //
#define BUTTON_4		  0x83	  // Secuencia: 4T 2T 2T 1T 1T 1T 1T 1T 2T 1T 1T 1T 1T // Código: 000010000011 //
#define BUTTON_5		  0x84	  // Secuencia: 4T 1T 1T 2T 1T 1T 1T 1T 2T 1T 1T 1T 1T // Código: 000010000100 //
#define BUTTON_6		  0x85	  // Secuencia: 4T 2T 1T 2T 1T 1T 1T 1T 2T 1T 1T 1T 1T // Código: 000010000101 //
#define BUTTON_7		  0x86	  // Secuencia: 4T 1T 2T 2T 1T 1T 1T 1T 2T 1T 1T 1T 1T // Código: 000010000110 //
#define BUTTON_8		  0x87	  // Secuencia: 4T 2T 2T 2T 1T 1T 1T 1T 2T 1T 1T 1T 1T // Código: 000010000111 //
#define BUTTON_9		  0x88	  // Secuencia: 4T 1T 1T 1T 2T 1T 1T 1T 2T 1T 1T 1T 1T // Código: 000010001000 //
#define BUTTON_enter	  0x8C	  // Secuencia: 4T 1T 1T 2T 2T 1T 1T 1T 2T 1T 1T 1T 1T // Código: 000010001100 //
#define BUTTON_0		  0x89	  // Secuencia: 4T 2T 1T 1T 2T 1T 1T 1T 2T 1T 1T 1T 1T // Código: 000010001001 //
#define BUTTON_AB		  0xA5	  // Secuencia: 4T 2T 1T 2T 1T 1T 2T 1T 2T 1T 1T 1T 1T // Código: 000010100101 //
#define BUTTON_CH_plus	  0x90	  // Secuencia: 4T 1T 1T 1T 1T 2T 1T 1T 2T 1T 1T 1T 1T // Código: 000010010000 //
#define BUTTON_CH_minus	  0x91	  // Secuencia: 4T 2T 1T 1T 1T 2T 1T 1T 2T 1T 1T 1T 1T // Código: 000010010001 //
#define BUTTON_VOL_plus	  0x92	  // Secuencia: 4T 1T 2T 1T 1T 2T 1T 1T 2T 1T 1T 1T 1T // Código: 000010010010 //
#define BUTTON_VOL_minus  0x93	  // Secuencia: 4T 2T 2T 1T 1T 2T 1T 1T 2T 1T 1T 1T 1T // Código: 000010010011 //
#define BUTTON_mute		  0x94	  // Secuencia: 4T 1T 1T 2T 1T 2T 1T 1T 2T 1T 1T 1T 1T // Código: 000010010100 //
#define BUTTON_AV		  0xA5	  // Secuencia: 4T 2T 1T 2T 1T 1T 2T 1T 2T 1T 1T 1T 1T // Código: 000010100101 //
#define BUTTON_red		  0x1CC	  // Secuencia: 4T 1T 1T 2T 2T 1T 1T 2T 2T 2T 1T 1T 1T // Código: 000111001100 //
#define BUTTON_green	  0x1CD	  // Secuencia: 4T 2T 1T 2T 2T 1T 1T 2T 2T 2T 1T 1T 1T // Código: 000111001101 //
#define BUTTON_yellow	  0x1CE	  // Secuencia: 4T 1T 2T 2T 2T 1T 1T 2T 2T 2T 1T 1T 1T // Código: 000111001110 //
#define BUTTON_blue		  0x1CF	  // Secuencia: 4T 2T 2T 2T 2T 1T 1T 2T 2T 2T 1T 1T 1T // Código: 000111001111 //


// Variables auxiliares para decodificar la señal del mando
unsigned long dato = 0;				// Valor decodificado de la señal enviada por el mando
unsigned long captura_1, captura_2;	// Variables para tomar el número de cuentas en Capture0.0
int n, i, aux, start = 0, inicio = 0;		// Variables auxiliares para tomar la señal del mando
char cadena[30];					// Variable auxiliar para guardar el texto a imprimir en el LCD


// Interrupción IRQ, que detecta y decodifica la señal del mando
void mando(void)__irq
{
	double F = Fpclk;
  	captura_2 = T1CR3;	// Número de cuentas

	// Si la diferencia de cuentas entre los instantes 1 y 2 es de 3ms: bit de start
	if((((captura_2-captura_1)/F) > 2.8e-3) && ((((captura_2)-(captura_1))/F) < 3.2e-3))
	{
		start = 1;	// LLega el bit de start y comienza la secuencia
		n = -1;		// Bit de start con peso -1
		dato = 0;	// Se inicia el dato a 0
	}
	// Si la diferencia de cuentas entre los instantes 1 y 2 es de 1.8ms: bit del dato a 1
	if(start && (((captura_2-captura_1)/F > 1.6e-3) && (captura_2-captura_1)/F < 2.0e-3))
	{
		i = 1;
		aux = 1;	// Bit
		while( i <= n )	// Multiplicar bit por su peso en binario
		{
			aux *= 2;
			++i;
		} 
		dato += aux;	// Sumar bit decodificado al dato
	}
	n++;	// Aumentar el peso para el siguiente bit
	
	if(!captura_1)
		inicio = 1;		// Primer flanco 	
	if(n==11)		// Si ha llegado el último bit del dato
		T1CCR = 5<<9;	// Configurar captura en flanco de subida
	if(n==12)
	{
		// Si la diferencia de cuentas entre los instantes 1 y 2 es de 1.2ms: bit de stop
		if(start && (((captura_2-captura_1)/F > 1.0e-3) && (captura_2-captura_1)/F < 1.4e-3))
			dato += 2048;		//Captura último dato
		start = 0;		// Borrar variables
		inicio = 0;
		captura_1 = 0;											
		T1CCR = 6<<9;	// Configurar captura en flanco de bajada
	}			
	if(inicio)	// Si la toma del dato está en proceso:
		captura_1 = T1CR3;	// Preparar captura_1 para el siguiente bit

	T1IR |= (1<<7);		// Activar flag de interrupción del Capture1.3
	VICVectAddr = 0;
}


// Configuración de la entrada de captura de la señal del mando
void config_TIMER1(void)
{
	PINSEL1 |= 1<<4;
	T1TCR = 2;
	T1CTCR = 0;
	T1PR = 0;
	T1TCR = 1;
 	T1CCR = 6<<9;
	VICVectCntl0 = (0x20|5);
	VICVectAddr0 = (unsigned long) mando;
	VICIntEnable |= (1<<5);
}
