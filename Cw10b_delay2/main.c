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

void GPIO_Config(void);
void RCC_Config(void);
void NVIC_Config(void);
unsigned long int SysTick_Config_Mod(unsigned long int SysTick_CLKSource, unsigned long int Ticks);

extern volatile unsigned long int Milisekundy;
void Delay(unsigned long int Opoznienie);


int main(void)
{
  volatile unsigned long int i;
  unsigned int pozWlaczLED=0x0100;
  unsigned int stanGPIOB;
  unsigned long int Opoznienie;
  bool Przyspieszanie;
  //konfiguracja systemu
  RCC_Config();   
  GPIO_Config(); 
  NVIC_Config();  
  /*Tu nalezy umiescic ewentualne dalsze funkcje konfigurujace system*/

  GPIO_ResetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);		 

	if (SysTick_Config_Mod(SysTick_CLKSource_HCLK_Div8, 9000ul))
	{
		while(1);		// W razie bledu petla nieskonczona
	}					

  Opoznienie=1000;
  Przyspieszanie=TRUE;
  while (1) {
    /*Tu nalezy umiescic glowny kod programu*/
    if (Opoznienie<=20) {
      Przyspieszanie=FALSE;
    }
    if (Opoznienie>=1000) {
      Przyspieszanie=TRUE;
    }
    stanGPIOB=GPIO_ReadOutputData(GPIOB);       //pobierz aktualny stan calego portu B
    stanGPIOB=stanGPIOB&0x00FF;                 //wyzeruj bity odpowiedzialne za LEDy
    GPIO_Write(GPIOB, pozWlaczLED | stanGPIOB);   //zapisz nowy stan portu
    if (pozWlaczLED<0x8000) {                     //jesli pozycja jest mniejsza niz pozycja diody 8
      pozWlaczLED=pozWlaczLED<<1;                   //przesun pozycje o jeden w lewo
    } else {
      pozWlaczLED=0x0100;                         //jesli byla wlaczona dioda 8, ustaw pozycje 1
    }
    Delay(Opoznienie);		
    if (Przyspieszanie){
      Opoznienie*=190;
      Opoznienie/=200;
    } else {
      Opoznienie*=200;
      Opoznienie/=190;
    }
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
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
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
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//wlacz taktowanie portu GPIO B
	
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
 

void Delay(unsigned long int Opoznienie) {
  //unsigned long int MilisekundyStart;

  //MilisekundyStart = Milisekundy;
  //while ((Milisekundy - MilisekundyStart) < Opoznienie);
  Milisekundy=0;
  while ((Milisekundy) < Opoznienie);
}
