#include "common_cpu_infra.h"
#include "smw_rtl.h"
#include "snes/snes.h"
#include "variables.h"
#include "funcs.h"

static const uint32 kPatchedCarrys_SMW[] = {
  0xFE1F,
  0xFE26,
  0xFE35,
  0x1807a,
  0x18081,
  0x1A2CC,
  0x1B066,
  0x0fe79,
  0x0fe80,
  0x0fe88,

  0x1DDFB,
  0x1E0DD,
  0x2AAFB,
  0x2B05B,
  0x2B0A2,
  0x2B0A4,
  0x2B1DD,
  0x2B29B,
  0x2B2F6,
  0x3AD9B,
  0x498A2,
  0x2FBF5,
  0x2FBF7,
  0x2FC11,
  0x2FC13,
  0x2FC34,
  0x2FBFA,
  0x1D021,
  0x1D028,
  0x1B182,
  0x1FDD6,
  0x2B368,
  0x2BB3E,

  0x2C061,
  0x2C06C,
  0x2AD15,
  0x02DDA1,

  0x0399DB,

  0x1BC75,
  0x1BC78,
  0x1BC7A,
  0x2B228,

  0x2f231,
  0x2f23d,
  0x2f245,

  0x3C073,
};

static uint8 preserved_db;

uint32 PatchBugs_SMW1(void) {
  if (FixBugHook(0xA33C) || FixBugHook(0xa358) || FixBugHook(0xA378)) {
    if (g_cpu->a == 0x0)
      g_cpu->a = 0x2000;
    return 0;
  } else if (FixBugHook(0x1C641)) {
    // PowerUpAndItemGFXRt_DrawCoinSprite doesn't set B
    preserved_db = g_cpu->db;
    g_cpu->db = 1;
  } else if (FixBugHook(0x1C644)) {
    g_cpu->db = preserved_db;
  } else if (FixBugHook(0x4e686)) {
    // CheckIfDestroyTileEventIsActive doesn't zero Y
    g_cpu->y = 0;
  } else if (FixBugHook(0x058AFB) || FixBugHook(0x58CE0)) {

    int lvl_setting = misc_level_mode_setting;
    int max_n = (lvl_setting == 7 || lvl_setting == 8 || lvl_setting == 10 || lvl_setting == 13) ? 28 : 16;
    // BufferScrollingTiles_Layer1_VerticalLevel reads oob
    if ((uint8)g_cpu->a >= max_n)
      g_cpu->a = 0;
  } else if (FixBugHook(0xfda5)) {
    // SpawnPlayerWaterSplashAndManyBreathBubbles Y not inited
    g_cpu->y = 0;
  } else if (FixBugHook(0xCC32)) {
    // UpdateHDMAWindowBuffer_00CC14 reads bad ptr
    if (WORD(g_ram[6]) == 0) {
      g_cpu->a = 0;
      return 0xCC34;
    }
  } else if (FixBugHook(0x04FC00)) {  // OWSpr06_KoopaKid uninited Y
    g_cpu->y = owspr_table0df5[(uint8)g_cpu->x];
  } else if (FixBugHook(0x03B830)) {  //  CheckPlayerPositionRelativeToSprite_Y in bank 3 writes to R15 instead of R14
    g_ram[0xe] = g_cpu->a;
    return 0x3b832;
  } else if (FixBugHook(0x2F2FC)) {  // Wiggler reads from spr_ylos_lo instead of hi
    g_cpu->a = spr_ypos_hi[g_cpu->x & 0xff];
    return 0x2F2Fe;
  } else if (FixBugHook(0xCAC7)) {
    // UpdateHDMAWindowBuffer_KeyholeEntry writes oob
    if (g_cpu->x >= 0x1e0)
      return 0xCAD6;
  } else if (FixBugHook(0xCA9F)) {
    // UpdateHDMAWindowBuffer_KeyholeEntry writes oob
    if (g_cpu->x >= 0x1e0)
      return 0xCAA5;
  } else if (FixBugHook(0xCA86)) {
    if (LOBYTE(g_cpu->a) == 255 || LOBYTE(g_cpu->a) == 0) g_cpu->a = 1;
  } else if (FixBugHook(0x4862E)) {
    // DrawOverworldPlayer doesn't init
    WORD(g_ram[0]) = 0;
    WORD(g_ram[4]) = 0;
  } else if (FixBugHook(0x3A0A7)) {  // Spr0A8_Blargg OOB
    g_ram[3] = (spr_table1602[g_cpu->x] != 0) * 5;
  } else if (FixBugHook(0x811D)) {
    return 0x8125;
  } else if (FixBugHook(0x80F7)) {
    return 0x80fc;
  } else if (FixBugHook(0xE3FB)) {
    g_ram[12] = g_ram[13] = 0; // R13 not initialized
  } else if (FixBugHook(0x1FD50)) {
    // Spr029_KoopaKid_Status08_IggyLarry_01FD50 may not init its outputs
    WORD(g_ram[0x14b8]) = spr_xpos_lo[g_cpu->x];
    WORD(g_ram[0x14ba]) = spr_ypos_lo[g_cpu->x]; 
  } else if (FixBugHook(0x1d7f4)) {
    WORD(g_ram[8]) = GetSprYPos(g_cpu->x);
    WORD(g_ram[10]) = GetSprXPos(g_cpu->x);
  } else if (FixBugHook(0x1ec36)) {
    g_cpu->a = 1;
  } else if (FixBugHook(0x19F1C)) {
    if (g_cpu->y >= 84)
      g_cpu->y = 0;
  } else if (FixBugHook(0x817e)) {
    return 0x8181;
  }
  return 0;
}

void SmwCpuInitialize(void) {
  if (g_rom) {
    *SnesRomPtr(0x843B) = 0x60; // remove WaitForHBlank_Entry2
    *SnesRomPtr(0x2DDA2) = 5;
    *SnesRomPtr(0xCA5AC) = 7;
  }
}

static uint32 RunCpuUntilPC(uint32 pc1, uint32 pc2) {
  uint32 addr_last = g_snes->cpu->k << 16 | g_snes->cpu->pc;

  for(;;) {
    snes_runCpu(g_snes);
//    snes_runCycle(g_snes);
    uint32 addr = g_snes->cpu->k << 16 | g_snes->cpu->pc;
    if (addr != addr_last && (addr == pc1 || addr == pc2)) {
      return addr;
    }
    addr_last = addr;
  }
}


const RtlGameInfo kSmwGameInfo = {
  "smw",
  kGameID_SMW,
  kPatchedCarrys_SMW, arraysize(kPatchedCarrys_SMW),
  &PatchBugs_SMW1,
  &SmwCpuInitialize,
  &SmwRunOneFrameOfGame,
  &SmwDrawPpuFrame,
};
