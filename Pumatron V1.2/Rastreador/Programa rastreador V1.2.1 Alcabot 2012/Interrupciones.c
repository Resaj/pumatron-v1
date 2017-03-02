#include <lpc213x.h>
#include <stdlib.h>
#include "Declaraciones.h"
#include "PID.h"
#include "s1d15g00.h"
#include <stdio.h>

/************************ INTERRUPCIONES ************************/
// Interrupción IRQ asociada a EINT1. Pulsador de LOAD
void EXTERNA1(void)__irq
{
	if(ESTADO == Parado)
	{
		IO0PIN &= ~(1<<31);
		IO1PIN &= ~(1<<22);
		
		if(v_rapida < 0.5)
			velocidad = v_rapida;
		else
			velocidad = 0.4;
		giro = '0';

		if(IO1PIN&(1<<21))
		{
			opcion_recta = '0';
			GIRO = '0';
		}
		else
		{
			opcion_recta = '1';
			GIRO = REC;
		}

		if(seg_espera == 0)
			ESTADO = Rastrear;
		else
		{
			cont = 0;
			ESTADO = Espera;
		}
	}

	EXTINT |= 1<<1;
	VICVectAddr = 0;
}

// Interrupción IRQ asociada a EINT3. Pulsador ROJO
void EXTERNA3(void)__irq
{
	if(ESTADO == Seleccion_velocidad)
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
	else if(ESTADO == Calcular_umbral)
	{
		IO1PIN |= 1<<22;
		IO0PIN |= 1<<31;
		ESTADO = Parado;

//		for(i = 0; i < S; i++)
//		{
//			sprintf(cadena, "%4d", umbral[i]);   
//			LCDPutStr(cadena, (120-(i*10)), 25, SMALL, WHITE, BLACK);
///*			if(ADCresult[i] > umbral[i])
//				LCDSetRect((120-(i*10)), 10, (125-(i*10)), 15, 1, RED);
//			else
//				LCDSetRect((120-(i*10)), 10, (125-(i*10)), 15, 1, WHITE);
//*/		}
	}
	else if(ESTADO != Parado)
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

		integral = 0;
		last_proportional = 0;

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
			ESTADO = Calcular_umbral;
			cond = '0';
			IO1PIN |= 1<<22;
			IO0PIN &= ~(1<<31);
		}
		else
		{
			if(cont*Periodo > 0.2)
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

			cont++;
		}
	}
	else if(ESTADO == Calcular_umbral)
	{
		leer_sensores();
		calcular_umbral();
	}
	else if(ESTADO == Espera)
	{
		cont++;
		if(cont*Periodo > seg_espera)
			ESTADO = Rastrear;
	}
	else
	{
		// Digitalizar el resultado	de la conversión ADC
		leer_sensores();
		digitalizar();
		
		// Detectar secuencias de línea
		detectar_secuencias();

		// Cambios de estado
		if(ESTADO == Rastrear)
		{
			if(secuencia_actual == '2' || secuencia_actual == '3')
			{
				if(secuencia_anterior == '7')
				{
					ESTADO = Bifurcacion;
					if(GIRO != REC)
						velocidad = v_lenta;
				}
				else
				{
					ESTADO = Linea_giro;
					GIRO_aux_izq = '0';
					GIRO_aux_der = '0';
					GIRO_aux = '0';
					aux_der = '0';
					aux_izq = '0';
				}
			}
		}
		else if(ESTADO == Linea_giro)
		{
			if(GIRO_aux != REC)
			{
				if(GIRO_aux_izq == '1' && GIRO_aux_der == '1' && opcion_recta == '1')
					GIRO_aux = REC;
				else if(GIRO_aux_izq == '1' && GIRO_aux_der == '0')
				{
					GIRO_aux = IZQ;
					giro = IZQ;
				}
				else if(GIRO_aux_izq == '0' && GIRO_aux_der == '1')
				{
					GIRO_aux = DER;
					giro = DER;
				}
				else // Estamos en una bifurcación
				{
					velocidad = v_lenta;
					ESTADO = Bifurcacion;
				}
			}

			if(secuencia_actual == '1')
			{
				ESTADO = Bifurcacion_proxima;
				cont = 0;
				GIRO_tomado = '0';

				if(GIRO_aux == REC)
					GIRO = REC;
				else
				{
					velocidad = v_lenta;
	
					if(aux_act[0] < 3 && GIRO_aux == IZQ)
						GIRO = IZQ;
					else if(aux_act[2] > S-4 && GIRO_aux == DER)
						GIRO = DER;
					else if(abs(aux_act[0] - aux_ant[1]) <= 1)
						GIRO = DER;
					else
						GIRO = IZQ;
				}
			}
			else if(secuencia_actual == '7' && GIRO_tomado == '0')
			{
				velocidad = v_lenta;
				ESTADO = Bifurcacion;

				if(GIRO_aux_izq == '1' && GIRO == IZQ && LINEAresult[S-1] == 0)
				{
					for(i=aux_act[0]; i<aux_act[2]+1; i++)
						LINEAresult[i] = 0;
					aux_act[0] = aux_ant[0];
					linea_buena = false;
					negros = 0;
					secuencia_actual = '1';
					giro = IZQ;
					last_proportional = S/2*1000;
				}
				if(GIRO_aux_der == '1' && GIRO == DER && LINEAresult[0] == 0)
				{
					for(i=aux_act[0]; i<aux_act[2]+1; i++)
						LINEAresult[i] = 0;
					aux_act[2] = aux_ant[2];
					linea_buena = false;
					negros = 0;
					secuencia_actual = '1';
					giro = DER;
					last_proportional = -S/2*1000;
				}
			}
		}
		else if(ESTADO == Bifurcacion_proxima)
		{
			if(cont*Periodo > 2)
			{
				ESTADO = Rastrear;
				//velocidad = v_rapida;
			}
			else if(secuencia_actual == '7')
				ESTADO = Bifurcacion;
			else if(secuencia_actual == '2' || secuencia_actual == '3')
			{
				ESTADO = Linea_giro;
				GIRO_aux_izq = '0';
				GIRO_aux_der = '0';
				GIRO_aux = '0';
				aux_der = '0';
				aux_izq = '0';
			}
			cont++;	
		}
		else // ESTADO == Bifurcacion
		{
			if(secuencia_actual == '1' && linea_buena == true)
				ESTADO = Rastrear;
		}

		// Ejecutar seguimiento
		if(ESTADO == Linea_giro)
			borrar_marca();
		else if(ESTADO == Bifurcacion && secuencia_actual != '1')
			borrar_linea();
		loop_PID();
		
		// Actualizar variables
		actualizar();
	}
	
	T0IR |= (1<<2);		// Activar flag de interrupción del Match0.2
	VICVectAddr = 0;
}
