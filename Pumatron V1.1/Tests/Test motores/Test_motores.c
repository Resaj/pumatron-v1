#include <lpc213x.h>
#include <stdio.h>
#include "font.h"		
#include "s1d15g00.h"
#include "mando.h"

// Frecuencia del cristal del LPC2138
#define Fpclk 19.6608e6*3/4
								 
// Variables auxiliares 
char cadena[30];		// Variable auxiliar para guardar el texto a imprimir en el LCD
float velocidad = 0.8;	// Variable para controlar la velocidad. De 0 a 1
float motor_izq = 0;
float motor_der = 0;
float Per_pwm = 0.001;

/********************** FUNCIONES DE CONTROL **********************/
// Interrupción IRQ para el TIMER0
void TIMER0(void)__irq
{
	if(dato == BUTTON_CH_plus && motor_izq < 1)
		motor_izq = motor_izq + 0.05;
	else if(dato == BUTTON_CH_minus && motor_izq > 0)
		motor_izq = motor_izq - 0.05;
	else if(dato == BUTTON_VOL_plus && motor_der < 1)
		motor_der = motor_der + 0.05;
	else if(dato == BUTTON_VOL_minus && motor_der > 0)
		motor_der = motor_der - 0.05;
	else if(dato == BUTTON_1)
	{
		IO0PIN |= (1<<23);
		IO0PIN &= ~(1<<20);
		IO1PIN |= (1<<31);
		IO1PIN &= ~(1<<30);
		motor_izq = 0.3;
		motor_der = 1;
	}
	else if(dato == BUTTON_2)
	{
		IO0PIN |= (1<<23);
		IO0PIN &= ~(1<<20);
		IO1PIN |= (1<<31);
		IO1PIN &= ~(1<<30);
		motor_izq = 1;
		motor_der = 1;
	}
	else if(dato == BUTTON_3)
	{
		IO0PIN |= (1<<23);
		IO0PIN &= ~(1<<20);
		IO1PIN |= (1<<31);
		IO1PIN &= ~(1<<30);
		motor_izq = 1;
		motor_der = 0.3;
	}
	else if(dato == BUTTON_4)
	{
		IO0PIN |= (1<<23);
		IO0PIN &= ~(1<<20);
		motor_izq = 0;
		motor_der = 1;
	}
	else if(dato == BUTTON_5)
	{
		motor_izq = 0;
		motor_der = 0;
	}
	else if(dato == BUTTON_6)
	{
		IO1PIN |= (1<<31);
		IO1PIN &= ~(1<<30);
		motor_izq = 1;
		motor_der = 0;
	}
	else if(dato == BUTTON_7)
	{
		IO0PIN |= (1<<20);
		IO0PIN &= ~(1<<23);
		IO1PIN |= (1<<30);
		IO1PIN &= ~(1<<31);
		motor_izq = 0.3;
		motor_der = 1;
	}
	else if(dato == BUTTON_8)
	{
		IO0PIN |= (1<<20);
		IO0PIN &= ~(1<<23);
		IO1PIN |= (1<<30);
		IO1PIN &= ~(1<<31);
		motor_izq = 1;
		motor_der = 1;
	}
	else if(dato == BUTTON_9)
	{
		IO0PIN |= (1<<20);
		IO0PIN &= ~(1<<23);
		IO1PIN |= (1<<30);
		IO1PIN &= ~(1<<31);
		motor_izq = 1;
		motor_der = 0.3;
	}

	dato = 0;

	PWMMR2 = velocidad * motor_izq * 0.001*Fpclk;
	PWMMR4 = velocidad * motor_der * 0.001*Fpclk;
	PWMLER = (1<<0)|(1<<2)|(1<<4);//|(1<<5);	// Enable PWM Match 0, 2, 4 & 5 Latch

	sprintf(cadena, "motor izq %f", motor_izq);   
	LCDPutStr(cadena, 120, 5, SMALL, WHITE, BLACK);
	sprintf(cadena, "motor der %f", motor_der);   
	LCDPutStr(cadena, 100, 5, SMALL, WHITE, BLACK);

	T0IR |= (1<<2);		// Activar flag de interrupción del Match0.2
	VICVectAddr = 0;
}


/************************ CONFIGURACIONES ************************/
// Configuración del Match0.2 del TIMER0
void config_TIMER0 (void)
{
	PINSEL1 |= 1<<1;
	T0TCR = 0x02;
	T0PR = 0;
	T0MCR = (1<<6)|(1<<7);
	T0MR2 = 0.1 * Fpclk;
	VICVectAddr1 = (unsigned long) TIMER0;
	VICVectCntl1 = (0x20|4);
	T0TCR = 0x01;
	VICIntEnable |= 1<<4;
}

// Configuración del display
void config_LCD (void)
{
  LCD_BL_DIR();			// BL = Output
  LCD_CS_DIR();			// CS = Output
  LCD_SCLK_DIR();		// SCLK = Output
  LCD_SDATA_DIR();		// SDATA = Output
  LCD_RESET_DIR();		// RESET = Output
  LCD_SCLK_LOW();		// Standby SCLK
  LCD_CS_HIGH();		// Disable CS
  LCD_SDATA_HIGH();		// Standby SDATA
  LCD_BL_HIGH();		// Black Light ON = 100%

  InitLcd(); 			// Initial LCD
  LCDClearScreen();		// Limpiar pantalla
}

// Configuración de las salidas de PWM para los motores
void config_MOTORES (void)
{
	PINSEL0 |= (1<<15)|(1<<17);		// PWM2 y PWM4
	PINSEL1 |= (1<<10);				// PWM5
	IO0DIR |= (1<<20)|(1<<23);		// Salidas para el sentido de los motores
	IO1DIR |= (1<<30)|(1<<31);
	PWMTCR = 0x02;					// Reset TIMER y PREESCALER										   
	PWMPR = 0;						// El PRESCALER no modifica la frecuencia
	PWMMR0 = 0.001 * Fpclk;			// Periodo PWM										   
	PWMLER = (1<<0)|(1<<2)|(1<<4)|(1<<5);	// Habilitar MATCH0, MATCH2, MATCH4 y MATCH5
	PWMMCR = 0x02;					// Reset TIMER COUNTER REGISTER ON MATCH0							   
	PWMPCR = (1<<10)|(1<<12)|(1<<13);		// Habilitar PWM2, PWM4 y PWM5 en flanco
	PWMTCR = (1<<0)|(1<<3);			// Habilitar PWM y comenzar la cuenta
}
 

/************************** FUNCIÓN MAIN **************************/
int main (void)
{
	// Configuraciones previas a la iniciación de las respectivas funciones
	config_LCD();
	config_MOTORES();
	config_TIMER1();
	config_TIMER0();

	while(1) // Bucle infinito para la ejecución del programa
	{
	}
}
