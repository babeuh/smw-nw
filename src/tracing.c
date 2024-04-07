
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "tracing.h"
#include "snes/snes.h"

// name for each opcode, to be filled in with sprintf (length = 14 (13+\0))
static const char* opcodeNames[256] = {
  "brk          ", "ora ($%02x,x)  ", "cop #$%02x     ", "ora $%02x,s    ", "tsb $%02x      ", "ora $%02x      ", "asl $%02x      ", "ora [$%02x]    ", "php          ", "ora #$%04x   ", "asl          ", "phd          ", "tsb $%04x    ", "ora $%04x    ", "asl $%04x    ", "ora $%06x  ",
  "bpl $%04x    ", "ora ($%02x),y  ", "ora ($%02x)    ", "ora ($%02x,s),y", "trb $%02x      ", "ora $%02x,x    ", "asl $%02x,x    ", "ora [$%02x],y  ", "clc          ", "ora $%04x,y  ", "inc          ", "tcs          ", "trb $%04x    ", "ora $%04x,x  ", "asl $%04x,x  ", "ora $%06x,x",
  "jsr $%04x    ", "and ($%02x,x)  ", "jsl $%06x  ", "and $%02x,s    ", "bit $%02x      ", "and $%02x      ", "rol $%02x      ", "and [$%02x]    ", "plp          ", "and #$%04x   ", "rol          ", "pld          ", "bit $%04x    ", "and $%04x    ", "rol $%04x    ", "and $%06x  ",
  "bmi $%04x    ", "and ($%02x),y  ", "and ($%02x)    ", "and ($%02x,s),y", "bit $%02x,x    ", "and $%02x,x    ", "rol $%02x,x    ", "and [$%02x],y  ", "sec          ", "and $%04x,y  ", "dec          ", "tsc          ", "bit $%04x,x  ", "and $%04x,x  ", "rol $%04x,x  ", "and $%06x,x",
  "rti          ", "eor ($%02x,x)  ", "wdm #$%02x     ", "eor $%02x,s    ", "mvp $%02x, $%02x ", "eor $%02x      ", "lsr $%02x      ", "eor [$%02x]    ", "pha          ", "eor #$%04x   ", "lsr          ", "phk          ", "jmp $%04x    ", "eor $%04x    ", "lsr $%04x    ", "eor $%06x  ",
  "bvc $%04x    ", "eor ($%02x),y  ", "eor ($%02x)    ", "eor ($%02x,s),y", "mvn $%02x, $%02x ", "eor $%02x,x    ", "lsr $%02x,x    ", "eor [$%02x],y  ", "cli          ", "eor $%04x,y  ", "phy          ", "tcd          ", "jml $%06x  ", "eor $%04x,x  ", "lsr $%04x,x  ", "eor $%06x,x",
  "rts          ", "adc ($%02x,x)  ", "per $%04x    ", "adc $%02x,s    ", "stz $%02x      ", "adc $%02x      ", "ror $%02x      ", "adc [$%02x]    ", "pla          ", "adc #$%04x   ", "ror          ", "rtl          ", "jmp ($%04x)  ", "adc $%04x    ", "ror $%04x    ", "adc $%06x  ",
  "bvs $%04x    ", "adc ($%02x),y  ", "adc ($%02x)    ", "adc ($%02x,s),y", "stz $%02x,x    ", "adc $%02x,x    ", "ror $%02x,x    ", "adc [$%02x],y  ", "sei          ", "adc $%04x,y  ", "ply          ", "tdc          ", "jmp ($%04x,x)", "adc $%04x,x  ", "ror $%04x,x  ", "adc $%06x,x",
  "bra $%04x    ", "sta ($%02x,x)  ", "brl $%04x    ", "sta $%02x,s    ", "sty $%02x      ", "sta $%02x      ", "stx $%02x      ", "sta [$%02x]    ", "dey          ", "bit #$%04x   ", "txa          ", "phb          ", "sty $%04x    ", "sta $%04x    ", "stx $%04x    ", "sta $%06x  ",
  "bcc $%04x    ", "sta ($%02x),y  ", "sta ($%02x)    ", "sta ($%02x,s),y", "sty $%02x,x    ", "sta $%02x,x    ", "stx $%02x,y    ", "sta [$%02x],y  ", "tya          ", "sta $%04x,y  ", "txs          ", "txy          ", "stz $%04x    ", "sta $%04x,x  ", "stz $%04x,x  ", "sta $%06x,x",
  "ldy #$%04x   ", "lda ($%02x,x)  ", "ldx #$%04x   ", "lda $%02x,s    ", "ldy $%02x      ", "lda $%02x      ", "ldx $%02x      ", "lda [$%02x]    ", "tay          ", "lda #$%04x   ", "tax          ", "plb          ", "ldy $%04x    ", "lda $%04x    ", "ldx $%04x    ", "lda $%06x  ",
  "bcs $%04x    ", "lda ($%02x),y  ", "lda ($%02x)    ", "lda ($%02x,s),y", "ldy $%02x,x    ", "lda $%02x,x    ", "ldx $%02x,y    ", "lda [$%02x],y  ", "clv          ", "lda $%04x,y  ", "tsx          ", "tyx          ", "ldy $%04x,x  ", "lda $%04x,x  ", "ldx $%04x,y  ", "lda $%06x,x",
  "cpy #$%04x   ", "cmp ($%02x,x)  ", "rep #$%02x     ", "cmp $%02x,s    ", "cpy $%02x      ", "cmp $%02x      ", "dec $%02x      ", "cmp [$%02x]    ", "iny          ", "cmp #$%04x   ", "dex          ", "wai          ", "cpy $%04x    ", "cmp $%04x    ", "dec $%04x    ", "cmp $%06x  ",
  "bne $%04x    ", "cmp ($%02x),y  ", "cmp ($%02x)    ", "cmp ($%02x,s),y", "pei $%02x      ", "cmp $%02x,x    ", "dec $%02x,x    ", "cmp [$%02x],y  ", "cld          ", "cmp $%04x,y  ", "phx          ", "stp          ", "jml [$%04x]  ", "cmp $%04x,x  ", "dec $%04x,x  ", "cmp $%06x,x",
  "cpx #$%04x   ", "sbc ($%02x,x)  ", "sep #$%02x     ", "sbc $%02x,s    ", "cpx $%02x      ", "sbc $%02x      ", "inc $%02x      ", "sbc [$%02x]    ", "inx          ", "sbc #$%04x   ", "nop          ", "xba          ", "cpx $%04x    ", "sbc $%04x    ", "inc $%04x    ", "sbc $%06x  ",
  "beq $%04x    ", "sbc ($%02x),y  ", "sbc ($%02x)    ", "sbc ($%02x,s),y", "pea #$%04x   ", "sbc $%02x,x    ", "inc $%02x,x    ", "sbc [$%02x],y  ", "sed          ", "sbc $%04x,y  ", "plx          ", "xce          ", "jsr ($%04x,x)", "sbc $%04x,x  ", "inc $%04x,x  ", "sbc $%06x,x"
};

// for 8/16 bit immediates
// TODO: probably a better way to do this...
static const char* opcodeNamesSp[256] = {
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "ora #$%02x     ", NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "and #$%02x     ", NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "eor #$%02x     ", NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "adc #$%02x     ", NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "bit #$%02x     ", NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  "ldy #$%02x     ", NULL, "ldx #$%02x     ", NULL, NULL, NULL, NULL, NULL, NULL, "lda #$%02x     ", NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  "cpy #$%02x     ", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "cmp #$%02x     ", NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  "cpx #$%02x     ", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "sbc #$%02x     ", NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

// address types for each opcode
static const int opcodeType[256] = {
  0, 1, 1, 1, 1, 1, 1, 1, 0, 4, 0, 0, 2, 2, 2, 3,
  6, 1, 1, 1, 1, 1, 1, 1, 0, 2, 0, 0, 2, 2, 2, 3,
  2, 1, 3, 1, 1, 1, 1, 1, 0, 4, 0, 0, 2, 2, 2, 3,
  6, 1, 1, 1, 1, 1, 1, 1, 0, 2, 0, 0, 2, 2, 2, 3,
  0, 1, 1, 1, 8, 1, 1, 1, 0, 4, 0, 0, 2, 2, 2, 3,
  6, 1, 1, 1, 8, 1, 1, 1, 0, 2, 0, 0, 3, 2, 2, 3,
  0, 1, 7, 1, 1, 1, 1, 1, 0, 4, 0, 0, 2, 2, 2, 3,
  6, 1, 1, 1, 1, 1, 1, 1, 0, 2, 0, 0, 2, 2, 2, 3,
  6, 1, 7, 1, 1, 1, 1, 1, 0, 4, 0, 0, 2, 2, 2, 3,
  6, 1, 1, 1, 1, 1, 1, 1, 0, 2, 0, 0, 2, 2, 2, 3,
  5, 1, 5, 1, 1, 1, 1, 1, 0, 4, 0, 0, 2, 2, 2, 3,
  6, 1, 1, 1, 1, 1, 1, 1, 0, 2, 0, 0, 2, 2, 2, 3,
  5, 1, 1, 1, 1, 1, 1, 1, 0, 4, 0, 0, 2, 2, 2, 3,
  6, 1, 1, 1, 1, 1, 1, 1, 0, 2, 0, 0, 2, 2, 2, 3,
  5, 1, 1, 1, 1, 1, 1, 1, 0, 4, 0, 0, 2, 2, 2, 3,
  6, 1, 1, 1, 2, 1, 1, 1, 0, 2, 0, 0, 2, 2, 2, 3
};

static void getDisassemblyCpu(Snes* snes, char* line);

void getProcessorStateCpu(Snes* snes, char* line) {
  // 0        1         2         3         4         5         6         7         8
  // 12345678901234567890123456789012345678901234567890123456789012345678901234567890
  // CPU 12:3456 1234567890123 A:1234 X:1234 Y:1234 SP:1234 DP:1234 DP:12 e nvmxdizc
  char disLine[14] = "             ";
  getDisassemblyCpu(snes, disLine);
  sprintf(
    line, "CPU %02x:%04x %s A:%04x X:%04x Y:%04x SP:%04x DP:%04x DB:%02x %c %c%c%c%c%c%c%c%c",
    snes->cpu->k, snes->cpu->pc, disLine, snes->cpu->a, snes->cpu->x, snes->cpu->y,
    snes->cpu->sp, snes->cpu->dp, snes->cpu->db, snes->cpu->e ? 'E' : 'e',
    snes->cpu->n ? 'N' : 'n', snes->cpu->v ? 'V' : 'v', snes->cpu->mf ? 'M' : 'm', snes->cpu->xf ? 'X' : 'x',
    snes->cpu->d ? 'D' : 'd', snes->cpu->i ? 'I' : 'i', snes->cpu->z ? 'Z' : 'z', snes->cpu->c ? 'C' : 'c'
  );
}

static void getDisassemblyCpu(Snes* snes, char* line) {
  uint32_t adr = snes->cpu->pc | (snes->cpu->k << 16);
  // read 4 bytes
  // TODO: this can have side effects, implement and use peaking
  uint8_t opcode = snes_read(snes, adr);
  uint8_t byte = snes_read(snes, (adr + 1) & 0xffffff);
  uint8_t byte2 = snes_read(snes, (adr + 2) & 0xffffff);
  uint16_t word = (byte2 << 8) | byte;
  uint32_t longv = (snes_read(snes, (adr + 3) & 0xffffff) << 16) | word;
  uint16_t rel = snes->cpu->pc + 2 + (int8_t) byte;
  uint16_t rell = snes->cpu->pc + 3 + (int16_t) word;
  // switch on type
  switch(opcodeType[opcode]) {
    case 0: sprintf(line, "%s", opcodeNames[opcode]); break;
    case 1: sprintf(line, opcodeNames[opcode], byte); break;
    case 2: sprintf(line, opcodeNames[opcode], word); break;
    case 3: sprintf(line, opcodeNames[opcode], longv); break;
    case 4: {
      if(snes->cpu->mf) {
        sprintf(line, opcodeNamesSp[opcode], byte);
      } else {
        sprintf(line, opcodeNames[opcode], word);
      }
      break;
    }
    case 5: {
      if(snes->cpu->xf) {
        sprintf(line, opcodeNamesSp[opcode], byte);
      } else {
        sprintf(line, opcodeNames[opcode], word);
      }
      break;
    }
    case 6: sprintf(line, opcodeNames[opcode], rel); break;
    case 7: sprintf(line, opcodeNames[opcode], rell); break;
    case 8: sprintf(line, opcodeNames[opcode], byte2, byte); break;
  }
}