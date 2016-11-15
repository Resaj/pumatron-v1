#include <lpc213x.h>
#include <stdio.h>

// Frecuencia del cristal del LPC2138
#define Fpclk 19.6608e6*3/4

// Estados para implementar la máquina de estados
#define Parado 0
#define Rastrear 1
char ESTADO = 10;
#define IZQ	5
#define DER 6
								 
// Variables auxiliares 
static unsigned int ADCresult[8];	// Array para la lectura de los sensores de línea
static unsigned int LINEAresult[8]; // Array que contiene los valores digitalizados de ADCresult
unsigned long r, ch;				// Variables para guardar los datos del ADC
int umbral = 600;					// Umbral para diferenciar 0 y 1 en la lectura del sensor
int giro = DER;						// Variable auxiliar para no perder la línea
int i;								// Variables auxiliares para crear los arrays
float velocidad = 1;   				// Velocidad máxima de los motores. 1 máximo
float Per_pwm0 = 0.001;				// Periodo de pwm0
float Per_timer = 0.01;				// Periodo del timer

//Variables PID
int sensors_average = 0;
int sensors_sum = 0;
int position = 0;
int proportional = 0;
int last_proportional = 0;
int derivative = 0;
int last_derivative = 0;
int control_value = 0;
int control_value_real = 0;
int max_difference = 1000;
float Kp = 1.5;	//0.3 para vel=0.5
float Kd = 3000;	//100 para vel=0.5


/********************** ALGORITMO PD **********************/
void get_average(void)
{
	//Zero the sensors_average variable
	sensors_average = 0;

	//Generate the weighted sensor average starting at sensor one to
	// keep from doing a needless multiplication by zero which will
	// always discarded any value read by sensor zero
	for(i = 0; i < 8; i++)
		sensors_average += LINEAresult[i]*i*1000;
}

void get_sum(void)
{
	//Zero the sensors_sum variable
	sensors_sum = 0;

	//Sum the total of all the sensors readings
	for(i = 0; i < 8; i++)
		sensors_sum += LINEAresult[i];
}

void get_position(void)
{
	//Calculate the current position on the line
	if (sensors_sum != 0)
		position = (int)(sensors_average / sensors_sum);
}

void get_proportional(void)
{
	//Calculate the proportional value
	proportional = position - 3500;
}

void get_derivative(void)
{
	//Calculate the derivative value
	derivative = proportional - last_proportional;

	//Store proportional value in last_proportional for the derivative
	// calculation on next loop
	last_proportional = proportional;
}

void get_control(void)
{
	//Calculate the control value
	control_value_real = (int)(proportional * Kp + derivative * Kd);
}

void adjust_control(void)
{
	//If the control value is greater than the allowed maximum set it
	// to the maximum
	if(control_value_real > max_difference)
		control_value = max_difference;
	else
		control_value = control_value_real;

	//If the control value is less than the allowed minimum set it to
	// the minimum
	if(control_value_real < -max_difference)
		control_value = -max_difference;
	else
		control_value = control_value_real;
}

void set_motors()
{
	//If control_value is less than zero a right turn ir needed to
	// acquire the center of the line
	if(control_value < 0)
	{
		PWMMR2 = velocidad*Per_pwm0*Fpclk;
		PWMMR4 = (max_difference + control_value)*(velocidad/max_difference)*Per_pwm0*Fpclk;
	}
	//If control_value is greater than zero a left turn is needed to
	// acquire the center of the line
	else
	{
		PWMMR2 = (max_difference - control_value)*(velocidad/max_difference)*Per_pwm0*Fpclk;
		PWMMR4 = velocidad*Per_pwm0*Fpclk;
	}

	PWMLER = (1<<0)|(1<<2)|(1<<4);	// Enable PWM Match 0, 2 & 4 Latch
}

void loop_PD(void)
{
	get_average();
	get_sum();
	get_position();
	get_proportional();
	get_derivative();
	get_control();
	adjust_control();
	set_motors();
}


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
		// Transformar valores analógicos de los sensores a digitales
		for(i=0;i<8;i++)
		{
			if(ADCresult[i] > umbral)
				LINEAresult[i] = 1;
			else
				LINEAresult[i] = 0;
		}

		loop_PD();
	}

	T0IR |= (1<<2);		// Activar flag de interrupción del Match0.2
	VICVectAddr = 0;
}

// Interrupción IRQ asociada a EINT1
void EXTERNA1(void)__irq
{
	ESTADO = Rastrear;
	
	IO0PIN |= (1<<23);
	IO0PIN &= ~(1<<20);
	IO1PIN |= (1<<31);
	IO1PIN &= ~(1<<30);

	IO0PIN &= ~(1<<31);
	IO1PIN &= ~(1<<22);

	EXTINT |= 1<<1;
	VICVectAddr = 0;
}

// Interrupción IRQ asociada a EINT3
void EXTERNA3(void)__irq
{
	PWMMR2 = 0;
	PWMMR4 = 0;
	PWMLER = (1<<0)|(1<<2)|(1<<4);	// Enable PWM Match 0, 2 & 4 Latch

	IO0PIN |= 1<<31;
	IO1PIN |= 1<<22;

	ESTADO = Parado;

	EXTINT |= 1<<3;
	VICVectAddr = 0;
}


/************************ CONFIGURACIONES ************************/
// Configuración del ADC0 para los sensores de línea
void config_ADC0 (void)
{
	PINSEL0 |= (3<<8)|(3<<10);									// Habilitar ADC0[6,7]
	PINSEL1 |= (1<<18)|(1<<20)|(1<<22)|(1<<24)|(1<<26)|(1<<28);	// Habilitar ADC0[0..5]

	VICVectAddr2 = (unsigned long) ADC0;
	VICVectCntl2 = (0x20|18);
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
	T0MR2 = Per_timer*Fpclk;
	VICVectAddr3 = (unsigned long) TIMER0;
	VICVectCntl3 = (0x20|4);
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
	PWMMR0 = Per_pwm0 * Fpclk;		// Periodo PWM									   
	PWMLER = (1<<0)|(1<<2)|(1<<4);	// Habilitar MATCH0, MATCH2 y MATCH4
	PWMMCR = 0x02;					// Reset TIMER COUNTER REGISTER ON MATCH0							   
	PWMPCR = (1<<10)|(1<<12);		// Habilitar PWM2 y PWM4 en flanco
	PWMTCR = (1<<0)|(1<<3);			// Habilitar PWM y comenzar la cuenta
}
 
// Configuración de EINT1
void config_EINT1(void)
{
	PINSEL0 |= 1<<29;
	EXTMODE |= 1<<1;
	EXTPOLAR |= 1<<1;
	VICVectAddr0 = (unsigned long) EXTERNA1;
	VICVectCntl0 = (0x20|15);
	VICIntEnable |= 1<<15;
}

// Configuración de EINT3
void config_EINT3(void)
{
	PINSEL0 |= 3<<18;
	EXTMODE |= 1<<3;
	EXTPOLAR &= ~(1<<3);
	VICVectAddr1 = (unsigned long) EXTERNA3;
	VICVectCntl1 = (0x20|17);
	VICIntEnable |= 1<<17;
}


/************************** FUNCIÓN MAIN **************************/
int main (void)
{
	// Configuraciones previas a la iniciación de las respectivas funciones
	IO0DIR |= 1<<31;
	IO1DIR |= 1<<22;
	config_MOTORES();
	config_EINT1();
	config_EINT3();
	ESTADO = Parado;
	IO0PIN |= 1<<31;
	IO1PIN |= 1<<22;
	config_ADC0();
	config_TIMER0();

	while(1) // Bucle infinito para la ejecución del programa
	{
	}
}
