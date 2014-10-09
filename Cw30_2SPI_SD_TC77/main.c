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

//polaczenia do wykonania na zestawie ZL27ARM
//-------------------------------------
//JP5 PB12           <-> JP7 CS
//JP5 PB13           <-> JP7 SCK
//JP5 PB15           <-> JP7 SIO

#include "stm32f10x.h"
#include "./../Moduly/LCD1602/lcd_hd44780_lib.h"
#include "stdio.h"
#include "./../FATFs/src/ff.h"
#include "./../FATFs/src/diskio.h"

void GPIO_Config(void);
void RCC_Config(void);
void NVIC_Config(void);
void SPI_Config(void);
unsigned long int SysTick_Config_Mod(unsigned long int SysTick_CLKSource, unsigned long int Ticks);

#define STAN_PROG_OCZEKIWANIE 0
#define STAN_PROG_GOTOWOSC_ZAPISU 1
#define STAN_PROG_JEST_KARTA	2
#define STAN_PROG_BRAK_KARTY	3
#define STAN_PROG_BLAD_ZAPISU	4
#define STAN_PROG_ZAPIS_PLIKU 5
char stanProg=STAN_PROG_OCZEKIWANIE;

static FATFS g_sFatFs;
bool sekunda=FALSE;

int main(void)
{
  volatile unsigned long int i;
  int temperatura;
  unsigned char temperaturaTekst[8]={" 0,0 C\0"};
  static const unsigned char stopienSymbol[8] = {0x06,0x09,0x09,0x06,0x00,0x00,0x00,0}; //symbol stopnia

  FRESULT fresult;
  FIL plik;
  UINT bajtowZapisanych=0;
  char poprzedniStanGPIOA8=0;
  char obecnyStanGPIOA8=0;
  bool zapis=FALSE;
  unsigned long int t=0;
  char tTekst[8]={"0\0"};
   

  //konfiguracja systemu
  RCC_Config();   
  GPIO_Config(); 
  NVIC_Config();
  SPI_Config();     
  /*Tu nalezy umiescic ewentualne dalsze funkcje konfigurujace system*/
  GPIO_ResetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11);		 

  LCD_Initialize();                                       //inicjalizacja wysietlacza
  LCD_SetUserChar(1, 1, stopienSymbol);                   //Umiesc symbol stopnia pod kodem =1
  LCD_WriteCommand(HD44780_CLEAR);                        //wyczysc wyswietlacz
  LCD_WriteText("Temp.:\0");        
  temperaturaTekst[5]=1;
  LCD_WriteTextXY(temperaturaTekst,7,0); //Wstaw do tekstu znak stopnia
  

	if (SysTick_Config_Mod(SysTick_CLKSource_HCLK, 720000ul))	{
		while(1);		// W razie bledu petla nieskonczona
	}					

  obecnyStanGPIOA8=(BitAction)(1-GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8));//zanegowanie rzeczywistego stanu GPIOA8 spwoduje, ze stan obecny i stan poprzedni beda rozne -> wymusi to testowanie obecnosci karty przy pierwszym wejsciu w STAN_PROG_OCZEKIWANIE 

  #define SPI_Mode_Slave_Mask ((unsigned short int)0xFEFB)  //Maska pozwalajaca wyzerowac bity trybu pracy wprost w rejestrzez SPIx->CR1
  while (1) {
    /*Tu nalezy umiescic glowny kod programu*/
    //Czesc odpowiedzialna za pobranie temperatury z czujnika i jej wyswietlenie na LCD
    SPI2->CR1 |= SPI_Mode_Master;                           //Ustaw tryb master - wymusi to zmiane stanu NSS na niski
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET); //Czekaj na dane
    temperatura = SPI_I2S_ReceiveData(SPI2);                //Odczytaj dane
    if ((temperatura&0x04)==0){                             //Sprawdz, czy zakonczono juz pierwszy pomiar po wlaczeniu ukladu TC77
      temperatura=0;                                        //Jesli nie, ustaw temp=0,  wprzeciwnym razie wynik bedzie bledny (>500stC)
    }
    temperatura = temperatura >> 3;                         //Usun 3 LSB
    SPI2->CR1 &= SPI_Mode_Slave_Mask;                       //Ustaw tryb slave - wymusi to zmiane stanu NSS na wysoki

    temperatura = (temperatura * 625)/100;                  //1 bit temperatury odpowiada 0,0625 stopnia
    sprintf((char *)temperaturaTekst, "%2d,%d C ", temperatura / 100, (temperatura % 100)/10 );
    temperaturaTekst[4]=1;                                  //Wstaw do tekstu znak stopnia
    LCD_WriteTextXY((unsigned char *)temperaturaTekst,7,0);


    //Czesc odpowiedzialna za obsluge karty SD, w tym zapis temperatury do pliku co 1s
    poprzedniStanGPIOA8=obecnyStanGPIOA8;
    obecnyStanGPIOA8=GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8);
    switch (stanProg){
      case STAN_PROG_OCZEKIWANIE : {  
                                     if (obecnyStanGPIOA8==poprzedniStanGPIOA8){
                                       for (i=0;i<10000ul;i++); 		
                                     } else {
                                       if ((obecnyStanGPIOA8==SET)&&(poprzedniStanGPIOA8==RESET)){ 
                                         stanProg=STAN_PROG_BRAK_KARTY;
                                       } else {
                                         stanProg=STAN_PROG_JEST_KARTA;
                                       }
                                     }
                                   } break;
      case STAN_PROG_GOTOWOSC_ZAPISU : {  
                                     if (obecnyStanGPIOA8==poprzedniStanGPIOA8){ //brak zmiany stanu oznacza, ze nie wyjeto karty w czasie zapisu
                                       for (i=0;i<10000ul;i++); 
                                       		
                                       if (!GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)){     //SW0 - wlacz zapis
                                         zapis=TRUE;
                                         t=0;
                                       } 
                                       if (!GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1)){     //SW1 - wylacz zapis
                                         zapis=FALSE;
                                       } 

                                       if (zapis) {                              //trwa zapis
                                         LCD_WriteTextXY("Zapis pliku       \0",0,1);  
                                         GPIO_WriteBit(GPIOB, GPIO_Pin_11, SET);   
                                         if (sekunda) {                          //zapis kolejnych wartosci tylko w chwilach, gdy uplynela kolejna sekunda
                                           stanProg=STAN_PROG_ZAPIS_PLIKU;
                                         }
                                       } else {                                  //brak zapisu, ale karta zainicjalizowana i gotowa do zapisu
                                         LCD_WriteTextXY("Gotowy do zapisu  \0",0,1);  
                                         GPIO_WriteBit(GPIOB, GPIO_Pin_11, RESET); 
                                       }
                                     } else {                                    //nastapila zmiana stanu linii DETECT
                                       if ((obecnyStanGPIOA8==SET)&&(poprzedniStanGPIOA8==RESET)){ //wyjeto karte
                                         stanProg=STAN_PROG_BRAK_KARTY;
                                       }
                                     }
                                   } break;
      case STAN_PROG_BRAK_KARTY : {
                                     LCD_WriteTextXY("Brak karty        \0",0,1);   
                                     stanProg=STAN_PROG_OCZEKIWANIE;
                                     zapis=FALSE;                                //jesli brak karty - zapis jest anulowany
                                     GPIO_WriteBit(GPIOB, GPIO_Pin_11, RESET); 
                                   } break;
      case STAN_PROG_JEST_KARTA : {
                                     fresult = f_mount(0, &g_sFatFs);
                                     if (fresult==0) {
                                        stanProg=STAN_PROG_GOTOWOSC_ZAPISU;
                                     } else {
                                        stanProg=STAN_PROG_BRAK_KARTY;
                                     }
                                  } break;
      case STAN_PROG_BLAD_ZAPISU : {
                                     LCD_WriteTextXY("Blad zapisu   \0",0,1);  
                                     zapis=FALSE;                                //jesli byl blad - zapis jest anulowany
                                     GPIO_WriteBit(GPIOB, GPIO_Pin_11, RESET);   //brak zapisu
                                     stanProg=STAN_PROG_OCZEKIWANIE;
                                   } break;
      case STAN_PROG_ZAPIS_PLIKU : {
                                     GPIO_WriteBit(GPIOB, GPIO_Pin_10, SET);     //rozpoczeto zapis
                                     fresult = f_open (&plik, "dane.txt", FA_WRITE | FA_OPEN_ALWAYS);
                                     if (fresult) {stanProg=STAN_PROG_BLAD_ZAPISU; break;}
                                     if (t>0) {                                  //jesli czas >0, dopisujemy na koncu pliku nowa linie danych
                                       fresult = f_lseek(&plik, plik.fsize);
                                     } else {                                    //jesli czas ==0, to oznacza to, ze zapis dopiero sie zaczyna i nalezy plik "obciac" i zaczac zapis od nowa
                                       fresult = f_lseek(&plik, 0);
                                       fresult = f_truncate(&plik);
                                     }
    		                             sprintf(tTekst, "%5d  \0", t);                                                    //kolumna czasu
                                     fresult = f_write (&plik, (const void*)tTekst, 6, &bajtowZapisanych);	           
                                     if (fresult) {stanProg=STAN_PROG_BLAD_ZAPISU; break;}
                                     fresult = f_write (&plik, (const void*)temperaturaTekst, 4, &bajtowZapisanych);	 //wartosc temperatury, zapis 4 bajtow czyli tylko wartosci, bez symbolu stC
                                     if (fresult) {stanProg=STAN_PROG_BLAD_ZAPISU; break;}
                                     fresult = f_write (&plik, (const void*)"\r\n", 2, &bajtowZapisanych);	           //nowa linia  (wg. standardu Windows)
                                     if (fresult) {stanProg=STAN_PROG_BLAD_ZAPISU; break;}
                                   	 fresult = f_close (&plik);
                                     GPIO_WriteBit(GPIOB, GPIO_Pin_10, RESET);   //koniec zapisu
                                     stanProg=STAN_PROG_GOTOWOSC_ZAPISU;
                                     t+=1;                                       //czas dla nastepnej wartosci
                                   } break;

      default : stanProg=STAN_PROG_OCZEKIWANIE;
    }
    sekunda=FALSE;
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
    RCC_PCLK2Config(RCC_HCLK_Div1); 		          			 //ustaw PCLK2=HCLK
    RCC_PCLK1Config(RCC_HCLK_Div2);						           //ustaw PCLK1=HCLK/2
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); //ustaw PLLCLK = HSE*9 czyli 8MHz * 9 = 72 MHz
    RCC_PLLCmd(ENABLE);									                 //wlacz PLL
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);  //odczekaj na poprawne uruchomienie PLL
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);           //ustaw PLL jako zrodlo sygnalu zegarowego
    while(RCC_GetSYSCLKSource() != 0x08);                //odczekaj az PLL bedzie sygnalem zegarowym systemu

  /*Tu nalezy umiescic kod zwiazny z konfiguracja sygnalow zegarowych potrzebnych w programie peryferiow*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);//wlacz taktowanie portu GPIO B, C (LED, LCD)
                                                                                //GPIO A i SPI inicjalizowane w sd_stm32.c
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);	
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);	
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
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  // PA4 - nSS/CS SPI
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  //SPI - SCK, MISO, MOSI
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  //linia DETECT zlacza SD
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);


  //SPI2 : SS, SCK, MISO and MOSI 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}



unsigned long int  SysTick_Config_Mod(unsigned long int SysTick_CLKSource, unsigned long int Ticks) 
{ 
  //inicjalizacja licznika SysTick
  //zastepuje funkcje z bibliotek STM w zwiazku z bledem w funcji SysTick_Config
  unsigned long int Settings;

  assert_param(IS_SYSTICK_CLK_SOURCE(SysTick_CLKSource));
  
  if (Ticks > SysTick_LOAD_RELOAD_Msk)  return (1);             //Kontrola, czy wartosc poczatkowa nie przekracza max
                                                                
  SysTick->LOAD = (Ticks & SysTick_LOAD_RELOAD_Msk) - 1;        //Ustaw wartosc poczatkowa licznika 
  NVIC_SetPriority (SysTick_IRQn, 0);                           //Ustaw priorytet przerwania
  SysTick->VAL  = 0;                                            //Ustaw wartosc aktualna licznika
  Settings=SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;  //Ustaw flagi wlaczaenia SysTick IRQ  i samego licznika
  if (SysTick_CLKSource == SysTick_CLKSource_HCLK){             //Wybierz flage ustawien zrodla sygnalu zegarowego
    Settings |= SysTick_CLKSource_HCLK;
  } else {
    Settings &= SysTick_CLKSource_HCLK_Div8;
  }
  SysTick->CTRL = Settings;                                     //Zapisz ustawienia do rejestru sterujacego SysTick (i wlacz licznik)
  return (0);                                                  
}


void SPI_Config(void)
{
  //konfigurowanie interfejsu SPI
  SPI_InitTypeDef   SPI_InitStructure;

  //SPI1 - karta SD
  SPI_InitStructure.SPI_Direction =  SPI_Direction_2Lines_FullDuplex;//transmisja z wykorzystaniem jednej linii, transmisja jednokierunkowa
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                     //tryb pracy SPI
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b ;                //8-bit ramka danych 
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;                       //stan sygnalu taktujacego przy braku transmisji - wysoki
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;                      //aktywne zbocze sygnalu taktujacego - 2-gie zbocze
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                         //programowa obsluga linii NSS (CS)
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;//prescaler szybkosci tansmisji  72MHz/4=18MHz
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                //pierwszy bit w danych najbardziej znaczacy
  SPI_InitStructure.SPI_CRCPolynomial = 7;                          //stopien wielomianu do obliczania sumy CRC
  SPI_Init(SPI1, &SPI_InitStructure);                               //inicjalizacja SPI

  SPI_Cmd(SPI1, ENABLE);  	// Wlacz SPI1

  //SPI2 - termometr TC77
  SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Rx;         //transmisja z wykorzystaniem jednej linii, transmisja jednokierunkowa
  SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;                      //tryb pracy SPI
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;                //16-bit ramka danych 
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;                        //stan sygnalu taktujacego przy braku transmisji - niski
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;                      //aktywne zbocze sygnalu taktujacego - 1-sze zbocze
  SPI_InitStructure.SPI_NSS = SPI_NSS_Hard;                         //sprzetowa obsluga linii NSS (CS)
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;//prescaler szybkosci tansmisji  36MHz/256=140.625kHz
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                //pierwszy bit w danych najbardziej znaczacy
  SPI_InitStructure.SPI_CRCPolynomial = 7;                          //stopien wielomianu do obliczania sumy CRC
  SPI_Init(SPI2, &SPI_InitStructure);                               //inicjalizacja SPI
  SPI_SSOutputCmd(SPI2, ENABLE);                                    

  SPI_Cmd(SPI2, ENABLE);  	// Wlacz SPI2
}
