// cRTED - a lightweight (integer-only) RealTED playback environment by Hermit

#ifndef CRTED_PLATFORM_PC
 #define CRTED_PLATFORM_PC
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#ifdef LINUX
  #include <termios.h> //<curses.h>
#elif defined(WINDOWS)
 #include <conio.h>
#endif

void printTuneInfo (char subtunestepping);
#include "libcRTED.c"

#include "GUI/GUI.c"


#define DEFAULT_SUBTUNE 1 //1..
#define BUILTIN_MUSIC_DEFAULT_SUBTUNE 5
#define DEFAULT_SAMPLERATE 44100
#define BUFFERSIZE_MIN       256
#define BUFFERSIZE_DEFAULT  4096 //8192
#define BUFFERSIZE_MAX     32768
#define DIRENTRIES_MAX      1000
#define PLAYLIST_FILE_EXTENSION ".tel"


int fileExists (char *path) { FILE *fp;  if ( (fp=fopen(path,"r")) != NULL ) { fclose(fp); return 1;} else return 0; }
char isFile (char *path) { static struct stat st; stat(path, &st); return S_ISREG(st.st_mode); }
char isDir (char *path) { static struct stat st; stat(path, &st); return S_ISDIR(st.st_mode); }

void addToPlayList (char* filename, char checkextension) {
 if ( cRTED_CPlus4.PlayListSize < CRTED_PLAYLIST_ENTRIES_MAX && (!checkextension || cRTED_playableExtension(filename)) ) {
  strcpy( cRTED_CPlus4.PlayList[cRTED_CPlus4.PlayListSize], filename );
  if (checkextension) cRTED_CPlus4.PlayListNumbering[cRTED_CPlus4.PlayListSize] = cRTED_CPlus4.PlayListNumber++;
  ++cRTED_CPlus4.PlayListSize;
}}


void findSortTEDfiles (char* basefolder, char recursive) {


 int cmpstr(char *string1, char *string2) { //compare strings alphabetically for sorting
  for (;;) {
   unsigned char char1 = tolower(*string1++); unsigned char char2 = tolower(*string2++);
   if (char1 < char2) return -1; if (char1 > char2) return 1; if ((!char1) || (!char2)) return 0;
 }}


 void findTEDfiles (char* basefolder, char recursive) {
  int i; DIR *Dir; struct dirent *Entry; char Path[CRTED_PATH_LENGTH_MAX]; static char DirPath[CRTED_PATH_LENGTH_MAX];
  int FileListSize=0; int FileListHash[CRTED_PLAYLIST_ENTRIES_MAX];
  char *FileList; //[DIRENTRIES_MAX][PATH_LENGTH_MAX]; //a preallocated array wouldn't fit on stack

  void addToFileList (char* filename, char checkextension) {
   if ( FileListSize < DIRENTRIES_MAX && (!checkextension || cRTED_playableExtension(filename)) ) {
    strcpy( FileList+FileListSize*CRTED_PATH_LENGTH_MAX, filename ); FileListHash[FileListSize]=FileListSize; ++FileListSize;
  }}

  void sortFileList () {
   static int i,j,first, entrytmp;
   for (i=0; i<FileListSize; i++) {
    first=i;
    for (j=i+1; j<FileListSize; j++) {
     if ( cmpstr(FileList+FileListHash[j]*CRTED_PATH_LENGTH_MAX, FileList+FileListHash[first]*CRTED_PATH_LENGTH_MAX)<0 ) first=j;
    }
    if (first!=i) { entrytmp=FileListHash[i]; FileListHash[i]=FileListHash[first]; FileListHash[first]=entrytmp; }
  }}

  //process files in folder
  Dir = opendir(basefolder); if (!Dir) return;
  FileList = malloc(DIRENTRIES_MAX*CRTED_PATH_LENGTH_MAX); FileListSize=0;
  while ( (Entry=readdir(Dir)) != NULL && FileListSize < DIRENTRIES_MAX ) {
   strcpy(Path,basefolder); strcat(Path,"/"); strcat(Path,Entry->d_name);
   if (isFile(Path) && cRTED_playableExtension(Entry->d_name)) addToFileList(Path,1);
  }
  closedir(Dir);
  if (FileListSize>0) {
   if ( strcmp(basefolder,".") ) {
    strcpy(Path,"---"); strcat(Path,cRTED_fileNameOnly(basefolder)); strcat(Path,"---"); addToPlayList(Path,0);
   }
   else addToPlayList("Content of current folder:",0);
   sortFileList(); for (i=0; i<FileListSize; ++i) { addToPlayList( FileList+FileListHash[i]*CRTED_PATH_LENGTH_MAX, 1 ); }
  }
  //process subfolders recursively
  if (recursive) {
   Dir = opendir(basefolder); if (!Dir) { free(FileList); return; }
   FileListSize=0;
   while ( (Entry=readdir(Dir)) != NULL && FileListSize < DIRENTRIES_MAX ) {
    strcpy(Path,basefolder); strcat(Path,"/"); strcat(Path,Entry->d_name);
    if ( isDir(Path) && strcmp(Entry->d_name,".") && strcmp(Entry->d_name,"..") ) addToFileList(Path,0);
   }
   closedir(Dir);
   if (FileListSize>0) {
    sortFileList(); for (i=0; i<FileListSize; ++i) { findTEDfiles( FileList+FileListHash[i]*CRTED_PATH_LENGTH_MAX, recursive ); }
   }
  }
  free(FileList);
 }

 //find the files
 findTEDfiles (basefolder, recursive);
}


void setKeyCapture (char state) { //refinement for CLI mode key-handling
 #ifdef LINUX
  struct termios TTYstate;
  tcgetattr(STDIN_FILENO, &TTYstate);
  if (state) TTYstate.c_lflag &= ~ICANON; else TTYstate.c_lflag |= ICANON;
  tcsetattr(STDIN_FILENO, TCSANOW, &TTYstate);
 #endif
}


//======================================================================================

int main (int argc, char *argv[]) {
 #ifdef LINUX
  enum CursorKeys { KEY_LEFT=0x44, KEY_RIGHT=0x43, KEY_UP=0x41, KEY_DOWN=0x42 };
 #elif defined(WINDOWS)
  enum CursorKeys { KEY_LEFT=0x4B, KEY_RIGHT=0x4D, KEY_UP=0x48, KEY_DOWN=0x50 };
 #endif

 int i, Tmp, ArgMainVolume=0;
 char ArgFileIdx=0, ArgSubtuneIdx=0, Info=0;
 char PressedKeyChar=0, Exit=0;
 char *TEDfileName="";
 char SubTune=0, CIAisSet;
 FILE *fp;
 cRTED_TEDheader *TEDheader;
 cRTED_CPlus4instance *CPlus4;

 void printUsage() {
  static char WasDisplayed=0;
  if(!WasDisplayed) {
   printf("\nUsage of cRTED-"VERSION" (arguments can follow in any order):\n"
            "crted [ -cli | filename(.prg/.tmf/.ted) | subtunenumber | folder "
            "| playlistfile.tel | -info | -noadvance | -autoexit | -nofadeout "
            "| -volume <0..255> | -bufsize <256..32768> ] \n");
   if (cRTED_CPlus4.CLImode) printf("\n%s\n",HelpText);
  }
  WasDisplayed=1;
 }

 char processListFile(char* filename) {
  static char Path[CRTED_PATH_LENGTH_MAX]; int SizeTmp;
  fp = fopen(filename,"r");
  if (fp!=NULL) {
   SizeTmp = cRTED_CPlus4.PlayListSize; addToPlayList(cRTED_fileNameOnly(filename),0);
   while ( fgets (Path, CRTED_PATH_LENGTH_MAX, fp) ) {
    cRTED_removeNewline(Path);
    if ( fileExists(Path) && isFile(Path) ) { addToPlayList( Path, 1 ); }
    else if (isDir(Path)) { cRTED_removeNewline(Path); findSortTEDfiles(Path,1); }
   }
   fclose(fp); if (cRTED_CPlus4.PlayListSize==SizeTmp+1) { cRTED_CPlus4.PlayListSize=SizeTmp; return 0; } //if nothing was found
   return 1;
  }
  return 0;
 }

 char findListFile (char* basefolder) {
  char Path[CRTED_PATH_LENGTH_MAX]; static char DirPath[CRTED_PATH_LENGTH_MAX]; DIR *Dir; struct dirent *Entry;
  Dir = opendir(basefolder); if (!Dir) return 0;
  while ( (Entry=readdir(Dir)) != NULL ) {
   if ( strcmp(Entry->d_name,".") && strcmp(Entry->d_name,"..") ) {
    strcpy(Path,basefolder); strcat(Path,"/"); strcat(Path,Entry->d_name);
    if ( cRTED_compareFileExtension(Entry->d_name,PLAYLIST_FILE_EXTENSION) ) { closedir(Dir); return processListFile(Path); }
  }}
  closedir(Dir); return 0;
 }

 void setFirstPlayableItem() {
  TEDfileName = cRTED_CPlus4.PlayList[ cRTED_CPlus4.PlayListPlayPosition=cRTED_findNextPlayableItem(0) ];
 }


 i=1; cRTED_CPlus4.BufferSize=BUFFERSIZE_DEFAULT;
 while (i<argc) {
  if (!strcmp(argv[i],"-bufsize")) {
   ++i;
   if(i<argc) {
    sscanf(argv[i],"%d",&cRTED_CPlus4.BufferSize);
    if(cRTED_CPlus4.BufferSize<256) cRTED_CPlus4.BufferSize=256;
    if(32768<cRTED_CPlus4.BufferSize) cRTED_CPlus4.BufferSize=32768;
   }
  }
  ++i;
 }
 CPlus4 = cRTED_init( DEFAULT_SAMPLERATE, cRTED_CPlus4.BufferSize );
 if (CPlus4==NULL) exit(CRTED_ERROR_INIT);

 #ifdef WINDOWS //have output on console instead of txt files
  freopen("CON", "w", stdout); freopen("CON", "w", stderr);
 #endif

 i=1; ArgFileIdx=ArgSubtuneIdx=0;
 while (i<argc) {
  if (!strcmp(argv[i],"-h") || !strcmp(argv[i],"-?") || !strcmp(argv[i],"-help")
                         || !strcmp(argv[i],"--h") || !strcmp(argv[i],"--help")) printUsage();
  else if (!strcmp(argv[i],"-info")) Info=1;
  else if (!strcmp(argv[i],"-cli")) { cRTED_CPlus4.CLImode=1; }
  else if (!strcmp(argv[i],"-autoexit")) { cRTED_CPlus4.AutoExit=1; cRTED_CPlus4.AutoAdvance=1; } //auto-advance in subtunes or exit
  else if (!strcmp(argv[i],"-noadvance")) cRTED_CPlus4.AutoAdvance=0;
  else if (!strcmp(argv[i],"-nofadeout")) { cRTED_CPlus4.FadeOut=0; }
  else if (!strcmp(argv[i],"-volume")) {
   ++i;
   if(i<argc) {
    sscanf(argv[i],"%d",&ArgMainVolume);
    if(ArgMainVolume<0) CPlus4->MainVolume=0; else if(255<ArgMainVolume) CPlus4->MainVolume=255; else CPlus4->MainVolume = ArgMainVolume;
  }}
  else if ( cRTED_playableExtension(argv[i]) ) { //check file-extension and/or magic-string! add argument-file(s) to playlist
   if ((fp=fopen(argv[i],"rb")) != NULL ) {
    fclose(fp); ArgFileIdx=i; if (cRTED_CPlus4.PlayListSize>0) addToPlayList("File given as argument:",0);
    cRTED_CPlus4.PlayListPlayPosition = cRTED_CPlus4.PlayListSize; addToPlayList(argv[i],1);
   }
  }
  else if ( cRTED_compareFileExtension(argv[i],PLAYLIST_FILE_EXTENSION) ) { //process playlist-file into internal playlist
   chdir(cRTED_folderNameOnly(argv[i])); processListFile(argv[i]); //gives back starting point for relative-paths
  }
  else if (sscanf(argv[i],"%d",&Tmp)==1 && 1<Tmp && Tmp<256) { SubTune=Tmp; ArgSubtuneIdx=i; }
  else { //search playable files in given folder recursively and add them to playlist
   findSortTEDfiles(argv[i],1);
  }
  ++i;
 }
 if (ArgFileIdx) TEDfileName = argv[ArgFileIdx];
 else if (cRTED_CPlus4.PlayListSize==0) { //if no file/folder/playlist argument given, scan current folder as a fallback
  if (cRTED_CPlus4.CLImode || Info) printUsage();
  findSortTEDfiles(".",0);
  if (cRTED_CPlus4.PlayListSize>0 ) setFirstPlayableItem();
  else if (findListFile(".")) { setFirstPlayableItem(); }
  if (cRTED_CPlus4.PlayListSize==0) { //if nothing found at all, fallback to built-in music and help
   cRTED_CPlus4.BuiltInMusic=1; cRTED_CPlus4.PlayListSize=1; cRTED_CPlus4.AutoAdvance=1;
   if(SubTune==0) SubTune=BUILTIN_MUSIC_DEFAULT_SUBTUNE;
  }
 }
 else setFirstPlayableItem();

 //--------------------------------------------------------------------------------------------

 if ( (SubTune==0 && Info==0 && cRTED_CPlus4.PlayListSize<=1) || cRTED_CPlus4.CLImode==0 ) {
  if (cRTED_CPlus4.CLImode || SubTune==0) SubTune=DEFAULT_SUBTUNE;
  cRTED_CPlus4.TEDheader = TEDheader = cRTED_playTEDfile( CPlus4, TEDfileName, SubTune );
  if (TEDheader == NULL) { printf("Load error! (Single/first file not found.)\n"); return CRTED_ERROR_LOAD; }
 }
 else { //CLI detailed playback

  if ( (TEDheader = cRTED_loadTEDtune(CPlus4,TEDfileName)) == NULL )
  { printf("Load error! (Single/first file not found.)\n"); return CRTED_ERROR_LOAD; }

  cRTED_initTEDtune(CPlus4,TEDheader,SubTune); CPlus4->PlaybackSpeed=1; CPlus4->Paused=0;

  printTuneInfo(0);

  cRTED_playTEDtune();

 }


 CPlus4->PlaybackSpeed=1; CPlus4->Paused=0;


 if (cRTED_CPlus4.CLImode) {
  printf("Press ENTER to abort playback, SPACE to pause/continue, TAB for "
         "fast-forward/normal, 1..9/Left/Right for subtune/next/previous, Up/Down:Volume\n");
  setKeyCapture(1);
  while(!Exit && CPlus4->PlaytimeExpired==0) {
   PressedKeyChar=
   #ifdef LINUX
    getchar();
   #elif defined(WINDOWS)
    getch();
   #endif
   if (PressedKeyChar=='\n' || PressedKeyChar=='\r') Exit=1; //Enter?
   else if (PressedKeyChar==' ') {
    CPlus4->PlaybackSpeed=1; CPlus4->Paused^=1; if(CPlus4->Paused) cRTED_pauseTEDtune(); else cRTED_playTEDtune();
   }
   else if (PressedKeyChar==0x09 || PressedKeyChar=='`') {
    if(CPlus4->PlaybackSpeed==1) CPlus4->PlaybackSpeed = CRTED_FFWD_SPEED; else CPlus4->PlaybackSpeed = 1;
   }
   else if ('1'<=PressedKeyChar &&  PressedKeyChar<='9') {
    SubTune=PressedKeyChar-'1'+1; cRTED_startSubtune(SubTune); printTuneInfo(1);
   }
   else if (PressedKeyChar==KEY_RIGHT) { printTuneInfo( nextTuneButton() ); }
   else if (PressedKeyChar==KEY_LEFT) { printTuneInfo( prevTuneButton() ); }
   else if (PressedKeyChar==KEY_UP) { volumeUpButton(); }
   else if (PressedKeyChar==KEY_DOWN) { volumeDownButton(); }
   else CPlus4->PlaybackSpeed=1;
   usleep(5000);
  }
  setKeyCapture(0);
 }

 else { //GUI
  initGUI();
  #ifdef WINDOWS //have output on console instead of txt files
   freopen("CON", "w", stdout); freopen("CON", "w", stderr);
  #endif
  mainLoop(CPlus4);
 }

 cRTED_close();
 return 0;
}

//======================================================================================


void printTuneInfo (char subtunestepping) {
 cRTED_TEDheader *TEDheader = cRTED_CPlus4.TEDheader;

 if (!subtunestepping) {
  if (cRTED_CPlus4.PlayListSize>1)
   printf("\n------------------------------ PlayList-item %d -------------------------------------",
           cRTED_CPlus4.PlayListNumbering[cRTED_CPlus4.PlayListPlayPosition]);

  printf("\nAuthor: %s ", cRTED_getTEDtext(TEDheader->Author) ); printf(", Title: %s\n", cRTED_getTEDtext(TEDheader->Title) );
  printf("Info: %s ", cRTED_getTEDtext(TEDheader->ReleaseInfo) ); printf(", Tool: %s\n", cRTED_getTEDtext(TEDheader->Tool) );

  printf("Load-address:$%4.4X, End-address:$%4.4X, Size:%d bytes\n",
          cRTED_CPlus4.LoadAddress, cRTED_CPlus4.EndAddress, cRTED_CPlus4.EndAddress - cRTED_CPlus4.LoadAddress);
  if (!cRTED_CPlus4.BASICmode) printf("Init-address:$%4.4X, ", cRTED_CPlus4.InitAddress);
  else printf ("BASIC-SYS-starter:$%4.4X, ",cRTED_CPlus4.InitAddress);
  if (!cRTED_CPlus4.RealTEDmode) {
   printf("Play-address:$%4.4X, ", cRTED_CPlus4.PlayAddress);
   if (TEDheader->PlayAddressH==0 && TEDheader->PlayAddressL==0) printf("(IRQ), ");
  }
 }


 if (!cRTED_CPlus4.BASICmode) {
  printf("Subtune:%d (of %d)", cRTED_CPlus4.SubTune, TEDheader->SubtuneAmount);
 }
 if (cRTED_CPlus4.RealTEDmode) printf(", RealTED");
 if (cRTED_CPlus4.SubtuneDurations[cRTED_CPlus4.SubTune]) {
  printf(" (PlayTime: %2.2d:%2.2d)", cRTED_CPlus4.SubtuneDurations[cRTED_CPlus4.SubTune] / 60,
                                     cRTED_CPlus4.SubtuneDurations[cRTED_CPlus4.SubTune] % 60 );
 }
 printf("\n");


 if (!subtunestepping) {
  printf("TED:$%4.4X,%d ", cRTED_CPlus4.TED.BaseAddress, cRTED_CPlus4.TED.ChipModel);
  printf("\n");

  if (!cRTED_CPlus4.RealTEDmode) {
   printf( "Speed: %.1fx (player-call at every %d cycle) TimerSource:%s ",
           (cRTED_CPlus4.VideoStandard<=1? 17734.0:14915.0) / cRTED_CPlus4.FrameCycles,
             cRTED_CPlus4.FrameCycles, cRTED_CPlus4.TimerSource? "Timer":"Raster" );
  }
  printf ("Standard:%s\n", cRTED_CPlus4.VideoStandard? "NTSC":"PAL" );
 }

}
