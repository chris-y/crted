
//CPlus4 machine emulation of cRTED by Hermit


#include "../libcRTED.h"

#include "MEM.c"
#include "CPU.c"
#include "TED.c"



cRTED_CPlus4instance* cRTED_createCPlus4 (cRTED_CPlus4instance* CPlus4, unsigned short samplerate) { //init a basic PAL CPlus4 instance
 enum MachineConstants { CPLUS4_CRYSTAL_CLOCK=17734470, CPLUS4_CPU_CLOCK_PRESCALE=20,
                         CPLUS4_DOUBLE_CPU_CLOCK_PRESCALE = (CPLUS4_CPU_CLOCK_PRESCALE/2),
                         CPLUS4_DOUBLE_CPU_CLOCK = (CPLUS4_CRYSTAL_CLOCK / CPLUS4_DOUBLE_CPU_CLOCK_PRESCALE), //1773447 Hz
                         TED_AUDIO_CLOCK_PRESCALE = 4, TED_AUDIO_CLOCK_DOUBLE_PRESCALE = (TED_AUDIO_CLOCK_PRESCALE*2) };
 static const double TED_AUDIO_CLOCK = CPLUS4_DOUBLE_CPU_CLOCK / TED_AUDIO_CLOCK_DOUBLE_PRESCALE;

 if(samplerate) CPlus4->SampleRate = samplerate;
 else CPlus4->SampleRate = samplerate = 44100; //shifting (multiplication) enhances SampleClockRatio precision:
 CPlus4->SampleClockRatio = ((samplerate<<CRTED_FRACTIONAL_BITS) * TED_AUDIO_CLOCK_DOUBLE_PRESCALE) / CPLUS4_DOUBLE_CPU_CLOCK;
 CPlus4->SampleClockRatioReciproc = TED_AUDIO_CLOCK / CPlus4->SampleRate; //round( TED_AUDIO_CLOCK / CPlus4->SampleRate );
 CPlus4->CPU.CPlus4 = CPlus4;
 CPlus4->IObank = CPlus4->IObankRD = CPlus4->RAMbank;
 cRTED_createTEDchip ( CPlus4, &CPlus4->TED, 0xFF00 ); //default CPlus4 setup with only 1 TED and 2 CIAs and 1 VIC
 cRTED_setROMcontent ( CPlus4 );
 cRTED_initCPlus4(CPlus4);
 return CPlus4;
}



void cRTED_setCPlus4 (cRTED_CPlus4instance* CPlus4) {   //set hardware-parameters (Models, TEDs) for playback of loaded TED-tune
 enum MachineConstants { CPLUS4_CRYSTAL_CLOCK=17734470, CPLUS4_CPU_CLOCK_PRESCALE=20,
                         CPLUS4_DOUBLE_CPU_CLOCK_PRESCALE = (CPLUS4_CPU_CLOCK_PRESCALE/2),
                         CPLUS4_DOUBLE_CPU_CLOCK = (CPLUS4_CRYSTAL_CLOCK / CPLUS4_DOUBLE_CPU_CLOCK_PRESCALE), //1773447 Hz
                         TED_AUDIO_CLOCK_PRESCALE = 4, TED_AUDIO_CLOCK_DOUBLE_PRESCALE = (TED_AUDIO_CLOCK_PRESCALE*2) };
 //Hz  PAL crystal-osc.frequency: 17.734470MHz, divided by 20 -> mainboard-bus clock: 0.886MHz
 enum CPlus4clocks { CPlus4_PAL_CPUCLK=886723/*.5*/, CPlus4_NTSC_CPUCLK=715909 };
 //456 pixelclocks //horizontal:0..455, 1x clockspeed:400..344 (line 0..204),
 //                                     2x clockspeed: line205..311(or all if blank) (except hor. 304..344)
 enum CPlus4scanlines { CPlus4_PAL_SCANLINES = 312, CPlus4_NTSC_SCANLINES = 262 };
 enum CPlus4scanlineCycles { CPlus4_PAL_SCANLINE_CYCLES = 57, CPlus4_NTSC_SCANLINE_CYCLES = 45 }; //?
 static const double TED_AUDIO_CLOCK = CPLUS4_DOUBLE_CPU_CLOCK / TED_AUDIO_CLOCK_DOUBLE_PRESCALE;

 static const unsigned int CPUspeeds[] = { CPlus4_PAL_CPUCLK, CPlus4_NTSC_CPUCLK };
 static const unsigned short ScanLines[] = { CPlus4_PAL_SCANLINES, CPlus4_NTSC_SCANLINES };
 static const unsigned char ScanLineCycles[] = { CPlus4_PAL_SCANLINE_CYCLES, CPlus4_NTSC_SCANLINE_CYCLES };

 CPlus4->VideoStandard = 0; //PAL
 if (CPlus4->SampleRate==0) CPlus4->SampleRate = 44100;
 CPlus4->CPUfrequency = CPUspeeds[CPlus4->VideoStandard]; //shifting (multiplication) enhances SampleClockRatio precision:
 CPlus4->SampleClockRatio = ((CPlus4->SampleRate<<CRTED_FRACTIONAL_BITS) * TED_AUDIO_CLOCK_DOUBLE_PRESCALE) / CPLUS4_DOUBLE_CPU_CLOCK;
 CPlus4->SampleClockRatioReciproc = TED_AUDIO_CLOCK / CPlus4->SampleRate; //round( TED_AUDIO_CLOCK / CPlus4->SampleRate );

 CPlus4->TED.RasterLines = ScanLines[CPlus4->VideoStandard];
 CPlus4->TED.RasterRowCycles = ScanLineCycles[CPlus4->VideoStandard];
 CPlus4->FrameCycles = CPlus4->TED.RasterLines * CPlus4->TED.RasterRowCycles; //default for 1x speed tune with TED Vertical-blank timing

 CPlus4->TED.ChipModel = 8360; //7360; //TEDmodel
}



void cRTED_initCPlus4 (cRTED_CPlus4instance* CPlus4) { //CPlus4 Reset
 enum MachineConstants { TED_AUDIO_CLOCK_PRESCALE = 4 };

 cRTED_initTEDchip( &CPlus4->TED );
 cRTED_initMem(CPlus4);
 cRTED_initCPU( &CPlus4->CPU, (cRTED_readMemCPlus4(CPlus4,0xFFFD)<<8) + cRTED_readMemCPlus4(CPlus4,0xFFFC) );
 CPlus4->IRQ = CPlus4->NMI = 0;
 CPlus4->SampleCycleCnt=CPlus4->SampleCycleCnt2=0; CPlus4->ROMenabled=1; CPlus4->CPUspeedMul=TED_AUDIO_CLOCK_PRESCALE;
}



cRTED_Output cRTED_emulateCPlus4 (cRTED_CPlus4instance *CPlus4) {
 enum MachineConstants { TED_AUDIO_CLOCK_PRESCALE = 4 };

 enum SoundEmulationParameters {
  FRACTIONAL_MUL = (1 << CRTED_FRACTIONAL_BITS), //4096 $1000
  FRACTIONAL_AND = (FRACTIONAL_MUL-1), //4095 $0FFF
  INTEGER_AND = (-FRACTIONAL_MUL), //$BFFF
  RESAMPLEBUFFER_SIZE = 16, //entries
  RESAMPLEBUFFER_SIZE_MUL = (RESAMPLEBUFFER_SIZE<<CRTED_FRACTIONAL_BITS),
  SINCWINDOW_PERIODS = 12, //half-sines
  SINCPERIOD_SAMPLES = 128, //entries
  SINCWINDOW_SIZE = (SINCWINDOW_PERIODS*SINCPERIOD_SAMPLES), //1536
  SINCPERIOD_BITS = 7, //log2(SINCPERIOD_SAMPLES)
  SINCWINDOW_RESOLUTION = 11, //bits
  SINCWINDOW_VALUE_MAX = 2048, // (2**SINCWINDOW_RESOLUTION)
  PWM_HIGHPASS_BITSHIFT = 11,
  INTEGRATOR_LOWPASS_CUTOFF = 15, //0..16
  //how much shift=division made by output AC-decoupling capacitor (10uF with cca 5k load ~ 2.5Hz lower cutoff?):
  OUTPUT_HIGHPASS_BITSHIFT = 10,
  OUTPUT_LOWPASS_CUTOFF = 15, //0..16
  //Transistor emitter-follower output distortion-parameters:
  NONDISTORTED_MIDDLEVALUE = 0x8000,
  DISTORTION_SIGNALRANGE = 65536,
  TRANSISTOR_AMPLIFIER_OUTPUT_RESISTOR = (2*240), //Ohm //common emitter amplifier's output load-resistors
  //TRANSISTOR_AMPLIFIER_INPUT_RESISTOR = 1000, //Ohm
  //TRANSISTOR_INTRINSIC_BASE_RESISTANCE = 50, //Ohm //according to datasheet
  //TRANSISTOR_CURRENT_AMPLIFICATION = 300,    //determines how base and series resistances affect output distortion
  //TRANSISTOR_INTRINSIC_RESISTANCE_MAX = 10, //Ohm
  //variation of intrinsic emitter resistance through the entire voltage swing
  //(calculated by output current at min/max voltages, r'e=25mV/Ie=4..7Ohm):
  TRANSISTOR_INTRINSIC_RESISTANCE_VARIATION = 2, //deltaOhm
  //resistance ratio for emitter follower distortion caused by voltage divider between current-dependent intrinsic emitter resistance
  DISTORTION_RATIO = (TRANSISTOR_AMPLIFIER_OUTPUT_RESISTOR * TRANSISTOR_INTRINSIC_RESISTANCE_VARIATION * 65536)
 };


 static unsigned char i, InstructionCycles; static char ResampleBufWritePos;
 static char InstrCycleCnt=0; static short MachineNoiseSampleCounter=0;

 static int tmp, PWMhighPass=0, IntegratorLowPass=0, MachineOutputHighPass=0, MachineOutputLowPass=0; //, max=0,min=65535;
 static unsigned int TransistorCurrent;
 static int ResampleBufPos=0, NextResampleBufPos=FRACTIONAL_MUL, SincWindowPos;
 static cRTED_Output Output; static signed int TEDwavOutput;
 static signed int ResampleBuffer[RESAMPLEBUFFER_SIZE]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

 static const signed short Sinc2048[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
  -2,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-2,-2,-2,-2,-2,-2,
  -2,-2,-2,-2,-2,-1,-1,-1,-1,-1,-1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,3,3,3,3,4,4,4,5,5,5,6,6,7,7,7,
  8,8,8,9,9,10,10,10,11,11,11,12,12,13,13,13,14,14,15,15,15,16,16,16,17,17,17,18,18,18,19,19,19,20,
  20,20,20,21,21,21,21,22,22,22,22,22,22,23,23,23,23,23,23,23,23,23,23,23,23,23,23,22,22,22,22,22,22,
  21,21,21,20,20,20,19,19,18,18,18,17,17,16,15,15,14,14,13,12,12,11,10,9,9,8,7,6,5,4,3,2,1,0,0,-1,-2,
  -3,-4,-5,-6,-7,-8,-9,-10,-11,-13,-14,-15,-16,-17,-19,-20,-21,-22,-24,-25,-26,-27,-29,-30,-31,-32,-34,
  -35,-36,-37,-39,-40,-41,-42,-44,-45,-46,-47,-48,-49,-51,-52,-53,-54,-55,-56,-57,-58,-59,-60,-61,-62,-62,
  -63,-64,-65,-65,-66,-67,-67,-68,-69,-69,-69,-70,-70,-71,-71,-71,-71,-72,-72,-72,-72,-72,-72,-71,-71,-71,
  -71,-70,-70,-70,-69,-69,-68,-67,-67,-66,-65,-64,-63,-62,-61,-60,-59,-58,-56,-55,-54,-52,-51,-49,-48,-46,
  -44,-42,-41,-39,-37,-35,-33,-31,-29,-26,-24,-22,-20,-17,-15,-12,-10,-7,-5,-2,0,2,5,8,10,13,16,19,22,25,28,
  31,34,37,40,43,46,49,52,55,58,61,64,67,70,73,76,79,82,85,88,91,94,97,100,103,106,109,111,114,117,119,122,125,
  127,130,132,134,137,139,141,143,145,147,149,151,153,154,156,157,159,160,161,163,164,165,165,166,167,168,168,168,
  169,169,169,169,169,169,168,168,167,166,165,165,163,162,161,160,158,156,155,153,151,148,146,144,141,138,136,133,
  130,127,123,120,116,113,109,105,101,97,93,89,84,80,75,70,65,60,55,50,45,40,34,29,23,17,11,5,0,-6,-12,-18,-24,-30,
  -37,-43,-50,-56,-63,-69,-76,-83,-89,-96,-103,-110,-117,-123,-130,-137,-144,-151,-158,-164,-171,-178,-185,-191,-198,
  -205,-211,-218,-224,-231,-237,-244,-250,-256,-262,-268,-274,-280,-285,-291,-296,-302,-307,-312,-317,-322,-327,-331,
  -336,-340,-344,-348,-352,-355,-359,-362,-365,-368,-370,-373,-375,-377,-379,-381,-382,-383,-384,-385,-385,-386,-386,
  -385,-385,-384,-383,-382,-380,-379,-377,-374,-372,-369,-366,-362,-358,-354,-350,-346,-341,-336,-330,-324,-318,-312,
  -305,-299,-291,-284,-276,-268,-259,-251,-242,-232,-223,-213,-203,-192,-181,-170,-159,-147,-135,-123,-110,-97,-84,-71,
  -57,-43,-29,-14,0,15,30,45,61,77,94,110,127,144,162,179,197,215,233,251,270,289,308,327,346,366,385,405,425,446,466,
  486,507,528,549,570,591,612,633,655,676,698,719,741,763,784,806,828,850,872,894,916,938,959,981,1003,1025,1047,1068,
  1090,1112,1133,1155,1176,1197,1218,1239,1260,1281,1302,1322,1343,1363,1383,1403,1422,1442,1461,1480,1499,1518,1536,
  1554,1572,1590,1608,1625,1642,1658,1675,1691,1707,1722,1738,1753,1767,1782,1796,1809,1822,1835,1848,1860,1872,1884,
  1895,1906,1916,1926,1936,1945,1954,1963,1971,1979,1986,1993,2000,2006,2012,2017,2022,2026,2030,2034,2037,2040,2042,
  2044,2046,2047,2047,2048,2047,2047,2046,2044,2042,2040,2037,2034,2030,2026,2022,2017,2012,2006,2000,1993,1986,1979,
  1971,1963,1954,1945,1936,1926,1916,1906,1895,1884,1872,1860,1848,1835,1822,1809,1796,1782,1767,1753,1738,1722,1707,
  1691,1675,1658,1642,1625,1608,1590,1572,1554,1536,1518,1499,1480,1461,1442,1422,1403,1383,1363,1343,1322,1302,1281,
  1260,1239,1218,1197,1176,1155,1133,1112,1090,1068,1047,1025,1003,981,959,938,916,894,872,850,828,806,784,763,741,719,
  698,676,655,633,612,591,570,549,528,507,486,466,446,425,405,385,366,346,327,308,289,270,251,233,215,197,179,162,144,
  127,110,94,77,61,45,30,15,0,-14,-29,-43,-57,-71,-84,-97,-110,-123,-135,-147,-159,-170,-181,-192,-203,-213,-223,-232,
  -242,-251,-259,-268,-276,-284,-291,-299,-305,-312,-318,-324,-330,-336,-341,-346,-350,-354,-358,-362,-366,-369,-372,
  -374,-377,-379,-380,-382,-383,-384,-385,-385,-386,-386,-385,-385,-384,-383,-382,-381,-379,-377,-375,-373,-370,-368,
  -365,-362,-359,-355,-352,-348,-344,-340,-336,-331,-327,-322,-317,-312,-307,-302,-296,-291,-285,-280,-274,-268,-262,
  -256,-250,-244,-237,-231,-224,-218,-211,-205,-198,-191,-185,-178,-171,-164,-158,-151,-144,-137,-130,-123,-117,-110,
  -103,-96,-89,-83,-76,-69,-63,-56,-50,-43,-37,-30,-24,-18,-12,-6,0,5,11,17,23,29,34,40,45,50,55,60,65,70,75,80,84,89,
  93,97,101,105,109,113,116,120,123,127,130,133,136,138,141,144,146,148,151,153,155,156,158,160,161,162,163,165,165,
  166,167,168,168,169,169,169,169,169,169,168,168,168,167,166,165,165,164,163,161,160,159,157,156,154,153,151,149,
  147,145,143,141,139,137,134,132,130,127,125,122,119,117,114,111,109,106,103,100,97,94,91,88,85,82,79,76,73,70,67,
  64,61,58,55,52,49,46,43,40,37,34,31,28,25,22,19,16,13,10,8,5,2,0,-2,-5,-7,-10,-12,-15,-17,-20,-22,-24,-26,-29,-31,
  -33,-35,-37,-39,-41,-42,-44,-46,-48,-49,-51,-52,-54,-55,-56,-58,-59,-60,-61,-62,-63,-64,-65,-66,-67,-67,-68,-69,
  -69,-70,-70,-70,-71,-71,-71,-71,-72,-72,-72,-72,-72,-72,-71,-71,-71,-71,-70,-70,-69,-69,-69,-68,-67,-67,-66,-65,
  -65,-64,-63,-62,-62,-61,-60,-59,-58,-57,-56,-55,-54,-53,-52,-51,-49,-48,-47,-46,-45,-44,-42,-41,-40,-39,-37,-36,
  -35,-34,-32,-31,-30,-29,-27,-26,-25,-24,-22,-21,-20,-19,-17,-16,-15,-14,-13,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,
  0,0,1,2,3,4,5,6,7,8,9,9,10,11,12,12,13,14,14,15,15,16,17,17,18,18,18,19,19,20,20,20,21,21,21,22,22,22,22,22,22,23,
  23,23,23,23,23,23,23,23,23,23,23,23,23,22,22,22,22,22,22,21,21,21,21,20,20,20,20,19,19,19,18,18,18,17,17,17,16,16,
  16,15,15,15,14,14,13,13,13,12,12,11,11,11,10,10,10,9,9,8,8,8,7,7,7,6,6,5,5,5,4,4,4,3,3,3,3,2,2,2,1,1,1,1,0,0,0,0,
  0,0,0,0,0,0,-1,-1,-1,-1,-1,-1,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,
  -3,-3,-3,-3,-3,-3,-3,-3,-3,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
 };

 static unsigned char MachineNoise_1wave_S8 [] = {  //array generated from binary by bin2array 1.2 (made in 2023 by Hermit)
  0xEA,0xE3,0x35,0x3C,0x31,0x26,0x4B,0x1B,0x3C,0x31,0x22,0x31,0x35,0x1B,0x22,0x2A,
  0x1B,0x4F,0x2A,0x26,0x4F,0x22,0x3C,0x35,0x35,0x38,0x44,0x26,0x40,0x3C,0x2A,0x40,
  0x35,0x22,0x3C,0x26,0x38,0x35,0x2A,0x2D,0x40,0x26,0x35,0x40,0x1E,0x3C,0x2D,0x31,
  0x3C,0x3C,0x17,0x44,0x17,0x3C,0x2D,0x38,0x31,0x2D,0x35,0x35,0x31,0x2A,0x2A,0x38,
  0x26,0x2A,0x2D,0x26,0x38,0x26,0x1B,0x35,0x22,0x35,0x3C,0x26,0x1B,0x35,0x1B,0x35,
  0x35,0x1E,0x31,0x17,0x22,0x2D,0x2D,0x17,0x38,0x0C,0x26,0x35,0x0C,0x35,0x22,0x17,
  0x2D,0x2A,0x2A,0x2D,0x1E,0x2A,0x22,0x35,0x10,0x3C,0x13,0x26,0x35,0x13,0x2A,0x2A,
  0x0F,0x2D,0x22,0x1B,0x26,0x1B,0x26,0x35,0x22,0x1B,0x1E,0x26,0x1B,0x38,0x13,0x2A,
  0x22,0x17,0x22,0x35,0x08,0x35,0x13,0x26,0x3C,0x38,0x22,0x47,0x31,0x2A,0x4B,0x1E,
  0x47,0x44,0x31,0x4B,0x44,0x1B,0x4F,0x2D,0x2D,0x3C,0x31,0x1E,0x40,0x2D,0x1B,0x52,
  0x22,0x31,0x44,0x13,0x3C,0x3C,0x0C,0x4B,0x1E,0x1E,0x40,0x35,0x1B,0x47,0x17,0x35,
  0x40,0x26,0x2D,0x3C,0x1B,0x47,0x31,0x22,0x3C,0x22,0x1E,0x26,0x22,0x13,0x2A,0x04,
  0x26,0x38,0x10,0x1E,0x22,0x0C,0x2A,0x31,0x04,0x2D,0x1B,0x10,0x2D,0x0C,0x1E,0x10,
  0x1B,0x10,0x2D,0x0F,0x22,0x1B,0x0F,0x1B,0x26,0x04,0x22,0x13,0x0C,0x2A,0x10,0x10,
  0x2D,0xFD,0x2D,0x22,0x17,0x10,0x1B,0x04,0x1E,0x1E,0x0C,0x1B,0x0F,0x17,0x1E,0x17,
  0x04,0x22,0xF2,0x2A,0x0F,0x1E,0x04,0x2A,0xF2,0x26,0x1E,0x17,0x22,0x0F,0x0C,0x26,
  0x10,0x13,0x17,0x13,0x04,0x35,0x01,0x1B,0x1B,0x0C,0x2A,0x17,0x1B,0x08,0x1B,0x10,
  0x2A,0x0C,0x13,0x17,0x0C,0x0C,0x2A,0x01,0x1B,0x10,0x08,0x17,0x22,0xF9,0x1E,0x08,
  0x0C,0x2A,0x08,0x10,0x10,0x04,0x0C,0x17,0x01,0x08,0x0F,0xF9,0x13,0x17,0x08,0x13,
  0x08,0x0C,0x22,0x13,0xD4,0xA0,0xB3,0xBA,0x9C,0xB6,0xC1,0x8D,0xCD,0xAB,0x9C,0xC9,
  0xC1,0xBA,0xEA,0xC5,0xDF,0xDF,0xD8,0xDF,0xEE,0xD8,0xDF,0xDF,0xD4,0xEA,0xDF,0xE3,
  0xE3,0xDF,0xD8,0xE3,0xBE,0xBE,0xFD,0xB6,0xDF,0xF5,0xEE,0xEA,0xFD,0xD4,0xEE,0xF9,
  0xCD,0x08,0xD4,0xEE,0xE3,0xEE,0xD8,0xF9,0xD4,0xEA,0xE7,0xB3,0xF2,0xE7,0xC9,0xF2,
  0xEE,0xE3,0xF9,0xEA,0xDB,0x04,0xDF,0xEA,0xF5,0xDF,0xE7,0x01,0xD8,0xE3,0xEA,0xCD,
  0xF9,0xC5,0xBE,0xEE,0xCD,0xD0,0xF5,0xE7,0xF9,0xF2,0xEA,0xDF,0x08,0xD0,0x04,0xDF,
  0xE7,0xF5,0xE7,0xEA,0xEA,0xF5,0xDB,0xEE,0xB3,0xD8,0xF6,0xC9,0xE7,0xF2,0xE3,0xF9,
  0xE3,0xEE,0xF2,0xF2,0xD8,0xF2,0xDF,0xDB,0xFD,0xD8,0xF5,0xEE,0xE3,0xEE,0xD4,0xC9,
  0xDF,0xF9,0xCD,0xF5,0xEE,0xF2,0xF2,0xF9,0xE3,0xF5,0xEA,0xF2,0xF2,0xDB,0xEA,0xEA,
  0xEA,0xF2,0xEA,0xDF,0xEE,0xD4,0xB3,0xF9,0xD4,0xC1,0xFD,0xE7,0xEA,0xFD,0xCD,0x01,
  0xE7,0xEA,0xEA,0xF5,0xE3,0xF5,0xEE,0xE7,0x01,0xDF,0xEA,0xE3,0xC9,0xD4,0xF5,0xCD,
  0xD8,0x08,0xD8,0x08,0xEA,0xEA,0xF5,0xF2,0xE7,0xF9,0xEA,0xE3,0x01,0xF2,0xEE,0x04,
  0xEA,0xEA,0xEE,0xB6,0xFD,0xDF,0xE3,0xF2,0xF9,0xF9,0xFD,0xF2,0xF5,0xF2,0xF9,0xE3,
  0xFD,0xE3,0xF5,0xF2,0xEE,0xF5,0xF5,0xEA,0xF9,0xD0,0xBE,0x01,0xD0,0xD8,0x01,0xE3,
  0xF5,0x08,0xEA,0xF5,0x04,0xE7,0x04,0xF5,0xE3,0xF9,0xEE,0xE7,0xFD,0xE7,0xE7,0xF2,
  0xC5,0xDB,0x08,0xCD,0xF5,0x01,0xEE,0xF5,0x04,0xE7,0x08,0xFD,0xEE,0x0C,0xEA,0xEA,
  0x04,0xEE,0xF2,0xF9,0xEE,0xE7,0xF2,0xB6,0xF9,0xE7,0xD8,0xF5,0xFD,0xEE,0x01,0xEE,
  0xF5,0x01,0xEA,0xF9,0x01,0xE3,0x01,0xFD,0xEE,0x01,0xF2,0xF9,0xEA,0xDF,0xBA,0x0F,
  0xD4,0xDF,0x0C,0xEE,0x04,0xF9,0xF2,0xF2,0x04,0xEE,0xF5,0x04,0xE3,0x01,0xF2,0xF2,
  0x04,0xEA,0xF5,0xF9,0xD8,0xEE,0xF5,0xDB,0xF5,0x0C,0xEA,0x0C,0xFD,0xEE,0x0C,0xE7,
  0xFD,0xF9,0xEE,0xF2,0x04,0xF2,0xEE,0x08,0xE7,0xF2,0xF5,0xBE,0x04,0xEE,0xD8,0x0C,
  0xF9,0xEE,0x10,0xF5,0xEE,0x08,0xF2,0xF2,0x0C,0xE3,0x01,0xF9,0xE7,0x04,0xEE,0xEE,
  0x01,0xD4,0xD4,0x0C,0xE3,0xEE,0x10,0xEE,0x08,0xEE,0xF5,0xF2,0xFD,0xEA,0xF9,0xEA,
  0xE7,0x01,0xE7,0xEE,0xFD,0xE7,0xEE,0xF9,0xC1,0xEE,0xF2,0xD0,0xFD,0x08,0xF5,0xFD,
  0xF9,0xEA,0x04,0xFD,0xE7,0x04,0xF2,0xDB,0x1B,0xDB,0x04,0xF5,0xDB,0xF5,0xE3,0xC5,
  0x08,0xE7,0xDB,0x01,0xF5,0xE7,0x08,0xE7,0xEA,0x04,0xDF,0xFD,0xFD,0xEA,0xF5,0xFD,
  0xDF,0xF9,0xF6,0xE7,0xF5,0xE7,0xD4,0xFD,0xCD,0xF2,0xFD,0xEE,0xF2,0xF9,0xE3,0x04,
  0xF5,0xDF,0xF2,0xE7,0xE7,0xEE,0xE3,0xF5,0xFD,0xE7,0xDF,0xF9,0xAB,0xEE,0xEA,0xC5,
  0xF5,0xF6,0xF2,0x01,0xEE,0xE7,0x08,0xE7,0xF5,0xF9,0xE7,0xDB,0x13,0xFD,0x08,0x01,
  0x01,0x01,0xF9,0xB6,0x08,0xDF,0xCD,0x0C,0xEE,0xF9,0xF5,0xF9,0xE7,0xFD,0xD8,0xFD,
  0xF9,0xE3,0xF5,0xF2,0xDB,0x08,0xE7,0xE3,0xF5,0xD4,0xD8,0xF2,0xDB,0xE7,0x04,0xEA,
  0xFD,0xEE,0xDF,0x04,0xFD,0xEA,0x01,0xEE,0xD8,0x0F,0xE7,0xF9,0x01,0xEA,0xEA,0xF2,
  0xC9,0xE3,0xFD,0xCD,0xF9,0x0F,0xE7,0x08,0xF5,0xEA,0x08,0xE7,0xF2,0xFD,0xEE,0xF2,
  0x10,0xDB,0x08,0xF5,0xE7,0xF5,0xF2,0xD4,0x01,0xE3,0xE3,0x0C,0xF5,0x08,0x04,0xFD,
  0xF2,0x01,0xFD,0xF2,0x01,0xDF,0xFD,0xFD,0xDF,0x0C,0xEA,0xE7,0xFD,0xD0,0xE3,0xFD,
  0xD0,0xF2,0xF9,0xFD,0xEE,0x0C,0xE7,0x01,0x04,0xE3,0x13,0xEE,0xFD,0xF9,0xFD,0xDF,
  0x17,0xDF,0xF9,0xF5, 0
 };


 inline unsigned int iexp (unsigned int power) { // exp(x) = pow( 16385/16394=1.00006103516, x*16384 )  to get real exp value if needed
  enum { FRAC_BITS=14, FRAC_MUL=16384 };
  static unsigned int result, base;
  result=FRAC_MUL; base=FRAC_MUL+1;
  while(power) { if(power&1) result=(result*base)>>FRAC_BITS;  power>>=1; base=(base*base)>>FRAC_BITS; }
  return result-FRAC_MUL;
 }


 //Cycle-based part of emulations:


 while (ResampleBufPos < NextResampleBufPos) {

  if (!CPlus4->RealTEDmode) {
   if (CPlus4->FrameCycleCnt >= CPlus4->FrameCycles) {
    CPlus4->FrameCycleCnt -= CPlus4->FrameCycles;
    if (CPlus4->Finished) {
     cRTED_initCPU ( &CPlus4->CPU, CPlus4->PlayAddress );
     CPlus4->Finished=0;
   }}
   if (CPlus4->Finished==0) {
    if ( (InstructionCycles = cRTED_emulateCPU()) >= 0xFE ) { InstructionCycles=6; CPlus4->Finished=1; }
   }
   else InstructionCycles=7; //idle between player-calls
   CPlus4->FrameCycleCnt += TED_AUDIO_CLOCK_PRESCALE; //InstructionCycles;
  }

  else { //RealTED emulations:
   //CPU-instruction-timed:
   while (InstrCycleCnt < CPlus4->CPUspeedMul) {
    if ( cRTED_handleCPUinterrupts(&CPlus4->CPU) ) { CPlus4->Finished=0; InstructionCycles=7; } //interrupt starts
    else if (CPlus4->Finished==0) {
     if ( (InstructionCycles = cRTED_emulateCPU()) >= 0xFE ) {
      InstructionCycles=6; CPlus4->Finished=1;
     }
    }
    else InstructionCycles=7; //idle between IRQ-calls
    cRTED_emulateKeyMatrix();
    InstrCycleCnt += InstructionCycles;
   }
   InstrCycleCnt -= CPlus4->CPUspeedMul;

   CPlus4->IRQ = CPlus4->NMI = 0;
   CPlus4->IRQ |= cRTED_emulateTED (&CPlus4->TED, TED_AUDIO_CLOCK_PRESCALE);
  }

  TEDwavOutput = cRTED_emulateTEDaudio(&CPlus4->TED);

  //Resampling subsequent input-samples to output-sample-buffer
  ResampleBufWritePos = (ResampleBufPos>>CRTED_FRACTIONAL_BITS) - (SINCWINDOW_PERIODS/2-1);
   if (ResampleBufWritePos<0) ResampleBufWritePos += RESAMPLEBUFFER_SIZE;
  SincWindowPos = SINCPERIOD_SAMPLES - ( (ResampleBufPos&FRACTIONAL_AND) >> (CRTED_FRACTIONAL_BITS-SINCPERIOD_BITS) );
  while (SincWindowPos < SINCWINDOW_SIZE) {
   ResampleBuffer[ResampleBufWritePos] += (TEDwavOutput * Sinc2048[SincWindowPos]) >> SINCWINDOW_RESOLUTION;
   ++ResampleBufWritePos; if (ResampleBufWritePos >= RESAMPLEBUFFER_SIZE) ResampleBufWritePos=0;
   SincWindowPos += SINCPERIOD_SAMPLES;
  }
  ResampleBufPos += CPlus4->SampleClockRatio;
 }
 if (ResampleBufPos >= RESAMPLEBUFFER_SIZE_MUL) ResampleBufPos -= RESAMPLEBUFFER_SIZE_MUL;
 NextResampleBufPos = (ResampleBufPos & INTEGER_AND) + FRACTIONAL_MUL;
 tmp = ResampleBuffer[ResampleBufWritePos] / CPlus4->SampleClockRatioReciproc; ResampleBuffer[ResampleBufWritePos] = 0;


 //Samplerate-based part of emulations, output-circuitry after TED's output:


 //AC-coupling capacitor before C1815 transistor-buffer
 //(AC-blocking 2700pF is not relevant here, because it makes about 50..70kHz cutoff)
 PWMhighPass += (tmp - PWMhighPass) >> PWM_HIGHPASS_BITSHIFT; tmp -= PWMhighPass;

 //2.7pF Integrator (~50..70kHz?) (plus some Miller-capacitance filtering on the following transistor)
 IntegratorLowPass += ((tmp - IntegratorLowPass) * INTEGRATOR_LOWPASS_CUTOFF) >> 4;
 //IntegratorLowPass += 512; if (IntegratorLowPass>=0x7FFF) IntegratorLowPass -= 0xFFFF; //for SOUNDTEST (linearity of transistor) with sawtooth

 //2SC1815 transistor emitter-follower distortion //signal mustn't be bigger than short-int range here:
 //if (IntegratorLowPass+NONDISTORTED_MIDDLEVALUE > max) { max=IntegratorLowPass+NONDISTORTED_MIDDLEVALUE; printf("max:%d\n",max); }
 //if (IntegratorLowPass+NONDISTORTED_MIDDLEVALUE < min) { min=IntegratorLowPass+NONDISTORTED_MIDDLEVALUE; printf("min:%d\n",min); }
 TransistorCurrent = ( (iexp((IntegratorLowPass+NONDISTORTED_MIDDLEVALUE) >> 1) * 10) >> 4 );
 //if (TransistorCurrent > max) { max=TransistorCurrent; printf("max:%d\n",max); } //signal mustn't be bigger than short-int range here
 //if (TransistorCurrent < min) { min=TransistorCurrent; printf("min:%d\n",min); } //~16000...62000
 tmp = ( ( ((IntegratorLowPass+NONDISTORTED_MIDDLEVALUE)<<2) + TransistorCurrent ) << 1 ) / 10 - NONDISTORTED_MIDDLEVALUE;
 //signal mustn't be bigger than short-int range here (sim: Vbe is fairly linear, middle 0.45mV higher):
 //if (tmp > max) { max=tmp; printf("max:%d\n",max); }  //low edge must be about 1300 upper (20mV*65536),
 //if (tmp < min) { min=tmp; printf("min:%d\n",min); } //(~7000...60000)    or middle about 524 lower (8mV*65536) sim:1.5mV(98)

 //Add machine noise to the output (mainly made by RF-modulator AFTER the C1815 audio-buffering transistor)
 tmp += ((signed char)MachineNoise_1wave_S8[MachineNoiseSampleCounter])>>2;
 if ( MachineNoiseSampleCounter++ >= sizeof(MachineNoise_1wave_S8)-1 ) MachineNoiseSampleCounter=0;

 //Output-capacitor (there's another one before the C1815 transistor)
 MachineOutputHighPass += (tmp - MachineOutputHighPass) >> OUTPUT_HIGHPASS_BITSHIFT; //DC-component
 tmp -= MachineOutputHighPass; //remove DC (simulate decoupling capacitor)

 //Highpass-filtering (ferrite-bead's and other connected-to-outut-cable's/equipment's lowpass) ~ 15kHz (single-pole)
 MachineOutputLowPass += ((tmp - MachineOutputLowPass) * OUTPUT_LOWPASS_CUTOFF) >> 4;

 //Sound-output connector
 CPlus4->TED.MainOutput = MachineOutputLowPass;

 Output.L = CPlus4->MuteBeforeKeypress? 0 : (CPlus4->TED.MainOutput);


 CPlus4->TED.Level += ( abs(CPlus4->TED.MainOutput) - CPlus4->TED.Level ) >> 10; //for scope-display

 if (CPlus4->KeyPressDelayCounter>0) --CPlus4->KeyPressDelayCounter; //for subtune-selection at startup

 return Output;
}

