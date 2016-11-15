#include <lpc213x.h>
#include <stdbool.h>
#include "Declaraciones.h"
#include "PID.h"

/************************ VARIABLES ************************/
char ESTADO;

float Periodo = 1e-3;		// Periodo de ejecución del timer en modo match
float Per_pwm = 5e-4;		// Periodo de la PWM0

// Variables auxiliares 
char cadena[30];			// Variable auxiliar para guardar el texto a imprimir en el LCD
float start = 0;
unsigned int seg_espera = 0;
float cont = 0;
int i = 0, t = 0;
int flag = 1;

// Variables para el control del rastreo
unsigned int ADC[8];			// Array para recoger los datos del ADC
unsigned int ADCresult[S];		// Array con los datos del ADC ordenados
char LINEAresult[S];			// Array que contiene los valores digitalizados de ADCresult
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
float v_lenta = 0.5;			// Control de velocidad para las bifurcaciones. 1 max
float velocidad = 0.5;			// Velocidad de rastreo

//Variables PID
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
int max_difference = 3000;
float Kp = 4;
float Ki = 0;
float Kd = 10;

// Variables para el control de la UART
char dato_UART = 0;
int Fdiv = 0;
char * buffer_tx[N_MENSAJES];
char ptr_rd = 0, ptr_wr = 0, reposo = 1;
char mensajes_ptes = 0;
char *ptr_tx;
char mensaje[S];
char mensaje_anterior[S];
unsigned int rep = 1;


/************************** FUNCIÓN MAIN **************************/
int main (void)
{
	ESTADO = Seleccion_velocidad;

	// Actualizar umbrales
	for(i=0; i<S; i++)
	{
		blanco[i] = 700;
		umbral[i] = 800;
		negro[i] = 900;

		mensaje[i] = 'X';
		mensaje_anterior[i] = 'X';
	}

	// Configuraciones previas
	config_load();
	config_LCD();
	config_leds();
	config_EINT3();
	config_MOTORES();
	config_ADC0();
	config_ADC1();
	//config_UART0();
	//config_TIMER1();
	config_TIMER0();

	IOSET0 = 1<<27;
	IOSET0 = 1<<31;
	IOCLR1 = 1<<22;
	IOCLR1 = 1<<23;
	v_rapida = 1;

	while(1) // Bucle infinito para la ejecución del programa
	{
	}
}
