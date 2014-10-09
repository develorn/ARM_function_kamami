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

#include "lcd_pcd8544_lib.h"
#include "font1.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"

void LCDN_HwConfig(void)
{  //Hardware configuration (GPIO, SPI) for Nokia LCD
  GPIO_InitTypeDef  GPIO_InitStructure; 
  SPI_InitTypeDef   SPI_InitStructure;

  //Enable clocks
  //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);           //GPIO A Clock
  //RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);            //SPI Clock
  
  //GPIO config
  GPIO_InitStructure.GPIO_Pin =LCDN_CLK | LCDN_DIN | LCDN_DOU;    //SPI: SCK, MISO and MOSI lines
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(LCDN_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCDN_SCE;                         //SPI: SS line
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(LCDN_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCDN_RES | LCDN_DC;               //RES, D/C lines
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(LCDN_GPIO, &GPIO_InitStructure);

  //SPI1 Config
  SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(LCDN_SPI, &SPI_InitStructure);

  SPI_Cmd(LCDN_SPI, ENABLE);                                         //SPI1 Enable
} 


void LCDN_Init(void){
  //Display initialisaton
  volatile unsigned long int i;

  GPIO_ResetBits(LCDN_GPIO, LCDN_RES);	//RES=0
  for (i=0;i<100000ul;i++);
  GPIO_SetBits(LCDN_GPIO, LCDN_RES);		//RES=1

  GPIO_ResetBits(LCDN_GPIO, LCDN_SCE);	//SCE=0 (ENABLE)
  GPIO_ResetBits(LCDN_GPIO, LCDN_DC);		//D/C=0 (ENABLE) - control commands will be transfered next
  LCDN_SPI_Transmit(0x21);              //extended commands
  LCDN_SPI_Transmit(0x05);              //graphics positionig - needed for non-original PCD8544
  LCDN_SPI_Transmit(0xc8);              //Vop setting
  LCDN_SPI_Transmit(0x06);              //temperature compensation
  LCDN_SPI_Transmit(0x13);              //mux coefficient
  LCDN_SPI_Transmit(0x20);              //standard commands - hotizontal addressing
  //LCDN_SPI_Transmit(0x01);              //??
  LCDN_SPI_Transmit(0x0c);              //standard mode display
  LCDN_SPI_Transmit(0x40);              //rows counter reset
  LCDN_SPI_Transmit(0x80);              //columns counter reset

  GPIO_SetBits(LCDN_GPIO, LCDN_SCE);		//D/C=1 - data will be transfered next
  GPIO_SetBits(LCDN_GPIO, LCDN_DC);		  //SCE=1 (DISABLE)  
}


void LCDN_Mode(unsigned char mode){
  //Display mode setting
  //negative (mode = 1) or normal (mode = 0) display mode

  GPIO_ResetBits(LCDN_GPIO, LCDN_SCE);	//SCE=0 (ENABLE)
  GPIO_ResetBits(LCDN_GPIO, LCDN_DC);		//D/C=0 (ENABLE) - control commands will be transfered next
  if (mode==1){
    LCDN_SPI_Transmit(0x0D);            //Negative mode
  } else {
    LCDN_SPI_Transmit(0x0C);            //Normal mode
  }
  GPIO_SetBits(LCDN_GPIO, LCDN_SCE);		//D/C=1 - data will be transfered next
  GPIO_SetBits(LCDN_GPIO, LCDN_DC);		  //SCE=1 (DISABLE) 
}


void LCDN_Clear(void){
 //Clear display
 unsigned char w,k;
 for (w=0;w<6;w++){
   LCDN_SetPos(0, w);
   for (k=0;k<84;k++){
     LCDN_SPI_Transmit(0x00);
   }
 }
}


void LCDN_WriteChar(unsigned char charCode){
  //Write one character
  unsigned int codeStart, codeEnd, i;

  codeStart=((unsigned int)charCode)-LCDN_FONT_OFFSET;
  codeStart=codeStart*LCDN_FONT_WIDTH;
  GPIO_SetBits(LCDN_GPIO, LCDN_DC);      //D/C=1 (ENABLE) - data will be transfered next
  GPIO_ResetBits(LCDN_GPIO, LCDN_SCE);	 //SCE=0 (ENABLE)
  codeEnd=codeStart+(unsigned int)LCDN_FONT_WIDTH;
  for (i=codeStart; i<codeEnd; i++){
    LCDN_SPI_Transmit(LCDN_Font[i]);
  }
  GPIO_SetBits(LCDN_GPIO, LCDN_DC);		  //SCE=1 (DISABLE)
}


void LCDN_WriteXY(const unsigned char *text, char x,char y){	
  //Write text at XY position 
  unsigned char textPixLength;

  textPixLength=x*LCDN_FONT_WIDTH;
  LCDN_SetPos(x,y);                     //Set position
  while(*text!=0) {
    LCDN_WriteChar(*text);
    text++;
    textPixLength+=LCDN_FONT_WIDTH;
    if (textPixLength>=84){             //if next character is out of display, move to the next line
      LCDN_SetPos(x,++y);               //Set position
      textPixLength=x*LCDN_FONT_WIDTH;
    }
  }
} 


void LCDN_SetPos(unsigned char x,unsigned char y){
  //Set XY position, counted in char size
  GPIO_ResetBits(LCDN_GPIO, LCDN_DC);	  //D/C=0 (ENABLE) - control commands will be transfered next
  GPIO_ResetBits(LCDN_GPIO, LCDN_SCE);	//SCE=0 (ENABLE)
  LCDN_SPI_Transmit(y | 0x40);          //rows counter reset
  x=x*LCDN_FONT_WIDTH;
  LCDN_SPI_Transmit(x | 0x80);          //columns counter reset
  GPIO_SetBits(LCDN_GPIO, LCDN_SCE);		//D/C=1 - data will be transfered next
  GPIO_SetBits(LCDN_GPIO, LCDN_DC);		  //SCE=1 (DISABLE)
}


void LCDN_SetPosG(unsigned char x,unsigned char y){
  //Set XY position, X counted in pixels
  GPIO_ResetBits(LCDN_GPIO, LCDN_DC);	  //D/C=0 (ENABLE) - control commands will be transfered next
  GPIO_ResetBits(LCDN_GPIO, LCDN_SCE);  //SCE=0 (ENABLE)
  LCDN_SPI_Transmit(y | 0x40);          //rows counter reset
  LCDN_SPI_Transmit(x | 0x80);          //columns counter reset
  GPIO_SetBits(LCDN_GPIO, LCDN_SCE);		//D/C=1 - data will be transfered next
  GPIO_SetBits(LCDN_GPIO, LCDN_DC);		  //SCE=1 (DISABLE)
}


void LCDN_WriteBMP(const unsigned char *buffer){	
  //Write (display) bitmap, bitmap must have 504 words
  unsigned int w,k;

  LCDN_SetPosG(0,0);               
  k=w=0;
  for(w=0;w<6;w++){
    LCDN_SetPosG(0,w);                  //set next row
    for(k=0;k<84;k++)
      LCDN_SPI_Transmit(*buffer++);	
  }
  GPIO_SetBits(LCDN_GPIO, LCDN_DC);	    //SCE=1 (DISABLE)
} 


void LCDN_SPI_Transmit(char cData) {  
 //transmit datum to display on SPI1
  while (SPI_I2S_GetFlagStatus(LCDN_SPI, SPI_I2S_FLAG_BSY) == SET);    //SCE changes state for every datum, this slows down transmission a bit, but reduces errors
  GPIO_ResetBits(LCDN_GPIO, LCDN_SCE);  //SCE=0 (ENABLE)
  SPI_I2S_SendData(LCDN_SPI, cData);
  while (SPI_I2S_GetFlagStatus(LCDN_SPI, SPI_I2S_FLAG_BSY) == SET);
  GPIO_SetBits(LCDN_GPIO, LCDN_SCE);		//SCE=1 (DISABLE)
}


//void LCDN_SPI_CS_Enable(void)
//{
//	GPIO_ResetBits(LCDN_GPIO, LCDN_SCE);
//}

//void LCDN_SPI_CS_Disable(void)
//{
//	GPIO_SetBits(LCDN_GPIO, LCDN_SCE);
//}  










