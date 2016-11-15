#include <lpc213x.h>
#include <stdlib.h>
#include "Declaraciones.h"
#include "PID.h"

/************************ INTERRUPCIONES ************************/
// Interrupción IRQ asociada a EINT1. Pulsador de LOAD
void EXTERNA1(void)__irq
{
	if(ESTADO == Parado)
	{
		IO0PIN &= ~(1<<31);
		IO1PIN &= ~(1<<22);
		velocidad = v_rapida;
		if(IO1PIN&(1<<21))
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
		IO1PIN |= 1<<22;
		IO0PIN |= 1<<31;
		ESTADO = Parado;
	}
	else if(ESTADO == Calcular_umbral)
	{
		ESTADO = Parado;
		IO1PIN |= 1<<22;
		IO0PIN |= 1<<31;
	}
	else if(ESTADO != Parado)
	{
		PWMMR2 = 0;
		PWMMR4 = 0;
		PWMLER = (1<<0)|(1<<2)|(1<<4);	// Enable PWM Match 0, 2 & 4 Latch
		ESTADO = Parado;
		IO0PIN |= 1<<31;
		IO1PIN |= 1<<22;
		giro = '0';
		GIRO = REC;
	}

	EXTINT |= 1<<3;
	VICVectAddr = 0;
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
			IO1PIN |= 1<<22;
			IO0PIN &= ~(1<<31);
		}
	}
	else if(ESTADO == Calcular_umbral)
		calcular_umbral();
	else if(ESTADO == Espera)
	{
		cont++;
		if(cont*Periodo > 3)
		{
			ESTADO = Rastrear;
			velocidad = v_rapida;
		}
	}
	else
	{
		// Digitalizar el resultado	de la conversión ADC
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
				if(GIRO_aux_izq == '1' && GIRO_aux_der == '1')
					GIRO_aux = REC;
				else if(GIRO_aux_izq == '1' && GIRO_aux_der == '0')
					GIRO_aux = IZQ;
				else // GIRO_aux_izq == '0' && GIRO_aux_der == '1'
					GIRO_aux = DER;
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
			}
		}
		else if(ESTADO == Bifurcacion_proxima)
		{
			if(cont*Periodo > 2)
			{
				ESTADO = Rastrear;
				velocidad = v_rapida;
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
			{
				velocidad = v_rapida;
				ESTADO = Rastrear;
			}
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
