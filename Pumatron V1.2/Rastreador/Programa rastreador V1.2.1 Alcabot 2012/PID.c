#include <lpc213x.h>
#include "Declaraciones.h"
#include "PID.h"

/********************** ALGORITMO PID **********************/
void get_PID(void)
{
	sensors_average = 0;
	sensors_sum = 0;
	for(i = 0; i < S; i++)
	{
		sensors_average += LINEAresult[i]*i*1000;
		sensors_sum += LINEAresult[i];
	}

	if(sensors_sum == 0)
	{
		if(last_proportional > 0)
			position = (S-1)*1000;
		else // last_proportional < 0
			position = 0;
	}
	else // sensors_sum != 0
		position = (sensors_average / sensors_sum);

	proportional = position - (S-1)*1000/2;

	if(proportional < -2000)
		giro = DER;
	else if(proportional > 2000)
		giro = IZQ;

	if(proportional == 0)
		integral = 0;
	else
	{
		integral = integral + proportional;
		if(abs(integral) > integral_max)
		{
			if(integral > 0)
				integral = integral_max;
			else
				integral = -integral_max;
		}
	}

	derivative = proportional - last_proportional;
	last_proportional = proportional;

	control_value = (float)(proportional * Kp + integral * Ki + derivative * Kd);
}

void set_motors(void)
{
	if(velocidad < v_rapida && ESTADO != Bifurcacion_proxima && ESTADO != Bifurcacion)
	{
		velocidad += 0.01;
		if(velocidad > v_rapida)
			velocidad = v_rapida;
	}

	if(sensors_sum == 0)
	{
		if(giro == IZQ)
		{
			IO1PIN &= ~(1<<31);
			IO1PIN |= 1<<30;
			IO0PIN &= ~(1<<20);
			IO0PIN |= 1<<23;
		}
		else if(giro == DER)
		{
			IO1PIN &= ~(1<<30);
			IO1PIN |= 1<<31;
			IO0PIN &= ~(1<<23);
			IO0PIN |= 1<<20;
		}

		PWMMR2 = (float)velocidad*Per_pwm*Fpclk;
		PWMMR4 = (float)velocidad*Per_pwm*Fpclk;
	}
	else
	{
		if(control_value > max_difference)
		{
			IO1PIN |= (1<<30)|(1<<31);
			IO0PIN &= ~(1<<20);
			IO0PIN |= 1<<23;

			PWMMR2 = (float)Per_pwm*Fpclk;
			PWMMR4 = (float)velocidad*Per_pwm*Fpclk;
		}
		else if(control_value < -max_difference)
		{
			IO1PIN &= ~(1<<30);
			IO1PIN |= 1<<31;
			IO0PIN |= (1<<20)|(1<<23);

			PWMMR2 = (float)velocidad*Per_pwm*Fpclk;
			PWMMR4 = (float)Per_pwm*Fpclk;
		}
		else
		{
			IO1PIN &= ~(1<<30);
			IO1PIN |= 1<<31;
			IO0PIN &= ~(1<<20);
			IO0PIN |= 1<<23;
	
			if(control_value < 0)
			{
				PWMMR2 = (float)(max_difference + control_value)/max_difference * velocidad*Per_pwm*Fpclk;
				PWMMR4 = (float)velocidad*Per_pwm*Fpclk;
			}
			else //if(control_value >= 0)
			{
				PWMMR2 = (float)velocidad*Per_pwm*Fpclk;
				PWMMR4 = (float)(max_difference - control_value)/max_difference * velocidad*Per_pwm*Fpclk;
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
