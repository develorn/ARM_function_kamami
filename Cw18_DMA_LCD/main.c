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

//Do poprawnego dzialania programu konieczne jest wykonanie w zestawie ZL27ARM zmian:
//pozycja zworki LCD_PWM = uC
//polaczyc linie PA8 (JP6, port GPIOA) z linia PWM na zlaczu JP7

#include "stm32f10x.h"
#include ".\..\Moduly\LCD1602\lcd_hd44780_lib.h"
//#include "math.h"
#include "stdlib.h"

void GPIO_Config(void);
void RCC_Config(void);
void NVIC_Config(void);
void DMA_Config(void);
void TIM_Config(void);

unsigned short int buforPWM_1[200]={0};
unsigned short int buforPWM_2[200]={0};

int main(void)
{

  unsigned long int i;

  //konfiguracja systemu
  RCC_Config();   
  GPIO_Config(); 
  NVIC_Config();  
  DMA_Config();
  TIM_Config();
  /*Tu nalezy umiescic ewentualne dalsze funkcje konfigurujace system*/
  GPIO_ResetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);		 

  LCD_Initialize();                           //inicjalizacja wysietlacza
  LCD_WriteCommand(HD44780_CLEAR);            //wyczysc wyswietlacz
  LCD_WriteText("    DMA demo\0"); 
  LCD_WriteTextXY("PWM 1 - liniowo\0",0,1); 


  for (i=0; i<100; i++)                       //wypelnienie tablicy przebiegiem liniowymw gore i w dol
    buforPWM_1[i]=i+1;
  for (i=100; i<=199; i++)
    buforPWM_1[i]=199-i;
  
  for (i=0; i<=199; i++) 
    buforPWM_2[i]=rand()% 100 + 1 ;          //wypelnienie tablicy przebiegiem losowym
    //buforPWM_2[i]=50*(sin(i*3.14/50)+1);   //50*(sin(i*pi/50)+1); - 2 okresy sinusa o wartosciach 0~100


  while (1) {
    /*Tu nalezy umiescic glowny kod programu*/
		for (i=0; i<20000; i++);
    if (!GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)){
      DMA1_Channel5->CMAR = (unsigned long int)buforPWM_1;
      LCD_WriteTextXY("PWM 1 - liniowo\0",0,1); 
    }      
    if (!GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1)){
      DMA1_Channel5->CMAR = (unsigned long int)buforPWM_2;
      LCD_WriteTextXY("PWM 2 - losowo \0",0,1); 
    }      
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
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);   //wlacz taktowanie DMA
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE); //wlacz taktowanie licznika 1
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); //wlacz taktowanie ukladu funkcji alternatywnych
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

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;         //funkcja alternatywna - wyjscie PWM
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void TIM_Config(void)
{
  //konfigurowanie licznikow
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;

  // Konfiguracja Timera 1
  TIM_TimeBaseStructure.TIM_Prescaler = 100-1;          //FCKL = 72MHz/100  = 720kHz
  TIM_TimeBaseStructure.TIM_Period = 100;               //FPWM = 720kHz/100 = 7.2kHz
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 250;	  // Licznik powtarzania - co 250 okresow TIM1 nastapi wywolanie DMA

  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
  
  // Konfiguracja kanalu 1
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 50;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OC1Init(TIM1, &TIM_OCInitStructure);

  TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);       //wlaczenie buforowania ustawien  

  TIM_DMACmd(TIM1, TIM_DMA_Update, ENABLE);               //zdarzenie update bedzie generowac zadania DMA
  
  //Wlaczenie Timera 1 
  TIM_Cmd(TIM1, ENABLE);

  //Wlaczenie wyjscia PWM
  TIM_CtrlPWMOutputs(TIM1, ENABLE);  
}

void DMA_Config(void)
{
  //konfigurowanie ukladu DMA
  #define TIM1_CCR1_Address 0x40012C34;  //adres rejestru TIM1->CCR1
  
  DMA_InitTypeDef DMA_InitStructure;

  DMA_DeInit(DMA1_Channel5);                                                        //Usun ewentualna poprzednia konfiguracje DMA
  DMA_InitStructure.DMA_PeripheralBaseAddr = (unsigned long int)TIM1_CCR1_Address;  //Adres docelowy transferu
  DMA_InitStructure.DMA_MemoryBaseAddr = (unsigned long int)&buforPWM_1;            //Adres poczatku bloku do przeslania
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                                //Kierunek transferu
  DMA_InitStructure.DMA_BufferSize = 200;                                           //Liczba elementow do przeslania (dlugosc bufora)
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;                  //Wylaczenie automatycznego zwiekszania adresu po stronie licznika
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                           //Wlaczenie automatycznego zwiekszania adresu po stronie pamieci (bufora)
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;       //Rozmiar pojedynczych przesylanych danych po stronie licznika (HalfWord = 16bit)
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;               //Rozmiar pojedynczych przesylanych danych po stronie pamieci (bufora)
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                                   //Tryb dzialana kontrolera DMA - powtarzanie cykliczne
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;                               //Priorytet DMA - wysoki
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                                      //Wylaczenie obslugi transferu z pamieci do pamieci
  
  DMA_Init(DMA1_Channel5, &DMA_InitStructure);                                      //Zapis konfiguracji

  //Wlacz DMA, kanal 5
  DMA_Cmd(DMA1_Channel5, ENABLE);  
}

