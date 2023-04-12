
//cRTED CPU-emulation


void cRTED_initCPU (cRTED_CPUinstance* CPU, unsigned short mempos) {
 CPU->PC = mempos; CPU->A = 0; CPU->X = 0; CPU->Y = 0; CPU->ST = 0x04; CPU->SP = 0xFF; CPU->PrevNMI = 0;
}


unsigned char cRTED_emulateCPU (void) { //the CPU emulation for TED/PRG playback (ToDo: RESET vector)

 enum StatusFlagBitValues { N=0x80, V=0x40, B=0x10, D=0x08, I=0x04, Z=0x02, C=0x01 };

 static const unsigned char FlagSwitches[]={0x01,0x21,0x04,0x24,0x00,0x40,0x08,0x28}, BranchFlags[]={0x80,0x40,0x01,0x02};

 //could be a parameter but function-call is faster this way if only 1 main CPU exists:
 static cRTED_CPlus4instance* const CPlus4 = &cRTED_CPlus4;
 static cRTED_CPUinstance* const CPU = &CPlus4->CPU;

 static char Cycles, SamePage;
 static unsigned char IR, ST, X, Y;
 static short int A, SP, T;
 static unsigned int PC, Addr, PrevPC, Tmp;


 inline void loadReg (void) { PC = CPU->PC; SP = CPU->SP; ST = CPU->ST; A = CPU->A; X = CPU->X; Y = CPU->Y; }
 inline void storeReg (void) { CPU->PC = PC; CPU->SP = SP; CPU->ST = ST; CPU->A = A; CPU->X = X; CPU->Y = Y; }

 inline unsigned char rd (register unsigned short address) {
  static unsigned char value;
  value = *cRTED_getMemReadPtr(address);
  return value;
 }

 inline void wr (register unsigned short address, register unsigned char data) {
  *cRTED_getMemWritePtr(address)=data;
  if (address==0xFF09) cRTED_acknowledgeTEDIRQ( &CPlus4->TED, data );
 }

 inline void wr2 (register unsigned short address, register unsigned char data) { //PTED-hack specific memory-write
  static int Tmp;
  *cRTED_getMemWritePtr(address)=data;
  if (address==0xFF09) {
   cRTED_acknowledgeTEDIRQ( &CPlus4->TED, data );
  }
  else if (address==0xFF3E) CPlus4->ROMenabled=1; else if (address==0xFF3F) CPlus4->ROMenabled=0;
  else if (0xFF00<=address && address<=0xFF05) CPlus4->TED.TimerRunning[(address-0xFF00)>>1] = address&1;
 }


 inline void addrModeImmediate (void) { ++PC; Addr=PC; Cycles=2; } //imm.
 inline void addrModeZeropage (void) { ++PC; Addr=rd(PC); Cycles=3; } //zp
 inline void addrModeAbsolute (void) { ++PC; Addr = rd(PC); ++PC; Addr += rd(PC)<<8; Cycles=4; } //abs
 inline void addrModeZeropageXindexed (void) { ++PC; Addr = (rd(PC) + X) & 0xFF; Cycles=4; } //zp,x (with zeropage-wraparound of 6502)
 inline void addrModeZeropageYindexed (void) { ++PC; Addr = (rd(PC) + Y) & 0xFF; Cycles=4; } //zp,y (with zeropage-wraparound of 6502)

 inline void addrModeXindexed (void) { // abs,x (only STA is 5 cycles, others are 4 if page not crossed, RMW:7)
  ++PC; Addr = rd(PC) + X; ++PC; SamePage = (Addr <= 0xFF); Addr += rd(PC)<<8; Cycles=5;
 }

 inline void addrModeYindexed (void) { // abs,y (only STA is 5 cycles, others are 4 if page not crossed, RMW:7)
  ++PC; Addr = rd(PC) + Y; ++PC; SamePage = (Addr <= 0xFF); Addr += rd(PC)<<8; Cycles=5;
 }

 inline void addrModeIndirectYindexed (void) { // (zp),y (only STA is 6 cycles, others are 5 if page not crossed, RMW:8)
  ++PC; Addr = rd(rd(PC)) + Y; SamePage = (Addr <= 0xFF); Addr += rd( (rd(PC)+1)&0xFF ) << 8; Cycles=6;
 }

 inline void addrModeXindexedIndirect (void) { // (zp,x)
  ++PC; Addr = ( rd(rd(PC)+X)&0xFF ) + ( ( rd(rd(PC)+X+1)&0xFF ) << 8 ); Cycles=6;
 }


 inline void clrC (void) { ST &= ~C; } //clear Carry-flag
 inline void setC (unsigned char expr) { ST &= ~C; ST |= (expr!=0); } //set Carry-flag if expression is not zero
 inline void clrNZC (void) { ST &= ~(N|Z|C); } //clear flags
 inline void clrNVZC (void) { ST &= ~(N|V|Z|C); } //clear flags
 inline void setNZbyA (void) { ST &= ~(N|Z); ST |= ((!A)<<1) | (A&N); } //set Negative-flag and Zero-flag based on result in Accumulator
 inline void setNZbyT (void) { T&=0xFF; ST &= ~(N|Z); ST |= ((!T)<<1) | (T&N); }
 inline void setNZbyX (void) { ST &= ~(N|Z); ST |= ((!X)<<1) | (X&N); } //set Negative-flag and Zero-flag based on result in X-register
 inline void setNZbyY (void) { ST &= ~(N|Z); ST |= ((!Y)<<1) | (Y&N); } //set Negative-flag and Zero-flag based on result in Y-register
 //set Negative-flag and Zero-flag based on result at Memory-Address:
 inline void setNZbyM (void) { ST &= ~(N|Z); ST |= ((!rd(Addr))<<1) | (rd(Addr)&N); }
 inline void setNZCbyAdd (void) { ST &= ~(N|Z|C); ST |= (A&N)|(A>255||(ST&D)&&A>0x99); A&=0xFF; ST|=(!A)<<1; } //after increase/addition
 //calculate V-flag from A and T (previous A) and input2 (Memory):
 inline void setVbyAdd (unsigned char M) { ST &= ~V; ST |= ( (~(T^M)) & (T^A) & N ) >> 1; }
 inline void setNZCbySub (signed short* obj) { ST &= ~(N|Z|C); ST |= (*obj&N) | (*obj>=0); *obj&=0xFF; ST |= ((!(*obj))<<1); }

 inline void push (unsigned char value) { CPlus4->RAMbank[0x100+SP] = value; --SP; SP&=0xFF; } //push a value to stack
 inline unsigned char pop (void) { ++SP; SP&=0xFF; return CPlus4->RAMbank[0x100+SP]; } //pop a value from stack


 loadReg(); PrevPC=PC; //setting 'Cycles' variable: ensure smallest 6510 runtime (for implied/register instructions)
 IR = rd(PC); Cycles=2; SamePage=0; //printf("%4.4X-%2.2X\n",PC,IR); //printf point for debugging


 if(IR&1) {  //nybble2:  1/5/9/D:accu.instructions, 3/7/B/F:illegal opcodes

  switch ( (IR & 0x1F) >> 1 ) { //value-forming to cause jump-table //PC wraparound not handled inside to save codespace
   case    0:  case    1: addrModeXindexedIndirect(); break; //(zp,x)
   case    2:  case    3: addrModeZeropage(); break;
   case    4:  case    5: addrModeImmediate(); break;
   case    6:  case    7: addrModeAbsolute(); break;
   case    8:  case    9: addrModeIndirectYindexed(); break; //(zp),y (5..6 cycles, 8 for R-M-W)
   case  0xA:             addrModeZeropageXindexed(); break; //zp,x
   case  0xB:             if ((IR&0xC0)!=0x80) addrModeZeropageXindexed(); //zp,x for illegal opcodes
                          else addrModeZeropageYindexed(); //zp,y for LAX/SAX illegal opcodes
                          break;
   case  0xC:  case  0xD: addrModeYindexed(); break;
   case  0xE:             addrModeXindexed(); break;
   case  0xF:             if ((IR&0xC0)!=0x80) addrModeXindexed(); //abs,x for illegal opcodes
                          else addrModeYindexed(); //abs,y for LAX/SAX illegal opcodes
                          break;
  }
  Addr&=0xFFFF;

  switch ( (IR & 0xE0) >> 5 ) { //value-forming to cause gapless case-values and faster jump-table creation from switch-case

   case 0: if ( (IR&0x1F) != 0xB ) { //ORA / SLO(ASO)=ASL+ORA
            if ( (IR&3) == 3 ) { clrNZC(); setC(rd(Addr)>=N); wr( Addr, rd(Addr)<<1 ); Cycles+=2; } //for SLO
            else Cycles -= SamePage;
            A |= rd(Addr); setNZbyA(); //ORA
           }
           else { A&=rd(Addr); setNZbyA(); setC(A>=N); } //ANC (AND+Carry=bit7)
           break;

   case 1: if ( (IR&0x1F) != 0xB ) { //AND / RLA (ROL+AND)
            if ( (IR&3) == 3 ) { //for RLA
             T = (rd(Addr)<<1) + (ST&C); clrNZC(); setC(T>255); T&=0xFF; wr(Addr,T); Cycles+=2;
            }
            else Cycles -= SamePage;
            A &= rd(Addr); setNZbyA(); //AND
           }
           else { A&=rd(Addr); setNZbyA(); setC(A>=N); } //ANC (AND+Carry=bit7)
           break;

   case 2: if ( (IR&0x1F) != 0xB ) { //EOR / SRE(LSE)=LSR+EOR
            if ( (IR&3) == 3 ) { clrNZC(); setC(rd(Addr)&1); wr(Addr,rd(Addr)>>1); Cycles+=2; } //for SRE
            else Cycles -= SamePage;
            A ^= rd(Addr); setNZbyA(); //EOR
           }
           else { A&=rd(Addr); setC(A&1); A>>=1; A&=0xFF; setNZbyA(); } //ALR(ASR)=(AND+LSR)
           break;

   case 3: if ( (IR&0x1F) != 0xB ) { //RRA (ROR+ADC) / ADC
            if( (IR&3) == 3 ) { //for RRA
             T = (rd(Addr)>>1) + ((ST&C)<<7); clrNZC(); setC(T&1); wr(Addr,T); Cycles+=2;
            }
            else Cycles -= SamePage;
            T=A; A += rd(Addr)+(ST&C);
            if ((ST & D) && (A&0xF)>9) { A+=0x10; A&=0xF0; }
            setNZCbyAdd(); setVbyAdd(rd(Addr)); //ADC
           }
           else { // ARR (AND+ROR, bit0 not going to C, but C and bit7 get exchanged.)
            A &= rd(Addr); T += rd(Addr) + (ST&C);
            clrNVZC(); setC(T>255); setVbyAdd(rd(Addr)); //V-flag set by intermediate ADC mechanism: (A&mem)+mem
            T=A; A = (A>>1) + ((ST&C)<<7); setC(T>=N); setNZbyA();
           }
           break;

   case 4: if ( (IR&0x1F) == 0xB ) { A = X & rd(Addr); setNZbyA(); } //XAA (TXA+AND), highly instable on real 6502!
           else if ( (IR&0x1F) == 0x1B ) { SP = A&X; wr( Addr, SP&((Addr>>8)+1) ); } //TAS(SHS) (SP=A&X, mem=S&H} - instable on real 6502
           else { wr2( Addr, A & (((IR&3)==3)? X:0xFF) ); } //STA / SAX (at times same as AHX/SHX/SHY) (illegal)
           break;

   case 5: if ( (IR&0x1F) != 0x1B ) { A=rd(Addr); if((IR&3)==3) X=A; } //LDA / LAX (LAX #imm is instable on 6502)
           else { A=X=SP = rd(Addr) & SP; } //LAS(LAR)
           setNZbyA(); Cycles -= SamePage;
           break;

   case 6: if( (IR&0x1F) != 0xB ) { // CMP / DCP(DEC+CMP)
            if ( (IR&3) == 3 ) { wr(Addr,rd(Addr)-1); Cycles+=2;} //DCP
            else Cycles -= SamePage;
            T = A-rd(Addr);
           }
           else { X = T = (A&X) - rd(Addr); } //SBX(AXS)  //SBX (AXS) (CMP+DEX at the same time)
           setNZCbySub(&T);
           break;

   case 7: if( (IR&3)==3 && (IR&0x1F)!=0xB ) { wr( Addr, rd(Addr)+1 ); Cycles+=2; } //ISC(ISB)=INC+SBC / SBC
           else Cycles -= SamePage;
           T=A; A -= rd(Addr) + !(ST&C);
           if ((ST & D) && (A&0xF)>9) { A -= ((A&0xF)-9); }
           setNZCbySub(&A); setVbyAdd(~rd(Addr));
           break;
  }
 }


 else if (IR&2) {  //nybble2:  2:illegal/LDX, 6:A/X/INC/DEC, A:Accu-shift/reg.transfer/NOP, E:shift/X/INC/DEC

  switch (IR & 0x1F) { //Addressing modes
   case    2: addrModeImmediate(); break;
   case    6: addrModeZeropage(); break;
   case  0xE: addrModeAbsolute(); break;
   case 0x16: if ( (IR&0xC0) != 0x80 ) addrModeZeropageXindexed(); //zp,x
              else addrModeZeropageYindexed(); //zp,y
              break;
   case 0x1E: if ( (IR&0xC0) != 0x80 ) addrModeXindexed(); //abs,x
              else addrModeYindexed(); //abs,y
              break;
  }
  Addr&=0xFFFF;

  switch ( (IR & 0xE0) >> 5 ) {

   case 0: clrC(); case 1:
           if((IR&0xF)==0xA) { A = (A<<1) + (ST&C); setNZCbyAdd(); } //ASL/ROL (Accu)  //RMW (Read-Write-Modify):
           else { wr(Addr,rd(Addr)); T = (rd(Addr)<<1) + (ST&C); setC(T>255); setNZbyT(); wr(Addr,T); Cycles+=2; }
           break; //writing value at an address back to itself happens in real CPU and ASL $FF09 utilizes it in e.g. uwol.ted

   case 2: clrC(); case 3:
           if((IR&0xF)==0xA) { T=A; A=(A>>1)+((ST&C)<<7); setC(T&1); A&=0xFF; setNZbyA(); } //LSR/ROR (Accu) //memory (RMW):
           else { wr(Addr,rd(Addr)); T = (rd(Addr)>>1) + ((ST&C)<<7); setC(rd(Addr)&1); setNZbyT(); wr(Addr,T); Cycles+=2; }
           break;

   case 4: if (IR&4) {wr2(Addr,X);} //STX
           else if(IR&0x10) SP=X; //TXS
           else { A=X; setNZbyA(); } //TXA
           break;

   case 5: if ( (IR&0xF) != 0xA ) { X=rd(Addr); Cycles -= SamePage; } //LDX
           else if (IR & 0x10) X=SP; //TSX
           else X=A; //TAX
           setNZbyX();
           break;

   case 6: if (IR&4) { wr(Addr,rd(Addr)); wr(Addr,rd(Addr)-1); setNZbyM(); Cycles+=2; } //DEC
           else { --X; setNZbyX(); } //DEX
           break;

   case 7: if (IR&4) { wr(Addr,rd(Addr)); wr(Addr,rd(Addr)+1); setNZbyM(); Cycles+=2; } //INC/NOP
           break;
  }
 }


 else if ( (IR & 0xC) == 8 ) {  //nybble2:  8:register/statusflag
  if ( IR&0x10 ) {
   if ( IR == 0x98 ) { A=Y; setNZbyA(); } //TYA
   else { //CLC/SEC/CLI/SEI/CLV/CLD/SED
    if (FlagSwitches[IR>>5] & 0x20) ST |= (FlagSwitches[IR>>5] & 0xDF);
    else ST &= ~( FlagSwitches[IR>>5] & 0xDF );
   }
  }
  else {
   switch ( (IR & 0xF0) >> 5 ) {
    case 0: push(ST); Cycles=3; break; //PHP
    case 1: ST=pop(); Cycles=4; break; //PLP
    case 2: push(A); Cycles=3; break; //PHA
    case 3: A=pop(); setNZbyA(); Cycles=4; break; //PLA
    case 4: --Y; setNZbyY(); break; //DEY
    case 5: Y=A; setNZbyY(); break; //TAY
    case 6: ++Y; setNZbyY(); break; //INY
    case 7: ++X; setNZbyX(); break; //INX
   }
  }
 }


 else {  //nybble2:  0: control/branch/Y/compare  4: Y/compare  C:Y/compare/JMP

  if ( (IR&0x1F) == 0x10 ) { //BPL/BMI/BVC/BVS/BCC/BCS/BNE/BEQ  relative branch
   ++PC;
   T=rd(PC); if (T & 0x80) T-=0x100;
   if (IR & 0x20) {
    if (ST & BranchFlags[IR>>6]) { PC+=T; Cycles=3; }
   }
   else {
    if ( !(ST & BranchFlags[IR>>6]) ) { PC+=T; Cycles=3; } //plus 1 cycle if page is crossed?
   }
  }

  else {  //nybble2:  0:Y/control/Y/compare  4:Y/compare  C:Y/compare/JMP
   switch (IR&0x1F) { //Addressing modes
    case    0: addrModeImmediate(); break; //imm. (or abs.low for JSR/BRK)
    case    4: addrModeZeropage(); break;
    case  0xC: addrModeAbsolute(); break;
    case 0x14: addrModeZeropageXindexed(); break; //zp,x
    case 0x1C: addrModeXindexed(); break; //abs,x
   }
   Addr&=0xFFFF;

   switch ( (IR & 0xE0) >> 5 ) {

    case 0: if( !(IR&4) ) { //BRK / NOP-absolute/abs,x/zp/zp,x
             push((PC+2-1)>>8); push((PC+2-1)&0xFF); push(ST|B); ST |= I; //BRK
             PC = rd(0xFFFE) + (rd(0xFFFF)<<8) - 1; Cycles=7;
            }
            else if (IR == 0x1C) Cycles -= SamePage; //NOP abs,x
            break;

    case 1: if (IR & 0xF) { //BIT / NOP-abs,x/zp,x
             if ( !(IR&0x10) ) { ST &= 0x3D; ST |= (rd(Addr)&0xC0) | ( (!(A&rd(Addr))) << 1 ); } //BIT
             else if (IR == 0x3C) Cycles -= SamePage; //NOP abs,x
            }
            else { //JSR
             push((PC+2-1)>>8); push((PC+2-1)&0xFF);
             PC=rd(Addr)+rd(Addr+1)*256-1; Cycles=6;
             //Plus4 hacks to substitute complete KERNAL ROM
             if(cRTED_CPlus4.ROMenabled) { if (PC==0x8bbe -1) { //the_pit_manic_miner.ted KERNAL workaround
              Tmp=cRTED_findSYStarget(&cRTED_CPlus4.RAMbank[0x1001-2],100); wr(0x14,Tmp&0xFF); wr(0x15,Tmp>>8);
             } //hack for pac_man_sfx.ted, gets key symbols through jsr DB11 into A:
             else if (PC==0xDB11-1) {A=cRTED_CPlus4.PressedKeySymbol;} }
            }
            break;

    case 2: if (IR & 0xF) { //JMP / NOP-abs,x/zp/zp,x
             if (IR == 0x4C) { //JMP
              PC = Addr-1; Cycles=3;
              rd(Addr+1);
              //if (Addr==PrevPC) {storeReg(); CPlus4->Returned=1; return 0xFF;} //turn self-jump mainloop (after init) into idle time
             }
             else if (IR==0x5C) Cycles -= SamePage; //NOP abs,x
            }
            else { //RTI
             ST=pop(); T=pop(); PC = (pop()<<8) + T - 1; Cycles=6;
             if (CPlus4->Returned && SP>=0xFF) { ++PC; storeReg(); return 0xFE; }
            }
            break;

    case 3: if (IR & 0xF) { //JMP() (indirect) / NOP-abs,x/zp/zp,x
             if (IR == 0x6C) { //JMP() (indirect)
              PC = rd( (Addr&0xFF00) + ((Addr+1)&0xFF) ); //(with highbyte-wraparound bug)
              PC = (PC<<8) + rd(Addr) - 1; Cycles=5;
             }
             else if (IR == 0x7C) Cycles -= SamePage; //NOP abs,x
            }
            else { //RTS
             if (SP>=0xFF) {storeReg(); CPlus4->Returned=1; return 0xFF;} //Init returns, provide idle-time between IRQs
             T=pop(); PC = (pop()<<8) + T; Cycles=6;
            }
            break;

    case 4: if (IR & 4) { wr2( Addr, Y ); } //STY / NOP #imm
            break;

    case 5: Y=rd(Addr); setNZbyY(); Cycles -= SamePage; //LDY
            break;

    case 6: if ( !(IR&0x10) ) { //CPY / NOP abs,x/zp,x
             T=Y-rd(Addr); setNZCbySub(&T); //CPY
            }
            else if (IR==0xDC) Cycles -= SamePage; //NOP abs,x
            break;

    case 7: if ( !(IR&0x10) ) { //CPX / NOP abs,x/zp,x
             T=X-rd(Addr); setNZCbySub(&T); //CPX
            }
            else if (IR==0xFC) Cycles -= SamePage; //NOP abs,x
            break;
   }
  }
 }


 ++PC; //PC&=0xFFFF;

 storeReg();

 return Cycles;
}



 //handle entering into IRQ and NMI interrupt
static inline char cRTED_handleCPUinterrupts (cRTED_CPUinstance* CPU) {
 enum StatusFlagBitValues { B=0x10, I=0x04 }; //push a value onto stack:
 inline void push (unsigned char value) { CPU->CPlus4->RAMbank[0x100+CPU->SP] = value; --CPU->SP; CPU->SP&=0xFF; }

 //if IRQ and NMI at the same time, NMI is serviced first (but may not if starts at the exact same cycle, according to Soci):
 if (CPU->CPlus4->NMI > CPU->PrevNMI) {
  push(CPU->PC>>8); push(CPU->PC&0xFF); push(CPU->ST); CPU->ST |= I;
  CPU->PC = *cRTED_getMemReadPtr(0xFFFA) + (*cRTED_getMemReadPtr(0xFFFB)<<8); //NMI-vector
  CPU->PrevNMI = CPU->CPlus4->NMI;
  return 1;
 }
 else if ( CPU->CPlus4->IRQ && !(CPU->ST&I) ) {
  push(CPU->PC>>8); push(CPU->PC&0xFF); push(CPU->ST); CPU->ST |= I;
  CPU->PC = *cRTED_getMemReadPtr(0xFFFE) + (*cRTED_getMemReadPtr(0xFFFF)<<8); //maskable IRQ-vector
  CPU->PrevNMI = CPU->CPlus4->NMI;
  return 1;
 }
 CPU->PrevNMI = CPU->CPlus4->NMI; //prepare for NMI edge-detection

 return 0;
}
