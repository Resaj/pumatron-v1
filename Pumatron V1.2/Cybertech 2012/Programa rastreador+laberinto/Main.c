#include <lpc213x.h>
#include <stdbool.h>
#include "Declaraciones.h"
#include "PID.h"
#include "PID_lab.h"

/************************ VARIABLES ************************/
char ESTADO;
char SUPERESTADO;

float Periodo = 10e-3;		// Periodo de ejecución del timer en modo match
float Per_pwm = 5e-4;		// Periodo de la PWM0

// Variables auxiliares 
char cadena[30];			// Variable auxiliar para guardar el texto a imprimir en el LCD
char start = '0';
unsigned int seg_espera = 0;
float cont = 0, cont2 = 0;
int i = 0, t = 0;
int flag = 1;

// Variables para el control del rastreo
unsigned long r, ch;		// Variables para guardar los datos del ADC
unsigned int ADCresult1[8];	// Array para recoger los datos del ADC
unsigned int distancia[8];	// Array que contiene los valores de los sensores de distancia
unsigned int sensor_izq = 0;
unsigned int sensor_der = 0;
unsigned int sensor_frontal = 0;
char PARED = IZQ;			// Pared a seguir

unsigned int pared_izq[20], pared_der[20];
unsigned int promedio_izq = 0, promedio_der = 0;

unsigned int ADC0[8];			// Array para recoger los datos del ADC0
unsigned int ADCresult[S];		// Array con los datos del ADC ordenados
unsigned int LINEAresult[S];	// Array que contiene los valores digitalizados de ADCresult
char mux = '0';					// Variable para conmutar el multiplexor
unsigned int inf = 0, sup = 0;			// Variables para la estimación de los umbrales

unsigned int umbral[S];			// Umbral para diferenciar 0 y 1 en la lectura del sensor
unsigned int blanco[S];	
unsigned int negro[S];

char giro = '0';				// Variable auxiliar para no perder la línea. Se inicia a un valor distinto a IZQ y DER
char GIRO = '0';				// Variable de orden de giro en cruces. Se inicia a un valor distinto a IZQ y DER
char GIRO_aux_izq = '0';
char GIRO_aux_der = '0';
char GIRO_aux = '0';
int aux_ant[3];					// Detecta la posición de la línea de giro respecto a la línea principal
int aux_act[4];
bool marca[7];
int negros = 0;					// Número de sensores en negro
char cond = '1';				// Condición para habilitar o deshabilitar la suma de la tercera línea

char secuencia_actual = '1';	// Secuencia de línea en el instante actual
char secuencia_anterior = '0';	// Secuencia de línea en el instante anterior
bool linea_buena = true;		// Línea buena tomada en el cruce

// Variables para el control de velocidad
float v_rapida = 1;				// Control de velocidad de rastreo normal. 1 max
float v_lenta = 0.4;			// Control de velocidad para las bifurcaciones. 1 max
float velocidad = 0.4;			// Velocidad de rastreo
float velocidad_lab = 0.8;

// Variables PID línea
long sensors[S];
long sensors_average = 0;
int sensors_sum = 0;
int position = 0;
int proportional = 0;
long integral = 0;
long integral_max = 1000;
int derivative = 0;
int last_proportional = 0;
int control_value = 0;
int max_difference = 3000;	// 3000
float Kp = 0.5;		//4
float Ki = 0;		//0
float Kd = 10;		//50

// Variables PID laberinto
int position_lab = 0;
int proportional_lab = 0;
long integral_lab = 0;
long integral_max_lab = 100;
int derivative_lab = 0;
int last_proportional_lab = 0;
int control_value_lab = 0;
int max_difference_lab = 50;
float Kp_lab = 1;
float Ki_lab = 0;
float Kd_lab = 20;


/************************** FUNCIÓN MAIN **************************/
int main (void)
{
	SUPERESTADO = Rastreador;
	ESTADO = Seleccion_velocidad;

	for(i=0; i<20; i++)
	{
		pared_izq[i] = 350;
		pared_der[i] = 350;
	}

	// Actualizar umbrales
	for(i=0; i<S; i++)
	{
		blanco[i] = 700;
		umbral[i] = 800;
		negro[i] = 900;
	}

	// Configuraciones previas
	//config_LCD();
	config_leds();
	config_EINT1();
	config_EINT3();
	config_MOTORES();
	config_ADC();
	//config_TIMER1();
	config_TIMER0();

	IO0PIN |= (1<<27)|(1<<31);
	IO1PIN &= ~(1<<22)|(1<<23);
	v_rapida = 1;

	start = '1';

	while(1) // Bucle infinito para la ejecución del programa
	{
	}
}
