#include <lpc213x.h>

// Frecuencia del cristal del LPC2138
#define Fpclk 19.6608e6*3/4

// Estados para implementar la máquina de estados
#define Parado 0
#define Iniciar 1
#define Avanzar 2
#define Rastrear 3
#define Obstaculo 4
char ESTADO = Parado;
#define IZQ	7
#define DER 8

float Per_timer = 0.001;								 
float Per_pwm = 0.001;

// Variables auxiliares 
char cadena[30];		// Variable auxiliar para guardar el texto a imprimir en el LCD
unsigned int i = 0;					// Variable para la escritura en el LCD
int flag_1 = 1;
int flag_2 = 1;

// Variables para el control del seguimiento de la línea
static unsigned int ADCresult[8];		// Array para la lectura de los sensores de línea
static unsigned int LINEAresult[8];			// Array que contiene los valores digitalizados de ADCresult
static unsigned int distancia[8];			// Array que contiene los valores de los sensores de distancia
int giro = 0;								// Variable auxiliar para no perder la línea
unsigned long r, ch;						// Variables para guardar los datos del ADC
float velocidad = 0.7;						// Variable para regular la velocidad. De 0 a 1
int sensor_frontal = 0;
int channel = 0;
int umbral = 800;							// Umbral para diferenciar el blanco del negro de la línea
int LINEA = 0;
int cont = 0;
int nSensores = 0;

/********************** FUNCIONES DE CONTROL **********************/
void seguir_linea(void)
{
	if(LINEAresult[4] == 1 && LINEAresult[3] == 1)	// Secuencia 00011000
	{
		IO0PIN |= (1<<23);
		IO0PIN &= ~(1<<20);
		IO1PIN |= (1<<31);
		IO1PIN &= ~(1<<30);
		PWMMR2 = velocidad*Per_pwm*Fpclk;
		PWMMR4 = velocidad*Per_pwm*Fpclk;
	}
	else if(LINEAresult[5] == 1 && LINEAresult[4] == 1)	// Secuencia 00011000
	{
		IO0PIN |= (1<<23);
		IO0PIN &= ~(1<<20);
		IO1PIN |= (1<<31);
		IO1PIN &= ~(1<<30);
		PWMMR2 = 0.8 * velocidad*Per_pwm*Fpclk;
		PWMMR4 = velocidad*Per_pwm*Fpclk;
		giro = IZQ;
	}
	else if(LINEAresult[3] == 1 && LINEAresult[2] == 1)	// Secuencia 00001100
	{
		IO0PIN |= (1<<23);
		IO0PIN &= ~(1<<20);
		IO1PIN |= (1<<31);
		IO1PIN &= ~(1<<30);
		PWMMR2 = velocidad*Per_pwm*Fpclk;
		PWMMR4 = 0.8 * velocidad*Per_pwm*Fpclk;
		giro = DER;
	}
	else if(LINEAresult[4] == 1)					// Secuencia 00010000
	{
		IO0PIN |= (1<<23);
		IO0PIN &= ~(1<<20);
		IO1PIN |= (1<<31);
		IO1PIN &= ~(1<<30);
		PWMMR2 = 0.9 * velocidad*Per_pwm*Fpclk;
		PWMMR4 = velocidad*Per_pwm*Fpclk;
		giro = IZQ;
	}
	else if(LINEAresult[3] == 1)					// Secuencia 00001000
	{
		IO0PIN |= (1<<23);
		IO0PIN &= ~(1<<20);
		IO1PIN |= (1<<31);
		IO1PIN &= ~(1<<30);
		PWMMR2 = velocidad*Per_pwm*Fpclk;
		PWMMR4 = 0.9 * velocidad*Per_pwm*Fpclk;
		giro = DER;
	}
	else if(LINEAresult[6] == 1 && LINEAresult[5] == 1)	// Secuencia 00110000
	{
		IO0PIN |= (1<<23);
		IO0PIN &= ~(1<<20);
		IO1PIN |= (1<<31);
		IO1PIN &= ~(1<<30);
		PWMMR2 = 0.2 * velocidad*Per_pwm*Fpclk;
		PWMMR4 = velocidad*Per_pwm*Fpclk;
		giro = IZQ;
	}
	else if(LINEAresult[2] == 1 && LINEAresult[1] == 1)	// Secuencia 00000110
	{
		IO0PIN |= (1<<23);
		IO0PIN &= ~(1<<20);
		IO1PIN |= (1<<31);
		IO1PIN &= ~(1<<30);
		PWMMR2 = velocidad*Per_pwm*Fpclk;
		PWMMR4 = 0.2 * velocidad*Per_pwm*Fpclk;
		giro = DER;	  			
	}
	else if(LINEAresult[5] == 1)					// Secuencia 00100000
	{			  		
		IO0PIN |= (1<<23);
		IO0PIN &= ~(1<<20);
		IO1PIN |= (1<<31);
		IO1PIN &= ~(1<<30);
		PWMMR2 = 0.5 * velocidad*Per_pwm*Fpclk;
		PWMMR4 = velocidad*Per_pwm*Fpclk;
		giro = IZQ;
	}
	else if(LINEAresult[2] == 1)					// Secuencia 00000100
	{	   		
		IO0PIN |= (1<<23);
		IO0PIN &= ~(1<<20);
		IO1PIN |= (1<<31);
		IO1PIN &= ~(1<<30);
		PWMMR2 = velocidad*Per_pwm*Fpclk;
		PWMMR4 = 0.5 * velocidad*Per_pwm*Fpclk;
		giro = DER;
	}
	else if(LINEAresult[6] == 1)					// Secuencia 01000000
	{		 		
		IO0PIN |= (1<<23);
		IO0PIN &= ~(1<<20);
		IO1PIN |= (1<<31);
		IO1PIN &= ~(1<<30);
		PWMMR2 = 0 * velocidad*Per_pwm*Fpclk;
		PWMMR4 = velocidad*Per_pwm*Fpclk;
		giro = IZQ;
	}
	else if(LINEAresult[1] == 1)							// Secuencia 00000010
	{					  		
		IO0PIN |= (1<<23);
		IO0PIN &= ~(1<<20);
		IO1PIN |= (1<<31);
		IO1PIN &= ~(1<<30);
		PWMMR2 = velocidad*Per_pwm*Fpclk;
		PWMMR4 = 0 * velocidad*Per_pwm*Fpclk;
		giro = DER;
	}
	else											// Secuencia 00000000
	{
		if(giro == IZQ)	// Si la secuencia anterior es 10000000
		{				  			
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<30);
			IO1PIN &= ~(1<<31);
			PWMMR2 = velocidad*Per_pwm*Fpclk;
			PWMMR4 = velocidad*Per_pwm*Fpclk;
		}
		else if(giro == DER) // Si la secuencia anterior es 00000001
		{	 			
			IO0PIN |= (1<<20);
			IO0PIN &= ~(1<<23);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = velocidad*Per_pwm*Fpclk;
			PWMMR4 = velocidad*Per_pwm*Fpclk;
		}
		else
		{
			IO0PIN |= (1<<20);
			IO0PIN &= ~(1<<23);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = velocidad*Per_pwm*Fpclk;
			PWMMR4 = velocidad*Per_pwm*Fpclk;
		}
	}
}

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

// Función que realiza la conversión de tensión de los sensores a distancia	GP2D120
void GP2D120_read(void)
{
	distancia[channel] = 15.1*((2914.0 / (ADCresult[channel] + 10) - 0.5)); // Fórmula de conversión de tensión a mm.

	if (distancia[channel] > 350)
		distancia[channel] = 350; //saturamos
	else if (distancia[channel] < 40)
		distancia[channel] = 40; //saturamos
}

void leer_sensores(void)
{
	channel = 7;
	GP2D120_read();
	sensor_frontal = distancia[7];
}

// Interrupción IRQ para el TIMER0
void TIMER0(void)__irq
{
	if(ESTADO != Parado)
	{
		// Digitalizar el resultado
		nSensores = 0;
		for(i=1; i<7; i++)
		{
			if(ADCresult[i] > umbral)
			{
				LINEAresult[i] = 1;
				nSensores++;
			}
			else
				LINEAresult[i] = 0;
		}

		leer_sensores();

		// Cambios de estado
		if(ESTADO == Iniciar)
		{
			if(nSensores != 0)
				ESTADO = Rastrear;
		}
		else if(ESTADO == Avanzar)
		{
			if((LINEA == IZQ && (LINEAresult[0] == 1 || LINEAresult[1] == 1)) || (LINEA == DER && (LINEAresult[6] == 1 || LINEAresult[7] == 1)))
			{
				if(LINEA == IZQ)
					LINEA = DER;
				else // LINEA == DER
					LINEA = IZQ;
				ESTADO = Rastrear;
			}
			cont = 0;
		}
		else if(ESTADO == Rastrear)
		{
			if(sensor_frontal < 300)
				cont++;
			else
				cont = 0;

			if(cont*Per_timer > 0.1)
			{
				ESTADO = Obstaculo;
				cont = 0;
			}
		}
		else // ESTADO == Obstaculo
		{
			if(sensor_frontal > 300)
				cont++;
			else
				cont = 0;

			if(cont*Per_timer > 0.01)
				ESTADO = Avanzar;
		}

		// Ejecución del seguimiento
		if(ESTADO == Iniciar)
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			if(LINEA == IZQ)
			{
				PWMMR2 = 0.5 * velocidad * Per_pwm*Fpclk;
				PWMMR4 = velocidad * Per_pwm*Fpclk;
			}
			else // LINEA == DER
			{
				PWMMR2 = velocidad * Per_pwm*Fpclk;
				PWMMR4 = 0.5 * velocidad * Per_pwm*Fpclk;
			}
			cont++;
		}
		else if(ESTADO == Avanzar)
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = velocidad * Per_pwm*Fpclk;
			PWMMR4 = velocidad * Per_pwm*Fpclk;
		}
		else if(ESTADO == Rastrear)
			seguir_linea();			
		else // ESTADO == Obstaculo
		{
			if(LINEA == IZQ)
			{
				IO0PIN |= (1<<20);
				IO0PIN &= ~(1<<23);
				IO1PIN |= (1<<31);
				IO1PIN &= ~(1<<30);
				PWMMR2 = velocidad * Per_pwm*Fpclk;
				PWMMR4 = velocidad * Per_pwm*Fpclk;
			}
			else // LINEA == DER
			{
				IO0PIN |= (1<<23);
				IO0PIN &= ~(1<<20);
				IO1PIN |= (1<<30);
				IO1PIN &= ~(1<<31);
				PWMMR2 = velocidad * Per_pwm*Fpclk;
				PWMMR4 = velocidad * Per_pwm*Fpclk;
			}
		}

		PWMLER = (1<<0)|(1<<2)|(1<<4);	// Enable PWM Match 0, 2 & 4 Latch
	}

	T0IR |= (1<<2);		// Activar flag de interrupción del Match0.2
	VICVectAddr = 0;
}

// Interrupción IRQ asociada a EINT1
void EXTERNA1(void)__irq
{
	if(ESTADO == Parado)
	{
		if((IO1PIN&(1<<21)) == 0)
			LINEA = IZQ;
		else
			LINEA = DER;

		ESTADO = Iniciar;
		cont = 0;
	}

	EXTINT |= 1<<1;
	VICVectAddr = 0;
}

// Interrupción IRQ asociada a EINT3
void EXTERNA3(void)__irq
{
	if(ESTADO != Parado)
	{
		PWMMR2 = 0;
		PWMMR4 = 0;
		PWMLER = (1<<0)|(1<<2)|(1<<4);	// Enable PWM Match 0, 2 & 4 Latch
		ESTADO = Parado;
	}

	EXTINT |= 1<<3;
	VICVectAddr = 0;
}


/************************ CONFIGURACIONES ************************/
// Configuración del ADC0 para los sensores de línea
void config_ADC0 (void)
{
	PINSEL0 |= (1<<8)|(1<<9)|(1<<10)|(1<<11);		// Habilitar ADC0[6,7]
	PINSEL1 |= (1<<18)|(1<<20)|(1<<22)|(1<<24)|(1<<26)|(1<<28);	// Habilitar ADC0[0..5]

	VICVectAddr0 = (unsigned long) ADC0;
	VICVectCntl0 = (0x20|18);
	VICIntEnable |= (1<<18);

	AD0CR = 0x2103FE;	// Activación de la conversión en modo burst para los 8 canales de ADC0
}

// Configuración del Match0.2 del TIMER0
void config_TIMER0 (void)
{
	PINSEL1 |= 1<<1;
	T0TCR = 0x02;
	T0PR = 0;
	T0MCR = (1<<6)|(1<<7);
	T0MR2 = Per_timer * Fpclk;
	VICVectAddr1 = (unsigned long) TIMER0;
	VICVectCntl1 = (0x20|4);
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
	PWMMR0 = Per_pwm * Fpclk;		// Periodo PWM										   
	PWMLER = (1<<0)|(1<<2)|(1<<4);	// Habilitar MATCH0, MATCH2 y MATCH4
	PWMMCR = 0x02;					// Reset TIMER COUNTER REGISTER ON MATCH0							   
	PWMPCR = (1<<10)|(1<<12);		// Habilitar PWM2 y PWM4 en flanco
	PWMTCR = (1<<0)|(1<<3);			// Habilitar PWM y comenzar la cuenta
}
 
// Configuración de EINT1
void config_EINT1(void)
{
	PINSEL0 |= 1<<29; //3<<6;
	EXTMODE |= 1<<1;
	EXTPOLAR &= ~(1<<1);
	VICVectAddr2 = (unsigned long) EXTERNA1;
	VICVectCntl2 = (0x20|15);
	VICIntEnable |= 1<<15;
}

// Configuración de EINT3
void config_EINT3(void)
{
	PINSEL0 |= 3<<18;
	EXTMODE = 1<<3;
	EXTPOLAR &= ~(1<<3);
	VICVectAddr3 = (unsigned long) EXTERNA3;
	VICVectCntl3 = (0x20|17);
	VICIntEnable |= 1<<17;
}


/************************** FUNCIÓN MAIN **************************/
int main (void)
{
	// Selección de velocidad de los sensores
	if((IO1PIN&(1<<21)) == 0)
		LINEA = IZQ;
	else
		LINEA = DER;

	// Configuraciones previas
	config_EINT1();
	config_EINT3();
	config_MOTORES();
	config_ADC0();
	config_TIMER0();

	while(1) // Bucle infinito para la ejecución del programa
	{
	}
}
