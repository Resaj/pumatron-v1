#include <lpc213x.h>
#include <stdio.h>
	   
// Ficheros de cabecera con las funciones para el LCD
#include "s1d15g00.h"
#include "font.h"		

// Frecuencia del cristal del LPC2138
#define Fpclk 19.6608e6*3/4

// Velocidad de la UART0
#define baudrate 9600

#define SI 1
#define NO 0
#define N_MENSAJES 8


// Variables auxiliares
char cadena[30];	// Variable para guardar el texto a imprimir en el LCD
int Fdiv = 0;		// Variable para el cálculo de los parámetros de la configuración de la UART0

char * buffer_tx[N_MENSAJES];
char ptr_rd = 0, ptr_wr = 0, reposo = 1;
char mensajes_ptes = 0;		// Indica si quedan mensajes por enviar

//mensajes para el puerto serie
char error[] = "Comando erroneo, vea la ayuda \n\r  ";
char hola[] = "\nBienvenido al programa \n\r  ";
char comando1[] = "Comando i aceptado \n\r  ";
char comando2[] = "Comando d aceptado \n\r  ";
char comando3[] = "Comando + aceptado \n\r  ";
char comando4[] = "Comando - aceptado \n\r  ";
char prompt[] = "\r:>";
char vacio[] = " ";
char cero = 0;
char *ptr_tx = &cero;


/********************** FUNCIONES DE CONTROL **********************/
// Función para transmitir un mensaje por la UART0
void ins_mensaje(char * mensaje)
{
	buffer_tx[ptr_wr] = mensaje;	// Se añade al buffer_tx el puntero al mensaje
	ptr_wr++; 						// Se gestiona el ptr_wr de forma circular.
	ptr_wr = ptr_wr&N_MENSAJES-1;	// Para mantener el modulo de cuenta.
	mensajes_ptes = 1;
	if (reposo)						// No quedaban caracteres pendientes de enviar
	{
		reposo = 0;
		U0THR = 0;
	}
}

// Interrupción IRQ asociada a la recepción de datos a través de la UART0
void UART0(void)__irq
{
	char dato;
	switch(U0IIR & 0x0E)
	{
		case 0x04:	// Recepción
			dato = U0RBR;
			sprintf(cadena, "%c", dato);   
			LCDPutStr(cadena, 100, 5, LARGE, WHITE, BLACK);

			switch(dato)
			{
				case 'h':
					ins_mensaje(comando1);
			 		ins_mensaje(comando2);
	 				ins_mensaje(comando3);
	 				ins_mensaje(comando4);
					break;
				default:
					ins_mensaje(error);
					break;
			}
			ins_mensaje(prompt);
			break;
		case 0x02:	// Transmisión
    		if(*ptr_tx == 0)
			{
				if(mensajes_ptes == SI) // Quedan mensajes por enviar
				{
					ptr_tx = buffer_tx[ptr_rd]; // Se busca el nuevo mensaje para enviar
					ptr_rd++;
					ptr_rd &= N_MENSAJES - 1;
					if(ptr_rd == ptr_wr)
						mensajes_ptes = NO;	// Se va a enviar el ultimo mensaje del buffer
				   	U0THR = *ptr_tx;
					ptr_tx++;
				}
				else
					reposo=1;
			}
			else	// Se continúa mandando el mensaje
				U0THR=*ptr_tx++;
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

	VICVectAddr0 = (unsigned long) UART0;
	VICVectCntl0 = 0x20|6;
	VICIntEnable = 1<<6;
}


/************************** FUNCIÓN MAIN **************************/
int main (void)
{
	// Configuraciones previas a la iniciación de las respectivas funciones
	config_LCD();
	config_UART0();

	while(1) // Bucle infinito para la ejecución del programa
	{
	}
}
