
enum WindowTheme { MARGIN_COLOR=0x202020, BACKGROUND_COLOR=MARGIN_COLOR, MARGIN_THICKNESS=4, FIELD_CHARCOUNT_MAX=32,
                   COLOR_HELPTEXT=0xFFFFFF, HELP_GRADIENT_COLOR=0x608070, FONTSIZE_HELPTEXT=10 };
enum OscilloscopeTheme { BGCOLOR_OSCILLOSCOPE=0x08100C, FGCOLOR_OSCILLOSCOPE=0x44CC99 };


#include "widgets.c"


static unsigned char* PushButtonTexts[] = { "\x1E\x1F", "\x1D"/*"1F"*/, "\x1F\x1F", "<", ">", "adv.",  "ch1",  "ch2" };
static byte PushButtonSeparation[] =      {      0,       0,                  0,     2,   2,    2,      4 ,      4   };
static word PushButtonX[] =               {      0,       0,                  0,     0,   0,    0,      0 ,      0   };
static byte PressedButton=0;

enum ButtonLayout { PUSHBUTTON_AMOUNT = sizeof(PushButtonTexts) / sizeof(char*),
                    PUSHBUTTON_HEIGHT = 30, PUSHBUTTON_WIDTH=30, BUTTONS_Y=PLAYER_HEIGHT-MARGIN_THICKNESS*2-PUSHBUTTON_HEIGHT,
                    PUSHBUTTON_SPACING=5, PUSHBUTTON_PADDING=(PUSHBUTTON_WIDTH+PUSHBUTTON_SPACING),
                    ROWHEIGHT_PLAYLIST=ELEMENT_HEIGHT
                  };


//Fields (blocks) & Separators
//enum FieldLayout { };


unsigned char* valueToHexString (word value) {
 static unsigned char String[10];
 sprintf(String,"$%4.4X",value);
 return String;
}


enum Layout {
              FIELDTITLE_X=MARGIN_THICKNESS, FIELDTITLE_WIDTH=50, TEXTFIELD_PADDING=22,
              AUTHOR_Y=MARGIN_THICKNESS, TITLE_Y=(AUTHOR_Y+TEXTFIELD_PADDING),
              RELEASEINFO_Y=(TITLE_Y+TEXTFIELD_PADDING), TOOLINFO_Y=(RELEASEINFO_Y+TEXTFIELD_PADDING),
              TEXTFIELD_X=(MARGIN_THICKNESS+FIELDTITLE_WIDTH), TEXTFIELD_WIDTH=(WINWIDTH-MARGIN_THICKNESS-TEXTFIELD_X),
              LOADINFO_X=FIELDTITLE_X, LOADINFO_Y=(TOOLINFO_Y+TEXTFIELD_PADDING+MARGIN_THICKNESS),
              VALUETITLE_WIDTH=32, VALUEFIELD_WIDTH=50, VALUEFIELD_PADDING=(VALUETITLE_WIDTH+VALUEFIELD_WIDTH+4),
              ENDINFO_X=(LOADINFO_X+VALUEFIELD_PADDING), ENDINFO_Y=LOADINFO_Y,
              SIZEINFO_X=(ENDINFO_X+VALUEFIELD_PADDING), SIZEINFO_Y=LOADINFO_Y,
               SIZEINFO_WIDTH=(WINWIDTH-MARGIN_THICKNESS-(SIZEINFO_X+VALUETITLE_WIDTH)),
              INITINFO_X=FIELDTITLE_X, INITINFO_Y=(SIZEINFO_Y+TEXTFIELD_PADDING),
              PLAYINFO_X=(INITINFO_X+VALUEFIELD_PADDING), PLAYINFO_Y=INITINFO_Y,
              SUBTUNEINFO_X=(PLAYINFO_X+VALUEFIELD_PADDING+4), SUBTUNEINFO_Y=INITINFO_Y, SUBTUNEINFO_WIDTH=VALUEFIELD_WIDTH,
              PLAYTIMEINFO_X=(SUBTUNEINFO_X+SUBTUNEINFO_WIDTH+4), PLAYTIMEINFO_Y=INITINFO_Y, PLAYTIMEINFO_WIDTH=VALUEFIELD_WIDTH,
              STANDARDINFO_X=FIELDTITLE_X, SPEEDINFO_Y=(INITINFO_Y+TEXTFIELD_PADDING+MARGIN_THICKNESS),
               SPEEDTITLE_WIDTH=56, SPEEDINFOFIELD_WIDTH=42,
               SPEEDINFO_PADDING=(SPEEDTITLE_WIDTH+SPEEDINFOFIELD_WIDTH+MARGIN_THICKNESS),
              TIMERINFO_X=(STANDARDINFO_X+SPEEDINFO_PADDING-2), TIMERINFO_Y=SPEEDINFO_Y, TIMERTITLE_WIDTH=50,
              FRAMESPEEDINFO_X=(TIMERINFO_X+TIMERTITLE_WIDTH+SPEEDINFOFIELD_WIDTH+MARGIN_THICKNESS),
              FRAMESPEEDINFO_Y=SPEEDINFO_Y, FRAMESPEEDTITLE_WIDTH=74,
              TED1INFO_X=FIELDTITLE_X, TED1INFO_Y=(SPEEDINFO_Y+TEXTFIELD_PADDING+MARGIN_THICKNESS), TEDINFOFIELD_WIDTH=82,
               SCOPE1_X=(TED1INFO_X+VALUETITLE_WIDTH+TEDINFOFIELD_WIDTH+MARGIN_THICKNESS)-1, SCOPE1_Y=TED1INFO_Y, SCOPE_WIDTH=14,
               SCOPE_HEIGHT=(TEXTFIELD_PADDING-MARGIN_THICKNESS), SCOPE_BGCOLOR=0x081810, SCOPE_FGCOLOR=0x60A080,
              CONTROLS_X=MARGIN_THICKNESS, CONTROLS_Y=(TED1INFO_Y+TEXTFIELD_PADDING+MARGIN_THICKNESS)+5,
              POTMETER_WIDTH=POTMETER_RADIUS+6, POTMETER_X=(WINWIDTH-MARGIN_THICKNESS-POTMETER_WIDTH), POTMETER_Y=CONTROLS_Y+15,
              PLAYTIME_WIDTH=44, PLAYTIME_X=WINWIDTH-PLAYTIME_WIDTH-MARGIN_THICKNESS, PLAYTIME_Y=SCOPE1_Y,
              OSCILLOSCOPE_X = (SCOPE1_X+SCOPE_WIDTH+4),
              OSCILLOSCOPE_WIDTH = (WINWIDTH-OSCILLOSCOPE_X-MARGIN_THICKNESS-PLAYTIME_WIDTH*2+8),
              PLAYLIST_Y=PLAYER_HEIGHT, PLAYLIST_X=0, PLAYLIST_ELEMENTS_Y=PLAYLIST_Y+FIELDTITLE_HEIGHT,
              SCROLLBAR_Y=PLAYLIST_ELEMENTS_Y, SCROLLBAR_X=WINWIDTH-SCROLLBAR_THICKNESS-MARGIN_THICKNESS,
              PLAYLIST_WIDTH=WINWIDTH, PLAYLIST_HEIGHT=(PLAYLIST_ROWS*ROWHEIGHT_PLAYLIST+FIELDTITLE_HEIGHT+MARGIN_THICKNESS),
              SCROLLBAR_HEIGHT=PLAYLIST_HEIGHT-FIELDTITLE_HEIGHT
            };


void drawOnce (cRTED_CPlus4instance *CPlus4) {

 static unsigned char String[32]; static word Tmp;
 static SDL_Rect BackGroundRectangle;


 BackGroundRectangle.x=BackGroundRectangle.y=0; BackGroundRectangle.w=WINWIDTH; BackGroundRectangle.h=TED1INFO_Y;
 SDL_FillRect(Screen, &BackGroundRectangle, MARGIN_COLOR); //SDL_FillRect(Screen, &Screen->clip_rect, MARGIN_COLOR);


 drawFieldTitle( FIELDTITLE_X, AUTHOR_Y, FIELDTITLE_WIDTH, "Author:" );
  drawTextField( TEXTFIELD_X, AUTHOR_Y, TEXTFIELD_WIDTH, cRTED_getTEDtext(CPlus4->TEDheader->Author) );

 drawFieldTitle( FIELDTITLE_X, TITLE_Y, FIELDTITLE_WIDTH, "Title:");
  drawTextField( TEXTFIELD_X, TITLE_Y, TEXTFIELD_WIDTH, cRTED_getTEDtext(CPlus4->TEDheader->Title) );

 drawFieldTitle( FIELDTITLE_X, RELEASEINFO_Y, FIELDTITLE_WIDTH, "Release:");
  drawTextField( TEXTFIELD_X, RELEASEINFO_Y, TEXTFIELD_WIDTH, cRTED_getTEDtext(CPlus4->TEDheader->ReleaseInfo) );

 drawFieldTitle( FIELDTITLE_X, TOOLINFO_Y, FIELDTITLE_WIDTH, "Tool:");
  drawTextField( TEXTFIELD_X, TOOLINFO_Y, TEXTFIELD_WIDTH, cRTED_getTEDtext(CPlus4->TEDheader->Tool) );


 drawHorizontalSeparator ( MARGIN_THICKNESS, LOADINFO_Y-MARGIN_THICKNESS-1, WINWIDTH-MARGIN_THICKNESS);


 drawFieldTitle( LOADINFO_X, LOADINFO_Y, VALUETITLE_WIDTH, "Load:");
  drawTextField( LOADINFO_X+VALUETITLE_WIDTH, LOADINFO_Y, VALUEFIELD_WIDTH, valueToHexString(CPlus4->LoadAddress) );

 drawFieldTitle( ENDINFO_X, ENDINFO_Y, VALUETITLE_WIDTH, "End:");
  drawTextField( ENDINFO_X+VALUETITLE_WIDTH, ENDINFO_Y, VALUEFIELD_WIDTH, valueToHexString(CPlus4->EndAddress) );

 drawFieldTitle( SIZEINFO_X, SIZEINFO_Y, VALUETITLE_WIDTH, "Size:");
  Tmp=CPlus4->EndAddress-CPlus4->LoadAddress; sprintf( String, "$%4.4X (%d)", Tmp, Tmp );
  drawTextField( SIZEINFO_X+VALUETITLE_WIDTH, SIZEINFO_Y, SIZEINFO_WIDTH, String );

 drawFieldTitle( INITINFO_X, INITINFO_Y, VALUETITLE_WIDTH, "Init:");
  drawTextField( INITINFO_X+VALUETITLE_WIDTH, INITINFO_Y, VALUEFIELD_WIDTH, valueToHexString(CPlus4->InitAddress) );

 drawFieldTitle( PLAYINFO_X, PLAYINFO_Y, VALUETITLE_WIDTH, "Play:");
  drawTextField( PLAYINFO_X+VALUETITLE_WIDTH, PLAYINFO_Y, VALUEFIELD_WIDTH+9,
                 (!CPlus4->RealTEDmode)? valueToHexString(CPlus4->PlayAddress) : (unsigned char*) "RealTED" );

 if (!CPlus4->BASICmode) {
  drawFieldTitle( SUBTUNEINFO_X, SUBTUNEINFO_Y, VALUETITLE_WIDTH, "Subtune:" );
   sprintf( String, "%d/%d", CPlus4->SubTune, CPlus4->TEDheader->SubtuneAmount );
   drawCenteredTextField( SUBTUNEINFO_X+VALUETITLE_WIDTH, SUBTUNEINFO_Y, SUBTUNEINFO_WIDTH, String );
  if (CPlus4->SubtuneDurations[CPlus4->SubTune]) {
   sprintf ( String, "%2.2d:%2.2d", CPlus4->SubtuneDurations[CPlus4->SubTune] / 60, CPlus4->SubtuneDurations[CPlus4->SubTune] % 60 );
   drawCenteredTextField( PLAYTIMEINFO_X+VALUETITLE_WIDTH, PLAYTIMEINFO_Y, PLAYTIMEINFO_WIDTH, String );
  }
  drawHorizontalSeparator ( MARGIN_THICKNESS, SPEEDINFO_Y-MARGIN_THICKNESS-1, WINWIDTH-MARGIN_THICKNESS);
 }
 else drawTextField( SUBTUNEINFO_X+MARGIN_THICKNESS, PLAYTIMEINFO_Y, WINWIDTH-SUBTUNEINFO_X-MARGIN_THICKNESS*2, "BASIC SYS launch" );

 drawFieldTitle( STANDARDINFO_X, SPEEDINFO_Y, SPEEDTITLE_WIDTH, "Standard:");
  drawCenteredTextField( STANDARDINFO_X+SPEEDTITLE_WIDTH, SPEEDINFO_Y, SPEEDINFOFIELD_WIDTH, CPlus4->VideoStandard? "NTSC":"PAL" );

 drawFieldTitle( TIMERINFO_X, TIMERINFO_Y, TIMERTITLE_WIDTH, "Timer:");
  drawCenteredTextField( TIMERINFO_X+TIMERTITLE_WIDTH-6, TIMERINFO_Y, SPEEDINFOFIELD_WIDTH+5,
                         CPlus4->RealTEDmode? "?" : (CPlus4->TimerSource? "Timer":"Raster") );

 drawFieldTitle( FRAMESPEEDINFO_X, FRAMESPEEDINFO_Y, FRAMESPEEDTITLE_WIDTH, "FrameSpeed:"); //might change during playback (refresh)?


 drawHorizontalSeparator ( MARGIN_THICKNESS, TED1INFO_Y-MARGIN_THICKNESS-1, WINWIDTH-MARGIN_THICKNESS);


}


void drawPlayList () {
 if (cRTED_CPlus4.BuiltInMusic==0) {
  if (cRTED_CPlus4.PlayListSize<2 ) return;
  drawSelector (PLAYLIST_X, PLAYLIST_Y, WINWIDTH, PLAYLIST_HEIGHT, SCROLLSIDE_RIGHT, "PlayList",
                (char*)cRTED_CPlus4.PlayList, cRTED_CPlus4.PlayListNumbering,
                CRTED_PLAYLIST_ENTRY_SIZE, cRTED_CPlus4.PlayListDisplayPosition,
                cRTED_CPlus4.PlayListPlayPosition, cRTED_CPlus4.PlayListSize );
 }
 else { //draw help
  setColor(HELP_GRADIENT_COLOR); setGradientColor(BACKGROUND_COLOR);
  drawGradientBox (PLAYLIST_X, PLAYLIST_Y, WINWIDTH, PLAYLIST_HEIGHT, GRADIENT_DISTANCE);
  setColor(COLOR_HELPTEXT); setFontSize(FONTSIZE_HELPTEXT);
  drawTextBox(PLAYLIST_X+2,PLAYLIST_Y,WINWIDTH,PLAYLIST_HEIGHT,HelpText);
 }
 SDL_UpdateRect( Screen, 0, PLAYLIST_Y, WINWIDTH, PLAYLIST_HEIGHT );
}


void drawRepeatedly (cRTED_CPlus4instance *CPlus4) {

 static byte i; static unsigned char String[32];
 static SDL_Rect BackGroundRectangle;


 BackGroundRectangle.x=0; BackGroundRectangle.y=TED1INFO_Y;
 BackGroundRectangle.w=WINWIDTH; BackGroundRectangle.h=PLAYER_HEIGHT-TED1INFO_Y;
 SDL_FillRect(Screen, &BackGroundRectangle, MARGIN_COLOR);


 if (!CPlus4->RealTEDmode) sprintf( String, "%.1fx", (CPlus4->VideoStandard<=1? 17734.0:14915.0) / CPlus4->FrameCycles );
 drawCenteredTextField( FRAMESPEEDINFO_X+FRAMESPEEDTITLE_WIDTH, FRAMESPEEDINFO_Y,
                        SPEEDINFOFIELD_WIDTH, (!CPlus4->RealTEDmode)? String:(unsigned char*)"?" );


 drawFieldTitle( TED1INFO_X, TED1INFO_Y, VALUETITLE_WIDTH, "TED:");
  sprintf( String, "$%4.4X,%d", CPlus4->TED.BaseAddress, CPlus4->TED.ChipModel);
  drawTextField( TED1INFO_X+VALUETITLE_WIDTH, TED1INFO_Y, TEDINFOFIELD_WIDTH, String );
 drawVUmeter( SCOPE1_X, SCOPE1_Y, SCOPE_WIDTH, SCOPE_HEIGHT, CPlus4->TED.Level>>6 );
 drawHorizontalScope (OSCILLOSCOPE_X, SCOPE1_Y, OSCILLOSCOPE_WIDTH, SCOPE_HEIGHT,
                      BGCOLOR_OSCILLOSCOPE, FGCOLOR_OSCILLOSCOPE, CPlus4->TED.ScopeData);


 drawHorizontalSeparator ( MARGIN_THICKNESS, TED1INFO_Y+TEXTFIELD_PADDING-1, WINWIDTH-MARGIN_THICKNESS);


 PushButtonTexts[1] = CPlus4->Paused ? "\x1F" : "\x1D";
 PushButtonTexts[5] = CPlus4->AutoAdvance ? "rep": "adv";
 for (i=0; i<PUSHBUTTON_AMOUNT; ++i) {
  drawButton( PushButtonX[i] = CONTROLS_X + PUSHBUTTON_PADDING*i + PushButtonSeparation[i],
              CONTROLS_Y, PUSHBUTTON_WIDTH, PUSHBUTTON_HEIGHT, PushButtonTexts[i],
              i>=6? CPlus4->TED.ChannelMutes[i-6] : PressedButton==i+1 );
 }

 drawFieldTitle( PLAYTIME_X-PLAYTIME_WIDTH+5, PLAYTIME_Y, PLAYTIME_WIDTH, "Time:");
  sprintf ( String, "%2.2d:%2.2d", CPlus4->PlayTime/60, CPlus4->PlayTime%60 );
  drawTextField( PLAYTIME_X, PLAYTIME_Y, PLAYTIME_WIDTH, String );

 drawPotMeter ( POTMETER_X, POTMETER_Y, CPlus4->MainVolume, "Vol.", VALUEMODE_PERCENTAGE );

}
