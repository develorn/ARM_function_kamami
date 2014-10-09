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

#define MEMS1_GPIO GPIOE     //MEMS1 GPIO port

#define MEMS1_LC  GPIO_Pin_0  
#define MEMS1_PC  GPIO_Pin_1
#define MEMS1_SGN GPIO_Pin_2
#define MEMS1_PD  GPIO_Pin_3

#define MEMS1_POWER_DOWN  0
#define MEMS1_POWER_UP    1


void MEMS1_GPIOConfig(void);
char MEMS1_ReadPos(void);
  //0 = Front up
  //4 = Front down
  //1 = Landscape left
  //5 = Landcape right
  //2 = Portrait up
  //6 = Portrait down
  //3 = 45 deg (any position)
void MEMS1_Power(unsigned char Power);
