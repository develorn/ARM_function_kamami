//******************************************************************************
//
//    THE SOFTWARE INCLUDED IN THIS FILE IS FOR GUIDANCE ONLY.
//    AUTHOR SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT
//    OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
//    FROM USE OF THIS SOFTWARE.
//
//******************************************************************************

#include "tft_pcf8833_lib.h"
#include "stm32f10x_gpio.h"

void TFTN_HW_Config(void)
{ //Hardware configuration (GPIO) for Nokia 6610 TFT (Philips PCF8833)
  //NOTE: since TFT needs 9-bit word transmision, SPI controller in STM32 CANNOT be used
  GPIO_InitTypeDef  GPIO_InitStructure; 

  //GPIO config

  GPIO_InitStructure.GPIO_Pin =TFTN_CLK | TFTN_MISO | TFTN_MOSI;    //SPI: SCK, MISO and MOSI lines
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(TFTN_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = TFTN_SS;                            //SPI: SS line
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(TFTN_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = TFTN_RES;                           //RES
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(TFTN_GPIO, &GPIO_InitStructure);
                                           
} 

void TFTN_Init(void){
  //Display initialisaton
  volatile unsigned long int i;

  GPIO_SetBits(TFTN_GPIO, TFTN_SS | TFTN_CLK |TFTN_MOSI);		
  GPIO_ResetBits(TFTN_GPIO, TFTN_RES);	//RES=0
  for (i=0;i<100000ul;i++);
  GPIO_SetBits(TFTN_GPIO, TFTN_RES);		//RES=1
  for (i=0;i<100000ul;i++);

	// Sleep out
	TFTN_SendCmd(SLEEPOUT);

	// Inversion on (command 0x20)
	TFTN_SendCmd(INVOFF); 

	// Color Interface Pixel Format (command 0x3A)
	TFTN_SendCmd(COLMOD);
	TFTN_SendData(0x03); // 0x03 = 12 bits-per-pixel (native)

	// Memory access controler (command 0x36)
	TFTN_SendCmd(MADCTL);
	TFTN_SendData(0x10); //display orientation

	for(i = 0; i < 2000; i++);

	// Display On (command 0x29)
	TFTN_SendCmd(DISPON);	

  // Contrast
  TFTN_SendCmd(SETCON);         
	TFTN_SendData(55);            //-64~63, this value must be set individually for your display
}

void TFTN_Transmit(unsigned char cData) {  
 //transmit datum to display
 	unsigned char i;
  GPIO_SetBits(TFTN_GPIO, TFTN_CLK);		 
  for (i = 0; i < 8; i++){
	  GPIO_ResetBits(TFTN_GPIO, TFTN_CLK);		 
    if ((cData&0x80))
  	  GPIO_SetBits(TFTN_GPIO, TFTN_MOSI);		 
	  else 
	    GPIO_ResetBits(TFTN_GPIO, TFTN_MOSI);		 
    GPIO_SetBits(TFTN_GPIO, TFTN_CLK);		 
    cData <<= 1;
  }
	GPIO_SetBits(TFTN_GPIO, TFTN_SS);		 
}


void TFTN_SendCmd(unsigned char cCmd){
  //Send command word to display
  GPIO_ResetBits(TFTN_GPIO, TFTN_SS | TFTN_CLK | TFTN_MOSI);		 //SCE=0 (ENABLE)
  TFTN_Transmit(cCmd);
}


void TFTN_SendData(unsigned char cData){
  //Send data word to display
  GPIO_ResetBits(TFTN_GPIO, TFTN_SS | TFTN_CLK);		 //SCE=0 (ENABLE)
  GPIO_SetBits(TFTN_GPIO, TFTN_MOSI);		 
  TFTN_Transmit(cData); 
}


void TFTN_Clear(unsigned int color){
  //Clear display
  TFTN_FillRect(0, 0, 131, 131, color);
}


void TFTN_WriteXY(const unsigned char *text, int xs, int ys, unsigned int fontColor, unsigned int bkgColor, const unsigned char * FontData){	
  //Write text at x,y position with a given foreground and background color
  unsigned int i, j, x, y;
  unsigned char pixelRow;
  unsigned char Mask=0x80;
  unsigned int Word0;
  unsigned int Word1;
  unsigned char charCode;

  x=xs;
  y=ys;
  while(*text!=0){
    if (x+FontData[0]>131){         //if next character is out of display, move to the next line
      y+=FontData[1];
      x=xs;
    }

   // TFTN_WriteChar(*text);
    TFTN_SendCmd(CASET);
    TFTN_SendData(x);
    TFTN_SendData(x+FontData[0]-1);	
      
    // Set row address range
    TFTN_SendCmd(PASET);
    TFTN_SendData(y);
    TFTN_SendData(y+FontData[1]-1);
    
    // Write memory
    TFTN_SendCmd(RAMWR);

    fontColor=fontColor&0xFFF;
    bkgColor=bkgColor&0xFFF;
    charCode=*text-0x1F;
    for(i = 0; i <FontData[1]; i++) {
      pixelRow=FontData[(charCode)*FontData[1]+i]; 
      Mask = 0x80;     
      for (j=0; j<FontData[0]; j+=2){
        if ((pixelRow & Mask) == 0)
          Word0 = bkgColor;
        else
          Word0 = fontColor;
        
        Mask = Mask >> 1;
        
        if ((pixelRow & Mask) == 0)
          Word1 = bkgColor;
        else
          Word1 = fontColor;
        Mask = Mask >> 1;

      	TFTN_SendData( (Word0 >> 4) & 0xFF);                         //pixel 1 RG
    	  TFTN_SendData(((Word0 & 0xF) << 4) | ((Word1 >> 8) & 0xF));  //pixel 1 B, pixel 2 R
    	  TFTN_SendData(  Word1 & 0xFF);                               //pixel 2 GB
      }
    }
    text++;
    x+=FontData[0];
    TFTN_SendCmd(NOP);
  }
} 


void TFTN_FillRect(int x0, int y0, int x1, int y1, unsigned int color){
  //Draw rectangle between x0,y0 and x1,y1
	unsigned int i;
  int swapBuf; 
  unsigned int Word0;

  if (x0>x1) {
    swapBuf=x1;
    x1=x0;
    x0=swapBuf;
  }

  if (y0>y1) {
    swapBuf=y1;
    y1=y0;
    y0=swapBuf;
  }
	// Set column address range
	TFTN_SendCmd(CASET);
	TFTN_SendData(x0);
	TFTN_SendData(x1);	
    
  // Set row address range
	TFTN_SendCmd(PASET);
	TFTN_SendData(y0);
	TFTN_SendData(y1);
  
	// Write memory
  TFTN_SendCmd(RAMWR);
  color=color&0xFFF;
	for(i = 0; i < (((x1-x0+1) * (y1-y0+1))/2+1); i++) 
	{
    Word0=color;
   	TFTN_SendData( (Word0 >> 4) & 0xFF);                           //pixel 1 RG
 	  TFTN_SendData(((Word0 & 0x0F) << 4) | ((Word0 >> 8) & 0x0F));  //pixel 1 B, pixel 2 R
 	  TFTN_SendData(  Word0 & 0xFF);                                 //pixel 2 GB
   
	}
  TFTN_SendCmd(NOP);
}


void TFTN_PutPixel(int x, int y, unsigned int color) {
  //Set a color of a pixel in x,y possition
  TFTN_FillRect(x, y, x, y, color);
}


void TFTN_Line(int x0, int y0, int x1, int y1, unsigned int color){
  //Draw line from x0,y0 to x1,y1
  //algorithm taken from http://www.sparkfun.com/tutorial/Nokia%206100%20LCD%20Display%20Driver.pdf
  //and                  http://www.cs.unc.edu/~mcmillan/comp136/Lecture6/Lines.html
  int dy = y1 - y0;
  int dx = x1 - x0;
  int stepx, stepy;
  int fraction;

  if (dy < 0) { dy = -dy; stepy = -1; } else { stepy = 1; }
  if (dx < 0) { dx = -dx; stepx = -1; } else { stepx = 1; }
  dy <<= 1; // dy is now 2*dy
  dx <<= 1; // dx is now 2*dx
  TFTN_PutPixel(x0, y0, color);
  if (dx > dy) {
    fraction = dy - (dx >> 1); // same as 2*dy - dx
    while (x0 != x1) {
      if (fraction >= 0) {
        y0 += stepy;
        fraction -= dx; // same as fraction -= 2*dx
      }
      x0 += stepx;
      fraction += dy; // same as fraction -= 2*dy
      TFTN_PutPixel(x0, y0, color);
    }
  } else {
    fraction = dx - (dy >> 1);
    while (y0 != y1) {
      if (fraction >= 0) {
        x0 += stepx;
        fraction -= dy;
      }
      y0 += stepy;
      fraction += dx;
      TFTN_PutPixel(x0, y0, color);
    }
  }
}


void TFTN_Circle(int x0, int y0, int radius, unsigned int color) {
  //Draw circle with a center in x0,y0 and a given radius
  //algorithm taken from http://www.sparkfun.com/tutorial/Nokia%206100%20LCD%20Display%20Driver.pdf

  int f = 1 - radius;
  int ddF_x = 0;
  int ddF_y = -2 * radius;
  int x = 0;
  int y = radius;

  TFTN_PutPixel(x0, y0 + radius, color);     //s1
  TFTN_PutPixel(x0, y0 - radius, color);     //s2
  TFTN_PutPixel(x0 + radius, y0, color);     //s3
  TFTN_PutPixel(x0 - radius, y0, color);     //s4
  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x + 1;
    TFTN_PutPixel(x0 + x, y0 + y, color);     //1
    TFTN_PutPixel(x0 - x, y0 + y, color);     //2
    TFTN_PutPixel(x0 + x, y0 - y, color);     //3
    TFTN_PutPixel(x0 - x, y0 - y, color);     //4
    TFTN_PutPixel(x0 + y, y0 + x, color);     //5
    TFTN_PutPixel(x0 - y, y0 + x, color);     //6
    TFTN_PutPixel(x0 + y, y0 - x, color);     //7
    TFTN_PutPixel(x0 - y, y0 - x, color);     //8
  }
}


void TFTN_FillCircle(int x0, int y0, int radius, unsigned int color) {
  //Draw circle filled with color with a center in x0,y0 and a given radius

  int f = 1 - radius;
  int ddF_x = 0;
  int ddF_y = -2 * radius;
  int x = 0;
  int y = radius;

  TFTN_FillRect(x0, y0 + radius, x0, y0 - radius, color); //s1-2  , draw rectangle instead of vertical line - faster
  TFTN_PutPixel(x0 + radius, y0, color);               //s3
  TFTN_PutPixel(x0 - radius, y0, color);               //s4
  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x + 1;

    TFTN_FillRect(x0 + x, y0 + y, x0 + x, y0 - y, color);  //1-3
    TFTN_FillRect(x0 - x, y0 + y, x0 - x, y0 - y, color);  //2-4
    TFTN_FillRect(x0 + y, y0 + x, x0 + y, y0 - x, color);  //5-7
    TFTN_FillRect(x0 - y, y0 + x, x0 - y, y0 - x, color);  //6-8
                                                
  }
}


void TFTN_WriteBMP(const unsigned int *buffer, int xPos, int yPos, int xSize, int ySize){
  //Draw bitmap
	unsigned int i;
  unsigned int Word0;
  unsigned int Word1;

	// Set column address range
	TFTN_SendCmd(CASET);
	TFTN_SendData(xPos);
	TFTN_SendData(xPos+xSize-1);	
    
  // Set row address range
	TFTN_SendCmd(PASET);
	TFTN_SendData(yPos);
	TFTN_SendData(yPos+ySize-1);
  
	// Write memory
  TFTN_SendCmd(RAMWR);

	for(i = 0; i < ((xSize * ySize)/2+1); i++) {
    Word0 = buffer[i*2];
    Word1 = buffer[i*2+1];
    TFTN_SendData( (Word0 >> 4) & 0xFF);                          //pixel 1 RG
    TFTN_SendData(((Word0 & 0x0F) << 4) | ((Word1 >> 8) & 0xF));  //pixel 1 B, pixel 2 R
    TFTN_SendData(  Word1 & 0xFF);                                //pixel 2 GB
	}
  TFTN_SendCmd(NOP); 
}
