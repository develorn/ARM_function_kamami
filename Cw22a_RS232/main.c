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

//Do dzialania programu konieczne jest podlaczenie zestawu ZL27ARM do portu COM (RS-232) komputera PC 

#include "stm32f10x.h"
#include ".\..\Moduly\LCD1602\lcd_hd44780_lib.h"

void GPIO_Config(void);
void RCC_Config(void);
void NVIC_Config(void);
void USART_Config(void);


unsigned char buforRx[17]={0};
unsigned char bufRxIndex=0;
bool odebranoDane=FALSE;

int main(void)
{
  volatile unsigned long int i;

  //konfiguracja systemu
  RCC_Config();   
  GPIO_Config(); 
  NVIC_Config();  
  USART_Config(); 
  
  /*Tu nalezy umiescic ewentualne dalsze funkcje konfigurujace system*/
  LCD_Initialize();                                //inicjalizacja wysietlacza
  LCD_WriteCommand(HD44780_CLEAR);                 //wyczysc wyswietlacz
  LCD_WriteText((unsigned char *)"Gotowy");      

  while (1) {
    /*Tu nalezy umiescic glowny kod programu*/
    if (odebranoDane){
       LCD_WriteCommand(HD44780_CLEAR);            //wyczysc wyswietlacz
       LCD_WriteText((unsigned char *)buforRx);    //wyswietl zawarotsc bufora
       GPIO_WriteBit(GPIOB, GPIO_Pin_14, (BitAction)(1-GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_14)));
       odebranoDane=FALSE;
    }
    for (i=0; i<2000000; i++);
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
    RCC_PCLK2Config(RCC_HCLK_Div1); 					           //ustaw PCLK2=HCLK
    RCC_PCLK1Config(RCC_HCLK_Div2);					             //ustaw PCLK1=HCLK/2
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); //ustaw PLLCLK = HSE*9 czyli 8MHz * 9 = 72 MHz
    RCC_PLLCmd(ENABLE);                                  //wlacz PLL
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);  //odczekaj na poprawne uruchomienie PLL
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);           //ustaw PLL jako zrodlo sygnalu zegarowego
    while(RCC_GetSYSCLKSource() != 0x08);                //odczekaj az PLL bedzie sygnalem zegarowym systemu

	/*Tu nalezy umiescic kod zwiazny z konfiguracja sygnalow zegarowych potrzebnych w programie peryferiow*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//wlacz taktowanie portu GPIO A
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//wlacz taktowanie portu GPIO B
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);//wlacz taktowanie portu GPIO C
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);//wlacz taktowanie USART1

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

  //Wybranie grupy priorytetów
 	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

 	//Przerwanie od USART1
 	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
 	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
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

  //PA0-3 - Przyciski
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  //PA9 - Tx
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);

 	//PA10 - Rx
 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void USART_Config(void)
{
  //konfigurowanie ukladu USART
  USART_InitTypeDef  USART_InitStructure; 

  USART_InitStructure.USART_BaudRate = 9600;                     //Predkosc transmisji w bps
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;    //Dlugosc dlowa danych
  USART_InitStructure.USART_StopBits = USART_StopBits_1;         //Bity stopu (1 bit)
  USART_InitStructure.USART_Parity = USART_Parity_No;            //Bit parzystosci (wylaczony)
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //Sprzetowa kontrola przeplywu wylaczona
  USART_InitStructure.USART_Mode = USART_Mode_Rx;                //Tryb pracy (tylko odbior)
	
	USART_Init(USART1, &USART_InitStructure);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
  USART_Cmd(USART1, ENABLE);  
}

