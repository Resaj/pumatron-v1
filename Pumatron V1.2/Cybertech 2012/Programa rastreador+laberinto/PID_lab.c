#include <lpc213x.h>
#include <stdlib.h>
#include <stdio.h>
#include "Declaraciones.h"
#include "PID_lab.h"
#include "s1d15g00.h"

/********************** ALGORITMO PID **********************/
void get_PID_lab(void)
{
	if(PARED == IZQ)
		position_lab = sensor_izq;
	else // PARED == DER
		position_lab = sensor_der;

	proportional_lab = position_lab - 120;

	integral_lab = integral_lab + proportional_lab;
	if(abs(integral_lab) > integral_max_lab)
	{
		if(integral_lab > 0)
			integral_lab = integral_max_lab;
		else
			integral_lab = -integral_max_lab;
	}

	derivative_lab = proportional_lab - last_proportional_lab;
	last_proportional_lab = proportional_lab;

	control_value_lab = (float)(proportional_lab * Kp_lab + integral_lab * Ki_lab + derivative_lab * Kd_lab);

//	sprintf(cadena, "sensors_average_lab");   
//	LCDPutStr(cadena, 120, 10, SMALL, WHITE, BLACK);
//	sprintf(cadena, "%6d", sensors_average_lab);   
//	LCDPutStr(cadena, 105, 20, SMALL, WHITE, BLACK);
//	sprintf(cadena, "sensors_sum_lab");   
//	LCDPutStr(cadena, 90, 10, SMALL, WHITE, BLACK);
//	sprintf(cadena, "%6d", sensors_sum_lab);   
//	LCDPutStr(cadena, 75, 20, SMALL, WHITE, BLACK);
//	sprintf(cadena, "position_lab");   
//	LCDPutStr(cadena, 60, 10, SMALL, WHITE, BLACK);
//	sprintf(cadena, "%6d", position_lab);   
//	LCDPutStr(cadena, 45, 20, SMALL, WHITE, BLACK);
//	sprintf(cadena, "proportional_lab");
//	LCDPutStr(cadena, 30, 10, SMALL, WHITE, BLACK);
//	sprintf(cadena, "%6d", proportional_lab);   
//	LCDPutStr(cadena, 15, 20, SMALL, WHITE, BLACK);
}

void set_motors_lab(void)
{
	if(PARED == IZQ)
	{
		if(sensor_izq < 250 && sensor_frontal < 250 && sensor_der > 250)
		{
			IO1PIN &= ~(1<<30);
			IO1PIN |= 1<<31;
			IO0PIN |= (1<<20)|(1<<23);

			PWMMR2 = (float)velocidad_lab*Per_pwm*Fpclk;
			PWMMR4 = (float)velocidad_lab*Per_pwm*Fpclk;
		}
		else if(sensor_izq < 250 && sensor_frontal < 250)
		{
			IO1PIN &= ~(1<<30);
			IO1PIN |= 1<<31;
			IO0PIN &= ~(1<<23);
			IO0PIN |= 1<<20;

			PWMMR2 = (float)velocidad_lab/5*Per_pwm*Fpclk;
			PWMMR4 = (float)velocidad_lab/2*Per_pwm*Fpclk;
		}
		else
		{
			if(control_value_lab > max_difference_lab)
			{
				control_value_lab = max_difference_lab;

				cont += Periodo;

				if(cont < 0.2)
				{
					IO1PIN |= (1<<30)|(1<<31);
					IO0PIN &= ~(1<<20);
					IO0PIN |= 1<<23;
	
					PWMMR2 = (float)0.4*velocidad_lab*Per_pwm*Fpclk;
					PWMMR4 = (float)velocidad_lab*Per_pwm*Fpclk;
				}
				else
				{
					IO1PIN &= ~(1<<30);
					IO1PIN |= 1<<31;
					IO0PIN &= ~(1<<20);
					IO0PIN |= 1<<23;
	
					PWMMR2 = (float)velocidad_lab/5*Per_pwm*Fpclk;
					PWMMR4 = (float)velocidad_lab*Per_pwm*Fpclk;
				}
			}
			else
			{
				if(control_value_lab < -max_difference_lab)
					control_value_lab = -max_difference_lab;

				IO1PIN &= ~(1<<30);
				IO1PIN |= 1<<31;
				IO0PIN &= ~(1<<20);
				IO0PIN |= 1<<23;
		
				if(control_value_lab < 0)
				{
					PWMMR2 = (float)velocidad_lab*Per_pwm*Fpclk;
					PWMMR4 = (float)(max_difference_lab + control_value_lab)/max_difference_lab * velocidad_lab*Per_pwm*Fpclk;
				}
				else // if(control_value_lab >= 0)
				{
					PWMMR2 = (float)(max_difference_lab - control_value_lab)/max_difference_lab * velocidad_lab*Per_pwm*Fpclk;
					PWMMR4 = (float)velocidad_lab*Per_pwm*Fpclk;
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

			PWMMR2 = (float)velocidad_lab*Per_pwm*Fpclk;
			PWMMR4 = (float)velocidad_lab*Per_pwm*Fpclk;
		}
		else if(sensor_der < 250 && sensor_frontal < 250)
		{
			IO1PIN |= 1<<30;
			IO1PIN &= ~(1<<31);
			IO0PIN &= ~(1<<20);
			IO0PIN |= 1<<23;

			PWMMR2 = (float)velocidad_lab/2*Per_pwm*Fpclk;
			PWMMR4 = (float)velocidad_lab/5*Per_pwm*Fpclk;
		}
		else
		{
			if(control_value_lab > max_difference_lab)
			{
				control_value_lab = max_difference_lab;

				cont += Periodo;

				if(cont < 0.2)
				{
					IO1PIN &= ~(1<<30);
					IO1PIN |= (1<<31);
					IO0PIN |= (1<<20)|(1<<23);
	
					PWMMR2 = (float)velocidad_lab*Per_pwm*Fpclk;
					PWMMR4 = (float)0.4*velocidad_lab*Per_pwm*Fpclk;
				}
				else
				{
					IO1PIN &= ~(1<<30);
					IO1PIN |= 1<<31;
					IO0PIN &= ~(1<<20);
					IO0PIN |= 1<<23;
	
					PWMMR2 = (float)velocidad_lab*Per_pwm*Fpclk;
					PWMMR4 = (float)velocidad_lab/5*Per_pwm*Fpclk;
				}
			}
			else
			{
				if(control_value_lab < -max_difference_lab)
					control_value_lab = -max_difference_lab;

				IO1PIN &= ~(1<<30);
				IO1PIN |= 1<<31;
				IO0PIN &= ~(1<<20);
				IO0PIN |= 1<<23;
		
				if(control_value_lab < 0)
				{
					PWMMR2 = (float)(max_difference_lab - control_value_lab)/max_difference_lab * velocidad_lab*Per_pwm*Fpclk;
					PWMMR4 = (float)velocidad_lab*Per_pwm*Fpclk;
				}
				else // if(control_value_lab >= 0)
				{
					PWMMR2 = (float)velocidad_lab*Per_pwm*Fpclk;
					PWMMR4 = (float)(max_difference_lab + control_value_lab)/max_difference_lab * velocidad_lab*Per_pwm*Fpclk;
				}

				cont = 0;
			}
		}
	}

	PWMLER = (1<<0)|(1<<2)|(1<<4);
}

void loop_PID_lab(void)
{
	get_PID_lab();
	set_motors_lab();
}
