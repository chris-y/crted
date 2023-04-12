//TED emulation of cRTED by Hermit


void cRTED_createTEDchip (cRTED_CPlus4instance* CPlus4, cRTED_TEDinstance* TED, unsigned short baseaddress) {
 TED->CPlus4 = CPlus4;
 TED->BaseAddress = baseaddress; TED->BasePtr = &CPlus4->IObank[baseaddress];
 TED->BasePtrWR = &CPlus4->IObankWR[baseaddress]; TED->BasePtrRD = &CPlus4->IObankRD[baseaddress];
 //heebie_jeebies.ted needs timer3 running by default //don't run if not needed,
 //other tunes seem to use Timer1 only, and all start the timer by writing value to TIMER1H:
 TED->TimerRunning[0]=1; TED->TimerRunning[1]=TED->TimerRunning[2] = 1;
 cRTED_initTEDchip (TED);
}


void cRTED_initTEDchip (cRTED_TEDinstance* TED) {

 enum initConstants { SOUNDDECAY_AUDIOCYCLES = 188416 }; //not exact cycles but an analog sympthom,
                                                         //depending on how much the machine warmed up?:
 short i;
 TED->RowCycleCnt=0; //for (i=0; i<0x3F; ++i) TED->BasePtrWR[i] = TED->BasePtrRD[i] = 0x00;
 TED->AudioClock = TED->FreqCounters[0] = TED->FreqCounters[1] = TED->PulseStates[0] = TED->PulseStates[1] = 0;
 TED->NoiseLFSR = 0x00; TED->SoundDecayCounters[0] = TED->SoundDecayCounters[1] = SOUNDDECAY_AUDIOCYCLES;
 TED->FreqCounterOverflows[0] = TED->FreqCounterOverflows[1] = 0;
 TED->PrevFreqCounterOverflows[0] = TED->PrevFreqCounterOverflows[1] = 0;
 TED->Level = TED->ChannelMutes[0] = TED->ChannelMutes[1] = 0;
}


static inline char cRTED_emulateTED (cRTED_TEDinstance* TED, char cycles) {

 enum TEDregisters { TIMER1LO = 0x00, TIMER1HI = 0x01, TIMER2LO = 0x02, TIMER2HI = 0x03, TIMER3LO = 0x04, TIMER3HI = 0x05,
                     CONTROL = 0x06, KEYLATCH=0x08, INTERRUPT = 0x09, INTERRUPT_ENABLE = 0x0A, INTERRUPT_RASTERROWL = 0x0B,
                     BORDERCOLOR = 0x19, RASTERROWH = 0x1C, RASTERROWL = 0x1D, FLASHCOUNTER = 0x1F,
                     ROM_ON = 0x3E, ROM_OFF = 0x3F }; //write any value to ROMoff/ROMon

 enum ControlBitVal { RASTERROWMSB = /*reg$0A,reg$1C*/0x01, /*reg6:*/ DISPLAY_ENABLE = 0x10, ROWS = 0x08, YSCROLL_MASK = 0x07 };

 enum InterruptBitVal { TED_IRQ = /*reg9*/0x80, TIMER3_ZERO_IRQ=0x40, TIMER2_ZERO_IRQ=0x10, TIMER1_ZERO_IRQ=0x08,
                        RASTERROW_MATCH_IRQ = /*reg9,reg$0A*/0x02 };

 enum DoubleSpeedEstimates {
  CPLUS4_BORDER_SCANLINE = 205, //starts doubling CPU frequency after this (and in non-badline rows there are double cycles too btw.)
  TED_CPU_SPEED_NOBORDER = 5, //4*1.32 = 5.28 //(33*TED_AUDIO_CLOCK_PRESCALE/25) //1.32 = 2 * 150*45+50*16 / 200*57
  TED_CPU_SPEED_BORDER = 7 //4*1.92 = 7.28 //(91*TED_AUDIO_CLOCK_PRESCALE/50) //1.82 = 2*0.912 = 2*52/57
 };


 static unsigned short RasterRow; static signed int TimerValue; static char IRQ=0;


 TED->RowCycleCnt += cycles;
 if (TED->RowCycleCnt >= TED->RasterRowCycles) {
  TED->RowCycleCnt -= TED->RasterRowCycles;

  RasterRow = ( (TED->BasePtrRD[RASTERROWH]&RASTERROWMSB&1) ? 0x100:0 ) + TED->BasePtrRD[RASTERROWL];
  ++RasterRow;
  if (RasterRow >= TED->RasterLines) {
   RasterRow = 0; TED->BasePtrRD[FLASHCOUNTER]+=8; TED->BasePtrRD[FLASHCOUNTER]&=0xF8;
  }
  cRTED_CPlus4.CPUspeedMul = (RasterRow >= CPLUS4_BORDER_SCANLINE) ? TED_CPU_SPEED_BORDER : TED_CPU_SPEED_NOBORDER;
  //tunes like 8sob_optika.ted read the register, code at $103C expects value $FE:
  TED->BasePtrRD[RASTERROWH] = (RasterRow&0x100) ? ((0xFF^RASTERROWMSB)|RASTERROWMSB) : (0xFF^RASTERROWMSB);
  TED->BasePtrRD[RASTERROWL] = RasterRow & 0xFF;

  if ( RasterRow == ( (TED->BasePtrRD[INTERRUPT_ENABLE]&RASTERROWMSB) ? 0x100:0 ) + TED->BasePtrRD[INTERRUPT_RASTERROWL] ) {
   TED->BasePtrRD[INTERRUPT] |= RASTERROW_MATCH_IRQ;
  }
 }

 if(TED->TimerRunning[0]) {
  TimerValue = (TED->BasePtrRD[TIMER1HI]<<8) | TED->BasePtrRD[TIMER1LO]; TimerValue -= cycles;
  if (TimerValue <= 0) {
   TimerValue += (TED->BasePtrWR[TIMER1HI]<<8) | TED->BasePtrWR[TIMER1LO]; TED->BasePtrRD[INTERRUPT] |= TIMER1_ZERO_IRQ;
  }
  TED->BasePtrRD[TIMER1HI] = TimerValue>>8; TED->BasePtrRD[TIMER1LO] = TimerValue&0xFF;
 }

 if(TED->TimerRunning[1]) {
  TimerValue = (TED->BasePtrRD[TIMER2HI]<<8) | TED->BasePtrRD[TIMER2LO]; if(TED->TimerRunning[0]) TimerValue -= cycles;
  if (TimerValue <= 0) { TimerValue += 0xFFFF; TED->BasePtrRD[INTERRUPT] |= TIMER2_ZERO_IRQ; }
  TED->BasePtrRD[TIMER2HI] = TimerValue>>8; TED->BasePtrRD[TIMER2LO] = TimerValue&0xFF;
 }

 if(TED->TimerRunning[2]) {
  TimerValue = (TED->BasePtrRD[TIMER3HI]<<8) | TED->BasePtrRD[TIMER3LO]; TimerValue -= cycles;
  if (TimerValue <= 0) { TimerValue += 0xFFFF; TED->BasePtrRD[INTERRUPT] |= TIMER3_ZERO_IRQ; }
  TED->BasePtrRD[TIMER3HI] = TimerValue>>8; TED->BasePtrRD[TIMER3LO] = TimerValue&0xFF;
 }

 IRQ = TED->BasePtrRD[INTERRUPT]
       & ( TED->BasePtrRD[INTERRUPT_ENABLE] & (RASTERROW_MATCH_IRQ | TIMER1_ZERO_IRQ | TIMER2_ZERO_IRQ | TIMER3_ZERO_IRQ) );
 TED->BasePtrRD[INTERRUPT] |= (IRQ? TED_IRQ:0) | 0x25;
 return IRQ;
}


static inline void cRTED_acknowledgeTEDIRQ (cRTED_TEDinstance* TED, unsigned char mask) {
 enum TEDregisters { INTERRUPT = 0x09 };
 enum InterruptBitVal { TED_IRQ = /*reg9*/0x80, TIMER3_ZERO_IRQ=0x40, TIMER2_ZERO_IRQ=0x10, TIMER1_ZERO_IRQ=0x08,
                        RASTERROW_MATCH_IRQ = /*reg9,reg$0A*/0x02 };
 //remove IRQ flag and state:
 TED->BasePtrRD[INTERRUPT] &= ~( TED_IRQ | (mask & (RASTERROW_MATCH_IRQ | TIMER1_ZERO_IRQ | TIMER2_ZERO_IRQ | TIMER3_ZERO_IRQ)) );
}


static inline void cRTED_emulateKeyMatrix () { //keyboard-emulation (for e.g. kane.ted, arkanoid.ted)
 enum KeyboardConstants { KEYCOLUMN=0xFD30, KEYPRESS_DURATION = 2000, NOKEY = 0xFF }; //duration is given in samples
 enum TEDregisters { KEYLATCH = 0xFF08, KEYCHAR = 0xC6, NOKEY2 = 0x40 };
 enum StatusFlagBitValues { I=0x04 };

 static unsigned char Key;
 static const unsigned char Pow2[8]={1,2,4,8,16,32,64,128};
 static const unsigned char InvPow2[8]={0xFE,0xFD,0xFB,0xF7,0xEF,0xDF,0xBF,0x7F};

 static const unsigned char KeyMatrixSymbols[] = {
 // 0       1         2       3       4      5     6     7     8       9       A       B     C        D     E     F  //$FF08 / $FD30 bits
  0x00,   0x00,      0x00,  0x00,    0x00, 0x00, 0x00,  '@',  '3',    'W',    'A',    '4',  'Z',     'S',  'E', 0x00,     //0x //$FE,$FD
 '5',    'R',      'D',     '6',     'C',   'F',  'T',  'X',  '7',    'Y',    'G',    '8',  'B',     'H',  'U', 'V',      //1x //$FB,$F7
 '9',    'I',      'J',     '0',     'M',   'K',  'O',  'N',  0x00,   'P',    'L',    0x00, '.',     ':',  '-', ',',      //2x //$EF,$DF
 0x00,   '*',      ';',     0x00,    0x00,  '=',  '+',  '/',  '1',    0x00,   0x00,   '2',  ' ',    0x00,  'Q', 0x00   }; //3x //$BF,$7F

 Key = cRTED_CPlus4.SubtuneKeyPresses[ cRTED_CPlus4.SubTune ];
 cRTED_CPlus4.PressedKeySymbol=0x00; cRTED_CPlus4.MuteBeforeKeypress = 0;
 if ( (cRTED_CPlus4.TEDheader->FileFlags & 2) && Key != NOKEY && cRTED_CPlus4.KeyPressDelayCounter > 0 ) {
  cRTED_CPlus4.IObankRD[KEYLATCH] |= Pow2[ Key & 7 ];
  if (cRTED_CPlus4.KeyPressDelayCounter < KEYPRESS_DURATION ) {
   cRTED_CPlus4.PressedKeySymbol = KeyMatrixSymbols[Key];
   //workaround of $C6 for squirm_16.ted and others using it, but elvarazsolt_kastely.ted uses $C6:
   if (cRTED_CPlus4.ROMenabled && cRTED_CPlus4.IRQ) cRTED_CPlus4.RAMbank[KEYCHAR] = cRTED_CPlus4.KeyPressDelayCounter==1? NOKEY2:Key;
   if ( !( cRTED_CPlus4.IObank[KEYCOLUMN] & Pow2[ ((Key&0x3F)>>3) ] ) ) {
    cRTED_CPlus4.IObankRD[KEYLATCH] &= InvPow2[ Key & 7 ];
  }}
  else cRTED_CPlus4.MuteBeforeKeypress = 1;
 }
 else { cRTED_CPlus4.IObankRD[KEYLATCH]=0xFF; }
}


//============================== TED Audio emulation =================================


int cRTED_emulateTEDaudio (cRTED_TEDinstance *TED) {
 enum TEDregisters { FREQ1LO=0x0E, FREQ2LO=0x0F, FREQ2HI=0x10, SNDCTRL=0x11, FREQ1HI=0x12 };
 enum SoundControlBitVal { CHANNEL1_PULSE_ON=0x10, CHANNEL2_PULSE_ON=0x20, CHANNEL2_NOISE_ON=0x40, DA_MODE=0x80 };
 enum Constants { FREQCOUNTER_MAX=0x3FF, PULSE_MAX=0x7F, PULSE_HALF=0x40, VOLMUL=1,
                  SOUNDDECAY_AUDIOCYCLES = 188416/*0x2E000*/ }; //not exact cycles but an analog sympthom,
                                                                //depending on how much the machine warmed up?
                  //,DOUBLECHANNEL_VOLMUL=128, SINGLECHANNEL_VOLMUL=124 }; //96.8% //2.9Vp-p vs 2.8Vp-p analog (3.4Vp-p vs 3.2Vp-p PWM)
                  // when both channels are On (in digital recording: 108 vs 114)


 static unsigned char NoiseLFSRfeedBack=0, DAmode=0;
 static short Channel1, Channel2; static int ChannelsLowPass=0;

 static const char soundVolumeTable[] = {
   //(percentages are for 1 channel, based on oscilloscope PWM-dutycycle measurement)
   //PWM    0%      3.81%     10.2%      16.4%      22.61%     28.91%     35.29%    41.5%
    //output-recording: F1 E3 D4 C4 B5 A6 97 89 80 -> 113,99,84,68,53,38,23,9,0:
          0*VOLMUL, 9*VOLMUL, 23*VOLMUL, 38*VOLMUL, 53*VOLMUL, 68*VOLMUL, 84*VOLMUL, 99*VOLMUL,
    //16 is added for each step (based on oscilloscope signal-level measurements):
        //0*VOLMUL,10*VOLMUL, 26*VOLMUL, 42*VOLMUL, 58*VOLMUL, 74*VOLMUL, 90*VOLMUL, 106*VOLMUL,
   //PWM: 46.82% ... (based on oscilloscope measurement)
          113*VOLMUL,113*VOLMUL,113*VOLMUL,113*VOLMUL,113*VOLMUL,113*VOLMUL,113*VOLMUL,113*VOLMUL
        //119*VOLMUL,119*VOLMUL,119*VOLMUL,119*VOLMUL,119*VOLMUL,119*VOLMUL,119*VOLMUL,119*VOLMUL //46.82% of 255
 };
 /*static const short soundVolumeTableDoubleChannel[VOLTABLE_SIZE] = {
  #define doubChn(idx) (soundVolumeTable[idx]*DOUBLECHANNEL_VOLMUL)
   doubChn(0),doubChn(1),doubChn(2),doubChn(3),doubChn(4),doubChn(5),doubChn(6),doubChn(7),
   doubChn(8),doubChn(9),doubChn(10),doubChn(11),doubChn(12),doubChn(13),doubChn(14),doubChn(15)
 };
 static const short soundVolumeTableSingleChannel[VOLTABLE_SIZE] = {
  #define singChn(idx) (soundVolumeTable[idx]*SINGLECHANNEL_VOLMUL)
   singChn(0),singChn(1),singChn(2),singChn(3),singChn(4),singChn(5),singChn(6),singChn(7),
   singChn(8),singChn(9),singChn(10),singChn(11),singChn(12),singChn(13),singChn(14),singChn(15)
 };*/

 //static const
        #include "PulseDecayNoise-U8.h"


 DAmode = (TED->BasePtrRD[SNDCTRL] & DA_MODE);

 TED->FreqCounterOverflows[0] = (TED->FreqCounters[0] == FREQCOUNTER_MAX);
 if (TED->FreqCounterOverflows[0] || DAmode) {
  TED->FreqCounters[0] = ( (((TED->BasePtrRD[FREQ1HI]&3)<<8) | TED->BasePtrRD[FREQ1LO]) );
  if (DAmode) {
   if (TED->FreqCounters[0]<FREQCOUNTER_MAX-1) { TED->PulseStates[0]=PULSE_MAX; TED->SoundDecayCounters[0]=SOUNDDECAY_AUDIOCYCLES; }
   else { TED->PulseStates[0]=0; } //else TED->FreqCounterOverflows[0]=1; //prevent flipping by terminating of nonDA+<$3FE by a single-audioclock?
  }
  else if (!TED->PrevFreqCounterOverflows[0]) { //edge-detection ensures $3FE being inaudible (but executed once?):
   TED->PulseStates[0]^=PULSE_MAX; TED->SoundDecayCounters[0]=SOUNDDECAY_AUDIOCYCLES;
 }}
 TED->PrevFreqCounterOverflows[0] = TED->FreqCounterOverflows[0];
 ++TED->FreqCounters[0]; TED->FreqCounters[0] &= FREQCOUNTER_MAX;
 if (TED->SoundDecayCounters[0]>0) --TED->SoundDecayCounters[0];  else TED->PulseStates[0]=PULSE_MAX;

 TED->FreqCounterOverflows[1] = (TED->FreqCounters[1] == FREQCOUNTER_MAX);
 if (TED->FreqCounterOverflows[1] || DAmode) {
  TED->FreqCounters[1] = ( (((TED->BasePtrRD[FREQ2HI]&3)<<8) | TED->BasePtrRD[FREQ2LO]) );
  if (DAmode) { //SoundDelayCounter during DAmode:
   if (TED->FreqCounters[1]<FREQCOUNTER_MAX-1) { TED->PulseStates[1]=PULSE_MAX; TED->SoundDecayCounters[1]=SOUNDDECAY_AUDIOCYCLES; }
   else { TED->PulseStates[1]=0; } //else TED->FreqCounterOverflows[1]=1; //prevent flipping by terminating of nonDA+<$3FE by a single-audioclock?
   TED->NoiseLFSR = (TED->NoiseLFSR<<4); //fill LFSR by amount of single CPU-cycles happening meanwhile
  }
  else { //executed only when overflow happens (and no DAmode)
   if (!TED->PrevFreqCounterOverflows[1]) {
    TED->PulseStates[1]^=PULSE_MAX; TED->SoundDecayCounters[1]=SOUNDDECAY_AUDIOCYCLES;
    NoiseLFSRfeedBack = ((TED->NoiseLFSR&0x80)>>3) ^ ((TED->NoiseLFSR&0x20)>>1) ^ (TED->NoiseLFSR&0x10) ^ ((TED->NoiseLFSR&2)<<3);
    TED->NoiseLFSR<<=1; TED->NoiseLFSR |= (NoiseLFSRfeedBack&0x10)? 0:1;
 }}}
 TED->PrevFreqCounterOverflows[1] = TED->FreqCounterOverflows[1];
 ++TED->FreqCounters[1]; TED->FreqCounters[1] &= FREQCOUNTER_MAX;
 if (TED->SoundDecayCounters[1]>0) --TED->SoundDecayCounters[1];  else TED->PulseStates[1]=PULSE_MAX;


 Channel1 = (TED->BasePtrRD[SNDCTRL]&CHANNEL1_PULSE_ON) ?
              ( DAmode? PULSE_MAX     //MachineNoise happening before total Pulse-decay:
                        : ( (!TED->PulseStates[0] && TED->SoundDecayCounters[0] < sizeof(PulseDecayNoise_U8))?
                              (PulseDecayNoise_U8[ sizeof(PulseDecayNoise_U8)-1 - TED->SoundDecayCounters[0] ] >> 1)
                              : TED->PulseStates[0] ) )
              : 0;
 Channel2 = (TED->BasePtrRD[SNDCTRL]&CHANNEL2_PULSE_ON) ?
              ( DAmode? PULSE_MAX
                        : ( (!TED->PulseStates[1] && TED->SoundDecayCounters[1] < sizeof(PulseDecayNoise_U8))?
                              (PulseDecayNoise_U8[ sizeof(PulseDecayNoise_U8)-1 - TED->SoundDecayCounters[1] ] >> 1)
                              : TED->PulseStates[1] ) )
              : ( ((TED->BasePtrRD[SNDCTRL]&CHANNEL2_NOISE_ON) && !DAmode) ?
                     ((TED->NoiseLFSR&1)?PULSE_MAX:0)
                     : 0 );

 if (TED->ChannelMutes[0]) Channel1=0;  if(TED->ChannelMutes[1]) Channel2=0;

 ChannelsLowPass += ( ( ( ( (Channel1+Channel2) * soundVolumeTable[ TED->BasePtrRD[SNDCTRL] & 0xF ] ) )
                                                    //* (Channel1&&Channel2? DOUBLECHANNEL_VOLMUL:SINGLECHANNEL_VOLMUL) ) >> 7 )
                              //( (Channel1 && Channel2)? soundVolumeTableDoubleChannel[ TED->BasePtrRD[SNDCTRL] & 0xF ] // >> 7; // / 128; // TED->MainOutput;
                              //                          : soundVolumeTableSingleChannel[ TED->BasePtrRD[SNDCTRL] & 0xF ] ) ) >> 7 )
                        - ChannelsLowPass ) /* * 4*/ ) >> 1; //ferrite bead FB19 lowpass (?kHz)
 return ChannelsLowPass;
}


