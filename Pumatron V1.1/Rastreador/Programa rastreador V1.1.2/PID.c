#include <lpc213x.h>
#include "Declaraciones.h"
#include "PID.h"


/********************** ALGORITMO PID **********************/

void get_average(void)
{
	//Zero the sensors_average variable
	sensors_average = 0;

	//Generate the weighted sensor average starting at sensor one to
	// keep from doing a needless multiplication by zero which will
	// always discarded any value read by sensor zero
	for(i = 0; i < S; i++)
		sensors_average += LINEAresult[i]*i*1000;
}

void get_sum(void)
{
	//Zero the sensors_sum variable
	sensors_sum = 0;

	//Sum the total of all the sensors readings
	for(i = 0; i < S; i++)
		sensors_sum += LINEAresult[i];
}

void get_position(void)
{
	position = 0;

	//Calculate the current position on the line
	if (sensors_sum != 0)
		position = (int)(sensors_average / sensors_sum);
}

void get_proportional(void)
{
	//Calculate the proportional value
	proportional = position - (S-1)*1000/2;
}

void get_integral(void)
{
	//Calculate the integral value
	integral = integral + proportional;
}

void get_derivative(void)
{
	//Calculate the derivative value
	derivative = proportional - last_proportional;

	//Store proportional value in last_proportional for the derivative
	// calculation on next loop
	last_proportional = proportional;
}

void get_control(void)
{
	//Calculate the control value
	control_value = (int)(proportional * Kp + integral * Ki + derivative * Kd);
}

void adjust_control(void)
{
	//If the control value is greater than the allowed maximum set it
	// to the maximum
	if(control_value > max_difference)
		control_value = max_difference;

	//If the control value is less than the allowed minimum set it to
	// the minimum
	if(control_value < -max_difference)
		control_value = -max_difference;
}

void set_motors(void)
{
//	if(LINEAresult[0] == 1 || LINEAresult[1] == 1)
//		giro = DER;
//	else if(LINEAresult[S-2] == 1 || LINEAresult[S-1] == 1)
//		giro = IZQ;
//
//	if(sensors_sum == 0 && LINEAresult[0] == 0)
//	{
//		if(giro == IZQ)	// Si la secuencia anterior es 100...
//		{				  			
//			IO0PIN |= (1<<23);
//			IO0PIN &= ~(1<<20);
//			IO1PIN |= (1<<30);
//			IO1PIN &= ~(1<<31);
//			PWMMR2 = velocidad*Per_pwm*Fpclk;
//			PWMMR4 = velocidad*0.5*Per_pwm*Fpclk;
//		}
//		else			// Si la secuencia anterior es ...001
//		{	 			
//			IO0PIN |= (1<<20);
//			IO0PIN &= ~(1<<23);
//			IO1PIN |= (1<<31);
//			IO1PIN &= ~(1<<30);
//			PWMMR2 = velocidad*Per_pwm*Fpclk;
//			PWMMR4 = velocidad*0.5*Per_pwm*Fpclk;
//		}
//	}
//	else
//	{
		IO0PIN |= (1<<23);
		IO0PIN &= ~(1<<20);
		IO1PIN |= (1<<31);
		IO1PIN &= ~(1<<30);

		//If control_value is less than zero a right turn ir needed to
		// acquire the center of the line
		if(control_value < 0)
		{
			PWMMR2 = velocidad*Per_pwm*Fpclk;
			PWMMR4 = (max_difference + control_value)/max_difference * velocidad*Per_pwm*Fpclk;
		}
		//If control_value is greater than zero a left turn is needed to
		// acquire the center of the line
		else
		{
			PWMMR2 = (max_difference - control_value)/max_difference * velocidad*Per_pwm*Fpclk;
			PWMMR4 = velocidad*Per_pwm*Fpclk;
		}
//	}

	PWMLER = (1<<0)|(1<<2)|(1<<4);	// Enable PWM Match 0, 2 & 4 Latch
}

void loop_PID(void)
{
	get_average();
	get_sum();
	get_position();
	get_proportional();
	get_integral();
	get_derivative();
	get_control();
	adjust_control();
	set_motors();
}
