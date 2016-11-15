/***********************************************/
/* Examples For "CP-JR ARM7 LPC2138" Board     */
/* Target MCU  : Philips ARM7-LPC2138          */
/*       	   : X-TAL : 19.6608 MHz           */
/*             : Run Speed 59.982MHz(With PLL) */
/*             : PLL Setup = M(3),P(2)		   */
/*             : VPB Clock=CPU Clock=58.982MHz */
/* Keil Editor : uVision3 V3.03a               */
/* Compiler    : Keil Realview-MDK V3.15       */
/* Create By   : Eakachai Makarn(WWW.ETT.CO.TH)*/
/* Last Update : 06/March/2008                 */
/* Function    : LCD-NOKIA6610 Demo Display    */
/*             : Used GPIO Pin Connect LCD     */
/***********************************************/
/***********************************************/
/* Pin Interface to LCD-NOKIA6610              */
/* P1.17  --> LCD_CS#					       */
/* P1.18 --> LCD_SCLK       		               */
/* P1.19 --> LCD_SDATA                          */
/* P1.20 --> LCD_RESET#     	   		       	   */
/* P1.16  --> LCD_BL(Backlight Control)         */
/***********************************************/

/* Include  Section */
#include "LPC213x.H"                       						// LPC2138 MPU Register
#include <stdio.h>												// For "sprintf" Function
//#include "bmp.h"												// Mitmap Picture 132 x 132
#include "bug.h"												// Mitmap Picture 132 x 132
#include "font.h"												// font 5x7 Table
 
// Define LCD-NOKIA6610 PinIO Interface Mask Bit 
#define  LCD_CS_PIN		        0x00020000   					// P1.17  (0000 0000 0000 00x0 0000 0000 0000 0000)
#define  LCD_SCLK_PIN		    0x00040000						// P1.18  (0000 0000 0000 0x00 0000 0000 0000 0000)
#define  LCD_SDATA_PIN			0x00080000						// P1.19  (0000 0000 0000 x000 0000 0000 0000 0000)
#define  LCD_RESET_PIN			0x00100000						// P1.20  (0000 0000 000x 0000 0000 0000 0000 0000)
#define  LCD_BL_PIN 		    0x00010000   					// P1.16  (0000 0000 0000 000x 0000 0000 0000 0000)

#define  LCD_BL_DIR()			IO1DIR |= LCD_BL_PIN			// BL       = Output
#define  LCD_CS_DIR()			IO1DIR |= LCD_CS_PIN			// CS#      = Output 
#define  LCD_SCLK_DIR()			IO1DIR |= LCD_SCLK_PIN			// SCLK     = Output 
#define  LCD_SDATA_DIR()		IO1DIR |= LCD_SDATA_PIN			// SDATA    = Output
#define  LCD_RESET_DIR()		IO1DIR |= LCD_RESET_PIN			// RESET#   = Output

#define  LCD_BL_HIGH()  		IOSET1  = LCD_BL_PIN			// BL     = '1' 
#define  LCD_BL_LOW()  			IOCLR1  = LCD_BL_PIN			// BL     = '0'
#define  LCD_CS_HIGH()  		IOSET1  = LCD_CS_PIN			// CS#    = '1' 
#define  LCD_CS_LOW()  			IOCLR1  = LCD_CS_PIN			// CS#    = '0'
#define  LCD_SCLK_HIGH() 		IOSET1  = LCD_SCLK_PIN			// SCLK   = '1' 
#define  LCD_SCLK_LOW() 		IOCLR1  = LCD_SCLK_PIN			// SCLK   = '0'
#define  LCD_SDATA_HIGH() 		IOSET1  = LCD_SDATA_PIN			// SDATA  = '1' 
#define  LCD_SDATA_LOW() 		IOCLR1  = LCD_SDATA_PIN			// SDATA  = '0'
#define  LCD_RESET_HIGH() 		IOSET1  = LCD_RESET_PIN			// RESET# = '1' 
#define  LCD_RESET_LOW() 		IOCLR1  = LCD_RESET_PIN			// RESET# = '0'
// End of Define For LCD-NOKIA6610

// Epson S1D15G10 Command Set 
#define DISON       0xaf   										// LCD-NOKIA6610(Epson) Register
#define DISOFF      0xae										
#define DISNOR      0xa6
#define DISINV      0xa7
#define COMSCN      0xbb
#define DISCTL      0xca
#define SLPIN       0x95
#define SLPOUT      0x94
#define PASET       0x75
#define CASET       0x15
#define DATCTL      0xbc
#define RGBSET8     0xce
#define RAMWR       0x5c
#define RAMRD       0x5d
#define PTLIN       0xa8
#define PTLOUT      0xa9
#define RMWIN       0xe0
#define RMWOUT      0xee
#define ASCSET      0xaa
#define SCSTART     0xab
#define OSCON       0xd1
#define OSCOFF      0xd2
#define PWRCTR      0x20
#define VOLCTR      0x81
#define VOLUP       0xd6
#define VOLDOWN     0xd7
#define TMPGRD      0x82
#define EPCTIN      0xcd
#define EPCOUT      0xcc
#define EPMWR       0xfc
#define EPMRD       0xfd
#define EPSRRD1     0x7c
#define EPSRRD2     0x7d
#define NOP         0x25

//a few basic colors
#define RED			0xE0	   									// Color Code
#define GREEN		0x1C
#define BLUE		0x03
#define YELLOW		0xFC
#define MAGENTA		0xE3
#define CYAN		0x1F
#define BLACK		0x00
#define WHITE		0xFF

/* User Define Function */
void Send_CMD(unsigned char LCD_Command);						// Send Command Byte
void Send_Data(unsigned char LCD_Data);							// Send Data Byte
void LCD_Initial(void);											// Initial LCD
void Draw_Color_Bar(void);										// Draw Color Bar								
void LCD_Fill_Screen(unsigned char color, 	   					// Clear Screen
                     unsigned char x1, 
			         unsigned char y1, 
			         unsigned char x2, 
			         unsigned char y2);
void LCD_Print_String(unsigned char fcolor, 					// Print String on Screen
                      unsigned char bcolor, 			
				      unsigned char x, 
				      unsigned char y,char *text);
void LCD_Put_Pixel(unsigned char color, 	   					// Plot Character on Screen
				   unsigned char x, 
				   unsigned char y);
void LCDWrite130x130bmp(void);
void delay(unsigned long);										// Delay Time Function(1..4294967295)

/* Main Program Start Here */
int main (void) 
{
   char String_Buff[30];										// String Array Buffer	 

   LCD_Initial();	 											// Initial LCD-NOKIA6610

   while(1)
   {
      // Demo Fill Screen(Bar Color) with Block
      Draw_Color_Bar();											// Draw 4-Bar Color	on Screen
      delay(15000000);											// Display Page Delay

	  // Demo Display Bitmap Picture 132x132
      LCDWrite130x130bmp();
      delay(15000000);											// Display Page Delay
   	  
      Send_CMD(DATCTL);  										// Return Graphic Mode to Normal Display Mode
      Send_Data(0x00);
      Send_Data(0);
      Send_Data(0x01);
      Send_Data(0);

	  // Demo Print String to Display
      LCD_Fill_Screen(BLACK,0,0,131,131);  						// Fill Screen with Black Color
      sprintf(String_Buff," TEST ET-LCD6610 ");
      LCD_Print_String(YELLOW, BLACK, 7, 5,String_Buff);  				
      sprintf(String_Buff," WWW.ETTEAM.COM ");
      LCD_Print_String(CYAN, BLACK, 7, 15,String_Buff);
      sprintf(String_Buff," TEST RED COLOR ");
      LCD_Print_String(RED, BLACK, 7, 30,String_Buff);
      sprintf(String_Buff," TEST GREEN COLOR ");
      LCD_Print_String(GREEN, BLACK, 7, 45,String_Buff);
      sprintf(String_Buff," TEST BLUE COLOR ");
      LCD_Print_String(BLUE, BLACK, 7, 60,String_Buff);
      sprintf(String_Buff," TEST WHITE COLOR ");
      LCD_Print_String(WHITE, BLACK, 7, 75,String_Buff);
      sprintf(String_Buff," TEST MAGENTA COLOR ");
      LCD_Print_String(MAGENTA, BLACK, 7, 90,String_Buff);
      sprintf(String_Buff," TEST CYAN COLOR ");
      LCD_Print_String(CYAN, BLACK, 7, 105,String_Buff);
      sprintf(String_Buff," TEST YELLOW COLOR ");
      LCD_Print_String(YELLOW, BLACK, 7, 120,String_Buff);
      delay(15000000); 											// Display Page Delay  
  }
}

/*************************/
/* Initial LCD-NOKIA6610 */
/*************************/
void LCD_Initial(void)
{
   //PINSEL0 &= 0xFFFF003F;                                		// Config P0[7..3] = GPIO (xxxx xxxx xxxx xxxx 0000 0000 00xx xxxx)
   PINSEL2 &= 0xFFF30000;										// Config P1[31..16] = GPIO (xxxx xxxx xxxx 00xx 0000 0000 0000 0000)
   LCD_BL_DIR();
   LCD_CS_DIR();
   LCD_SCLK_DIR();
   LCD_SDATA_DIR();
   LCD_RESET_DIR();

   LCD_RESET_LOW();												// Start Reset
   LCD_SCLK_LOW();   											// Standby SCLK
   LCD_CS_HIGH();												// Disable CS
   LCD_SDATA_HIGH();											// Standby SDATA
   LCD_BL_HIGH();												// Black Light ON = 100%
   delay(500000);												// Reset Pulse Time
   LCD_RESET_HIGH();											// Release Reset
      
   // Display Control
   Send_CMD(DISCTL);
   Send_Data(0x03); 											// 2 divisions, Field swithcing period
   Send_Data(32);   											// 132 lines to be display
   Send_Data(12);   											// Inversely hightlighted lines - 1
   Send_Data(0);

   Send_CMD(COMSCN);  											// comscn
   Send_Data(0x01);

   Send_CMD(OSCON);  											// oscon

   Send_CMD(SLPOUT);  											// sleep out

   Send_CMD(VOLCTR);  											// electronic volume, this is kinda contrast/brightness
   Send_Data(0x25);       										// this might be different for individual LCDs
   Send_Data(0x03);    

   Send_CMD(PWRCTR);  											// power ctrl
   Send_Data(0x0F);      										// everything on, no external reference resistors
   delay(10000);

   Send_CMD(DISINV);  											// display mode

   Send_CMD(DATCTL);  											// datctl
   Send_Data(0x00);
   Send_Data(0);
   Send_Data(0x01);
   Send_Data(0);

   Send_CMD(RGBSET8);   										// setup color lookup table

   // Color Table
   Send_Data(0);
   Send_Data(2);
   Send_Data(4);
   Send_Data(6);
   Send_Data(8);
   Send_Data(10);
   Send_Data(12);
   Send_Data(15);

   // Green
   Send_Data(0);
   Send_Data(2);
   Send_Data(4);
   Send_Data(6);
   Send_Data(8);
   Send_Data(10);
   Send_Data(12);
   Send_Data(15);

   //Blue
   Send_Data(0);
   Send_Data(4);
   Send_Data(9);
   Send_Data(15);

   Send_CMD(NOP);
   LCD_Fill_Screen(WHITE,0,0,131,131);
   Send_CMD(DISON);    											// Display On   
}


/********************/
/* Send Data to LCD */
/********************/
void Send_Data(unsigned char LCD_Data) 
{   
   unsigned char Bit = 0;										// Bit Counter

   LCD_CS_HIGH();												// Makesure CS = Disable
   LCD_SCLK_LOW(); 												// Standby SCLK 

   LCD_CS_LOW();												// Start CS = Enable
   LCD_SDATA_HIGH();  											// D/C# = 1(Data)
   LCD_SCLK_HIGH();												// Strobe Signal Bit(SDATA)
   LCD_SCLK_LOW(); 												// Standby SCLK 

   for (Bit = 0; Bit < 8; Bit++)								// 8 Bit Write
   {
       LCD_SCLK_LOW(); 											// Standby SCLK 

	   if((LCD_Data&0x80)>>7) LCD_SDATA_HIGH();  
	   else LCD_SDATA_LOW();

	   LCD_SCLK_HIGH();											// Strobe Signal Bit(SDATA)

	   LCD_Data <<= 1;	 										// Next Bit Data
   } 
   LCD_SCLK_LOW(); 												// Standby SCLK 
   LCD_CS_HIGH();												// Stop CS = Disable  
}

/***********************/
/* Send Command to LCD */
/***********************/
void Send_CMD(unsigned char LCD_Command) 
{
   unsigned char Bit = 0;										// Bit Counter
																
   LCD_CS_HIGH();												// Makesure CS = Disable
   LCD_SCLK_LOW(); 												// Prepared SCLK

   LCD_CS_LOW();												// Start CS = Enable
   LCD_SDATA_LOW();   											// D/C# = 0(Command)
   LCD_SCLK_HIGH();												// Strobe Signal Bit(SDATA)
   LCD_SCLK_LOW(); 												// Standby SCLK 
   
   for (Bit = 0; Bit < 8; Bit++)								// 8 Bit Write
   {
       LCD_SCLK_LOW(); 											// Standby SCLK 

	   if((LCD_Command&0x80)>>7) LCD_SDATA_HIGH();  
	   else LCD_SDATA_LOW();

	   LCD_SCLK_HIGH();											// Strobe Signal Bit(SDATA)

	   LCD_Command <<= 1;	 									// Next Bit Data
   }  
   LCD_SCLK_LOW();  											// Standby SCLK 
   LCD_CS_HIGH();												// Stop CS = Disable
}

/********************************/
/*  (x1,y1)						*/
/*     ********************		*/
/*     *                  *		*/
/*     *                  *		*/
/*     ********************		*/
/*                    (x2,y2)	*/
/********************************/

void LCD_Set_Box(unsigned char x1, 
                 unsigned char y1, 
				 unsigned char x2, 
				 unsigned char y2)
{
   Send_CMD(CASET);   											// Page Start/Eend ram
   Send_Data(x1);     
   Send_Data(x2);

   Send_CMD(PASET);   											// Column Start/End ram
   Send_Data(y1);
   Send_Data(y2);
}

/**************************/
/* Fill Screen With Color */
/**************************/
void LCD_Fill_Screen(unsigned char color, 
                     unsigned char x1, 
			         unsigned char y1, 
			         unsigned char x2, 
			         unsigned char y2)
{
  unsigned int i;
  unsigned int total_bytes1;
  unsigned int total_bytes2;
  unsigned int total_bytes;

  LCD_Set_Box(x1, y1, x2, y2);

  Send_CMD(RAMWR);
  total_bytes1 = (x2 - x1 + 1); 
  total_bytes2 = (y2 - y1 + 1);
  total_bytes = total_bytes1 * total_bytes2;
  for (i = 0; i < total_bytes; i++)
  {
    Send_Data(color);
  }
}


/**********************/
/* Create 4-Bar Color */
/**********************/
void Draw_Color_Bar(void)
{
   LCD_Fill_Screen(RED,0,0,131,33);
   LCD_Fill_Screen(GREEN,0,34,131,66);
   LCD_Fill_Screen(BLUE,0,67,131,99);
   LCD_Fill_Screen(WHITE,0,100,131,131);
}

/************************/
/* Print Char on Screen */
/************************/
void LCD_Print_Char(unsigned char fcolor, 
					unsigned char bcolor,
					unsigned char x, 
					unsigned char y,char c)
{
   unsigned int i;
   LCD_Set_Box(x,y,x,y+7);
   Send_CMD(RAMWR);
   for(i=0;i<8;i++)
   {
     if (1<<i & c)
        Send_Data(fcolor);
     else
        Send_Data(bcolor);
   }
}

/**************************/
/* Print String on Screen */
/**************************/
void LCD_Print_String(unsigned char fcolor, 
				      unsigned char bcolor, 
				      unsigned char x, 
				      unsigned char y,char *text)
{
   unsigned char c;
   unsigned char t;
   unsigned int i;
   unsigned int j;

   while(*text != 0)
   {
     t = *text;
     i = t - 32;
     i = i * 5;
     for(j = i; j < i+5; j++)
     {
       c = font[j];
       LCD_Print_Char(fcolor, bcolor, x++, y, c);
     }
     LCD_Print_Char(fcolor, bcolor, x++, y, 0);
     text++;
   }
}

/************************/
/* Plot Pixed on Screen */
/************************/
void LCD_Put_Pixel(unsigned char color, 
				   unsigned char x, 
				   unsigned char y)
{
   Send_CMD(CASET);   											// Page Start/End RAM
   Send_Data(x);      
   Send_Data(x+1);

   Send_CMD(PASET);   											// Column Start/End RAM
   Send_Data(y);
   Send_Data(y+1);

   Send_CMD(RAMWR);	 											// Plot Pixel with Color
   Send_Data(color);
}

/*********************************/
/* File Bitmap Picture to Screen */
/* Bitmap Size : 132x132 Pixel   */
/*             : 24 Bit Color    */
/*********************************/
void LCDWrite130x130bmp(void) 
{
   long j; 														// Pixed Counter

   Send_CMD(DATCTL);
   Send_Data(0x00); 											// P1: 0x00 = Page,Column Address Normal
   Send_Data(0x01); 											// P2: 0x01 = RGB sequence B-> G-> R(Invert Direction)
   Send_Data(0x02); 											// P3: 0x02 = Grayscale -> 16
   
   Send_CMD(DISOFF); 											// Display OFF
   
   Send_CMD(CASET);	 											// Column address set (command 0x2A)
   Send_Data(0);
   Send_Data(131);
   
   Send_CMD(PASET);												// Page address set (command 0x2B)
   Send_Data(0);
   Send_Data(131);

   
   Send_CMD(RAMWR);	 											// Write Memory
   for(j = 0; j < 25740; j++) 
   {
      Send_Data(bmp[j]);
   }
   
   Send_CMD(DATCTL);											// Data Control (Return To "Inverted" Page Address)
   Send_Data(0x01); 											// P1: 0x01 = Page Address Inverted, Column Address Normal
   Send_Data(0x01); 											// P2: 0x01 = RGB sequence B-> G-> R(Invert Direction)
   Send_Data(0x02); 											// P3: 0x02 = Grayscale -> 16

   
   Send_CMD(DISON);	 											// Display On
}

/*******************************************/
/* Long Delay Time Function(1..4294967295) */
/*******************************************/
void delay(unsigned long i)
{
  while(i > 0) {i--;}											// Loop Decrease Counter	
  return;
}
			


