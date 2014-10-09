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

#include "kb4x4_lib.h"
#include "stm32f10x_gpio.h"

const char keyMap[4][4]={{KBD_R1C1, KBD_R1C2, KBD_R1C3, KBD_R1C4},
                         {KBD_R2C1, KBD_R2C2, KBD_R2C3, KBD_R2C4},
                         {KBD_R3C1, KBD_R3C2, KBD_R3C3, KBD_R3C4},
                         {KBD_R4C1, KBD_R4C2, KBD_R4C3, KBD_R4C4}};
const char GPIORows[4]={KBD_R1, KBD_R2, KBD_R3, KBD_R4};
const char GPIOCols[4]={KBD_C1, KBD_C2, KBD_C3, KBD_C4};

void KBD_GPIOConfig(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure; 

  GPIO_InitStructure.GPIO_Pin = KBD_R1 | KBD_R2 | KBD_R3 | KBD_R4;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(KBD_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = KBD_C1 | KBD_C2 | KBD_C3 | KBD_C4;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
  GPIO_Init(KBD_GPIO, &GPIO_InitStructure);
}


unsigned char KBD_ReadKey(void)
{ //returns ASCII code of a pressed key
  char r, c;
  unsigned char pressedKey=0x00;

  r=0;
  while (r<4) {
    GPIO_WriteBit(KBD_GPIO, GPIORows[r], SET);            //activate row
    c=0;
    while (c<4) {
      if (GPIO_ReadInputDataBit(KBD_GPIO, GPIOCols[c])){  //check columns
        pressedKey=keyMap[r][c];                          //if column active - find key value 
      }
      c++;
    }
    GPIO_WriteBit(KBD_GPIO, GPIORows[r], RESET);          //deactivate row
    r++;
  }
  return pressedKey;                                      //return key ASCII value
}
