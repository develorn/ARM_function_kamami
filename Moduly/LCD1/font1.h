#define LCDN_FONT_WIDTH 6;     //width of font characters in pixels
#define LCDN_FONT_OFFSET 32;   //code of the first character in font table (reduces memory usage for unused codes at the beggining of the tabel)
const unsigned char LCDN_Font[]={
/*0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 0
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 1
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 2
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 3
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 4
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 5
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 6
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 7
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 8
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 9
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 10
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 11
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 12
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 13
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 14
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 15
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 16
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 17
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 18
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 19
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 20
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 21 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 22
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 23
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 24
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 25
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 26
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 27
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 28
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 29
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 30
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //code 31 */

0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // space
0x00, 0x00, 0x2F, 0x00, 0x00, 0x00,  // !
0x00, 0x07, 0x00, 0x07, 0x00, 0x00,  // "
0x14, 0x7F, 0x14, 0x7F, 0x14, 0x00,  // #
0x24, 0x2a, 0x7f, 0x2a, 0x12, 0x00,  // $
0xc4, 0xc8, 0x10, 0x26, 0x46, 0x00,  // %
0x36, 0x49, 0x55, 0x22, 0x50, 0x00,  // &
0x00, 0x05, 0x03, 0x00, 0x00, 0x00,  // '
0x00, 0x1c, 0x22, 0x41, 0x00, 0x00,  // (
0x00, 0x41, 0x22, 0x1c, 0x00, 0x00,  // )
0x14, 0x08, 0x3E, 0x08, 0x14, 0x00,  // *
0x08, 0x08, 0x3E, 0x08, 0x08, 0x00,  // +
0x00, 0x00, 0x50, 0x30, 0x00, 0x00,  // ,
0x10, 0x10, 0x10, 0x10, 0x10, 0x00,  // -
0x00, 0x60, 0x60, 0x00, 0x00, 0x00,  // .
0x20, 0x10, 0x08, 0x04, 0x02, 0x00,  // /
0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00,  // 0
0x00, 0x42, 0x7F, 0x40, 0x00, 0x00,  // 1
0x42, 0x61, 0x51, 0x49, 0x46, 0x00,  // 2
0x21, 0x41, 0x45, 0x4B, 0x31, 0x00,  // 3
0x18, 0x14, 0x12, 0x7F, 0x10, 0x00,  // 4
0x27, 0x45, 0x45, 0x45, 0x39, 0x00,  // 5
0x3C, 0x4A, 0x49, 0x49, 0x30, 0x00,  // 6
0x01, 0x71, 0x09, 0x05, 0x03, 0x00,  // 7
0x36, 0x49, 0x49, 0x49, 0x36, 0x00,  // 8
0x06, 0x49, 0x49, 0x29, 0x1E, 0x00,  // 9
0x00, 0x36, 0x36, 0x00, 0x00, 0x00,  // :
0x00, 0x56, 0x36, 0x00, 0x00, 0x00,  // ;
0x08, 0x14, 0x22, 0x41, 0x00, 0x00,  // <
0x14, 0x14, 0x14, 0x14, 0x14, 0x00,  // =
0x00, 0x41, 0x22, 0x14, 0x08, 0x00,  // >
0x02, 0x01, 0x51, 0x09, 0x06, 0x00,  // ?
0x32, 0x49, 0x59, 0x51, 0x3E, 0x00,  // @

0x7E, 0x11, 0x11, 0x11, 0x7E, 0x00,  // A
0x7F, 0x49, 0x49, 0x49, 0x36, 0x00,  // B
0x3E, 0x41, 0x41, 0x41, 0x22, 0x00,  // C
0x7F, 0x41, 0x41, 0x22, 0x1C, 0x00,  // D
0x7F, 0x49, 0x49, 0x49, 0x41, 0x00,  // E
0x7F, 0x09, 0x09, 0x09, 0x01, 0x00,  // F
0x3E, 0x41, 0x49, 0x49, 0x7A, 0x00,  // G
0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00,  // H
0x00, 0x41, 0x7F, 0x41, 0x00, 0x00,  // I
0x20, 0x40, 0x41, 0x3F, 0x01, 0x00,  // J
0x7F, 0x08, 0x14, 0x22, 0x41, 0x00,  // K
0x7F, 0x40, 0x40, 0x40, 0x40, 0x00,  // L
0x7F, 0x02, 0x0C, 0x02, 0x7F, 0x00,  // M
0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00,  // N
0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00,  // O
0x7F, 0x09, 0x09, 0x09, 0x06, 0x00,  // P
0x3E, 0x41, 0x51, 0x21, 0x5E, 0x00,  // Q
0x7F, 0x09, 0x19, 0x29, 0x46, 0x00,  // R
0x46, 0x49, 0x49, 0x49, 0x31, 0x00,  // S
0x01, 0x01, 0x7F, 0x01, 0x01, 0x00,  // T
0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00,  // U
0x1F, 0x20, 0x40, 0x20, 0x1F, 0x00,  // V
0x3F, 0x40, 0x38, 0x40, 0x3F, 0x00,  // W
0x63, 0x14, 0x08, 0x14, 0x63, 0x00,  // X
0x07, 0x08, 0x70, 0x08, 0x07, 0x00,  // Y
0x61, 0x51, 0x49, 0x45, 0x43, 0x00,  // Z
0x00, 0x7F, 0x41, 0x41, 0x00, 0x00,  // [
0x02, 0x04, 0x08, 0x10, 0x20, 0x00,  // code 92 (backslash)
0x00, 0x41, 0x41, 0x7F, 0x00, 0x00,  // ]
0x04, 0x02, 0x01, 0x02, 0x04, 0x00,  // ^
0x40, 0x40, 0x40, 0x40, 0x40, 0x00,  // _
0x00, 0x01, 0x02, 0x04, 0x00, 0x00,  // '

0x20, 0x54, 0x54, 0x54, 0x78, 0x00,  // a
0x7F, 0x48, 0x44, 0x44, 0x38, 0x00,  // b
0x38, 0x44, 0x44, 0x44, 0x20, 0x00,  // c
0x38, 0x44, 0x44, 0x48, 0x7F, 0x00,  // d
0x38, 0x54, 0x54, 0x54, 0x18, 0x00,  // e
0x08, 0x7E, 0x09, 0x01, 0x02, 0x00,  // f
0x0C, 0x52, 0x52, 0x52, 0x3E, 0x00,  // g
0x7F, 0x08, 0x04, 0x04, 0x78, 0x00,  // h
0x00, 0x44, 0x7D, 0x40, 0x00, 0x00,  // i
0x20, 0x40, 0x44, 0x3D, 0x00, 0x00,  // j
0x7F, 0x10, 0x28, 0x44, 0x00, 0x00,  // k
0x00, 0x41, 0x7F, 0x40, 0x00, 0x00,  // l
0x7C, 0x04, 0x18, 0x04, 0x78, 0x00,  // m
0x7C, 0x08, 0x04, 0x04, 0x78, 0x00,  // n
0x38, 0x44, 0x44, 0x44, 0x38, 0x00,  // o
0x7C, 0x14, 0x14, 0x14, 0x08, 0x00,  // p
0x08, 0x14, 0x14, 0x18, 0x7C, 0x00,  // q
0x7C, 0x08, 0x04, 0x04, 0x08, 0x00,  // r
0x48, 0x54, 0x54, 0x54, 0x20, 0x00,  // s
0x04, 0x3F, 0x44, 0x40, 0x20, 0x00,  // t
0x3C, 0x40, 0x40, 0x20, 0x7C, 0x00,  // u
0x1C, 0x20, 0x40, 0x20, 0x1C, 0x00,  // v
0x3C, 0x40, 0x30, 0x40, 0x3C, 0x00,  // w
0x44, 0x28, 0x10, 0x28, 0x44, 0x00,  // x
0x0C, 0x50, 0x50, 0x50, 0x3C, 0x00,  // y
0x44, 0x64, 0x54, 0x4C, 0x44, 0x00,  // z
0x00, 0x08, 0x36, 0x41, 0x00, 0x00,  // {
0x00, 0x00, 0x7F, 0x00, 0x00, 0x00,  // |
0x00, 0x41, 0x36, 0x08, 0x00, 0x00,  // }
0x00, 0x10, 0x08, 0x10, 0x08, 0x00,  // ~

0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // code 127

0x20, 0x54, 0x54, 0xD4, 0x78, 0x00,  // A, code 128, x80
0x3C, 0x42, 0x42, 0x43, 0x24, 0x00,  // C' 
0x7F, 0x49, 0x49, 0xC9, 0x41, 0x00,  // E,
0x10, 0x7F, 0x44, 0x40, 0x40, 0x00,  // L/
0x7E, 0x08, 0x13, 0x20, 0x7E, 0x00,  // N'
0x3C, 0x42, 0x46, 0x43, 0x3C, 0x00,  // O'
0x44, 0x4A, 0x4A, 0x4B, 0x32, 0x00,  // S'
0x62, 0x56, 0x4B, 0x46, 0x42, 0x00,  // X'
0x69, 0x59, 0x49, 0x4D, 0x4B, 0x00,  // Z'
0x20, 0x54, 0x54, 0xD4, 0x78, 0x00,  // a, code 137, x89
0x38, 0x44, 0x44, 0x46, 0x20, 0x00,  // c'
0x1C, 0x2A, 0x2A, 0x6A, 0x0C, 0x00,  // e,
0x00, 0x51, 0x7F, 0x44, 0x00, 0x00,  // l/
0x7C, 0x08, 0x04, 0x06, 0x78, 0x00,  // n'
0x38, 0x44, 0x44, 0x46, 0x38, 0x00,  // o'
0x48, 0x54, 0x54, 0x56, 0x20, 0x00,  // s'
0x44, 0x64, 0x56, 0x4D, 0x44, 0x00,  // x'
0x44, 0x64, 0x55, 0x4C, 0x44, 0x00,  // z'

0x00, 0x06, 0x09, 0x09, 0x06, 0x00,  // �


};
