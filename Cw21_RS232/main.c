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

void GPIO_Config(void);
void RCC_Config(void);
void NVIC_Config(void);
void USART_Config(void);

int main(void)
{
  volatile unsigned long int i;
  unsigned int stanPrzyciskow, stanDiod, stanGPIOB;

  //konfiguracja systemu
  RCC_Config();   
  GPIO_Config(); 
  NVIC_Config();  
  USART_Config(); 
  /*Tu nalezy umiescic ewentualne dalsze funkcje konfigurujace system*/
  
  while (1) {
    /*Tu nalezy umiescic glowny kod programu*/
    if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE)!=RESET){
      stanDiod=USART_ReceiveData(USART1);        //Odbierz stan diod do wlaczenia
      stanDiod-=48;                              //odejmij 48 od odebranej danej; pozwala to zamienic tekst z liczba na jej postac binarna
      stanDiod=stanDiod<<8;                      //przesuniecie stanu diod w lewo o 8 poniewaz diody zaczynaja sie w porcie B na pozycji 8
      stanGPIOB=GPIO_ReadOutputData(GPIOB);		   //pobierz aktualny stan calego portu B
      stanGPIOB=stanGPIOB&0x00FF;					       //wyzeruj bity odpowiedzialne za LEDy
                                                 //ustaw pozycje zapalonych LEDow i wstaw pozostala czesc stanu portu odczytana wczesniej, tak, by nie psuc ustawien innych ledow i mlodszej czesci portu
      GPIO_Write(GPIOB, stanDiod | stanGPIOB);   //zapisz nowy stan portu
    }

    stanPrzyciskow=((0xFFFF-GPIO_ReadInputData(GPIOA))&0x000F);

		if(stanPrzyciskow>0)
		{
	    USART_SendData(USART1, stanPrzyciskow+48); //Zakoduj i wyslij aktualny stan przyciskow
      GPIO_WriteBit(GPIOB, GPIO_Pin_14, (BitAction)(1-GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_14)));
		}

    for (i=0; i<5000000; i++);
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
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);//wlacz taktowanie USART1
	
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

  USART_InitStructure.USART_BaudRate = 9600;                                      //Predkosc transmisji =9600bps
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     //Dlugosc slowa = 8 bitow
  USART_InitStructure.USART_StopBits = USART_StopBits_1;                          //1 bit stopu
  USART_InitStructure.USART_Parity = USART_Parity_No;                             //Brak kontroli parzystosci
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //Kontrola przeplywu danych (brak)
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	                //Tryb pracy
	USART_Init(USART1, &USART_InitStructure);

	USART_Cmd(USART1, ENABLE);
}
