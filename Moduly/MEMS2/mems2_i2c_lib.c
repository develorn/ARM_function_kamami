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


#include "mems2_i2c_lib.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_i2c.h"

#define LIS35_Addr 0x3A         // 0011 101x

void LIS35_I2C_HwConfig(void)
{ //GPIO and SPI configuration
  GPIO_InitTypeDef  GPIO_InitStructure; 
  I2C_InitTypeDef I2C_InitStructure;


  //GPIO Config
  GPIO_InitStructure.GPIO_Pin = LIS35_SCL | LIS35_SDA;      //I2C - SCL & SDA
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
  GPIO_Init(LIS35_I2C_GPIO, &GPIO_InitStructure);
  
  //I2C Config
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;                //I2C mode
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;        //High to low state ratio of SCK, important for >100kHz transmission speed
  I2C_InitStructure.I2C_OwnAddress1 = 0x00;                 //I2C address at MCU side (if it works as slave)
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;               //Acknowledge enable
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; //Addressing mode
  I2C_InitStructure.I2C_ClockSpeed=100000;                  //Clock frequency
  I2C_Init(LIS35_I2C,&I2C_InitStructure);

  I2C_Cmd(LIS35_I2C, ENABLE); 
}

char LIS35_I2C_Init(void)
{
  char RegVal, LIS35Settings; 
  volatile long int i;

  //reset LIS35 settings
  LIS35_WriteRegister(LIS35_REG_CR2, LIS35_REG_CR2_BOOT);    

  //Write settings - activate all axis
  LIS35Settings = LIS35_REG_CR1_XEN | LIS35_REG_CR1_YEN | LIS35_REG_CR1_ZEN | LIS35_REG_CR1_ACTIVE;
  //LIS35Settings | = LIS35_REG_CR1_FULL_SCALE; //Full Scale select
  LIS35_WriteRegister(LIS35_REG_CR1, LIS35Settings);
  //Read configuration - if OK, LIS35 is up and running
  LIS35_ReadRegister(LIS35_REG_CR1, &RegVal);
  if (RegVal == LIS35Settings) return LIS35_OK;
  return LIS35_ERROR;
}


void LIS35_ReadRegister(char addr,char *v)
{
  //Reads one register value

  I2C_GenerateSTART(LIS35_I2C,ENABLE);
  //Test on EV5 and clear it
  while(!I2C_CheckEvent(LIS35_I2C, I2C_EVENT_MASTER_MODE_SELECT));  
  //Send LIS35 address, set I2C master in transmiter mode
  I2C_Send7bitAddress(LIS35_I2C, LIS35_Addr, I2C_Direction_Transmitter);
  //Test on EV6 and clear it
  while(!I2C_CheckEvent(LIS35_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
  //Send LIS35 local register address, which will be read
  I2C_SendData(LIS35_I2C, addr);
  // Test on EV8 and clear it
  while(!I2C_CheckEvent(LIS35_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

  //Re-generate START, transmition from slave beginning
  I2C_GenerateSTART(LIS35_I2C,ENABLE);
  //Test on EV5 and clear it
  while(!I2C_CheckEvent(LIS35_I2C, I2C_EVENT_MASTER_MODE_SELECT)); 
  //Send LIS35 address, set I2C master in receiver mode
  I2C_Send7bitAddress(LIS35_I2C, LIS35_Addr, I2C_Direction_Receiver);
  //Only one byte will be received
  //NAck must be configured just before checking EV6 -> disable Acknowledge
  I2C_AcknowledgeConfig(LIS35_I2C, DISABLE);
  //Test on EV6 and clear it
  while(!I2C_CheckEvent(LIS35_I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
  //Generate STOP Condition
  I2C_GenerateSTOP(LIS35_I2C,ENABLE);
  //Test on EV7 and clear it - Wait until DataN is in Shift register
  while(!I2C_CheckEvent(LIS35_I2C, I2C_EVENT_MASTER_BYTE_RECEIVED));
  //Read DataN from DR register
  *v=I2C_ReceiveData(LIS35_I2C);
  //Enable Acknowledge for next transmission 
  I2C_AcknowledgeConfig(LIS35_I2C, ENABLE);
}



void LIS35_WriteRegister(char addr,char v)
{
  //Writes one register value 

  I2C_GenerateSTART(LIS35_I2C,ENABLE);
  //Test on EV5 and clear it
  while(!I2C_CheckEvent(LIS35_I2C, I2C_EVENT_MASTER_MODE_SELECT));  
  //Send LIS35 address, set I2C master in transmiter mode
  I2C_Send7bitAddress(LIS35_I2C, LIS35_Addr, I2C_Direction_Transmitter);
  //Test on EV6 and clear it
  while(!I2C_CheckEvent(LIS35_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
  //Send LIS35 local register address, which will be written 
  I2C_SendData(LIS35_I2C, addr);
  //Test on EV8 and clear it
  while(!I2C_CheckEvent(LIS35_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
  //Send new register value
  I2C_SendData(LIS35_I2C, v);
  //Test on EV8 and clear it
  while(!I2C_CheckEvent(LIS35_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
  //Generate STOP Condition
  I2C_GenerateSTOP(LIS35_I2C,ENABLE); 
}

void LIS35_GetPosition(signed char * x, signed char * y, signed char * z)
{
  //reads 5 registers values into buffer in multiple bytes mode, buffer[0]=x, [2]=y, [3]=z
  signed char Bufor[5]={0};
  unsigned int NumByteToReadN, i;

  //Check, if I2C is free
  while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

  I2C_GenerateSTART(I2C1,ENABLE);
  //Test on EV5 and clear it
  while(!I2C_CheckEvent(LIS35_I2C, I2C_EVENT_MASTER_MODE_SELECT));  
  //Send LIS35 address, set I2C master in transmiter mode
  I2C_Send7bitAddress(LIS35_I2C, LIS35_Addr, I2C_Direction_Transmitter);
  //Test on EV6 and clear it
  while(!I2C_CheckEvent(LIS35_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
  //Send base register address, set address autoincrement
  I2C_SendData(LIS35_I2C, LIS35_REG_OUTX|LIS35_ADDR_INC_I2C);
  //Test on EV8 and clear it
  while(!I2C_CheckEvent(LIS35_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

  //Re-generate START, transmition from slave beginning
  I2C_GenerateSTART(LIS35_I2C,ENABLE);
  //Test on EV5 and clear it
  while(!I2C_CheckEvent(LIS35_I2C, I2C_EVENT_MASTER_MODE_SELECT)); 
  //Send LIS35 address, set I2C master in receiver mode
  I2C_Send7bitAddress(LIS35_I2C, LIS35_Addr, I2C_Direction_Receiver);
  //Test on EV6 and clear it 
  while(!I2C_CheckEvent(LIS35_I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
 
  NumByteToReadN=5;  //Read 5 consecutive registers
  i=0;               //Current read count
  while(NumByteToReadN) {
    //Before receiving last byte, disable acknowledge and generate stop
    if(NumByteToReadN == 1) {
      I2C_AcknowledgeConfig(LIS35_I2C, DISABLE);
      I2C_GenerateSTOP(LIS35_I2C, ENABLE);
    }
    //Test on EV7 and clear it
    while(!I2C_CheckEvent(LIS35_I2C, I2C_EVENT_MASTER_BYTE_RECEIVED));
    //Read a byte from the LI35 
    Bufor[i] = I2C_ReceiveData(LIS35_I2C);
    i++; 
    NumByteToReadN--;        
  }

  //Enable Acknowledge for next transmission 
  I2C_AcknowledgeConfig(LIS35_I2C, ENABLE);

  //Assign data to axis variables
  *x=Bufor[0];
  *y=Bufor[2];
  *z=Bufor[4];

  //LIS35_ReadRegister(LIS35_REG_OUTX,x);
  //LIS35_ReadRegister(LIS35_REG_OUTY,y);
  //LIS35_ReadRegister(LIS35_REG_OUTZ,z);
}
