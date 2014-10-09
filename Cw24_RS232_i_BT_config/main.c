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

//Do dzialania programu potrzebny jest modul KAmodBTM 222
//polaczenia do wykonania pomiedzy modulem a zestawem ZL27ARM
//Zl27ARM            |  BTM 222
//JP6=GPIOA          |
//-------------------------------------
//JP6 PA9           <-> JP1 RXI
//JP6 PA10          <-> JP1 TXO
//JP6 PA11          <-> JP1 nRES
//JP6 GND           <-> JP1 GND
//JP6 +3,3V (GPIOA) <-> JP1 VCC
//                      zworka JP2 w pozycji 3V

#include "stm32f10x.h"
#include ".\..\Moduly\LCD1602\lcd_hd44780_lib.h"
#include "string.h"
#include "stdio.h"

void GPIO_Config(void);
void RCC_Config(void);
void NVIC_Config(void);
void USART_Config(void);

void BTM222_Cmd(unsigned char komenda[], volatile  unsigned char rezultat[]);
void BTM222_Error(void);


unsigned char buforRx[22]={0};
unsigned char bufRxIndex=0;
unsigned char buforTx[22]={0};
unsigned char bufTxIndex=0;
volatile bool odebranoDane=FALSE;

int main(void)
{
  volatile unsigned long int i,k;
  unsigned char rezultatKomendy[20]={0};

  //konfiguracja systemu
  RCC_Config();   
  GPIO_Config(); 
  NVIC_Config();  
  USART_Config();
  /*Tu nalezy umiescic ewentualne dalsze funkcje konfigurujace system*/
  GPIO_ResetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);		 
  LCD_Initialize();                                          //inicjalizacja wysietlacza
  LCD_WriteCommand(HD44780_CLEAR);                           //wyczysc wyswietlacz
  LCD_WriteText("Start\0");               

  GPIO_ResetBits(GPIOA, GPIO_Pin_11);                        //reset ukladu BTM222
  GPIO_SetBits(GPIOA, GPIO_Pin_11);
  for (i=0; i<50000000ul; i++);                              //odczekanie na wykonanie resetu


  BTM222_Cmd("ATE0\r\0", rezultatKomendy);                   //wylacz echo
  if (strcmp((char *)rezultatKomendy, "OK")!=0) {            //sprawdz, czy komenda przyjeta i wykonana
     BTM222_Error();
  }

  //-------
  BTM222_Cmd("ATB?\r\0", rezultatKomendy);                   //pobierz adres BT
  if (strcmp((char *)rezultatKomendy, "ERROR")==0) {         //sprawdz, czy komenda przyjeta i wykonana
     BTM222_Error();
  } else {
    LCD_WriteTextXY("Adres BT: \0",0,0);
    LCD_WriteTextXY(rezultatKomendy,0,1);           
  }
  while (!odebranoDane);                                     //odbierz OK po ostatniej komendzie
  odebranoDane=FALSE;
  if (strcmp((char *)buforRx, "OK")!=0) {
     BTM222_Error();
  }
  for (i=0; i<70000000ul; i++);
  LCD_WriteCommand(HD44780_CLEAR);

  //-------
  //BTM222_Cmd("ATN=KAmodBT demo\r\0", rezultatKomendy);       //ustaw nazwe modulu

  //-------    
  BTM222_Cmd("ATN?\r\0", rezultatKomendy);                   //pobierz nazwe
  if (strcmp((char *)rezultatKomendy, "ERROR")==0) {         //sprawdz, czy komenda przyjeta i wykonana
     BTM222_Error();
  } else {
     LCD_WriteTextXY(rezultatKomendy,0,0);           
  }
  while (!odebranoDane);                                     //odbierz OK po ostatniej komendzie
  odebranoDane=FALSE;
  if (strcmp((char *)buforRx, "OK")!=0) {
     BTM222_Error();
  }

  //-------
  //BTM222_Cmd( "ATP=1234\r\0", rezultatKomendy);             //Ustaw PIN

  //-------
  BTM222_Cmd("ATP?\r\0", rezultatKomendy);                    //Pobierz PIN
  if (strcmp((char *)rezultatKomendy, "ERROR")==0) {          //sprawdz, czy komenda przyjeta i wykonana
     BTM222_Error();
  } else {
    LCD_WriteTextXY("PIN: \0",0,1);
    LCD_WriteTextXY(rezultatKomendy,5,1);     
  }
  while (!odebranoDane);                                     //odbierz OK po ostatniej komendzie
  odebranoDane=FALSE;
  if (strcmp((char *)buforRx, "OK")!=0) {
     BTM222_Error();
  }
  for (i=0; i<7000000ul; i++);

  //-------
  BTM222_Cmd( "ATO\r\0", rezultatKomendy);                   //przejdz do trybu Data mode


  GPIO_SetBits(GPIOB, GPIO_Pin_8);		 
  k=0;
  while (1) {
    /*Tu nalezy umiescic glowny kod programu*/
    GPIO_SetBits(GPIOB, GPIO_Pin_9);	
    sprintf((char *)buforTx,"K=%d\r",k);                     //Co kilka sekund wyslij stan licznika k
    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);            //Wlacz przerwanie = rozpocznij transmisje
    k++;
    GPIO_ResetBits(GPIOB, GPIO_Pin_9);		 
    for (i=0; i<30000000; i++); 
  };
  return 0;
}


void RCC_Config(void)
//konfigurowanie sygnalow taktujacych
{
  ErrorStatus HSEStartUpStatus;  //zmienna opisujaca rezultat uruchomienia HSE

  RCC_DeInit();	                                         //Reset ustawien RCC
  RCC_HSEConfig(RCC_HSE_ON);                             //Wlaczenie HSE
  HSEStartUpStatus = RCC_WaitForHSEStartUp();		     //Odczekaj az HSE bedzie gotowy
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

	/*Tu nalezy umiescic kod zwiazny z konfiguracja sygnalow zegarowych potrzebnych w programie peryferiow*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//wlacz taktowanie portu GPIO A
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//wlacz taktowanie portu GPIO B
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);//wlacz taktowanie portu GPIO B
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

  //PA11 - nRES
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

               
void USART_Config(void)
{
  //konfigurowanie ukladu USART
  USART_InitTypeDef  USART_InitStructure; 

  USART_InitStructure.USART_BaudRate = 19200;                    //Predkosc transmisji w bps
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;    //Dlugosc dlowa danych
  USART_InitStructure.USART_StopBits = USART_StopBits_1;         //Bity stopu (1 bit)
  USART_InitStructure.USART_Parity = USART_Parity_No;            //Bit parzystosci (wylaczony)
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //Sprzetowa kontrola przeplywu wylaczona
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//Tryb pracy (odbior i nadawnie)
	
  USART_Init(USART1, &USART_InitStructure);

  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
  USART_Cmd(USART1, ENABLE);  
}


void BTM222_Cmd(unsigned char * komenda, volatile unsigned char * rezultat){

  unsigned long int i=0;

  do {
    buforTx[i]=komenda[i];
    i++;
  } while ((komenda[i-1]!=0x00)&&(komenda[i-1]!=0x0D));            //Skopiuj komende do bufora nadawczego
  USART_ITConfig(USART1, USART_IT_TXE, ENABLE);                    //Wlacz przerwanie = rozpocznij wysylanie komendy
  odebranoDane=FALSE;                                              //Rozpocznij oczekiwanie na informacje zwrotna z modulu
  while (!odebranoDane);                                           //Odczekaj, az odebrane zostana informacje zwrotne z modulu
  i=0;
  do {
    rezultat[i]=buforRx[i];
    i++;
  } while ((rezultat[i-1]!=0x00)&&(rezultat[i-1]!=0x0D));          //Skopiuj dane z bufora odbiorczego jako rezultat ostatniej komendy
  odebranoDane=FALSE;                                             
}


void BTM222_Error(void){
  LCD_WriteCommand(HD44780_CLEAR);
  LCD_WriteText("Blad modulu BTM\0");               
  while(1);
}
