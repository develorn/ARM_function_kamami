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

//Do dzialania programu potrzebny jest modul KAmod MEMS1
//polaczenia do wykonania pomiedzy modulem, a zestawem ZL27ARM
//Zl27ARM            |  MEMS1
//JP10=GPIOE         |
//-------------------------------------
//JP10 PE0           <-> Con1 LC
//JP10 PE1           <-> Con1 PC
//JP10 PE2           <-> Con1 SGN
//JP10 PE3           <-> Con1 PD
//JP10 GND           <-> Con1 GND
//JP10 +3,3V (GPIOE) <-> Con1 +V
//                      zworka PD w pozycji 1 (uklad zawsze wlaczony), lub zdjeta (wlaczanie sterowane linia PD)
//                      zworka LED w pozycji On

#include "stm32f10x.h"
#include ".\..\Moduly\LCD1602\lcd_hd44780_lib.h"
#include ".\..\Moduly\MEMS1\mems1_lib.h"
#include "stdio.h"

void GPIO_Config(void);
void RCC_Config(void);
void NVIC_Config(void);

int main(void)
{
  volatile unsigned long int i;
  char orientacja1, orientacja2;
  unsigned char Tekst[3] = {"\0"};

  //konfiguracja systemu
  RCC_Config();   
  GPIO_Config(); 
  NVIC_Config();  
  /*Tu nalezy umiescic ewentualne dalsze funkcje konfigurujace system*/
  GPIO_ResetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);		 
  MEMS1_GPIOConfig();
  LCD_Initialize();                              //inicjalizacja wyswietlacza
  LCD_WriteCommand(HD44780_CLEAR);               //wyczysc wyswietlacz
  LCD_WriteText((unsigned char *)"Orientacja: \0");  
  orientacja1=255;                               //pozycja niedopuszczalna wymusi odswiezenie stanu po pierszym odczycie pozycji

  //MEMS1_Power(MEMS1_POWER_DOWN);               //test trybu power-down
  while (1) {
    /*Tu nalezy umiescic glowny kod programu*/
    orientacja2=MEMS1_ReadPos();
    if (orientacja2!=orientacja1) {              //wysietlanie nowej orientacji tylko gdy sie zmienila
      sprintf((char*)Tekst,"%d\0",orientacja2);
      LCD_WriteTextXY(Tekst,12,0);         
      //LCD_WriteTextXY((unsigned char *)"                \0",0,1);     //16 spacji    
      switch (orientacja2){
        case 0 : {LCD_WriteTextXY("Front up       \0",0,1);} break;
        case 4 : {LCD_WriteTextXY("Front down     \0",0,1);} break;
        case 1 : {LCD_WriteTextXY("Landscape left \0",0,1);} break;
        case 5 : {LCD_WriteTextXY("Landscape right\0",0,1);} break;
        case 2 : {LCD_WriteTextXY("Portrait up    \0",0,1);} break;
        case 6 : {LCD_WriteTextXY("Portrait down  \0",0,1);} break;
        case 3 : {LCD_WriteTextXY("45 deg         \0",0,1);} break;
        default : {LCD_WriteTextXY("??            \0",0,1);} break;
      }
    }
    orientacja1=orientacja2;
    GPIO_WriteBit(GPIOB, GPIO_Pin_15, (BitAction)(1-GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_15)));
    for (i=0;i<750000ul;i++); 					   
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
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//wlacz taktowanie portu GPIO B
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);//wlacz taktowanie portu GPIO C	
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);//wlacz taktowanie portu GPIO C	
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
}
