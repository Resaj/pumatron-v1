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
		IO0PIN |= 1<<31;
		IO1PIN |= 1<<22;
		
		velocidad = 0.4;
		giro = '0';
		GIRO = REC;

		integral = 0;
		last_proportional = 0;

		ESTADO = Espera;
	}

	EXTINT |= 1<<1;
	VICVectAddr = 0;
}

// Interrupción IRQ asociada a EINT3. Pulsador ROJO
void EXTERNA3(void)__irq
{
	if(ESTADO == Seleccion_velocidad || ESTADO == Umbralizar_sensores)
	{
		IO0PIN |= (1<<20)|(1<<23);
		IO1PIN |= (1<<30)|(1<<31);
		PWMMR2 = (float)velocidad*Per_pwm*Fpclk;
		PWMMR4 = (float)velocidad*Per_pwm*Fpclk;
		PWMLER = (1<<0)|(1<<2)|(1<<4);	// Enable PWM Match 0, 2 & 4 Latch

		IO0PIN &= ~(1<<27);
		IO1PIN &= ~(1<<23);
		IO1PIN |= 1<<22;
		IO0PIN |= 1<<31;

		ESTADO = Parado;
	}

	EXTINT |= 1<<3;
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
	else if(ESTADO == Parado)
	{
		if((IO0PIN&(1<<3)) == 0)
		{
			for(i=0; i<S; i++)
			{
				blanco[i] = 1023;
				negro[i] = 0;
			}
			ESTADO = Umbralizar_sensores;
			cond = '0';
			IO1PIN |= 1<<22;
			IO0PIN &= ~(1<<31);
		}
		else
		{
			// Selección de línea
			if((IO1PIN&(1<<21)) == 0)
			{
				LINEA = IZQ;
				IO0PIN &= ~(1<<27);
				IO1PIN |= 1<<23;
			}
			else
			{
				LINEA = DER;
				IO0PIN |= 1<<27;
				IO1PIN &= ~(1<<23);
			}

			if(cont > 0.2)
			{
				if(t == 1)
				{
					IO1PIN &= ~(1<<22);
					IO0PIN &= ~(1<<31);
					t = 0;
				}
				else
				{
					IO1PIN |= 1<<22;
					IO0PIN |= 1<<31;
					t = 1;
				}
				cont = 0;
			}

			cont += Periodo;
		}
	}
	else if(ESTADO == Umbralizar_sensores)
	{
		leer_sensores();
		estimar_umbrales();
	}
	else if(ESTADO == Espera)
	{
		leer_sensor_frontal();
		if(sensor_frontal > 200)
		{
			ESTADO = Rastrear;
			IO0PIN &= ~(1<<31);
			IO1PIN &= ~(1<<22);
		}

//		sprintf(cadena, "sensor_frontal %4d", sensor_frontal);   
//		LCDPutStr(cadena, 100, 10, SMALL, WHITE, BLACK);
	}
	else
	{
		// Digitalizar el resultado	de la conversión ADC
		leer_sensores();
		//leer_sensor_frontal();
		calibrar_sensores();
		digitalizar();		

		if(ESTADO == Avanzar1)
		{
			if(cont*Periodo > 0.4)
			{
				ESTADO = Avanzar2;
				cont = 0;
			}

			cont++;
		}
		else if(ESTADO == Avanzar2)
		{
			if(LINEAresult[0] == 1 || LINEAresult[1] == 1 || LINEAresult[2] == 1 || LINEAresult[3] == 1)
			{
				LINEA = DER;
				ESTADO = Rastrear;
				cont = 0;
			}
			else if(LINEAresult[S-2] == 1 || LINEAresult[S-1] == 1  || LINEAresult[S-3] == 1 || LINEAresult[S-4] == 1)
			{
				LINEA = IZQ;
				ESTADO = Rastrear;
				cont = 0;
			}
//			if((LINEA == IZQ && (LINEAresult[0] == 1 || LINEAresult[1] == 1)) || (LINEA == DER && (LINEAresult[S-2] == 1 || LINEAresult[S-1] == 1)))
//			{
//				if(LINEA == IZQ)
//					LINEA = DER;
//				else // LINEA == DER
//					LINEA = IZQ;
//				ESTADO = Rastrear;
//			}
//			cont = 0;
		}
		else if(ESTADO == Rastrear)
		{
			if((IO0PIN&(1<<12)) != 0)
			{
				ESTADO = Obstaculo;
				cont = 0;
			}
		}
		else if(ESTADO == Obstaculo)
		{
			if(cont*Periodo > 0.6)
			{
				ESTADO = Avanzar1;
				cont = 0;
			}
			cont++;
		}

		// Ejecución del seguimiento
		if(ESTADO == Avanzar1)
		{
			IO1PIN &= ~(1<<30);
			IO1PIN |= 1<<31;
			IO0PIN &= ~(1<<20);
			IO0PIN |= 1<<23;
			if(LINEA == DER)
			{
				PWMMR2 = 0.5*velocidad * Per_pwm*Fpclk;
				PWMMR4 = velocidad * Per_pwm*Fpclk;
			}
			else
			{
				PWMMR2 = velocidad * Per_pwm*Fpclk;
				PWMMR4 = 0.5*velocidad * Per_pwm*Fpclk;
			}
			PWMLER = (1<<0)|(1<<2)|(1<<4);	// Enable PWM Match 0, 2 & 4 Latch
		}
		else if(ESTADO == Avanzar2)
		{
			IO1PIN &= ~(1<<30);
			IO1PIN |= 1<<31;
			IO0PIN &= ~(1<<20);
			IO0PIN |= 1<<23;
			if(LINEA == DER)
			{
				PWMMR2 = velocidad * Per_pwm*Fpclk;
				PWMMR4 = 0.7*velocidad * Per_pwm*Fpclk;
			}
			else
			{
				PWMMR2 = 0.7*velocidad * Per_pwm*Fpclk;
				PWMMR4 = velocidad * Per_pwm*Fpclk;
			}
			PWMLER = (1<<0)|(1<<2)|(1<<4);	// Enable PWM Match 0, 2 & 4 Latch
		}
		else if(ESTADO == Rastrear)
			loop_PID();			
		else if(ESTADO == Obstaculo)
		{
			IO0PIN |= (1<<20);
			IO0PIN &= ~(1<<23);
			IO1PIN |= (1<<30);
			IO1PIN &= ~(1<<31);
			PWMMR2 = 0.5*velocidad * Per_pwm*Fpclk;
			PWMMR4 = 0.5*velocidad * Per_pwm*Fpclk;
			PWMLER = (1<<0)|(1<<2)|(1<<4);	// Enable PWM Match 0, 2 & 4 Latch
		}

//		for(i = 0; i < S; i++)
//		{
///*			sprintf(cadena, "%4d", umbral[i]);   
//			LCDPutStr(cadena, (120-(i*10)), 25, SMALL, WHITE, BLACK);
//*/			if(ADCresult[i] > umbral[i])
//				LCDSetRect((120-(i*10)), 10, (125-(i*10)), 15, 1, RED);
//			else
//				LCDSetRect((120-(i*10)), 10, (125-(i*10)), 15, 1, WHITE);
//		}
//		sprintf(cadena, "EST %c", ESTADO);   
//		LCDPutStr(cadena, 86, 60, SMALL, WHITE, BLACK);
		
		// Actualizar variables
		actualizar();
	}
	
	T0IR |= (1<<2);		// Activar flag de interrupción del Match0.2
	VICVectAddr = 0;
}
