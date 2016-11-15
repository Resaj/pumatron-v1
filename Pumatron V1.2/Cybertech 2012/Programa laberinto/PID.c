#include <lpc213x.h>
#include <stdlib.h>
#include <stdio.h>
#include "Declaraciones.h"
#include "PID.h"
#include "s1d15g00.h"

/********************** ALGORITMO PID **********************/
void get_PID(void)
{
	if(PARED == IZQ)
		position = sensor_izq;
	else // PARED == DER
		position = sensor_der;

	proportional = position - 120;

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

//	sprintf(cadena, "sensors_average");   
//	LCDPutStr(cadena, 120, 10, SMALL, WHITE, BLACK);
//	sprintf(cadena, "%6d", sensors_average);   
//	LCDPutStr(cadena, 105, 20, SMALL, WHITE, BLACK);
//	sprintf(cadena, "sensors_sum");   
//	LCDPutStr(cadena, 90, 10, SMALL, WHITE, BLACK);
//	sprintf(cadena, "%6d", sensors_sum);   
//	LCDPutStr(cadena, 75, 20, SMALL, WHITE, BLACK);
//	sprintf(cadena, "position");   
//	LCDPutStr(cadena, 60, 10, SMALL, WHITE, BLACK);
//	sprintf(cadena, "%6d", position);   
//	LCDPutStr(cadena, 45, 20, SMALL, WHITE, BLACK);
//	sprintf(cadena, "proportional");
//	LCDPutStr(cadena, 30, 10, SMALL, WHITE, BLACK);
//	sprintf(cadena, "%6d", proportional);   
//	LCDPutStr(cadena, 15, 20, SMALL, WHITE, BLACK);
}

void set_motors(void)
{
	if(PARED == IZQ)
	{
		if(sensor_izq < 250 && sensor_frontal < 250 && sensor_der > 250)
		{
			IO1PIN &= ~(1<<30);
			IO1PIN |= 1<<31;
			IO0PIN |= (1<<20)|(1<<23);

			PWMMR2 = (float)velocidad*Per_pwm*Fpclk;
			PWMMR4 = (float)velocidad*Per_pwm*Fpclk;
		}
		else if(sensor_izq < 250 && sensor_frontal < 250)
		{
			IO1PIN &= ~(1<<30);
			IO1PIN |= 1<<31;
			IO0PIN &= ~(1<<23);
			IO0PIN |= 1<<20;

			PWMMR2 = (float)velocidad/5*Per_pwm*Fpclk;
			PWMMR4 = (float)velocidad/2*Per_pwm*Fpclk;
		}
		else
		{
			if(control_value > max_difference)
			{
				control_value = max_difference;

				cont += Periodo;

				if(cont < 0.2)
				{
					IO1PIN |= (1<<30)|(1<<31);
					IO0PIN &= ~(1<<20);
					IO0PIN |= 1<<23;
	
					PWMMR2 = (float)0.4*velocidad*Per_pwm*Fpclk;
					PWMMR4 = (float)velocidad*Per_pwm*Fpclk;
				}
				else
				{
					IO1PIN &= ~(1<<30);
					IO1PIN |= 1<<31;
					IO0PIN &= ~(1<<20);
					IO0PIN |= 1<<23;
	
					PWMMR2 = (float)velocidad/5*Per_pwm*Fpclk;
					PWMMR4 = (float)velocidad*Per_pwm*Fpclk;
				}
			}
			else
			{
				if(control_value < -max_difference)
					control_value = -max_difference;

				IO1PIN &= ~(1<<30);
				IO1PIN |= 1<<31;
				IO0PIN &= ~(1<<20);
				IO0PIN |= 1<<23;
		
				if(control_value < 0)
				{
					PWMMR2 = (float)velocidad*Per_pwm*Fpclk;
					PWMMR4 = (float)(max_difference + control_value)/max_difference * velocidad*Per_pwm*Fpclk;
				}
				else // if(control_value >= 0)
				{
					PWMMR2 = (float)(max_difference - control_value)/max_difference * velocidad*Per_pwm*Fpclk;
					PWMMR4 = (float)velocidad*Per_pwm*Fpclk;
				}

				cont = 0;
			}
		}
	}
	else // PARED == DER
	{
		if(sensor_der < 250 && sensor_frontal < 250 && sensor_izq > 250)
		{
			IO1PIN |= (1<<30)|(1<<31);
			IO0PIN &= ~(1<<20);
			IO0PIN |= 1<<23;

			PWMMR2 = (float)velocidad*Per_pwm*Fpclk;
			PWMMR4 = (float)velocidad*Per_pwm*Fpclk;
		}
		else if(sensor_der < 250 && sensor_frontal < 250)
		{
			IO1PIN |= 1<<30;
			IO1PIN &= ~(1<<31);
			IO0PIN &= ~(1<<20);
			IO0PIN |= 1<<23;

			PWMMR2 = (float)velocidad/2*Per_pwm*Fpclk;
			PWMMR4 = (float)velocidad/5*Per_pwm*Fpclk;
		}
		else
		{
			if(control_value > max_difference)
			{
				control_value = max_difference;

				cont += Periodo;

				if(cont < 0.2)
				{
					IO1PIN &= ~(1<<30);
					IO1PIN |= (1<<31);
					IO0PIN |= (1<<20)|(1<<23);
	
					PWMMR2 = (float)velocidad*Per_pwm*Fpclk;
					PWMMR4 = (float)0.4*velocidad*Per_pwm*Fpclk;
				}
				else
				{
					IO1PIN &= ~(1<<30);
					IO1PIN |= 1<<31;
					IO0PIN &= ~(1<<20);
					IO0PIN |= 1<<23;
	
					PWMMR2 = (float)velocidad*Per_pwm*Fpclk;
					PWMMR4 = (float)velocidad/5*Per_pwm*Fpclk;
				}
			}
			else
			{
				if(control_value < -max_difference)
					control_value = -max_difference;

				IO1PIN &= ~(1<<30);
				IO1PIN |= 1<<31;
				IO0PIN &= ~(1<<20);
				IO0PIN |= 1<<23;
		
				if(control_value < 0)
				{
					PWMMR2 = (float)(max_difference - control_value)/max_difference * velocidad*Per_pwm*Fpclk;
					PWMMR4 = (float)velocidad*Per_pwm*Fpclk;
				}
				else // if(control_value >= 0)
				{
					PWMMR2 = (float)velocidad*Per_pwm*Fpclk;
					PWMMR4 = (float)(max_difference + control_value)/max_difference * velocidad*Per_pwm*Fpclk;
				}

				cont = 0;
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
