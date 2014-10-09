//******************************************************************************
//    THE SOFTWARE INCLUDED IN THIS FILE IS FOR GUIDANCE ONLY.
//    AUTHOR SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT
//    OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
//    FROM USE OF THIS SOFTWARE.
//
//    PROGRAM ZAWARTY W TYM PLIKU PRZEZNACZONY JEST WYLACZNIE
//    DO CELOW SZKOLENIOWYCH. AUTOR NIE PONOSI ODPOWIEDZIALNOSCI
//    ZA ZADNE EWENTUALNE, BEZPOSREDNIE I POSREDNIE SZKODY
//    WYNIKLE Z JEGO WYKORZYSTANIA.
//******************************************************************************

#include "lcd_hd44780_lib.h"
#include "stm32f10x_gpio.h"

GPIO_InitTypeDef GPIO_InitStructure;

//-----------------------------------------------------------------------------
void LCD_WriteNibble(unsigned char nibbleToWrite)
{
  GPIO_WriteBit(LCD_GPIO, LCD_EN, Bit_SET);
  GPIO_WriteBit(LCD_GPIO, LCD_D4, (nibbleToWrite & 0x01));
  GPIO_WriteBit(LCD_GPIO, LCD_D5, (nibbleToWrite & 0x02));
  GPIO_WriteBit(LCD_GPIO, LCD_D6, (nibbleToWrite & 0x04));
  GPIO_WriteBit(LCD_GPIO, LCD_D7, (nibbleToWrite & 0x08));
  GPIO_WriteBit(LCD_GPIO, LCD_EN, Bit_RESET);
}


//-----------------------------------------------------------------------------
unsigned char LCD_ReadNibble(void)
{
  unsigned char tmp = 0;
  GPIO_WriteBit(LCD_GPIO, LCD_EN, Bit_SET);
  tmp |= (GPIO_ReadInputDataBit(LCD_GPIO, LCD_D4) << 0);
  tmp |= (GPIO_ReadInputDataBit(LCD_GPIO, LCD_D5) << 1);
  tmp |= (GPIO_ReadInputDataBit(LCD_GPIO, LCD_D6) << 2);
  tmp |= (GPIO_ReadInputDataBit(LCD_GPIO, LCD_D7) << 3);
  GPIO_WriteBit(LCD_GPIO, LCD_EN, Bit_RESET);
  return tmp;
}


//-----------------------------------------------------------------------------
unsigned char LCD_ReadStatus(void)
{
  unsigned char status = 0;
  
  GPIO_InitStructure.GPIO_Pin   =  LCD_D4 | LCD_D5 | LCD_D6 | LCD_D7;
  GPIO_InitStructure.GPIO_Mode  =  GPIO_Mode_IPU;
  GPIO_Init(LCD_GPIO, &GPIO_InitStructure);
  
  GPIO_WriteBit(LCD_GPIO, LCD_RW, Bit_SET);
  GPIO_WriteBit(LCD_GPIO, LCD_RS, Bit_RESET);
  
  status |= (LCD_ReadNibble() << 4);
  status |= LCD_ReadNibble();
  
  GPIO_InitStructure.GPIO_Pin   =  LCD_D4 | LCD_D5 | LCD_D6 | LCD_D7;
  GPIO_InitStructure.GPIO_Mode  =  GPIO_Mode_Out_PP;
  GPIO_Init(LCD_GPIO, &GPIO_InitStructure);
  
  return status;
}


//-----------------------------------------------------------------------------
void LCD_WriteData(unsigned char dataToWrite)
{
  GPIO_WriteBit(LCD_GPIO, LCD_RW, Bit_RESET);
  GPIO_WriteBit(LCD_GPIO, LCD_RS, Bit_SET);
  
  LCD_WriteNibble(dataToWrite >> 4);
  LCD_WriteNibble(dataToWrite & 0x0F);
  
  while(LCD_ReadStatus() & 0x80);
}


//-----------------------------------------------------------------------------
void LCD_WriteCommand(unsigned char commandToWrite)
{
  GPIO_WriteBit(LCD_GPIO, LCD_RW | LCD_RS, Bit_RESET);
  LCD_WriteNibble(commandToWrite >> 4);
  LCD_WriteNibble(commandToWrite & 0x0F);
  
  while(LCD_ReadStatus() & 0x80);
}


//-----------------------------------------------------------------------------
void LCD_WriteText(unsigned char * text)
{
  while(*text)
    LCD_WriteData(*text++);
}


//-----------------------------------------------------------------------------
void LCD_GoTo(unsigned char x, unsigned char y)
{
  LCD_WriteCommand(HD44780_DDRAM_SET | (x + (0x40 * y)));
}


//-----------------------------------------------------------------------------
void LCD_WriteTextXY(unsigned char * text, unsigned char x, unsigned char y)
{
  LCD_GoTo(x,y);
  while(*text)
    LCD_WriteData(*text++);
}


//-----------------------------------------------------------------------------
void LCD_WriteBinary(unsigned int var, unsigned char bitCount)
{
  signed char i;
  
  for(i = (bitCount - 1); i >= 0; i--)
     {
     LCD_WriteData((var & (1 << i))?'1':'0');
     }
}


//-----------------------------------------------------------------------------
void LCD_ShiftLeft(void)
{
  LCD_WriteCommand(HD44780_DISPLAY_CURSOR_SHIFT | HD44780_SHIFT_LEFT | HD44780_SHIFT_DISPLAY);
}


//-----------------------------------------------------------------------------
void LCD_ShiftRight(void)
{
  LCD_WriteCommand(HD44780_DISPLAY_CURSOR_SHIFT | HD44780_SHIFT_RIGHT | HD44780_SHIFT_DISPLAY);
}


//-----------------------------------------------------------------------------
void LCD_Initialize(void)
{
  volatile unsigned char i = 0;
  volatile unsigned int delayCnt = 0;
  
  GPIO_InitStructure.GPIO_Pin   =  LCD_D4|LCD_D5|LCD_D6|LCD_D7|LCD_RS|LCD_RW|LCD_EN;
  GPIO_InitStructure.GPIO_Speed =  GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode  =  GPIO_Mode_Out_PP;
  
  GPIO_Init(LCD_GPIO, &GPIO_InitStructure);
  
  GPIO_ResetBits(LCD_GPIO, LCD_RS | LCD_EN | LCD_RW);
  
  for(delayCnt = 0; delayCnt < 300000; delayCnt++);
  
  for(i = 0; i < 3; i++) {
    LCD_WriteNibble(0x03);            
    for(delayCnt = 0; delayCnt < 30000; delayCnt++);
  }
  
  LCD_WriteNibble(0x02);             
  
  for(delayCnt = 0; delayCnt < 6000; delayCnt++);
     
  LCD_WriteCommand(HD44780_FUNCTION_SET | 
                   HD44780_FONT5x7 | 
                   HD44780_TWO_LINE | 
                   HD44780_4_BIT);
  
  LCD_WriteCommand(HD44780_DISPLAY_ONOFF | 
                   HD44780_DISPLAY_OFF); 
  
  LCD_WriteCommand(HD44780_CLEAR); 
  
  LCD_WriteCommand(HD44780_ENTRY_MODE | 
                   HD44780_EM_SHIFT_CURSOR | 
                   HD44780_EM_INCREMENT);
  
  LCD_WriteCommand(HD44780_DISPLAY_ONOFF | 
                   HD44780_DISPLAY_ON |
                   HD44780_CURSOR_OFF | 
                   HD44780_CURSOR_NOBLINK);
}


//-----------------------------------------------------------------------------
void LCD_SetUserChar (unsigned char chrNum, unsigned char n, const unsigned char *p)
{         //chrNum  - character number (code) to be registered (0..7)
          //n       - number of characters to register
          //*p      - pointer to the character pattern (8 * n bytes)
	LCD_WriteCommand(HD44780_CGRAM_SET | chrNum * 8);
	n *= 8;
	do
		LCD_WriteData(*p++);
	while (--n);
}


//-----------------------------------------------------------------------------
