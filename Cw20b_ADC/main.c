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


//Napiecie zewnetrzne, mierzone przetwornikiem ADC2 podawac na linie GPIOC5 (z³acza JP3, linia PC5 zestawu ZL27ARM)

//Do dzialania programu przydatny jest modul KAmod LED8
//polaczenia do wykonania pomiaedzy modulem a zestawem ZL27ARM
//Zl27ARM         |  LED8
//JP10=GPIOE      |
//-------------------------------------
//JP10 PE0       <-> Con1 D0
//JP10 PE1       <-> Con1 D1
//JP10 PE2       <-> Con1 D2
//JP10 PE3       <-> Con1 D3
//JP10 PE4       <-> Con1 D4
//JP10 PE5       <-> Con1 D5
//JP10 PE6       <-> Con1 D6
//JP10 PE7       <-> Con1 D7
//JP10 GND       <-> Con1 GND
//JP10 +3,3V     <-> Con1 +5V

#include "stm32f10x.h"
#include ".\..\Moduly\LCD1602\lcd_hd44780_lib.h"
#include "stdio.h"

void GPIO_Config(void);
void RCC_Config(void);
void NVIC_Config(void);
void ADC_Config(void);

int main(void)
{
  volatile unsigned long int i;
  unsigned long int wartoscADC1 = 0;
  unsigned long int wartoscADC2 = 0;
  unsigned long int wartoscADC1V = 0;
  unsigned long int wartoscADC2V = 0;
  unsigned char wartoscADC1VTekst[7] = {"0\0"};
  unsigned char wartoscADC2VTekst[7] = {"0\0"};
  unsigned int stanGPIOB, stanGPIOE;
  unsigned long int liczbaLED = 0;
  unsigned int wlaczoneLED[10]={0,0x1,0x3,0x7,0xF,0x1F,0x3F,0x7F,0xFF,0xFF};

  //konfiguracja systemu
  RCC_Config();   
  GPIO_Config(); 
  NVIC_Config();  
  ADC_Config();
  /*Tu nalezy umiescic ewentualne dalsze funkcje konfigurujace system*/
  GPIO_ResetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);		 
  GPIO_ResetBits(GPIOE, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);		 

  LCD_Initialize();                                          //inicjalizacja wysietlacza
  LCD_WriteCommand(HD44780_CLEAR);                           //wyczysc wyswietlacz
  LCD_WriteTextXY((char *)"ADC1:  0,000 V\0",0,0);            
  LCD_WriteTextXY((char *)"ADC2:  0,000 V\0",0,1);            

	ADC_SoftwareStartConvCmd(ADC1, ENABLE);                    //rozpocznij przetwaranie AC
  while (1) {
    /*Tu nalezy umiescic glowny kod programu*/
    while (!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));           //odczekaj na zakonczenie konwersji
    //wartoscADC1V = ADC_GetConversionValue(ADC1) * 0.8059;  //przelicz wartosc wyrazona jako calkowita, 12-bit na rzeczywista
    
    wartoscADC1  = ADC_GetConversionValue(ADC1);
    wartoscADC1V = wartoscADC1 * 8059/10000;                 //przelicz wartosc wyrazona jako calkowita, 12-bit na rzeczywista
 	  sprintf((char *)wartoscADC1VTekst, "%d,%03d V\0", wartoscADC1V / 1000, wartoscADC1V % 1000); //Dzielenie calkowite wyznacza wartosc w V,  dzielenie modulo - czeasc po przecinku
    LCD_WriteTextXY(wartoscADC1VTekst,7,0);         

    liczbaLED=(wartoscADC1*9)>>12;                           //wyznacz liczbe wlaczonych LED
    stanGPIOB=GPIO_ReadOutputData(GPIOB) & 0x00FF;           //pobierz aktualny stan calego portu B i wyzeruj bity odpowiedzialne za LEDy
    GPIO_Write(GPIOB, wlaczoneLED[liczbaLED]<<8 | stanGPIOB);//stan diod przesun o 8 w lewo poniewaz diody w GPIOA podlaczone sa do pinow 8-15, polacz z reszta stanu portu i zapisz nowy stan portu


    while (!ADC_GetFlagStatus(ADC2,ADC_FLAG_EOC));           //odczekaj na zakonczenie konwersji
    //wartoscADC2V = ADC_GetConversionValue(ADC2) * 0.8059;  //przelicz wartosc wyrazona jako calkowita, 12-bit na rzeczywista
    wartoscADC2  = ADC_GetConversionValue(ADC2);             
    wartoscADC2V = wartoscADC2 * 8059/10000;                 //przelicz wartosc wyrazona jako calkowita, 12-bit na rzeczywista
    sprintf((char *)wartoscADC2VTekst, "%d,%03d V\0", wartoscADC2V / 1000, wartoscADC2V % 1000); //Dzielenie calkowite wyznacza wartosc w V,  dzielenie modulo - czeasc po przecinku
    LCD_WriteTextXY(wartoscADC2VTekst,7,1);         

    liczbaLED=(wartoscADC2*9)>>12;                           //wyznacz liczbe wlaczonych LED
    stanGPIOE=GPIO_ReadOutputData(GPIOE) & 0xFF00;           //pobierz aktualny stan calego portu B i wyzeruj bity odpowiedzialne za LEDy
    GPIO_Write(GPIOE, wlaczoneLED[liczbaLED] | stanGPIOE);   //stan diod polacz z reszta stanu portu i zapisz nowy stan portu, przesuniecie nie potrzebne - diody podlaczone do pinow 0-8 GPIOE

		// Aktualizacja wyswietlacza co okolo 0.25s
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
  HSEStartUpStatus = RCC_WaitForHSEStartUp();	           //Odczekaj az HSE bedzie gotowy
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
    RCC_PLLCmd(ENABLE);									                 //wlacz PLL
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);  //odczekaj na poprawne uruchomienie PLL
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);           //ustaw PLL jako zrodlo sygnalu zegarowego
    while(RCC_GetSYSCLKSource() != 0x08);                //odczekaj az PLL bedzie sygnalem zegarowym systemu

	/*Tu nalezy umiescic kod zwiazany z konfiguracja sygnalow zegarowych potrzebnych w programie peryferiow*/
    RCC_ADCCLKConfig(RCC_PCLK2_Div4);                    // ADCCLK = PCLK2/4 = 56 MHz /4 = 14 MHz (maksymalna mozliwa)
        
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//wlacz taktowanie portu GPIO A
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//wlacz taktowanie portu GPIO B
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);//wlacz taktowanie portu GPIO C
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);//wlacz taktowanie portu GPIO E
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); //wlacz taktowanie ADC1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE); //wlacz taktowanie ADC2
	
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

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4 | GPIO_Pin_5;   
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  //wejscie analogowe
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
}



void ADC_Config(void)
{
  //konfigurowanie przetwornika AC
  ADC_InitTypeDef ADC_InitStructure;
	  
  ADC_InitStructure.ADC_Mode = ADC_Mode_RegSimult;                    //Dwa przetworniki, praca jednoczesna
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;                       //Pomiar jednego kanalu, skanowanie kanalow nie potrzebne
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                  //Pomiar w trybie ciaglym
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //Brak wyzwalania zewnetrznego
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;              //Wyrownanie danych do prawej - 12 mlodszych bitow znaczacych
  ADC_InitStructure.ADC_NbrOfChannel = 1;                             //Liczba uzywanych kanalow =1
  ADC_Init(ADC1, &ADC_InitStructure);                                 //Incjalizacja przetwornika

  ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 1, ADC_SampleTime_1Cycles5); 
                                                                      //Kanal 14 - GPIOC4 - potencjometr na plytce ZL27ARM
                                                                      //Grupa podstawowa, czas probkowania 1,5+12,5=14 cykli ->  f probkowania = 14MHz/14cykli = 1MHz. 


  ADC_InitStructure.ADC_Mode = ADC_Mode_RegSimult;                    //Dwa przetworniki, praca jednoczesna
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;                       //Pomiar jednego kanalu, skanowanie kanalow nie potrzebne
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                  //Pomiar w trybie ciaglym
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //Brak wyzwalania zewnetrznego
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;              //Wyrownanie danych do prawej - 12 mlodszych bitow znaczacych
  ADC_InitStructure.ADC_NbrOfChannel = 1;                             //Liczba uzywanych kanalow =1
  ADC_Init(ADC2, &ADC_InitStructure);                                 //Incjalizacja przetwornika

  ADC_RegularChannelConfig(ADC2, ADC_Channel_15, 1, ADC_SampleTime_1Cycles5); 
                                                                      //Kanal 15 - GPIOC5 - wejscie sygnalu zewnetrznego
                                                                      //Grupa podstawowa, czas probkowania 1,5+12,5=14 cykli ->  f probkowania = 14MHz/14cykli = 1MHz. 
  ADC_ExternalTrigConvCmd(ADC2, ENABLE);                              //Wlaczenie wyzwalania ADC2 przez ADC1


  ADC_Cmd(ADC1, ENABLE);                                              //Wlacz ADC1

  ADC_ResetCalibration(ADC1);                                         //Reset rejestrow kalibracyjnych ADC1
  while(ADC_GetResetCalibrationStatus(ADC1));                         //Odczkeanie na wykonanie resetu
  ADC_StartCalibration(ADC1);                                         //Kalibracja ADC1
  while(ADC_GetCalibrationStatus(ADC1));                              //Odczekanie na zakonczenie kalibracji ADC1
 
  ADC_Cmd(ADC2, ENABLE);                                              //Wlacz ADC1
  
  ADC_ResetCalibration(ADC2);                                         //Reset rejestrow kalibracyjnych ADC1
  while(ADC_GetResetCalibrationStatus(ADC2));                         //Odczkeanie na wykonanie resetu
  ADC_StartCalibration(ADC2);                                         //Kalibracja ADC1
  while(ADC_GetCalibrationStatus(ADC2));                              //Odczekanie na zakonczenie kalibracji ADC1
}
