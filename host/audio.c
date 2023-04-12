
#ifdef CRTED_PLATFORM_PC

#include <SDL.h>


void cRTED_soundCallback(void* userdata, unsigned char *buf, int len) {
 cRTED_CPlus4.SoundStarted=1;
 cRTED_generateSound( (cRTED_CPlus4instance*)userdata, buf, len );

 if ( cRTED_CPlus4.SubtuneDurations[cRTED_CPlus4.SubTune] != 0 && cRTED_CPlus4.AutoAdvance
      && (cRTED_CPlus4.TEDheader->SubtuneAmount>1 || cRTED_CPlus4.AutoExit || cRTED_CPlus4.PlayListSize>1) ) {
  if (cRTED_CPlus4.PlayTime == cRTED_CPlus4.SubtuneDurations[cRTED_CPlus4.SubTune]) {
   if (cRTED_CPlus4.SubTune<cRTED_CPlus4.TEDheader->SubtuneAmount) {
    if (cRTED_CPlus4.CLImode) {
     cRTED_nextSubtune();
     #ifndef LIBRARY
      printTuneInfo(1);
     #endif
    }
    else cRTED_CPlus4.PlaytimeExpired=1;
   }
   else {
    if (cRTED_CPlus4.PlayListSize<=1 && cRTED_CPlus4.AutoExit) { printf("\n"); cRTED_close(); exit(0); }
    else {
     if (cRTED_CPlus4.CLImode) {
      cRTED_nextTune();
      #ifndef LIBRARY
       printTuneInfo(0);
      #endif
     }
     else cRTED_CPlus4.PlayListAdvance=1;
  }}}
  else if ( cRTED_CPlus4.FadeOut && cRTED_CPlus4.PlayTime+1 == cRTED_CPlus4.SubtuneDurations[cRTED_CPlus4.SubTune]
            && cRTED_CPlus4.SubtuneDurations[cRTED_CPlus4.SubTune]>4 ) {
   cRTED_CPlus4.FadeLevel= 0xF - 0xF * cRTED_CPlus4.SecondCnt / cRTED_CPlus4.SampleRate;
  }
  else cRTED_CPlus4.FadeLevel=0xF;
 }
 else cRTED_CPlus4.FadeLevel=0xF;
}


void* cRTED_initSound(cRTED_CPlus4instance* CPlus4, unsigned short samplerate, unsigned short buflen) {
 static SDL_AudioSpec soundspec;
 CPlus4->SoundStarted=0;
 if ( SDL_Init(SDL_INIT_AUDIO) < 0 ) {
  fprintf(stderr, "Couldn't initialize SDL-Audio: %s\n",SDL_GetError()); return NULL;
 }
 soundspec.freq=samplerate;
 soundspec.channels=1;
 soundspec.format=AUDIO_S16;
 soundspec.samples=buflen; soundspec.userdata=CPlus4; soundspec.callback=cRTED_soundCallback;
 if ( SDL_OpenAudio(&soundspec, NULL) < 0 ) {
  fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError()); return NULL;
 }
 return (void*)&soundspec;
}


void cRTED_closeSound (void) {
 SDL_PauseAudio(1);
 if (cRTED_CPlus4.SoundStarted) { //SDL_CloseAudio() freezed when somehow sound-callbacks didn't start
  SDL_CloseAudio();
 }
}


void cRTED_startSound (void) {
 SDL_PauseAudio(0);
}


void cRTED_stopSound (void) {
 SDL_PauseAudio(1);
}


void cRTED_generateSound(cRTED_CPlus4instance* CPlus4, unsigned char *buf, unsigned short len) {
 static unsigned short i; static unsigned char j, ScopeCnt=0; static signed char FiltedSignal=0;
 static cRTED_Output Output;
 static signed short OutputL, OutputR;

 for (i=ScopeCnt=0; i<len; i+=2) {
  for(j=0; j<CPlus4->PlaybackSpeed; ++j) Output=cRTED_generateSample(CPlus4);
  Output.L = ( Output.L * CPlus4->MainVolume * CPlus4->FadeLevel ) >> 12;
  buf[i+0] = Output.L&0xFF; buf[i+1] = Output.L>>8;

  FiltedSignal += ((Output.L>>8) - FiltedSignal) >> 2;
  if (ScopeCnt<CRTED_OSCILLOSCOPE_WIDTH_MAX && (i%CRTED_OSCILLOSCOPE_STRETCH)==0) {
   CPlus4->TED.ScopeData[ScopeCnt] = FiltedSignal; ++ScopeCnt;
 }}
}


#endif


static inline cRTED_Output cRTED_generateSample (cRTED_CPlus4instance* CPlus4) { //call this from custom buffer-filler
 static cRTED_Output Output; signed short PSIDdigi;
 Output=cRTED_emulateCPlus4(CPlus4);

 if (CPlus4->SecondCnt < CPlus4->SampleRate) ++CPlus4->SecondCnt;
 else { CPlus4->SecondCnt = 0; if(CPlus4->PlayTime<3600) ++CPlus4->PlayTime; }

 if (Output.L>=32767) Output.L=32767; else if (Output.L<=-32768) Output.L=-32768; //saturation logic on overflow
 if (Output.R>=32767) Output.R=32767; else if (Output.R<=-32768) Output.R=-32768; //saturation logic on overflow
 return Output;
}

