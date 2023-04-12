// cRTED lightweight (integer-only) RealTED library (with API-calls) by Hermit (Mihaly Horvath), Year 2023
// License: WTF - do what the fuck you want with the code, but please mention me as the original author

#include <stdlib.h>
//#include <stdint.h>

#ifdef CRTED_PLATFORM_PC
 #include <stdio.h>
 //#include <math.h>
 #include <libgen.h> //for dirname()
#endif

#include "libcRTED.h"

#include "CPlus4/CPlus4.c"
#include "host/file.c"
#include "host/audio.c"


cRTED_CPlus4instance* cRTED_init (unsigned short samplerate, unsigned short buflen) {
 int i;
 static cRTED_CPlus4instance* CPlus4 = &cRTED_CPlus4;

 CPlus4->PlaybackSpeed=1;
 CPlus4->MainVolume=204; CPlus4->CLImode=0; CPlus4->AutoExit=CPlus4->BuiltInMusic=0;
 CPlus4->AutoAdvance=CPlus4->FadeOut=1; CPlus4->FadeLevel=0xF;
 CPlus4->KeyPressDelayCounter = CPlus4->MuteBeforeKeypress = 0;
 CPlus4->RealTEDmode = CPlus4->BASICmode = 0; CPlus4->PlayListSize = 0; CPlus4->PlayListNumber = 1;
 CPlus4->PlayListPlayPosition = CPlus4->PlayListAdvance = 0;

 for (i=0; i<sizeof(cRTED_TEDheader); ++i) ((unsigned char*)&cRTED_CPlus4.DummyTEDheader)[i]=0;

 CPlus4 = cRTED_createCPlus4 (CPlus4, samplerate);
#ifdef CRTED_PLATFORM_PC
 if ( cRTED_initSound (CPlus4, samplerate,buflen) == NULL) return NULL;
#else
 if (buflen) return CPlus4; //this is here just to eliminate unused 'buflen' variable warning
#endif

 return CPlus4;
}


void cRTED_initTEDtune (cRTED_CPlus4instance* CPlus4, cRTED_TEDheader* TEDheader, char subtune) { //subtune: 1..255
 static const unsigned char PowersOf2[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
 unsigned int InitTimeout=10000000; //allowed instructions, value should be selected to allow
                                    //for long-running memory-copiers in init-routines

 CPlus4->PlaytimeExpired=0; CPlus4->KeyPressDelayCounter = CPlus4->KeyPressDelay;

 if (subtune==0) subtune = 1;
 else if (subtune > TEDheader->SubtuneAmount) subtune = TEDheader->SubtuneAmount;
 CPlus4->SubTune = subtune; CPlus4->SecondCnt = CPlus4->PlayTime = CPlus4->Paused = 0;

 cRTED_setCPlus4(CPlus4); cRTED_initCPlus4(CPlus4); //set CPlus4 hardware and init (reset) it

 //determine init-address:
 if (!CPlus4->BASICmode) {
  CPlus4->InitAddress = ((TEDheader->InitAddressH)<<8) + (TEDheader->InitAddressL); //get info from BASIC-startupcode for some tunes
 }
 //8BDC sometimes used to jump to code directly after BASIC-starter (rablo-rulett_loader.ted)
 //(leading to A7CC using zp $14..$15 as address)
 CPlus4->RAMbank[0x14] = CPlus4->InitAddress&0xFF; CPlus4->RAMbank[0x15] = CPlus4->InitAddress>>8;

 if (CPlus4->ROMenabled) { //are there TEDs with routine under IO area? some PTEDs don't set bank-registers themselves
  if (CPlus4->InitAddress >= 0x8000) CPlus4->ROMenabled=0;
 }
 cRTED_initCPU( &CPlus4->CPU, CPlus4->InitAddress ); //prepare init-routine call
 CPlus4->CPU.A = subtune - 1;

 //determine timing-source, if timer, replace FrameCycles previouisly set to raster-interrupt timing
 CPlus4->TimerSource = CPlus4->TEDheader->TimingSource;
 if (CPlus4->TimerSource>=2) CPlus4->FrameCycles = (CPlus4->TEDheader->Timer1valueH<<8) + CPlus4->TEDheader->Timer1valueL;

 //determine playaddress:
 CPlus4->PlayAddress = (TEDheader->PlayAddressH<<8) + TEDheader->PlayAddressL;
 if (CPlus4->PlayAddress) { //normal play-address called with JSR
  if (CPlus4->ROMenabled) { //are there TEDs with routine under IO area?
   if (CPlus4->PlayAddress >= 0x8000) CPlus4->ROMenabled=0;
 }}
 else { CPlus4->RealTEDmode=1; }

 if (!CPlus4->RealTEDmode) {  //prepare (PTED) play-routine playback:
  for (InitTimeout=10000000; InitTimeout>0; InitTimeout--) { if ( cRTED_emulateCPU()>=0xFE ) break; } //give error when timed out?
  cRTED_initCPU( &CPlus4->CPU, CPlus4->PlayAddress ); //point CPU to play-routine
  CPlus4->FrameCycleCnt=0; CPlus4->Finished=1; CPlus4->SampleCycleCnt=0;
 } //some tunes (like c16_shit.ted, culture_demo_ii.ted) require CPU-IRQ enabled by default:
 else { CPlus4->Finished=0; CPlus4->Returned=0; CPlus4->CPU.ST=0x00; }

}


#ifdef CRTED_PLATFORM_PC


cRTED_TEDheader* cRTED_playTEDfile(cRTED_CPlus4instance* CPlus4, char* filename, char subtune) {
 static cRTED_TEDheader* TEDheader;

 TEDheader = cRTED_loadTEDtune(CPlus4,filename);
 if (TEDheader==NULL) return NULL;

 cRTED_initTEDtune (CPlus4 , TEDheader , subtune);
 cRTED_playTEDtune ();

 return TEDheader;
}


char* cRTED_fileNameOnly (char* path) {
 int LastPos; char* name;
 LastPos=strlen(path)-1;
 #ifndef WINDOWS
  if (path[LastPos] == '/') path[LastPos]='\0'; //remove possible trailing slash if foldername
  return basename(path);
 #else
  if (path[LastPos] == '\\') path[LastPos]='\0'; //remove possible trailing slash if foldername
  name=strrchr(path,'\\'); if(name==NULL) name=strrchr(path,'/');  return (name!=NULL)? name+1:path;
 #endif
}

char* cRTED_folderNameOnly (char* path) {
  return dirname(path);
}


cRTED_TEDheader* cRTED_loadTEDtune(cRTED_CPlus4instance* CPlus4, char* filename) {

 cRTED_pauseTEDtune(); //stop if already playing

 CPlus4->TEDfileSize = cRTED_loadTEDfile( CPlus4->TEDfileData, filename, CRTED_FILESIZE_MAX);
 if ( CPlus4->TEDfileSize == CRTED_ERROR_LOAD ) return NULL;
 strcpy (CPlus4->FileNameOnly, cRTED_fileNameOnly(filename));

 return cRTED_processTEDfile ( CPlus4, CPlus4->FileNameOnly, CPlus4->TEDfileData, CPlus4->TEDfileSize );
}


void cRTED_close (void) {
 cRTED_closeSound();
}


void cRTED_playTEDtune (void) {
 cRTED_startSound();
}


void cRTED_pauseTEDtune (void) {
 cRTED_stopSound();
}


void cRTED_startSubtune(unsigned char subtune) {
 int i;
 cRTED_pauseTEDtune();
 cRTED_processTEDfile ( &cRTED_CPlus4, cRTED_CPlus4.FileNameOnly, cRTED_CPlus4.TEDfileData, cRTED_CPlus4.TEDfileSize );
 cRTED_initTEDtune (&cRTED_CPlus4, cRTED_CPlus4.TEDheader, subtune);
 cRTED_CPlus4.PlaybackSpeed=1; cRTED_CPlus4.Paused=0; cRTED_playTEDtune();
}
void cRTED_nextSubtune() { if (cRTED_CPlus4.SubTune<CRTED_SUBTUNE_AMOUNT_MAX) cRTED_startSubtune(++cRTED_CPlus4.SubTune); }
void cRTED_prevSubtune() { if (cRTED_CPlus4.SubTune>0) cRTED_startSubtune(--cRTED_CPlus4.SubTune); }


unsigned int cRTED_findNextPlayableItem (int itemnumber) {
 while (cRTED_CPlus4.PlayListSize>0 && !cRTED_playableExtension(cRTED_CPlus4.PlayList[itemnumber])
        && itemnumber<cRTED_CPlus4.PlayListSize-1) ++itemnumber; //skip folder-texts
 return itemnumber;
}

unsigned int cRTED_findPreviousPlayableItem (int itemnumber) {
 while (!cRTED_playableExtension(cRTED_CPlus4.PlayList[itemnumber]) && itemnumber>0) --itemnumber; //skip folder-texts
 return itemnumber;
}

char cRTED_startPlayListItem (int itemnumber) {
 cRTED_TEDheader *TEDheader=NULL;
 if (itemnumber < cRTED_CPlus4.PlayListSize) {
  itemnumber = cRTED_findNextPlayableItem (itemnumber); //skip folder-texts
  if ( cRTED_playableExtension(cRTED_CPlus4.PlayList[itemnumber]) ) {
   TEDheader = cRTED_playTEDfile( &cRTED_CPlus4, cRTED_CPlus4.PlayList[itemnumber], 1 ); //default subtune?
   if (TEDheader == NULL) { printf("Load error! (Playlist-item not found.)\n"); return CRTED_ERROR_LOAD; }
   else { cRTED_CPlus4.TEDheader=TEDheader; cRTED_CPlus4.PlayListPlayPosition = itemnumber; }
 }}
}

void cRTED_nextTune() {
 cRTED_CPlus4.PlayListAdvance = 0;
 if ( cRTED_CPlus4.PlayListPlayPosition+1 < cRTED_CPlus4.PlayListSize ) {
  cRTED_startPlayListItem( cRTED_CPlus4.PlayListPlayPosition+1 );
 }
 else cRTED_startPlayListItem(0); //loop over the playlist or end playback? (setting for this behaviour?)
}

void cRTED_prevTune() {
 if ( cRTED_CPlus4.PlayListPlayPosition-1 >= 0 ) {
  cRTED_CPlus4.PlayListPlayPosition = cRTED_findPreviousPlayableItem(cRTED_CPlus4.PlayListPlayPosition-1); //skip folder-texts
  cRTED_startPlayListItem( cRTED_CPlus4.PlayListPlayPosition );
 }
 else cRTED_startPlayListItem(cRTED_CPlus4.PlayListSize-1); //loop over the playlist or end playback? (setting for this behaviour?)
}


char* cRTED_getTEDtext (char* TEDtextField) {
 char i;
 static char Text[CRTED_INFOFIELD_SIZE+1];
 for (i=CRTED_INFOFIELD_SIZE-1; i>=0; --i) if (TEDtextField[i]!=' ') break;
 for (Text[i+1]='\0'; i>=0; --i) Text[i] = TEDtextField[i];
 return Text;
}


char cRTED_compareFileExtension (char *filename, char *extension) {
 short i,j;  //get pointer of file-extension from filename string  //if no '.' found, point to end of the string
 for (i=strlen(filename)-1,j=strlen(extension)-1; i>=0 && j>=0; --i,--j) {
  if(tolower(filename[i])!=tolower(extension[j])) return 0;
 }
 return 1;
}

void cRTED_removeNewline(char *name) { //for fgets result containing newlines
 if (strrchr(name,'\n') != NULL) *strrchr(name,'\n') = '\0';
 if (strrchr(name,'\r') != NULL) *strrchr(name,'\r') = '\0';
}

char cRTED_playableExtension (char *name) {
 if (name==NULL) return 0;
 cRTED_removeNewline(name);
 return cRTED_compareFileExtension(name,".ted") || cRTED_compareFileExtension(name,".tmf")
        || cRTED_compareFileExtension(name,".ted.prg") || cRTED_compareFileExtension(name,".tmf.prg")
        || cRTED_compareFileExtension(name,".prg");
}


#endif

