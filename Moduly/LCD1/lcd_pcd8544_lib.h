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
                                               

#define LCDN_GPIO GPIOA      //LCD GPIO port
#define LCDN_SPI  SPI1       //LCD SPI port

#define LCDN_CLK GPIO_Pin_5  //SPI SCK  = LCD CLK line
#define LCDN_DIN GPIO_Pin_6  //SPI MISO = LCD DIN line
#define LCDN_DOU GPIO_Pin_7  //SPI MOSI  - unused, only MCU->LCD communication
#define LCDN_SCE GPIO_Pin_4  //SPI SS   = LCD SCE line
#define LCDN_RES GPIO_Pin_8  //           LCD RES line
#define LCDN_DC  GPIO_Pin_9  //           LCD D/C line
  
void LCDN_HwConfig(void);
void LCDN_Init(void);
void LCDN_Mode(unsigned char mode);
void LCDN_Clear(void);
void LCDN_WriteChar(unsigned char charCode);
void LCDN_WriteXY(const unsigned char *text, char x, char y);
void LCDN_SetPos(unsigned char x, unsigned char y);
void LCDN_SetPosG(unsigned char x, unsigned char y);
void LCDN_WriteBMP(const unsigned char *buffer);
void LCDN_SPI_Transmit(char cData);
//void LCDN_SPI_CS_Enable(void);
//void LCDN_SPI_CS_Disable(void);
