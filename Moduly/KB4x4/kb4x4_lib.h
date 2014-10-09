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

#define KBD_GPIO GPIOE     //Keyboard GPIO port

#define KBD_R1 GPIO_Pin_0  //GIPO pins connected to keyboard row lines
#define KBD_R2 GPIO_Pin_1
#define KBD_R3 GPIO_Pin_2
#define KBD_R4 GPIO_Pin_3

#define KBD_C1 GPIO_Pin_4  //GIPO pins connected to keyboard column lines
#define KBD_C2 GPIO_Pin_5
#define KBD_C3 GPIO_Pin_6
#define KBD_C4 GPIO_Pin_7


                           //Keys mapping - keys ASCII codes
#define KBD_R1C1 0x31 //1
#define KBD_R1C2 0x32 //2
#define KBD_R1C3 0x33 //3
#define KBD_R1C4 0x41 //A

#define KBD_R2C1 0x34 //4
#define KBD_R2C2 0x35 //5
#define KBD_R2C3 0x36 //6
#define KBD_R2C4 0x42 //B

#define KBD_R3C1 0x37 //7
#define KBD_R3C2 0x38 //8
#define KBD_R3C3 0x39 //9
#define KBD_R3C4 0x43 //C

#define KBD_R4C1 0x2A //*
#define KBD_R4C2 0x30 //0
#define KBD_R4C3 0x23 //#
#define KBD_R4C4 0x44 //D

void KBD_GPIOConfig(void);
unsigned char KBD_ReadKey(void);
