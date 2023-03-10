/* Main.c file generated by New Project wizard
 *
 * Created:   Tue Nov 9 2021
 * Processor: AT89C52
 * Compiler:  Keil for 8051
 */

//-----------------------------------------------------------------------------------------
//includes the library required for the program
#include <reg51.h> 
//---------------------------------------------------------------------------------------
//Initates the functions required so that it can be used
void change_state(int state);
void cct_init(void);
void delay(int);
void lcdinit(void);
void writecmd(unsigned char a);
void writedata(unsigned char t);
void write_string(unsigned char *a);
void InitI2c(void);
void StartI2c(void);
void StopI2c(void);
bit write_i2c(unsigned char byte);
void adc(int ch);
void write_byte_to_eeprom(unsigned int addr,unsigned char byte,unsigned int test);

void SerialInitialize(void);
void uart_msg(unsigned char *c);
void uart_tx(unsigned char serialdata);

void off_data_lines();

void send_signal();

//-----------------------------------------------------------------------------------------------
//define gives names to objects

#define I2C_DELAY 50
#define device_addr 0x40
#define device_addr_ADC 0x70
#define ACK_BIT 0
#define lcd P1

//--------------------------------------------------------------------------------
// Defines names for the Pins
sbit RS = P1^0;
sbit E  = P1^1;

sbit DC_Motor = P1^2;
sbit Check = P1^3;

sbit signal1 = P2^2;
sbit signal2 = P2^3;

sbit D4 = P1^4;
sbit D5 = P1^5;
sbit D6 = P1^6;
sbit D7 = P1^7;

sbit SDA_BUS = P2^0;
sbit SCL_BUS = P2^1;

//----------------------------------------------------------------------------------
//Starts the main void function
void main(void)
{
//----------------------------------------------------------------------------------
//initializes the local variables
		int i;
//---------------------------------------------------------------------------------
// initializes the required pins and ports	
		SDA_BUS = 0;
		SCL_BUS = 0;
		
		cct_init();                                     
		lcdinit();
		SerialInitialize();
		InitI2c();
//--------------------------------------------------------------------------------------
//Writes the name of the company when the system gets booted up	
				
		write_string("C0Nt Filtrations");
		uart_msg("C0Nt Filtrations");
		uart_tx(0x0d);
		delay(2000);
//---------------------------------------------------------------------------------------
//Start of the main program	

	 while(1)
    {
//------------------------------------------------------------------------------------------
//Sends a message to press a key to begin filtration to avoid accidentally filtrating or inputting a value
			uart_msg("Enter Any Key to Filtrate");
			uart_tx(0x0d);
			uart_msg("---> ");
//------------------------------------------------------------------------------------------------
/// sends a signal to the other code to get input from user	and turns off data lines to prevent interfering with the other codes LCD Display
			off_data_lines();
			send_signal();
//-------------------------------------------------------------------------------------------------
//Sends a message to user to notify that its initiating the valve check step of the sequence			
			uart_tx(0x0d);
			writecmd(0x01);
			uart_msg("Checking Valves 1 ---> 7");
			uart_tx(0x0d);
			write_string("Checking Valves ");
			writecmd(0xc4);
			uart_tx(0x0d);
//---------------------------------------------------------------------------------------------------
//This for loop checks the valves individually by changing the selectors
			
     for (i = 1; i <= 7; i++)
		{	
//--------------------------------------------------------------------------------------------------
//Shows the user which valve it is currently checking		
			writedata(i + 0x30);
			write_string("/7");
			delay(1000);
//-------------------------------------------------------------------------------------------------
//Changes the multiplexer address to the valve needed to be checked
			change_state(i);
//-----------------------------------------------------------------------------------------------
			writecmd(0x02);
			writecmd(0xc4);
			delay(1000);
//----------------------------------------------------------------------------------------------
//If check is equals to 1 then it indicates the valve is faulty and sends an error
			while (Check == 1)
			{
				writecmd(0xc4);
				uart_msg("Valve Faulty: ");
				uart_tx(i+0x30);
				uart_tx(0x0d);
				write_string("Valve ");
				writedata(i+0x30);
				write_string(" faulty");
				while(Check == 1);
			}
		}
//--------------------------------------------------------------------------------------------------
//It indicates the user that it is at the next sequence
		writecmd(0x01);
		uart_msg("Checking Tanks");
		uart_tx(0x0d);
		write_string("Checking Bacteria tank");
		writecmd(0xc4);
//--------------------------------------------------------------------------------------------------
//Changes the multiplexer address to chech if bacteria tank is completely empty
		change_state(8);
//--------------------------------------------------------------------------------------------------
//if liquid is detected within the bacteria tank then it sends an error and urge the user to empty it		
		while (Check == 1)
		{
			uart_msg("Please Empty Bacteria Tank");
		}	
		uart_msg("Bacteria Tank Check Satisfactory");
//-------------------------------------------------------------------------------------------------
//Changes the address channel, ABC for Elution Tank Analogue Sensor
		adc(3);
//------------------------------------------------------------------------------------------------
//Notify the user of the current step of the sequence which is to prepare the elution tank
		writecmd(0x01);
		uart_tx(0x0d);
		uart_msg("Preparing Elution Tank");
		writecmd(0x80);
		write_string("El.req: ");
		writecmd(0xC0);
		write_string("Tnk El.: ");
//---------------------------------------------------------------------------------------------------------------------------------
///sends a signal to the other code and waits until a signal is received from the other code that it is finished with the sequence
///Turns off LCD Data lines to avoid interfering with the LCD Display of the other code
		off_data_lines();
		send_signal();

//---------------------------------------------------------------------------------------------------------------------------------		
//Notifies the user that Elution Tank check passed and that the sequence now moves on to preparing the water tank
		writecmd(0x01);
		uart_tx(0x0d);
		uart_msg("Preparing Water tank");
		off_data_lines();
		delay(3000);
//-----------------------------------------------------------------------------------------------------------------------------------
//Open Valve 1
		write_byte_to_eeprom(0x0001, 0x01, 0x70);
		
//-----------------------------------------------------------------------------------------------------------------------------------
//Changes Analogue channel to analogue sensor for Flow meter
		adc(1);
//-----------------------------------------------------------------------------------------------------------------------------------
/// sends a signal to the other code to check if water flow has reached its required level		
		send_signal();
//-----------------------------------------------------------------------------------------------------------------------------------
		writecmd(0x80);
		writecmd(0x80);
		write_string("W.req: ");
		writecmd(0xC0);
		write_string("W.lvl: ");
		uart_tx(0x0d);
//----------------------------------------------------------------------------------------------------------------------------------
///Turns of LCD data lines
		off_data_lines();
//------------------------------------------------------------------------------------------------------------------------------------
//Changes analogue channel to analogue sensor for water tank
		adc(2);
//-------------------------------------------------------------------------------------------------------------------------------------
///sends a signal to the other code to check if water tank has reached its required level		
		send_signal();
		send_signal();
//-------------------------------------------------------------------------------------------------------------------------------------
//Notifies the user that water tank level has been reached and now its time for the filtration sequence
		uart_tx(0x0d);
		uart_msg("Water Tank required level reached");
		uart_tx(0x0d);
//---------------------------------------------------------------------------------------------------------------------------------
//Close Valve 1
		write_byte_to_eeprom(0x0001, 0 , 0x70);
//--------------------------------------------------------------------------------------------------------------------------------
		uart_tx(0x0d);
		uart_msg("Filtration Initiated");
		uart_tx(0x0d);
//-------------------------------------------------------------------------------------------------------------------------------
//Open Valve 2,3 and 4
		write_byte_to_eeprom(0x0001, 0xE, 0x70);
//------------------------------------------------------------------------------------------------------------------------------	
///sends a signal to the other code to check if water tank has reached its required level		
///Turns off data lines for LCD so that it does not affect the display of the other code
		send_signal();
		off_data_lines();
//-------------------------------------------------------------------------------------------------------------------------------
//Close Valve 2, 3, 4
		write_byte_to_eeprom(0x0001, 0, 0x70);
//-------------------------------------------------------------------------------------------------------------------------------
//Sends notification to the user that backflush is about to begin
		uart_msg("Water Filtration Finished");
		writecmd(0x01);
		delay(1000);
		
		uart_tx(0x0d);
	  uart_msg("Begin Backflush");
		uart_tx(0x0d);
//--------------------------------------------------------------------------------------------------------------------------------
//Open Valves 5, 6 and 7
		write_byte_to_eeprom(0x0001, 0x70, 0x70);
//---------------------------------------------------------------------------------------------------------------------------------
//Change analogue channel to Elution Tank analogue sensor
		adc(3);
//----------------------------------------------------------------------------------------------------------------------------------		
		writecmd(0x01);
		writecmd(0x80);
		write_string("El.req: ");
		writecmd(0xC0);
		write_string("Tnk El.: ");
//----------------------------------------------------------------------------------------------------------------------------------
//sends a signal to the other code to check if elution tank has reached its required level		
///Turns off LCD Data lines to avoid interfering the display of the other code
		off_data_lines();
		send_signal();
//----------------------------------------------------------------------------------------------------------------------------------
//Close Valve 5, 6 and 7
		write_byte_to_eeprom(0x0001, 0, 0x70);
//-----------------------------------------------------------------------------------------------------------------------------------
//Sends a message to user to notify that the whole filtration system is finished with its sequence
		uart_tx(0x0d);
	  uart_msg("Backflush Finished");
		uart_tx(0x0d);
		writecmd(0x01);
//----------------------------------------------------------------------------------------------------------------------------------
			
		}

}		

void cct_init(void)
{
P0 = 0x08;
P1 = 0x00;
P2 = 0x00;
P3 = 0xF;   
write_byte_to_eeprom(0x0001, 0 , 0x70);   //used for generating E and RS 
}

void delay(int a)
{
   int i;
   for(i=0;i<a;i++);   //null statement
}

void writedata(unsigned char t)
{
		RS=1; 
    lcd&=0x0F;
    lcd|=(t&0xf0);
    E=1;
    delay(1000);
    E=0;
    delay(1000);
    lcd&=0x0f;
    lcd|=(t<<4&0xf0);
    E=1;
    delay(1000);
    E=0;
    delay(1000);

}

void write_string(unsigned char *a)
{
		while(*a) 
		{
        writedata(*a++);
    }

}


void writecmd(unsigned char a)
{
    RS =0; 
    lcd&=0x0F;
    P1|=(a&0xf0);
    E =1;
    delay(1000);
    E =0;
    delay(1000);
    lcd&=0x0f;
    lcd|=(a<<4&0xf0);
    E =1;
    delay(1000);
    E =0;
    delay(1000);

}

void lcdinit(void)
{
		writecmd(0x02);
    writecmd(0x28);
    writecmd(0x0e);
    writecmd(0x06);
    writecmd(0x80);

}


//--------------------------------------------------------------------------------------------
//Function Starts the SDA and SCL sequence to indicate to start writing to EEPROM
void StartI2c(void)
{
    SDA_BUS = 1;
    SCL_BUS  = 1;
    delay(50);
    SDA_BUS  = 0;
    delay(50);
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//SDA and SCL sequence required to let the EEPROM know that is finished
void StopI2c(void)
{
    SCL_BUS  = 0;
    delay(25);
    SDA_BUS  = 0;
    delay(25);
    SCL_BUS  = 1;
    delay(25);
    SDA_BUS  = 1;
    delay(50);
}

//--------------------------------------------------------------------------------------------
///This Function writes data to I2C
bit write_i2c(unsigned char byte)
{
//-----------------------------------------------------------------------------------------------------------------------------
// initiates a local variable
unsigned char i;
//--------------------------------------------------------------------------------------------------------------------------------
///for each bit in byte, if bit is 1 then set SDA_BUS to 0, else if 0 then set SDA_BUS to 0. After, set SCL to 1 to indicate that
///it is finished writing the bit
for(i=0; i<8; i++)
{
SCL_BUS = 0;
delay(I2C_DELAY);
if((byte<<i)&0x80)
SDA_BUS = 1;
else
SDA_BUS = 0;
delay(I2C_DELAY/2);
SCL_BUS = 1;
delay(I2C_DELAY);
}
//----------------------------------------------------------------------------------------------------------------------
//Initiates the Acknowledge sequence of I2C to indicate that a byte is finsihed writing
SCL_BUS = 0;
SDA_BUS = 0;
delay(I2C_DELAY/2);
SCL_BUS = 1;
delay(I2C_DELAY);
//----------------------------------------------------------------------------------------------------------------------
//Sends a signal that it finished its operation
return SDA_BUS;
//-----------------------------------------------------------------------------------------------------------------------
}

//------------------------------------------------------------------------------------------------------------------------
//initializes I2C to perform properly
void InitI2c(void)
{
SDA_BUS =1;
SCL_BUS =1;
}

//----------------------------------------------------------------------------------------------------------------------
//This Function writes a byte to EEPROM, some EEPROM have word address hence the reason for having a third address
void write_byte_to_eeprom(unsigned int addr,unsigned char byte, unsigned int device_address)
{
StartI2c();
//---------------------------------------------------------------------------------------------------------------------
//it overwrites the slave address to ensure that it is in write mode and sends it to the required EEPROM
while(write_i2c(device_address+0)==1)
{
StartI2c();
}
//-------------------------------------------------------------------------------------------------------------------
//Writes the Word address to the EEPROM
write_i2c(addr>>8);
write_i2c((unsigned char)addr);
//-------------------------------------------------------------------------------------------------------------------
//Writes the byte inputted by the user
write_i2c(byte);
//--------------------------------------------------------------------------------------------------------------------
//Lets the I2C know that it has finished the operation
StopI2c();
//----------------------------------------------------------------------------------------------------------------------
}


//----------------------------------------------------------------------------------------------------------------------
//This function changes the address A, B and C to select the required channel
void change_state(int state)
{
	switch(state)
	{	 
	 	case 1:
//-----------------------------------------------------------------------------------------------------------------------
//Selects Valve 1 or Flow meter analogue channel
			write_byte_to_eeprom(0x0001, 0, 0x40);
			break;
//----------------------------------------------------------------------------------------------------------------------------		
		case 2:
//-----------------------------------------------------------------------------------------------------------------------
//Selects Valve 2 or Water Tank Analogue Sensor
			write_byte_to_eeprom(0x0001, 0x01, 0x40);
			break;
//-----------------------------------------------------------------------------------------------------------------------
		
		case 3:
//-----------------------------------------------------------------------------------------------------------------------
//Selects Valve 3 or Elution Tank Analogue Sensor
			write_byte_to_eeprom(0x0001, 0x02, 0x40);
			break;	
//-----------------------------------------------------------------------------------------------------------------------
		
		case 4:
//-----------------------------------------------------------------------------------------------------------------------
//Selects Valve 4
			write_byte_to_eeprom(0x0001, 0x03, 0x40);
			break;
//-----------------------------------------------------------------------------------------------------------------------
		
		case 5:
//-----------------------------------------------------------------------------------------------------------------------
//Selects Valve 5

			write_byte_to_eeprom(0x0001, 0x04, 0x40);
			break;
//-----------------------------------------------------------------------------------------------------------------------
		
		case 6:
//-----------------------------------------------------------------------------------------------------------------------
//Selects Valve 6
			write_byte_to_eeprom(0x0001, 0x05, 0x40);
			break;
//-----------------------------------------------------------------------------------------------------------------------
		
		case 7:
//-----------------------------------------------------------------------------------------------------------------------
//Selects Valve 7
			write_byte_to_eeprom(0x0001, 0x06, 0x40);
			break;
//-----------------------------------------------------------------------------------------------------------------------
		
		case 8:
//-----------------------------------------------------------------------------------------------------------------------
//Selects Bacteria Concentrate Tank
			write_byte_to_eeprom(0x0001, 0x07, 0x40);
			break;
//-----------------------------------------------------------------------------------------------------------------------
	}
}

//-----------------------------------------------------------------------------------------------------------------------
///renamed to make it more understandable within the code
void adc(int ch)
{
    change_state(ch);
}
//------------------------------------------------------------------------------------------------------------------------
void SerialInitialize(void)       //Initialize Serial Port

{
TMOD=0x20;
SCON=0x50;          //Mode 1, Baudrate generating using Timer 1
TMOD=0x20;          //Timer 1 Auto reload mode
TH1=0xFD;       //Values Calculated for 9600 baudrate
TR1=1;              //Run the timer
}

void uart_msg(unsigned char *c)

{
     while(*c != 0)
       {
					uart_tx(*c++);
       }
}

void uart_tx(unsigned char serialdata)

{
		SBUF = serialdata;
		while(TI == 0);
		TI = 0;
}

void send_signal()

{
			signal1 = 1;
			delay(1000);
			signal1 = 0;
			while(signal2 == 0);
}
	
void off_data_lines()
{
	D4 = 0;
	D5 = 0;
	D6 = 0;
	D7 = 0;
	RS = 0;
}