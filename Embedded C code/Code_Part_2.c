/* Main.c file generated by New Project wizard
 *
 * Created:   Tue Nov 9 2021
 * Processor: AT89C52
 * Compiler:  Keil for 8051
 */

//--------------------------------------------------------------
//Calls the Required Libraries for the program
#include <reg51.h>                 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//---------------------------------------------------------------
//Defines the functions before operation
void cct_init(void);
void delay(int);
void lcdinit(void);
void writecmd(unsigned char a);
void writedata(unsigned char t);
void write_string(unsigned char *a);

void delay_multiplier(int a);
void drive_stepper(int n);
void show_pressure();


bit write_i2c(unsigned char byte);
unsigned int adc();


void SerialInitialize(void);
void uart_msg(unsigned char *c);
void uart_tx(unsigned char serialdata);
unsigned char get_input();

unsigned concatenate(unsigned x, unsigned y);
unsigned int get_filtration_vol(void);
void show_adc(int n, int a);

void off_data_lines();

void send_signal();
void metric();
//-----------------------------------------------------------
//defines ports
#define lcd P1
#define adc_data P0
//-----------------------------------------------------------



//-----------------------------------------------------------
//Defines and names pins
sbit RS = P1^0;
sbit E  = P1^1;

sbit signal1= P1^2;
sbit signal2 = P1^3;
sbit ADC_pressure = P3^2;

sbit D4 = P1^4;
sbit D5 = P1^5;
sbit D6 = P1^6;
sbit D7 = P1^7;

sbit D0 = P2^4;
sbit D1 = P2^5;
sbit D2 = P2^6;
sbit D3 = P2^7;


sbit bacteria_concentrate = P3^2;

sbit OE = P3^4;
sbit EOC = P3^5;
sbit START = P3^6;
sbit ALE = P3^7;
//-------------------------------------------------------------
// Names and initialises global variables
unsigned int Water_flow,Water_level, Elution_level;
unsigned int adc_val;
//--------------------------------------------------------------
// Main program
//
void main(void)
{
//--------------------------------------------------------------
//Initialises Local Variables
			int i;
			unsigned int Filtration_Required;
			unsigned int b;
			char a;
      float Current_ADC_val =1.0;
//---------------------------------------------------------------
// initialises ports, serial comms and LCD	
		cct_init();                                     //Make all ports zero
		lcdinit();
		SerialInitialize();
//----------------------------------------------------------------
	
	
	 while(1)
    {
//-----------------------------------------------------------------
//puts j into 0 so that concatena		
		Filtration_Required =0;
//------------------------------------------------------------------
// waits for the signal to begin the procedure from the other half of the code 			
		while (signal1 == 0);
//----------------------------------------------------------------------
//Waits for the user input
		a = get_input();
//-------------------------------------------------------------------------
// Shows message to user on the filtration value required
			A:		
		uart_tx(0x0d);
		uart_msg("Range:00001 --->10000mL");
		uart_tx(0x0d);
		uart_msg("---> ");
//----------------------------------------------------------------------------------------------------------------			
// since 10^(4) values are needed, it loops 5 times to get the users inputs from the serial comms and concatenate them
// this is necessary since input from serial comms is character by character
			for(b = 0; b <=4; b++)
		{
			Filtration_Required = concatenate(Filtration_Required, get_filtration_vol());
			
// Checks whether the value inputted is out of range and restarts to point A to Input sequence again
			if( Filtration_Required > 10000)
		{
			uart_tx(0x0d);
			uart_msg("Error");
			uart_tx(0x0d);
			writecmd(0x01);
			Filtration_Required = 0;
			goto A;
		}
		}
//--------------------------------------------------------------------------------------------------------------
// restarts b for the next loop
		b = 0;
//--------------------------------------------------------------------------------------------------------------
//Sends a signal to the other code to begin its procedure	& Turns off data lines for LCD since Code 1 & Code 2 share the same LCD screen
		off_data_lines();
		send_signal();
//-----------------------------------------------------------------------------------------------------------------
// from the users input, it gains the required Elution level and shows it on the LCD
// The values are integers therefore it is crucial to single out integers and convert them to ASCII and show them to the 
// LCD character by character. +0x30 converts it to ASCII.
// variable i is reused to calculate Required Elution
			i = Filtration_Required*0.2;
			writecmd(0x88);
			writedata((i/1000)%10 +0x30);			
      writedata((i/100)%10 +0x30);
      writedata((i/10)%10 +0x30);
      writedata(i%10 +0x30);
			metric();
//--------------------------------------------------------------------------------------------------------------------
//This loop keeps on showing Current Elution until Current Elution is above or equal to Required Elution
			do
			{
			writecmd(0xc8);
			Elution_level = adc();
			show_adc(1, 8);
			Current_ADC_val = Elution_level*8;
			metric();
			} while(Current_ADC_val <= i);
//------------------------------------------------------------------------------------------------------------------	
//The Current_ADC_val is set to 0 so that it will enter the next loop 			
		Current_ADC_val = 0.0;
		writecmd(0x01);
//------------------------------------------------------------------------------------------------
//Turns off LCD data lines and waits until other part of code finishes its sequence
		off_data_lines();
		send_signal();
//---------------------------------------------------------------------------------------------------
//i is reused again to save memory as required flow and its displayed on the LCD			
			writecmd(0x80);
			write_string("Flw.req: ");
			writedata((i/1000)%10 +0x30);			
      writedata((i/100)%10 +0x30);
      writedata((i/10)%10 +0x30);
      writedata(i%10 +0x30);
			writecmd(0xc0);
			write_string("Curr.Flw: ");
//--------------------------------------------------------------------------------------------------
// While Current flow is below required flow it keeps on looping to show current flow			
			do
			{
			writecmd(0xc9);
			Water_flow = adc();
			show_adc(1, 8);
			Current_ADC_val = Water_flow*8;
			} while(Current_ADC_val < i);
//--------------------------------------------------------------------------------------------------			
//Turns off LCD data lines and waits until other part of code finishes its sequence		
			off_data_lines();
			
			send_signal();
			
			send_signal();
			
//--------------------------------------------------------------------------------------------------
//Shows the Required Water Filtration			
			writecmd(0x87);
			writedata((Filtration_Required/10000)%10 +0x30);
			writedata((Filtration_Required/1000)%10 +0x30);			
      writedata((Filtration_Required/100)%10 +0x30);
      writedata((Filtration_Required/10)%10 +0x30);
      writedata(Filtration_Required%10 +0x30);
			metric();
//-------------------------------------------------------------------------------------------------	
//While Current water level is below the required filtration it keeps on displaying the current water level			
			do
			{
			writecmd(0xc7);
			Water_level = adc();
			show_adc(2, 47);
			Current_ADC_val = Water_level*47;
			metric();
			} while (Current_ADC_val <= Filtration_Required);
//------------------------------------------------------------------------------------------------------------
//Turns off LCD data lines and waits until other part of code finishes its sequence		
// Current_ADC_val i set to 11000 to enter the while loop and i is restarted			
		off_data_lines();
		Current_ADC_val = 11000.0;
		i = 0;
		send_signal();
//-------------------------------------------------------------------------------------------------------------
// While Current water level is below water level after filtration, it keeps on showing the current level				
		while(Current_ADC_val > i)
		{		
//--------------------------------------------------------------------------------------------------------------
// Calculates the difference between current water level and required water filtration to get water after filtration
			i = Filtration_Required - adc()*47;
//--------------------------------------------------------------------------------------------------------------	
//if i is a negative number then it means that there is more water than necessary, so it is changed into a positive number			
			if( i < 0)
			{
				i = abs(i);
			}
//-----------------------------------------------------------------------------------------------------------------------	
//Shows water level after filtration			
			writecmd(0x87);
			writedata((i/10000)%10 +0x30);
			writedata((i/1000)%10 +0x30);			
      writedata((i/100)%10 +0x30);
      writedata((i/10)%10 +0x30);
      writedata(i%10 +0x30);
//----------------------------------------------------------------------------------------------------------------------------
//Current water level is above below water level then it displays the current water level and turns on the stepper motor
//Shows the pressure too
			
			while(Current_ADC_val > i)
			{
			
			writecmd(0xc7);	
			Water_level = adc();
			show_adc(2, 47);
			Current_ADC_val = Water_level*47;

	
			show_pressure();
			drive_stepper(5);
			}
		}
//---------------------------------------------------------------------------------------------------------------------------
///Turns off LCD data lines and waits until other part of code finishes its sequence		
// Current_ADC_val i set to 1000 to enter the while loop
		send_signal();
		off_data_lines();
		Current_ADC_val = 1000.0;
//--------------------------------------------------------------------------------------------------------------------------
//i is reused as elution after backflush. And displays the value
			i = (adc()*8) - (Filtration_Required*0.2);
		
			writecmd(0x80);
			write_string("El.req: ");
			writedata((i/1000)%10 +0x30);			
      writedata((i/100)%10 +0x30);
      writedata((i/10)%10 +0x30);
      writedata(i%10 +0x30);
			writecmd(0xc0);
			write_string("Tnk El.: ");
			metric();
//----------------------------------------------------------------------------------------------------------------------
//while Current elution tank is above elution after back flush level, display current elution tank and turn on stepper motor
			do
			{
			writecmd(0xc8);	
			Elution_level = adc();
			show_adc(1, 8);
			Current_ADC_val = Elution_level*8;
			metric();
				
			show_pressure();
			drive_stepper(5);
			} while(Current_ADC_val > i);
//----------------------------------------------------------------------------------------------------------------------
/// Turns off LCD data lines and waits until other part of code finishes its sequence		
			off_data_lines();
			send_signal();
//---------------------------------------------------------------------------------------------------------------------		
	}

}

//-------------------------------------------------------------------------------------------------------------------
//intitalises the ports

void cct_init(void)
{
P0 = 0x08;
P1 = 0x00;
P2 = 0x00;
P3 = 0x03;   
}
//-----------------------------------------------------------------------------------------------------------------------
//counts up in a for loop to create a delay, max is 1000

void delay(int a)
{
   int i;
   for(i=0;i<a;i++); 
}
//------------------------------------------------------------------------------------------------------------------------
//repeats delay function to increase delay time 

void delay_multiplier(int a)
{
	int i;
	
		for(i=0;i<a;i++)
	{
		delay(1000);
	}
}

//-------------------------------------------------------------------------------------------------------------------------
///writes data  on the LCD in 4 bit mode

void writedata(unsigned char t)
{
//-----------------------------------------------------------------------------------------------------
/// Configures the LCD on data input mode
		RS=1; 
//-----------------------------------------------------------------------------------------------	
//Sends the first 4 bits of the byte to the LCD	
    lcd&=0x0F;
    lcd|=(t&0xf0);
    E=1;
    delay(400);
    E=0;
//-----------------------------------------------------------------------------------------------
//Shifts the bits of the bytes 4 times to send the last 4 bytes of the character to the LCD
    delay(400);
    lcd&=0x0f;
    lcd|=(t<<4&0xf0);
    E=1;
    delay(400);
    E=0;
    delay(400);
//-------------------------------------------------------------------------------------------------

}
//--------------------------------------------------------------------------------------------------
//Writes sentences to the LCD
void write_string(unsigned char *a)
{
//-----------------------------------------------------------------------------------------------
//For each character in the string diplay it on the LCD
		while(*a) 
		{
        writedata(*a++);
    }
//-----------------------------------------------------------------------------------------------
}
//--------------------------------------------------------------------------------------------------
//This function writes command to LCD
void writecmd(unsigned char a)
{
//-----------------------------------------------------------------------------------------
// Sets LCD in instruction mode
    RS =0; 
//---------------------------------------------------------------------------------------------
// Sends the first 4 bits of the instruction byte to the LCD
    lcd&=0x0F;
    P1|=(a&0xf0);
    E =1;
    delay(1000);
    E =0;
    delay(1000);
//-------------------------------------------------------------------------------------------------
//Shifts the bits in the incstruction byte by 4 and sends the last 4 bits to the LCD
    lcd&=0x0f;
    lcd|=(a<<4&0xf0);
    E =1;
    delay(1000);
    E =0;
    delay(1000);
//-------------------------------------------------------------------------------------------
}

//-------------------------------------------------------------------------------------------
///initializes LCD by setting it up in 4 bit mode and etc.
void lcdinit(void)
{
		writecmd(0x02);
    writecmd(0x28);
    writecmd(0x0e);
    writecmd(0x06);
    writecmd(0x80);

}
//-----------------------------------------------------------------------------------------------------------------
//this function gets the value read by the adc as an integer
unsigned int adc()
{
    adc_data=0xff;
 
//------------------------------------------------------------------------------------------------------------------
//Prepares the ADC and Sets End of conversion to 1 to indicate that it is going to be converting analogue to digital
    ALE=START=OE=0;
    EOC=1;
//------------------------------------------------------------------------------------------------------------------
//Sends out the required pulse to latch in the address and start the conversion from analogue to digital
    ALE=1;
    START=1;
		delay(100);
    ALE=0;
    START=0;
//-------------------------------------------------------------------------------------------------------------------
//Waits until End of conversion indicates that conversion is finsished	
		while(EOC==1);
    while(EOC==0);
//-------------------------------------------------------------------------------------------------------------------
///Sends the necessary pulse to Output Enable to send tha ADC readings from ADC0808 internal register to Output pins
    OE=1;
    adc_val=adc_data;
	delay(100);
    OE=0;
//-----------------------------------------------------------------------------------------------------------------------
//returns the adc value
	return adc_val;
//-----------------------------------------------------------------------------------------------------------------------
}

//----------------------------------------------------------------------------------------------------------------------
//Initializes the baud rate to 9600 bits/sec
void SerialInitialize(void)      

{
//-----------------------------------------------------------------------------------------------------------------

TMOD=0x20;
//----------------------------------------------------------------------------------------------------------------
///Set serial mode 1, UART 8 bit mode and Receive Enable on
SCON=0x50;          
//-----------------------------------------------------------------------------------------------------------------
//Set Timer 1 Auto reload mode
TMOD=0x20; 
//------------------------------------------------------------------------------------------------------------------
//Values Calculated for 9600 baudrate	
TH1=0xFD; 
//-----------------------------------------------------------------------------------------------------------------
//Start the Timer
TR1=1;    
//----------------------------------------------------------------------------------------------------------------	
}

//--------------------------------------------------------------------------------------------------------------
//This Function sends message to UART RS-232
void uart_msg(unsigned char *c)

{
//-------------------------------------------------------------------------------------------------------------
///for each character in string, send serial data byte to serial comms
     while(*c != 0)
       {
					uart_tx(*c++);
       }
//------------------------------------------------------------------------------------------------------------
}

//------------------------------------------------------------------------------------------------------------
///This function sends the byte serial data to serial comms
void uart_tx(unsigned char serialdata)

{
///The serialdata is sent to the SBUF register of the serial comms
		SBUF = serialdata;
//---------------------------------------------------------------------------------------------------------------
//Waits until the byte is received for the serial comms and flags TI to 1 if its finished and manually sets TI to 0
		while(TI == 0);
		TI = 0;
//----------------------------------------------------------------------------------------------------------------
}

//----------------------------------------------------------------------------------------------------------------
//Concatenates the integers individually
unsigned concatenate(unsigned x, unsigned y) 
{
//--------------------------------------------------------------------------------------------------------------
//sets power to 10   
		unsigned pow = 10;
//--------------------------------------------------------------------------------------------------------------
//Finds out how many digits are present within the integer y
    while(y >= pow)
        pow *= 10;
//----------------------------------------------------------------------------------------------------------------
//Increases the number of digits with x and concatenates y
    return x * pow + y;
//-----------------------------------------------------------------------------------------------------------------		
}
//---------------------------------------------------------------------------------------------------------------
//This function gets the number character input from the user
unsigned int get_filtration_vol(void)
{	
//---------------------------------------------------------------------------------------------------------------
//sets the local variables needed for this function	
			int i;
			char a;
			int flag;
//----------------------------------------------------------------------------------------------------------------
//This character array contains all numbers as characters so it can be used to compare the user character input
			char b[] = "1234567890";
//----------------------------------------------------------------------------------------------------------------
			unsigned int filtration_vol;
	
					flag = 0;		
//----------------------------------------------------------------------------------------------------------------
//Gets the input from the user from the serial comms
					a = get_input();
	
//------------------------------------------------------------------------------------------------------------------
//For every character in the array, this function repeats and checks whether the character inputted by the user is a number present in the array b
//if not then a flag is set
	
					for(i = 0; i <= strlen(b); i++)
					{		
						if(b[i] == a)
							{
								flag = 1;
							}		
					}
//-----------------------------------------------------------------------------------------------------------------------------------------
// if flag is 1 then it purposely sends an error to the concatenate function and restart the number input again
					if (flag < 1)
					{	
						uart_tx(0x0d);
						return 99999;
					}
//----------------------------------------------------------------------------------------------------------------------------------------
//if the character inputted by the user passes the checks, it gets converted into an integer and returned to the function that needed it
					filtration_vol = atoi(&a);
					return filtration_vol;
//----------------------------------------------------------------------------------------------------------------------------------------
}

//---------------------------------------------------------------------------------------------------------------------------------------
unsigned char get_input()
{
//--------------------------------------------------------------------------------------------------------------------------------------
//initializes local variables
				unsigned char a;
//--------------------------------------------------------------------------------------------------------------------------------------
///Waits until serial data is received from the serial comms. RI is then manually set to 0.
					while(RI == 0);
					RI=0;
//----------------------------------------------------------------------------------------------------------------------------------------
//Shows the serial comms the inputted value and is stored in the variable as well
					a=SBUF;
					SBUF=a;
//----------------------------------------------------------------------------------------------------------------------------------------
///Serial comms TI is 1 since data is sent and is manually set to 0
					while(TI==0);
					TI = 0;
//-------------------------------------------------------------------------------------------------------------------------------------
//unisgned character a is returned to the main function
					return a;
	
//--------------------------------------------------------------------------------------------------------------------------------
}

//-----------------------------------------------------------------------------------------------------------------------------
///This function sends a signal to the part 1 of the code to indicate the sequence is finished and waits until a signal comes back
void send_signal()

{
			signal2 = 1;
			delay(1000);
			signal2 = 0;
			while(signal1 == 0);
}
//------------------------------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------------------------------
//This function shows the required display to the LCD, due to memory limitations. a is set as a local variable instead of 8 and 48
void show_adc(int n, int a)
{
	switch(n)
	{
		case 1:
			writedata((adc()*a/1000)%10 +0x30);
      writedata((adc()*a/100)%10 +0x30);
      writedata((adc()*a/10)%10 +0x30);
      writedata(adc()*a%10 +0x30);
			break;
		
		case 2:
			writedata((adc()*a/10000)%10 +0x30);
      writedata((adc()*a/1000)%10 +0x30);
			writedata((adc()*a/100)%10 +0x30);
			writedata((adc()*a/10)%10 +0x30);
      writedata(adc()*a%10 +0x30);
		break;
			
	}
}
//---------------------------------------------------------------------------------------------------------------------------------------
//This function drives the stepper motor sequence
void drive_stepper(int n)
{

		D1 = 0; D2 = 0; D0 = 1; D3 = 1;
    delay_multiplier(n);
    D2 = 0; D3 = 0; D0 = 1; D1 = 1;
    delay_multiplier(n);
    D1 = 0; D3 = 0; D1 = 1; D2 = 1;
    delay_multiplier(n);
    D0 = 0; D1 = 0; D2 = 1; D3 = 1;
    delay_multiplier(n);
		D0 = 0; D1 = 0; D2 = 0; D3 = 0;
}
//-----------------------------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------------------------
//This function shows the pressure gauge in the ADC, its not going to be integrated in the electronic system, its a mechanical feature
void show_pressure()
{
			writecmd(0x94);	
	write_string("Press.: ");
			ADC_pressure = 1;
			show_adc(2, 263);
			write_string("Pa");
	//------------------------------------------------------------------------------------------------------------------------------------
	//if ever pressure goes above 66kPa, it shuts down the motor
			while(adc()*263 > 66000)
			{
				P2=0;
				writecmd(0xD4);
				
				write_string("Error");
				writecmd(0xD4);
				write_string("          ");
			}
//-------------------------------------------------------------------------------------------------------------------------------------
			ADC_pressure = 0;

}
//------------------------------------------------------------------------------------------------------------------------------------
//This function turns off the LCD Data lines so that it does not interfere with the other code's LCD display
void off_data_lines()
{
	D4 = 0;
	D5 = 0;
	D6 = 0;
	D7 = 0;
	RS = 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
//This function merely shows the repeated metric
void metric()
{
	write_string("ml");
}
//----------------------------------------------------------------------------------------------------------------------------------------