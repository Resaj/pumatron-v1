#include <lpc213x.h>
#include <stdio.h>

// Frecuencia del cristal del LPC2138
#define Fpclk 19.6608e6*3/4

// Estados para implementar la máquina de estados
#define Parado 0
#define Seleccion_velocidad 1
#define Avanzar 2
#define Seguir_pared 3
char ESTADO = 0;
#define IZQ	7
#define DER 8

float Per_timer = 0.05;	// Periodo de ejecución del timer en modo match
float Per_pwm = 0.02;	// Periodo de la PWM0
								 
// Variables auxiliares 
char cadena[30];		// Variable auxiliar para guardar el texto a imprimir en el LCD
int i = 0;
int flag_1 = 1;
int flag_2 = 1;
float velocidad = 0.4;	// Control de velocidad. 1 max

// Variables para el control del seguimiento de la pared
unsigned long r, ch;				// Variables para guardar los datos del ADC
static unsigned int ADCresult[8];	// Array para la lectura de los sensores de línea
static unsigned int distancia[8];	// Array que contiene los valores de los sensores de distancia
float distancia_actual = 0;
float distancia_anterior = 0;
int sensor_lateral = 0;
int sensor_frontal = 0;
int PARED = 0;						// Pared a seguir
int channel = 0;
int cont = 0;
int cont2 = 0;


/********************** FUNCIONES DE CONTROL **********************/
void seguir_pared(void)
{
	if(PARED == IZQ)
	{
		if(sensor_lateral < 300 && sensor_frontal < 200)
		{
			IO0PIN |= (1<<20);
			IO0PIN &= ~(1<<23);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = 0.5 * velocidad * Per_pwm*Fpclk;
			PWMMR4 = 0.5 * velocidad * Per_pwm*Fpclk;
		}
		else
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);

			if(sensor_lateral < 120)
			{
				PWMMR2 = 1 * velocidad * Per_pwm*Fpclk;
				PWMMR4 = 0.2 * velocidad * Per_pwm*Fpclk;
			}
			else if(sensor_lateral > 250)
			{
				PWMMR2 = 0.2 * velocidad * Per_pwm*Fpclk;
				PWMMR4 = 1 * velocidad * Per_pwm*Fpclk;
			}
			else if(distancia_actual > distancia_anterior)
			{
				PWMMR2 = 0.85 * velocidad * Per_pwm*Fpclk;
				PWMMR4 = 1 * velocidad * Per_pwm*Fpclk;
			}
			else
			{
				PWMMR2 = 1 * velocidad * Per_pwm*Fpclk;
				PWMMR4 = 0.7 * velocidad * Per_pwm*Fpclk;
			}
		}
	}
	else // PARED == DER
	{
		if(sensor_lateral < 300 && sensor_frontal < 200)
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<30);
			IO1PIN &= ~(1<<31);
			PWMMR2 = 0.5 * velocidad * Per_pwm*Fpclk;
			PWMMR4 = 0.5 * velocidad * Per_pwm*Fpclk;
		}
		else
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			if(sensor_lateral < 140)
			{
				PWMMR2 = 0.2 * velocidad * Per_pwm*Fpclk;
				PWMMR4 = 1 * velocidad * Per_pwm*Fpclk;
			}
			else if(sensor_lateral > 250)
			{
				PWMMR2 = 1 * velocidad * Per_pwm*Fpclk;
				PWMMR4 = 0.2 * velocidad * Per_pwm*Fpclk;
			}
			else if(distancia_actual > distancia_anterior)
			{
				PWMMR2 = 1 * velocidad * Per_pwm*Fpclk;
				PWMMR4 = 0.7 * velocidad * Per_pwm*Fpclk;
			}
			else
			{
				PWMMR2 = 0.85 * velocidad * Per_pwm*Fpclk;
				PWMMR4 = 1 * velocidad * Per_pwm*Fpclk;
			}
		}
	}
}

void config_EINT1(void);
void config_EINT3(void);

void selec_velocidad(void)
{
	if((IO0PIN&(1<<3)) == 0 && flag_1 == 1)
	{
		if(velocidad == (float)0.4)
		{
			IO0PIN &= ~(1<<31);
			IO1PIN |= (1<<22);
			velocidad = 0.6;
		}
		else if(velocidad == (float)0.6)
		{
			IO0PIN |= (1<<31);
			IO1PIN &= ~(1<<22);
			velocidad = 0.8;
		}
		else if(velocidad == (float)0.8)
		{
			IO0PIN |= (1<<31);
			IO1PIN |= (1<<22);
			velocidad = 1;
		}
		else // velocidad == 1
		{
			IO0PIN &= ~(1<<31);
			IO1PIN &= ~(1<<22);
			velocidad = 0.4;
		}
	}
	flag_1 = (IO0PIN&(1<<3))>>3;

	if((IO0PIN&(1<<9)) == 0 && flag_2 == 1)
	{
		IO0PIN &= ~(1<<31);
		IO1PIN &= ~(1<<22);
		config_EINT1();
		config_EINT3();
		ESTADO = Parado;
	}
	flag_2 = (IO0PIN&(1<<9))>>9;
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

// Función que realiza la lectura de los sensores de distancia
void ADC1(void)__irq
{
	// Se toman los valores de los sensores y se guardan en un array
	r = AD1DR;

	// Selección del canal convertido en ADC0
	ch = (r>>24)&0x07;

	// Guardar resultado de la conversión
	ADCresult[ch] = (r>>6)&0x03FF;

	VICVectAddr = 0;
}

void leer_sensores(void)
{
	channel = 0;
	GP2D120_read();
	channel = 4;
	GP2D120_read();

	sensor_lateral = distancia[0];
	sensor_frontal = distancia[4];
}

// Interrupción IRQ para el TIMER0
void TIMER0(void)__irq
{
	if(ESTADO == Seleccion_velocidad)
		selec_velocidad();
	else if(ESTADO != Parado)
	{
		leer_sensores();

		if(ESTADO == Avanzar && cont*Per_timer > 0.4)
			ESTADO = Seguir_pared;

		// Ejecución del seguimiento de la pared
		if(ESTADO == Seguir_pared)
		{
			if(cont*Per_timer > 0.1)
			{
				distancia_actual = sensor_lateral;
				if(distancia_actual == 0)
					distancia_anterior = distancia_actual;
				cont = 0;
			}
			else
				cont++;

			seguir_pared();
			if(cont == 0)
				distancia_anterior = distancia_actual;
		}
		else // ESTADO == Avanzar
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = 1 * velocidad * Per_pwm*Fpclk;
			PWMMR4 = 0.9 * velocidad * Per_pwm*Fpclk;
			cont++;
		}


		PWMLER = (1<<0)|(1<<2)|(1<<4);	// Enable PWM Match 0, 2 & 4 Latch

		// Visualización de los datos
//		sprintf(cadena, "ESTADO %d", ESTADO);   
//		LCDPutStr(cadena, 120, 30, SMALL, WHITE, BLACK);
	}
	
	T0IR |= (1<<2);		// Activar flag de interrupción del Match0.2
	VICVectAddr = 0;
}

// Interrupción IRQ asociada a EINT1
void EXTERNA1(void)__irq
{
	if(ESTADO == Parado)
	{
		ESTADO = Avanzar;
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
	else // ESTADO == Parado
	{
		if(PARED == IZQ)
		{
			PARED = DER;
			PWMMR5 = 0.042 * Per_pwm*Fpclk;	//0.032 en perpendicular // 0.05 para simetría con la otra posición
		}
		else
		{
			PARED = IZQ;
			PWMMR5 = 0.11 * Per_pwm*Fpclk;	//0.128 en perpendicular
		}

		PWMLER = (1<<0)|(1<<5);	// Enable PWM Match 0 & 5 Latch
	}

	EXTINT |= 1<<3;
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
	PINSEL0 |= (3<<12)|(3<<26);	// Habilitar ADC1[0,4]

	VICVectAddr0 = (unsigned long) ADC1;
	VICVectCntl0 = (0x20|21);
	VICIntEnable |= (1<<21);

	AD1CR = 0x210311;	// Activación de la conversión para los canales 0 y 4
}

// Configuración del Match2 del TIMER0
void config_TIMER0 (void)
{
	PINSEL1 |= 1<<1;
	T0TCR = 0x02;
	T0PR = 0;
	T0MCR = (1<<6)|(1<<7);
	T0MR2 = Per_timer*Fpclk;
	VICVectAddr1 = (unsigned long) TIMER0;
	VICVectCntl1 = (0x20|4);
	T0TCR = 0x01;
	VICIntEnable |= 1<<4;
}

// Configuración de las salidas de PWM para los motores
void config_MOTORES (void)
{
	PINSEL0 |= (1<<15)|(1<<17);				// PWM2 y PWM4
	PINSEL1 |= (1<<10);						// PWM5
	IO0DIR |= (1<<20)|(1<<23);				// Salidas para el sentido de los motores
	IO1DIR |= (1<<30)|(1<<31);
	PWMTCR = 0x02;							// Reset TIMER y PREESCALER										   
	PWMPR = 0;								// El PRESCALER no modifica la frecuencia
	PWMMR0 = Per_pwm * Fpclk;				// Periodo PWM									   
	PWMLER = (1<<0)|(1<<2)|(1<<4)|(1<<5);	// Habilitar MATCH0, MATCH2 y MATCH4
	PWMMCR = 0x02;							// Reset TIMER COUNTER REGISTER ON MATCH0							   
	PWMPCR = (1<<10)|(1<<12)|(1<<13);		// Habilitar PWM2 y PWM4 en flanco
	PWMTCR = (1<<0)|(1<<3);					// Habilitar PWM y comenzar la cuenta
}
 
// Configuración de EINT1
void config_EINT1(void)
{
	PINSEL0 |= 3<<6;	//ó 1<<29
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
	// Activar leds
	IO0DIR |= (1<<31);
	IO1DIR |= (1<<22);

	// Selección de velocidad de los sensores
	if((IO1PIN&(1<<21)) == 0)
		ESTADO = Seleccion_velocidad;
	else
	{
		ESTADO = Parado;
		config_EINT1();
		config_EINT3();
	}

	// Configuraciones previas
	//config_LCD();
	config_MOTORES();
	config_ADC1();
	config_TIMER0();

	while(1) // Bucle infinito para la ejecución del programa
	{
	}
}
