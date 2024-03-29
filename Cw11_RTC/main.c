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
#include ".\..\Moduly\LCD1602\lcd_hd44780_lib.h"

void GPIO_Config(void);
void RCC_Config(void);
void NVIC_Config(void);
void RTC_Config(void);

int main(void)
{     
  //konfiguracja systemu
  RCC_Config();   
  GPIO_Config(); 
  NVIC_Config();  
  RTC_Config();
  /*Tu nalezy umiescic ewentualne dalsze funkcje konfigurujace system*/
  GPIO_ResetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
  LCD_Initialize();                           //inicjalizacja wysietlacza
  LCD_WriteCommand(HD44780_CLEAR);            //wyczysc wyświetlacz
  LCD_WriteText("Licznik RTC:\0");            //wyswietl tekst pierwszej linii
  //LCD_WriteTextXY((char *)"0\0",15,1);        //wyswietl poczatkowy tekst drugiej linii
  LCD_WriteTextXY("0:00:00\0",9,1);           //wyswietl poczatkowy tekst drugiej linii

  while (1) {
    /*Tu nalezy umiescic glowny kod programu*/

    //obsluga LCD w funkcji obslugi przerwania RTC_IRQ

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
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
    FLASH_SetLatency(FLASH_Latency_2);                   //ustaw zwloke dla pamieci Flash; zaleznie od taktowania rdzenia
	                                                       //0:<24MHz; 1:24~48MHz; 2:>48MHz
    RCC_HCLKConfig(RCC_SYSCLK_Div1);                     //ustaw HCLK=SYSCLK
    RCC_PCLK2Config(RCC_HCLK_Div1);                      //ustaw PCLK2=HCLK
    RCC_PCLK1Config(RCC_HCLK_Div2);	                     //ustaw PCLK1=HCLK/2
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); //ustaw PLLCLK = HSE*9 czyli 8MHz * 9 = 72 MHz
    RCC_PLLCmd(ENABLE);	                                 //wlacz PLL
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);  //odczekaj na poprawne uruchomienie PLL
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);           //ustaw PLL jako zrodlo sygnalu zegarowego
    while(RCC_GetSYSCLKSource() != 0x08);                //odczekaj az PLL bedzie sygnalem zegarowym systemu

  /*Tu nalezy umiescic kod zwiazany z konfiguracja sygnalow zegarowych potrzebnych w programie peryferiow*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//wlacz taktowanie portu GPIO B
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);//wlacz taktowanie portu GPIO B
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	
  } else {
  } 
}

void NVIC_Config(void)
{
  	NVIC_InitTypeDef NVIC_InitStructure;

//Konfigurowanie kontrolera przerwan NVIC
#ifdef  VECT_TAB_RAM  
  // Jezeli tablica wektorow w RAM, to ustaw jej adres na 0x20000000
  NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
#else  // VECT_TAB_FLASH
  // W przeciwnym wypadku ustaw na 0x08000000
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
#endif
  	
  // Ustalenie modelu przerwan
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

  // Wlacz przerwanie od RTC
  NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
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


void RTC_Config(void)
{
  //konfigurowanie RTC

  PWR_BackupAccessCmd(ENABLE);             //wlaczenie rejestrow domeny Backup
  RCC_LSEConfig(RCC_LSE_ON);
  while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);  //LSE bedzie zrodlem sygnalu zegarowego dla RTC
  RCC_RTCCLKCmd(ENABLE);                   //Wlaczenie taktowania RTC
  RTC_WaitForSynchro();                    //Oczekiwanie na synchronizacje
  RTC_WaitForLastTask();                   //Oczekiwanie az rejestry RTC zostana zapisane
  RTC_SetPrescaler(32767);                 //Okres RTC bedzie wynosil 1s  RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1)
  RTC_WaitForLastTask();
  RTC_ITConfig(RTC_IT_SEC, ENABLE);        // Wlacz przerwanie "sekundnika" RTC 
  RTC_WaitForLastTask();
  RCC_ClearFlag();
}
