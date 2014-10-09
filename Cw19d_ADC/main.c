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

#include "stm32f10x.h"
#include ".\..\Moduly\LCD1602\lcd_hd44780_lib.h"
#include "stdio.h"

void GPIO_Config(void);
void RCC_Config(void);
void NVIC_Config(void);
void DMA_Config(void);
void ADC_Config(void);
            
unsigned short int buforADC[2]={0};

int main(void)
{
  volatile unsigned long int i;
  unsigned long int napiecie, temperatura;
  unsigned char Tekst[7] = {"0\0"};
  const unsigned char stopienSymbol[8] = {0x06,0x09,0x09,0x06,0x00,0x00,0x00,0}; //symbol stopnia

  //konfiguracja systemu
  RCC_Config();   
  GPIO_Config(); 
  NVIC_Config();  
  DMA_Config();
  ADC_Config();
  /*Tu nalezy umiescic ewentualne dalsze funkcje konfigurujace system*/
  GPIO_ResetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);		 

  LCD_Initialize();                                                           //inicjalizacja wysietlacza
  LCD_SetUserChar(1, 1, stopienSymbol);                                       //umiesc symbol stopnia pod kodem =1
  LCD_WriteCommand(HD44780_CLEAR);                                            //wyczysc wyswietlacz
  LCD_WriteText("Pot. : 0,000 V\0");            
  LCD_WriteTextXY("Temp.:  0 C\0",0,1);            

	ADC_SoftwareStartConvCmd(ADC1, ENABLE);                                     //rozpocznij przetwarzanie AC
  while (1) {
    /*Tu nalezy umiescic glowny kod programu*/
    napiecie = buforADC[0] * 8059/10000;                                     //przelicz wartosc wyrazona jako calkowita, 12-bit na rzeczywista
 	  sprintf((char *)Tekst, "%d,%03d V\0", napiecie / 1000, napiecie % 1000); //Dzielenie calkowite wyznacza wartosc w V,  dzielenie modulo - czeasc po przecinku
    LCD_WriteTextXY(Tekst, 7, 0);         

    temperatura = (1430 - buforADC[1] * 8059/10000)*10/43+25;                //przelicz wartosc wyrazona jako calkowita, 12-bit na rzeczywista, wartosci typowe wg. Datasheet, 5.3.18, str. 75.
 	  sprintf((char *)Tekst, "%2d C\0", temperatura); 
    Tekst[2]=1;                                                              //Wstaw do tekstu znak stopnia
    LCD_WriteTextXY(Tekst, 7, 1);        

		// Aktualizacja wyswietlacza co okolo 0.25s
    GPIO_WriteBit(GPIOB, GPIO_Pin_15, (BitAction)(1-GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_15)));
    GPIO_WriteBit(GPIOB, GPIO_Pin_14, RESET);
    for (i=0;i<1500000ul;i++); 			   
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
    //RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); //ustaw PLLCLK = HSE*9 czyli 8MHz * 9 = 72 MHz
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_7); //ustaw PLLCLK = HSE*9 czyli 8MHz * 7 = 56 MHz - konieczne dla ADC
    RCC_PLLCmd(ENABLE);									 //wlacz PLL
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);  //odczekaj na poprawne uruchomienie PLL
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);           //ustaw PLL jako zrodlo sygnalu zegarowego
    while(RCC_GetSYSCLKSource() != 0x08);                //odczekaj az PLL bedzie sygnalem zegarowym systemu

	/*Tu nalezy umiescic kod zwiazany z konfiguracja sygnalow zegarowych potrzebnych w programie peryferiow*/
    RCC_ADCCLKConfig(RCC_PCLK2_Div4);                    // ADCCLK = PCLK2/4 = 56 MHz /4 = 14 MHz (maksymalna mozliwa)
        
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//wlacz taktowanie portu GPIO A
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//wlacz taktowanie portu GPIO B
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);//wlacz taktowanie portu GPIO C
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); //wlacz taktowanie ADC1
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);   //wlacz taktowanie DMA
	
  } else {
  } 
}


void NVIC_Config(void)
{
  //Konfigurowanie kontrolera przerwan NVIC

  NVIC_InitTypeDef NVIC_InitStructure;

#ifdef  VECT_TAB_RAM  
  // Jezeli tablica wektorow w RAM, to ustaw jej adres na 0x20000000
  NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
#else  // VECT_TAB_FLASH
  // W przeciwnym wypadku ustaw na 0x08000000
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
#endif

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

  //Wlacz przerwanie od ADC
  NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
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

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4;   
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;                                   //wejscie analogowe
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
}


void DMA_Config(void)
{
  //konfigurowanie DMA

  #define ADC1_DR_Address 0x4001244C;                                             //adres rejestru ADC1->DR
  
  DMA_InitTypeDef DMA_InitStructure;

  DMA_DeInit(DMA1_Channel1);                                                      //Usun ewentualna poprzednia konfiguracje DMA
   
  DMA_InitStructure.DMA_PeripheralBaseAddr = (unsigned long int)ADC1_DR_Address;  //Adres docelowy transferu
  DMA_InitStructure.DMA_MemoryBaseAddr = (unsigned long int)&buforADC;            //Adres poczatku bloku do przeslania
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                              //Kierunek transferu
  DMA_InitStructure.DMA_BufferSize = 2;                                           //Liczba elementow do przeslania (dlugosc bufora)
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;                //Wylaczenie automatycznego zwiekszania adresu po stronie ADC
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                         //Wlaczenie automatycznego zwiekszania adresu po stronie pamieci (bufora)
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;     //Rozmiar pojedynczych przesylanych danych po stronie ADC (HalfWord = 16bit)
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;             //Rozmiar pojedynczych przesylanych danych po stronie pamieci (bufora)
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                                 //Tryb dzialania kontrolera DMA - powtarzanie cykliczne
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;                             //Priorytet DMA - wysoki
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                                    //Wylaczenie obslugi transferu z pamieci do pamieci
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);                                    //Zapis konfiguracji

  //Wlacz DMA, kanal 1
  DMA_Cmd(DMA1_Channel1, ENABLE);  
}


void ADC_Config(void)
{
  //konfigurowanie przetwornika AC
  ADC_InitTypeDef ADC_InitStructure;
	  
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                  //Jeden przetwornik, praca niezalezna
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;                        //Pomiar dwoch kanalow, konieczne skanowanie kanalow 
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                  //Pomiar w trybie ciaglym
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//Brak wyzwalania zewnetrznego
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;            	//Wyrownanie danych do prawej - 12 mlodszych bitow znaczacych
	ADC_InitStructure.ADC_NbrOfChannel = 2; 	                          //Liczba uzywanych kanalow =2
	ADC_Init(ADC1, &ADC_InitStructure);                                 //Incjalizacja przetwornika

	ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 1, ADC_SampleTime_1Cycles5); 
                                                                      //Kanal 14 - GPIOC4 - potencjometr na plytce ZL27ARM
                                                                      //Grupa podstawowa, czas probkowania 1,5+12,5=14 cykli = 1us (calkowity czas przetwarania)
  ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 2, ADC_SampleTime_239Cycles5); 
                                                                      //Kanal 16 - wewnetrzny czujnik temp. procesora
                                                                      //Grupa podstawowa, czas probkowania 239,5+12,5=252 cykli = 18us (calkowity czas przetwarzania)
                                                                      //239,5 => 17.1us = czas zalecany w dokumentacji MUC dla miernika temperatury

	ADC_TempSensorVrefintCmd(ENABLE);                                   //Wlaczenie czujnika temperatury procesora
  ADC_DMACmd(ADC1,ENABLE);                                            //Wlaczenie DMA dla ADC
  
  ADC_AnalogWatchdogThresholdsConfig(ADC1, 0x0C1E, 0x04D8);           //prog gorny 2.5V/3.3V*4095=3102=0xC1E, prog dolny 1V=1241=0x4D9
  ADC_AnalogWatchdogCmd(ADC1, ADC_AnalogWatchdog_SingleRegEnable);
  ADC_AnalogWatchdogSingleChannelConfig(ADC1, ADC_Channel_14);

  ADC_ITConfig(ADC1, ADC_IT_AWD, ENABLE);                             //Wlaczenie przerwania od watchdoga analogowego
    
  ADC_Cmd(ADC1, ENABLE);	                                            //Wlacz ADC1

	ADC_ResetCalibration(ADC1);	                                        //Reset rejestrow kalibracyjnych ADC1
	while(ADC_GetResetCalibrationStatus(ADC1));	                        //Odczekanie na wykonanie resetu
	ADC_StartCalibration(ADC1);	                                        //Kalibracja ADC1
	while(ADC_GetCalibrationStatus(ADC1));    	                        //Odczekanie na zakonczenie kalibracji ADC1
}
