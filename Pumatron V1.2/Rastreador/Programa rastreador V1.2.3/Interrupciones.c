#include <lpc213x.h>
#include <stdlib.h>
#include <stdio.h>
#include "Declaraciones.h"
#include "PID.h"
#include "s1d15g00.h"

/************************ INTERRUPCIONES ************************/
// Interrupción IRQ asociada a EINT3. Pulsador ROJO
void EXTERNA3(void)__irq
{
	if(ESTADO != Parado)
	{
		IOSET0 = 1<<20;
		IOSET0 = 1<<23;
		IOSET1 = 1<<30;
		IOSET1 = 1<<31;
		PWMMR2 = (float)velocidad*Per_pwm*Fpclk;
		PWMMR4 = (float)velocidad*Per_pwm*Fpclk;
		PWMLER = (1<<0)|(1<<2)|(1<<4);	// Enable PWM Match 0, 2 & 4 Latch

		IOCLR0 = 1<<27;
		IOSET0 = 1<<31;
		IOSET1 = 1<<22;
		IOCLR1 = 1<<23;
		IO0PIN &= ~(1<<27);
		IO1PIN &= ~(1<<23);
		IO1PIN |= 1<<22;
		IO0PIN |= 1<<31;

		if(ESTADO == Seleccion_velocidad)
			regular_PID();

		for(i=0; i<S; i++)
		{
			mensaje[0] = 'X';
			mensaje_anterior[i] = 'X';
		}
		rep = 1;

		start = 0;
		ESTADO = Parado;
	}


	EXTINT |= 1<<3;
	VICVectAddr = 0;
}

// Interrupción para el capture1.2 y capture1.3
void TIMER1(void)__irq
{
	if(T1IR&(1<<6))
	{

		T1IR |= 1<<6;		// Activar flag de interrupción del Capture1.2
	}
	else if(T1IR&(1<<7))
	{

		T1IR |= 1<<7;		// Activar flag de interrupción del Capture1.3
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

			IOCLR0 = 1<<31;
			IOSET1 = 1<<22;
		}
		else
		{
			if((IO0PIN&(1<<14)) == 0 && start < 0.5)
				start += Periodo;
			else if((IO0PIN&(1<<14)) == 0 && start >= 0.5)
			{
				IOSET0 = 1<<31;
				IOSET1 = 1<<22;
			}
			else if((IO0PIN&(1<<14)) != 0 && start >= 0.5)
			{
				IOCLR0 = 1<<31;
				IOCLR1 = 1<<22;
				
				velocidad = 0.4;
				giro = '0';
				GIRO = REC;
		
				integral = 0;
				last_proportional = 0;
		
				if(seg_espera == 0)
					ESTADO = Rastrear;
				else
				{
					cont = 0;
					ESTADO = Espera;
				}
			}
			else
				start = 0;

			if(start < 0.5)
			{
				if(cont > 0.2)
				{
					if(t == 1)
					{
						IOCLR0 = 1<<31;
						IOCLR1 = 1<<22;
						t = 0;
					}
					else
					{
						IOSET0 = 1<<31;
						IOSET1 = 1<<22;
						t = 1;
					}
					cont = 0;
				}
	
				cont += Periodo;
			}
		}
	}
	else if(ESTADO == Umbralizar_sensores)
	{
		leer_sensores();
		estimar_umbrales();
	}
	else if(ESTADO == Espera)
	{
		cont += Periodo;
		if(cont >= seg_espera)
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
				GIRO_aux = '0';

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
				{
					GIRO_aux = IZQ;
					giro = IZQ;
				}
				else //if(GIRO_aux_izq == '0' && GIRO_aux_der == '1')
				{
					GIRO_aux = DER;
					giro = DER;
				}
			}

			if(secuencia_actual == '1')
			{
				ESTADO = Bifurcacion_proxima;
				cont = 0;

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
			else if(secuencia_actual == '7')
			{
				velocidad = v_lenta;
				ESTADO = Bifurcacion;

				if(GIRO_aux_izq == '1' && GIRO == IZQ && LINEAresult[S-1] == '0')
				{
					for(i=aux_act[0]; i<aux_act[2]+1; i++)
						LINEAresult[i] = '0';
					aux_act[2] = S-1;
					linea_buena = false;
					negros = 0;
					secuencia_actual = '1';
					giro = IZQ;
					last_proportional = S/2*1000;
				}
				else if(GIRO_aux_der == '1' && GIRO == DER && LINEAresult[0] == '0')
				{
					for(i=aux_act[0]; i<aux_act[2]+1; i++)
						LINEAresult[i] = '0';
					aux_act[0] = 0;
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
			if(cont > 1)
			{
				ESTADO = Rastrear;
				//velocidad = v_rapida;
			}
			else if(secuencia_actual == '2' || secuencia_actual == '3')
			{
				GIRO_aux = '0';

				if(secuencia_anterior == '7')
					ESTADO = Bifurcacion;
				else
				{
					ESTADO = Linea_giro;
					GIRO_aux_izq = '0';
					GIRO_aux_der = '0';
				}
			}
			cont += Periodo;	
		}
		else // ESTADO == Bifurcacion
		{
			if(secuencia_actual == '1' && linea_buena == true)
				ESTADO = Rastrear;
		}

		// Ejecutar seguimiento
		if(ESTADO == Linea_giro)
			borrar_marca();
		else if((ESTADO == Bifurcacion) || (ESTADO == Bifurcacion_proxima && secuencia_actual == '7'))
			borrar_linea();
		loop_PID();

		//enviar_secuencias(LINEAresult);
		//representar_sensores(ANALOGICO);
		//representar_estado();
				
		// Actualizar variables
		actualizar();
	}
	
	T0IR |= (1<<2);		// Activar flag de interrupción del Match0.2
	VICVectAddr = 0;
}

void UART0(void)__irq
{
	switch(U0IIR & 0x0E)
	{
		case 0x04:	// Recepción
			dato_UART = U0RBR;
			break;
		case 0x02:	// Transmisión
    		if(*ptr_tx == 0)
			{
				if(mensajes_ptes == SI) // Quedan mensajes por enviar
				{
					ptr_tx = buffer_tx[ptr_rd]; // Se busca el nuevo mensaje para enviar
					ptr_rd++;
					ptr_rd &= N_MENSAJES - 1;
					if(ptr_rd == ptr_wr)
						mensajes_ptes = NO;	// Se va a enviar el ultimo mensaje del buffer
				   	U0THR = *ptr_tx;
					ptr_tx++;
				}
				else
					reposo=1;
			}
			else	// Se continúa mandando el mensaje
				U0THR=*ptr_tx++;
	}	
    
	VICVectAddr = 0;	 
}
