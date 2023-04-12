//anti-aliased GUI library by Hermit (for SDL)

typedef unsigned char byte; //typedef uint8_t byte;
typedef unsigned short word; //typedef uint16_t word;
typedef unsigned int  dword; //typedef uint32_t word;

enum Defaults { WINWIDTH=320, PLAYER_HEIGHT=240, PLAYLIST_ROWS=10, PAGEUP_ROWS=PLAYLIST_ROWS-1, SCROLLSTEPS=5,
                BYTES_PER_PIXEL=4, BITS_PER_PIXEL=(BYTES_PER_PIXEL*8),
                ERRORCODE_DEFAULT=-1
              };

#include <math.h>

static unsigned char* HelpText = "   cRTED TED music player by Hermit\n"
                                 "\n"
                                 "-To play .ted/.tmf/.prg files, give filename\n"
                                 " as argument or associate them to crted.\n"
                                 "-You can give a textual .tel file containing\n"
                                 " a filelist to have a customized playlist.\n"
                                 "-If you give a foldername instead (even '.'),\n"
                                 " all the TED music-files in the subfolders\n"
                                 " will be added to the playlist automatically.\n"
                                 "-If no argument is given to crted it seeks\n"
                                 " music/playlist-files in the current folder.\n"
                                 "(You can combine all these, see README.)\n";

#include "layout.c"


static char *mouse_xpm[] = {
"32 32 3 1 0 0",
"0	c #000000",
"1	c #FFFFFF",
" 	c None",
"00                              ",
"010                             ",
"01100                           ",
"011100                          ",
"0111100                         ",
"01110100                        ",
"010111100                       ",
"0111111100                      ",
"01111111100                     ",
"01111111100                     ",
"01111111100                     ",
"01111111000                     ",
"0011111000                      ",
" 00000100                       ",
"  000010                        ",
"      010                       ",
"      010                       ",
"       01                       ",
"         1                      ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                "
};

#include "graphics/cRTED-icon-16x16.xpm"


// SDL interprets each pixel as a 32-bit number, so our masks must depend on the endianness (byte order) of the machine
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
 Uint32 rmask = 0xff000000, gmask = 0x00ff0000, bmask = 0x0000ff00, amask = 0x000000ff;
#else
 Uint32 rmask = 0x000000ff, gmask = 0x0000ff00, bmask = 0x00ff0000, amask = 0xff000000;
#endif

static unsigned char icon_pixels[16*16*32]; static SDL_Surface *cRTED_Icon;
static SDL_Cursor *MouseCursor;
static SDL_Event Event;
static char ExitSignal=0;
static byte FirstDraw=1;
static short WinHeight = PLAYER_HEIGHT;


void XPMtoPixels(char* source[], unsigned char* target) {
 int i,j,k,sourceindex,targetindex,width,height,colours,charpix,transpchar=0xff;
 unsigned int colr[256],colg[256],colb[256];
 unsigned char colchar[256], colcode,colindex;
 sscanf(source[0],"%d %d %d %d",&width,&height,&colours,&charpix);
 for (i=0;i<colours;i++) {
  if (sscanf(source[1+i],"%c%*s #%2X%2X%2X",&colchar[i],&colr[i],&colg[i],&colb[i])!=4) transpchar=colchar[i];
 }
 for (i=0;i<height;i++) {
  for (j=0;j<width;j++) {
   sourceindex=(i*width+j); colcode=source[colours+1+i][j]; for(k=0;k<colours;k++) if (colcode==colchar[k]) break; colindex=k;
   targetindex=(i*width+j)*4;
   target[targetindex+0] = colr[colindex]; //Red
   target[targetindex+1] = colg[colindex]; //Green
   target[targetindex+2] = colb[colindex]; //Blue
   target[targetindex+3] = (colcode==transpchar)?0x00:0xff ;//Aplha - 0:fully transparent...0xFF:fully opaque
 }}
}


static SDL_Cursor *init_system_cursor(char *image[]) { // Create a new SDL mouse-cursor from an XPM
 int i, row, col; Uint8 data[4*32]; Uint8 mask[4*32]; int hot_x, hot_y;
 for ( i=-1, row=0; row<32; ++row ) {
  for ( col=0; col<32; ++col ) {
   if ( col % 8 ) { data[i] <<= 1; mask[i] <<= 1; }
   else { ++i; data[i] = mask[i] = 0; }
   switch (image[4+row][col]) {
    case '0': data[i] |= 0x01; mask[i] |= 0x01; break;
    case '1': mask[i] |= 0x01; break;
    case ' ': break;
 }}}
 hot_x=hot_y=0; //sscanf(image[4+row], "%d,%d", &hot_x, &hot_y);
 return SDL_CreateCursor(data, mask, 32, 32, hot_x, hot_y);
}


void initGUI() {
 if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
  fprintf(stderr, "Couldn't initialize SDL-Video: %s\n",SDL_GetError()); exit(-1);
 }

 SDL_WM_SetCaption("cRTED-"VERSION" CPlus4 TED-music player","cRTED-"VERSION);

 XPMtoPixels(cRTED_xpm, icon_pixels);
 cRTED_Icon = SDL_CreateRGBSurfaceFrom( (void*)icon_pixels, 16, 16, 32, 16*4, rmask, gmask, bmask, amask );
 SDL_WM_SetIcon(cRTED_Icon, NULL);

 MouseCursor = init_system_cursor(mouse_xpm); SDL_SetCursor(MouseCursor);

 WinHeight = PLAYER_HEIGHT + ( (cRTED_CPlus4.PlayListSize>1 || cRTED_CPlus4.BuiltInMusic)? PLAYLIST_HEIGHT:0 );
 Screen = SDL_SetVideoMode( WINWIDTH, WinHeight, BITS_PER_PIXEL, SDL_SWSURFACE ); //|SDL_HWACCEL|SDL_ANYFORMAT);
 if(Screen==NULL) { printf("Couldn't create SDL window: %s",SDL_GetError()); exit(ERRORCODE_DEFAULT); }

 SDL_EnableKeyRepeat(250,40);
}


void drawGUI (cRTED_CPlus4instance *CPlus4) {
 static word angle=0;
 Pixels = Screen->pixels;

 if (FirstDraw) {
  FirstDraw=0; drawOnce(CPlus4); SDL_UpdateRect( Screen, 0, 0, WINWIDTH, TED1INFO_Y ); drawPlayList();
 }

 drawRepeatedly(CPlus4); SDL_UpdateRect( Screen, 0, FRAMESPEEDINFO_Y, WINWIDTH, PLAYER_HEIGHT-FRAMESPEEDINFO_Y );
}


void startSubTune(byte subtune) { cRTED_startSubtune(subtune); FirstDraw=1; }

void startNextPlayListItem () {
 cRTED_nextTune(); FirstDraw=1; //drawOnce(CPlus4);
}

void startPrevPlayListItem () {
 cRTED_prevTune(); FirstDraw=1; //drawOnce(CPlus4);
}

char prevTuneButton() {
 if (cRTED_CPlus4.SubTune>1) { cRTED_prevSubtune(); FirstDraw=1; return 1; }
 else if (cRTED_CPlus4.PlayListSize>1) startPrevPlayListItem();
 return 0;
}

char nextTuneButton() {
 if (cRTED_CPlus4.SubTune<cRTED_CPlus4.TEDheader->SubtuneAmount) { cRTED_nextSubtune(); FirstDraw=1; return 1; }
 else if (cRTED_CPlus4.PlayListSize>1) startNextPlayListItem();
 return 0;
}

void volumeUpButton() {
 if (cRTED_CPlus4.MainVolume+32<255) cRTED_CPlus4.MainVolume+=32; else cRTED_CPlus4.MainVolume=255;
}

void volumeDownButton() {
 if (cRTED_CPlus4.MainVolume-32>0) cRTED_CPlus4.MainVolume-=32; else cRTED_CPlus4.MainVolume=0;
}


void mainLoop (cRTED_CPlus4instance *CPlus4) {
 enum Periods { INPUTSCAN_PERIOD=5, FRAMETIME=20, //ms
                SCREENUPDATE_PERIOD = (FRAMETIME/INPUTSCAN_PERIOD) };

 static short i, ScreenUpdateCounter=0, PrevPressedButton=0; static const SDL_VideoInfo *VideoInfo;
 static Uint8* KeyStates; static int Tmp, MouseX, MouseY;
 static Uint32 MouseButtonStates; static char ScrollBarGrabbed=0, PrevScrollBarGrabbed=0;


 void playListDown() {
  if(CPlus4->PlayListDisplayPosition<CPlus4->PlayListSize-PLAYLIST_ROWS) {++CPlus4->PlayListDisplayPosition; } //drawPlayList();}
 }

 void playListUp() {
  if(CPlus4->PlayListDisplayPosition>0) {--CPlus4->PlayListDisplayPosition; } //drawPlayList();}
 }

 void startPlayListItem (unsigned int itemnumber) {
  cRTED_startPlayListItem(itemnumber); FirstDraw=1; //drawOnce(CPlus4);
 }



 while(!ExitSignal) {
  while (SDL_PollEvent(&Event)) {
   if (Event.type == SDL_QUIT) { ExitSignal=1; }
   else if(Event.type==SDL_KEYDOWN) {
    switch (Event.key.keysym.sym) {
     case SDLK_ESCAPE: ExitSignal=1; break;
     case SDLK_SPACE: CPlus4->PlaybackSpeed=1; CPlus4->Paused^=1;
                      if(CPlus4->Paused) cRTED_pauseTEDtune(); else cRTED_playTEDtune(); break;
     case SDLK_1: startSubTune(1); break;
     case SDLK_2: startSubTune(2); break;
     case SDLK_3: startSubTune(3); break;
     case SDLK_4: startSubTune(4); break;
     case SDLK_5: startSubTune(5); break;
     case SDLK_6: startSubTune(6); break;
     case SDLK_7: startSubTune(7); break;
     case SDLK_8: startSubTune(8); break;
     case SDLK_9: startSubTune(9); break;
     case SDLK_RETURN: startSubTune(CPlus4->SubTune); break;
     case SDLK_PLUS: case SDLK_KP_PLUS: case SDLK_EQUALS: cRTED_nextSubtune(); FirstDraw=1; break;
     case SDLK_MINUS: case SDLK_KP_MINUS: cRTED_prevSubtune(); FirstDraw=1; break;
     case SDLK_UP: volumeUpButton(); break;
     case SDLK_DOWN: volumeDownButton(); break;
     case SDLK_PAGEDOWN: for(i=0;i<PAGEUP_ROWS;++i) playListDown(); drawPlayList(); break;
     case SDLK_PAGEUP: for(i=0;i<PAGEUP_ROWS;++i) playListUp(); drawPlayList(); break;
     case SDLK_HOME: CPlus4->PlayListDisplayPosition=0; drawPlayList(); break;
     case SDLK_END: CPlus4->PlayListDisplayPosition=CPlus4->PlayListSize-PLAYLIST_ROWS; drawPlayList(); break;
     case SDLK_LEFT: prevTuneButton(); break;
     case SDLK_RIGHT: nextTuneButton(); break;
    }
   }
   else if (Event.type == SDL_MOUSEBUTTONDOWN) {
    if (Event.button.button==SDL_BUTTON_LEFT) {
     if (BUTTONS_Y <= Event.button.y && Event.button.y < BUTTONS_Y+PUSHBUTTON_HEIGHT) {
      for(i=0;i<PUSHBUTTON_AMOUNT;++i)
       if (PushButtonX[i]<=Event.button.x && Event.button.x < PushButtonX[i]+PUSHBUTTON_WIDTH) { PressedButton=i+1; break; }
     }
     else if (PLAYLIST_ELEMENTS_Y < Event.button.y && Event.button.y < PLAYLIST_Y+PLAYLIST_HEIGHT-(PLAYLIST_HEIGHT%ELEMENT_HEIGHT)-1) {
      if (Event.button.x < SCROLLBAR_X) {
       startPlayListItem( (Event.button.y-PLAYLIST_ELEMENTS_Y ) / ROWHEIGHT_PLAYLIST + CPlus4->PlayListDisplayPosition );
      }
      else { ScrollBarGrabbed=1; }
     }
    }
    else if (Event.button.button==SDL_BUTTON_RIGHT) {
     if (Event.button.y >= PLAYLIST_ELEMENTS_Y && Event.button.x >= SCROLLBAR_X) {
      for(i=SCROLLSTEPS; i>=0 ; --i) playListUp();  drawPlayList();
     }
    }
    else if (Event.button.button==SDL_BUTTON_WHEELUP) {
     if (Event.button.y<PLAYER_HEIGHT) { if (CPlus4->MainVolume+8<255) CPlus4->MainVolume+=8; else CPlus4->MainVolume=255; }
     else { for(i=SCROLLSTEPS*(Event.button.x>SCROLLBAR_X); i>=0 ; --i) playListUp();  drawPlayList(); }
    }
    else if (Event.button.button==SDL_BUTTON_WHEELDOWN) {
     if (Event.button.y<PLAYER_HEIGHT) { if (CPlus4->MainVolume-8>0) CPlus4->MainVolume-=8; else CPlus4->MainVolume=0; }
     else { for(i=SCROLLSTEPS*(Event.button.x>SCROLLBAR_X); i>=0 ; --i) playListDown(); drawPlayList(); }
    }
   }

  }


  if (PressedButton && PressedButton!=PrevPressedButton) {
   switch (PressedButton) {
    case 1: startSubTune(CPlus4->SubTune); break;
    case 2: CPlus4->PlaybackSpeed=1; CPlus4->Paused^=1;
            if(CPlus4->Paused) cRTED_pauseTEDtune(); else cRTED_playTEDtune(); break;
    case 4: prevTuneButton(); break;
    case 5: nextTuneButton(); break;
    case 6: CPlus4->AutoAdvance^=1; break;
    case 7: CPlus4->TED.ChannelMutes[0]^=1;
            if(CPlus4->TED.ChannelMutes[0] & CPlus4->TED.ChannelMutes[1]) CPlus4->TED.ChannelMutes[1]=0; break;
    case 8: CPlus4->TED.ChannelMutes[1]^=1;
            if(CPlus4->TED.ChannelMutes[0] & CPlus4->TED.ChannelMutes[1]) CPlus4->TED.ChannelMutes[0]=0; break;
   }
  }
  PrevPressedButton=PressedButton;
  MouseButtonStates = SDL_GetMouseState(&MouseX,&MouseY);
  if ((MouseButtonStates & SDL_BUTTON_LMASK)==0) { PressedButton=ScrollBarGrabbed=0; }
  if ( ScrollBarGrabbed && (ScreenUpdateCounter==0 || !PrevScrollBarGrabbed) ) {
   Tmp = (MouseY-SCROLLBAR_Y) * CPlus4->PlayListSize / SCROLLBAR_HEIGHT;
   if (Tmp<0) Tmp=0;  if (Tmp >= CPlus4->PlayListSize-PLAYLIST_ROWS) Tmp = CPlus4->PlayListSize-PLAYLIST_ROWS-1;
   CPlus4->PlayListDisplayPosition = Tmp; drawPlayList();
  }
  PrevScrollBarGrabbed=ScrollBarGrabbed;

  if (CPlus4->PlayListAdvance) startNextPlayListItem();
  else if (CPlus4->PlaytimeExpired) { cRTED_nextSubtune(); FirstDraw=1; }


  KeyStates = SDL_GetKeyState(NULL);
  if ( (KeyStates[SDLK_TAB]
  #ifndef WINDOWS
   && !KeyStates[SDLK_LALT]  //in Linux we protect from Alt+TAB to fast-forward but in Windows the
  #endif                     //ALT-behaviour is buggy (gets stuck) so there we ignore ALT state for fast-fwd
                            ) || KeyStates[SDLK_BACKQUOTE] ) {
   CPlus4->PlaybackSpeed=CRTED_FFWD_SPEED; PressedButton=3;
  }
  else if (PressedButton==3) CPlus4->PlaybackSpeed=CRTED_FFWD_SPEED;
  else CPlus4->PlaybackSpeed=1;


  if(ScreenUpdateCounter<SCREENUPDATE_PERIOD) ++ScreenUpdateCounter;  else { ScreenUpdateCounter=0; drawGUI(CPlus4); }

  SDL_Delay(INPUTSCAN_PERIOD);
 }
}

