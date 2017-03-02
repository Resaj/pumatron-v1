#include <lpc213x.h>

// Frecuencia del cristal del LPC2138
#define Fpclk 19.6608e6*3/4

// Estados para implementar la máquina de estados
#define Parado 0
#define Rastrear 1
char ESTADO = Parado;	// La aplicación comienza en el estado "0"
#define IZQ	5
#define DER 6
								 
// Variables auxiliares 
static unsigned int ADCresult[8];	// Array para la lectura de los sensores de línea
int giro = DER;						// Variable auxiliar para no perder la línea
unsigned long r, ch;				// Variables para guardar los datos del ADC
float velocidad = 1;					// Variable para regular la velocidad. De 0 a 1
int umbral = 700;					// Umbral para diferenciar el blanco del negro de la línea
float  Per_pwm = 0.001;


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

// Interrupción IRQ para el TIMER0
void TIMER0(void)__irq
{
	if(ESTADO == Rastrear)
	{
		if(ADCresult[3] > umbral || ADCresult[4] > umbral)
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = velocidad * Per_pwm*Fpclk;
			PWMMR4 = velocidad * Per_pwm*Fpclk;
		}
		else if(ADCresult[3] > umbral)
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = velocidad * Per_pwm*Fpclk;
			PWMMR4 = velocidad * 0.9 * Per_pwm*Fpclk;
		}
		else if(ADCresult[4] > umbral)
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = velocidad * 0.9 * Per_pwm*Fpclk;
			PWMMR4 = velocidad * Per_pwm*Fpclk;
		}
		else if(ADCresult[2] > umbral)
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = velocidad * Per_pwm*Fpclk;
			PWMMR4 = velocidad * 0.7 * Per_pwm*Fpclk;
			giro = DER;
		}
		else if(ADCresult[5] > umbral)
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = velocidad * 0.7 * Per_pwm*Fpclk;
			PWMMR4 = velocidad * Per_pwm*Fpclk;
			giro = IZQ;
		}
		else if(ADCresult[1] > umbral)
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = velocidad * Per_pwm*Fpclk;
			PWMMR4 = velocidad * 0.4 * Per_pwm*Fpclk;
			giro = DER;
		}
		else if(ADCresult[6] > umbral)
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = velocidad * 0.4 * Per_pwm*Fpclk;
			PWMMR4 = velocidad * Per_pwm*Fpclk;
			giro = IZQ;
		}
		else if(ADCresult[0] > umbral)
		{
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = velocidad * Per_pwm*Fpclk;
			PWMMR4 = velocidad * 0.2 * Per_pwm*Fpclk;
			giro = DER;
		}
		else if(ADCresult[7] > umbral)
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			PWMMR2 = velocidad * 0.2 * Per_pwm*Fpclk;
			PWMMR4 = velocidad * Per_pwm*Fpclk;
			giro = IZQ;
		}
		else
		{
			if(giro == DER)
			{
				IO0PIN |= (1<<20);
				IO0PIN &= ~(1<<23);
				IO1PIN |= (1<<31);
				IO1PIN &= ~(1<<30);
				PWMMR2 = velocidad * Per_pwm*Fpclk;
				PWMMR4 = velocidad * 0.2 * Per_pwm*Fpclk;
			}
			else
			{
				IO0PIN |= (1<<23);
				IO0PIN &= ~(1<<20);
				IO1PIN |= (1<<30);
				IO1PIN &= ~(1<<31);
				PWMMR2 = velocidad * 0.2 * Per_pwm*Fpclk;
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
	ESTADO = Rastrear;

	EXTINT |= 1<<1;
	VICVectAddr = 0;
}

// Interrupción IRQ asociada a EINT3
void EXTERNA3(void)__irq
{
	PWMMR2 = 0;
	PWMMR4 = 0;
	PWMLER = (1<<0)|(1<<2)|(1<<4);	// Enable PWM Match 0, 2 & 4 Latch

	ESTADO = Parado;

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

	AD0CR = 0x2103FF;	// Activación de la conversión en modo burst para los 8 canales de ADC0
}

// Configuración del Match0.2 del TIMER0
void config_TIMER0 (void)
{
	PINSEL1 |= 1<<1;
	T0TCR = 0x02;
	T0PR = 0;
	T0MCR = (1<<6)|(1<<7);
	T0MR2 = 0.005 * Fpclk;
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
	PWMMR0 = 0.001 * Fpclk;			// Periodo PWM										   
	PWMLER = (1<<0)|(1<<2)|(1<<4);	// Habilitar MATCH0, MATCH2 y MATCH4
	PWMMCR = 0x02;					// Reset TIMER COUNTER REGISTER ON MATCH0							   
	PWMPCR = (1<<10)|(1<<12);		// Habilitar PWM2 y PWM4 en flanco
	PWMTCR = (1<<0)|(1<<3);			// Habilitar PWM y comenzar la cuenta
}
 
// Configuración de EINT1
void config_EINT1(void)
{
	PINSEL0 |= 3<<6; //1<<29;
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
	// Selección de velocidad
	if(IO1PIN&(1<<21))
		velocidad = 1;
	else
		velocidad = 0.8;

	// Configuraciones previas a la iniciación de las respectivas funciones
	config_MOTORES();
	config_EINT1();
	config_EINT3();
	config_ADC0();
	config_TIMER0();

	while(1) // Bucle infinito para la ejecución del programa
	{
	}
}
