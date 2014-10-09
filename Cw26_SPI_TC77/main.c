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
//JP6 PA4           <-> JP7 CS
//JP6 PA5           <-> JP7 SCK
//JP6 PA7           <-> JP7 SIO

#include "stm32f10x.h"
#include "./../Moduly/LCD1602/lcd_hd44780_lib.h"
#include "stdio.h"

void GPIO_Config(void);
void RCC_Config(void);
void NVIC_Config(void);
void SPI_Config(void);

int main(void)
{
  volatile unsigned long int i;
  int temperatura;
  unsigned char temperaturaTekst[8]={" 0,0 C\0"};
	static const unsigned char stopienSymbol[8] = {0x06,0x09,0x09,0x06,0x00,0x00,0x00,0}; //symbol stopnia

  //konfiguracja systemu
  RCC_Config();   
  GPIO_Config(); 
  NVIC_Config();  
  SPI_Config(); 
  /*Tu nalezy umiescic ewentualne dalsze funkcje konfigurujace system*/
  GPIO_ResetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);		 

  LCD_Initialize();                                         //Inicjalizacja wysietlacza
  LCD_SetUserChar(1, 1, stopienSymbol);                     //Umiesc symbol stopnia pod kodem =1
  LCD_WriteCommand(HD44780_CLEAR);                          //Wyczysc wyswietlacz
  LCD_WriteText("Temp.:\0");        
  temperaturaTekst[5]=1;
  LCD_WriteTextXY(temperaturaTekst,7,0);                    //Wstaw do tekstu znak stopnia
  
  #define SPI_Mode_Slave_Mask ((unsigned short int)0xFEFB)  //Maska pozwalajaca wyzerowac bity trybu pracy wprost w rejestrzez SPIx->CR1
  while (1) {
    /*Tu nalezy umiescic glowny kod programu*/
    SPI1->CR1 |= SPI_Mode_Master;                           //Ustaw tryb master - wymusi to zmiane stanu NSS na niski
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET); //Czekaj na dane
    temperatura = SPI_I2S_ReceiveData(SPI1);                //Odczytaj dane
    if ((temperatura&0x04)==0){                             //Sprawdz, czy zakonczono juz pierwszy pomiar po wlaczeniu ukladu TC77
      temperatura=0;                                        //Jesli nie, ustaw temp=0,  wprzeciwnym razie wynik bedzie bledny (>500stC)
    }
    temperatura = temperatura >> 3;                         //Usun 3 LSB
    SPI1->CR1 &= SPI_Mode_Slave_Mask;                       //Ustaw tryb slave - wymusi to zmiane stanu NSS na wysoki

		temperatura = (temperatura * 625)/100;                  //1 bit temperatury odpowiada 0,0625 stopnia
		sprintf((char *)temperaturaTekst, "%2d,%d C ", temperatura / 100, (temperatura % 100)/10 );
    temperaturaTekst[4]=1;                                  //Wstaw do tekstu znak stopnia
    LCD_WriteTextXY(temperaturaTekst,7,0);  
    for (i=0;i<4500000ul;i++); 
    GPIO_WriteBit(GPIOB, GPIO_Pin_15, (BitAction)(1-GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_15)));
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
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE); //wlacz taktowanie SPI        
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); //wlacz taktowanie funkcji alternatywnych GPIO

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

  // SPI1 : SCK, MISO and MOSI 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

}


void SPI_Config(void)
{
  //konfigurowanie interfejsu SPI
  SPI_InitTypeDef   SPI_InitStructure;

  SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Rx;         //transmisja z wykorzystaniem jednej linii, transmisja jednokierunkowa
  SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;                      //tryb pracy SPI
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;                //16-bit ramka danych 
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;                        //stan sygnalu taktujacego przy braku transmisji - niski
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;                      //aktywne zbocze sygnalu taktujacego - 1-sze zbocze
  SPI_InitStructure.SPI_NSS = SPI_NSS_Hard;                         //sprzetowa obsluga linii NSS (CS)
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;//prescaler szybkosci tansmisji  36MHz/256=140.625kHz
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                //pierwszy bit w danych najbardziej znaczacy
  SPI_InitStructure.SPI_CRCPolynomial = 7;                          //stopien wielomianu do obliczania sumy CRC
  SPI_Init(SPI1, &SPI_InitStructure);                               //inicjalizacja SPI
  SPI_SSOutputCmd(SPI1, ENABLE);                                    

  SPI_Cmd(SPI1, ENABLE);  	// Wlacz SPI1
}
