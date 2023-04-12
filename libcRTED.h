// cRTED lightweight RealTED (integer-only) library-header (with API-calls) by Hermit (Mihaly Horvath)

#ifndef LIBCRTED_HEADER
#define LIBCRTED_HEADER //used  to prevent double inclusion of this header-file


enum cRTED_Limitations { CRTED_PATH_LENGTH_MAX=1024, CRTED_FILENAME_LEN_MAX = 255, CRTED_FILESIZE_MAX = 100000,
                         CRTED_SUBTUNE_AMOUNT_MAX=255, CRTED_INFOFIELD_SIZE=32,
                         CRTED_PLAYLIST_ENTRIES_MAX=2000, CRTED_PLAYLIST_ENTRY_SIZE=CRTED_PATH_LENGTH_MAX+2 };
enum cRTED_EmulationParameters { CRTED_FRACTIONAL_BITS = 12, CRTED_FFWD_SPEED=4 };
enum cRTED_StatusCodes    { CRTED_STATUS_OK=0, CRTED_ERROR_INIT=-1, CRTED_ERROR_LOAD=-2 };
#ifdef CRTED_PLATFORM_PC
 enum cRTED_GUIconfig      { CRTED_OSCILLOSCOPE_WIDTH_MAX=255, CRTED_OSCILLOSCOPE_STRETCH=10,  };
#endif


typedef struct cRTED_TEDheader cRTED_TEDheader;
typedef struct cRTED_CPlus4instance cRTED_CPlus4instance;
typedef struct cRTED_CPUinstance cRTED_CPUinstance;
typedef struct cRTED_TEDinstance cRTED_TEDinstance;
typedef struct cRTED_Output cRTED_Output;
typedef struct cRTED_TEDwavOutput cRTED_TEDwavOutput;


cRTED_CPlus4instance cRTED_CPlus4; //the only global object (for faster & simpler access than with struct-pointers, in some places)


// Main API functions (mainly in libcRTED.c)
cRTED_CPlus4instance* cRTED_init           (unsigned short samplerate, unsigned short buflen); //init emulation objects and sound
#ifdef CRTED_PLATFORM_PC
cRTED_TEDheader*   cRTED_playTEDfile    (cRTED_CPlus4instance* CPlus4, char* filename, char subtune); //simple single-call TED playback
cRTED_TEDheader*   cRTED_loadTEDtune    (cRTED_CPlus4instance* CPlus4, char* filename); //load and process TED-filedata to CPlus4 memory
void               cRTED_playTEDtune    (void); //start/continue playback (enable playing audio-buffer)
void               cRTED_pauseTEDtune   (void); //pause playback (enable playing audio-buffer)
void               cRTED_close          (void); //close sound etc.
void               cRTED_startSubtune   (unsigned char subtune);
void               cRTED_nextSubtune    ();
void               cRTED_prevSubtune    ();
unsigned int       cRTED_findNextPlayableItem (int itemnumber);
unsigned int       cRTED_findPreviousPlayableItem (int itemnumber);
char               cRTED_startPlayListItem (int itemnumber);
void               cRTED_nextTune       ();
void               cRTED_prevTune       ();
char*              cRTED_getTEDtext     (char* TEDtextField);
char               cRTED_compareFileExtension (char *filename, char *extension);
char               cRTED_playableExtension (char *name);
void               cRTED_removeNewline  (char *name);
char*              cRTED_fileNameOnly   (char* path);
char*              cRTED_folderNameOnly (char* path);
unsigned short int cRTED_findSYStarget  (unsigned char* data, short int size);
#endif
void               cRTED_initTEDtune    (cRTED_CPlus4instance* CPlus4, cRTED_TEDheader* TEDheader, char subtune); //init tune/subtune
static inline cRTED_Output cRTED_generateSample (cRTED_CPlus4instance* CPlus4); //in host/audio.c, calculate a single sample


//Internal functions

// CPlus4/CPlus4.c
cRTED_CPlus4instance* cRTED_createCPlus4  (cRTED_CPlus4instance* CPlus4, unsigned short samplerate);
void                  cRTED_setCPlus4     (cRTED_CPlus4instance* CPlus4); //configure hardware (TEDs) for TED-tune
void                  cRTED_initCPlus4    (cRTED_CPlus4instance* CPlus4); //hard-reset
cRTED_Output          cRTED_emulateCPlus4 (cRTED_CPlus4instance* CPlus4);
// CPlus4/MEM.c
static inline unsigned char* cRTED_getMemReadPtr        (register unsigned short address); //for global cRTED_CPlus4 fast-access
static inline unsigned char* cRTED_getMemReadPtrCPlus4  (cRTED_CPlus4instance* CPlus4, register unsigned short address); //maybe slower
static inline unsigned char* cRTED_getMemWritePtr       (register unsigned short address); //for global cRTED_CPlus4 fast-access
static inline unsigned char* cRTED_getMemWritePtrCPlus4 (cRTED_CPlus4instance* CPlus4, register unsigned short address); //maybe slower
static inline unsigned char  cRTED_readMem        (register unsigned short address); //for global cRTED_CPlus4 fast-access
static inline unsigned char  cRTED_readMemCPlus4  (cRTED_CPlus4instance* CPlus4, register unsigned short address); //maybe slower
static inline void           cRTED_writeMem       (register unsigned short address,
                                                    register unsigned char data); //for global cRTED_CPlus4 fast-access
static inline void           cRTED_writeMemCPlus4 (cRTED_CPlus4instance* CPlus4, register unsigned short address,
                                                    register unsigned char data); //maybe slower
void                         cRTED_setROMcontent  (cRTED_CPlus4instance* CPlus4); //KERNAL, BASIC
void                         cRTED_initMem        (cRTED_CPlus4instance* CPlus4);
// CPlus4/CPU.c
void               cRTED_initCPU       (cRTED_CPUinstance* CPU, unsigned short mempos);
unsigned char      cRTED_emulateCPU    (void); //direct instances inside for hopefully faster operation
static inline char cRTED_handleCPUinterrupts (cRTED_CPUinstance* CPU);
// CPlus4/TED.c
void               cRTED_createTEDchip           (cRTED_CPlus4instance* CPlus4, cRTED_TEDinstance* TED, unsigned short baseaddress);
void               cRTED_initTEDchip             (cRTED_TEDinstance* TED);
static inline char cRTED_emulateTED              (cRTED_TEDinstance* TED, char cycles);
int                cRTED_emulateTEDaudio         (cRTED_TEDinstance *TED);
static inline void cRTED_acknowledgeTEDIRQ       (cRTED_TEDinstance* TED, unsigned char mask);
static inline void cRTED_emulateKeyMatrix        ();

// host/file.c
#ifdef CRTED_PLATFORM_PC
int                cRTED_loadTEDfile   (unsigned char* TEDfileData,
                                         char* filename, int maxlen); //load TED-file to a memory location (and return size)
cRTED_TEDheader*   cRTED_processTEDfile (cRTED_CPlus4instance* CPlus4, unsigned char* filename,
                                          unsigned char* filedata, int filesize); //in host/file.c, copy TED-data to CPlus4 memory
#endif
// host/audio.c
#ifdef CRTED_PLATFORM_PC
void*              cRTED_initSound     (cRTED_CPlus4instance* CPlus4, unsigned short samplerate, unsigned short buflen);
void               cRTED_startSound    (void);
void               cRTED_stopSound     (void);
void               cRTED_closeSound    (void);
void               cRTED_generateSound (cRTED_CPlus4instance* CPlus4, unsigned char* buf, unsigned short len);
#endif


struct cRTED_TEDheader {                       //Offset:      default/info: //TMF-format header within Hermit's custom TED-format container
 unsigned char LoadAddressL,LoadAddressH;         //0,1        PRG load-address, always $1001 to be runnable from BASIC prompt
 unsigned char BASICnextLineL,BASICnextLineH;     //2,3        next BASIC line's address ($100B)
 unsigned char BASIClineNumberL,BASIClineNumberH; //4,5        BASIC SYS-command's line-number, POINTER TO THE TED(TMF)-HEADER
 unsigned char BASICsysCommand;                   //6          BASIC SYS-command token ($9E)
 unsigned char BASICsysParameter[4];              //7..10      BASIC SYS-parameter, the address of runnable machine code in ASCII
 unsigned char BASIClineEnd;                      //11     $0B BASIC line-end ($00)
 unsigned char BASICend[2];                       //12,13  $0C BASIC program end ($00,$00)
 unsigned char Filler[3];                         //14..16 $0E filler empty places (0,0,0)
 //TMF-only part
 char MagicString[8];                             //17..24 $11 "TEDMUSIC" signature/MagicString
 unsigned char Version;                           //25     $19 Preliminary specification-version (0)
 unsigned char DataOffsetL,DataOffsetH;           //26,27  $1A Where the music (maybe with loadaddress) resides - relative to beginning of file
 unsigned char DataAddressL,DataAddressH;         //28,29  $1C Where the music should be loaded/copied
 unsigned char InitAddressL,InitAddressH;         //30,31  $1E Init-routine to start/set song, subtune in Accu. Init mustn't change IRQ
 unsigned char PlayAddressL,PlayAddressH;         //32,33  $20 Play-routine's address, don't disturb IRQ (0=RTED?)
 unsigned char SubtuneAmount;                     //34     $22 amount of subtunes ('1' if only 1)
 unsigned char TimingSource;                      //35     $23 0:PAL Vblank(50Hz), 1:NTSC Vblank(60Hz), 2:PAL Timer1, 3:NTSC Timer1, 4:PAL Timer1(set by init), 5:NTSC Timer1(set by init)
 unsigned char Timer1valueL,Timer1valueH;         //36,37  $24 TED timer1 value (PAL: (1x:17734), 2x:8867, 4x:4433) (NTSC: (1x:14915), 2x:7457, 4x:3728)
 unsigned char SongFlags;                         //38     $26 bit4:FM?, bit3:AY?, bit2:DigiBlaster?, bit1:TEDcard?, bit0:Screen-off?
 unsigned char FileFlags;                         //39     $27 bit0:SongLength (duration) block is present (2 byte-words for all subtunes: seconds), bit1:keypress-block is present
 unsigned char DurationOffsetL,DurationOffsetH;   //40,41  $28 file-offset of the duration(songlength)-block (and/or keypress-block)
 unsigned char Reserved[23];                      //42..64 $2A reserved for future use (padding to have coming textfields aligned to $10 in memory)
 unsigned char Title[CRTED_INFOFIELD_SIZE];       //65..96 $41 Title of music
 unsigned char Author[CRTED_INFOFIELD_SIZE];      //97...  $61 Author/composer of music
 unsigned char ReleaseInfo[CRTED_INFOFIELD_SIZE]; //129..  $81 Release-info, recommended: [Year] [Group]
 unsigned char Tool[CRTED_INFOFIELD_SIZE];        //161..  $A1 Tracker/composer-tool that the music was created with
 unsigned char ReservedInfoField[64];             //193..  $C1 Reserved textfield for possible future use
 //(257..=$101.. built-in starter/initializer routine for pure TMF,) duration-block, etc.
};


struct cRTED_CPUinstance {
 cRTED_CPlus4instance* CPlus4; //reference to the containing CPlus4
 unsigned int       PC;
 short int          A, SP;
 unsigned char      X, Y, ST;  //STATUS-flags: N V - B D I Z C
 unsigned char      PrevNMI; //used for NMI leading edge detection
};


struct cRTED_TEDinstance {
 //TED-chip data:
 cRTED_CPlus4instance* CPlus4;           //reference to the containing CPlus4
 unsigned short     ChipModel;   //values: MOS7360(HMOS-I:Intel,1976,3micron,depletion-mode-load)
                                 //      / MOS8360(HMOS-II:Intel,1979,2micron,depletion-mode-load)
                                 // (is there a difference in output linearity or drive-strength?)
 unsigned short     BaseAddress; //TED-baseaddress location in CPlus4-memory (IO)
 unsigned char*     BasePtr;     //TED-baseaddress location in host's memory
 unsigned char*     BasePtrWR;   //TED-baseaddress location in host's memory for writing
 unsigned char*     BasePtrRD;   //TED-baseaddress location in host's memory for reading
 //Video
 unsigned short     RasterLines;
 unsigned char      RasterRowCycles;
 unsigned char      RowCycleCnt;
 //Audio
 char               AudioClock; //~1MHz CPU-clock / 4
 int                FreqCounters[2];
 char               FreqCounterOverflows[2];
 char               PrevFreqCounterOverflows[2];
 char               PulseStates[2];
 unsigned char      NoiseLFSR;
 int                SoundDecayCounters[2];
 int                MainOutput;
 int                Level;      //filtered version, good for VU-meter display
 unsigned char      ChannelMutes[2]; //0:ON, 1:Mute
#ifdef CRTED_PLATFORM_PC
 char               ScopeData[CRTED_OSCILLOSCOPE_WIDTH_MAX];
#endif
 //Timers (counting itself is done in memory $FF00..$FF05)
 char               TimerRunning[3];
};


struct cRTED_Output {
 signed int L;
 signed int R;
};


struct cRTED_CPlus4instance {
 //platform-related:
 unsigned char     CLImode;
 unsigned short    SampleRate;
 unsigned int      BufferSize;
 //CPlus4-machine (mainboard) related:
 unsigned char     VideoStandard; //0:NTSC, 1:PAL (based on the TED-header field)
 unsigned int      CPUfrequency;
 unsigned char     CPUspeedMul; //TEDCLOCK/4 vs 1x or 2x depending on TED (if in border)
 unsigned short    SampleClockRatio; //ratio of CPU-clock and samplerate
 unsigned short    SampleClockRatioReciproc; //ratio of CPU-clock and samplerate
 unsigned char     MainVolume;
 char              Finished;
 char              Returned;
 unsigned char     IRQ; //collected IRQ line from devices
 unsigned char     NMI; //collected NMI line from devices
 char              ROMenabled;
 //TED-file related
 cRTED_TEDheader*  TEDheader;
 char              RealTEDmode;
 char              BASICmode;
 unsigned char     SubTune;
 unsigned short    LoadAddress;
 unsigned short    InitAddress;
 unsigned short    PlayAddress;
 unsigned short    EndAddress;
 char              TimerSource; //for current subtune, (as in TED-header: 0:PAL Vblank(50Hz), 1:NTSC Vblank(60Hz), 2:PAL Timer1,
                                //                       3:NTSC Timer1, 4:PAL Timer1(set by init), 5:NTSC Timer1(set by init)
 char              FileNameOnly [CRTED_FILENAME_LEN_MAX];
 int               TEDfileSize;
 unsigned short    TEDdataOffset;
 cRTED_TEDheader   DummyTEDheader; //for header-less PRG files with BASIC-starter
 unsigned char     TEDfileData [CRTED_FILESIZE_MAX];
 //PTED/RTED-playback-related
 unsigned char     SoundStarted;
 int               FrameCycles;
 int               FrameCycleCnt; //this is a substitution in PTED-mode for TED timers/rastercounters
 short             SampleCycleCnt;
 short             SampleCycleCnt2;
 //playback-control related
 unsigned char     PlaybackSpeed;
 unsigned char     Paused;
 int               KeyPressDelay;
 int               KeyPressDelayCounter;
 unsigned char     PressedKeySymbol;
 char              MuteBeforeKeypress;
 unsigned char     SubtuneKeyPresses[CRTED_SUBTUNE_AMOUNT_MAX+1];
 //playlist-related
 unsigned short    SubtuneDurations[CRTED_SUBTUNE_AMOUNT_MAX+1];
 short             TenthSecondCnt;
 unsigned short    SecondCnt;
 short             PlayTime;
 char              PlaytimeExpired;
 char              AutoAdvance;
 char              AutoExit;
 char              FadeOut;
 char              FadeLevel;
 char              BuiltInMusic;
 int               PlayListSize;
 int               PlayListPlayPosition; //currently played entry
 int               PlayListDisplayPosition; //top of the list
 char              PlayListAdvance;
 char              PlayList[CRTED_PLAYLIST_ENTRIES_MAX][CRTED_PLAYLIST_ENTRY_SIZE];
 unsigned int      PlayListNumbering[CRTED_PLAYLIST_ENTRIES_MAX];
 unsigned int      PlayListNumber;

 //Hardware-elements:
 cRTED_CPUinstance CPU;
 cRTED_TEDinstance TED;
 //Overlapping system memories, which one is read/written in an address region depends on bankselector fake registers $FF3E/$FF3F)
 unsigned char RAMbank[0x10100];  //$0000..$FFFF RAM (and RAM under IO/ROM/CPUport)
 unsigned char *IObank; //$FD00..$FF40 IO-RAM (registers) to write/read (TED/IO), set the same as IObankRD
 unsigned char *IObankRD; //$FD00..$FF40 IO-RAM (registers) to read from (TED/IO)
 unsigned char IObankWR[0x10100]; //$FD00..$FF40 IO-RAM (registers) to write (TED/IO), differs from IObankRD for some TED-registers
 unsigned char ROMbanks[0x10100]; //$8000..$FD00/$FF40..$FFFF (KERNAL,BASIC)
};


#endif //LIBCRTED_HEADER
