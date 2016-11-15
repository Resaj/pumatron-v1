#include <stdbool.h>

#ifndef Declaraciones_h
#define Declaraciones_h

#define Fpclk 19.6608e6*3/4		// Frecuencia del cristal del LPC2138
#define S 12					// Número de sensores

// Estados para implementar la máquina de estados
#define Parado '0'
#define Seleccion_velocidad '1'
#define Umbralizar_sensores '2'
#define Espera '3'
#define Rastrear '4'
#define Linea_giro '5'
#define Bifurcacion_proxima '6'
#define Bifurcacion '7'
#define Avanzar1 '8'
#define Avanzar2 '9'
#define Obstaculo 'A'
extern char ESTADO;

#define IZQ	'1'
#define DER '2'
#define REC '3'


extern float Periodo;
extern float Per_pwm;

// Variables auxiliares
extern char cadena[30];
extern char start;
extern unsigned int seg_espera;
extern float cont;
extern int i, t;
extern int flag;

// Variables para el control del rastreo
extern unsigned int ADC0[8];
extern unsigned int ADC1_5;
extern unsigned int ADCresult[S];
extern unsigned int LINEAresult[S];
extern unsigned int sensor_frontal;
extern char mux;
extern unsigned int inf, sup;

extern unsigned int umbral[S];
extern unsigned int blanco[S];	
extern unsigned int negro[S];

extern char LINEA;
extern char giro;
extern char GIRO;
extern char GIRO_aux_izq;
extern char GIRO_aux_der;
extern char GIRO_aux;
extern int aux_ant[3];
extern int aux_act[4];
extern bool marca[7];
extern int negros;
extern char cond;

extern char secuencia_actual;
extern char secuencia_anterior;
extern bool linea_buena;

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
void TIMER1(void)__irq;
void TIMER0(void)__irq;

// Funciones de control
void selec_velocidad(void);
void leer_sensor_frontal(void);
void leer_sensores(void);
void reordenar(void);
void calibrar_sensores(void);
void estimar_umbrales(void);
void digitalizar(void);
void detectar_secuencias(void);
void borrar_marca(void);
void borrar_linea(void);
void actualizar(void);
void conmutar_mux(void);
