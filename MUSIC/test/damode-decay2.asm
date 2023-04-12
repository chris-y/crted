;DA-mode noise-test

        *= $1001
        .word ss,1
        .null $9e,format("%d",start) ;SYS command
ss      .word 0

SNDCTRL=$FF11
 DAMODE=$80
 CH2NOI=$40
 CH2PUL=$20
 CH1PUL=$10
 VOLUME=$0F
FREQ1LO=$FF0E
FREQ1HI=$FF12
FREQ2LO=$FF0F
FREQ2HI=$FF10
BORDER=$FF19
BACKGROUND=$FF15

start   sei
        lda #$66
        sta BACKGROUND

        lda #CH2NOI|CH2PUL|VOLUME
        sta SNDCTRL

        lda #$03
        sta FREQ2HI
        lda #$00
        sta FREQ2LO

sndtst1 lda SNDCTRL
        and #$FF-DAMODE
        sta SNDCTRL
        ldy #3*50
-       lda #$E0
        cmp $FF1D
        bne *-3
        lda #$41
        sta BORDER
        ldx #20
        dex
        bne *-1
        lda #$00
        sta BORDER
        dey
        bne -

sndtst2 lda SNDCTRL
        ora #DAMODE
        sta SNDCTRL
        ldy #3*50
-       lda #$E0
        cmp $FF1D
        bne *-3
        lda #$01
        sta BORDER
        ldx #20
        dex
        bne *-1
        lda #$00
        sta BORDER
        dey
        bne -

        jmp sndtst1
