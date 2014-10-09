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

#define MPL115_SPI  SPI1
#define MPL115_GPIO GPIOA         //MEMS1 GPIO port
                                
#define MPL115_CS  GPIO_Pin_4     //SPI SS
#define MPL115_SDI GPIO_Pin_7     //SPI MOSI
#define MPL115_SDO GPIO_Pin_6     //SPI MISO
#define MPL115_CLK GPIO_Pin_5     //SPI SCL

//#define MPL115_ERROR 1
//#define MPL115_OK 	0

#define MPL115_WRITE    0x00      //SPI
#define MPL115_READ 	  0x80      //SPI

#define MPL115_POUTH 	  0x00      //10-bit pressure output value MSB
#define MPL115_POUTL 	  0x01      //10-bit pressure output value LSB
#define MPL115_TOUTH 	  0x02      //10-bit temerature output value MSB
#define MPL115_TOUTL 	  0x03      //10-bit temerature output value LSB
#define MPL115_COEF1 	  0x04      //96-bit coefficient data 1st byte A0MSB
#define MPL115_COEF2 	  0x05      //96-bit coefficient data 2st byte A0LSB
#define MPL115_COEF3 	  0x06      //96-bit coefficient data 3st byte B1MSB
#define MPL115_COEF4  	0x07      //96-bit coefficient data 4st byte B1LSB
#define MPL115_COEF5  	0x08      //96-bit coefficient data 5st byte B2MSB
#define MPL115_COEF6  	0x09      //96-bit coefficient data 6st byte B2LSB
#define MPL115_COEF7  	0x0A      //96-bit coefficient data 7st byte C12MSB
#define MPL115_COEF8  	0x0B      //96-bit coefficient data 8st byte C12LSB
#define MPL115_COEF9  	0x0C      //96-bit coefficient data 9st byte C11MSB
#define MPL115_COEF10 	0x0D      //96-bit coefficient data 10st byte C11LSB
#define MPL115_COEF11 	0x0E      //96-bit coefficient data 11st byte C22MSB
#define MPL115_COEF12 	0x0F      //96-bit coefficient data 12st byte C22LSB

#define MPL115_PRESS 	  0x10      //Start Pressure Conversion
#define MPL115_TEMP 	  0x11      //Start Temperature Conversion
#define MPL115_BOTH 	  0x12      //Start Both Conversions

typedef struct {
  signed short int sia0;
  signed short int sib1;
  signed short int sib2;
  signed short int sic12;
  signed short int sic11;
  signed short int sic22;
} sCoeffs;

void MPL115_SPI_HwConfig(void);                                             
void MPL115_ReadCoeffs(sCoeffs * Coeffs);
void MPL115_ReadTemp(long int * RawTemp);
void MPL115_ReadPressTemp(unsigned short int * RawTemp, unsigned short int * RawPress);
void MPL115_PressCalc(long int * CalcPress, unsigned short int * RawTemp, unsigned short int * RawPress, sCoeffs * Coeffs);

void SPI_CS_Enable(void);
void SPI_CS_Disable(void);
