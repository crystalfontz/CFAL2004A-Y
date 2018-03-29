//===========================================================================
//
//  Code written for Seeeduino v4.2 set to 3.3v (important!)
//
//  CRYSTALFONTZ CFAL2002A-Y Character OLED Display
//
//  This code uses the 8-bit parallel MCU mode of the display.
//    https://www.crystalfontz.com/product/cfal2004ay
//
//
//  The controller is a Winstar WS0010:
//    https://www.crystalfontz.com/controllers/Winstar%20Display/WS0010
//
//  Seeeduino v4.2, an open-source 3.3v capable Arduino clone.
//    https://www.seeedstudio.com/Seeeduino-V4.2-p-2517.html
//    https://github.com/SeeedDocument/SeeeduinoV4/raw/master/resources/Seeeduino_v4.2_sch.pdf
//============================================================================
//
//
//
//===========================================================================
//This is free and unencumbered software released into the public domain.
//
//Anyone is free to copy, modify, publish, use, compile, sell, or
//distribute this software, either in source code form or as a compiled
//binary, for any purpose, commercial or non-commercial, and by any
//means.
//
//In jurisdictions that recognize copyright laws, the author or authors
//of this software dedicate any and all copyright interest in the
//software to the public domain. We make this dedication for the benefit
//of the public at large and to the detriment of our heirs and
//successors. We intend this dedication to be an overt act of
//relinquishment in perpetuity of all present and future rights to this
//software under copyright law.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
//OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
//ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
//OTHER DEALINGS IN THE SOFTWARE.
//
//For more information, please refer to <http://unlicense.org/>
//============================================================================



//============================================================================
// LCD & USD control lines
//   ARD      | Port | CFA2004A   |  Function                | Wire
//------------+------+------------+--------------------------+------------
//  5V        |      | #2         |  POWER 5V                | Yellow
//  GND       |      | #1         |  GROUND                  | Purple
// -----------+------+------------+--------------------------+------------
//  #8/D8     |  PB0 | #4         |  Data/Instruction   (RS) | Yellow
//  #9/D9     |  PB1 | #6         |  Chip Enable Signal (CS) | Purple
// #10/D10    |  PB2 | #5         |  Read/Write         (RW) | Grey
// -----------+------+------------+--------------------------+------------
//  #0/D0     |  PD0 | #7         |  LCD_D10 (DB0)           | Black
//  #1/D1     |  PD1 | #8         |  LCD_D11 (DB1)           | Brown
//  #2/D2     |  PD2 | #9         |  LCD_D12 (DB2)           | Red
//  #3/D3     |  PD3 | #10        |  LCD_D13 (DB3)           | Orange
//  #4/D4     |  PD4 | #11        |  LCD_D14 (DB4)           | Yellow
//  #5/D5     |  PD5 | #12        |  LCD_D15 (DB5)           | Green
//  #6/D6     |  PD6 | #13        |  LCD_D16 (DB6)           | Blue
//  #7/D7     |  PD7 | #14        |  LCD_D17 (DB7)           | Purple
// -----------+------+------------+--------------------------+------------
//============================================================================


#include <SoftwareSerial.h>

SoftwareSerial mySerial(18, 19);
#define MCU_6800 0
#define MCU_8080 1
#define MCU_SPI  2

#define INTERFACE MCU_6800

#if (INTERFACE == MCU_8080)
#define CLR_RS     (PORTB &= ~(0x01)) //pin #8  - Data/Instruction
#define SET_RS     (PORTB |=  (0x01)) //pin #8  - Data/Instruction
#define CLR_CS     (PORTB &= ~(0x02)) //pin #9  - Chip Enable Signal
#define SET_CS     (PORTB |=  (0x02)) //pin #9  - Chip Enable Signal
#define CLR_RW	   (PORTB &= ~(0x04)) //pin #10 - Read/Write
#define SET_RW	   (PORTB |=  (0x04)) //pin #10 - Read/Write
#elif (INTERFACE == MCU_6800)
#define CLR_RS     (PORTB &= ~(0x01)) //pin #8  - Data/Instruction
#define SET_RS     (PORTB |=  (0x01)) //pin #8  - Data/Instruction
#define CLR_E      (PORTB &= ~(0x02)) //pin #9  - Chip Enable Signal
#define SET_E      (PORTB |=  (0x02)) //pin #9  - Chip Enable Signal
#define CLR_RW	   (PORTB &= ~(0x04)) //pin #10 - Read/Write
#define SET_RW	   (PORTB |=  (0x04)) //pin #10 - Read/Write
#endif
#define LCD_DATA   (PORTD)
//#define LCD_WRITE(x) LCD_DATA=(x);CLR_nWR;SET_nWR

uint8_t pointer = 0;

//============================================================================
void sendCommand(uint8_t command)
{
	checkBusy();

	//Put the command on the port
	LCD_DATA = command;
	
	// Select the LCD's command register
	CLR_RS;

	CLR_RW;

	// Deselect the LCD controller
	SET_E;
	CLR_E;

}

//----------------------------------------------------------------------------
uint8_t sendData(uint8_t data)
{
	//Put the data on the port
	LCD_DATA = data;

	// Select the LCD's command register
	SET_RS;

	CLR_RW;

	// Deselect the LCD controller
	SET_E;
	CLR_E;
	
}

uint8_t checkBusy()
{
	PORTD = 0x80;
	DDRD = 0x00;
	
	CLR_RS;

	SET_RW;

	// Select the LCD controller
	SET_E;

	//Watch Pin 7 for the busy flag
	while (0x80 == (PIND & 0x80));
	CLR_E;


	DDRD = 0xFF;

	CLR_RW;

	return LCD_DATA;
}

void init_1602b()
{
	
	//Power on sequence
	//1. Display clear
	//Display Clear RS=0,R/W=0
	// 7654 3210
	// 0000 0001
	sendCommand(0x01);
	//2. Function set:
	//DL="1": 8-bit interface data
	//N="0": 1-line display
	//F="0": 5 x 8 dot character font
	//Function set, RS=0,R/W=0
	// 7654 3210
	// 001D NFFT
	//  D = Data Length
	//  N = lines: N=1 is 2 lines
	//  F = Font: 0 = 5x8, 1 = 5x10
	//  FT = Font Table:
	//     FT=00 is English/Japanese ~"standard" for character LCDs
	//     FT=01 is Western European I fractions, circle-c some arrows
	//     FT=10 is English/Russian
	//     FT=11 is Western European II my favorite, arrows, Greek letters
	sendCommand(0x30);

	//3. Power turn off
	//PWR=�0�
	//Graphic vs character mode setting, RS=0,R/W=0
	// 7654 3210
	// 0001 GP11
	//  G = Mode: 1=graphic, 0=character
	//  C = Power: 1=0n, 0=off
	sendCommand(0x13);

	//4. Display on/off control:
	//D="0": Display off
	//C="0": Cursor off
	//B="0": Blinking off
	//Display On/Off Control RS=0,R/W=0
	// 7654 3210
	// 0000 1DCB
	//  D = Display On
	//  C = Cursor On
	//  B = Cursor Blink
	sendCommand(0x08);

	//5. Entry mode set
	//I/D="0": Decrement by 1
	//S="0": No shift
	//Entry Mode Set RS=0,R/W=0
	// 7654 3210
	// 0000 01IS
	//  I = Increment/or decrement
	//  S = Shift(scroll) data on line
	sendCommand(0x04);

	//6. Cursor/Display shift/Mode / Pwr
	//S/C=�0�, R/L=�1�: Shifts cursor position to the right
	//G/C=�0�: Character mode
	//Pwr=�1�: Internal DCDC power on 
	// Cursor/Display shift/Mode / Pwr
	// 7654 3210
	// 0001 GP11
	//  G = Mode: 1=graphic, 0=character
	//  C = Power: 1=0n, 0=off
	sendCommand(0x14);
	sendCommand(0x17);
	
	

	delay(500);
	/*CLR_E;
	CLR_RS;
	CLR_RW;*/

	checkBusy();
	//Function Set
	sendCommand(0x3B);	//00111011
		//	0011XXXX
		//	    ||||---	Font Table Selection
		// 	    ||		  00: English_Japenese character font table
		//		||		  01: Western European character font table - I
		//		||		  10: English_Russian character font table
		//		||		>>11: Western European character font table - II
		//		||-----	Character Font Set
		//		|		>>0: 5x8 dot character font
		//		|		  1: 5x10 dot character font
		//		|------	Interface Data Length Control Bit	
		//				  0: data is sent or received in 4-bit length DB4-DB7
		//				>>1: data is sent or received in 8-bit length DB4-DB7

	//Display ON/OFF Control
	sendCommand(0x0C);	//0001100
		//	00001XXX
		//		 |||--	Blinking Control Bit
		//		 ||		  1: Cursor Blinking ON
		//		 ||		>>0: Cursor Blinking OFF
		//		 ||---	Cursor Display Control
		//		 |		>>0: Cursor OFF
		//		 |		  1: Cursor ON
		//		 |----	Display ON/OFF
		//		  		  0: Display OFF
		//		  		>>1: Display ON

	//Display Clear 00000001
	sendCommand(0x01);

	//Return Home 00000010
	sendCommand(0x02);

	//sendCommand(0x17);
	//Graphic vs character mode setting, RS=0,R/W=0
	// 7654 3210
	// 0001 GP11
	//  G = Mode: 1=graphic, 0=character
	//  C = Power: 1=0n, 0=off

	//Entry Mode Set
	sendCommand(0x06); //00000110
		//	000001XX
		//		  ||--	Shift Entire Display Control Bit
		//		  | >>0: Decrement DDRAM Address by 1 when a character
		//		  |		 code is written into or read from DDRAM
		//		  |   1: Increment DDRAM Address by 1 when a character
		//		  |		 code is written into or read from DDRAM
		//		  |
		//		  |---	Increment/Decrement bit
		//		  	  0: when writing to DDRAM, each
		//		  		 entry moves the cursor to the left
		//		  	>>1: when writing to DDRAM, each 
		//		  		 entry moves the cursor to the right

	//set DDRAM
	sendCommand(0x80);


}
void setup()
{
	delay(500);

  mySerial.begin(9600);
  mySerial.println("Setup started");

	DDRD = 0xFF;
	PORTD = 0x00;

	DDRB = 0x07;
	PORTB = 0x00;

	CLR_RW;
	init_1602b();
}



void writeString(char* myString)
{
	uint8_t i = 0;
	
  while (myString[i] != NULL)
	{
		sendData((uint8_t)myString[i]);
		i++;
	} 
}

void loop()
{

  mySerial.println("loop");

  //line 1
  sendCommand(0x80 | 0);
  writeString("*** Crystalfontz ***");
  
  //line 2
  sendCommand(0x80 | 64);
  writeString("** 20x4 Character **");
  
  //line 3
  sendCommand(0x80 | 20);
	writeString("* CFAL2004A Family *");
  
  //line 4
  sendCommand(0x80 | 84);
  writeString("Passive Matrix OLED");
  
	delay(5000);

	sendCommand(0x01);
  delay(2000);


}
