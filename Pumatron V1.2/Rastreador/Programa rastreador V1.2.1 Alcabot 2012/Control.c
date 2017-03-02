#include <lpc213x.h>
#include <stdlib.h>
#include "Declaraciones.h"
#include "s1d15g00.h"

/********************** FUNCIONES DE CONTROL **********************/
void selec_velocidad(void)
{
	if((IO0PIN&(1<<3)) == 0 && flag == 1)
	{
		if(v_rapida == (float)0.4)
		{
			IO0PIN &= ~((1<<27)|(1<<31));
			IO1PIN |= (1<<22)|(1<<23);
			v_rapida = 0.5;
		}
		else if(v_rapida == (float)0.5)
		{
			IO0PIN &= ~(1<<27);
			IO1PIN |= 1<<23;
			IO1PIN &= ~(1<<22);
			IO0PIN |= 1<<31;
			v_rapida = 0.6;
		}
		else if(v_rapida == (float)0.6)
		{
			IO0PIN &= ~(1<<27);
			IO1PIN |= (1<<22)|(1<<23);
			IO0PIN |= 1<<31;
			v_rapida = 0.7;
		}
		else if(v_rapida == (float)0.7)
		{
			IO0PIN |= 1<<27;
			IO1PIN &= ~((1<<22)|(1<<23));
			IO0PIN &= ~(1<<31);
			v_rapida = 0.8;
		}
		else if(v_rapida == (float)0.8)
		{
			IO0PIN |= 1<<27;
			IO1PIN &= ~(1<<23);
			IO1PIN |= 1<<22;
			IO0PIN &= ~(1<<31);
			v_rapida = 0.9;
		}
		else if(v_rapida == (float)0.9)
		{
			IO0PIN |= (1<<27)|(1<<31);
			IO1PIN &= ~(1<<22)|(1<<23);
			v_rapida = 1;
		}
		else // if(v_rapida == 1)
		{
			IO0PIN &= ~((1<<27)|(1<<31));
			IO1PIN |= (1<<23);
			IO1PIN &= ~(1<<22);
			v_rapida = 0.4;
		}
	}
	flag = (IO0PIN&(1<<3))>>3;

	for(t=0; t<100000; t++);	// Anti-rebotes
}

void leer_sensores(void)
{
	for(i=0; i<2; i++)
	{
		for(t=0; t<8; t++)
		{
			if(t != 0 && t != 4)
			{
				AD0CR |= (1<<24)|(1<<t);		/* Start A/D Conversion        */
				while ((AD0DR&(1<<31))==0);		/* Wait for end of A/D Convers.*/
				AD0CR &= ~((1<<24)|(1<<t));		/* Stop A/D Conversion         */
				ADC[t] = (AD0DR>>6)&0x03FF;		/* result of A/D process       */ 
			}
		}

		reordenar();
		conmutar_mux();
	}
}

void calcular_umbral(void)
{
	cont = 0;

	for(i=0; i<S; i++)
	{
		if(ADCresult[i] < blanco[i])
			blanco[i] = ADCresult[i];
		if(ADCresult[i] > negro[i])
			negro[i] = ADCresult[i];

		if((negro[i]-blanco[i]) > 200)
		{
			cont++;
			umbral[i] = (negro[i]+blanco[i])/2;
		}
	}

	if(cont == S)
	{
		IO1PIN &= ~(1<<22);
		IO0PIN |= 1<<31;
	}
}

void reordenar(void)
{
	if(mux == '0')
	{
		ADCresult[4] = ADC[1];
		ADCresult[2] = ADC[2];
		ADCresult[0] = ADC[3];
		ADCresult[6] = ADC[5];
		ADCresult[8] = ADC[6];
		ADCresult[10] = ADC[7];
	}
	else
	{
		ADCresult[5] = ADC[1];
		ADCresult[3] = ADC[2];
		ADCresult[1] = ADC[3];
		ADCresult[7] = ADC[5];
		ADCresult[9] = ADC[6];
		ADCresult[11] = ADC[7];
	}
}

void digitalizar(void)
{
	for(i=0; i<S; i++)
	{
		if(ADCresult[i] > umbral[i])
			LINEAresult[i] = 1;
		else
			LINEAresult[i] = 0;
	}
}

void detectar_secuencias(void)
{
	negros = 0;
	cond = '1';

	// Número de sensores en negro
	for (i=0; i<S; i++)
		negros += LINEAresult[i];

	// Si detectamos 101 hay hueco y consideramos sólo 2 sensores juntos
	for (i=0; i<S; i++)
	{
		if (LINEAresult[i] == 1 && marca[0] == false)
		{
			aux_act[0] = i;
			marca[0] = true;
		}
		else if (LINEAresult[i] == 0 && marca[0] == true && marca[1] == false)
			marca[1] = true;
		else if (LINEAresult[i] == 1 && marca[1] == true && marca[2] == false)
		{
			aux_act[1] = i;
			marca[2] = true;
			secuencia_actual = '2';
		}
		else if (LINEAresult[i] == 0 && marca[2] == true && marca[3] == false)
			marca[3] = true;
		else if (LINEAresult[i] == 1 && marca[3] == true)
		{
			secuencia_actual = '3';
			if(cond == '1')
				cond = '0';
		} 
	}

	for (i=S-1; i>-1; i--)
	{
		if (LINEAresult[i] == 1 && marca[4] == false)
		{
			aux_act[2] = i;
			marca[4] = true;
		}
		else if (LINEAresult[i] == 0 && marca[4] == true && marca[5] == false)
			marca[5] = true;
		else if (LINEAresult[i] == 1 && marca[5] == true && marca[6] == false)
		{
			aux_act[3] = i;
			marca[6] = true;
		}
	}

	// Si hay más de 2 sensores en negro y no hay huecos, tenemos más de 2 sensores juntos en negro
	if (secuencia_actual == '1' && negros > 2)
		secuencia_actual = '7';

	// Evitar tomar el camino malo en la bifurcación si por inercia se ha pasado el bueno
	if(ESTADO == Bifurcacion && GIRO != REC)
	{
		if(GIRO == DER)
		{
			if(aux_act[0] > aux_ant[0] + 1)
			{
				for(i=aux_act[0]; i<aux_act[2]+1; i++)
					LINEAresult[i] = 0;
				aux_act[0] = aux_ant[0];

				giro = DER;
				linea_buena = false;
				negros = 0;
			}
			else
				linea_buena = true;
		}
		else	// GIRO == IZQ
		{
			if(aux_act[2] < aux_ant[2] - 1)
			{
				for(i=aux_act[0]; i<aux_act[2]+1; i++)
					LINEAresult[i] = 0;
				aux_act[2] = aux_ant[2];

				giro = IZQ;
				linea_buena = false;
				negros = 0;
			}
			else
				linea_buena = true;
		}
	}
}

void borrar_marca(void)
{
	if(aux_ant[0] > aux_act[0] + 1)	// Los negros detectados en los instantes actual y anterior por la derecha están separados
		GIRO_aux_der = '1';
	else if(aux_ant[0] < aux_act[0] - 1)
		GIRO_aux_der = '0';
	if(aux_ant[2] < aux_act[2] - 1)	// Los negros detectados en los instantes actual y anterior por la izquierda están separados
		GIRO_aux_izq = '1';
	else if(aux_ant[2] > aux_act[2] + 1)
		GIRO_aux_izq = '0';

	// Prever bifurcación o marca de giro
	if(aux_der == '0' && (GIRO_aux == DER || GIRO_aux_der == '1') && aux_act[0] > 0 && GIRO_tomado == '0')
	{
		GIRO_tomado = '1';	// La marca de giro es correcta
		aux_der = '1';
	}
	else if(aux_izq == '0' && (GIRO_aux == IZQ || GIRO_aux_izq == '1') && aux_act[2] < S-1 && GIRO_tomado == '0')
	{
		GIRO_tomado = '1';	// La marca de giro es correcta
		aux_izq = '1';
	}

	// Borrar marca de giro
	if(GIRO_aux == DER || GIRO_aux_der == '1')
	{
		for(i=aux_act[1]-2; i>=aux_act[0]; i--)
			LINEAresult[i] = 0;
		aux_act[0] = aux_act[1];
	}
	else if(GIRO_aux == IZQ || GIRO_aux_izq == '1')
	{
		for(i=aux_act[3]+2; i<=aux_act[2]; i++)
			LINEAresult[i] = 0;
		aux_act[2] = aux_act[3];
	}
}

void borrar_linea(void)
{
	if(GIRO == DER) // Estamos en un cruce y debemos girar a la derecha
	{
		for(i=aux_act[0]+1; i<S; i++)
			LINEAresult[i] = 0;
	}
	else if(GIRO == IZQ) // Estamos en un cruce y debemos girar a la izquierda
	{
		for(i=aux_act[2]-1; i>-1; i--)
			LINEAresult[i] = 0;
	}
	else if(GIRO == REC && secuencia_actual != '7')	// GIRO == REC y hay más de una línea
	{
		if(abs((float)aux_act[1] - (S-1)/2) < abs((float)aux_act[0] - (S-1)/2))	// aux_act[1] está más cerca del centro que aux_act[0]
		{
			for(i=aux_act[1]-1; i>-1; i--)
				LINEAresult[i] = 0;
			for(i=aux_act[1]+2; i<S; i++)
				LINEAresult[i] = 0;
		}
		else	// aux_act[0] está más cerca del centro que aux_act[1]
		{
			// Se borra sólo el lado izquierdo porque en el derecho no hay línea
			for(i=aux_act[0]+2; i<S; i++)
				LINEAresult[i] = 0;
		}
	}
}

void actualizar(void)
{
	// Variables para la detección de secuencia
	for(i=0; i<7; i++)
		marca[i] = 0;

	secuencia_anterior = secuencia_actual;
	secuencia_actual = '1';

	// Variables para la detección de la marca de giro
	for(i=0; i<4; i++)
		aux_ant[i] = aux_act[i];
	aux_act[0] = 15;		// Se iguala a un número mayor que el número de sensores para evitar errores en estados siguientes
	aux_act[2] = -5;		// Se iguala a un número menor que -1 para evitar errores en estados siguientes

	// Visualización de los datos
	if(GIRO == REC)
	{
		IO0PIN |= 1<<27;
		IO1PIN |= 1<<23;
	}
	else if(GIRO == IZQ)
	{
		IO0PIN &= ~(1<<27);
		IO1PIN |= 1<<23;
	}
	else if(GIRO == DER)
	{
		IO0PIN |= 1<<27;
		IO1PIN &= ~(1<<23);
	}
	
	if(giro == IZQ)
	{
		IO0PIN &= ~(1<<31);
		IO1PIN |= 1<<22;
	}
	else if(giro == DER)
	{
		IO0PIN |= 1<<31;
		IO1PIN &= ~(1<<22);
	}
}

void conmutar_mux(void)
{
	// Conmutar sensores
	if(mux == '0')
	{
		mux = '1';
		IO0PIN |= (1<<25);
	}
	else
	{
		mux = '0';
		IO0PIN &= ~(1<<25);
	}
}
