#include <lpc213x.h>
#include <stdio.h>
#include "font.h"		
#include "s1d15g00.h"

// Frecuencia del cristal del LPC2138
#define Fpclk 19.6608e6*3/4

// Estados para implementar la máquina de estados
#define Parado 0
#define Rastrear 1
char ESTADO = 0;	// La aplicación comienza en el estado "0"
#define IZQ	5
#define DER 6
								 
// Variables auxiliares 
char cadena[30];					// Variable auxiliar para guardar el texto a imprimir en el LCD
static unsigned int ADCresult[8];	// Array para la lectura de los sensores de línea
unsigned long r, ch;				// Variables para guardar los datos del ADC
int umbral = 500;					// Umbral para diferenciar 0 y 1 en la lectura del sensor
int giro = DER;						// Variable auxiliar para no perder la línea


/********************** ALGORITMO PD **********************/

//********
//VARIBLES
//********
long sensors[] = {0, 0, 0, 0};
long sensors_average = 0;
int sensors_sum = 0;
int position = 0;
int proportional = 0;
int derivative = 0;
int last_proportional = 0;
int control_value = 0;
int max_difference = 80;
float Kp = .05;
float Kd = 1.105;

//*********************
//READ_SENSORS FUNCTION
//*********************
void read_sensors()
{
	//Take a reading from each of the five line sensors
	for(int i = 0; i < 4; i++)
		sensors[i] = ADCresult[i];
}

//********************
//GET_AVERAGE FUNCTION
//********************
void get_average()
{
	//Zero the sensors_average variable
	sensors_average = 0;

	//Generate the weighted sensor average starting at sensor one to
	// keep from doing a needless multiplication by zero which will
	// always discarded any value read by sensor zero
	for(int i = 0; i < 8; i++)
		sensors_average += sensors[i]*i*1000;
}

//****************
//GET_SUM FUNCTION
//****************
void get_sum()
{
	//Zero the sensors_sum variable
	sensors_sum = 0;

	//Sum the total of all the sensors readings
	for(int i = 0; i < 5; i++)
		sensors_sum += int(sensors[i]);
}

//**************
//GET_POSITION FUNCTION
//**************
void get_position()
{
	//Calculate the current position on the line
	position = int(sensors_average / sensors_sum);
}

//*************************
//GET_PROPORTIONAL FUNCTION
//*************************
void get_proportional()
{
	//Calculate the proportional value
	proportional = position - 2000;
}

//***********************
//GET_DERIVATIVE FUNCTION
//***********************
void get_derivative()
{
	//Calculate the derivative value
	derivative = proportional - last proportional;

	//Store proportional value in last_proportional for the derivative
	// calculation on next loop
	last_proportional = proportional;
}

//********************
//GET_CONTROL FUNCTION
//********************
void get_control()
{
	//Calculate the control value
	control_value = int(proportional * Kp + derivative * Kd);
}

//***********************
//ADJUST_CONTROL FUNCTION
//***********************
void adjust_control()
{
	//If the control value is greater than the allowed maximum set it
	// to the maximum
	if(control_value > max_difference)
		control_value = max_difference;

	//If the control value is less than the allowed minimum set it to
	// the minimum
	if(control_value < -max_difference)
		control_value = -max_difference;
}

//*******************
//SET_MOTORS FUNCTION
//*******************
void set_motors()
{
	//If control_value is less than zero a right turn ir needed to
	// acquire the center of the line
	if(control_value < 0)
	{
		analogWrite(RIGHT_MOTOR, max_difference + control_value);
		analogWrite(LEFT_MOTOR, max_difference);
	}
	//If control_value is greater than zero a left turn is needed to
	// acquire the center of the line
	else
	{
		analogWrite(RIGHT_MOTOR, max_difference);
		analogWrite(LEFT_MOTOR, max_difference - control_value);
	}
}

//*************
//LOOP FUNCTION
//*************
void loop()
{
	read_sensors();
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
		loop();

		PWMLER = (1<<0)|(1<<2)|(1<<4);	// Enable PWM Match 0, 2 & 4 Latch

		LCDClearScreen();	// Limpiar pantalla
		for(i = 0; i < 8; i++)
		{
			sprintf(cadena, "%d", ADCresult[i]);   
			LCDPutStr(cadena, (120-(i*10)), 5, SMALL, WHITE, BLACK);
		}
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
	T0MR2 = 0.1 * Fpclk;
	VICVectAddr3 = (unsigned long) TIMER0;
	VICVectCntl3 = (0x20|4);
	T0TCR = 0x01;
	VICIntEnable |= 1<<4;
}

// Configuración del display
void config_LCD (void)
{
  LCD_BL_DIR();			// BL = Output
  LCD_CS_DIR();			// CS = Output
  LCD_SCLK_DIR();		// SCLK = Output
  LCD_SDATA_DIR();		// SDATA = Output
  LCD_RESET_DIR();		// RESET = Output
  LCD_SCLK_LOW();		// Standby SCLK
  LCD_CS_HIGH();		// Disable CS
  LCD_SDATA_HIGH();		// Standby SDATA
  LCD_BL_HIGH();		// Black Light ON = 100%

  InitLcd(); 			// Initial LCD
  LCDClearScreen();		// Limpiar pantalla
}

// Configuración de las salidas de PWM para los motores
void config_MOTORES (void)
{
	PINSEL0 |= (1<<15)|(1<<17);		// PWM2 y PWM4
	IO0DIR |= (1<<20)|(1<<23);		// Salidas para el sentido de los motores
	IO1DIR |= (1<<30)|(1<<31);
	PWMTCR = 0x02;					// Reset TIMER y PREESCALER										   
	PWMPR = 0;						// El PRESCALER no modifica la frecuencia
	PWMMR0 = 0.02 * Fpclk;			// Periodo PWM										   
	PWMLER = (1<<0)|(1<<2)|(1<<4);	// Habilitar MATCH0, MATCH2 y MATCH4
	PWMMCR = 0x02;					// Reset TIMER COUNTER REGISTER ON MATCH0							   
	PWMPCR = (1<<10)|(1<<12);		// Habilitar PWM2 y PWM4 en flanco
	PWMTCR = (1<<0)|(1<<3);			// Habilitar PWM y comenzar la cuenta
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
	// Condición para evitar que el robot se pare si se produce un reset
	if(IO1PIN&(1<<21))
		ESTADO = Rastrear;

	// Configuraciones previas a la iniciación de las respectivas funciones
	config_LCD();
	config_MOTORES();
	config_EINT1();
	config_EINT3();
	config_ADC0();
	config_TIMER0();

	while(1) // Bucle infinito para la ejecución del programa
	{
	}
}
