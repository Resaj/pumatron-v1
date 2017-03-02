#include <lpc213x.h>

// Frecuencia del cristal del LPC2138
#define Fpclk 19.6608e6*3/4

// Estados para implementar la máquina de estados
#define Parado 0
#define Rastrear 1
#define Linea_giro 2
#define Bifurcacion 3
char ESTADO = Parado;	// La aplicación comienza en el estado "0"
#define IZQ	6
#define DER 7

float Periodo = 1e-3;	// Periodo de ejecución del timer en modo match
float Per_pwm = 1e-3;	// Periodo de la PWM0
								 
// Variables auxiliares 
int i = 0;
int cont = 0;

// Variables para el control del rastreo
static unsigned int ADCresult[8];	// Array para la lectura de los sensores de línea
static unsigned int LINEAresult[8]; // Array que contiene los valores digitalizados de ADCresult
static unsigned int LINEAanterior[8] = {0, 0, 0, 1, 1, 0, 0, 0}; // Arrays que contienen la situación de la línea de los instantes actual y anterior
float k=1;
int posicion=7;
int anterior=7;
unsigned long r, ch;				// Variables para guardar los datos del ADC
int umbral = 800;					// Umbral para diferenciar 0 y 1 en la lectura del sensor
int giro = 0;						// Variable auxiliar para no perder la línea. Se inicia a un valor distinto a IZQ y DER
int GIRO = DER;						// Variable de orden de giro en cruces. Se inicia a un valor distinto a IZQ y DER

int aux_ant =  0;		 	// Detecta la posición de la línea de giro respecto a la línea principal
int aux_act1 =  0;
int aux_act2 =  0;
int hueco1 =  0;	 		// Detecta 101
int hueco2 =  0;			// Detecta 101
int nSensores =  0;			// num de sensores en negro

int secuencia_actual = 0;	// Secuencia de línea en el instante actual
int secuencia_anterior = 0;	// Secuencia de línea en el instante anterior
int habilitar_marca = 1;	// Variable que habilita la lectura de la marca de giro
int linea_buena = 1;		// Linea buena cogida en el cruce

// Variables para el control de velocidad
float velocidad1 = 0.9;					// Control de velocidad de rastreo normal. 1 max
float velocidad2 = 0.4;					// Control de velocidad para las bifurcaciones. 1 max
float velocidad_rastreo = 1;	// Velocidad de rastreo
float reductor_peq = 0.8;
float reductor_med = 0.5;
float reductor_GRAN = 0.3;
float reductor_gran = 0.1;
float compensa = 0.75;
float compensa_peq = 0.7;
float compensa_med = 0.9;
float compensa_gran = 1.0;


/********************** FUNCIONES DE CONTROL **********************/

void detectar_secuencias(void)
{
	// num de sensores en negro
	for (i=0; i<8; i++)
		nSensores += LINEAresult[i];

	// Si detectamos 101 hay hueco y consideramos sólo 2 sensores juntos
	for (i=0; i<8; i++)
	{
		if (LINEAresult[i] == 1 && hueco1 != 1)
		{
			aux_act1 = i;
			hueco1 = 1;
		}
		if (LINEAresult[i] == 0 && hueco1 == 1 && hueco2 != 1)
			hueco2 = 1;
		if (LINEAresult[i] == 1 && hueco2 == 1)
			secuencia_actual = 101;

	}

	// Si hay más de 2 sensores en negro y no hay hueco, tenemos más de 2 sensores juntos en negro
	if (secuencia_actual != 101 && nSensores > 2)
		secuencia_actual = 111;
}

void calc_variacion(void)
{
	posicion=0;
	for(i=0;i<8;i++)
		posicion+=LINEAresult[i]*i;
}

void borrar_linea(void)
{
	if(GIRO == DER) // Estamos en un cruce y debemos girar a la derecha
	{
		// Detectamos el primer negro por la derecha y borramos a partir de los 2 sensores siguientes hacia la izquierda
		if(aux_act1 > aux_ant+1)
		{
			for(i=0; i<8; i++)
				LINEAresult[i] = 0;
			aux_act1 = aux_ant;
			giro = DER;
			linea_buena = 0;
		}
		else
		{
			i = aux_act1+2;
			for(i; i<8; i++)
				LINEAresult[i] = 0;
			linea_buena = 1;
		}
	}
	else if(GIRO == IZQ) // Estamos en un cruce y debemos girar a la izquierda
	{
		// Detectamos el primer negro por la izquierda y borramos a partir de los 2 sensores siguientes hacia la derecha 
		if(aux_act1 < aux_ant-1)
		{
			for(i=0; i<8; i++)
				LINEAresult[i] = 0;
			aux_act1 = aux_ant;
			giro = IZQ;
			linea_buena = 0;
		}
		else
		{
			for(i=7; LINEAresult[i] == 0; i--) {}
			i -= 2;
			for(i; i>-1; i--)
				LINEAresult[i] = 0;
			linea_buena = 1;
		}
	}
}

void borrar_marca(void)
{
	// Detectamos el primer 1 por la derecha en los instantes anterior y actual
	for (aux_ant=0; LINEAanterior[aux_ant] == 0; aux_ant++) {}
	for (aux_act1=0; LINEAresult[aux_act1] == 0; aux_act1++) {}
	for (aux_act2=aux_act1; LINEAresult[aux_act2] == 1; aux_act2++) {}
	for (aux_act2; LINEAresult[aux_act2] == 0; aux_act2++) {}

	if(aux_ant > aux_act1 + 1) // Si los negros detectados están separados, el giro es a la derecha
		GIRO = DER;
	else
		GIRO = IZQ;

	// Borrar marca de giro
	if(GIRO == DER)
	{
		i = aux_act2-2;
		for(i; i>-1; i--)
			LINEAresult[i] = 0;
		aux_act1 = aux_act2;
	}
	else
	{
		i = aux_act2;
		for(i; i<8; i++)
			LINEAresult[i] = 0;
	}
}

void seguir_linea(void)
{
	anterior=posicion;
	calc_variacion();
	k=1-(abs(posicion-anterior)/20);
	if (GIRO == IZQ)	// Si el giro en la bifurcación es a la izquierda se da preferencia al giro a la izquierda
	{
		if(LINEAresult[7] == 1)									// Secuencia 10000000
		{				
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<30);
			IO1PIN &= ~(1<<31);
			PWMMR2 = k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = reductor_gran*k * velocidad_rastreo*Per_pwm*compensa_med*Fpclk;
			giro = IZQ;
		}
		else if(LINEAresult[7] == 1 && LINEAresult[6] == 1)	// Secuencia 11000000
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<30);
			IO1PIN &= ~(1<<31);
			PWMMR2 = k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = reductor_GRAN*k * velocidad_rastreo*Per_pwm*compensa_med*Fpclk;
			giro = IZQ;	   			
		}

		else if(LINEAresult[6] == 1)					// Secuencia 01000000
		{		 		
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = reductor_med * k * velocidad_rastreo*Per_pwm*compensa_gran*Fpclk;
			giro = IZQ;
		}
		else if(LINEAresult[6] == 1 && LINEAresult[5] == 1)	// Secuencia 01100000
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = reductor_GRAN * k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = reductor_peq * k * velocidad_rastreo*Per_pwm*compensa_med*Fpclk;
			giro = IZQ;
		}
		else if(LINEAresult[5] == 1)					// Secuencia 00100000
		{			  		
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = reductor_med * k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = k * velocidad_rastreo*Per_pwm*compensa_med*Fpclk;
			giro = IZQ;
		}
		else if(LINEAresult[5] == 1 && LINEAresult[4] == 1)	// Secuencia 00110000
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = reductor_GRAN * k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = reductor_peq * k * velocidad_rastreo*Per_pwm*compensa_med*Fpclk;
			giro = IZQ;
		}
		else if(LINEAresult[4] == 1)					// Secuencia 00010000
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = reductor_peq * k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = k * velocidad_rastreo*compensa_peq*Per_pwm*Fpclk;
			giro = IZQ;
		}
		else if(LINEAresult[4] == 1 && LINEAresult[3] == 1)	// Secuencia 00011000
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = k * velocidad_rastreo*compensa*Per_pwm*Fpclk;
		}
		else if(LINEAresult[3] == 1)					// Secuencia 00001000
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = reductor_peq * k * velocidad_rastreo*compensa_peq*Per_pwm*Fpclk;
			giro = DER;
		}
		else if(LINEAresult[3] == 1 && LINEAresult[2] == 1)	// Secuencia 00001100
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = reductor_peq * k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = reductor_med * k * velocidad_rastreo*compensa_peq*Per_pwm*Fpclk;
			giro = DER;
		}
		else if(LINEAresult[2] == 1)					// Secuencia 00000100
		{	   		
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = reductor_med * k * velocidad_rastreo*compensa_med*Per_pwm*Fpclk;
			giro = DER;
		}
		else if(LINEAresult[2] == 1 && LINEAresult[1] == 1)	// Secuencia 00000110
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = reductor_med * k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = reductor_GRAN * k * velocidad_rastreo*compensa_med*Per_pwm*Fpclk;
			giro = DER;	  			
		}
		else if(LINEAresult[1] == 1)							// Secuencia 00000010
		{					  		
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = reductor_med * k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = reductor_gran * k * velocidad_rastreo*compensa_gran*Per_pwm*Fpclk;
			giro = DER;
		}
		else if(LINEAresult[1] == 1 && LINEAresult[0] == 1)	// Secuencia 00000011
		{
			IO0PIN |= (1<<20);
			IO0PIN &= ~(1<<23);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = reductor_peq * k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = reductor_gran * k * velocidad_rastreo*compensa_gran*Per_pwm*Fpclk;
			giro = DER;			
		}
		else if(LINEAresult[0] == 1)							// Secuencia 00000001
		{		
			IO0PIN |= (1<<20);
			IO0PIN &= ~(1<<23);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = 0*k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = k * velocidad_rastreo*compensa_med*Per_pwm*Fpclk;
			giro = DER;
		}
		else											// Secuencia 00000000
		{
			if(giro == IZQ)	// Si la secuencia anterior es 10000000
			{				  			
				IO0PIN |= (1<<23);
				IO0PIN &= ~(1<<20);
				IO1PIN |= (1<<30);
				IO1PIN &= ~(1<<31);
				PWMMR2 = reductor_med*k * velocidad_rastreo*Per_pwm*Fpclk;
				PWMMR4 = reductor_med*k * velocidad_rastreo*compensa_med*Per_pwm*Fpclk;
			}
			else			// Si la secuencia anterior es 00000001
			{	 			
				IO0PIN |= (1<<20);
				IO0PIN &= ~(1<<23);
				IO1PIN |= (1<<31);
				IO1PIN &= ~(1<<30);
				PWMMR2 = reductor_med*k * velocidad_rastreo*Per_pwm*Fpclk;
				PWMMR4 = reductor_med*k * velocidad_rastreo*Per_pwm*compensa_med*Fpclk;
			}
		}
	}
	else // GIRO = DER. Si el giro en la bifurcación es a la derecha se da preferencia al giro a la derecha
	{
		if(LINEAresult[0] == 1)								// Secuencia 00000001
		{		
			IO0PIN |= (1<<20);
			IO0PIN &= ~(1<<23);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = 0*k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = k * velocidad_rastreo*compensa_med*Per_pwm*Fpclk;
			giro = DER;
		}
		else if(LINEAresult[1] == 1 && LINEAresult[0] == 1)	// Secuencia 00000011
		{
			IO0PIN |= (1<<20);
			IO0PIN &= ~(1<<23);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = reductor_peq * k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = reductor_gran * k * velocidad_rastreo*compensa_gran*Per_pwm*Fpclk;
			giro = DER;			
		}
		else if(LINEAresult[1] == 1)						// Secuencia 00000010
		{					  		
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = reductor_med * k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = k * velocidad_rastreo*compensa_gran*Per_pwm*Fpclk;
			giro = DER;
		}
		else if(LINEAresult[2] == 1 && LINEAresult[1] == 1)	// Secuencia 00000110
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = reductor_med * k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = reductor_GRAN * k * velocidad_rastreo*compensa_med*Per_pwm*Fpclk;
			giro = DER;	  			
		}
		else if(LINEAresult[2] == 1)					// Secuencia 00000100
		{	   		
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = reductor_med * k * velocidad_rastreo*compensa_med*Per_pwm*Fpclk;
			giro = DER;
		}
		else if(LINEAresult[3] == 1 && LINEAresult[2] == 1)	// Secuencia 00001100
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = reductor_peq * k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = reductor_med * k * velocidad_rastreo*compensa_peq*Per_pwm*Fpclk;
			giro = DER;
		}
		else if(LINEAresult[3] == 1)					// Secuencia 00001000
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = reductor_peq * k * velocidad_rastreo*compensa_peq*Per_pwm*Fpclk;
		}
		else if(LINEAresult[3] == 1 && LINEAresult[4] == 1)	// Secuencia 00011000
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = k * velocidad_rastreo*compensa*Per_pwm*Fpclk;
		}
		else if(LINEAresult[4] == 1)					// Secuencia 00010000
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = reductor_peq * k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = k * velocidad_rastreo*compensa_peq*Per_pwm*Fpclk;
		}
		else if(LINEAresult[4] == 1 && LINEAresult[5] == 1)	// Secuencia 00110000
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = reductor_peq * k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = k * velocidad_rastreo*Per_pwm*compensa_med*Fpclk;
			giro = IZQ;
		}
		else if(LINEAresult[5] == 1)					// Secuencia 00100000
		{			  		
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = reductor_med * k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = k * velocidad_rastreo*Per_pwm*compensa_med*Fpclk;
			giro = IZQ;
		}
		else if(LINEAresult[6] == 1 && LINEAresult[5] == 1)	// Secuencia 01100000
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = reductor_med * k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = reductor_peq * k * velocidad_rastreo*Per_pwm*compensa_med*Fpclk;
			giro = IZQ;
		}
		else if(LINEAresult[6] == 1)					// Secuencia 01000000
		{		 		
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = reductor_GRAN * reductor_med * k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = reductor_peq * k * velocidad_rastreo*Per_pwm*compensa_gran*Fpclk;
			giro = IZQ;
		}
		else if(LINEAresult[7] == 1 && LINEAresult[6] == 1)	// Secuencia 11000000
		{
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<31);
			IO1PIN &= ~(1<<30);
			PWMMR2 = reductor_GRAN*k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = reductor_peq * k * velocidad_rastreo*Per_pwm*compensa_med*Fpclk;
			giro = IZQ;	   			
		}

		else if(LINEAresult[7] == 1)					// Secuencia 10000000
		{				
			IO0PIN |= (1<<23);
			IO0PIN &= ~(1<<20);
			IO1PIN |= (1<<30);
			IO1PIN &= ~(1<<31);
			PWMMR2 = k * velocidad_rastreo*Per_pwm*Fpclk;
			PWMMR4 = 0*k * velocidad_rastreo*Per_pwm*compensa_med*Fpclk;
			giro = IZQ;
		}
		else											// Secuencia 00000000
		{
			if(giro == DER)	// Si la secuencia anterior es 00000001
			{	 			
				IO0PIN |= (1<<20);
				IO0PIN &= ~(1<<23);
				IO1PIN |= (1<<31);
				IO1PIN &= ~(1<<30);
				PWMMR2 = reductor_med*k * velocidad_rastreo*Per_pwm*Fpclk;
				PWMMR4 = reductor_med*k * velocidad_rastreo*Per_pwm*compensa_med*Fpclk;
			}
			else			// Si la secuencia anterior es 10000000
			{				  			
				IO0PIN |= (1<<23);
				IO0PIN &= ~(1<<20);
				IO1PIN |= (1<<30);
				IO1PIN &= ~(1<<31);
				PWMMR2 = reductor_med*k * velocidad_rastreo*Per_pwm*Fpclk;
				PWMMR4 = reductor_med*k * velocidad_rastreo*compensa_med*Per_pwm*Fpclk;
			}
		}
	}

	PWMLER = (1<<0)|(1<<2)|(1<<4);	// Enable PWM Match 0, 2 & 4 Latch
}

void Actualizar(void)
{
	// Variables para la detección de secuencia
	nSensores = 0;
	hueco1 = 0; hueco2 = 0;
	secuencia_anterior = secuencia_actual;
	secuencia_actual = 0;

	// Variables para la detección de línea
	for (i=0; i<8; i++)
		LINEAanterior[i] = LINEAresult[i];

	// Variable para la detección de la marca de giro
	aux_ant = aux_act1;

	// Contador
	cont++;
}

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
	if(ESTADO != Parado)
	{
		// Digitalizar el resultado
		for(i=0; i<8; i++)
		{
			if(ADCresult[i] > umbral)
				LINEAresult[i] = 1;
			else
				LINEAresult[i] = 0;
		}
		
		// Detectar secuencias de línea
		detectar_secuencias();

		// Cambios de estado
		if(ESTADO == Rastrear)
		{
			if(secuencia_actual == 101 && secuencia_anterior == 111)
			{
				ESTADO = Bifurcacion;
				velocidad_rastreo = velocidad2;
			}
			else if(secuencia_actual == 101 && habilitar_marca == 1)
			{
				ESTADO = Linea_giro;
				velocidad_rastreo = velocidad2;
			}		  
		}
		else if(ESTADO == Linea_giro)
		{
			if(secuencia_actual == 0)
			{
				ESTADO = Bifurcacion;
				habilitar_marca = 0;
				cont = 0;
			}
		}
		else // ESTADO == Bifurcacion
		{
			if(secuencia_actual != 101 && linea_buena == 1)
			{
				velocidad_rastreo = velocidad1;
				habilitar_marca = 1;
				ESTADO = Rastrear;
			}
		}

		// Ejecutar seguimiento
		if (ESTADO == Linea_giro)
			borrar_marca();
		else if (ESTADO == Bifurcacion && nSensores > 2)
			borrar_linea();

		seguir_linea();

		// Actualizar variables
		Actualizar();
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
	T0MR2 = (unsigned long)(Periodo*Fpclk);
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
	PWMMR0 = Per_pwm * Fpclk;		// Periodo PWM									   
	PWMLER = (1<<0)|(1<<2)|(1<<4);	// Habilitar MATCH0, MATCH2 y MATCH4
	PWMMCR = 0x02;					// Reset TIMER COUNTER REGISTER ON MATCH0							   
	PWMPCR = (1<<10)|(1<<12);		// Habilitar PWM2 y PWM4 en flanco
	PWMTCR = (1<<0)|(1<<3);			// Habilitar PWM y comenzar la cuenta
}
 
// Configuración de EINT1
void config_EINT1(void)
{
	PINSEL0 |= 1<<29;	//ó 3<<6
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
	EXTMODE |= 1<<3;
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
		velocidad1 = 0.9;
	else
		velocidad1 = 0.4;
	velocidad_rastreo = velocidad1;

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
