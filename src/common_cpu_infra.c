#include "common_cpu_infra.h"
#include "types.h"
#include "common_rtl.h"
#include "snes/cpu.h"
#include "snes/snes.h"
#include "tracing.h"
#include "util.h"
#include <time.h>

extern int g_got_mismatch_count;

Snes *g_snes;
Cpu *g_cpu;

bool g_calling_asm_from_c;
int g_calling_asm_from_c_ret;
bool g_fail;
extern bool g_other_image;
const RtlGameInfo *g_rtl_game_info;

static uint32 hookmode, hookcnt, hookadr;
static uint32 hooked_func_pc;
static uint8 hook_orgbyte[1024];
static uint8 hook_fixbug_orgbyte[1024];
static uint8 kPatchedCarrysOrg[1024];

static void RtlRunFrameCompare(uint16 input, int run_what);

uint8_t *SnesRomPtr(uint32 v) {
  return (uint8 *)RomPtr(v);
}

bool ProcessHook(uint32 v) {
  uint8_t *rombyte = SnesRomPtr(v);
  switch (hookmode) {
  case 0: // remove hooks
    *rombyte = hook_orgbyte[hookcnt++];
    return false;
  case 1: // install hooks
    hook_orgbyte[hookcnt++] = *rombyte;
    *rombyte = 0;
    return false;
  case 2:  // run hook
    if (v == hookadr) {
      hookmode = 3;
      return true;
    }
    return false;
  }
  return false;
}

bool FixBugHook(uint32 addr) {
  switch (hookmode) {
  case 1: { // install hooks
    uint8_t *rombyte = SnesRomPtr(addr);
    hook_fixbug_orgbyte[hookcnt++] = *rombyte;
    *rombyte = 0;
    return false;
  }
  case 2:  // run hook
    if (addr == hookadr) {
      hookmode = 3;
      return true;
    }
    hookcnt++;
    return false;
  }
  return false;
}

uint32 PatchBugs(uint32 mode, uint32 addr) {
  hookmode = mode, hookadr = addr, hookcnt = 0;
  return g_rtl_game_info->patch_bugs();
}

int RunPatchBugHook(uint32 addr) {
  uint32 new_pc = PatchBugs(2, addr);
  if (hookmode == 3) {
    if (new_pc == 0) {
      return hook_fixbug_orgbyte[hookcnt];
    } else {
      g_cpu->k = new_pc >> 16;
      g_cpu->pc = (new_pc & 0xffff) + 1;
      return *SnesRomPtr(new_pc);
    }
  }
  return -1;
}

int CpuOpcodeHook(uint32 addr) {
  for (size_t i = 0; i != g_rtl_game_info->patch_carrys_count; i++) {
    if (addr == g_rtl_game_info->patch_carrys[i]) {
      return kPatchedCarrysOrg[i];
    }
  }
  {
    int i = RunPatchBugHook(addr);
    if (i >= 0) return i;
  }
  assert(0);
  return 0;
}

bool HookedFunctionRts(int is_long) {
  if (g_calling_asm_from_c) {
    g_calling_asm_from_c_ret = is_long;
    g_calling_asm_from_c = false;
    return false;
  }
  assert(0);
  return false;
}

static void FixupCarry(uint32 addr) {
  *SnesRomPtr(addr) = 0;
}
  
Snes *SnesInit(const uint8 *data, int data_size) {
  g_my_ppu = ppu_init();
  ppu_reset(g_my_ppu);

  g_snes = snes_init(g_ram);
  g_cpu = g_snes->cpu;
  g_dma = g_snes->dma;

  RtlSetupEmuCallbacks(NULL, &RtlRunFrameCompare, NULL);

  g_snes->cart->ramSize = 2048;
  g_snes->cart->ram = calloc(1, 2048);
  g_rtl_game_info = &kSmwGameInfo;
  g_rtl_game_info->initialize();

  ppu_reset(g_snes->ppu);
  dma_reset(g_snes->dma);

  g_sram = g_snes->cart->ram;
  g_sram_size = g_snes->cart->ramSize;
  game_id = g_rtl_game_info->game_id;
  g_rtl_game_info->initialize();

  return g_snes;
}

static void RtlRunFrameCompare(uint16 input, int run_what) {
  g_snes->input1->currentState = input;

  g_snes->runningWhichVersion = 2;
  g_rtl_game_info->run_frame();
  g_snes->runningWhichVersion = 0;
}
