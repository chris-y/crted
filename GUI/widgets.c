//Widgets (with styling, gradients)

#include "primitives.c"

//Common parts for widgets

enum WidgetTheme { COLOR_FIELDTITLE=0xC0B080, FONTSIZE_FIELDTITLE=8, MARGIN_FIELDTITLE=3,
                   FIELDTITLE_HEIGHT = FONTSIZE_FIELDTITLE+MARGIN_FIELDTITLE*2,
                   BACKGROUND_COLOR_TEXTFIELD=0x304020, TEXTCOLOR_TEXTFIELD=0xC0E0F0, FONTSIZE_TEXTFIELD=10
                 };

//Graphics (vector/raster)

//Menubutton

//Pager
enum PagerTheme { COLOR_PAGEBUTTON_SIDE=0x404040, COLOR_PAGEBUTTON_CENTER=0x606468, COLORALPHA_PAGEBUTTON_TEXT=0xA0E8F0F8,
                  COLOR_ACTIVE_PAGEBUTTON_TOP=0x404040, COLOR_ACTIVE_PAGEBUTTON_BOTTOM=BACKGROUND_COLOR,
                  COLOR_ACTIVE_PAGEBUTTON_TEXT=0xFFFFFF, RADIUS_PAGEBUTTON_TOP=3, FONTSIZE_PAGEBUTTON=10 };

void drawPageButton (word x, word y, word w, word h, char* text, char active) {
 if(active) { setColor(COLOR_ACTIVE_PAGEBUTTON_TOP); setGradientColor(COLOR_ACTIVE_PAGEBUTTON_BOTTOM); }
 else setColor(COLOR_PAGEBUTTON_SIDE);
 drawBox( x+RADIUS_PAGEBUTTON_TOP, y, w-RADIUS_PAGEBUTTON_TOP*2, RADIUS_PAGEBUTTON_TOP );
 drawQuarterDisc( x+RADIUS_PAGEBUTTON_TOP, y+RADIUS_PAGEBUTTON_TOP, RADIUS_PAGEBUTTON_TOP, 1);
 drawQuarterDisc( x+w-RADIUS_PAGEBUTTON_TOP-1, y+RADIUS_PAGEBUTTON_TOP, RADIUS_PAGEBUTTON_TOP, 0);
 if (active) drawGradientBox( x, y+RADIUS_PAGEBUTTON_TOP, w, h-RADIUS_PAGEBUTTON_TOP, GRADIENT_HORIZONTAL );
 else {
  drawBox( x+RADIUS_PAGEBUTTON_TOP, y+h-RADIUS_PAGEBUTTON_TOP, w-RADIUS_PAGEBUTTON_TOP*2, RADIUS_PAGEBUTTON_TOP );
  drawQuarterDisc( x+RADIUS_PAGEBUTTON_TOP, y+h-RADIUS_PAGEBUTTON_TOP-1, RADIUS_PAGEBUTTON_TOP, 2);
  drawQuarterDisc( x+w-RADIUS_PAGEBUTTON_TOP-1, y+h-RADIUS_PAGEBUTTON_TOP-1, RADIUS_PAGEBUTTON_TOP, 3);
  setColor(COLOR_PAGEBUTTON_CENTER); setGradientColor(COLOR_PAGEBUTTON_SIDE);
  drawGradientBox( x, y+RADIUS_PAGEBUTTON_TOP, w, h-RADIUS_PAGEBUTTON_TOP*2, GRADIENT_CENTER );
 }
 if(active) setColor(COLOR_ACTIVE_PAGEBUTTON_TEXT); else setColorAlpha(COLORALPHA_PAGEBUTTON_TEXT);
 setFontSize(10); drawCenteredString( x, y, w, h, text );
}


//Separator-Line
enum SeparatorTheme { SEPARATOR_SHADOW=128, SEPARATOR_EDGELIGHT=20 };

void drawHorizontalSeparator (word x1, word y, word x2) {
 setColorAlpha((SEPARATOR_SHADOW<<24)); drawHorizontalLine(x1,y,x2);
 setColorAlpha((SEPARATOR_EDGELIGHT<<24)+0xFFFFFF); drawHorizontalLine(x1,y+1,x2);
}

void drawVerticalSeparator (word x, word y1, word y2) {
 setColorAlpha((SEPARATOR_SHADOW<<24)); drawVerticalLine(x,y1,y2);
 setColorAlpha((SEPARATOR_EDGELIGHT<<24)+0xFFFFFF); drawVerticalLine(x+1,y1,y2);
}

void drawFieldSeparator (word x1, word y1, word x2, word y2) {
 setColorAlpha((SEPARATOR_SHADOW<<24)); drawHorizontalLine(x1+1,y1,x2-1); drawHorizontalLine(x1+1,y2,x2-1);
 drawVerticalLine(x1,y1+1,y2-1); drawVerticalLine(x2,y1+1,y2-1);
 setColorAlpha((SEPARATOR_EDGELIGHT<<24)+0xFFFFFF); drawHorizontalLine(x1+2,y1+1,x2-2); drawHorizontalLine(x1+1,y2+1,x2);
 drawVerticalLine(x1+1,y1+2,y2-3); drawVerticalLine(x2+1,y1+1,y2);
}

/*
void drawBlockFill(hWindow *hw, int x1, int y1, int x2, int y2, unsigned long color)
{ drawBlock(hw,x1-1,y1-1,x2,y2); SETDRAWCOL(color); DRAWRECTFILL(x1,y1,x2-x1,y2-y1); }
*/


//Titles(Group,Track)
void drawFieldTitle (word x, word y, word w, char* title) {
 setColor(COLOR_FIELDTITLE); setFontSize(FONTSIZE_FIELDTITLE); drawCenteredString(x,y+MARGIN_FIELDTITLE+2,w,FontHeight,title);
}

//TextField
void drawTextField (word x, word y, word w, char* title) {
 setColor(BACKGROUND_COLOR_TEXTFIELD); setFontSize(FONTSIZE_TEXTFIELD); drawRoundedBox( x, y, w, FontHeight+MARGIN_THICKNESS/2, 4 );
 setColor(TEXTCOLOR_TEXTFIELD); drawStringPart( x+MARGIN_THICKNESS, y+MARGIN_THICKNESS/2+1, w, FIELD_CHARCOUNT_MAX, title);
}

void drawCenteredTextField (word x, word y, word w, char* title) {
 setColor(BACKGROUND_COLOR_TEXTFIELD); setFontSize(FONTSIZE_TEXTFIELD); drawRoundedBox( x, y, w, FontHeight+MARGIN_THICKNESS/2, 4 );
 setColor(TEXTCOLOR_TEXTFIELD); drawCenteredString( x, y+MARGIN_THICKNESS/2+1, w, FontHeight, title);
}

//PushButton(MuteSolo,Transport)
enum ButtonTheme { COLOR_BUTTON=0xA0A0A0,        COLOR_BUTTON_GRADIENT=0x405060,
                   COLOR_BUTTON_PUSHED=0x707070, COLOR_BUTTON_PUSHED_GRADIENT=0x808080,
                   COLOR_BUTTONTEXT=0x102010, FONTSIZE_BUTTONTEXT=12, RADIUS_BUTTON=3 };
void drawButton (word x, word y, word w, word h, char* title, byte state) {
 if (state) { setColor(COLOR_BUTTON_PUSHED); } //setGradientColor(COLOR_BUTTON_PUSHED_GRADIENT); }
 else { setColor(COLOR_BUTTON); }
  setGradientColor(COLOR_BUTTON_GRADIENT); //}
 drawGradientBox(x,y,w,h,GRADIENT_DISTANCE); //state?GRADIENT_DIAGONAL:GRADIENT_DISTANCE);
 setRGBcolorAlpha(0,0,0,0.8); drawRoundedRectangle(x-1,y-1,w+2,h+2,RADIUS_BUTTON);
 if(state) { setRGBcolorAlpha(0,0,0,0.4); drawRectangle(x,y,x+w,y+h); }
 setColor(COLOR_BUTTONTEXT); setFontSize(FONTSIZE_BUTTONTEXT); drawCenteredString(x,y+state,w+1,h,title);
}


//Valuedisplay(Tempo/Time)

//ValueList(Pitch,PW,Cutoff/ec)

//NumberList(Frame-TimeLine)

//StringDisplay(Orderlist,LCD,etc.)


//Scrollbar (hor, ver)
enum ScrollBarTheme { SCROLLBAR_COLOR=0x106080, SCROLLBAR_BUTTON_COLOR=0x60A0E0, SCROLLBAR_PLAYPOS_COLOR=0x102040,
                      SCROLLBAR_THICKNESS=10 };

void drawVerticalScrollBar (word x, word y, word h, word buttonpos, word buttonsize, word playpos) {
 setColor(SCROLLBAR_COLOR); drawVerticalBox( x, y, SCROLLBAR_THICKNESS, h );
 setColor(SCROLLBAR_PLAYPOS_COLOR); drawHorizontalLine (x, y+playpos, x+SCROLLBAR_THICKNESS-1);
 setColor(SCROLLBAR_BUTTON_COLOR); drawVerticalBox( x+3, y+buttonpos, SCROLLBAR_THICKNESS-4, buttonsize);
}

void drawHorizontalScrollBar (word x, word y, word w) {
 setColorAlpha(SCROLLBAR_COLOR); drawBox( x, y, w, SCROLLBAR_THICKNESS );
}


//Oscilloscope
void drawHorizontalScope (word x, word y, word w, word h, dword bgcolor, dword fgcolor, char* data) {
 static byte i, m; static float Yratio;
 setColor(bgcolor); drawRoundedBox( x, y, w, h, 2 );
 setColor(fgcolor); m=y+h/2; Yratio=255/h;
 for (i=1; i<w; ++i) drawRawPixel( x+i, m - (data[i]/Yratio) );
}

void drawVerticalScope (word x, word y, word w, word h) {
 setColor(0x040C08); drawRoundedBox( x, y, w, h, 4 );
 setColor(0x103020); drawVerticalLine( x+w/2, y+1, y+h-1 );
}


//Envelope-display+editor
void drawEnvelope (word x, word y, word w, word h, dword bgcolor, dword fgcolor) {
 setColor(bgcolor); drawRoundedBox( x, y, w, h, 4 );
 setColor(fgcolor); drawLine( x+1, y+h*3/4, x+w-1, y+h/4 );
}


//Switch/ToggleButton (selector, 2 or more states)

//LED (bit-display)


//Potmeter/Slider (with scale, detent, etc.)
enum PotStyle { POTMETER_RADIUS=8, POTMETERMIN_ANGLE=225, POTMETERMAX_ANGLE=-45, POTMETER_RANGE=POTMETERMIN_ANGLE-POTMETERMAX_ANGLE,
                POTMETER_TITLEWIDTH=44, POTMETER_FONTSIZE=8,
                POTMETER_SHAFTCOLOR1=0xC0B0C0, POTMETER_SHAFTCOLOR2=0x407080, POTMETER_SCALECOLOR=0x30C050,
                VALUEMODE_DECBYTE=0, VALUEMODE_HEXBYTE=1, VALUEMODE_DECNYBBLE=2, VALUEMODE_HEXNYBBLE=3, VALUEMODE_PERCENTAGE=4 };

void drawPotMeter (word x, word y, word value, char* title, byte valuemode) {
 static short angle; static char ValueString[10];
 angle = POTMETERMIN_ANGLE - ( ( POTMETER_RANGE * ( (valuemode==VALUEMODE_DECNYBBLE||valuemode==VALUEMODE_HEXNYBBLE) ? (value<<4):value ) ) >> 8 );
 setColor(POTMETER_SHAFTCOLOR1); setGradientColor(POTMETER_SHAFTCOLOR2); drawGradientDisc( x, y, POTMETER_RADIUS );
 setColorAlpha(0xE0000000); drawAngledLine ( x, y, POTMETER_RADIUS, angle ); drawCircle( x, y, POTMETER_RADIUS+2 );
 setColor(POTMETER_SCALECOLOR);
 if (angle>=0) drawArc( x, y, POTMETER_RADIUS+2, angle, POTMETERMIN_ANGLE );
 else { drawArc( x, y, POTMETER_RADIUS+2, 0, POTMETERMIN_ANGLE ); drawArc( x, y, POTMETER_RADIUS+2, 360+angle, 360 ); }
 setColor(COLOR_FIELDTITLE); setFontSize(POTMETER_FONTSIZE);
  drawCenteredString( x-POTMETER_TITLEWIDTH/2, y + POTMETER_RADIUS + 4, POTMETER_TITLEWIDTH, FontHeight, title );
 setColor(0xE0E0E0);
 switch (valuemode) {
  case VALUEMODE_DECBYTE: sprintf(ValueString,"%d",value); break;
  case VALUEMODE_HEXBYTE: sprintf(ValueString,"%2.2X",value); break;
  case VALUEMODE_DECNYBBLE: sprintf(ValueString,"%d",value); break;
  case VALUEMODE_HEXNYBBLE: sprintf(ValueString,"%1.1X",value); break;
  case VALUEMODE_PERCENTAGE: sprintf(ValueString,"%d%%",value*100/255); break;
 }
 drawCenteredString ( x-POTMETER_TITLEWIDTH/2, y - POTMETER_RADIUS - FontHeight - 2, POTMETER_TITLEWIDTH, FontHeight, ValueString );
}


//PitchWheel?

//Progress-bar (magnitude-display)


//Selector (value/text)
//Scrollable-list(Instr,Patt,Subtune)
enum SelectorTheme { SCROLLSIDE_LEFT=0, SCROLLSIDE_RIGHT=1, ELEMENT_FONTSIZE=10, ELEMENT_HEIGHT=ELEMENT_FONTSIZE+5,
                     ELEMENT_CHARS_MAX=10, ELEMENT_NUMBERING_WIDTH = 44,
                     COLOR_SELECTOR_BACKGROUND=0x040810, COLOR_SELECTOR_TITLEBACKGROUND=0x203037, COLOR_FOLDERTEXT=0x405060,
                     COLOR_NUMBERING=0x80B0D0, COLOR_ELEMENTTEXT=0x4060C0, COLOR_HIGHLIGHTED_ELEMENTTEXT=0xC0E0F0,
                     MARGIN_SELECTOR=3, RADIUS_SELECTOR=2 };

void drawSelector(word x, word y, word w, word h, byte scrollside, char* title, char* list, unsigned int *numlist, int elementsize,
                  int position, int highlightposition, int elementcount ) {
 enum SelectorPreCalc { ELEMENTTEXT_SHORTENING = SCROLLBAR_THICKNESS+ELEMENT_NUMBERING_WIDTH };
 static byte i, ElementCountMax; static word Top, LeftSide, Height, ScrollButtonHeight, ElementLeftSide;
 static char ElementText[ELEMENT_CHARS_MAX];

 Top = y + FIELDTITLE_HEIGHT; Height = h-FIELDTITLE_HEIGHT-MARGIN_FIELDTITLE; ElementCountMax = Height/ELEMENT_HEIGHT;
 setColor(BACKGROUND_COLOR); setGradientColor(COLOR_SELECTOR_TITLEBACKGROUND);
 drawGradientBox( x+2, y+2, w-4, FIELDTITLE_HEIGHT, GRADIENT_HORIZONTAL); drawFieldTitle(x,y,w,title);
 setColor(COLOR_SELECTOR_BACKGROUND);
 if (scrollside==SCROLLSIDE_LEFT) {
  LeftSide = x + MARGIN_SELECTOR;
  drawRoundedBox( LeftSide + SCROLLBAR_THICKNESS - 1, Top, w-SCROLLBAR_THICKNESS-MARGIN_SELECTOR*2+1, Height, RADIUS_SELECTOR );
  ElementLeftSide = LeftSide + SCROLLBAR_THICKNESS + 2;
 }
 else {
  LeftSide = w-SCROLLBAR_THICKNESS-MARGIN_SELECTOR;
  drawRoundedBox( x + MARGIN_SELECTOR, Top, LeftSide-MARGIN_SELECTOR, Height, RADIUS_SELECTOR );
  ElementLeftSide = x;
 }
 setColor(COLOR_NUMBERING); setFontSize(ELEMENT_FONTSIZE);
 for (i=0; i<ElementCountMax && i<elementcount; ++i) {
  if (numlist[position+i]) {
   sprintf(ElementText,"%2.2d",numlist[position+i]);
   drawCenteredString( ElementLeftSide, Top + ELEMENT_HEIGHT*i + 1, ELEMENT_NUMBERING_WIDTH, ELEMENT_HEIGHT, ElementText );
 }}
 for (i=0; i<ElementCountMax && i<elementcount; ++i) {
  setColor( position+i==highlightposition? COLOR_HIGHLIGHTED_ELEMENTTEXT : (numlist[position+i]?COLOR_ELEMENTTEXT:COLOR_FOLDERTEXT) );
  drawStringPart( ElementLeftSide + (numlist[position+i]?ELEMENT_NUMBERING_WIDTH:4), Top + ELEMENT_HEIGHT*i, w-ELEMENTTEXT_SHORTENING,
                  CRTED_FILENAME_LEN_MAX, cRTED_fileNameOnly(list+(position+i)*elementsize) ); // "..............." );
 }
 if (elementcount > ElementCountMax) {
  ScrollButtonHeight = ElementCountMax * Height / elementcount; if (ScrollButtonHeight<1) ScrollButtonHeight=1;
  if (scrollside==SCROLLSIDE_LEFT) {
   drawVerticalScrollBar( LeftSide, Top, Height, position * Height / elementcount,
                          ScrollButtonHeight, highlightposition * Height / elementcount );
  }
  else {
   drawVerticalScrollBar( x + LeftSide, Top, Height, position * Height / elementcount,
                          ScrollButtonHeight, highlightposition * Height / elementcount );
 }}
 else {
  if (scrollside==SCROLLSIDE_LEFT)   drawVerticalScrollBar( LeftSide, Top, Height, 0, 0, 0 );
  else drawVerticalScrollBar( x + LeftSide, Top, Height, 0, 0, 0 );
 }
}


//Increase/Decrease


//Dropdown(FxList/SubMenu)


//Piano-roll(Grid,Notes,Effects) with piano-keyboard, horizontal/vertical, separate/combined

//WaveformDisplay


//Curve/diagram-display


//VU-meter
void drawVUmeter (word x, word y, word w, word h, byte value) {
 setColor(0x103040); drawRoundedBox( x, y, w, h, 2 );
 setColor(0xFF4020); setGradientColor(0x20FF40); drawGradientBox( x+2, y+2, w-3, h-4, GRADIENT_HORIZONTAL );
 setColor(0); drawBox( x+2, y+2, w-3, h-4-value*(h/255.0) );
}


//Piano-keyboard

//Search-bar

//File-selector
