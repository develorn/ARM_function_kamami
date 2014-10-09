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

#include "stm32f10x.h"
#include "./../moduly/LCD1602/lcd_hd44780_lib.h"
#include <stdio.h>

//enum bool {False=0, True=1};
void RCC_Config(void);
void GPIO_Config(void);
void NVIC_Config(void);
void Opcja1(void);
void Opcja2(void);
void Opcja3(void);
void Opcja4(void);
void Opcja5(void);
void Opcja6(void);
void Opcja7(void);
void Opcja8(void);
                          
int main(void)
{
  volatile unsigned int i; 
  unsigned char pozycja=1;  
  unsigned char *menu[8] = {"Dioda 1\0","Dioda 2\0","Dioda 3\0","Dioda 4\0","Dioda 5\0","Dioda 6\0","Dioda 7\0","Dioda 8\0"};

  //konfiguracja systemu
  RCC_Config();   
  GPIO_Config();
  NVIC_Config();
  /*Tu nalezy umiescic ewentualne dalsze funkcje konfigurujace system*/
  
  GPIO_ResetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);		 
  LCD_Initialize();
  LCD_WriteCommand(HD44780_CLEAR);
  
  LCD_WriteText("> \0");
  LCD_WriteText(menu[pozycja-1]);         
  LCD_WriteText(" <\0");         
  LCD_WriteTextXY(menu[pozycja-1+1],2,1);         
  while (1) {
    if (!GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) && (pozycja<8)) {
      pozycja++;
      LCD_WriteCommand(HD44780_CLEAR);       //umieszczenie wyswietlania w tym miejscu usuwa problem "migotania" LCD
      LCD_WriteText("> \0");                 //poniewaz jest odswiwezane tylko wtedy, gdy jest zmiana pozycji, a nie bez przerwy
      LCD_WriteText(menu[pozycja-1]);         
      LCD_WriteText(" <\0");         
      if (pozycja<8){
        LCD_WriteTextXY(menu[pozycja-1+1],2,1);         
      }
    }
    if (!GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) && (pozycja>1)) {
      pozycja--;
      LCD_WriteCommand(HD44780_CLEAR);
      LCD_WriteText("> \0");         
      LCD_WriteText(menu[pozycja-1]);         
      LCD_WriteText(" <\0");         
      if (pozycja<8){
        LCD_WriteTextXY(menu[pozycja-1+1],2,1);         
      }
    }
    if (!GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5)){
      switch (pozycja){
        case 1: Opcja1(); break;
        case 2: Opcja2(); break;
        case 3: Opcja3(); break;
        case 4: Opcja4(); break;
        case 5: Opcja5(); break;
        case 6: Opcja6(); break;
        case 7: Opcja7(); break;
        case 8: Opcja8(); break;
        default: Opcja1(); break;
      }
    }
    for (i=0; i<1000000ul; i++);
  };
  return 0;
}


void RCC_Config(void)
//konfigurowanie sygnalow taktujacych
{
  ErrorStatus HSEStartUpStatus;  //zmienna opisujaca rezultat uruchomienia HSE

  RCC_DeInit();	                                         //Reset ustawien RCC
  RCC_HSEConfig(RCC_HSE_ON);                             //Wlaczenie HSE
  HSEStartUpStatus = RCC_WaitForHSEStartUp();		         //Odczekaj az HSE bedzie gotowy
  if(HSEStartUpStatus == SUCCESS)
  {
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);//
    FLASH_SetLatency(FLASH_Latency_2);                   //ustaw zwloke dla pamieci Flash; zaleznie od taktowania rdzenia
	                                                       //0:<24MHz; 1:24~48MHz; 2:>48MHz
    RCC_HCLKConfig(RCC_SYSCLK_Div1);                     //ustaw HCLK=SYSCLK
    RCC_PCLK2Config(RCC_HCLK_Div1); 					           //ustaw PCLK2=HCLK
    RCC_PCLK1Config(RCC_HCLK_Div2);						           //ustaw PCLK1=HCLK/2
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); //ustaw PLLCLK = HSE*9 czyli 8MHz * 9 = 72 MHz
    RCC_PLLCmd(ENABLE);                                  //wlacz PLL
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);  //odczekaj na poprawne uruchomienie PLL
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);           //ustaw PLL jako zrodlo sygnalu zegarowego
    while(RCC_GetSYSCLKSource() != 0x08);                //odczekaj az PLL bedzie sygnalem zegarowym systemu

	/*Tu nalezy umiescic kod zwiazany z konfiguracja sygnalow zegarowych potrzebnych w programie peryferiow*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//wlacz taktowanie portu GPIO A
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//wlacz taktowanie portu GPIO B
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);//wlacz taktowanie portu GPIO C
    	
  } else {
  } 
}

void NVIC_Config(void)
{
//Konfigurowanie kontrolera przerwan NVIC
#ifdef  VECT_TAB_RAM  
  // Jezeli tablica wektorow w RAM, to ustaw jej adres na 0x20000000
  NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
#else  // VECT_TAB_FLASH
  // W przeciwnym wypadku ustaw na 0x08000000
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
#endif
}

void GPIO_Config(void)
{
  //konfigurowanie portow GPIO
  GPIO_InitTypeDef  GPIO_InitStructure; 

  /*Tu nalezy umiescic kod zwiazny z konfiguracja poszczegolnych portow GPIO potrzebnych w programie*/
  //port A - Przyciski i joystick
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure); 
  
  //port B - diody LED
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  //port C - Joystick
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
}


   
void Opcja1(void){
  GPIO_WriteBit(GPIOB, GPIO_Pin_8, (BitAction) (1-GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_8)));
}   

void Opcja2(void){
  GPIO_WriteBit(GPIOB, GPIO_Pin_9, (BitAction) (1-GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_9)));
}   

void Opcja3(void){
  GPIO_WriteBit(GPIOB, GPIO_Pin_10, (BitAction) (1-GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_10)));
}   

void Opcja4(void){
  GPIO_WriteBit(GPIOB, GPIO_Pin_11, (BitAction) (1-GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_11)));
}   

void Opcja5(void){
  GPIO_WriteBit(GPIOB, GPIO_Pin_12, (BitAction) (1-GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_12)));
}   

void Opcja6(void){
  GPIO_WriteBit(GPIOB, GPIO_Pin_13, (BitAction) (1-GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_13)));
}   

void Opcja7(void){
  GPIO_WriteBit(GPIOB, GPIO_Pin_14, (BitAction) (1-GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_14)));
}   

void Opcja8(void){
  GPIO_WriteBit(GPIOB, GPIO_Pin_15, (BitAction) (1-GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_15)));
}   

