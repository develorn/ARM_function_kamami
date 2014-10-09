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

#include "mems2_spi_lib.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"

void LIS35_SPI_HwConfig(void)
{ //GPIO and SPI configuration
  GPIO_InitTypeDef  GPIO_InitStructure; 
  SPI_InitTypeDef  SPI_InitStructure;

  //GPIO Config
  GPIO_InitStructure.GPIO_Pin =LIS35_MOSI | LIS35_MISO | LIS35_SCK;    //SPI - SCK, MISO & MOSI
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(LIS35_SPI_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LIS35_SS;                             //SPI - SS
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(LIS35_SPI_GPIO, &GPIO_InitStructure);

  //SPI Config
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(LIS35_SPI, &SPI_InitStructure);

  SPI_Cmd(LIS35_SPI, ENABLE);                                         // Wlacz SPI
}


char SPI_Transmit(char Data)
{ 
  SPI_I2S_SendData(LIS35_SPI, Data);
  while (SPI_I2S_GetFlagStatus(LIS35_SPI, SPI_I2S_FLAG_RXNE) == RESET);
  return (char) SPI_I2S_ReceiveData(LIS35_SPI);
}

    
void SPI_CS_Enable(void)
{
  GPIO_ResetBits(LIS35_SPI_GPIO, LIS35_SS);
}


void SPI_CS_Disable(void)
{
  GPIO_SetBits(LIS35_SPI_GPIO, LIS35_SS);
}


void LIS35_RebootMemory()
{
  SPI_CS_Enable();
  SPI_Transmit(LIS35_WRITE|LIS35_ADDR_NO_INC|LIS35_REG_CR2);
  SPI_Transmit(LIS35_REG_CR2_BOOT);
  SPI_CS_Disable();
}


char LIS35_SPI_Init(void)
{
  char RegVal, LIS35Settings;
  
  //Reset LIS35 configuration
  LIS35_RebootMemory(); 
  LIS35_ReadRegister(LIS35_REG_CR1, &RegVal);

  //Write settings - activate all axis
  LIS35Settings = LIS35_REG_CR1_XEN | LIS35_REG_CR1_YEN | LIS35_REG_CR1_ZEN | LIS35_REG_CR1_ACTIVE;
  //LIS35Settings | = LIS35_REG_CR1_FULL_SCALE; //Full Scale select
  LIS35_WriteRegister(LIS35_REG_CR1, LIS35Settings);
  //Read configuration - if OK, LIS35 is up and running
  LIS35_ReadRegister(LIS35_REG_CR1, &RegVal);
  if (RegVal == LIS35Settings) return LIS35_OK;
  return LIS35_ERROR;
}


void LIS35_ReadRegister(char addr,char * v)
{
  SPI_CS_Enable();
  SPI_Transmit(LIS35_READ|LIS35_ADDR_NO_INC|addr);
  *v=SPI_Transmit(0xFF);
  SPI_CS_Disable();
}


void LIS35_WriteRegister(char addr, char v)
{
  SPI_CS_Enable();
  SPI_Transmit(LIS35_WRITE|LIS35_ADDR_NO_INC|addr);
  SPI_Transmit(v);
  SPI_CS_Disable();
}


void LIS35_GetPosition(signed char * x, signed char * y, signed char * z)
{
  SPI_CS_Enable();
  SPI_Transmit(LIS35_READ|LIS35_ADDR_INC_SPI|LIS35_REG_OUTX);

  *x=SPI_Transmit(0xFF);
  SPI_Transmit(0xFF);
  *y=SPI_Transmit(0xFF);
  SPI_Transmit(0xFF);
  *z=SPI_Transmit(0xFF);
  SPI_CS_Disable();
}
