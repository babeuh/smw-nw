#include "common_rtl.h"
#include "common_cpu_infra.h"
#include "util.h"
#include "config.h"
#include "snes/snes.h"

uint8 g_ram[0x20000];
uint8 *g_sram;
int g_sram_size;
const uint8 *g_rom;
bool g_did_finish_level_hook;
uint8 game_id;
bool g_playback_mode;
Ppu *g_my_ppu;
Dma *g_dma;

static uint8 *g_rtl_memory_ptr;
static RunFrameFunc *g_rtl_runframe;
static SyncAllFunc *g_rtl_syncall;

void RtlSetupEmuCallbacks(uint8 *emu_ram, RunFrameFunc *func, SyncAllFunc *sync_all) {
  g_rtl_memory_ptr = emu_ram;
  g_rtl_runframe = func;
  g_rtl_syncall = sync_all;
}

static void RtlSynchronizeWholeState(void) {
  if (g_rtl_syncall)
    g_rtl_syncall();
}

// |ptr| must be a pointer into g_ram, will synchronize the RAM memory with the
// emulator.
static void RtlSyncMemoryRegion(void *ptr, size_t n) {
  uint8 *data = (uint8 *)ptr;
  assert(data >= g_ram && data < g_ram + 0x20000);
  if (g_rtl_memory_ptr)
    memcpy(g_rtl_memory_ptr + (data - g_ram), data, n);
}

void ByteArray_AppendVl(ByteArray *arr, uint32 v) {
  for (; v >= 255; v -= 255)
    ByteArray_AppendByte(arr, 255);
  ByteArray_AppendByte(arr, v);
}

typedef struct SaveFuncState {
  SaveLoadInfo base;
  ByteArray array;
} SaveFuncState;

void saveFunc(SaveLoadInfo *sli, void *data, size_t data_size) {
  SaveFuncState *st = (SaveFuncState *)sli;
  ByteArray_AppendData(&st->array, (uint8 *)data, data_size);
}

typedef struct LoadFuncState {
  SaveLoadInfo base;
  uint8 *pstart, *p, *pend;
} LoadFuncState;

void loadFunc(SaveLoadInfo *sli, void *data, size_t data_size) {
  LoadFuncState *st = (LoadFuncState *)sli;
  assert((size_t)(st->pend - st->p) >= data_size);
  memcpy(data, st->p, data_size);
  st->p += data_size;
}

static void LoadSnesState(SaveLoadInfo *sli) {
  // Do the actual loading
  snes_saveload(g_snes, sli);
  RtlSynchronizeWholeState();
}

static void SaveSnesState(SaveLoadInfo *sli) {
  snes_saveload(g_snes, sli);
}

void ReadFromFile(FILE *f, void *data, size_t n) {
  if (fread(data, 1, n, f) != n)
    Die("fread failed\n");
}

void RtlReset(int mode) {
  snes_frame_counter = 0;
  snes_reset(g_snes, true);
  ppu_reset(g_my_ppu);
  if (!(mode & 1))
    memset(g_sram, 0, g_sram_size);

  RtlSynchronizeWholeState();
}

int GetFileSize(FILE *f) {
  fseek(f, 0, SEEK_END);
  int r = ftell(f);
  fseek(f, 0, SEEK_SET);
  return r;
}

bool RtlRunFrame(int inputs) {
  if (g_did_finish_level_hook) {
    g_did_finish_level_hook = false;
  }

  // Avoid up/down and left/right from being pressed at the same time
  if ((inputs & 0x30) == 0x30) inputs ^= 0x30;
  if ((inputs & 0xc0) == 0xc0) inputs ^= 0xc0;

  g_rtl_runframe(inputs, 0);

  snes_frame_counter++;

  return false;
}


void MemCpy(void *dst, const void *src, int size) {
  memcpy(dst, src, size);
}

void mov24(struct LongPtr *a, uint32 d) {
  a->addr = d & 0xffff;
  a->bank = d >> 16;
}

uint32 Load24(LongPtr src) {
  return *(uint32 *)&src & 0xffffff;
}

bool Unreachable(void) {
  printf("Unreachable!\n");
  assert(0);
  g_ram[0x1ffff] = 1;
  return false;
}

uint8 *RomPtr(uint32_t addr) {
  if (!(addr & 0x8000) || addr >= 0x7e0000) {
    printf("RomPtr - Invalid access 0x%x!\n", addr);
    if (!g_fail) {
      g_fail = true;
    }
  }
  return (uint8 *)&g_rom[(((addr >> 16) << 15) | (addr & 0x7fff)) & 0x3fffff];
}

void WriteReg(uint16 reg, uint8 value) {
  snes_write(g_snes, reg, value);
}

uint16 Mult8x8(uint8 a, uint8 b) {
  return a * b;
}

uint16 SnesDivide(uint16 a, uint8 b) {
  return (b == 0) ? 0xffff : a / b;
}

uint16 SnesModulus(uint16 a, uint8 b) {
  return (b == 0) ? a : a % b;
}

OamEnt *get_OamEnt(OamEnt *base, uint16 off) {
  return (OamEnt *)((uint8 *)base + off);
}

PointU16 *get_PointU16(PointU16 *base, uint8 off) {
  return (PointU16 *)((uint8 *)base + off);
}


uint8 ReadReg(uint16 reg) {
  return snes_read(g_snes, reg);
}

uint16 ReadRegWord(uint16 reg) {
  uint16_t rv = ReadReg(reg);
  rv |= ReadReg(reg + 1) << 8;
  return rv;
}

void WriteRegWord(uint16 reg, uint16 value) {
  WriteReg(reg, (uint8)value);
  WriteReg(reg + 1, value >> 8);
}

uint8 *IndirPtr(LongPtr ptr, uint16 offs) {
  uint32 a = (*(uint32 *)&ptr & 0xffffff) + offs;
  if ((a >> 16) >= 0x7e && (a >> 16) <= 0x7f || (a & 0xffff) < 0x2000) {
    return &g_ram[a & 0x1ffff];
  } else {
    return RomPtr(a);
  }
}

void IndirWriteByte(LongPtr ptr, uint16 offs, uint8 value) {
  uint8 *p = IndirPtr(ptr, offs);
  p[0] = value;
}

void SetHiLo(uint8 *hi, uint8 *lo, uint16 v) {
  *hi = v >> 8;
  *lo = v;
}

void AddHiLo(uint8 *hi, uint8 *lo, uint16 v) {
  SetHiLo(hi, lo, PAIR16(*hi, *lo) + v);
}

void RtlCheat(char c) {
  if (c == 'w') {
  } else if (c == 'q') {
  } else if (c == 'q') {
  }
}

void RtlReadSram(void) {
  char filename[64];
  snprintf(filename, sizeof(filename), "saves/%s.srm", g_rtl_game_info->title);
  FILE *f = fopen(filename, "rb");
  if (f) {
    if (fread(g_sram, 1, g_sram_size, f) != g_sram_size)
      fprintf(stderr, "Error reading %s\n", filename);
    fclose(f);
    RtlSynchronizeWholeState();
  }
}

void RtlWriteSram(void) {
  char filename[64], filename_bak[64];
  snprintf(filename, sizeof(filename), "saves/%s.srm", g_rtl_game_info->title);
  snprintf(filename_bak, sizeof(filename_bak), "saves/%s.srm.bak", g_rtl_game_info->title);
  rename(filename, filename_bak);
  FILE *f = fopen(filename, "wb");
  if (f) {
    fwrite(g_sram, 1, g_sram_size, f);
    fclose(f);
  } else {
    fprintf(stderr, "Unable to write %s\n", filename);
  }
}



void SmwCopyToVram(uint16 vram_addr, const uint8 *src, int n) {
  for (size_t i = 0; i < (n >> 1); i++)
    g_my_ppu->vram[vram_addr + i] = WORD(src[i * 2]);
}

void SmwCopyToVramPitch32(uint16 vram_addr, const uint8 *src, int n) {
  for (size_t i = 0; i < (n >> 1); i++)
    g_my_ppu->vram[vram_addr + i * 32] = WORD(src[i * 2]);
}

void SmwCopyToVramLow(uint16 vram_addr, const uint8 *src, int n) {
  for (size_t i = 0; i < n; i++)
    g_my_ppu->vram[vram_addr + i] = (g_my_ppu->vram[vram_addr + i] & 0xff00) | src[i];
}

void RtlUpdatePalette(const uint16 *src, int dst, int n) {
  for(int i = 0; i < n; i++)
    g_my_ppu->cgram[dst + i] = src[i];
}

void SmwClearVram(uint16 vram_addr, uint16 value, int n) {
  for (int i = 0; i < n; i++)
    g_my_ppu->vram[vram_addr + i] = value;
}

uint16 *RtlGetVramAddr() {
  return g_my_ppu->vram;
}

void RtlPpuWrite(uint16 addr, uint8 value) {
  assert((addr & 0xff00) == 0x2100);
  ppu_write(g_my_ppu, addr, value);
}

void RtlPpuWriteTwice(uint16 addr, uint16 value) {
  RtlPpuWrite(addr, value);
  RtlPpuWrite(addr, value >> 8);
}

void RtlHdmaSetup(uint8 which, uint8 transfer_unit, uint8 reg, uint32 addr, uint8 indirect_bank) {
  Dma *dma = g_dma;
  dma_write(dma, DMAP0 + which * 16, transfer_unit);
  dma_write(dma, BBAD0 + which * 16, reg);
  dma_write(dma, A1T0L + which * 16, addr);
  dma_write(dma, A1T0H + which * 16, addr >> 8);
  dma_write(dma, A1B0 + which * 16, addr >> 16);
  dma_write(dma, DAS00 + which * 16, indirect_bank);
}

void RtlEnableVirq(int line) {
  g_snes->vIrqEnabled = line >= 0;
  if (line >= 0)
    g_snes->vTimer = line;
}

static const uint8 kSetupHDMAWindowingEffects_DATA_00927C[] = { 0xF0,0xA0,   4,0xF0,0x80,   5,   0 };
static const uint8 *SimpleHdma_GetPtr(uint32 p) {
  if (game_id == kGameID_SMW) {
    switch (p) {
    case 0x927c: return kSetupHDMAWindowingEffects_DATA_00927C;
    }
    if (p < 0x2000)
      return g_ram + p;
  }
  printf("SimpleHdma_GetPtr: bad addr 0x%x\n", p);
  return NULL;
}

void SimpleHdma_Init(SimpleHdma *c, DmaChannel *dc) {
  if (!dc->hdmaActive) {
    c->table = 0;
    return;
  }
  c->table = SimpleHdma_GetPtr(dc->aAdr | dc->aBank << 16);
  c->rep_count = 0;
  c->mode = dc->mode | dc->indirect << 6;
  c->ppu_addr = dc->bAdr;
  c->indir_bank = dc->indBank;
}

void SimpleHdma_DoLine(SimpleHdma *c) {
  static const uint8 bAdrOffsets[8][4] = {
    {0, 0, 0, 0},
    {0, 1, 0, 1},
    {0, 0, 0, 0},
    {0, 0, 1, 1},
    {0, 1, 2, 3},
    {0, 1, 0, 1},
    {0, 0, 0, 0},
    {0, 0, 1, 1}
  };
  static const uint8 transferLength[8] = {
    1, 2, 2, 4, 4, 4, 2, 4
  };

  if (c->table == NULL)
    return;
  bool do_transfer = false;
  if ((c->rep_count & 0x7f) == 0) {
    c->rep_count = *c->table++;
    if (c->rep_count == 0) {
      c->table = NULL;
      return;
    }
    if(c->mode & 0x40) {
      c->indir_ptr = SimpleHdma_GetPtr(c->indir_bank << 16 | c->table[0] | c->table[1] * 256);
      c->table += 2;
    }
    do_transfer = true;
  }
  if(do_transfer || c->rep_count & 0x80) {
    for(int j = 0, j_end = transferLength[c->mode & 7]; j < j_end; j++) {
      uint8 v = c->mode & 0x40 ? *c->indir_ptr++ : *c->table++;
      RtlPpuWrite(0x2100 + c->ppu_addr + bAdrOffsets[c->mode & 7][j], v);
    }
  }
  c->rep_count--;
}


void LoadStripeImage_UploadToVRAM(const uint8 *pp) {  // 00871e
  while (1) {
    if ((*pp & 0x80) != 0)
      break;
    uint16 vram_addr = pp[0] << 8 | pp[1];
    uint8 vmain = __CFSHL__(pp[2], 1);
    uint8 fixed_addr = (uint8)(pp[2] & 0x40) >> 3;
    uint16 num = (swap16(WORD(pp[2])) & 0x3FFF) + 1;
    uint16 *dst = g_my_ppu->vram + vram_addr;
    pp += 4;
    
    if (fixed_addr) {
      uint16 src_data = WORD(*pp);
      int ctr = (num + 1) >> 1;
      if (vmain) {
        for (int i = 0; i < ctr; i++)
          dst[i * 32] = src_data;
      } else {
        // uhm...?
        uint8 *dst_b = (uint8 *)dst;
        for (int i = 0; i < num; i++)
          dst_b[i + ((i & 1) << 1)] = src_data;
        for (int i = 0; i < num; i += 2)
          dst_b[i + 1] = src_data >> 8;
      }
      pp += 2;
    } else {
      uint16 *src = (uint16 *)pp;
      if (vmain) {
        for (int i = 0; i < (num >> 1); i++)
          dst[i * 32] = src[i];
      } else {
        for (int i = 0; i < (num >> 1); i++)
          dst[i] = src[i];
      }
      pp += num;
    }
  }
}
