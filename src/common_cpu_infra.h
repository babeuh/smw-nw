#pragma once

#include "types.h"

typedef struct Snes Snes;
typedef struct Cpu Cpu;

extern Snes *g_snes;
extern Cpu *g_cpu;
extern bool g_fail;

typedef struct Snes Snes;

Snes *SnesInit(const uint8 *data, int data_size);
bool FixBugHook(uint32 addr);
uint8_t *SnesRomPtr(uint32 v);

typedef uint32 PatchBugsFunc(void);
typedef void CpuInfraInitializeFunc(void);
typedef void RunOneFrameOfGameFunc(void);

typedef struct RtlGameInfo {
  const char *title;
  uint8 game_id;
  const uint32 *patch_carrys;
  size_t patch_carrys_count;
  PatchBugsFunc *patch_bugs;
  CpuInfraInitializeFunc *initialize;
  RunOneFrameOfGameFunc *run_frame;
  RunOneFrameOfGameFunc *draw_ppu_frame;
} RtlGameInfo;

extern const RtlGameInfo kSmwGameInfo;
extern const RtlGameInfo kSmb1GameInfo;
extern const RtlGameInfo kSmbllGameInfo;
extern const RtlGameInfo *g_rtl_game_info;