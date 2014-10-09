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


#include "bar_spi_lib.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h" 


void MPL115_SPI_HwConfig(void)
{ //GPIO and SPI configuration
  GPIO_InitTypeDef  GPIO_InitStructure; 
  SPI_InitTypeDef  SPI_InitStructure;

  //GPIO Config
  GPIO_InitStructure.GPIO_Pin =MPL115_SDI | MPL115_SDO | MPL115_CLK;    //SPI - SCK, MISO & MOSI
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(MPL115_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = MPL115_CS;                             //SPI - SS
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(MPL115_GPIO, &GPIO_InitStructure);

  SPI_CS_Disable();

  //SPI Config
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(MPL115_SPI, &SPI_InitStructure);

  SPI_Cmd(MPL115_SPI, ENABLE);  // Wlacz SPI
}


char SPI_Transmit(char cData)
{ //SPI data transmision
  volatile short int i; 
  SPI_I2S_SendData(MPL115_SPI, cData);
  while (SPI_I2S_GetFlagStatus(MPL115_SPI, SPI_I2S_FLAG_RXNE) == RESET);
  for (i=0; i<20; i++);
  return SPI_I2S_ReceiveData(MPL115_SPI);  
}


void SPI_CS_Enable(void)
{
  volatile short int i;
  GPIO_ResetBits(MPL115_GPIO, MPL115_CS);
  for (i=0; i<20000; i++);
}


void SPI_CS_Disable(void)
{
  GPIO_SetBits(MPL115_GPIO, MPL115_CS); 
}


void MPL115_ReadTemp(long int * RawTemp)
{ //Temperature read
  short int val1, val2;
  volatile long int i;

  SPI_CS_Enable();
  SPI_Transmit(MPL115_WRITE|(MPL115_TEMP<<1));
  SPI_Transmit(0x00);
  SPI_CS_Disable();

  for (i=0; i<20000; i++);                        //3ms for temp & press. conversion 

  SPI_CS_Enable();
  SPI_Transmit(MPL115_READ|(MPL115_TOUTH<<1));
  val1=SPI_Transmit(0x0);
  val1=(val1<<8);
  SPI_Transmit(MPL115_READ|(MPL115_TOUTL<<1));
  val2=SPI_Transmit(0x0);
  val1=val1 | val2;
  val1=(val1>>6);
  *RawTemp=val1;
  SPI_CS_Disable();
}


signed int MPL115_ReadCoeffParts(unsigned char MSBaddr, unsigned char LSBaddr)
{ //read correction coefficient parts and combine them into a coefficient value
  signed int retVal;
  signed char valMSB, valLSB;

  SPI_CS_Enable();
  SPI_Transmit(MPL115_READ|(MSBaddr<<1));
  valMSB=SPI_Transmit(0x0);
  SPI_CS_Disable();

  SPI_CS_Enable();
  SPI_Transmit(MPL115_READ|(LSBaddr<<1));
  valLSB=SPI_Transmit(0x0);
  SPI_CS_Disable();

  retVal = (signed int) valMSB << 8;
  retVal += (signed int) valLSB & 0x00FF;
  return retVal;

}

void MPL115_ReadCoeffs(sCoeffs * Coeffs)
{ //read correction coefficients
  Coeffs->sia0  = MPL115_ReadCoeffParts(MPL115_COEF1,  MPL115_COEF2);
  Coeffs->sib1  = MPL115_ReadCoeffParts(MPL115_COEF3,  MPL115_COEF4);
  Coeffs->sib2  = MPL115_ReadCoeffParts(MPL115_COEF5,  MPL115_COEF6);
  Coeffs->sic12 = MPL115_ReadCoeffParts(MPL115_COEF7,  MPL115_COEF8);
  Coeffs->sic11 = MPL115_ReadCoeffParts(MPL115_COEF9,  MPL115_COEF10);
  Coeffs->sic22 = MPL115_ReadCoeffParts(MPL115_COEF11, MPL115_COEF12);
}


void MPL115_ReadPressTemp(unsigned short int * RawTemp, unsigned short int * RawPress)
{ //Read presure and temperature
  unsigned short int valMSB, valLSB;
  volatile int i;

  SPI_CS_Enable();
  SPI_Transmit(MPL115_WRITE|(MPL115_BOTH<<1));
  SPI_Transmit(0x00);
  SPI_CS_Disable();

  for (i=0; i<20000; i++);                        //3ms for temp & press. conversion 

  SPI_CS_Enable();
  SPI_Transmit(MPL115_READ|(MPL115_POUTH<<1));
  valMSB=SPI_Transmit(0x0);
  valMSB=(valMSB<<8);
  SPI_Transmit(MPL115_READ|(MPL115_POUTL<<1));
  valLSB=SPI_Transmit(0x0);
  valMSB=valMSB | valLSB;
  valMSB=(valMSB>>6);
  *RawPress=valMSB;

  SPI_Transmit(MPL115_READ|(MPL115_TOUTH<<1));
  valMSB=SPI_Transmit(0x0);
  valMSB=(valMSB<<8);
  SPI_Transmit(MPL115_READ|(MPL115_TOUTL<<1));
  valLSB=SPI_Transmit(0x0);
  valMSB=valMSB | valLSB;
  valMSB=(valMSB>>6);
  *RawTemp=valMSB;
  SPI_CS_Disable();
}



void MPL115_PressCalc(long int * CalcPress, unsigned short int * RawTemp, unsigned short int * RawPress, sCoeffs * Coeffs)
{  //pressure calculation

  //float decPcomp;
  long int decPcomp;
  signed long lt1, lt2, lt3, si_c11x1, si_a11, si_c12x2;
  signed long si_a1, si_c22x2, si_a2, si_a1x1, si_y1, si_a2x2;
  //signed int siPcomp;
  unsigned int uiPadc, uiTadc;
  uiPadc=*RawPress;
  uiTadc=*RawTemp;
  
  // Step 1 c11x1 = c11 * Padc
  lt1 = (signed long) Coeffs->sic11;
  lt2 = (signed long) uiPadc;
  lt3 = lt1*lt2;
  si_c11x1 = (signed long) lt3;
  
  // Step 2 a11 = b1 + c11x1
  lt1 = ((signed long) Coeffs->sib1)<<14;
  lt2 = (signed long) si_c11x1;
  lt3 = lt1 + lt2;
  si_a11 = (signed long)(lt3>>14);
  
  // Step 3 c12x2 = c12 * Tadc
  lt1 = (signed long) Coeffs->sic12;
  lt2 = (signed long) uiTadc;
  lt3 = lt1*lt2;
  si_c12x2 = (signed long)lt3;
  
  // Step 4 a1 = a11 + c12x2
  lt1 = ((signed long)si_a11<<11);
  lt2 = (signed long)si_c12x2;
  lt3 = lt1 + lt2;
  si_a1 = (signed long) lt3>>11;
  
  // Step 5 c22x2 = c22*Tadc
  lt1 = (signed long) Coeffs->sic22;
  lt2 = (signed long)uiTadc;
  lt3 = lt1 * lt2;
  si_c22x2 = (signed long)(lt3);
  
  // Step 6 a2 = b2 + c22x2
  lt1 = ((signed long) Coeffs->sib2<<15);
  lt2 = ((signed long)si_c22x2>1);
  lt3 = lt1+lt2;
  si_a2 = ((signed long)lt3>>16);
  
  // Step 7 a1x1 = a1 * Padc
  lt1 = (signed long)si_a1;
  lt2 = (signed long)uiPadc;
  lt3 = lt1*lt2;
  si_a1x1 = (signed long)(lt3);
  
  // Step 8 y1 = a0 + a1x1
  lt1 = ((signed long) Coeffs->sia0<<10);
  lt2 = (signed long)si_a1x1;
  lt3 = lt1+lt2;
  si_y1 = ((signed long)lt3>>10);
  
  // Step 9 a2x2 = a2 * Tadc
  lt1 = (signed long)si_a2;
  lt2 = (signed long)uiTadc;
  lt3 = lt1*lt2;
  si_a2x2 = (signed long)(lt3);
  
  // Step 10 pComp = y1 + a2x2
  lt1 = ((signed long)si_y1<<10);
  lt2 = (signed long)si_a2x2;
  lt3 = lt1+lt2;
  
  // Fixed point result with rounding
  //siPcomp = ((signed int)lt3>>13);
  //siPcomp = lt3/8192;

  // decPcomp is defined as a floating point number
  // Conversion to decimal value from 1023 ADC count value
  // ADC counts are 0 to 1023, pressure is 50 to 115kPa respectively
  //decPcomp = ((65.0/1023.0)*siPcomp)+50;
  //decPcomp = ((650*siPcomp)/1023)+500;
  decPcomp = ((650*((signed long)lt3/8192))/1023)+500;

  *CalcPress=decPcomp;
}

