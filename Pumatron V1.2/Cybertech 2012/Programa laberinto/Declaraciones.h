#include <stdbool.h>

#ifndef Declaraciones_h
#define Declaraciones_h

#define Fpclk 19.6608e6*3/4		// Frecuencia del cristal del LPC2138

// Estados para implementar la máquina de estados
#define Parado '0'
#define Seleccion_velocidad '1'
#define Espera '2'
#define Ejecucion '3'
extern char ESTADO;

#define IZQ	'1'
#define DER '2'
#define REC '3'


extern float Periodo;
extern float Per_pwm;

// Variables auxiliares
extern char cadena[30];
extern char start;
extern float cont, cont2;
extern int i, t;
extern int flag;

// Variables para el control del rastreo
extern unsigned int ADCresult[8];
extern unsigned int distancia[8];
extern unsigned long r, ch;
extern int sensor_izq;
extern int sensor_der;
extern int sensor_frontal;
extern char PARED;

// Variables para el control de velocidad
extern float v_rapida;
extern float v_lenta;
extern float velocidad;

#endif


// Configuraciones
void config_EINT1(void);
void config_EINT3(void);
void config_ADC(void);
void config_TIMER1(void);
void config_TIMER0(void);
void config_MOTORES(void);
void config_leds(void);
void config_LCD(void);

// Interrupciones
void EXTERNA1(void)__irq;
void EXTERNA3(void)__irq;
void ADC1(void)__irq;
void TIMER1(void)__irq;
void TIMER0(void)__irq;

// Funciones de control
void selec_velocidad(void);
void GP2Y0A_read(void);
void GP2D120_read(void);
void leer_sensores(void);
