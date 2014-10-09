//*****************************************************************************
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

//Do dzialania programu potrzebny jest modul KAmod BAR wersja SPI
//polaczenia do wykonania pomiedzy modulem a zestawem ZL27ARM
//Zl27ARM            |  BAR
//JP6=GPIOA          |
//-------------------------------------
//JP6 PA4           <-> Con2 CS
//JP6 PA5           <-> Con2 SCL/CLK
//JP6 PA6           <-> Con2 SDO
//JP6 PA7           <-> Con2 SDI
//JP6 GND           <-> Con2 GND
//JP6 +3,3V (GPIOA) <-> Con2 +V

#include "stm32f10x.h"
#include ".\..\Moduly\LCD1602\lcd_hd44780_lib.h"
#include ".\..\Moduly\BAR\bar_spi_lib.h"
#include "stdio.h"
#include "stdlib.h"


void GPIO_Config(void);
void RCC_Config(void);
void NVIC_Config(void);

int main(void)
{
  volatile unsigned long int i;
  unsigned short int Temperatura, Cisnienie;
  sCoeffs Wspolczynniki;
  long int TempObliczona, CisObliczone;
  unsigned char Tekst[10] = {"\0"};
  static const unsigned char stopienSymbol[8] = {0x06,0x09,0x09,0x06,0x00,0x00,0x00,0}; //symbol stopnia


  //konfiguracja systemu
  RCC_Config();   
  GPIO_Config(); 
  NVIC_Config();  
  /*Tu nalezy umiescic ewentualne dalsze funkcje konfigurujace system*/
  GPIO_ResetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);		 
  MPL115_SPI_HwConfig();

  LCD_Initialize();                           //inicjalizacja wyswietlacza
  LCD_SetUserChar(1, 1, stopienSymbol);       //Umiesc symbol stopnia pod kodem =1
  LCD_WriteCommand(HD44780_CLEAR);            //wyczysc wyswietlacz
  LCD_WriteTextXY("Temp.:     \0",0,0);        
  LCD_WriteTextXY("Cisn.:     \0",0,1);  

  MPL115_ReadCoeffs(&Wspolczynniki);          //Pobierz z ukladu MPL115 wspolczynniki wielomianu do obliczania cisnienia
 
  while (1) {
    /*Tu nalezy umiescic glowny kod programu*/
    MPL115_ReadPressTemp(&Temperatura, &Cisnienie);

    Temperatura-=38;                          //korekta temperatury do ralnych wartosci, uwaga! niezgodnie z nota aplikacyjna

    TempObliczona=(2500 -((Temperatura - 472)*10000 / 535))/10;    
    MPL115_PressCalc(&CisObliczone, &Temperatura, &Cisnienie, &Wspolczynniki);

    sprintf((char *)Tekst, "%2d,%d C\0", TempObliczona/10, abs(TempObliczona%10));     
    //sprintf((char *)Tekst, "%2d C\0", Temperatura);     
    Tekst[4]=1;                               //Wstaw do tekstu znak stopnia
    LCD_WriteTextXY(Tekst,7,0);  

    sprintf((char *)Tekst, "%4d hPa\0", CisObliczone);     
    LCD_WriteTextXY(Tekst,7,1);         

    GPIO_WriteBit(GPIOB, GPIO_Pin_15, (BitAction)(1-GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_15)));
    for (i=0;i<5000000ul;i++); 					   
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

	/*Tu nalezy umiescic kod zwiazany z konfiguracja sygnalow zegarowych potrzebnych w programie peryferiow*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//wlacz taktowanie portu GPIO A
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//wlacz taktowanie portu GPIO B
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);//wlacz taktowanie portu GPIO C
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE); //wlacz taktowanie ukladu SPI 
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

  /*Tu nalezy umiescic kod zwiazany z konfiguracja poszczegolnych portow GPIO potrzebnych w programie*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}
