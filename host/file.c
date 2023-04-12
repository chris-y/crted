#include <string.h>

#ifdef CRTED_PLATFORM_PC

int cRTED_loadTEDfile (unsigned char* TEDfileData, char* filename, int maxlen) {
 static signed short Data;
 static signed int SizeCnt;
 static FILE *TEDfile;
 #ifndef LIBRARY
  //static
         #include "../resources/builtin-music.h"
 #endif

 SizeCnt=0;
 if (cRTED_CPlus4.BuiltInMusic==0) {
  if ( (TEDfile=fopen(filename,"rb")) == NULL || !cRTED_playableExtension(filename)) return CRTED_ERROR_LOAD;
  while ( (Data=fgetc(TEDfile)) != EOF ) {
   if (SizeCnt >= maxlen) return CRTED_ERROR_LOAD;
   TEDfileData[SizeCnt] = Data; SizeCnt++;
  }
  fclose(TEDfile);
 }
 #ifndef LIBRARY
  else while(SizeCnt<sizeof(builtin_music)) { TEDfileData[SizeCnt] = builtin_music[SizeCnt]; ++SizeCnt; }
 #endif

 return SizeCnt;
}

#endif


unsigned short int cRTED_findSYStarget (unsigned char* data, short int size) {
 static unsigned short i, j;
 for (j=0x0000, i=6; i<32 && i<size; ++i) if (data[i]==0x9E) break; //'SYS' BASIC-command found?
 if (i==32) j = 4109; //fallback trial if no SYS-command found
 else {
  for (++i; i<32 && i<size; ++i) { if ('0'<=data[i] && data[i]<='9') j = j*10 + data[i]-'0';   else if (data[i]!=' ') break; }
 }
 if (j==0x0000) j = 4109; //another fallback
 return j;
}


cRTED_TEDheader* cRTED_processTEDfile (cRTED_CPlus4instance* CPlus4, unsigned char* filename,
                                        unsigned char* filedata, int filesize) {
 int i, Tmp, EndAddressSub;
 cRTED_TEDheader* TEDheader;
 static const char MagicStringTEDMUSIC[]="TEDMUSIC";

 void setBASICmode () {
  for (i=0; i<sizeof(cRTED_TEDheader); ++i) ((unsigned char*)&cRTED_CPlus4.DummyTEDheader)[i]=0;
  CPlus4->TEDheader = TEDheader = &cRTED_CPlus4.DummyTEDheader;
  CPlus4->TEDheader->SubtuneAmount=1; strncpy(CPlus4->TEDheader->Title,filename,CRTED_INFOFIELD_SIZE);
  CPlus4->RealTEDmode = 1;
  CPlus4->LoadAddress = 0x1001; CPlus4->TEDdataOffset = 0x0002; EndAddressSub=0;
 }


 for (i=0x0000; i < 0x8000; ++i) CPlus4->RAMbank[i]=0; //fresh start (maybe some bugged TEDs want 0 at certain RAM-locations)

 CPlus4->BASICmode = cRTED_compareFileExtension(filename,".prg")
                     && !cRTED_compareFileExtension(filename,".ted.prg") && !cRTED_compareFileExtension(filename,".tmf.prg");


 if (CPlus4->BASICmode) setBASICmode();
 else {
  CPlus4->RealTEDmode = 0;
  CPlus4->TEDheader = TEDheader = (cRTED_TEDheader*) filedata;
  EndAddressSub = 0;
  for (i=0; i < (int)(sizeof(MagicStringTEDMUSIC)-1); ++i) { if (TEDheader->MagicString[i] != MagicStringTEDMUSIC[i]) break; }
  Tmp = TEDheader->BASIClineNumberL + (TEDheader->BASIClineNumberH<<8);
  if ( i<sizeof(MagicStringTEDMUSIC)-1 || Tmp>=0x1000 ) { //Not Siz's TMF format?
   Tmp = Tmp-0x1001; CPlus4->TEDheader = TEDheader = (cRTED_TEDheader*)&filedata[Tmp-17]; EndAddressSub = filesize - Tmp;
  }
  for (i=0; i < (int)(sizeof(MagicStringTEDMUSIC)-1); ++i) { //Hermit's TED(TMF) format with off-set header?
   if (TEDheader->MagicString[i] != MagicStringTEDMUSIC[i]) break; //No header no play! (or fall back to BASIC?)
  }
  Tmp = TEDheader->InitAddressL + (TEDheader->InitAddressH<<8);
  if ( i==(sizeof(MagicStringTEDMUSIC)-1) && Tmp>0 ) {
   CPlus4->LoadAddress = (TEDheader->DataAddressH<<8) + (TEDheader->DataAddressL);
   CPlus4->TEDdataOffset = (TEDheader->DataOffsetH<<8) + (TEDheader->DataOffsetL);
  }
  else setBASICmode();
 }


 for (i=CPlus4->TEDdataOffset; i<filesize-EndAddressSub; ++i) {
  CPlus4->RAMbank [ CPlus4->LoadAddress + (i-CPlus4->TEDdataOffset) ] = filedata[i];
 }
 Tmp = CPlus4->LoadAddress + (filesize-CPlus4->TEDdataOffset) - EndAddressSub;
 CPlus4->EndAddress = (Tmp<0x10000) ? Tmp : 0xFFFF;


 for (i=0; i<=CRTED_SUBTUNE_AMOUNT_MAX; ++i) { CPlus4->SubtuneDurations[i] = 0; CPlus4->SubtuneKeyPresses[i] = 0xFF; }
 Tmp = (TEDheader->DurationOffsetH<<8) + TEDheader->DurationOffsetL;
 if (TEDheader->FileFlags & 1) {
  for (i=0; i<TEDheader->SubtuneAmount; ++i) {
   CPlus4->SubtuneDurations[i+1] = filedata[ Tmp + i*2 ] + (filedata[ Tmp + i*2 + 1 ]<<8);
  }
  Tmp += i * 2;
 }
 if (TEDheader->FileFlags & 2) {
  CPlus4->KeyPressDelay = filedata[Tmp] * (CPlus4->SampleRate/10);
  if (CPlus4->KeyPressDelay==0) CPlus4->KeyPressDelay=50000; //samples
  for (i=1; i<=TEDheader->SubtuneAmount; ++i) { CPlus4->SubtuneKeyPresses[i] = filedata[Tmp + i]; }
 }


 if (CPlus4->BASICmode) { CPlus4->InitAddress = cRTED_findSYStarget(filedata, filesize); }

 return CPlus4->TEDheader;
}

