#include <lpc213x.h>
#include <stdlib.h>
#include <stdio.h>
#include "Declaraciones.h"
#include "PID.h"
#include "s1d15g00.h"

/************************ INTERRUPCIONES ************************/
// Interrupción IRQ asociada a EINT1. Pulsador de LOAD
void EXTERNA1(void)__irq
{
	if(ESTADO == Parado && start == '1')
	{
		IO0PIN &= ~(1<<27);
		IO0PIN |= 1<<31;
		IO1PIN |= 1<<22;
		IO1PIN &= ~(1<<23);

		velocidad = v_rapida;
				
		integral = 0;
		last_proportional = 0;

		ESTADO = Espera;

		// Selección de pared a seguir
		if((IO1PIN&(1<<21)) == 0)
			PARED = DER;
		else
			PARED = IZQ;
	}

	EXTINT |= 1<<1;
	VICVectAddr = 0;
}

// Interrupción IRQ asociada a EINT3. Pulsador ROJO
void EXTERNA3(void)__irq
{
	if(ESTADO != Parado && ESTADO != Ejecucion)
	{
		IO0PIN |= (1<<20)|(1<<23);
		IO1PIN |= (1<<30)|(1<<31);
		PWMMR2 = (float)velocidad*Per_pwm*Fpclk;
		PWMMR4 = (float)velocidad*Per_pwm*Fpclk;
		PWMLER = (1<<0)|(1<<2)|(1<<4);	// Enable PWM Match 0, 2 & 4 Latch

		IO1PIN |= 1<<22;
		IO1PIN &= ~(1<<23);
		IO0PIN &= ~(1<<27);
		IO0PIN |= 1<<31;

		ESTADO = Parado;
	}

	EXTINT |= 1<<3;
	VICVectAddr = 0;
}

// Función que realiza la lectura de los sensores de distancia
void ADC1(void)__irq
{
	// Se toman los valores de los sensores y se guardan en un array
	r = AD1DR;

	// Selección del canal convertido en ADC0
	ch = (r>>24)&0x07;

	// Guardar resultado de la conversión
	ADCresult[ch] = (r>>6)&0x03FC;

	VICVectAddr = 0;
}

// Interrupción para el capture1.2 y capture1.3
void TIMER1(void)__irq
{
	if(T1IR&(1<<4))
	{

		T1IR |= 1<<4;		// Activar flag de interrupción del Capture1.0
	}
	else if(T1IR&(1<<6))
	{

		T1IR |= 1<<6;		// Activar flag de interrupción del Capture1.2
	}

	VICVectAddr = 0;	 
}

// Interrupción IRQ para el TIMER0
void TIMER0(void)__irq
{
	if(ESTADO == Seleccion_velocidad)
		selec_velocidad();
	if(ESTADO == Parado)
	{
		if(cont > 0.2)
		{
			if(t == 1)
			{
				IO0PIN &= ~(1<<31);
				IO1PIN &= ~(1<<22);
				t = 0;
			}
			else
			{
				IO0PIN |= 1<<31;
				IO1PIN |= 1<<22;
				t = 1;
			}
			cont = 0;
		}

		cont += Periodo;
	}
	else if(ESTADO == Espera)
	{
		leer_sensores();

		if(sensor_frontal > 200)
			ESTADO = Ejecucion;
	}
	
	if(ESTADO == Ejecucion)
	{
		leer_sensores();

		// Ejecución del seguimiento de la pared
		if(ESTADO == Ejecucion)
			loop_PID();


		if(PARED == IZQ)
		{
			IO0PIN &= ~((1<<27)|(1<<31));
			IO1PIN |= (1<<22)|(1<<23);
		}
		else // PARED ==DER
		{
			IO0PIN |= (1<<27)|(1<<31);
			IO1PIN &= ~((1<<22)|(1<<23));
		}

		PWMLER = (1<<0)|(1<<2)|(1<<4);	// Enable PWM Match 0, 2 & 4 Latch
	}
	
	T0IR |= (1<<2);		// Activar flag de interrupción del Match0.2
	VICVectAddr = 0;
}
