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

#include "mems1_lib.h"
#include "stm32f10x_gpio.h"


//------------------------------------------------
void MEMS1_GPIOConfig(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure; 

  GPIO_InitStructure.GPIO_Pin = MEMS1_PC | MEMS1_LC | MEMS1_SGN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(MEMS1_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = MEMS1_PD;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(MEMS1_GPIO, &GPIO_InitStructure);

  GPIO_SetBits(MEMS1_GPIO, MEMS1_PD);		 //power up
}


//------------------------------------------------
char MEMS1_ReadPos(void)
{
  //0 = Front up
  //4 = Front down
  //1 = Landscape left
  //5 = Landcape right
  //2 = Portrait up
  //6 = Portrait down
  //3 = 45 deg (any position)
  char stan;
  stan=(GPIO_ReadInputDataBit(MEMS1_GPIO, MEMS1_SGN)<<2);
  stan+=(GPIO_ReadInputDataBit(MEMS1_GPIO, MEMS1_PC)<<1);
  stan+=(GPIO_ReadInputDataBit(MEMS1_GPIO, MEMS1_LC));
  return stan;
}

//------------------------------------------------
void MEMS1_Power(unsigned char Power)
{
  if (Power==MEMS1_POWER_DOWN)
    GPIO_ResetBits(MEMS1_GPIO, MEMS1_PD);	 //power down	 
  else
    GPIO_SetBits(MEMS1_GPIO, MEMS1_PD);		 //power up
}
