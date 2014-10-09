#define TFTN_GPIO GPIOA      //GPIO where TFT is connected
                                               
#define TFTN_CLK  GPIO_Pin_5 //SPI SCK  = TFT CLK line
#define TFTN_MISO GPIO_Pin_6 //SPI MOSI = TFT DIN line
#define TFTN_MOSI GPIO_Pin_7 //SPI MISO  - unused, only MCU->TFT communication in KAmod TFT2
#define TFTN_SS   GPIO_Pin_4 //SPI SS   = TFT SCE line
#define TFTN_RES  GPIO_Pin_8 //           TFT RES line
//NOTE: since TFT needs 9-bit word transmision, SPI controller in STM32 CANNOT be used


 // Philips PCF8833 LCD controller command codes
#define NOP 0x00      // nop
#define SWRESET 0x01  // software reset
#define BSTROFF 0x02  // booster voltage OFF
#define BSTRON 0x03   // booster voltage ON
#define RDDIDIF 0x04  // read display identification
#define RDDST 0x09    // read display status
#define SLEEPIN 0x10  // sleep in
#define SLEEPOUT 0x11 // sleep out
#define PTLON 0x12    // partial display mode
#define NORON 0x13    // display normal mode
#define INVOFF 0x20   // inversion OFF
#define INVON 0x21    // inversion ON
#define DALO 0x22     // all pixel OFF
#define DAL 0x23      // all pixel ON
#define SETCON 0x25   // write contrast
#define DISPOFF 0x28  // display OFF
#define DISPON 0x29   // display ON
#define CASET 0x2A    // column address set
#define PASET 0x2B    // page address set
#define RAMWR 0x2C    // memory write
#define RGBSET 0x2D   // colour set
#define PTLAR 0x30    // partial area
#define VSCRDEF 0x33  // vertical scrolling definition
#define TEOFF 0x34    // test mode
#define TEON 0x35     // test mode
#define MADCTL 0x36   // memory access control
#define SEP 0x37      // vertical scrolling start address
#define IDMOFF 0x38   // idle mode OFF
#define IDMON 0x39    // idle mode ON
#define COLMOD 0x3A   // interface pixel format
#define SETVOP 0xB0   // set Vop
#define BRS 0xB4      // bottom row swap
#define TRS 0xB6      // top row swap
#define DISCTR 0xB9   // display control
#define DOR 0xBA      // data order
#define DATCTL 0xBC   // Data scan direction, etc.
#define TCDFE 0xBD    // enable/disable DF temperature compensation
#define TCVOPE 0xBF   // enable/disable Vop temp comp
#define EC 0xC0       // internal or external oscillator
#define SETMUL 0xC2   // set multiplication factor
#define TCVOPAB 0xC3  // set TCVOP slopes A and B
#define TCVOPCD 0xC4  // set TCVOP slopes c and d
#define TCDF 0xC5     // set divider frequency
#define DF8COLOR 0xC6 // set divider frequency 8-color mode
#define SETBS 0xC7    // set bias system
#define RDTEMP 0xC8   // temperature read back
#define NLI 0xC9      // n-line inversion
                              
void TFTN_HW_Config(void);
void TFTN_Init(void);
void TFTN_SendCmd(unsigned char cCmd);
void TFTN_SendData(unsigned char cData);

void TFTN_Clear(unsigned int color);
void TFTN_WriteXY(const unsigned char *text, int x, int y, unsigned int fontColor, unsigned int bkgColor, const unsigned char * FontData);
void TFTN_FillRect(int x0, int y0, int x1, int y1, unsigned int color);
void TFTN_PutPixel(int x, int y, unsigned int color);
void TFTN_Line(int x0, int y0, int x1, int y1, unsigned int color);
void TFTN_Circle(int x0, int y0, int radius, unsigned int color);
void TFTN_FillCircle(int x0, int y0, int radius, unsigned int color);
void TFTN_WriteBMP(const unsigned int *buffer, int xPos, int yPos, int xSize, int ySize);


