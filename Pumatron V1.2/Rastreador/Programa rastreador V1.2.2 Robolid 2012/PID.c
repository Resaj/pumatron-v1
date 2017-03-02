#include <lpc213x.h>
#include <stdlib.h>
#include <stdio.h>
#include "Declaraciones.h"
#include "PID.h"
#include "s1d15g00.h"

/********************** ALGORITMO PID **********************/
void get_PID(void)
{
	sensors_average = 0;
	sensors_sum = 0;
	for(i = 0; i < S; i++)		   
	{
		sensors[i] = (ADCresult[i]-blanco[i])*1000/(negro[i]-blanco[i]);

		if(((ESTADO == Rastrear || ESTADO == Bifurcacion_proxima) && sensors[i] < 100) || 
			((LINEAresult[i] == 1 && sensors[i] < 200) || LINEAresult[i] == 0))
				sensors[i] = 0;

		sensors_average += sensors[i]*(i+1)*1000;
		sensors_sum += sensors[i];
	}

	if(negros == 0)
	{
		if(giro == IZQ)
			position = (S+1)*1000;
		else // giro == DER
			position = 0;
	}
	else // sensors_sum != 0
		position = sensors_average / sensors_sum;

	proportional = position - (S+1)*1000/2;

	if(negros != 0)
	{
		if(proportional < -3500)
			giro = DER;
		else if(proportional > 3500)
			giro = IZQ;
	}

	integral = integral + proportional;
	if(abs(integral) > integral_max)
	{
		if(integral > 0)
			integral = integral_max;
		else
			integral = -integral_max;
	}

	derivative = proportional - last_proportional;
	last_proportional = proportional;

	control_value = (float)(proportional * Kp + integral * Ki + derivative * Kd);

//	for(i = 0; i < S; i++)
//	{
//		if(ADCresult[i] > umbral[i])
//			LCDSetRect((120-(i*10)), 5, (125-(i*10)), 10, 1, RED);
//		else
//			LCDSetRect((120-(i*10)), 5, (125-(i*10)), 10, 1, WHITE);
//
//		sprintf(cadena, "%4d", ADCresult[i]);
//		LCDPutStr(cadena, (120-(i*10)), 15, SMALL, WHITE, BLACK);
//	}
//	sprintf(cadena, "sensors_average");   
//	LCDPutStr(cadena, 120, 60, SMALL, WHITE, BLACK);
//	sprintf(cadena, "%6d", sensors_average);   
//	LCDPutStr(cadena, 105, 70, SMALL, WHITE, BLACK);
//	sprintf(cadena, "sensors_sum");   
//	LCDPutStr(cadena, 90, 60, SMALL, WHITE, BLACK);
//	sprintf(cadena, "%6d", sensors_sum);   
//	LCDPutStr(cadena, 75, 70, SMALL, WHITE, BLACK);
//	sprintf(cadena, "position");   
//	LCDPutStr(cadena, 60, 60, SMALL, WHITE, BLACK);
//	sprintf(cadena, "%6d", position);   
//	LCDPutStr(cadena, 45, 70, SMALL, WHITE, BLACK);
//	sprintf(cadena, "proportional");
//	LCDPutStr(cadena, 30, 60, SMALL, WHITE, BLACK);
//	sprintf(cadena, "%6d", proportional);   
//	LCDPutStr(cadena, 15, 70, SMALL, WHITE, BLACK);
}

void set_motors(void)
{
	if(velocidad < v_rapida && ESTADO != Bifurcacion_proxima && ESTADO != Bifurcacion)
	{
		velocidad += 0.02;
		if(velocidad > v_rapida)
			velocidad = v_rapida;
	}

//	if(abs(control_value) < max_difference/2)
//	{
//		IO1PIN &= ~(1<<30);
//		IO1PIN |= 1<<31;
//		IO0PIN &= ~(1<<20);
//		IO0PIN |= 1<<23;
//
//		if(control_value < 0)
//		{
//			PWMMR2 = (float)velocidad*Per_pwm*Fpclk;
//			PWMMR4 = (float)(max_difference/2 + control_value)/max_difference/2 * velocidad*Per_pwm*Fpclk;
//		}
//		else
//		{
//			PWMMR2 = (float)(max_difference/2 - control_value)/max_difference/2 * velocidad*Per_pwm*Fpclk;
//			PWMMR4 = (float)velocidad*Per_pwm*Fpclk;
//		}
//	}
//	else
//	{
//		if(control_value > max_difference)
//			control_value = max_difference;
//		else if(control_value < -max_difference)
//			control_value = -max_difference;
//
//		if(control_value < 0)
//		{
//			IO1PIN &= ~(1<<30);
//			IO1PIN |= 1<<31;
//			IO0PIN &= ~(1<<23);
//			IO0PIN |= 1<<20;
//
//			PWMMR2 = (float)velocidad*Per_pwm*Fpclk;
//			PWMMR4 = (float)(-max_difference/2 - control_value)/max_difference/2 * velocidad*Per_pwm*Fpclk;
//		}
//		else
//		{
//			IO1PIN &= ~(1<<31);
//			IO1PIN |= 1<<30;
//			IO0PIN &= ~(1<<20);
//			IO0PIN |= 1<<23;
//
//			PWMMR2 = (float)(-max_difference/2 + control_value)/max_difference/2 * velocidad*Per_pwm*Fpclk;
//			PWMMR4 = (float)velocidad*Per_pwm*Fpclk;
//		}
//	}

	if(negros == 0)
	{
		if(giro == IZQ)
		{
			IO0PIN &= ~(1<<20);
			IO0PIN |= 1<<23;
			IO1PIN |= 1<<30;
			IO1PIN &= ~(1<<31);
			PWMMR2 = (float)velocidad*Per_pwm*Fpclk;
			PWMMR4 = (float)velocidad*Per_pwm*Fpclk/2;
		}
		else if(giro == DER)
		{
			IO0PIN &= ~(1<<23);
			IO0PIN |= 1<<20;
			IO1PIN &= ~(1<<30);
			IO1PIN |= 1<<31;
			PWMMR2 = (float)velocidad*Per_pwm*Fpclk/2;
			PWMMR4 = (float)velocidad*Per_pwm*Fpclk;
		}
	}
	else
	{
		if(control_value > max_difference)
		{
			control_value = max_difference;
			IO1PIN |= (1<<30)|(1<<31);
			IO0PIN &= ~(1<<20);
			IO0PIN |= 1<<23;

			PWMMR2 = (float)Per_pwm*Fpclk;
			PWMMR4 = (float)velocidad*Per_pwm*Fpclk;
		}
		else if(control_value < -max_difference)
		{
			control_value = -max_difference;
			IO1PIN &= ~(1<<30);
			IO1PIN |= 1<<31;
			IO0PIN |= (1<<20)|(1<<23);

			PWMMR2 = (float)velocidad*Per_pwm*Fpclk;
			PWMMR4 = (float)Per_pwm*Fpclk;
		}
		else
		{
//			if(control_value > max_difference)
//				control_value = max_difference;
//			else if(control_value < -max_difference)
//				control_value = -max_difference;

			IO1PIN &= ~(1<<30);
			IO1PIN |= 1<<31;
			IO0PIN &= ~(1<<20);
			IO0PIN |= 1<<23;
	
			if(control_value < 0)
			{
				PWMMR2 = (float)velocidad*Per_pwm*Fpclk;
				PWMMR4 = (float)(max_difference + control_value)/max_difference * velocidad*Per_pwm*Fpclk;
			}
			else //if(control_value >= 0)
			{
				PWMMR2 = (float)(max_difference - control_value)/max_difference * velocidad*Per_pwm*Fpclk;
				PWMMR4 = (float)velocidad*Per_pwm*Fpclk;
			}
		}
	}

	PWMLER = (1<<0)|(1<<2)|(1<<4);
}

void loop_PID(void)
{
	get_PID();
	set_motors();
}
