#include <lpc213x.h>
#include <stdbool.h>
#include "Declaraciones.h"
#include "PID.h"

/************************ VARIABLES ************************/
char ESTADO = Seleccion_velocidad;

float Periodo = 5e-3;		// Periodo de ejecución del timer en modo match
float Per_pwm = 1e-3;		// Periodo de la PWM0

// Variables auxiliares 
char cadena[30];			// Variable auxiliar para guardar el texto a imprimir en el LCD
unsigned int seg_espera = 0;
char opcion_recta = '0';
int cont = 0;
int i = 0, t = 0;
int flag = 1;

// Variables para el control del rastreo
unsigned int ADC[8];			// Array para recoger los datos del ADC
unsigned int ADCresult[S];		// Array con los datos del ADC ordenados
unsigned int LINEAresult[S];	// Array que contiene los valores digitalizados de ADCresult
unsigned long r, ch;			// Variables para guardar los datos del ADC
char mux = '0';					// Variable para conmutar el multiplexor

unsigned int umbral[S];			// Umbral para diferenciar 0 y 1 en la lectura del sensor
unsigned int blanco[S];	
unsigned int negro[S];

char giro = '0';				// Variable auxiliar para no perder la línea. Se inicia a un valor distinto a IZQ y DER
char GIRO = '0';				// Variable de orden de giro en cruces. Se inicia a un valor distinto a IZQ y DER
char GIRO_aux_izq = '0';
char GIRO_aux_der = '0';
char GIRO_aux = '0';
char GIRO_tomado = '0';
char aux_der = '0';
char aux_izq = '0';
int aux_ant[3];					// Detecta la posición de la línea de giro respecto a la línea principal
int aux_act[4];
bool marca[7];
int negros = 0;					// Número de sensores en negro
char cond = '1';				// Condición para habilitar o deshabilitar la suma de la tercera línea

char secuencia_actual = '1';	// Secuencia de línea en el instante actual
char secuencia_anterior = '0';	// Secuencia de línea en el instante anterior
bool linea_buena = true;		// Línea buena tomada en el cruce

// Variables para el control de velocidad
float v_rapida = 0.4;			// Control de velocidad de rastreo normal. 1 max
float v_lenta = 0.4;			// Control de velocidad para las bifurcaciones. 1 max
float velocidad = 0.4;			// Velocidad de rastreo

//Variables PID
int sensors_average = 0;
int sensors_sum = 0;
int position = 0;
int proportional = 0;
long integral = 0;
long integral_max = 1000;
int derivative = 0;
int last_proportional = 0;
int control_value = 0;
int max_difference = 3000;	// 3000
float Kp = 4;		//4
float Ki = 0;		//0
float Kd = 50;		//50


/************************** FUNCIÓN MAIN **************************/
int main (void)
{
	// Actualizar umbrales
	for(i=0; i<S; i++)
		umbral[i] = 750;

	// Configuraciones previas
	//config_LCD();
	config_leds();
	config_EINT1();
	config_EINT3();
	config_MOTORES();
	config_ADC0();
	//config_TIMER1();
	config_TIMER0();

	IO0PIN &= ~((1<<27)|(1<<31));
	IO1PIN |= (1<<23);
	IO1PIN &= ~(1<<22);

	while(1) // Bucle infinito para la ejecución del programa
	{
	}
}
