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


//Do dzialania programu potrzebny jest modul KAmod TFT2
//polaczenia do wykonania pomiedzy modulem a zestawem ZL27ARM
//Zl27ARM            |  TFT2
//JP6=GPIOA          |
//-------------------------------------   
//JP6 PA4           <-> Con1 SCE
//JP6 PA5           <-> Con1 CLK
//JP6 PA7           <-> Con1 DIN
//JP6 PA8           <-> Con1 RES
//JP6 GND           <-> Con1 GND
//JP6 +3,3V (GPIOB) <-> Con1 +V

#include "stm32f10x.h"
#include ".\..\Moduly\TFT2\tft_pcf8833_lib.h"
#include ".\..\Moduly\TFT2\logo.h"
#include ".\..\Moduly\TFT2\font1.h"
#include ".\..\Moduly\TFT2\font2.h"

void GPIO_Config(void);
void RCC_Config(void);
void NVIC_Config(void);
void SPI_Config(void);
void Opcja_Info(void);
void Opcja_5Kolorow(void);
void Opcja_Kolory(void);
void Opcja_Negatyw(void);
void Opcja_Rysunek(void);

int main(void)
{
  volatile unsigned long int i;
  signed char pozycja=1;
  signed char aktualizacja=0;
  const unsigned char *menu[6] = {"Info\0", "5 kolor\x8Ew\0", "Paleta kolor\x8Ew\0", "Negatyw\0", "Rysunek\0"};

  //konfiguracja systemu
  RCC_Config();   
  GPIO_Config(); 
  NVIC_Config();  
  /*Tu nalezy umiescic ewentualne dalsze funkcje konfigurujace system*/
  GPIO_ResetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);		 

  TFTN_HwConfig();
  TFTN_Init();

  while (1) {
    /*Tu nalezy umiescic glowny kod programu*/
    GPIO_WriteBit(GPIOB, GPIO_Pin_15, (BitAction)(1-GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_15)));

    if (!GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) && (pozycja>1)){
      aktualizacja=pozycja;
      pozycja--;
    }
    if (!GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) && (pozycja<5)){
      aktualizacja=pozycja;
      pozycja++;
    }

    //ENTER joysticka
    if (!GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5)){
      switch (pozycja){
        case 1: Opcja_Info(); break;
        case 2: Opcja_5Kolorow(); break;
        case 3: Opcja_Kolory(); break;
        case 4: Opcja_Negatyw(); break;
        case 5: Opcja_Rysunek(); break;
      }
      aktualizacja=0;
    }                          



    if (aktualizacja>=0){
      if (aktualizacja==0) {
        TFTN_Clear(0x0E0);
        TFTN_FillRect(  0, 0, 131, 18, 0x070);
        TFTN_WriteXY("MENU\0",45,2,0xFFF,0x070, font2);
      }
      for (i=1;i<=5;i++){
        if ((i==aktualizacja)||(aktualizacja==0)) {        
          TFTN_FillRect(  0, i*18+8, 131, i*18+8+18, 0x0E0);
          TFTN_WriteXY(menu[i-1],5,i*18+10,0xFFF,0x0E0, font2);
        }
        if (i==pozycja) {
          TFTN_FillRect(  0, i*18+8, 131, i*18+8+18, 0x070);
          TFTN_WriteXY(menu[i-1],5,i*18+10,0xFFF,0x070, font2);
        }
      }
      aktualizacja=-1;
    }

    for (i=0;i<1000000ul;i++); 	
  };
  return 0;
}


void RCC_Config(void)
//konfigurowanie sygnalow taktujacych
{
  ErrorStatus HSEStartUpStatus;  //zmienna opisujaca rezultat uruchomienia HSE

  RCC_DeInit();	                                         //Reset ustawien RCC
  RCC_HSEConfig(RCC_HSE_ON);                             //Wlaczenie HSE
  HSEStartUpStatus = RCC_WaitForHSEStartUp();            //Odczekaj az HSE bedzie gotowy
  if(HSEStartUpStatus == SUCCESS)
  {
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);//
    FLASH_SetLatency(FLASH_Latency_2);                   //ustaw zwloke dla pamieci Flash; zaleznie od taktowania rdzenia
	                                                       //0:<24MHz; 1:24~48MHz; 2:>48MHz
    RCC_HCLKConfig(RCC_SYSCLK_Div1);                     //ustaw HCLK=SYSCLK
    RCC_PCLK2Config(RCC_HCLK_Div1); 					           //ustaw PCLK2=HCLK
    RCC_PCLK1Config(RCC_HCLK_Div2);						           //ustaw PCLK1=HCLK/2
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); //ustaw PLLCLK = HSE*9 czyli 8MHz * 9 = 72 MHz
    RCC_PLLCmd(ENABLE);									                 //wlacz PLL
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);  //odczekaj na poprawne uruchomienie PLL
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);           //ustaw PLL jako zrodlo sygnalu zegarowego
    while(RCC_GetSYSCLKSource() != 0x08);                //odczekaj az PLL bedzie sygnalem zegarowym systemu

	/*Tu nalezy umiescic kod zwiazny z konfiguracja sygnalow zegarowych potrzebnych w programie peryferiow*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//wlacz taktowanie portu GPIO A - transmisja danych, jostick
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//wlacz taktowanie portu GPIO B - diody
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);//wlacz taktowanie portu GPIO C - joystick
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
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);    

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;  //Joystick
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);  

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;                                         //Joystick - Enter
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOC, &GPIO_InitStructure);  
}


void Opcja_Info(void){
  volatile unsigned long int i;

  TFTN_Clear(0x00F);
  TFTN_WriteXY("    DEMO     \0"   ,  13, 10,0xFFF,0x00F, font2);
  TFTN_WriteXY(" wy\x8Fwietlacza\0",  10, 26,0xFFF,0x00F, font2);
  TFTN_WriteXY("  z telefonu \0"   ,  10, 42,0xFFF,0x00F, font2);
  TFTN_WriteXY("  NOKIA 6610 \0"   ,  10, 58,0xFFF,0x00F, font2);
  TFTN_WriteXY("    modu\x8C    \0",  13, 74,0xFFF,0x00F, font2);
  TFTN_WriteXY("  KAmod TFT2 \0"   ,  10, 90,0xFFF,0x00F, font2);
  TFTN_WriteXY("ekran informacyjny\0",10,110,0xFFF,0x00F, font1);
  for (i=0;i<35000000ul;i++); 
}


void Opcja_5Kolorow(void){
  volatile unsigned long int i;
  TFTN_FillRect(  0, 0,  28, 131, 0x000);
  TFTN_FillRect( 29, 0,  54, 131, 0xF00);
  TFTN_FillRect( 55, 0,  80, 131, 0x0F0);
  TFTN_FillRect( 81, 0, 106, 131, 0x00F);
  TFTN_FillRect(107, 0, 131, 131, 0xFFF);
  for (i=0;i<25000000ul;i++); 
}


void Opcja_Kolory(void){
  volatile unsigned long int i;
  unsigned int color, x, y;
  TFTN_Clear(0x000);
  TFTN_FillRect(  0, 0, 131, 18, 0xF00);
  TFTN_WriteXY("4096 KOLOR\x85W\0", 18, 2, 0xFFF, 0xF00, font2);

  for (color=0;color<=0xFFF;color++){
    x=(color*2)%128;
    y=(color*2)/128+20;
    TFTN_FillRect(x,y,x+1,y,color);
  }

  for (i=0;i<=0xF;i++){
    color=(i<<8)+(0<<4)+0;
    TFTN_FillRect(1+i*8,  90, 1+i*8+7,  97, color);
    color=(0<<8)+(i<<4)+0;
    TFTN_FillRect(1+i*8,  98, 1+i*8+7, 105, color);
    color=(0<<8)+(0<<4)+i;
    TFTN_FillRect(1+i*8, 106, 1+i*8+7, 113, color);
    color=(i<<8)+(i<<4)+i;
    TFTN_FillRect(1+i*8, 114, 1+i*8+7, 121, color);
  }
  for (i=0;i<100000000ul;i++); 
}


void Opcja_Negatyw(void){
  static bool negatyw=FALSE;

  if (negatyw) {
    TFTN_SendCmd(INVOFF);  //wlacz tryb normalny
    negatyw=FALSE;
  } else {
    TFTN_SendCmd(INVON);  //wlacz negatyw
    negatyw=TRUE;
  } 
}


void Opcja_Rysunek(void){
  volatile unsigned long int i;

  TFTN_Clear(0x000);

  TFTN_FillRect(  0, 0, 131, 18, 0xF00);
  TFTN_WriteXY("RYSUNEK\0"   ,38, 2,0xFFF,0xF00, font2);

  TFTN_FillCircle(65, 50, 20, 0xF07);
  TFTN_Circle(44, 50, 20, 0xFE3);
  TFTN_Circle(86, 50, 20, 0xFE3);

  TFTN_FillRect(  90, 75, 110, 105, 0x0E0);
  TFTN_Line(  90, 75, 110, 105, 0x050);
  TFTN_Line(  90, 105, 110, 75, 0x050);

  TFTN_WriteBMP(logo, 5, 80, 50, 48);
  TFTN_WriteXY("STM32F103\0" ,45, 120,0x05F,0x000, font1);
  for (i=0;i<100000000ul;i++); 
}



