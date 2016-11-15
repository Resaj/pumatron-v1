#include <lpc213x.h>
#include <stdlib.h>
#include <stdio.h>
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

void GP2Y0A_read(void)
{
	if(ADCresult[ch] > 23)
		distancia[ch] = 25*((3000.0 / (ADCresult[ch] - 23) + 0.3)); // Fórmula de conversión de tensión a mm.
	else
		distancia[ch] = 800;

	if (distancia[ch] > 800)
		distancia[ch] = 800; //saturamos
	else if (distancia[ch] < 100)
		distancia[ch] = 100; //saturamos
}

void GP2D120_read(void)
{
	distancia[ch] = 16.8*((2914.0 / (ADCresult[ch] + 35) - 0.5)); // Fórmula de conversión de tensión a mm.

	if (distancia[ch] > 350)
		distancia[ch] = 350; //saturamos
	else if (distancia[ch] < 40)
		distancia[ch] = 40; //saturamos
}

void leer_sensores(void)
{
	ch = 4;
	GP2D120_read();
	ch = 5;
	GP2Y0A_read();
	ch = 7;
	GP2D120_read();

	sensor_izq = distancia[7];
	sensor_der = distancia[4];
	sensor_frontal = distancia[5];

//	sprintf(cadena, "%3d", sensor_izq);   
//	LCDPutStr(cadena, 50, 80, SMALL, WHITE, BLACK);
////	sprintf(cadena, "%4d", ADCresult[7]);   
////	LCDPutStr(cadena, 65, 80, SMALL, WHITE, BLACK);
//	sprintf(cadena, "%3d", sensor_frontal);
//	LCDPutStr(cadena, 20, 50, SMALL, WHITE, BLACK);
////	sprintf(cadena, "%4d", ADCresult[5]);
////	LCDPutStr(cadena, 35, 50, SMALL, WHITE, BLACK);
//	sprintf(cadena, "%3d", sensor_der);   
//	LCDPutStr(cadena, 50, 10, SMALL, WHITE, BLACK);
////	sprintf(cadena, "%4d", ADCresult[4]);   
////	LCDPutStr(cadena, 65, 10, SMALL, WHITE, BLACK);
}
