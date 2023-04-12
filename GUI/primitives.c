//Graphic primitives

//#include "graphics/fonts-size06.h"
//#include "graphics/fonts-size07.h"
#include "graphics/fonts-size08.h"
#include "graphics/fonts-size10.h"
#include "graphics/fonts-size12.h"
//#include "graphics/fonts-size16.h"


#define PI 3.14159
#define RAD (PI/180.0)
#define SQRT05 0.7071
#define ANGLERESOMUL ( (360*SQRT05) / (2.0*PI) ) //to draw pixels of circle at 1-pixel distance
#define GRADIENT_DISTANCE_BALANCE 0.9 //0.7 //0.9 //max. 0.9, how far gradient-color blends into the normal color
#define GRADIENT_DISTANCE_BALANCE_CIRCLE 1.0

enum FontProperties { CHARCODE_MIN=0x18, FONT_SPACING=1, BUTTONMARGIN=1 };
enum GradientTypes { GRADIENT_HORIZONTAL, GRADIENT_VERTICAL, GRADIENT_DIAGONAL,
                     GRADIENT_CENTER_HORIZONTAL, GRADIENT_CENTER_VERTICAL,
                     GRADIENT_CENTER, GRADIENT_DISTANCE };
//enum ArrowDirections { ARROW_UP=1, ARROW_DOWN=-1 };


SDL_Surface *Screen;
byte/*Uint32*/ *Pixels, DrawRed=0, DrawGreen=0, DrawBlue=0, FontSize=8;
byte DrawRedGradient=0, DrawGreenGradient=0, DrawBlueGradient=0;
float DrawAlpha=1.0;
byte **FontSetPointer=(byte**)FontPointers8, *FontWidthPointer=(byte*)FontWidths8, FontHeight=FontHeights8;


static inline void setColor(dword color) {
 DrawBlue=color&0xFF; color>>=8; DrawGreen=color&0xFF; DrawRed=color>>8; DrawAlpha=1.0;
}

static inline void setRGBcolor(byte R, byte G, byte B) {
 DrawRed=R; DrawGreen=G; DrawBlue=B; DrawAlpha=1.0;
}

static inline void setGradientColor(dword color) {
 DrawBlueGradient=color&0xFF; color>>=8; DrawGreenGradient=color&0xFF; DrawRedGradient=color>>8;
}

static inline void setGradientRGBcolor(byte R, byte G, byte B) {
 DrawRedGradient=R; DrawGreenGradient=G; DrawBlueGradient=B;
}

static inline void setColorAlpha(dword color) {
 DrawBlue=color&0xFF; color>>=8; DrawGreen=color&0xFF; color>>=8; DrawRed=color&0xFF; DrawAlpha=(color>>8)/255.0;
}

static inline void setRGBcolorAlpha(byte R, byte G, byte B, float A) {
 DrawRed=R; DrawGreen=G; DrawBlue=B; DrawAlpha=A;
}


static inline void setFontSize(byte size) {
 switch (size) {
  //case 6: FontSetPointer=(byte**)FontPointers6; FontWidthPointer=(byte*)FontWidths6; FontHeight=FontHeights6; break;
  //case 7: FontSetPointer=(byte**)FontPointers7; FontWidthPointer=(byte*)FontWidths7; FontHeight=FontHeights7; break;
  case 8: FontSetPointer=(byte**)FontPointers8; FontWidthPointer=(byte*)FontWidths8; FontHeight=FontHeights8; break;
  case 10: FontSetPointer=(byte**)FontPointers10; FontWidthPointer=(byte*)FontWidths10; FontHeight=FontHeights10; break;
  case 12: FontSetPointer=(byte**)FontPointers12; FontWidthPointer=(byte*)FontWidths12; FontHeight=FontHeights12; break;
  //case 16: FontSetPointer=(byte**)FontPointers16; FontWidthPointer=(byte*)FontWidths16; FontHeight=FontHeights16; break;
 }
}


static inline void drawRawPixel(word x, word y) {
 static int PixIndex;
 PixIndex = y*Screen->pitch + x*BYTES_PER_PIXEL; //if(PixIndex>PixelArraySize) return;
 Pixels[PixIndex+0] = DrawBlue;
 Pixels[PixIndex+1] = DrawGreen;
 Pixels[PixIndex+2] = DrawRed;
}

static inline void drawGradientPixel(word x, word y, float grad) { //with gradient-support
 static int PixIndex; float rgrad = 1.0-grad;
 PixIndex = y*Screen->pitch + x*BYTES_PER_PIXEL; //if(PixIndex>PixelArraySize) return;
 Pixels[PixIndex+0] = DrawBlue * rgrad  + DrawBlueGradient * grad;
 Pixels[PixIndex+1] = DrawGreen * rgrad + DrawGreenGradient * grad;;
 Pixels[PixIndex+2] = DrawRed * rgrad   + DrawRedGradient * grad;;
}

static inline void drawPixel(word x, word y) { //with alpha-support
 static int PixIndex;
 PixIndex = y*Screen->pitch + x*BYTES_PER_PIXEL; //if(PixIndex>PixelArraySize) return;
 Pixels[PixIndex+0] += (DrawBlue  - Pixels[PixIndex+0]) * DrawAlpha;
 Pixels[PixIndex+1] += (DrawGreen - Pixels[PixIndex+1]) * DrawAlpha;
 Pixels[PixIndex+2] += (DrawRed   - Pixels[PixIndex+2]) * DrawAlpha;
}

static inline void drawAlphaPixel(word x, word y, float alpha) { //, unsigned char alpha) {
 static int PixIndex; static float Alpha;
 PixIndex = y*Screen->pitch + x*BYTES_PER_PIXEL; Alpha = alpha*DrawAlpha;
 Pixels[PixIndex+0] += (DrawBlue  - Pixels[PixIndex+0]) * Alpha;
 Pixels[PixIndex+1] += (DrawGreen - Pixels[PixIndex+1]) * Alpha;
 Pixels[PixIndex+2] += (DrawRed   - Pixels[PixIndex+2]) * Alpha;
}

static inline void drawGradientAlphaPixel(word x, word y, float alpha) { //, unsigned char alpha) {
 static int PixIndex; static float Alpha;
 PixIndex = y*Screen->pitch + x*BYTES_PER_PIXEL; Alpha = alpha*DrawAlpha;
 Pixels[PixIndex+0] += (DrawBlueGradient  - Pixels[PixIndex+0]) * Alpha;
 Pixels[PixIndex+1] += (DrawGreenGradient - Pixels[PixIndex+1]) * Alpha;
 Pixels[PixIndex+2] += (DrawRedGradient   - Pixels[PixIndex+2]) * Alpha;
}

static inline void drawHorizontalPixel (float x, word y) { //anti-aliased horizontally
 static int PixIndex; static word intx; static float fracx, rfracx;
 intx = (word)x; fracx = (x-intx)*DrawAlpha; rfracx = DrawAlpha-fracx;
 PixIndex = y*Screen->pitch + intx*BYTES_PER_PIXEL;
 Pixels[PixIndex+0] += (DrawBlue  - Pixels[PixIndex+0]) * rfracx;
 Pixels[PixIndex+1] += (DrawGreen - Pixels[PixIndex+1]) * rfracx;
 Pixels[PixIndex+2] += (DrawRed   - Pixels[PixIndex+2]) * rfracx;
 PixIndex += BYTES_PER_PIXEL;
 Pixels[PixIndex+0] += (DrawBlue  - Pixels[PixIndex+0]) * fracx;
 Pixels[PixIndex+1] += (DrawGreen - Pixels[PixIndex+1]) * fracx;
 Pixels[PixIndex+2] += (DrawRed   - Pixels[PixIndex+2]) * fracx;
}

static inline void drawVerticalPixel (word x, float y) { //anti-aliased vertically
 static int PixIndex; static word inty; static float fracy, rfracy;
 inty = (word)y; fracy = (y-inty)*DrawAlpha; rfracy = DrawAlpha-fracy;
 PixIndex = inty*Screen->pitch + x*BYTES_PER_PIXEL;
 Pixels[PixIndex+0] += (DrawBlue  - Pixels[PixIndex+0]) * rfracy;
 Pixels[PixIndex+1] += (DrawGreen - Pixels[PixIndex+1]) * rfracy;
 Pixels[PixIndex+2] += (DrawRed   - Pixels[PixIndex+2]) * rfracy;
 PixIndex += Screen->pitch;
 Pixels[PixIndex+0] += (DrawBlue  - Pixels[PixIndex+0]) * fracy;
 Pixels[PixIndex+1] += (DrawGreen - Pixels[PixIndex+1]) * fracy;
 Pixels[PixIndex+2] += (DrawRed   - Pixels[PixIndex+2]) * fracy;
}

static inline void drawAntiAliasedPixel (float x, float y) {
 static int PixIndex; static word intx,inty; static float fracx,rfracx,fracy,rfracy,portion;
 intx = (word)x; inty = (word)y;
 fracx = (x-intx)*DrawAlpha; rfracx = DrawAlpha-fracx;
 fracy = (y-inty)*DrawAlpha; rfracy = DrawAlpha-fracy;
 PixIndex = inty*Screen->pitch + intx*BYTES_PER_PIXEL; portion = rfracx * rfracy;
 Pixels[PixIndex+0] += (DrawBlue  - Pixels[PixIndex+0]) * portion;
 Pixels[PixIndex+1] += (DrawGreen - Pixels[PixIndex+1]) * portion;
 Pixels[PixIndex+2] += (DrawRed   - Pixels[PixIndex+2]) * portion;
 PixIndex += BYTES_PER_PIXEL; portion = fracx * rfracy;
 Pixels[PixIndex+0] += (DrawBlue  - Pixels[PixIndex+0]) * portion;
 Pixels[PixIndex+1] += (DrawGreen - Pixels[PixIndex+1]) * portion;
 Pixels[PixIndex+2] += (DrawRed   - Pixels[PixIndex+2]) * portion;
 PixIndex += Screen->pitch; portion = fracx * fracy;
 Pixels[PixIndex+0] += (DrawBlue  - Pixels[PixIndex+0]) * portion;
 Pixels[PixIndex+1] += (DrawGreen - Pixels[PixIndex+1]) * portion;
 Pixels[PixIndex+2] += (DrawRed   - Pixels[PixIndex+2]) * portion;
 PixIndex -= BYTES_PER_PIXEL; portion = rfracx * fracy;
 Pixels[PixIndex+0] += (DrawBlue  - Pixels[PixIndex+0]) * portion;
 Pixels[PixIndex+1] += (DrawGreen - Pixels[PixIndex+1]) * portion;
 Pixels[PixIndex+2] += (DrawRed   - Pixels[PixIndex+2]) * portion;
}


void drawHorizontalLine (word x1, word y, word x2) {
 for(;x1<=x2;++x1) drawPixel(x1,y);
}


void drawVerticalLine (word x, word y1, word y2) {
 for(;y1<=y2;++y1) drawPixel(x,y1);
}


void drawLine (word x1, word y1, word x2, word y2) { //, color c, char wide) {
 static word i; static short step; static float m, f;
 step=+1;  if(y2==y1) m=1000000.0; else m = (x2-x1)/(float)(y2-y1);
 if (-1.0<m && m<1.0) { if(y2<y1){step=-1;m*=-1;}  for (i=y1,f=x1; i!=y2; i+=step,f+=m) {drawHorizontalPixel(f,i);} } //,&c);} // if(wide){draw_pixel(f+1,i,&c);draw_pixel(f-1,i,&c);}} }
 else { if(x2<x1){step=-1;m=-1.0/m;} else m=1.0/m;  for (i=x1,f=y1; i!=x2; i+=step,f+=m) {drawVerticalPixel(i,f);} } //,&c);} // if(wide){draw_pixel(i,f+1,&c);draw_pixel(i,f-1,&c);}} }
}


void drawAngledLine (word x1, word y1, word length, short angle) {
 static short i; //, step; //static word x2, y2;
 //x2 = x1 + round( length * cos(angle*RAD) ); y2 = round( y1 - length * sin(angle*RAD) );
 for (i=0; i<length; ++i) drawAntiAliasedPixel( x1+i*cos(angle*RAD), y1-i*sin(angle*RAD) ); //drawLine(x1,y1,x2,y2);
}


void drawRectangle (word x1, word y1, word x2, word y2) {
 //static word x,y;
 drawHorizontalLine(x1,y1,x2); drawHorizontalLine(x1,y2,x2); //for(x=x1;x<=x2;++x) { drawPixel(x,y1); drawPixel(x,y2); }
 drawVerticalLine(x1,y1,y2); drawVerticalLine(x2,y1,y2); //for(y=y1;y<=y2;++y) { drawPixel(x1,y); drawPixel(x2,y); }
}


void drawBox (word x, word y, word w, word h) { //solid color (with alpha-support)
 static word i,j,x2,y2;
 x2=x+w; y2=y+h;
 for(i=y; i<y2; ++i) for(j=x; j<x2; ++j) drawPixel(j,i);
}


void drawVerticalBox (word x, word y, word w, word h) { //solid color (with alpha-support), faster for vertical boxes
 static word i,j,x2,y2;
 x2=x+w; y2=y+h;
 for(i=x; i<x2; ++i) for(j=y; j<y2; ++j) drawPixel(i,j);
}


void drawGradientBox (word x, word y, word w, word h, byte gradtype) { //gradient (without alpha-support)
 static word i,j,x2,y2,halfw,halfh; float Grad,distw,disth;
 switch (gradtype) {
  case GRADIENT_HORIZONTAL: x2=x+w; for(i=0; i<h; ++i) { Grad = (float)i/h; for(j=x; j<x2; ++j) drawGradientPixel(j,y+i,Grad); }
                            break;
  case GRADIENT_VERTICAL: y2=y+h; for(i=0; i<w; ++i) { Grad = (float)i/w; for(j=y; j<y2; ++j) drawGradientPixel(x+i,j,Grad); }
                          break;
  case GRADIENT_DIAGONAL: for(i=0; i<h; ++i) {
                           for(j=0; j<w; ++j) { Grad = (((float)i/h)+((float)j/w))/2; drawGradientPixel(x+j,y+i,Grad); }
                          } break;
  case GRADIENT_CENTER_HORIZONTAL: x2=x+w; halfh=h/2;
                                   for(i=0; i<h; ++i) {
                                    Grad = (float)abs(halfh-i)/halfh; for(j=x; j<x2; ++j) drawGradientPixel(j,y+i,Grad);
                                   } break;
  case GRADIENT_CENTER_VERTICAL: y2=y+h; halfw=w/2;
                                 for(i=0; i<w; ++i) {
                                  Grad = (float)abs(halfw-i)/halfw; for(j=y; j<y2; ++j) drawGradientPixel(x+i,j,Grad);
                                 } break;
  case GRADIENT_CENTER: halfh=h/2; halfw=w/2;
                        for(i=0; i<h; ++i) {
                         for(j=0; j<w; ++j) drawGradientPixel( x+j, y+i, ((float)abs(halfh-i)/halfh+(float)abs(halfw-j)/halfw)/2 );
                        } break;
  case GRADIENT_DISTANCE: halfh=h/2; halfw=w/2;
                          for(i=0; i<h; ++i) {
                           for(j=0; j<w; ++j) { distw=(float)abs(halfw-j)/halfw; disth=(float)abs(halfh-i)/halfh;
                            drawGradientPixel( x+j, y+i, sqrt(distw*distw+disth*disth) * GRADIENT_DISTANCE_BALANCE ); }
                          }  break;
 }
}


void drawCircle (word x, word y, word r) { //circle
 static word i,j,startx,starty,endx,endy,endr; static short d; static float s;
 endr=r*SQRT05; starty=y-endr; endy=y+endr; startx=x-endr-1; endx=x+endr+1;
 for(i=starty; i<=endy; ++i) { d=i-y; s=sqrt(r*r-d*d); drawHorizontalPixel(x-s,i); drawHorizontalPixel(x+s,i); }
 for(i=startx; i<=endx; ++i) { d=i-x; s=sqrt(r*r-d*d); drawVerticalPixel(i,y-s); drawVerticalPixel(i,y+s); }
}


void drawArc (word x, word y, word r, word angle1, word angle2) {
 static float a, step;
 step = ANGLERESOMUL / r;
 if(angle2>angle1) { for (a=angle1; a<=angle2; a+=step) drawAntiAliasedPixel( x + r * cos(a*RAD) , y - r * sin(a*RAD) ); }
 else { for (a=angle2; a<=angle1; a+=step) drawAntiAliasedPixel( x + r * cos(a*RAD) , y - r * sin(a*RAD) ); }
}


void drawDisc (word x, word y, word r) { //filled circle
 static word i,j,startx,starty,endx,endy,endr,startAAy,endAAy,ints; static short d; static float s,fracs;
 starty=y-r; endy=y+r; endr=r*SQRT05; startAAy=y-endr; endAAy=y+endr;
 for(i=starty; i<=endy; ++i) {
  d=i-y; s=sqrt(r*r-d*d); ints=(word)s; fracs=s-ints; startx=x-ints; endx=x+ints; for(j=startx; j<=endx; j++) drawPixel(j,i);
  if(startAAy<=i && i<=endAAy) { drawAlphaPixel(startx-1,i,fracs); drawAlphaPixel(endx+1,i,fracs); }
 }
 startx=x-endr-1; endx=x+endr+1;
 for(i=startx; i<endx; ++i) {
  d=i-x; s=sqrt(r*r-d*d); ints=(word)s; fracs=s-ints;
  drawAlphaPixel(i,y-ints-1,fracs); drawAlphaPixel(i,y+ints+1,fracs);
 }
}


void drawGradientDisc (word x, word y, word r) { //filled circle with gradient towards the center
 static short i,j,d; static float s,fracs,distw,disth;
 static word ints,endr,startAAy,endAAy,yi,startx,endx;
 endr=r*SQRT05; startAAy=y-endr; endAAy=y+endr;
 for(i=-r; i<=+r; ++i) {
  yi=y+i; s=sqrt(r*r-i*i); ints=(word)s; disth = (float)abs(i)/r; fracs=s-ints;
  for(j=-ints; j<=+ints; j++) {
   distw = (float)abs(j)/r;
   drawGradientPixel(x+j,yi, sqrt(distw*distw+disth*disth) * GRADIENT_DISTANCE_BALANCE_CIRCLE );
  }
  if(startAAy<=yi && yi<=endAAy) { drawGradientAlphaPixel(x-ints-1,yi,fracs); drawGradientAlphaPixel(x+ints+1,yi,fracs); }
 }
 startx=x-endr-1; endx=x+endr+1;
 for(i=startx; i<endx; ++i) {
  d=i-x; s=sqrt(r*r-d*d); ints=(word)s; fracs=s-ints;
  drawGradientAlphaPixel(i,y-ints-1,fracs); drawGradientAlphaPixel(i,y+ints+1,fracs);
 }
}


void drawQuarterDisc (word x, word y, word r, byte quarter) {
 static word i,j,startx,starty,endx,endy,endr,startAAy,endAAy,ints; static short d; static float s,fracs;
 endr=r*SQRT05;
 if (quarter==0) {
  startAAy=y-endr;
  for(i=y-r; i<=y; ++i) {
   d=i-y; s=sqrt(r*r-d*d); ints=(word)s; fracs=s-ints; endx=x+ints; for(j=x; j<=endx; ++j) drawPixel(j,i);
   if(startAAy<=i) { drawAlphaPixel(endx+1,i,fracs); }
  }
  endx=x+endr+1; for(i=x; i<=endx; ++i) { d=i-x; s=sqrt(r*r-d*d); ints=(word)s; fracs=s-ints; drawAlphaPixel(i,y-ints-1,fracs); }
 }
 else if (quarter==1) {
  startAAy=y-endr;
  for(i=y-r; i<=y; ++i) {
   d=i-y; s=sqrt(r*r-d*d); ints=(word)s; fracs=s-ints; endx=x-ints; for(j=x; j>=endx; --j) drawPixel(j,i);
   if(startAAy<=i) { drawAlphaPixel(endx-1,i,fracs); }
  }
  endx=x-endr-1; for(i=x; i>=endx; --i) { d=i-x; s=sqrt(r*r-d*d); ints=(word)s; fracs=s-ints; drawAlphaPixel(i,y-ints-1,fracs); }
 }
 else if (quarter==2) {
  endy=y+r; endAAy=y+endr;
  for(i=y; i<=endy; ++i) {
   d=i-y; s=sqrt(r*r-d*d); ints=(word)s; fracs=s-ints; endx=x-ints; for(j=x; j>=endx; --j) drawPixel(j,i);
   if(i<=endAAy) { drawAlphaPixel(endx-1,i,fracs); }
  }
  endx=x-endr-1; for(i=x; i>=endx; --i) { d=i-x; s=sqrt(r*r-d*d); ints=(word)s; fracs=s-ints; drawAlphaPixel(i,y+ints+1,fracs); }
 }
 else {
  endy=y+r; endAAy=y+endr;
  for(i=y; i<=endy; ++i) {
   d=i-y; s=sqrt(r*r-d*d); ints=(word)s; fracs=s-ints; endx=x+ints; for(j=x; j<=endx; ++j) drawPixel(j,i);
   if(i<=endAAy) { drawAlphaPixel(endx+1,i,fracs); }
  }
  endx=x+endr+1; for(i=x; i<=endx; ++i) { d=i-x; s=sqrt(r*r-d*d); ints=(word)s; fracs=s-ints; drawAlphaPixel(i,y+ints+1,fracs); }
 }
}


void drawFilledTriangle (word x1, word y1, word x2, word y2, word x3, word y3) {
 word x,y, tmp, x4, y4; float xs, xe, slope1,slope2;
 if (y1>y2) {tmp=y2;y2=y1;y1=tmp; tmp=x2;x2=x1;x1=tmp;} if (y2>y3) {tmp=y3;y3=y2;y2=tmp; tmp=x3;x3=x2;x2=tmp;}
 if (y1>y2) {tmp=y2;y2=y1;y1=tmp; tmp=x2;x2=x1;x1=tmp;} //now y coordinates are ordered: y1 <= y2 <= y3
 x4 = ( x1 + ((float)(y2-y1)/(y3-y1)) * (x3-x1) );  y4 = y2;
 if(y1!=y2) { //bottom-is-flat sub-triangle of the main triangle
  slope1=(float)(x2-x1)/(y2-y1); slope2=(float)(x4-x1)/(y4-y1); xs=xe=x1;
  for(y=y1;y<=y4;y++) {
   if ((int)xe>=(int)xs) {
    for(x=(int)xs;x<=(int)xe;x++) drawPixel(x,y);
    drawAlphaPixel((word)xs-1,y,1.0-(xs-((word)xs))); drawAlphaPixel((word)xe+1,y,xe-((word)xe));
   }
   else {
    for(x=(int)xs;x>=(int)xe;x--) drawPixel(x,y);
    drawAlphaPixel((word)xe-1,y,1.0-(xe-((word)xe))); drawAlphaPixel((word)xs+1,y,xs-((word)xs));
   }
   xs+=slope1; xe+=slope2;
 }}
 if(y2!=y3) { //top-is-flat sub-triangle of the main triangle
  slope1=(float)(x3-x2)/(y3-y2); slope2=(float)(x3-x4)/(float)(y3-y4); xs=xe=x3;
  for(y=y3;y>y4;y--) {
   if ((int)xe>=(int)xs) {
    for(x=(int)xs;x<=(int)xe;x++) drawPixel(x,y);
    drawAlphaPixel((word)xs-1,y,1.0-(xs-((word)xs))); drawAlphaPixel((word)xe+1,y,xe-((word)xe));
   }
   else {
    for(x=(int)xs;x>=(int)xe;x--) drawPixel(x,y);
    drawAlphaPixel((word)xe-1,y,1.0-(xe-((word)xe))); drawAlphaPixel((word)xs+1,y,xs-((word)xs));
   }
   xs-=slope1; xe-=slope2;
 }}
}


void drawRoundedRectangle (word x, word y, word w, word h, word r) {
 static word x1,x2,y1,y2,x3,y3;
 x1=x+r; x2=x+w; y1=y+r; y2=y+h; x3=x2-r; y3=y2-r;
 drawHorizontalLine(x1,y,x3); drawHorizontalLine(x1,y2,x3);
 drawVerticalLine(x,y1,y3); drawVerticalLine(x2,y1,y3);
 drawArc(x1,y1,r,90,180); drawArc(x3,y1,r,0,90); drawArc(x1,y3,r,180,270); drawArc(x3,y3,r,270,360);
}


void drawRoundedBox (word x, word y, word w, word h, word r) {
 static word x1, y1, y2, x3, y3, w2;
 x1=x+r; y1=y+r; y2=y+h; x3=x+w-r; y3=y2-r; w2=w-r-r-1;
 drawBox(x,y1+1,w+1,h-r-r-1); drawBox(x1+1,y,w2,r+1); drawBox(x1+1,y3,w2,r+1);
 drawQuarterDisc(x1, y1, r, 1); drawQuarterDisc(x3, y1, r, 0); drawQuarterDisc(x1, y3, r, 2); drawQuarterDisc(x3, y3, r, 3);
}


static inline unsigned char getCharCode (unsigned char acode) { //get printable charcode from ASCII-code (convert accents)
 static byte Code; //optimized for ISO-8859-1:
 static char noaccent[]={"__cLOYIS_C__-_R-__23_uS__10_____AAAAAAACEEEEIIIIDNOOOOOx0UUUUYPBaaaaaaaceeeeiiiidnooooo_0uuuuyPy"};

 if (acode<128) Code = acode - CHARCODE_MIN;
 else if (acode>=160) Code = noaccent[acode-160] - CHARCODE_MIN;
 else Code = '_' - CHARCODE_MIN;

 return Code;
}


unsigned char drawFont (word x, word y, unsigned char acode) {
 static word i; static byte Fx,Fy,CharCode,Alpha;
 static byte* FontPtr;
 CharCode = getCharCode(acode);
 FontPtr = FontSetPointer[CharCode]; if (FontPtr==NULL) return '_';
 for (i=Fx=Fy=0; Fy<FontHeight; ++i) {
  Alpha = FontPtr[i];  if (Alpha) drawAlphaPixel( x+Fx, y+Fy, Alpha/256.0 );
  if (Fx < FontWidthPointer[CharCode]-1) ++Fx;  else {Fx=0; ++Fy;}
 }
 return CharCode;
}


void drawString (word x, word y, unsigned char* str) {
 static byte i, CharCode;
 for (i=0; str[i]!='\0'; ++i) {
  CharCode = drawFont(x, y, str[i]);
  x += FontWidthPointer[ CharCode ] + FONT_SPACING;
 }
}


void drawStringPart (word x, word y, word w, word count, unsigned char* str) {
 static byte i, CharCode; static word CharX, CharWidth;
 for (i=0, CharX=x; i<count && str[i]!='\0' && CharX+FontWidthPointer[getCharCode(str[i])]<x+w; ++i) {
  CharCode = drawFont( CharX, y, str[i] );
  CharX += FontWidthPointer[ CharCode ] + FONT_SPACING;
 }
 if (str[i]!='\0' && str[i]!='\n' && CharX+FontWidthPointer[getCharCode(str[i])]<x+w) drawFont(CharX,y,'.');
}


word getTextWidth (char* str) {
 static byte i; static word x;
 for (i=x=0; str[i]!='\0'; ++i) {
  x += FontWidthPointer[ getCharCode(str[i]) ] + FONT_SPACING;
 }
 return x-FONT_SPACING;
}


void drawCenteredString (word x, word y, word w, word h, unsigned char* str) {
 static byte i; static word CharWidth, TextWidth;
 TextWidth=getTextWidth(str);
 if (TextWidth+BUTTONMARGIN*2 < w) drawString( x+round(w/2.0-TextWidth/2.0), y+round(h/2.0-FontHeight/2.0), str );
 else {
  for (i=TextWidth=0; str[i]!='\0'; ++i) {
   CharWidth = FontWidthPointer[ getCharCode(str[i]) ];
   if (TextWidth+CharWidth+BUTTONMARGIN*2+2 > w) break;
   //drawFont(x+TextWidth, y, str[i]);
   TextWidth += CharWidth + FONT_SPACING;
  }
  drawStringPart ( x+(w/2-(TextWidth-FONT_SPACING)/2), y+(h/2-FontHeight/2), w, i, str );
 }
}


void drawTextBox (word x, word y, word w, word h, unsigned char* str) {
 static char End; static int RowCount; static unsigned char* NewLinePtr;
 RowCount=End=0;
 while (End==0) {
  NewLinePtr = strchr(str, '\n'); if (NewLinePtr==NULL) { NewLinePtr = str+strlen(str); End=1; }
  drawStringPart (x, y+RowCount*FontHeight, w, NewLinePtr-str, str ); ++RowCount; str = NewLinePtr+1;
 }
}

