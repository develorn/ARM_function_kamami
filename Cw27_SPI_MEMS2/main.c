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

//Do dzialania programu potrzebny jest modul KAmod MEMS2
//polaczenia do wykonania pomiedzy modulem a zestawem ZL27ARM
//Zl27ARM            |  MEMS2
//JP6=GPIOB          |
//-------------------------------------
//JP6 PA4           <-> Con1 SS
//JP6 PA5           <-> Con1 SCK
//JP6 PA6           <-> Con1 MISO
//JP6 PA7           <-> Con1 MOSI
//JP6 GND           <-> Con1 GND
//JP6 +3,3V (GPIOB) <-> Con1 +V

#include "stm32f10x.h"
#include ".\..\Moduly\LCD1602\lcd_hd44780_lib.h"
#include ".\..\Moduly\MEMS2\mems2_spi_lib.h"
#include "stdio.h"
#include "stdlib.h"

void GPIO_Config(void);
void RCC_Config(void);
void NVIC_Config(void);

int main(void)
{
  volatile unsigned long int i;
	signed char x, y, z;
  char click;
  unsigned char Tekst[7] = {"\0"};

  //konfiguracja systemu
  RCC_Config();   
  GPIO_Config(); 
  NVIC_Config();  
  /*Tu nalezy umiescic ewentualne dalsze funkcje konfigurujace system*/
  GPIO_ResetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);		 
  LIS35_SPI_HwConfig();
  if (LIS35_SPI_Init()==0){
    GPIO_WriteBit(GPIOB, GPIO_Pin_8, SET);       //akceleromatr zainicjalizowany 
  }

	LIS35_WriteRegister(LIS35_CLICK_CFG, 0x55);    //aktywacja wykrywania pojedynczych klikniec we wszystkich osiach
  LIS35_WriteRegister(LIS35_CLICK_TIME_LIMIT, 0x0F); 
  LIS35_WriteRegister(LIS35_CLICK_LATENCY, 0x07);
  LIS35_WriteRegister(LIS35_CLICK_THSY_X, 0x11); //progi wykrywania dla osi X i Y = 0.5g 
  LIS35_WriteRegister(LIS35_CLICK_THSZ, 0x02);   //prog wykrywania dla osi Z = 1g 

  LCD_Initialize();                              //inicjalizacja wyswietlacza
  LCD_WriteCommand(HD44780_CLEAR);               //wyczysc wyswietlacz
  LCD_WriteTextXY("X: \0",0,0);  
  LCD_WriteTextXY("Y: \0",8,0);  
  LCD_WriteTextXY("Z: \0",0,1);  
  LCD_WriteTextXY("Klik:\0",8,1);  

  //korekty i rzeczywiste czulosci poszczegolnych kanalow dla danego egzemplarza ukladu LIS35
  //uklad w trybie pracy +-2g 
  //standardowa korekta = 0, czulosc 1800
  #define xKor -4    //-(odczytana wartosc przy pol poziomym)
  #define yKor 2
  #define zKor 1
  #define xCzul 1751  //9.8067g / ((wartosc przy polozeniu "+1g" - wartosc przy polozeniu "-1g") / 2) * 10000
  #define yCzul 1783
  #define zCzul 1767
  while (1) {
    /*Tu nalezy umiescic glowny kod programu*/
		LIS35_GetPosition(&x, &y, &z);
    LIS35_ReadRegister(0x39,&click);
    x+=xKor; 
    y+=yKor; 
    z+=zKor; 
    //sprintf((char *)Tekst, "%3d \0", x);     
    sprintf((char *)Tekst, " %1d.%02d\0", abs((x*xCzul)/98067), abs(((x*xCzul*100)/98067)%100));      
    if (x<0) Tekst[0]='-';
    LCD_WriteTextXY(Tekst,2,0);         
    //sprintf((char *)Tekst, "%3d \0", y);      
    sprintf((char *)Tekst, " %1d.%02d\0", abs((y*yCzul)/98067), abs(((y*yCzul*100)/98067)%100));      
    if (y<0) Tekst[0]='-';
    LCD_WriteTextXY(Tekst,10,0);         
    //sprintf((char *)Tekst, "%3d \0", z);      
    sprintf((char *)Tekst, " %1d.%02d\0", abs((z*zCzul)/98067), abs(((z*zCzul*100)/98067)%100));      
    if (z<0) Tekst[0]='-';
    LCD_WriteTextXY(Tekst,2,1);         

    if ((click&0x40)==0x40) {
      switch (click){
        case 0x50: {LCD_WriteTextXY("Z\0",15,1);} break;
        case 0x44: {LCD_WriteTextXY("Y\0",14,1);} break;         
        case 0x41: {LCD_WriteTextXY("X\0",13,1);} break;
      }
    } else {
      LCD_WriteTextXY("   \0",13,1);         
    }

    GPIO_WriteBit(GPIOB, GPIO_Pin_15, (BitAction)(1-GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_15)));
    for (i=0;i<2000000ul;i++); 					   
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

	/*Tu nalezy umiescic kod zwiazny z konfiguracja sygnalow zegarowych potrzebnych w programie peryferiow*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//wlacz taktowanie portu GPIO A
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//wlacz taktowanie portu GPIO B
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);//wlacz taktowanie portu GPIO C
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE); //wlacz taktowanie ukladu SPI 
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

  /*Tu nalezy umiescic kod zwiazny z konfiguracja poszczegolnych portow GPIO potrzebnych w programie*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

}
