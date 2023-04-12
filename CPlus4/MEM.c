
//Emulation of CPlus4 memories and memory bus (TED MUXing)

//#include "../resources/KERNAL.h" //well, I'm not good at copyright terms, so no dice to include a complete KERNAL

static inline unsigned char* cRTED_getMemReadPtr (register unsigned short address) {
 //cRTED_CPlus4instance* const CPlus4 = &cRTED_CPlus4; //for faster (?) operation we use a global object as memory
 if ( cRTED_CPlus4.ROMenabled && (0x8000<=address && address<0xFD00 || address>=0xFF40) ) return &cRTED_CPlus4.ROMbanks[address];
 return &cRTED_CPlus4.RAMbank[address];
}

static inline unsigned char* cRTED_getMemReadPtrCPlus4 (cRTED_CPlus4instance* CPlus4, register unsigned short address) {
 if ( cRTED_CPlus4.ROMenabled && (0x8000<=address && address<0xFD00 || address>=0xFF40) ) return &CPlus4->ROMbanks[address];
 return &CPlus4->RAMbank[address];
}


static inline unsigned char* cRTED_getMemWritePtr (register unsigned short address) {
 //cRTED_CPlus4instance* const CPlus4 = &cRTED_CPlus4; //for faster (?) operation we use a global object as memory
 if ((0xFF00<=address && address<=0xFF01) || address==0xFF08 || address==0xFF09) { //TED (INTERRUPT / keylatch registers)
  return &cRTED_CPlus4.IObankWR[address];
 }
 return &cRTED_CPlus4.RAMbank[address];
}


static inline unsigned char* cRTED_getMemWritePtrCPlus4 (cRTED_CPlus4instance* CPlus4, register unsigned short address) {
 if ((0xFF00<=address && address<=0xFF01) || address==0xFF08 || address==0xFF09) { //TED (INTERRUPT / keylatch registers)
  return &CPlus4->IObankWR[address];
 }
 return &CPlus4->RAMbank[address];
}


static inline unsigned char cRTED_readMem (register unsigned short address) {
 return *cRTED_getMemReadPtr(address);
}

static inline unsigned char cRTED_readMemCPlus4 (cRTED_CPlus4instance* CPlus4, register unsigned short address) {
 return *cRTED_getMemReadPtrCPlus4(CPlus4,address);
}


static inline void cRTED_writeMem (register unsigned short address, register unsigned char data) {
 *cRTED_getMemWritePtr(address)=data;
}

static inline void cRTED_writeMemCPlus4 (cRTED_CPlus4instance* CPlus4, register unsigned short address, register unsigned char data) {
 *cRTED_getMemWritePtrCPlus4(CPlus4,address)=data;
}


void cRTED_setROMcontent (cRTED_CPlus4instance* CPlus4) { //fill KERNAL/BASIC-ROM areas with content needed for TED-playback
 int i;

 static const unsigned char ROM_8090[] = { //$8090, for Csabo's rockstar_ate_my_border.ted
  0xa9, 0xd0, 0x8d, 0xe4, 0x02, 0xa2, 0x02, 0xbd, 0x32, 0x05, 0x95, 0x36, 0x95, 0x32, 0xca, 0xd0,
  0xf6, 0xa0, 0x00, 0xb9, 0x47, 0x81, 0x9d, 0xa5, 0x04, 0xe8, 0xc8, 0xc0, 0x0b, 0x90, 0xf4, 0xa4,
  0x22, 0xb9, 0xbc, 0x80, 0x9d, 0x9f, 0x04, 0xc6, 0x22, 0x10, 0xe6, 0x60, 0x64, 0x5f, 0x6f, 0x24,
  0x22, 0x3b, 0xa5, 0x2b, 0xa4, 0x2c, 0x20, 0x23, 0x89, 0x20, 0x4f, 0xff, 0x93, 0x0d, 0x20, 0x43,
  0x4f, 0x4d, 0x4d, 0x4f, 0x44, 0x4f, 0x52, 0x45, 0x20, 0x42, 0x41, 0x53, 0x49, 0x43, 0x20, 0x56,
  0x33, 0x2e, 0x35, 0x20, 0x00, 0xa5, 0x37, 0x38, 0xe5, 0x2b, 0xaa, 0xa5, 0x38, 0xe5, 0x2c, 0x20,
  0x5f, 0xa4, 0x20, 0x4f, 0xff, 0x20, 0x42, 0x59, 0x54, 0x45, 0x53, 0x20, 0x46, 0x52, 0x45, 0x45 };

 static const unsigned char ROM_8800[] = { //$8800 for Csabo's 6fx
  0xa2, 0x00, 0xbd, 0x01, 0x01, 0xf0, 0x06, 0x9d, 0x27, 0x05, 0xe8, 0xd0, 0xf5, 0xa9, 0x1d, 0x9d,
  0x27, 0x05, 0xe8, 0x86, 0xef, 0x4c, 0x0f, 0x87, 0xa5, 0x2b, 0xa4, 0x2c, 0x85, 0x22, 0x84, 0x23,
  0x18, 0xa0, 0x00, 0x20, 0xb0, 0x04, 0xd0, 0x06, 0xc8, 0x20, 0xb0, 0x04, 0xf0, 0x2b, 0xa0, 0x04,
  0xc8, 0x20, 0xb0, 0x04, 0xd0, 0xfa, 0xc8, 0x98, 0x65, 0x22, 0xaa, 0xa0, 0x00, 0x91, 0x22, 0x98,
  0x65, 0x23, 0xc8, 0x91, 0x22, 0x86, 0x22, 0x85, 0x23, 0x90, 0xd6, 0x18, 0xa5, 0x22, 0xa4, 0x23,
  0x69, 0x02, 0x90, 0x01, 0xc8, 0x85, 0x2d, 0x84, 0x2e, 0x60, 0xa2, 0x00, 0x20, 0x91, 0xa7, 0xc9,
  0x0d, 0xf0, 0x0b, 0x9d, 0x00, 0x02, 0xe8, 0xe0, 0x59, 0x90, 0xf1, 0x4c, 0x4c, 0xcc, 0x4c, 0x31,
  0x90, 0x20, 0x60, 0xa7, 0xa5, 0x3d, 0xc9, 0xb0, 0xd0, 0x06, 0xa5, 0x3e, 0xc9, 0x07, 0xf0, 0x3d,
  0xa0, 0x00, 0xa5, 0x02, 0xc9, 0x81, 0xd0, 0x1b, 0xd1, 0x3d, 0xd0, 0x33, 0xa0, 0x02, 0xa5, 0x4a,
  0xc9, 0xff, 0xf0, 0x2b, 0xd1, 0x3d, 0xd0, 0x07, 0x88, 0xa5, 0x49, 0xd1, 0x3d, 0xf0, 0x20, 0xa2,
  0x12, 0xd0, 0x0e, 0xb1, 0x3d, 0xc5, 0x02, 0xf0, 0x16, 0xa2, 0x12, 0xc9, 0x81, 0xf0, 0x02, 0xa2,
  0x05, 0x8a, 0x18, 0x65, 0x3d, 0x85, 0x3d, 0x90, 0xbb, 0xe6, 0x3e, 0xd0, 0xb7, 0xa0, 0x01, 0x60,
  0x20, 0x23, 0x89, 0x85, 0x31, 0x84, 0x32, 0x38, 0xa5, 0x5a, 0xe5, 0x5f, 0x85, 0x22, 0xa8, 0xa5,
  0x5b, 0xe5, 0x60, 0xaa, 0xe8, 0x98, 0xf0, 0x25, 0xa5, 0x5a, 0x38, 0xe5, 0x22, 0x85, 0x5a, 0xb0,
  0x03, 0xc6, 0x5b, 0x38, 0xa5, 0x58, 0xe5, 0x22, 0x85, 0x58, 0xb0, 0x09, 0xc6, 0x59, 0x90, 0x05,
  0x20, 0x89, 0x81, 0x91, 0x58, 0x88, 0xd0, 0xf8, 0x20, 0x89, 0x81, 0x91, 0x58, 0xc6, 0x5b, 0xc6 };

 static const unsigned char ROM_BackToBASIC[] = { 0xA2,0x80, 0x29,0xA2,0x10, 0x6C,0x00,0x03,  0x4C,0x86,0x86 }; //867E
                                                  //A2,80, 29,A2,10, 6C,00,03, 8A, (30,7A), 8E,EF,04, 24,81, (10,35), ...

 //8BDC sometimes used to jump to code directly after BASIC-starter (rablo-rulett_loader.ted)
 //(leading to A7CC using zp $14..$15 as address):
 static const unsigned char ROM_JumpAfterBASICcmd[] = { 0x6C, 0x14, 0x00 }; //{ 0x4C, 0x0D, 0x10 };

 static const unsigned char ROM_SoundCode1[] = { //B861
  0xA6,0x80, 0xE0,0x02, 0xD0,0x01, 0xCA, 0x48, 0xC0,0x00, 0xD0,0x07, 0xC9,0x00, 0xD0,0x03, 0xC8,
  0xD0,0x0F, 0x98, 0x48, 0xEA,0xEA,0xEA,/*20,0xC0,0x8C*/ 0xBD,0xFE,0x04, 0x1D,0xFC,0x04, 0xD0,0xF5,
  0x68, 0xA8, 0x98, 0x49,0xFF, 0x18, 0x69,0x01, 0x78,  0x9D,0xFC,0x04, 0x68, 0x49,0xFF, 0x69,0x00,
  0x9D,0xFE,0x04, 0xA5,0x7E, 0x9D,0x0E,0xFF, 0xBD,0xB8,0xB8, 0xAA,
  0xBD,0x10,0xFF, 0x29,0xFC, 0x05,0x7F, 0x9D,0x10,0xFF, 0xA6,0x80, 0xBD,0xBA,0xB8, 0x0D,0x11,0xFF };

 static const unsigned char ROM_SoundCode2[] = { 0x8D,0x11,0xFF, 0x58, 0x60,  0x4C,0x1C,0x99,   0x02,0x00,0x10,0x20,0x40 }; //B8B0

 static const unsigned char ROM_SoundCode3[] = { //B8C0
  0xE0,0x09, 0xB0,0xF1, 0x86,0x80, 0xAD,0x11,0xFF, 0x29,0xF0, 0x05,0x80, 0x8D,0x11,0xFF, 0x60
 };

 static const unsigned char ROM_IRQBRKstartCode2[] = { //CE00
  0xBA, 0xBD,0x04,0x01, 0x29,0x10, 0xEA,0xEA,/*0xD0,0x03,*/ 0x6C,0x14,0x03,  0x6C,0x16,0x03 };

 static const unsigned char ROM_IRQBRKstartCode3[] = { //CE2E  //originally CFBF->CEF0 A3-A4-A5 software-counter
  0xAD,0x09,0xFF, 0x29,0x02, 0xF0,0x28, 0x8D,0x09,0xFF, 0x2C,0x0B,0xFF, 0xA9,0xCC, 0x50,0x1B //};
   , 0x6C,0x12,0x03, 0x20,0xF0,0xCE,/*0x20,0xBF,0xCF*//*0xEA,0xEA,0xEA,*/ 0x20,0xCD,0xCE };

 static const unsigned char ROM_IRQBRKstartCode4[] = { 0xA9,0xA1, 0x8D,0x0B,0xFF, 0x4C,0xBE,0xFC }; //CE58

 static const unsigned char ROM_SoundCode[] = { //CECD
  0xA2,0x01, 0xBD,0xFC,0x04, 0x1D,0xFE,0x04, 0xF0,0x13, 0xFE,0xFC,0x04, 0xD0,0x0E, 0xFE,0xFE,0x04, 0xD0,0x09,
  0xBD,0xEE,0xCE, 0x2D,0x11,0xFF, 0x8D,0x11,0xFF, 0xCA, 0x10,0xE2, 0x60,  0xEF,0x9F };

 static const unsigned char ROM_SoftwareCounter[] = { //CEF0 A3..A5 software counter
  0xE6,0xA5, 0xD0,0x06, 0xE6,0xA4, 0xD0,0x02, 0xE6,0xA3, 0x38, 0xA5,0xA5, 0xE9,0x01, 0xA5,0xA4, 0xE9,0x1A, 0xA5,0xA3, 0xE9,0x4F,
  0x90,0x08, 0xA2,0x00, 0x86,0xA3, 0x86,0xA4, 0x86,0xA5, 0x60/*fake*/ };

 static const unsigned char ROM_F3D2[] = { //$F3D2, for Csabo's rockstar_ate_my_border.ted
  0x07, 0x06, 0x0a, 0x07, 0x06, 0x04, 0x05, 0x05, 0x47, 0x52, 0x41, 0x50, 0x48, 0x49, 0x43, 0x44,
  0x4c, 0x4f, 0x41, 0x44, 0x22, 0x44, 0x49, 0x52, 0x45, 0x43, 0x54, 0x4f, 0x52, 0x59, 0x0d, 0x53,
  0x43, 0x4e, 0x43, 0x4c, 0x52, 0x0d, 0x44, 0x53, 0x41, 0x56, 0x45, 0x22, 0x52, 0x55, 0x4e, 0x0d,
  0x4c, 0x49, 0x53, 0x54, 0x0d, 0x48, 0x45, 0x4c, 0x50, 0x0d, 0x85, 0xab, 0x86, 0xaf, 0x84, 0xb0 };

 static const unsigned char ROM_F83D[] = { //$F83D, for Csabo's rockstar_ate_my_border.ted
  0x40, 0x02, 0x45, 0x03, 0xd0, 0x08, 0x40, 0x09, 0x30, 0x22, 0x45, 0x33, 0xd0, 0x08, 0x40, 0x09,
  0x40, 0x02, 0x45, 0x33, 0xd0, 0x08, 0x40, 0x09, 0x40, 0x02, 0x45, 0xb3, 0xd0, 0x08, 0x40, 0x09,
  0x00, 0x22, 0x44, 0x33, 0xd0, 0x8c, 0x44, 0x00, 0x11, 0x22, 0x44, 0x33, 0xd0, 0x8c, 0x44, 0x9a,
  0x10, 0x22, 0x44, 0x33, 0xd0, 0x08, 0x40, 0x09, 0x10, 0x22, 0x44, 0x33, 0xd0, 0x08, 0x40, 0x09 };

 static const unsigned char ROM_FakePRIMM[] = { //FBD8
  0x48,0x98,0x48,0x8A,0x48,0xBA,0xE8,0xE8,0xE8,0xE8, 0xBD,0x00,0x01, 0x85,0xBC, 0xE8, 0xBD,0x00,0x01, 0x85,0xBD, 0xE6,0xBC, 0xD0,0x02,
  0xE6,0xBD, 0xA0,0x00, 0xB1,0xBC, 0xF0,0x06, 0xEA,0xEA,0xEA,/*0x20,0xD2,0xFF,*/ 0xC8, 0xD0,0xF6, 0x98,0xBA,0xE8,0xE8,0xE8,0xE8, 0x18,
  0x65,0xBC, 0x9D,0x00,0x01, 0xA9,0x00,  0x65,0xBD, 0xE8,  0x9D,0x00,0x01, 0x68,0xAA,0x68,0xA8,0x68,0x60 };

 static const unsigned char ROM_IRQBRKstartCode[11] = { //FCB3 //Full IRQ-return (handling BRK with the same RAM vector as IRQ)
  0x48,0x8A,0x48,0x98,0x48, 0x8D,0xD0,0xFD, 0x4C,0x00,0xCE };
   //0x48,0x8A,0x48,0x98,0x48,0xBA,0xBD,0x04,0x01,0x29,0x10,0xEA,0xEA,0xEA,0xEA,0xEA,0x6C,0x14,0x03

 static const unsigned char ROM_IRQreturnCode[11] = { //FCBE //IRQ-return routine
  0xA6,0xFB,  0x9D,0xD0,0xFD, 0x68,0xA8,0x68,0xAA,0x68, 0x40 };

 //FFE4 //0x0328 //for aliens_demo.ted keycheck
 static const unsigned char ROM_FakeGET[] = { 0xA9, 0x00, 0x60 }; // { 0x4C, 0xD9, 0xEB };

 //FF4F //returns to program after the 0-terminated string following the JSR:
 static const unsigned char ROM_JmpTablePRIMM[] = { 0x4C,0xD8,0xFB };


 for (i=0x8000; i<0x10000; ++i) CPlus4->ROMbanks[i] = 0x60; //RTS (at least return if some unsupported call is made to ROM)
 //for (i=0; i<sizeof(KERNAL); ++i) CPlus4->ROMbanks[0x8000+i] = KERNAL[i]; //no complete KERNAL
 for (i=0xCE00; i<0xCE60; ++i) CPlus4->ROMbanks[i] = 0xEA;  //NOP (full IRQ-return leading to simple IRQ-return without other tasks)
 for (i=0; i<sizeof(ROM_8090); ++i) CPlus4->ROMbanks[0x8090 + i] = ROM_8090[i]; //for Csabo's rockstar_ate_my_border.ted
 for (i=0; i<sizeof(ROM_F3D2); ++i) CPlus4->ROMbanks[0xF3D2 + i] = ROM_F3D2[i]; //for Csabo's rockstar_ate_my_border.ted
 for (i=0; i<sizeof(ROM_F83D); ++i) CPlus4->ROMbanks[0xF83D + i] = ROM_F83D[i]; //for Csabo's rockstar_ate_my_border.ted
 for (i=0; i<sizeof(ROM_8800); ++i) CPlus4->ROMbanks[0x8800 + i] = ROM_8800[i]; //for Csabo's rockstar_ate_my_border.ted
 CPlus4->ROMbanks[0xFEA9]=0x00; //0xFF //needed for 6fx by Csabo (BIT)?
  for (i=0; i<sizeof(ROM_IRQBRKstartCode2); ++i) CPlus4->ROMbanks[0xCE00 + i] = ROM_IRQBRKstartCode2[i];
  for (i=0; i<sizeof(ROM_IRQBRKstartCode3); ++i) CPlus4->ROMbanks[0xCE2E + i] = ROM_IRQBRKstartCode3[i];
  for (i=0; i<sizeof(ROM_IRQBRKstartCode4); ++i) CPlus4->ROMbanks[0xCE58 + i] = ROM_IRQBRKstartCode4[i];
 for (i=0; i<sizeof(ROM_IRQBRKstartCode); ++i) CPlus4->ROMbanks[0xFCB3 + i] = ROM_IRQBRKstartCode[i];
 for (i=0; i<sizeof(ROM_IRQreturnCode); ++i) CPlus4->ROMbanks[0xFCBE + i] = ROM_IRQreturnCode[i];
 for (i=0; i<sizeof(ROM_SoundCode); ++i) CPlus4->ROMbanks[0xCECD + i] = ROM_SoundCode[i];
 for (i=0; i<sizeof(ROM_SoundCode1); ++i) CPlus4->ROMbanks[0xB861 + i] = ROM_SoundCode1[i];
 for (i=0; i<sizeof(ROM_SoundCode2); ++i) CPlus4->ROMbanks[0xB8B0 + i] = ROM_SoundCode2[i];
 for (i=0; i<sizeof(ROM_SoundCode3); ++i) CPlus4->ROMbanks[0xB8C0 + i] = ROM_SoundCode3[i]; //golf_royal.ted needs this to set volume
 for (i=0; i<sizeof(ROM_BackToBASIC); ++i) CPlus4->ROMbanks[0x867E + i] = ROM_BackToBASIC[i];
 for (i=0; i<sizeof(ROM_JmpTablePRIMM); ++i) CPlus4->ROMbanks[0xFF4F + i] = ROM_JmpTablePRIMM[i];
 for (i=0; i<sizeof(ROM_FakePRIMM); ++i) CPlus4->ROMbanks[0xFBD8 + i] = ROM_FakePRIMM[i];
 for (i=0; i<sizeof(ROM_JumpAfterBASICcmd); ++i) CPlus4->ROMbanks[0x8BDC + i] = ROM_JumpAfterBASICcmd[i];
 for (i=0; i<sizeof(ROM_FakeGET); ++i) CPlus4->ROMbanks[0xFFE4 + i] = ROM_FakeGET[i]; //for aliens_demo.ted keycheck
 for (i=0; i<sizeof(ROM_SoftwareCounter); ++i) CPlus4->ROMbanks[0xCEF0 + i] = ROM_SoftwareCounter[i]; //for aliens_demo.ted keycheck

 CPlus4->ROMbanks[0xFFFF] = 0xFC; CPlus4->ROMbanks[0xFFFE] = 0xB3; //ROM IRQ-vector */

 //copy KERNAL & BASIC ROM contents into the RAM under them? (So PTEDs that don't select bank correctly will work better.)
 for (i=0x8000; i<0x10000; ++i) CPlus4->RAMbank[i] = CPlus4->ROMbanks[i];
}


void cRTED_initMem (cRTED_CPlus4instance* CPlus4) {
 static int i; //set default values that normally KERNEL ensures after startup/reset (only TED-playback related):

 static const unsigned charZeroPageInitValues[256] = { //e.g. 6fx, xplode_man, pogo_pete, etc. requires some zeropage startup-values
  0x0f, 0x58, 0x00, 0x00,  0x00, 0x00, 0x00, 0x22,  0x22, 0x00, 0x00, 0x00,  0x00, 0xff, 0x00, 0x00, //$00..
  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x19, 0x16,  0x00, 0x05, 0xf9, 0xfc,  0x00, 0x00, 0x00, 0x00, //$10..
  0x00, 0x00, 0xf9, 0xfc,  0x44, 0xa4, 0x00, 0x00,  0x00, 0x00, 0x00, 0x01,  0x10, 0x03, 0x10, 0x03, //$20..
  0x10, 0x03, 0x10, 0x00,  0xfd, 0xfe, 0xfc, 0x00,  0xfd, 0x00, 0xff, 0x00,  0x10, 0x00, 0x00, 0x00, //$30..
  0x00, 0x00, 0x10, 0x00,  0x00, 0x00, 0x00, 0x24,  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, //$40..
  0xff, 0xff, 0x00, 0x00,  0x4c, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,  0x00, 0xfc, 0x00, 0x00, //$50..
  0x00, 0x05, 0xf9, 0xfc,  0x19, 0x00, 0x20, 0x00,  0x00, 0x80, 0x00, 0x00,  0x00, 0x02, 0x00, 0x00, //$60..
  0x00, 0x05, 0x01, 0x00,  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,  0xb0, 0x07, 0x00, 0x00, //$70..
  0x00, 0x00, 0x00, 0x00,  0x00, 0x36, 0x10, 0x00,  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, //$80..
  0x00, 0xff, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,  0x00, 0x03, 0x80, 0x00,  0x00, 0x00, 0x00, 0x06, //$90..
  0x00, 0x00, 0x00, 0x00,  0x00, 0x26, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, //$A0..
  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,  0xeb, 0xf2, 0x00, 0x00,  0x72, 0x86, 0x00, 0x00, //$B0..
  0x00, 0x00, 0x00, 0x05,  0x05, 0x00, 0x40, 0x00,  0xc8, 0x0c, 0x00, 0x00,  0x00, 0x05, 0x0a, 0x00, //$C0..
  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, //$D0..
  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0xc8, 0x08,  0x00, 0x00, 0xff, 0x00, //$E0..
  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x20 }; //$F0..

 static const unsigned char CodeAt473[] = { //present at Plus4 startup
  0xE6,0x3B, 0xD0,0x02, 0xE6,0x3C, 0x78, 0x8D,0x3F,0xFF, 0xA0,0x00, 0xB1,0x3B, 0x8D,0x3E,0xFF,
  0x58, 0xC9,0x3A, 0xB0,0x0A, 0xC9,0x20, 0xF0,0xE6, 0x38, 0xE9,0x30, 0x38, 0xE9,0xD0, 0x60,
  0x8D,0x9C,0x04, 0x78, 0x8D,0x3F,0xFF, 0xB1,0x00, 0x8D,0x3E,0xFF, 0x58, 0x60 };

 static const unsigned char CodeAt4A5[] = { //present at Plus4 startup
  0x78, 0x8D,0x3F,0xFF, 0xB1,0x3B, 0x8D,0x3E,0xFF, 0x58, 0x60,  0x78, 0x8D,0x3F,0xFF, 0xB1,0x22, 0x8D,0x3E,0xFF, 0x58, 0x60,
  0x78, 0x8D,0x3F,0xFF, 0xB1,0x24, 0x8D,0x3E,0xFF, 0x58, 0x60,  0x78, 0x8D,0x3F,0xFF, 0xB1,0x6F, 0x8D,0x3E,0xFF, 0x58, 0x60,
  0x78, 0x8D,0x3F,0xFF, 0xB1,0x5F, 0x8D,0x3E,0xFF, 0x58, 0x60,  0x78, 0x8D,0x3F,0xFF, 0xB1,0x64, 0x8D,0x3E,0xFF, 0x58, 0x60 };

 static const unsigned char TEDdefaults[] = { //e.g. club_info_136 expects $EE at $FF19
 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x1b, 0x08,  0xff, 0x79, 0xa2, 0xa1,  0xfc, 0xc8, 0x00, 0x00,
 0xc0, 0x00, 0xc4, 0xd1,  0x08, 0xf1, 0xdb, 0xf5,  0xf7, 0xee, 0xfd, 0xe8,  0xff, 0x01, 0xc8, 0xd7,
 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0xff, 0xff };


 for (i=0; i<256; ++i) cRTED_writeMemCPlus4( CPlus4, i, charZeroPageInitValues[i]);
 /* //7501/8501 port (xplode_man_title.ted requires nonzero):
 cRTED_writeMemCPlus4( CPlus4, 0x0000, 0x0F ); cRTED_writeMemCPlus4( CPlus4, 0x0001, 0x58 );
 //8BDC sometimes used to jump to code directly after BASIC-starter (rablo-rulett_loader.ted)
 //(leading to A7CC using zp $14..$15 as address):
 cRTED_writeMemCPlus4( CPlus4, 0x0015, 0x10 ); cRTED_writeMemCPlus4( CPlus4, 0x0014, 0x0D );
 //mozaik_hrh.ted needs this because it sets $FF11 by zeropage $17 value:
 cRTED_writeMemCPlus4( CPlus4, 0x0017, 0x16 );
 cRTED_writeMemCPlus4( CPlus4, 0x003C, 0x10 ); cRTED_writeMemCPlus4( CPlus4, 0x007D, 0x07 ); //6fx by Csabo
 //reset sets it to $19, but RUN sets it to $10, needed for pogo_pete.ted at code-address $3A79:
 cRTED_writeMemCPlus4( CPlus4, 0x0064, 0x10 );
 cRTED_writeMemCPlus4( CPlus4, 0x0072, 0x01 ); cRTED_writeMemCPlus4( CPlus4, 0x009F, 0x06 ); //6fx by Csabo
 cRTED_writeMemCPlus4( CPlus4, 0x00C6, 0x40 ); //uranus.ted checks keypress to exit by this at $1a79 */

 //in ROM address $8683 'error message output' (next line $8686):
 cRTED_writeMemCPlus4( CPlus4, 0x0301, 0x86 ); cRTED_writeMemCPlus4( CPlus4, 0x0300, 0x86 );
 //inside IRQ, points to next line CE42, c16_shit.ted modifies:
 cRTED_writeMemCPlus4( CPlus4, 0x0313, 0xCE ); cRTED_writeMemCPlus4( CPlus4, 0x0312, 0x42 );
 cRTED_writeMemCPlus4( CPlus4, 0x0315, 0xCE ); cRTED_writeMemCPlus4( CPlus4, 0x0314, 0x0E ); //IRQ
 cRTED_writeMemCPlus4( CPlus4, 0x0319, 0xFC/*0xEF*/ ); cRTED_writeMemCPlus4( CPlus4, 0x0318, 0xC8/*0x53*/ ); //NMI
 cRTED_writeMemCPlus4( CPlus4, 0x0325, 0xEC ); cRTED_writeMemCPlus4( CPlus4, 0x0324, 0x4B ); //FFD2 output (print usually)

 for (i=0; i<sizeof(CodeAt473); ++i) CPlus4->RAMbank[0x473 + i] = CodeAt473[i];
 for (i=0; i<sizeof(CodeAt4A5); ++i) CPlus4->RAMbank[0x4A5 + i] = CodeAt4A5[i];

 for (i=0xFD00; i<0xFF40; ++i) CPlus4->IObankRD[i] = CPlus4->IObankWR[i] = 0; //initialize the whole IO area for a known base-state

 for (i=0; i<sizeof(TEDdefaults); ++i) CPlus4->IObankRD[0xFF00+i] = TEDdefaults[i];
 //PTED: rasterrow: any value <= $FF, IRQ:enable later if there is raster-interrupt-timingsource:
 CPlus4->IObankRD[0xFF09] = CPlus4->IObankWR[0xFF09] = 0x00;
 CPlus4->IObank[0xFF0B] = 0xA1; CPlus4->IObank[0xFF0A] = 0xA2;
 //Imitate keyboard/joy port, some tunes check if buttons are not pressed (for e.g. kane.ted, arkanoid.ted):
 CPlus4->IObank[0xFD30]=0xFF; CPlus4->IObank[0xFF08]=0xFF;
 CPlus4->IObank[0xFF19]=0xEE; //for club_info_136.ted
 //CPlus4->IObankRD[0xFF11] = 0x38; //CPlus4 has volumes zeroed when reset
}
