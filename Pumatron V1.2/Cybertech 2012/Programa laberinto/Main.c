#include <lpc213x.h>
#include <stdbool.h>
#include "Declaraciones.h"
#include "PID.h"

/************************ VARIABLES ************************/
char ESTADO;

float Periodo = 10e-3;		// Periodo de ejecución del timer en modo match
float Per_pwm = 5e-4;		// Periodo de la PWM0

// Variables auxiliares 
char cadena[30];			// Variable auxiliar para guardar el texto a imprimir en el LCD
char start = '0';
float cont = 0, cont2 = 0;
int i = 0, t = 0;
int flag = 1;

// Variables para el control del seguimiento de pared
unsigned int ADCresult[8];	// Array para recoger los datos del ADC
unsigned int distancia[8];	// Array que contiene los valores de los sensores de distancia
unsigned long r, ch;		// Variables para guardar los datos del ADC
int sensor_izq = 0;
int sensor_der = 0;
int sensor_frontal = 0;
char PARED = IZQ;			// Pared a seguir

// Variables para el control de velocidad
float v_rapida = 1;
float v_lenta = 0.4;
float velocidad = 1;

//Variables PID
int position = 0;
int proportional = 0;
long integral = 0;
long integral_max = 100;
int derivative = 0;
int last_proportional = 0;
int control_value = 0;
int max_difference = 50;
float Kp = 1;
float Ki = 0;
float Kd = 20;


/************************** FUNCIÓN MAIN **************************/
int main (void)
{
	ESTADO = Seleccion_velocidad;

	// Configuraciones previas
	//config_LCD();
	config_leds();
	config_EINT1();
	config_EINT3();
	config_MOTORES();
	config_ADC();
	//config_TIMER1();
	config_TIMER0();

	IO0PIN |= 1<<27;
	IO1PIN &= ~((1<<22)|(1<<23));
	IO0PIN &= ~(1<<31);
	v_rapida = 0.8;

	start = '1';

	while(1) // Bucle infinito para la ejecución del programa
	{
	}
}
