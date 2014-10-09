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

//Do dzialania programu potrzebne jest 
//wykonanie polaczenia w zestawie ZL27ARM
//Zl27ARM            |  ZL27ARM
//JP5=GPIOB          |  JP6=GPIOA
//-------------------------------------
//JP5 PB8            <-> JP6 PA8

#include "stm32f10x.h"
#include "./../Moduly/LCD1602/lcd_hd44780_lib.h"
#include <stdio.h>

void GPIO_Config(void);
void RCC_Config(void);
void NVIC_Config(void);
void TIM_Config(void);

extern unsigned short int wspWyp;
extern unsigned short int czestotliwosc;

int main(void)
{
  volatile unsigned long int i;
  unsigned char Tekst[5] = {"0\0"};

  //konfiguracja systemu
  RCC_Config();   
  GPIO_Config(); 
  NVIC_Config();  
  /*Tu nalezy umiescic ewentualne dalsze funkcje konfigurujace system*/
  TIM_Config();
  //GPIO_ResetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);

  LCD_Initialize();                      //inicjalizacja wysietlacza
  LCD_WriteCommand(HD44780_CLEAR);       //wyczysc wyœwietlacz
  LCD_WriteText("Parametry PWM:\0");        //wyswietl tekst pierwszej linii

  while (1) {
    /*Tu nalezy umiescic glowny kod programu*/
    if (!GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)){
        if (TIM4->CCR3<=(0xFFFF-250)){
          TIM4->CCR3 = TIM4->CCR3+250;     
        }
    }
    if (!GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1)){
        if (TIM4->CCR3>=250){
          TIM4->CCR3 = TIM4->CCR3-250;
        }
    }	

    sprintf((char *)Tekst,"f=%5i\0",czestotliwosc);
    LCD_WriteTextXY(Tekst,0,1);

    sprintf((char *)Tekst,"wyp=%3i\0",wspWyp);
    LCD_WriteTextXY(Tekst,9,1);

    for (i=0;i<400000;i++);
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
  if(HSEStartUpStatus == SUCCESS) {
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);//
    FLASH_SetLatency(FLASH_Latency_2);                   //ustaw zwloke dla pamieci Flash; zaleznie od taktowania rdzenia
	                                                       //0:<24MHz; 1:24~48MHz; 2:>48MHz
    RCC_HCLKConfig(RCC_SYSCLK_Div1);                     //ustaw HCLK=SYSCLK
    RCC_PCLK2Config(RCC_HCLK_Div1); 					           //ustaw PCLK2=HCLK
    RCC_PCLK1Config(RCC_HCLK_Div2);					             //ustaw PCLK1=HCLK/2
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); //ustaw PLLCLK = HSE*9 czyli 8MHz * 9 = 72 MHz
    RCC_PLLCmd(ENABLE);									                 //wlacz PLL
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);  //odczekaj na poprawne uruchomienie PLL
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);           //ustaw PLL jako zrodlo sygnalu zegarowego
    while(RCC_GetSYSCLKSource() != 0x08);                //odczekaj az PLL bedzie sygnalem zegarowym systemu

	/*Tu nalezy umiescic kod zwiazany z konfiguracja sygnalow zegarowych potrzebnych w programie peryferiow*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//wlacz taktowanie portu GPIO B
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//wlacz taktowanie portu GPIO A
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);//wlacz taktowanie portu GPIO A
   // RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);//wlacz taktowanie ukladu funkcji alternatywnych
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //wlacz taktowanie licznika TIM4
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE); //wlacz taktowanie licznika TIM1
	
  } else {
  } 
}

void NVIC_Config(void) {
//Konfigurowanie kontrolera przerwan NVIC

  NVIC_InitTypeDef NVIC_InitStructure;

#ifdef  VECT_TAB_RAM  
  // Jezeli tablica wektorow w RAM, to ustaw jej adres na 0x20000000
  NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
#else  // VECT_TAB_FLASH
  // W przeciwnym wypadku ustaw na 0x08000000
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
#endif

  //Wybor modelu grupowania przerwan
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1); 

  // konfiguracja przerwania od CC TIM1
  NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void GPIO_Config(void) {
  //konfigurowanie portow GPIO
  GPIO_InitTypeDef  GPIO_InitStructure; 

  /*Tu nalezy umiescic kod zwiazany z konfiguracja poszczegolnych portow GPIO potrzebnych w programie*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;  //joystick
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;  	         // PA8 - wejscie kanalu 1 timera TIM1
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

}


void TIM_Config(void) {
//Konfigurowanie licznikow TIM

  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  TIM_ICInitTypeDef  TIM_ICInitStructure;

	// Konfiguracja TIM4
  // Ustawienia ukladu podstawy czasu
  TIM_TimeBaseStructure.TIM_Prescaler = 0;  //bez prescelaera (prescaler=1)
  TIM_TimeBaseStructure.TIM_Period = 49999ul;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	// Kanaly 1 i 2 nie uzywane
	// Konfiguracja kanalu 3 - uzywamy kanalu 3 poniewaz jego wyjscie jest na GPIOB8 - gdzie jest LED1
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 50ul;        //wypelnienie poczatkowe = 50000/100=0.2%
  TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_High;
	TIM_OC3Init(TIM4, &TIM_OCInitStructure);

	TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);

	// Konfiguracja kanalu 4 - uzywamy kanalu 4 poniewaz jego wyjscie jest na GPIOB9 - gdzie jest LED2
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 25000ul;      //wypelnienie = 50000/25000=50%
  TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_High;
	TIM_OC4Init(TIM4, &TIM_OCInitStructure);

	TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);

	TIM_ARRPreloadConfig(TIM4, ENABLE);
  
  
  // Konfiguracja TIM1
  // Ustawienia ukladu podstawy czasu
  TIM_TimeBaseStructure.TIM_Prescaler = 8-1;  //prescaler=8
  TIM_TimeBaseStructure.TIM_Period = 65535ul;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

  // konfiguracja kanalu IC1 do pomiaru PWM, IC2 skonfigurowany bedzie automatycznie
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;               //wybor pierwszego konfigurowanego kanalu
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;          //dzielnik sygnalu wejsciowego
  TIM_ICInitStructure.TIM_ICFilter = 0;                          //dlugosc filtra sygnalu wejsciowego
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;    //aktywne zbocze sygnalu wejsciowego 
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;//wybor polaczen miedzy wejsciami licznika, a kanalami IC
	TIM_PWMIConfig(TIM1, &TIM_ICInitStructure);

  TIM_SelectInputTrigger(TIM1, TIM_TS_TI1FP1);                   //Wybor zrodla wyzwalania dla TIM1
  TIM_SelectSlaveMode(TIM1, TIM_SlaveMode_Reset);                //Wybranie trybu pracy slave-reset
  TIM_SelectMasterSlaveMode(TIM1, TIM_MasterSlaveMode_Enable);   //Wlaczenie trybu slave-reset
  
  
  // Wlaczenie timera
  TIM_Cmd(TIM4, ENABLE);  
  TIM_Cmd(TIM1, ENABLE);  
  //wlaczenie przrwania od CC1 dla TIM1
  TIM_ITConfig(TIM1, TIM_IT_CC1, ENABLE);  
}
