#pragma once

typedef enum SnesRegs {
  INIDISP = 0x2100,
  OBSEL = 0x2101,
  OAMADDL = 0x2102,
  OAMADDH = 0x2103,
  OAMDATA = 0x2104,
  BGMODE = 0x2105,
  MOSAIC = 0x2106,
  BG1SC = 0x2107,
  BG2SC = 0x2108,
  BG3SC = 0x2109,
  BG4SC = 0x210A,
  BG12NBA = 0x210B,
  BG34NBA = 0x210C,
  BG1HOFS = 0x210D,
  BG1VOFS = 0x210E,
  BG2HOFS = 0x210F,
  BG2VOFS = 0x2110,
  BG3HOFS = 0x2111,
  BG3VOFS = 0x2112,
  BG4HOFS = 0x2113,
  BG4VOFS = 0x2114,
  VMAIN = 0x2115,
  VMADDL = 0x2116,
  VMADDH = 0x2117,
  VMDATAL = 0x2118,
  VMDATAH = 0x2119,
  M7SEL = 0x211A,
  M7A = 0x211B,
  M7B = 0x211C,
  M7C = 0x211D,
  M7D = 0x211E,
  M7X = 0x211F,
  M7Y = 0x2120,
  CGADD = 0x2121,
  CGDATA = 0x2122,
  W12SEL = 0x2123,
  W34SEL = 0x2124,
  WOBJSEL = 0x2125,
  WH0 = 0x2126,
  WH1 = 0x2127,
  WH2 = 0x2128,
  WH3 = 0x2129,
  WBGLOG = 0x212A,
  WOBJLOG = 0x212B,
  TM = 0x212C,
  TS = 0x212D,
  TMW = 0x212E,
  TSW = 0x212F,
  CGWSEL = 0x2130,
  CGADSUB = 0x2131,
  COLDATA = 0x2132,
  SETINI = 0x2133,
  MPYL = 0x2134,
  MPYM = 0x2135,
  MPYH = 0x2136,
  SLHV = 0x2137,
  RDOAM = 0x2138,
  RDVRAML = 0x2139,
  RDVRAMH = 0x213A,
  RDCGRAM = 0x213B,
  OPHCT = 0x213C,
  OPVCT = 0x213D,
  STAT77 = 0x213E,
  STAT78 = 0x213F,
  APUI00 = 0x2140,
  APUI01 = 0x2141,
  APUI02 = 0x2142,
  APUI03 = 0x2143,
  WMDATA = 0x2180,
  WMADDL = 0x2181,
  WMADDM = 0x2182,
  WMADDH = 0x2183,
  JOYA = 0x4016,
  JOYB = 0x4017,
  NMITIMEN = 0x4200,
  WRIO = 0x4201,
  WRMPYA = 0x4202,
  WRMPYB = 0x4203,
  WRDIVL = 0x4204,
  WRDIVH = 0x4205,
  WRDIVB = 0x4206,
  HTIMEL = 0x4207,
  HTIMEH = 0x4208,
  VTIMEL = 0x4209,
  VTIMEH = 0x420A,
  MDMAEN = 0x420B,
  HDMAEN = 0x420C,
  MEMSEL = 0x420D,
  RDNMI = 0x4210,
  TIMEUP = 0x4211,
  HVBJOY = 0x4212,
  RDIO = 0x4213,
  RDDIVL = 0x4214,
  RDDIVH = 0x4215,
  RDMPYL = 0x4216,
  RDMPYH = 0x4217,
  JOY1L = 0x4218,
  JOY1H = 0x4219,
  JOY2L = 0x421A,
  JOY2H = 0x421B,
  JOY3L = 0x421C,
  JOY3H = 0x421D,
  JOY4L = 0x421E,
  JOY4H = 0x421F,
  DMAP0 = 0x4300,
  BBAD0 = 0x4301,
  A1T0L = 0x4302,
  A1T0H = 0x4303,
  A1B0 = 0x4304,
  DAS0L = 0x4305,
  DAS0H = 0x4306,
  DAS00 = 0x4307,
  A2A0L = 0x4308,
  A2A0H = 0x4309,
  NTRL0 = 0x430A,
  UNUSED0 = 0x430B,
  MIRR0 = 0x430F,
  DMAP1 = 0x4310,
  BBAD1 = 0x4311,
  A1T1L = 0x4312,
  A1T1H = 0x4313,
  A1B1 = 0x4314,
  DAS1L = 0x4315,
  DAS1H = 0x4316,
  DAS10 = 0x4317,
  A2A1L = 0x4318,
  A2A1H = 0x4319,
  NTRL1 = 0x431A,
  UNUSED1 = 0x431B,
  MIRR1 = 0x431F,
  DMAP2 = 0x4320,
  BBAD2 = 0x4321,
  A1T2L = 0x4322,
  A1T2H = 0x4323,
  A1B2 = 0x4324,
  DAS2L = 0x4325,
  DAS2H = 0x4326,
  DAS20 = 0x4327,
  A2A2L = 0x4328,
  A2A2H = 0x4329,
  NTRL2 = 0x432A,
  UNUSED2 = 0x432B,
  MIRR2 = 0x432F,
  DMAP3 = 0x4330,
  BBAD3 = 0x4331,
  A1T3L = 0x4332,
  A1T3H = 0x4333,
  A1B3 = 0x4334,
  DAS3L = 0x4335,
  DAS3H = 0x4336,
  DAS30 = 0x4337,
  A2A3L = 0x4338,
  A2A3H = 0x4339,
  NTRL3 = 0x433A,
  UNUSED3 = 0x433B,
  MIRR3 = 0x433F,
  DMAP4 = 0x4340,
  BBAD4 = 0x4341,
  A1T4L = 0x4342,
  A1T4H = 0x4343,
  A1B4 = 0x4344,
  DAS4L = 0x4345,
  DAS4H = 0x4346,
  DAS40 = 0x4347,
  A2A4L = 0x4348,
  A2A4H = 0x4349,
  NTRL4 = 0x434A,
  UNUSED4 = 0x434B,
  MIRR4 = 0x434F,
  DMAP5 = 0x4350,
  BBAD5 = 0x4351,
  A1T5L = 0x4352,
  A1T5H = 0x4353,
  A1B5 = 0x4354,
  DAS5L = 0x4355,
  DAS5H = 0x4356,
  DAS50 = 0x4357,
  A2A5L = 0x4358,
  A2A5H = 0x4359,
  NTRL5 = 0x435A,
  UNUSED5 = 0x435B,
  MIRR5 = 0x435F,
  DMAP6 = 0x4360,
  BBAD6 = 0x4361,
  A1T6L = 0x4362,
  A1T6H = 0x4363,
  A1B6 = 0x4364,
  DAS6L = 0x4365,
  DAS6H = 0x4366,
  DAS60 = 0x4367,
  A2A6L = 0x4368,
  A2A6H = 0x4369,
  NTRL6 = 0x436A,
  UNUSED6 = 0x436B,
  MIRR6 = 0x436F,

  DMAP7 = 0x4370,
  BBAD7 = 0x4371,
  A1T7L = 0x4372,
  A1T7H = 0x4373,
  A1B7 = 0x4374,
  DAS7L = 0x4375,
  DAS7H = 0x4376,


  DAS70 = 0x4377,
  A2A7L = 0x4378,
  A2A7H = 0x4379,
  NTRL7 = 0x437A,
  UNUSED7 = 0x437B,
  MIRR7 = 0x437F,
} SnesRegs;
