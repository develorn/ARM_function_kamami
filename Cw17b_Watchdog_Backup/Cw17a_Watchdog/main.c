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
void IWDG_Config(void);

int main(void)
{
  volatile unsigned long int i;
  //konfiguracja systemu
  RCC_Config();   
  GPIO_Config();
  if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET) {//Sprawdzenie, czy system rozpoczyna prace po resecie od IWDG
  	GPIO_SetBits(GPIOB, GPIO_Pin_15);                //jesli start po resecie - wlacz diode  8 i odczekaj 0,5s
  	RCC_ClearFlag();                                 //wyczysc flage resetu
    for (i=0;i<10000000ul;i++);
  } 
  NVIC_Config();  
  IWDG_Config(); 
  /*Tu nalezy umiescic ewentualne dalsze funkcje konfigurujace system*/
  GPIO_ResetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);

  while (1) {
    /*Tu nalezy umiescic glowny kod programu*/
    IWDG_ReloadCounter();                       	 //przeladowanie IWDG				   
    if (!GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)){ //po wcisneciu SW0 wejscie w nieskonczona petle
    	GPIO_SetBits(GPIOB, GPIO_Pin_8);            
      while (1);
    }
    GPIO_WriteBit(GPIOB, GPIO_Pin_11, (BitAction)(1-GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_11)));
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
    RCC_PLLCmd(ENABLE);								                   //wlacz PLL
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);  //odczekaj na poprawne uruchomienie PLL
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);           //ustaw PLL jako zrodlo sygnalu zegarowego
    while(RCC_GetSYSCLKSource() != 0x08);                //odczekaj az PLL bedzie sygnalem zegarowym systemu

	/*Tu nalezy umiescic kod zwiazny z konfiguracja sygnalow zegarowych potrzebnych w programie peryferiow*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//wlacz taktowanie portu GPIO A
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
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void IWDG_Config(void){
  //konfigurowanie watchdoga IWDG
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);	// Zezwolenie na zapis rejestrów IWDG
	IWDG_SetPrescaler(IWDG_Prescaler_4);       	  // IWDG taktowany zegarem: 40kHz/4 = 10kHz
	IWDG_SetReload(0xFFF);                        // Przepelnienie IWDG po okolo 0,4s    
	IWDG_ReloadCounter();                       	// Przeladowanie IWDG
 	IWDG_Enable();  	                            // Wlaczenie IWDG i LSI
}
