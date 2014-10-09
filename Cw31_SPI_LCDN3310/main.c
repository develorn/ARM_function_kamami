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

//Do dzialania programu potrzebny jest modul KAmod LCD1
//polaczenia do wykonania pomiedzy modulem a zestawem ZL27ARM
//Zl27ARM            |  LCD1
//JP6=GPIOB          |
//-------------------------------------   
//JP6 PA4           <-> Con1 SCE
//JP6 PA5           <-> Con1 CLK
//JP6 PA6           <-> 
//JP6 PA7           <-> Con1 DIN
//JP6 PA8           <-> Con1 RES
//JP6 PA9           <-> Con1 D/C
//JP6 GND           <-> Con1 GND
//JP6 +3,3V (GPIOA) <-> Con1 +V

#include "stm32f10x.h"
#include "stdlib.h"
#include ".\..\Moduly\LCD1\lcd_pcd8544_lib.h"
#include ".\..\Moduly\LCD1\logo.h"

void GPIO_Config(void);
void RCC_Config(void);
void NVIC_Config(void);
void Opcja_Info(void);
void Opcja_Logo(void);
void Opcja_Negatyw(void);
void Opcja_Animacja(void);

int main(void)
{
  volatile unsigned long int i, j;
  unsigned char pozycja=1;
  bool aktualizacja=TRUE;
  unsigned char *menu[5] = {"Info\0", "Logo\0", "Negatyw\0", "Animacja\0"};

  //konfiguracja systemu
  RCC_Config();   
  GPIO_Config(); 
  NVIC_Config();  
  /*Tu nalezy umiescic ewentualne dalsze funkcje konfigurujace system*/
  GPIO_ResetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);		 

  LCDN_HwConfig();
  LCDN_Init();
  LCDN_Clear();
  
  while (1) {
    /*Tu nalezy umiescic glowny kod programu*/
    GPIO_WriteBit(GPIOB, GPIO_Pin_15, (BitAction)(1-GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_15)));

    if (!GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) && (pozycja>1)){
      pozycja--;
      aktualizacja=TRUE;
    }
    if (!GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) && (pozycja<4)){
      pozycja++;
      aktualizacja=TRUE;
    }

    if (!GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5)){
      switch (pozycja){
        case 1: Opcja_Info(); break;
        case 2: Opcja_Logo(); break;
        case 3: Opcja_Negatyw(); break;
        case 4: Opcja_Animacja(); break;
      }
      aktualizacja=TRUE;
    }

    if (aktualizacja){
      LCDN_Clear();
      LCDN_WriteXY("     MENU     \0",0,0);
      for (i=1;i<=4;i++){
        if (i==pozycja) { LCDN_WriteXY(">",0,i); }
        LCDN_WriteXY(menu[i-1],2,i);
      }
      aktualizacja=FALSE;
    }

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

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);  

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOC, &GPIO_InitStructure);  
}


void Opcja_Info(void){
  volatile unsigned long int i;
  LCDN_Clear();
  LCDN_WriteXY("    DEMO     \0",0,0);
  LCDN_WriteXY(" wy\x8Fwietlacza\0",0,1);
  LCDN_WriteXY("  z telefonu \0",0,2);
  LCDN_WriteXY("  NOKIA 3310 \0",0,3);
  LCDN_WriteXY("    modu\x8C    \0",0,4);
  LCDN_WriteXY("  KAmod LCD1 \0",0,5);
  for (i=0;i<35000000ul;i++); 
}


void Opcja_Logo(void){
  volatile unsigned long int i;
  //LCDN_Clear();
  LCDN_WriteBMP(logo);
  for (i=0;i<25000000ul;i++); 
}


void Opcja_Negatyw(void){
  static bool negatyw=FALSE;
  if (negatyw) {
    LCDN_Mode(0);  //wlacz tryb normalny
    negatyw=FALSE;
  } else {
    LCDN_Mode(1);  //wlacz negatyw
    negatyw=TRUE;
  }
}


void Opcja_Animacja(void){
  volatile unsigned long int i;            
  volatile long int x, x0;                       //x - pozycja tylu samolotu, x0 - pozycja, od ktorej ma byc wyswietlany samolot
                                                 //                                jesli x<0 lub x>64, samolot wyswietlany we fragmencie
  volatile long int kolumna, kolumnaP, kolumnaK; //aktualnie transmitowana kolumna, pierwsza i ostatnia kolumna do wyslania
                                                 //                                jesli x<0 lub x>64, samolot wyswietlany we fragmencie
  unsigned char samolocik[]={0xFC,0x22,0x75,0x21,0x22,0x2C,0x10,0x20,0x20,0x20,0x30,0xB0,0xA8,0xA8,0xA8,0x30,0x20,0x20,0x40,0x80,
                             0x01,0x02,0x03,0x04,0x04,0x04,0x04,0x04,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x04,0x04,0x02,0x01};
  unsigned char smiglo[]={0xFC,0xC0,
                          0x3F,0x03};  //2 pozycje smigla w celu animowania ruchu
  LCDN_Clear();
  for (x=-19;x<84;x++){
    if (x<0) {kolumnaP=-x;   x0=0;} else {kolumnaP=0;  x0=x;};
    if (x>64){kolumnaK=84-x;      } else {kolumnaK=20;      };

    LCDN_SetPosG(x0,2);                //przeslij gorna polowe samolotu
    for (kolumna=kolumnaP; kolumna<kolumnaK; kolumna++){
      LCDN_SPI_Transmit(samolocik[kolumna]);
    }
    if (x<69) {LCDN_SPI_Transmit(smiglo[abs(x)%2+0]);}

    LCDN_SetPosG(x0,3);                //przeslij dolna polowe samolotu
    for (kolumna=kolumnaP+20; kolumna<kolumnaK+20; kolumna++){
      LCDN_SPI_Transmit(samolocik[kolumna]);
    }
    if (x<69) {LCDN_SPI_Transmit(smiglo[abs(x)%2+2]);}

    for (i=0;i<2000000ul;i++);         //odczekaj
    LCDN_SetPosG(x0,2);                //usun kolumne za samolotem, reszta samolotu zostanie nadpisana przy transmisji kolejnej klatki animacji
    LCDN_SPI_Transmit(0x00);
    LCDN_SetPosG(x0,3);
    LCDN_SPI_Transmit(0x00);
  }  
}

