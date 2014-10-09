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
#include ".\..\Moduly\TFT2\tft_pcf8833_lib.h"
#include ".\..\Moduly\TFT2\font1.h"
#include ".\..\STM32F10x_DSP_Lib_V2.0.0\Libraries\STM32F10x_DSP_Lib\inc\stm32_dsp.h"
#include <math.h>
#include <stdio.h>

void GPIO_Config(void);
void RCC_Config(void);
void NVIC_Config(void);
void ADC_Config(void);
void TIM_Config(void);
void DMA_Config(void);

void WidmoAmp(long int buforFFT[], long int buforFFTAmp[], long n);

#define N 256                      //liczba prazkow widma
long int buforADC[2*N]={0};        //bufor zmierzonych probek sygnalu (2 polowki, w danej chwili FFT liczone tylko dla polowy danych)
long int buforFFT[N]={0};          //wyznaczone zespolone wartosci widma 
long int buforFFTAmp[N]={0};       //wyznaczone prazki widma amplitudowego
unsigned char Transfer=0;          //sygnalizacja, ktora polowa buforaADC ma byc przetwarzana 0-dane nie gotowe, 1-pierwsza, 2-druga polowa


int main(void)
{
  volatile unsigned long int i,j;
  unsigned char Tekst[7] = {"0\0"};

  //konfiguracja systemu
  RCC_Config();   
  GPIO_Config(); 
  NVIC_Config();  
  DMA_Config();
  ADC_Config();
  TIM_Config();
  /*Tu nalezy umiescic ewentualne dalsze funkcje konfigurujace system*/
  GPIO_ResetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);		 

  TFTN_HwConfig();
  TFTN_Init();
  TFTN_Clear(0xEEE);
  TFTN_FillRect(  0, 0, 131, 14, 0x222);
  TFTN_WriteXY("Widmo sygna\x8Cu\0",26,4,0xFFF,0x222, font1);

  ADC_SoftwareStartConvCmd(ADC1, ENABLE);    //wykonaj jeden pomiar poczatkowy, zeby zainicjowac automatyczne wyzwalanie dalszych pomiarow
  ADC_ExternalTrigConvCmd(ADC1, ENABLE);     //wlacz triger dla drugiego pomiaru, ktory nastapi po 1/1000 sekundy

  while (1) {
    /*Tu nalezy umiescic glowny kod programu*/
    if (Transfer>0){
      cr4_fft_256_stm32((void *)buforFFT, (void *)(buforADC+(Transfer-1)*N), N);  //wyznaczanie widma sygnalu
      WidmoAmp(buforFFT, buforFFTAmp, N);
      Transfer=0;

      TFTN_WriteXY("65 pr\x89zk\x8Ew widma\0", 17, 16, 0x000, 0xEEE, font1 );           
      for (i=0; i<65; i++){
        sprintf((char *)Tekst, "%4d\0", buforFFTAmp[i]); 
        TFTN_WriteXY(Tekst, 4+(i/13)*25, 25+(i-13*(i/13))*8, 0xFFF, 0x222, font1 );         
      }
    }
    //GPIO_WriteBit(GPIOB, GPIO_Pin_15, (BitAction) ((1-GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_15))));
  };
  return 0;
}


void WidmoAmp(long int buforFFT[], long int buforFFTAmp[], long n) {
  //Wyznaczanie widma amplitudowego sygnalu
  signed long int lX, lY;
  unsigned int i;
  float X, Y, Mag;

  for (i=0; i < n; i++) {      //przeliczenie wartosci zespolonych na rzeczywiste - amplitude
    lX= (buforFFT[i]<<16)>>16; // sine_cosine --> cos 
    lY= (buforFFT[i] >> 16);   // sine_cosine --> sin     
    X = 64*((float)lX)/32768;
    Y = 64*((float)lY)/32768;
    Mag = sqrt(X*X+ Y*Y)/n;
    buforFFTAmp[i] = (uint32_t)(Mag*65536);
  }
  buforFFTAmp[0] = buforFFTAmp[0];
  buforFFTAmp[n/2] = buforFFTAmp[n/2];
  for (i=1; i < n/2; i++) {      //sumowanie prazkow dla czestotliowsci dodatniej i ujemnej
    buforFFTAmp[i] = buforFFTAmp[i] + buforFFTAmp[n-i];
    //buforFFTAmp[n-i] = 0x0;      //zerowanie niepotrzebnej czesci widma wyznaczonego dla czestotliwosci ujemnych
  }
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
    RCC_PLLCmd(ENABLE);									                 //wlacz PLL
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);  //odczekaj na poprawne uruchomienie PLL
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);           //ustaw PLL jako zrodlo sygnalu zegarowego
    while(RCC_GetSYSCLKSource() != 0x08);                //odczekaj az PLL bedzie sygnalem zegarowym systemu

	/*Tu nalezy umiescic kod zwiazny z konfiguracja sygnalow zegarowych potrzebnych w programie peryferiow*/
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);                    // ADCCLK = PCLK2/6 = 72 MHz /6 = 12 MHz

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//wlacz taktowanie portu GPIO A - wejscie analogowe + transmisja danych do wyswietlacza
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//wlacz taktowanie portu GPIO B
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); //wlacz taktowanie ADC1	
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);   //wlacz taktowanie DMA
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE); //Wlacz taktowanie TIM1
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

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

  //Wlacz przerwanie od DMA1
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  //Wlacz przerwanie od TIM1_CC
  NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
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

  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1;   
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;//wejscie analogowe
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}


void DMA_Config(void)
{
  //konfigurowanie DMA

  #define ADC1_DR_Address 0x4001244C;  //adres rejestru ADC1->DR
  
  DMA_InitTypeDef DMA_InitStructure;

  //Usun ewentualna poprzednia konfiguracje DMA
  DMA_DeInit(DMA1_Channel1); 
  //Adres docelowy transferu
  DMA_InitStructure.DMA_PeripheralBaseAddr = (unsigned long int)ADC1_DR_Address;
  //Adres poczatku bloku do przeslania
  DMA_InitStructure.DMA_MemoryBaseAddr = (unsigned long int)&buforADC; 
  //Kierunek transferu
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  //Liczba elementow do przeslania (dlugosc bufora)
  DMA_InitStructure.DMA_BufferSize = 2*N; 
  //Wylaczenie automatycznego zwiekszania adresu po stronie przetwornika
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  //Wlaczenie automatycznego zwiekszania adresu po stronie pamieci (bufora)
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  //Rozmiar pojedynczych przesylanych danych po stronie licznika (HalfWord = 16bit)
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  //Rozmiar pojedynczych przesylanych danych po stronie pamieci (bufora)
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;               
  //Tryb dzialana kontrolera DMA - powtarzanie cykliczne
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  //Priorytet DMA - wysoki
  DMA_InitStructure.DMA_Priority = DMA_Priority_High; 
  //Wylaczenie obslugi transferu z pamieci do pamieci
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  //Zais konfiguracji
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);

  DMA_ITConfig(DMA1_Channel1, DMA_IT_TC | DMA_IT_HT, ENABLE);

  //Wlacz DMA, kanal 1
  DMA_Cmd(DMA1_Channel1, ENABLE);  

}

void ADC_Config(void)
{
  //konfigurowanie przetwornika AC
  ADC_InitTypeDef ADC_InitStructure;
	  
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                  //Jeden przetwornik, praca niezalezna
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;                       //Pomiar jednego kanalu, skanowanie kanalow nie potrzebne
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                 //Pomiar w trybie ciaglym wylaczony
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;	//Wyzwalanie zewnetrzne przez kanal 1 licznika TIM1
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;            	//Wyrownanie danych do prawej - 12 mlodszych bitow znaczacych
  ADC_InitStructure.ADC_NbrOfChannel = 1; 	                          //Liczba uzywanych kanalow =1
  ADC_Init(ADC1, &ADC_InitStructure);                                 //Incjalizacja przetwornika

  ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_1Cycles5); 
                                                                     //Grupa podstawowa, czas probkowania 1,5+12,5=14 cykli = 1,167us (calkowity czas przetwarzania)

  ADC_DMACmd(ADC1,ENABLE);                                           //Wlaczenie DMA dla ADC
  ADC_Cmd(ADC1, ENABLE);				                                     //Wlacz ADC1

  ADC_ResetCalibration(ADC1);	                                       //Reset rejestrow kalibracyjnych ADC1
  while(ADC_GetResetCalibrationStatus(ADC1));	                       //Odczkeanie na wykonanie resetu
  ADC_StartCalibration(ADC1);	                                       //Kalibracja ADC1
  while(ADC_GetCalibrationStatus(ADC1));    	                       //Odczekanie na zakonczenie kalibracji ADC1
}


void TIM_Config(void) {
  //Konfiguracja timerow

  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;

  //Konfiguracja licznika 1
  //Ustawienia taktowania i trybu pracy licznika 1

  //taktowanie licznka fclk = 72MHz/720 = 100kHz
  TIM_TimeBaseStructure.TIM_Prescaler = 720-1;
  //okres przepelnienia licznika = 100 taktow -> 100kHz/100 = 1kHz
  TIM_TimeBaseStructure.TIM_Period = 100; 
  //dzielnik zegara dla ukladu generacji dead-time i filtra
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  //licznik powtorzen
  TIM_TimeBaseStructure.TIM_RepetitionCounter=0;
  //tryb pracy licznika
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);  //Inicjalizacja licznika

  //Konfiguracja kanalu 1
  //tryb pracy kanalu
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing; 
  //wlaczenie generowania sygnalu na wyjsciu licznika
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  //100 taktow
  TIM_OCInitStructure.TIM_Pulse =100;
  //polaryzacja wyjscia
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; 
  //Inicjalizacja kanalu 1 licznika TIM1
  TIM_OC1Init(TIM1, &TIM_OCInitStructure); 

  //Zapis ustawien
  //Wlaczenie rejestru preload - ewentualne zmiany konfiguracji wchodza w zycie dopiero przy nastepnym update
  TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);

  //Wlaczenie przerwania od licznika
  //wlaczenie przerwania od przepelnienia / aktualizacji wartosci Period TIM1
  TIM_ITConfig(TIM1, TIM_IT_CC1 , ENABLE); 

  //Wlaczenie timera
  TIM_Cmd(TIM1, ENABLE);
}
