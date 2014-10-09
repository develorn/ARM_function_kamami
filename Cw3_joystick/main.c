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

enum bool {False=0, True=1};
void RCC_Config(void);
void GPIO_Config(void);
void NVIC_Config(void);

int main(void)
{
   
  unsigned short int stanGPIOB, stanGPIOA;
  //konfiguracja systemu
  RCC_Config();   
  GPIO_Config();
  NVIC_Config();
  /*Tu nalezy umiescic ewentualne dalsze funkcje konfigurujace system*/
  
  GPIO_ResetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);		 
  
  while (1) {
    /*Tu nalezy umiescic glowny kod programu*/
    
    /*  wersja rozwiazania z "recznym" przypisaniem okcji poszczegolnych przyciskow */
    if (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)){
      GPIO_ResetBits(GPIOB, GPIO_Pin_8);
    } else {
      GPIO_SetBits(GPIOB, GPIO_Pin_8);
    }
    if (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1)){
      GPIO_ResetBits(GPIOB, GPIO_Pin_9);
    } else {
      GPIO_SetBits(GPIOB, GPIO_Pin_9);
    }
    if (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_2)){
      GPIO_ResetBits(GPIOB, GPIO_Pin_10);
    } else {
      GPIO_SetBits(GPIOB, GPIO_Pin_10);
    }
    if (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3)){
      GPIO_ResetBits(GPIOB, GPIO_Pin_11);
    } else {
      GPIO_SetBits(GPIOB, GPIO_Pin_11);
    }    

    /* wersja rozwiazania z wyliczaniem numeru diody na podstawie stanu przycisków (z przesuwaniem stanu przyciskow)
    stanGPIOA=0xFFFF-GPIO_ReadInputData(GPIOA);  //pobierz stan calego portu A, poniewaz przyciki zwieraja do masy, 
                                                 //a wejscie jest Floating, to przy braku przycisniecia na wejsciu jest stan 1 
                                                 //stad, po odjeciu od FFFF otrzymujemy jedynke tam, gdzie przycisk byl wcisniety
    stanGPIOA=stanGPIOA & 0x000F;                //wyzerowanie bitow niezwiazanych z przyciskami
    stanGPIOA=stanGPIOA<<8;                      //przesuniecie stanu przyscikow w lewo o 8 poniewaz diody zaczynaja sie w porcie B na pozycji 8

    stanGPIOB=GPIO_ReadOutputData(GPIOB);		     //pobierz aktualny stan calego portu B
    stanGPIOB=stanGPIOB&0x10FF;					         //wyzeruj bity odpowiedzialne za LEDy, z wyjatkiem 5-tego, zwiazanego z Enterem w joysticku
                                                 //ustaw pozycje zapalonych LEDow i wstaw pzostala czesc stanu portu odczytana wczesniej, tak, by nie psuc ustawien innych ledow i mlodszej czesci portu
    GPIO_Write(GPIOB, stanGPIOA | stanGPIOB);    //zapisz nowy stan portu

    if (GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)){//osobno - obsluga przycisku "Enter" joysticka
      GPIO_ResetBits(GPIOB, GPIO_Pin_12);
    } else {
      GPIO_SetBits(GPIOB, GPIO_Pin_12);
    }  */
  };
  return 0;
}


void RCC_Config(void)
//konfigurowanie sygnalow taktujacych
{
  ErrorStatus HSEStartUpStatus;                          //zmienna opisujaca rezultat uruchomienia HSE

  RCC_DeInit();                                          //Reset ustawien RCC
  RCC_HSEConfig(RCC_HSE_ON);                             //Wlaczenie HSE
  HSEStartUpStatus = RCC_WaitForHSEStartUp();            //Odczekaj az HSE bedzie gotowy
  if(HSEStartUpStatus == SUCCESS)
  {
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);//
    FLASH_SetLatency(FLASH_Latency_2);                   //ustaw zwloke dla pamieci Flash; zaleznie od taktowania rdzenia
                                                         //0:<24MHz; 1:24~48MHz; 2:>48MHz
    RCC_HCLKConfig(RCC_SYSCLK_Div1);                     //ustaw HCLK=SYSCLK
    RCC_PCLK2Config(RCC_HCLK_Div1);                      //ustaw PCLK2=HCLK
    RCC_PCLK1Config(RCC_HCLK_Div2);                      //ustaw PCLK1=HCLK/2
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

  /*Tu nalezy umiescic kod zwiazany z konfiguracja poszczegolnych portow GPIO potrzebnych w programie*/
  //port B - diody LED
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  //port A - Przyciski
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  //port C - Joystick
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
}
