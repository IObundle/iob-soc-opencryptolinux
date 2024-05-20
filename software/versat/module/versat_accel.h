// This file has been auto-generated

#ifndef INCLUDED_VERSAT_ACCELERATOR_HEADER
#define INCLUDED_VERSAT_ACCELERATOR_HEADER

#ifdef __cplusplus
#include <cstdint>
#else
#include "stdbool.h"
#include "stdint.h"
#endif

typedef intptr_t iptr;

// Config

typedef struct {
iptr ext_addr;
iptr perA;
iptr incrA;
iptr length;
iptr pingPong;
iptr iterB;
iptr perB;
iptr dutyB;
iptr startB;
iptr shiftB;
iptr incrB;
iptr reverseB;
iptr extB;
iptr iter2B;
iptr per2B;
iptr shift2B;
iptr incr2B;
iptr enableRead;
} VReadConfig;
#define VERSAT_DEFINED_VRead
typedef struct {
iptr enabled;
} SwapEndianConfig;
#define VERSAT_DEFINED_SwapEndian
typedef struct {
iptr disabled;
iptr iterA;
iptr perA;
iptr dutyA;
iptr startA;
iptr shiftA;
iptr incrA;
iptr reverseA;
iptr extA;
iptr iter2A;
iptr per2A;
iptr shift2A;
iptr incr2A;
} OnlyOutputMemConfig;
#define VERSAT_DEFINED_OnlyOutputMem
typedef struct {
iptr ext_addr;
iptr perA;
iptr incrA;
iptr length;
iptr pingPong;
iptr iterB;
iptr perB;
iptr dutyB;
iptr startB;
iptr shiftB;
iptr incrB;
iptr reverseB;
iptr extB;
iptr iter2B;
iptr per2B;
iptr shift2B;
iptr incr2B;
iptr enableWrite;
} VWriteConfig;
#define VERSAT_DEFINED_VWrite
typedef struct {
iptr constant;
} ConstConfig;
#define VERSAT_DEFINED_Const
typedef struct {
iptr stride;
} StridedMergeConfig;
#define VERSAT_DEFINED_StridedMerge
typedef struct {
iptr disabled;
iptr iterA;
iptr perA;
iptr dutyA;
iptr startA;
iptr shiftA;
iptr incrA;
iptr reverseA;
iptr extA;
iptr in0_wr;
iptr iterB;
iptr perB;
iptr dutyB;
iptr startB;
iptr shiftB;
iptr incrB;
iptr reverseB;
} ReadWriteMemConfig;
#define VERSAT_DEFINED_ReadWriteMem
typedef struct {
iptr sel;
} CombMux4Config;
#define VERSAT_DEFINED_CombMux4
typedef struct {
StridedMergeConfig Merge0;
} AES256PathConfig;
#define VERSAT_DEFINED_AES256Path
typedef struct {
ConstConfig rcon0;
ConstConfig rcon1;
ConstConfig rcon2;
ConstConfig rcon3;
ConstConfig rcon4;
ConstConfig rcon5;
ConstConfig rcon6;
AES256PathConfig mk0;
} AES256WithIterativeConfig;
#define VERSAT_DEFINED_AES256WithIterative
typedef struct {
VReadConfig MemRead;
SwapEndianConfig Swap;
iptr unused0;
iptr unused1;
iptr unused2;
iptr unused3;
iptr unused4;
iptr unused5;
iptr unused6;
iptr unused7;
iptr unused8;
iptr unused9;
iptr unused10;
iptr unused11;
iptr unused12;
iptr unused13;
iptr unused14;
iptr unused15;
iptr unused16;
iptr unused17;
iptr unused18;
iptr unused19;
iptr unused20;
iptr unused21;
iptr unused22;
iptr unused23;
iptr unused24;
iptr unused25;
iptr unused26;
iptr unused27;
iptr unused28;
iptr unused29;
iptr unused30;
iptr unused31;
iptr unused32;
iptr unused33;
iptr unused34;
iptr unused35;
iptr unused36;
iptr unused37;
iptr unused38;
iptr unused39;
iptr unused40;
iptr unused41;
iptr unused42;
iptr unused43;
iptr unused44;
iptr unused45;
iptr unused46;
iptr unused47;
iptr unused48;
iptr unused49;
iptr unused50;
iptr unused51;
iptr unused52;
iptr unused53;
iptr unused54;
iptr unused55;
iptr unused56;
iptr unused57;
iptr unused58;
iptr unused59;
iptr unused60;
iptr unused61;
} SHAConfig;
#define VERSAT_DEFINED_SHA
typedef struct {
VReadConfig key;
iptr unused0;
VReadConfig cypher;
VWriteConfig results;
AES256WithIterativeConfig aes;
iptr unused1;
iptr unused2;
iptr unused3;
iptr unused4;
iptr unused5;
iptr unused6;
iptr unused7;
iptr unused8;
iptr unused9;
iptr unused10;
iptr unused11;
iptr unused12;
iptr unused13;
iptr unused14;
iptr unused15;
iptr unused16;
iptr unused17;
iptr unused18;
} ReadWriteAES256Config;
#define VERSAT_DEFINED_ReadWriteAES256
typedef struct {
iptr unused0;
iptr unused1;
iptr unused2;
iptr unused3;
iptr unused4;
iptr unused5;
iptr unused6;
iptr unused7;
iptr unused8;
iptr unused9;
iptr unused10;
iptr unused11;
iptr unused12;
iptr unused13;
iptr unused14;
iptr unused15;
iptr unused16;
iptr unused17;
iptr unused18;
VReadConfig row;
VWriteConfig writer;
iptr unused19;
iptr unused20;
iptr unused21;
iptr unused22;
iptr unused23;
iptr unused24;
iptr unused25;
iptr unused26;
ReadWriteMemConfig mat;
ConstConfig mask;
} McElieceConfig;
#define VERSAT_DEFINED_McEliece
typedef struct {
union{
SHAConfig SHA;
ReadWriteAES256Config ReadWriteAES256;
McElieceConfig McEliece;
};
} CryptoAlgosConfig;
#define VERSAT_DEFINED_CryptoAlgos

// Address

typedef struct {
  void* addr;
} OnlyOutputMemAddr;
#define VERSAT_DEFINED_OnlyOutputMemAddr
typedef struct {
  void* addr;
} RegAddr;
#define VERSAT_DEFINED_RegAddr
typedef struct {
  void* addr;
} LookupTableAddr;
#define VERSAT_DEFINED_LookupTableAddr
typedef struct {
  LookupTableAddr mul2_0;
  LookupTableAddr mul2_1;
  LookupTableAddr mul3_0;
  LookupTableAddr mul3_1;
} DoRowAddr;
#define VERSAT_DEFINED_DoRowAddr
typedef struct {
  LookupTableAddr b0;
  LookupTableAddr b1;
} FirstLineKeyAddr;
#define VERSAT_DEFINED_FirstLineKeyAddr
typedef struct {
  LookupTableAddr b0;
  LookupTableAddr b1;
} FourthLineKeyAddr;
#define VERSAT_DEFINED_FourthLineKeyAddr
typedef struct {
  void* addr;
} ReadWriteMemAddr;
#define VERSAT_DEFINED_ReadWriteMemAddr
typedef struct {
  OnlyOutputMemAddr mem;
} ConstantsAddr;
#define VERSAT_DEFINED_ConstantsAddr
typedef struct {
  RegAddr reg;
} ShaSingleStateAddr;
#define VERSAT_DEFINED_ShaSingleStateAddr
typedef struct {
  LookupTableAddr s0;
  LookupTableAddr s1;
  LookupTableAddr s2;
  LookupTableAddr s3;
  LookupTableAddr s4;
  LookupTableAddr s5;
  LookupTableAddr s6;
  LookupTableAddr s7;
} SBoxAddr;
#define VERSAT_DEFINED_SBoxAddr
typedef struct {
  DoRowAddr d0;
  DoRowAddr d1;
  DoRowAddr d2;
  DoRowAddr d3;
} MixColumnsAddr;
#define VERSAT_DEFINED_MixColumnsAddr
typedef struct {
  FirstLineKeyAddr s;
  FourthLineKeyAddr q;
} KeySchedule256Addr;
#define VERSAT_DEFINED_KeySchedule256Addr
typedef struct {
  ShaSingleStateAddr s0;
  ShaSingleStateAddr s1;
  ShaSingleStateAddr s2;
  ShaSingleStateAddr s3;
  ShaSingleStateAddr s4;
  ShaSingleStateAddr s5;
  ShaSingleStateAddr s6;
  ShaSingleStateAddr s7;
} ShaStateAddr;
#define VERSAT_DEFINED_ShaStateAddr
typedef struct {
  SBoxAddr subBytes;
  MixColumnsAddr mixColumns;
} MainRoundAddr;
#define VERSAT_DEFINED_MainRoundAddr
typedef struct {
  MainRoundAddr round1;
  MainRoundAddr round2;
  KeySchedule256Addr key;
} RoundPairAndKeyAddr;
#define VERSAT_DEFINED_RoundPairAndKeyAddr
typedef struct {
  RoundPairAndKeyAddr roundPairAndKey;
} AES256PathAddr;
#define VERSAT_DEFINED_AES256PathAddr
typedef struct {
  AES256PathAddr mk0;
  KeySchedule256Addr key6;
  MainRoundAddr round0;
  SBoxAddr subBytes;
} AES256WithIterativeAddr;
#define VERSAT_DEFINED_AES256WithIterativeAddr
typedef struct {
  ConstantsAddr cMem0;
  ConstantsAddr cMem1;
  ConstantsAddr cMem2;
  ConstantsAddr cMem3;
  ShaStateAddr State;
  AES256WithIterativeAddr aes;
  ReadWriteMemAddr mat;
} CryptoAlgosAddr;
#define VERSAT_DEFINED_CryptoAlgosAddr

typedef struct{
  iptr TOP_MemRead_key_ext_addr;
  iptr TOP_MemRead_key_perA;
  iptr TOP_MemRead_key_incrA;
  iptr TOP_MemRead_key_length;
  iptr TOP_MemRead_key_pingPong;
  iptr TOP_MemRead_key_iterB;
  iptr TOP_MemRead_key_perB;
  iptr TOP_MemRead_key_dutyB;
  iptr TOP_MemRead_key_startB;
  iptr TOP_MemRead_key_shiftB;
  iptr TOP_MemRead_key_incrB;
  iptr TOP_MemRead_key_reverseB;
  iptr TOP_MemRead_key_extB;
  iptr TOP_MemRead_key_iter2B;
  iptr TOP_MemRead_key_per2B;
  iptr TOP_MemRead_key_shift2B;
  iptr TOP_MemRead_key_incr2B;
  iptr TOP_MemRead_key_enableRead;
  iptr TOP_Swap_enabled;
  iptr TOP_cypher_row_ext_addr;
  iptr TOP_cypher_row_perA;
  iptr TOP_cypher_row_incrA;
  iptr TOP_cypher_row_length;
  iptr TOP_cypher_row_pingPong;
  iptr TOP_cypher_row_iterB;
  iptr TOP_cypher_row_perB;
  iptr TOP_cypher_row_dutyB;
  iptr TOP_cypher_row_startB;
  iptr TOP_cypher_row_shiftB;
  iptr TOP_cypher_row_incrB;
  iptr TOP_cypher_row_reverseB;
  iptr TOP_cypher_row_extB;
  iptr TOP_cypher_row_iter2B;
  iptr TOP_cypher_row_per2B;
  iptr TOP_cypher_row_shift2B;
  iptr TOP_cypher_row_incr2B;
  iptr TOP_cypher_row_enableRead;
  iptr TOP_results_writer_ext_addr;
  iptr TOP_results_writer_perA;
  iptr TOP_results_writer_incrA;
  iptr TOP_results_writer_length;
  iptr TOP_results_writer_pingPong;
  iptr TOP_results_writer_iterB;
  iptr TOP_results_writer_perB;
  iptr TOP_results_writer_dutyB;
  iptr TOP_results_writer_startB;
  iptr TOP_results_writer_shiftB;
  iptr TOP_results_writer_incrB;
  iptr TOP_results_writer_reverseB;
  iptr TOP_results_writer_extB;
  iptr TOP_results_writer_iter2B;
  iptr TOP_results_writer_per2B;
  iptr TOP_results_writer_shift2B;
  iptr TOP_results_writer_incr2B;
  iptr TOP_results_writer_enableWrite;
  iptr TOP_aes_rcon0_constant;
  iptr TOP_aes_rcon1_constant;
  iptr TOP_aes_rcon2_constant;
  iptr TOP_aes_rcon3_constant;
  iptr TOP_aes_rcon4_constant;
  iptr TOP_aes_rcon5_constant;
  iptr TOP_aes_rcon6_constant;
  iptr TOP_aes_mk0_Merge0_stride;
  iptr TOP_mat_disabled;
  iptr TOP_mat_iterA;
  iptr TOP_mat_perA;
  iptr TOP_mat_dutyA;
  iptr TOP_mat_startA;
  iptr TOP_mat_shiftA;
  iptr TOP_mat_incrA;
  iptr TOP_mat_reverseA;
  iptr TOP_mat_extA;
  iptr TOP_mat_in0_wr;
  iptr TOP_mat_iterB;
  iptr TOP_mat_perB;
  iptr TOP_mat_dutyB;
  iptr TOP_mat_startB;
  iptr TOP_mat_shiftB;
  iptr TOP_mat_incrB;
  iptr TOP_mat_reverseB;
  iptr TOP_mask_constant;
  iptr Constants_mem_disabled;
  iptr Constants_mem_iterA;
  iptr Constants_mem_perA;
  iptr Constants_mem_dutyA;
  iptr Constants_mem_startA;
  iptr Constants_mem_shiftA;
  iptr Constants_mem_incrA;
  iptr Constants_mem_reverseA;
  iptr Constants_mem_extA;
  iptr Constants_mem_iter2A;
  iptr Constants_mem_per2A;
  iptr Constants_mem_shift2A;
  iptr Constants_mem_incr2A;
  iptr CryptoAlgos_versat_merge_mux_sel;
  iptr TOP_Delay0;
  iptr TOP_Delay1;
  iptr TOP_Delay2;
  iptr TOP_Delay3;
  iptr TOP_Delay4;
  iptr TOP_Delay5;
  iptr TOP_Delay6;
  iptr TOP_Delay7;
  iptr TOP_Delay8;
  iptr TOP_Delay9;
  iptr TOP_Delay10;
  iptr TOP_Delay11;
  iptr TOP_Delay12;
  iptr TOP_Delay13;
  iptr TOP_Delay14;
  iptr TOP_Delay15;
  iptr TOP_Delay16;
  iptr TOP_Delay17;
  iptr TOP_Delay18;
  iptr TOP_Delay19;
  iptr TOP_Delay20;
  iptr TOP_Delay21;
  iptr TOP_Delay22;
  iptr TOP_Delay23;
  iptr TOP_Delay24;
  iptr TOP_Delay25;
  iptr TOP_Delay26;
  iptr TOP_Delay27;
  iptr TOP_Delay28;
  iptr TOP_Delay29;
  iptr TOP_Delay30;
  iptr TOP_Delay31;
  iptr TOP_Delay32;
  iptr TOP_Delay33;
  iptr TOP_Delay34;
  iptr TOP_Delay35;
  iptr TOP_Delay36;
  iptr TOP_Delay37;
  iptr TOP_Delay38;
  iptr TOP_Delay39;
  iptr TOP_Delay40;
  iptr TOP_Delay41;
  iptr TOP_Delay42;
  iptr TOP_Delay43;
  iptr TOP_Delay44;
  iptr TOP_Delay45;
  iptr TOP_Delay46;
  iptr TOP_Delay47;
  iptr TOP_Delay48;
  iptr TOP_Delay49;
  iptr TOP_Delay50;
  iptr TOP_Delay51;
  iptr TOP_Delay52;
  iptr TOP_Delay53;
  iptr TOP_Delay54;
  iptr TOP_Delay55;
  iptr TOP_Delay56;
  iptr TOP_Delay57;
  iptr TOP_Delay58;
  iptr TOP_Delay59;
  iptr TOP_Delay60;
  iptr TOP_Delay61;
  iptr TOP_Delay62;
  iptr TOP_Delay63;
  iptr TOP_Delay64;
  iptr TOP_Delay65;
  iptr TOP_Delay66;
  iptr TOP_Delay67;
  iptr TOP_Delay68;
  iptr TOP_Delay69;
  iptr TOP_Delay70;
  iptr TOP_Delay71;
  iptr TOP_Delay72;
} AcceleratorConfig;

typedef struct{
  int TOP_State_s0_reg_currentValue;
  int TOP_State_s1_reg_currentValue;
  int TOP_State_s2_reg_currentValue;
  int TOP_State_s3_reg_currentValue;
  int TOP_State_s4_reg_currentValue;
  int TOP_State_s5_reg_currentValue;
  int TOP_State_s6_reg_currentValue;
  int TOP_State_s7_reg_currentValue;
} AcceleratorState;

static const int memMappedStart = 0x100000;
static const int versatAddressSpace = 2 * 0x100000;

extern iptr versat_base;

// NOTE: The address for memory mapped units depends on the address of
//       the accelerator.  Because of this, the full address can only
//       be calculated after calling versat_init(iptr), which is the
//       function that sets the versat_base variable.  It is for this
//       reason that the address info for every unit is a define. Addr
//       variables must be instantiated only after calling
//       versat_base.

// Base address for each memory mapped unit
#define TOP_cMem0_mem_addr (versat_base + memMappedStart + 0x0)
#define TOP_cMem1_mem_addr (versat_base + memMappedStart + 0x1000)
#define TOP_cMem2_mem_addr (versat_base + memMappedStart + 0x2000)
#define TOP_cMem3_mem_addr (versat_base + memMappedStart + 0x3000)
#define TOP_State_s0_reg_addr (versat_base + memMappedStart + 0x4000)
#define TOP_State_s1_reg_addr (versat_base + memMappedStart + 0x4004)
#define TOP_State_s2_reg_addr (versat_base + memMappedStart + 0x4008)
#define TOP_State_s3_reg_addr (versat_base + memMappedStart + 0x400c)
#define TOP_State_s4_reg_addr (versat_base + memMappedStart + 0x4010)
#define TOP_State_s5_reg_addr (versat_base + memMappedStart + 0x4014)
#define TOP_State_s6_reg_addr (versat_base + memMappedStart + 0x4018)
#define TOP_State_s7_reg_addr (versat_base + memMappedStart + 0x401c)
#define TOP_aes_mk0_roundPairAndKey_round1_subBytes_s0_addr (versat_base + memMappedStart + 0x10000)
#define TOP_aes_mk0_roundPairAndKey_round1_subBytes_s1_addr (versat_base + memMappedStart + 0x10100)
#define TOP_aes_mk0_roundPairAndKey_round1_subBytes_s2_addr (versat_base + memMappedStart + 0x10200)
#define TOP_aes_mk0_roundPairAndKey_round1_subBytes_s3_addr (versat_base + memMappedStart + 0x10300)
#define TOP_aes_mk0_roundPairAndKey_round1_subBytes_s4_addr (versat_base + memMappedStart + 0x10400)
#define TOP_aes_mk0_roundPairAndKey_round1_subBytes_s5_addr (versat_base + memMappedStart + 0x10500)
#define TOP_aes_mk0_roundPairAndKey_round1_subBytes_s6_addr (versat_base + memMappedStart + 0x10600)
#define TOP_aes_mk0_roundPairAndKey_round1_subBytes_s7_addr (versat_base + memMappedStart + 0x10700)
#define TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d0_mul2_0_addr (versat_base + memMappedStart + 0x11000)
#define TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d0_mul2_1_addr (versat_base + memMappedStart + 0x11100)
#define TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d0_mul3_0_addr (versat_base + memMappedStart + 0x11200)
#define TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d0_mul3_1_addr (versat_base + memMappedStart + 0x11300)
#define TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d1_mul2_0_addr (versat_base + memMappedStart + 0x11400)
#define TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d1_mul2_1_addr (versat_base + memMappedStart + 0x11500)
#define TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d1_mul3_0_addr (versat_base + memMappedStart + 0x11600)
#define TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d1_mul3_1_addr (versat_base + memMappedStart + 0x11700)
#define TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d2_mul2_0_addr (versat_base + memMappedStart + 0x11800)
#define TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d2_mul2_1_addr (versat_base + memMappedStart + 0x11900)
#define TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d2_mul3_0_addr (versat_base + memMappedStart + 0x11a00)
#define TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d2_mul3_1_addr (versat_base + memMappedStart + 0x11b00)
#define TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d3_mul2_0_addr (versat_base + memMappedStart + 0x11c00)
#define TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d3_mul2_1_addr (versat_base + memMappedStart + 0x11d00)
#define TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d3_mul3_0_addr (versat_base + memMappedStart + 0x11e00)
#define TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d3_mul3_1_addr (versat_base + memMappedStart + 0x11f00)
#define TOP_aes_mk0_roundPairAndKey_round2_subBytes_s0_addr (versat_base + memMappedStart + 0x12000)
#define TOP_aes_mk0_roundPairAndKey_round2_subBytes_s1_addr (versat_base + memMappedStart + 0x12100)
#define TOP_aes_mk0_roundPairAndKey_round2_subBytes_s2_addr (versat_base + memMappedStart + 0x12200)
#define TOP_aes_mk0_roundPairAndKey_round2_subBytes_s3_addr (versat_base + memMappedStart + 0x12300)
#define TOP_aes_mk0_roundPairAndKey_round2_subBytes_s4_addr (versat_base + memMappedStart + 0x12400)
#define TOP_aes_mk0_roundPairAndKey_round2_subBytes_s5_addr (versat_base + memMappedStart + 0x12500)
#define TOP_aes_mk0_roundPairAndKey_round2_subBytes_s6_addr (versat_base + memMappedStart + 0x12600)
#define TOP_aes_mk0_roundPairAndKey_round2_subBytes_s7_addr (versat_base + memMappedStart + 0x12700)
#define TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d0_mul2_0_addr (versat_base + memMappedStart + 0x13000)
#define TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d0_mul2_1_addr (versat_base + memMappedStart + 0x13100)
#define TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d0_mul3_0_addr (versat_base + memMappedStart + 0x13200)
#define TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d0_mul3_1_addr (versat_base + memMappedStart + 0x13300)
#define TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d1_mul2_0_addr (versat_base + memMappedStart + 0x13400)
#define TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d1_mul2_1_addr (versat_base + memMappedStart + 0x13500)
#define TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d1_mul3_0_addr (versat_base + memMappedStart + 0x13600)
#define TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d1_mul3_1_addr (versat_base + memMappedStart + 0x13700)
#define TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d2_mul2_0_addr (versat_base + memMappedStart + 0x13800)
#define TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d2_mul2_1_addr (versat_base + memMappedStart + 0x13900)
#define TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d2_mul3_0_addr (versat_base + memMappedStart + 0x13a00)
#define TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d2_mul3_1_addr (versat_base + memMappedStart + 0x13b00)
#define TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d3_mul2_0_addr (versat_base + memMappedStart + 0x13c00)
#define TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d3_mul2_1_addr (versat_base + memMappedStart + 0x13d00)
#define TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d3_mul3_0_addr (versat_base + memMappedStart + 0x13e00)
#define TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d3_mul3_1_addr (versat_base + memMappedStart + 0x13f00)
#define TOP_aes_mk0_roundPairAndKey_key_s_b0_addr (versat_base + memMappedStart + 0x14000)
#define TOP_aes_mk0_roundPairAndKey_key_s_b1_addr (versat_base + memMappedStart + 0x14100)
#define TOP_aes_mk0_roundPairAndKey_key_q_b0_addr (versat_base + memMappedStart + 0x14200)
#define TOP_aes_mk0_roundPairAndKey_key_q_b1_addr (versat_base + memMappedStart + 0x14300)
#define TOP_aes_key6_s_b0_addr (versat_base + memMappedStart + 0x18000)
#define TOP_aes_key6_s_b1_addr (versat_base + memMappedStart + 0x18100)
#define TOP_aes_key6_q_b0_addr (versat_base + memMappedStart + 0x18200)
#define TOP_aes_key6_q_b1_addr (versat_base + memMappedStart + 0x18300)
#define TOP_aes_round0_subBytes_s0_addr (versat_base + memMappedStart + 0x1a000)
#define TOP_aes_round0_subBytes_s1_addr (versat_base + memMappedStart + 0x1a100)
#define TOP_aes_round0_subBytes_s2_addr (versat_base + memMappedStart + 0x1a200)
#define TOP_aes_round0_subBytes_s3_addr (versat_base + memMappedStart + 0x1a300)
#define TOP_aes_round0_subBytes_s4_addr (versat_base + memMappedStart + 0x1a400)
#define TOP_aes_round0_subBytes_s5_addr (versat_base + memMappedStart + 0x1a500)
#define TOP_aes_round0_subBytes_s6_addr (versat_base + memMappedStart + 0x1a600)
#define TOP_aes_round0_subBytes_s7_addr (versat_base + memMappedStart + 0x1a700)
#define TOP_aes_round0_mixColumns_d0_mul2_0_addr (versat_base + memMappedStart + 0x1b000)
#define TOP_aes_round0_mixColumns_d0_mul2_1_addr (versat_base + memMappedStart + 0x1b100)
#define TOP_aes_round0_mixColumns_d0_mul3_0_addr (versat_base + memMappedStart + 0x1b200)
#define TOP_aes_round0_mixColumns_d0_mul3_1_addr (versat_base + memMappedStart + 0x1b300)
#define TOP_aes_round0_mixColumns_d1_mul2_0_addr (versat_base + memMappedStart + 0x1b400)
#define TOP_aes_round0_mixColumns_d1_mul2_1_addr (versat_base + memMappedStart + 0x1b500)
#define TOP_aes_round0_mixColumns_d1_mul3_0_addr (versat_base + memMappedStart + 0x1b600)
#define TOP_aes_round0_mixColumns_d1_mul3_1_addr (versat_base + memMappedStart + 0x1b700)
#define TOP_aes_round0_mixColumns_d2_mul2_0_addr (versat_base + memMappedStart + 0x1b800)
#define TOP_aes_round0_mixColumns_d2_mul2_1_addr (versat_base + memMappedStart + 0x1b900)
#define TOP_aes_round0_mixColumns_d2_mul3_0_addr (versat_base + memMappedStart + 0x1ba00)
#define TOP_aes_round0_mixColumns_d2_mul3_1_addr (versat_base + memMappedStart + 0x1bb00)
#define TOP_aes_round0_mixColumns_d3_mul2_0_addr (versat_base + memMappedStart + 0x1bc00)
#define TOP_aes_round0_mixColumns_d3_mul2_1_addr (versat_base + memMappedStart + 0x1bd00)
#define TOP_aes_round0_mixColumns_d3_mul3_0_addr (versat_base + memMappedStart + 0x1be00)
#define TOP_aes_round0_mixColumns_d3_mul3_1_addr (versat_base + memMappedStart + 0x1bf00)
#define TOP_aes_subBytes_s0_addr (versat_base + memMappedStart + 0x1c000)
#define TOP_aes_subBytes_s1_addr (versat_base + memMappedStart + 0x1c100)
#define TOP_aes_subBytes_s2_addr (versat_base + memMappedStart + 0x1c200)
#define TOP_aes_subBytes_s3_addr (versat_base + memMappedStart + 0x1c300)
#define TOP_aes_subBytes_s4_addr (versat_base + memMappedStart + 0x1c400)
#define TOP_aes_subBytes_s5_addr (versat_base + memMappedStart + 0x1c500)
#define TOP_aes_subBytes_s6_addr (versat_base + memMappedStart + 0x1c600)
#define TOP_aes_subBytes_s7_addr (versat_base + memMappedStart + 0x1c700)
#define TOP_mat_addr (versat_base + memMappedStart + 0x20000)

#define ACCELERATOR_TOP_ADDR_INIT { (void*) TOP_cMem0_mem_addr, (void*) TOP_cMem1_mem_addr, (void*) TOP_cMem2_mem_addr, (void*) TOP_cMem3_mem_addr, (void*) TOP_State_s0_reg_addr, (void*) TOP_State_s1_reg_addr, (void*) TOP_State_s2_reg_addr, (void*) TOP_State_s3_reg_addr, (void*) TOP_State_s4_reg_addr, (void*) TOP_State_s5_reg_addr, (void*) TOP_State_s6_reg_addr, (void*) TOP_State_s7_reg_addr, (void*) TOP_aes_mk0_roundPairAndKey_round1_subBytes_s0_addr, (void*) TOP_aes_mk0_roundPairAndKey_round1_subBytes_s1_addr, (void*) TOP_aes_mk0_roundPairAndKey_round1_subBytes_s2_addr, (void*) TOP_aes_mk0_roundPairAndKey_round1_subBytes_s3_addr, (void*) TOP_aes_mk0_roundPairAndKey_round1_subBytes_s4_addr, (void*) TOP_aes_mk0_roundPairAndKey_round1_subBytes_s5_addr, (void*) TOP_aes_mk0_roundPairAndKey_round1_subBytes_s6_addr, (void*) TOP_aes_mk0_roundPairAndKey_round1_subBytes_s7_addr, (void*) TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d0_mul2_0_addr, (void*) TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d0_mul2_1_addr, (void*) TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d0_mul3_0_addr, (void*) TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d0_mul3_1_addr, (void*) TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d1_mul2_0_addr, (void*) TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d1_mul2_1_addr, (void*) TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d1_mul3_0_addr, (void*) TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d1_mul3_1_addr, (void*) TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d2_mul2_0_addr, (void*) TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d2_mul2_1_addr, (void*) TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d2_mul3_0_addr, (void*) TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d2_mul3_1_addr, (void*) TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d3_mul2_0_addr, (void*) TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d3_mul2_1_addr, (void*) TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d3_mul3_0_addr, (void*) TOP_aes_mk0_roundPairAndKey_round1_mixColumns_d3_mul3_1_addr, (void*) TOP_aes_mk0_roundPairAndKey_round2_subBytes_s0_addr, (void*) TOP_aes_mk0_roundPairAndKey_round2_subBytes_s1_addr, (void*) TOP_aes_mk0_roundPairAndKey_round2_subBytes_s2_addr, (void*) TOP_aes_mk0_roundPairAndKey_round2_subBytes_s3_addr, (void*) TOP_aes_mk0_roundPairAndKey_round2_subBytes_s4_addr, (void*) TOP_aes_mk0_roundPairAndKey_round2_subBytes_s5_addr, (void*) TOP_aes_mk0_roundPairAndKey_round2_subBytes_s6_addr, (void*) TOP_aes_mk0_roundPairAndKey_round2_subBytes_s7_addr, (void*) TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d0_mul2_0_addr, (void*) TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d0_mul2_1_addr, (void*) TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d0_mul3_0_addr, (void*) TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d0_mul3_1_addr, (void*) TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d1_mul2_0_addr, (void*) TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d1_mul2_1_addr, (void*) TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d1_mul3_0_addr, (void*) TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d1_mul3_1_addr, (void*) TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d2_mul2_0_addr, (void*) TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d2_mul2_1_addr, (void*) TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d2_mul3_0_addr, (void*) TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d2_mul3_1_addr, (void*) TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d3_mul2_0_addr, (void*) TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d3_mul2_1_addr, (void*) TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d3_mul3_0_addr, (void*) TOP_aes_mk0_roundPairAndKey_round2_mixColumns_d3_mul3_1_addr, (void*) TOP_aes_mk0_roundPairAndKey_key_s_b0_addr, (void*) TOP_aes_mk0_roundPairAndKey_key_s_b1_addr, (void*) TOP_aes_mk0_roundPairAndKey_key_q_b0_addr, (void*) TOP_aes_mk0_roundPairAndKey_key_q_b1_addr, (void*) TOP_aes_key6_s_b0_addr, (void*) TOP_aes_key6_s_b1_addr, (void*) TOP_aes_key6_q_b0_addr, (void*) TOP_aes_key6_q_b1_addr, (void*) TOP_aes_round0_subBytes_s0_addr, (void*) TOP_aes_round0_subBytes_s1_addr, (void*) TOP_aes_round0_subBytes_s2_addr, (void*) TOP_aes_round0_subBytes_s3_addr, (void*) TOP_aes_round0_subBytes_s4_addr, (void*) TOP_aes_round0_subBytes_s5_addr, (void*) TOP_aes_round0_subBytes_s6_addr, (void*) TOP_aes_round0_subBytes_s7_addr, (void*) TOP_aes_round0_mixColumns_d0_mul2_0_addr, (void*) TOP_aes_round0_mixColumns_d0_mul2_1_addr, (void*) TOP_aes_round0_mixColumns_d0_mul3_0_addr, (void*) TOP_aes_round0_mixColumns_d0_mul3_1_addr, (void*) TOP_aes_round0_mixColumns_d1_mul2_0_addr, (void*) TOP_aes_round0_mixColumns_d1_mul2_1_addr, (void*) TOP_aes_round0_mixColumns_d1_mul3_0_addr, (void*) TOP_aes_round0_mixColumns_d1_mul3_1_addr, (void*) TOP_aes_round0_mixColumns_d2_mul2_0_addr, (void*) TOP_aes_round0_mixColumns_d2_mul2_1_addr, (void*) TOP_aes_round0_mixColumns_d2_mul3_0_addr, (void*) TOP_aes_round0_mixColumns_d2_mul3_1_addr, (void*) TOP_aes_round0_mixColumns_d3_mul2_0_addr, (void*) TOP_aes_round0_mixColumns_d3_mul2_1_addr, (void*) TOP_aes_round0_mixColumns_d3_mul3_0_addr, (void*) TOP_aes_round0_mixColumns_d3_mul3_1_addr, (void*) TOP_aes_subBytes_s0_addr, (void*) TOP_aes_subBytes_s1_addr, (void*) TOP_aes_subBytes_s2_addr, (void*) TOP_aes_subBytes_s3_addr, (void*) TOP_aes_subBytes_s4_addr, (void*) TOP_aes_subBytes_s5_addr, (void*) TOP_aes_subBytes_s6_addr, (void*) TOP_aes_subBytes_s7_addr, (void*) TOP_mat_addr}

static unsigned int delayBuffer[] = {0x0,0x2,0x12,0x23,0x34,0x1,0x12,0x23,0x0,0x0,0x0,0x0,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x0,0x9f,0x9e,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x3};

#ifdef __cplusplus
extern "C" {
#endif

// Always call first before calling any other function.
void versat_init(int base);

// In pc-emul provides a low bound on performance.
// In sim-run refines the lower bound but still likely to be smaller than reality due to memory delays that are only present in real circuits.
int GetAcceleratorCyclesElapsed();

void RunAccelerator(int times);
void StartAccelerator();
void EndAccelerator();
void VersatMemoryCopy(void* dest,void* data,int size);
void VersatUnitWrite(int baseaddr,int index,int val);
int VersatUnitRead(int baseaddr,int index);
float VersatUnitReadFloat(int base,int index);
void SignalLoop();

// PC-Emul side functions that allow to enable or disable certain portions of the emulation
// Their embedded counterparts simply do nothing
void ConfigCreateVCD(bool value);
void ConfigSimulateDatabus(bool value); 

#ifdef __cplusplus
} // extern "C"
#endif

// Needed by PC-EMUL to correctly simulate the design, embedded compiler should remove these symbols from firmware because not used by them 
static const char* acceleratorTypeName = "CryptoAlgos";
static bool isSimpleAccelerator = true;
static bool acceleratorSupportsDMA = false;

static const int staticStart = 0x56 * sizeof(iptr);
static const int delayStart = 0x64 * sizeof(iptr);
static const int configStart = 0x5 * sizeof(iptr);
static const int stateStart = 0x5 * sizeof(int);

extern volatile AcceleratorConfig* accelConfig;
extern volatile AcceleratorState* accelState;

// Simple input and output connection for simple accelerators
#define SimpleInputStart ((iptr*) accelConfig)
#define SimpleOutputStart ((int*) accelState)

#define ACCEL_TOP_MemRead_key_ext_addr accelConfig->TOP_MemRead_key_ext_addr
#define ACCEL_TOP_MemRead_key_perA accelConfig->TOP_MemRead_key_perA
#define ACCEL_TOP_MemRead_key_incrA accelConfig->TOP_MemRead_key_incrA
#define ACCEL_TOP_MemRead_key_length accelConfig->TOP_MemRead_key_length
#define ACCEL_TOP_MemRead_key_pingPong accelConfig->TOP_MemRead_key_pingPong
#define ACCEL_TOP_MemRead_key_iterB accelConfig->TOP_MemRead_key_iterB
#define ACCEL_TOP_MemRead_key_perB accelConfig->TOP_MemRead_key_perB
#define ACCEL_TOP_MemRead_key_dutyB accelConfig->TOP_MemRead_key_dutyB
#define ACCEL_TOP_MemRead_key_startB accelConfig->TOP_MemRead_key_startB
#define ACCEL_TOP_MemRead_key_shiftB accelConfig->TOP_MemRead_key_shiftB
#define ACCEL_TOP_MemRead_key_incrB accelConfig->TOP_MemRead_key_incrB
#define ACCEL_TOP_MemRead_key_reverseB accelConfig->TOP_MemRead_key_reverseB
#define ACCEL_TOP_MemRead_key_extB accelConfig->TOP_MemRead_key_extB
#define ACCEL_TOP_MemRead_key_iter2B accelConfig->TOP_MemRead_key_iter2B
#define ACCEL_TOP_MemRead_key_per2B accelConfig->TOP_MemRead_key_per2B
#define ACCEL_TOP_MemRead_key_shift2B accelConfig->TOP_MemRead_key_shift2B
#define ACCEL_TOP_MemRead_key_incr2B accelConfig->TOP_MemRead_key_incr2B
#define ACCEL_TOP_MemRead_key_enableRead accelConfig->TOP_MemRead_key_enableRead
#define ACCEL_TOP_Swap_enabled accelConfig->TOP_Swap_enabled
#define ACCEL_TOP_cypher_row_ext_addr accelConfig->TOP_cypher_row_ext_addr
#define ACCEL_TOP_cypher_row_perA accelConfig->TOP_cypher_row_perA
#define ACCEL_TOP_cypher_row_incrA accelConfig->TOP_cypher_row_incrA
#define ACCEL_TOP_cypher_row_length accelConfig->TOP_cypher_row_length
#define ACCEL_TOP_cypher_row_pingPong accelConfig->TOP_cypher_row_pingPong
#define ACCEL_TOP_cypher_row_iterB accelConfig->TOP_cypher_row_iterB
#define ACCEL_TOP_cypher_row_perB accelConfig->TOP_cypher_row_perB
#define ACCEL_TOP_cypher_row_dutyB accelConfig->TOP_cypher_row_dutyB
#define ACCEL_TOP_cypher_row_startB accelConfig->TOP_cypher_row_startB
#define ACCEL_TOP_cypher_row_shiftB accelConfig->TOP_cypher_row_shiftB
#define ACCEL_TOP_cypher_row_incrB accelConfig->TOP_cypher_row_incrB
#define ACCEL_TOP_cypher_row_reverseB accelConfig->TOP_cypher_row_reverseB
#define ACCEL_TOP_cypher_row_extB accelConfig->TOP_cypher_row_extB
#define ACCEL_TOP_cypher_row_iter2B accelConfig->TOP_cypher_row_iter2B
#define ACCEL_TOP_cypher_row_per2B accelConfig->TOP_cypher_row_per2B
#define ACCEL_TOP_cypher_row_shift2B accelConfig->TOP_cypher_row_shift2B
#define ACCEL_TOP_cypher_row_incr2B accelConfig->TOP_cypher_row_incr2B
#define ACCEL_TOP_cypher_row_enableRead accelConfig->TOP_cypher_row_enableRead
#define ACCEL_TOP_results_writer_ext_addr accelConfig->TOP_results_writer_ext_addr
#define ACCEL_TOP_results_writer_perA accelConfig->TOP_results_writer_perA
#define ACCEL_TOP_results_writer_incrA accelConfig->TOP_results_writer_incrA
#define ACCEL_TOP_results_writer_length accelConfig->TOP_results_writer_length
#define ACCEL_TOP_results_writer_pingPong accelConfig->TOP_results_writer_pingPong
#define ACCEL_TOP_results_writer_iterB accelConfig->TOP_results_writer_iterB
#define ACCEL_TOP_results_writer_perB accelConfig->TOP_results_writer_perB
#define ACCEL_TOP_results_writer_dutyB accelConfig->TOP_results_writer_dutyB
#define ACCEL_TOP_results_writer_startB accelConfig->TOP_results_writer_startB
#define ACCEL_TOP_results_writer_shiftB accelConfig->TOP_results_writer_shiftB
#define ACCEL_TOP_results_writer_incrB accelConfig->TOP_results_writer_incrB
#define ACCEL_TOP_results_writer_reverseB accelConfig->TOP_results_writer_reverseB
#define ACCEL_TOP_results_writer_extB accelConfig->TOP_results_writer_extB
#define ACCEL_TOP_results_writer_iter2B accelConfig->TOP_results_writer_iter2B
#define ACCEL_TOP_results_writer_per2B accelConfig->TOP_results_writer_per2B
#define ACCEL_TOP_results_writer_shift2B accelConfig->TOP_results_writer_shift2B
#define ACCEL_TOP_results_writer_incr2B accelConfig->TOP_results_writer_incr2B
#define ACCEL_TOP_results_writer_enableWrite accelConfig->TOP_results_writer_enableWrite
#define ACCEL_TOP_aes_rcon0_constant accelConfig->TOP_aes_rcon0_constant
#define ACCEL_TOP_aes_rcon1_constant accelConfig->TOP_aes_rcon1_constant
#define ACCEL_TOP_aes_rcon2_constant accelConfig->TOP_aes_rcon2_constant
#define ACCEL_TOP_aes_rcon3_constant accelConfig->TOP_aes_rcon3_constant
#define ACCEL_TOP_aes_rcon4_constant accelConfig->TOP_aes_rcon4_constant
#define ACCEL_TOP_aes_rcon5_constant accelConfig->TOP_aes_rcon5_constant
#define ACCEL_TOP_aes_rcon6_constant accelConfig->TOP_aes_rcon6_constant
#define ACCEL_TOP_aes_mk0_Merge0_stride accelConfig->TOP_aes_mk0_Merge0_stride
#define ACCEL_TOP_mat_disabled accelConfig->TOP_mat_disabled
#define ACCEL_TOP_mat_iterA accelConfig->TOP_mat_iterA
#define ACCEL_TOP_mat_perA accelConfig->TOP_mat_perA
#define ACCEL_TOP_mat_dutyA accelConfig->TOP_mat_dutyA
#define ACCEL_TOP_mat_startA accelConfig->TOP_mat_startA
#define ACCEL_TOP_mat_shiftA accelConfig->TOP_mat_shiftA
#define ACCEL_TOP_mat_incrA accelConfig->TOP_mat_incrA
#define ACCEL_TOP_mat_reverseA accelConfig->TOP_mat_reverseA
#define ACCEL_TOP_mat_extA accelConfig->TOP_mat_extA
#define ACCEL_TOP_mat_in0_wr accelConfig->TOP_mat_in0_wr
#define ACCEL_TOP_mat_iterB accelConfig->TOP_mat_iterB
#define ACCEL_TOP_mat_perB accelConfig->TOP_mat_perB
#define ACCEL_TOP_mat_dutyB accelConfig->TOP_mat_dutyB
#define ACCEL_TOP_mat_startB accelConfig->TOP_mat_startB
#define ACCEL_TOP_mat_shiftB accelConfig->TOP_mat_shiftB
#define ACCEL_TOP_mat_incrB accelConfig->TOP_mat_incrB
#define ACCEL_TOP_mat_reverseB accelConfig->TOP_mat_reverseB
#define ACCEL_TOP_mask_constant accelConfig->TOP_mask_constant
#define ACCEL_Constants_mem_disabled accelConfig->Constants_mem_disabled
#define ACCEL_Constants_mem_iterA accelConfig->Constants_mem_iterA
#define ACCEL_Constants_mem_perA accelConfig->Constants_mem_perA
#define ACCEL_Constants_mem_dutyA accelConfig->Constants_mem_dutyA
#define ACCEL_Constants_mem_startA accelConfig->Constants_mem_startA
#define ACCEL_Constants_mem_shiftA accelConfig->Constants_mem_shiftA
#define ACCEL_Constants_mem_incrA accelConfig->Constants_mem_incrA
#define ACCEL_Constants_mem_reverseA accelConfig->Constants_mem_reverseA
#define ACCEL_Constants_mem_extA accelConfig->Constants_mem_extA
#define ACCEL_Constants_mem_iter2A accelConfig->Constants_mem_iter2A
#define ACCEL_Constants_mem_per2A accelConfig->Constants_mem_per2A
#define ACCEL_Constants_mem_shift2A accelConfig->Constants_mem_shift2A
#define ACCEL_Constants_mem_incr2A accelConfig->Constants_mem_incr2A
#define ACCEL_CryptoAlgos_versat_merge_mux_sel accelConfig->CryptoAlgos_versat_merge_mux_sel
#define ACCEL_TOP_Delay0 accelConfig->TOP_Delay0
#define ACCEL_TOP_Delay1 accelConfig->TOP_Delay1
#define ACCEL_TOP_Delay2 accelConfig->TOP_Delay2
#define ACCEL_TOP_Delay3 accelConfig->TOP_Delay3
#define ACCEL_TOP_Delay4 accelConfig->TOP_Delay4
#define ACCEL_TOP_Delay5 accelConfig->TOP_Delay5
#define ACCEL_TOP_Delay6 accelConfig->TOP_Delay6
#define ACCEL_TOP_Delay7 accelConfig->TOP_Delay7
#define ACCEL_TOP_Delay8 accelConfig->TOP_Delay8
#define ACCEL_TOP_Delay9 accelConfig->TOP_Delay9
#define ACCEL_TOP_Delay10 accelConfig->TOP_Delay10
#define ACCEL_TOP_Delay11 accelConfig->TOP_Delay11
#define ACCEL_TOP_Delay12 accelConfig->TOP_Delay12
#define ACCEL_TOP_Delay13 accelConfig->TOP_Delay13
#define ACCEL_TOP_Delay14 accelConfig->TOP_Delay14
#define ACCEL_TOP_Delay15 accelConfig->TOP_Delay15
#define ACCEL_TOP_Delay16 accelConfig->TOP_Delay16
#define ACCEL_TOP_Delay17 accelConfig->TOP_Delay17
#define ACCEL_TOP_Delay18 accelConfig->TOP_Delay18
#define ACCEL_TOP_Delay19 accelConfig->TOP_Delay19
#define ACCEL_TOP_Delay20 accelConfig->TOP_Delay20
#define ACCEL_TOP_Delay21 accelConfig->TOP_Delay21
#define ACCEL_TOP_Delay22 accelConfig->TOP_Delay22
#define ACCEL_TOP_Delay23 accelConfig->TOP_Delay23
#define ACCEL_TOP_Delay24 accelConfig->TOP_Delay24
#define ACCEL_TOP_Delay25 accelConfig->TOP_Delay25
#define ACCEL_TOP_Delay26 accelConfig->TOP_Delay26
#define ACCEL_TOP_Delay27 accelConfig->TOP_Delay27
#define ACCEL_TOP_Delay28 accelConfig->TOP_Delay28
#define ACCEL_TOP_Delay29 accelConfig->TOP_Delay29
#define ACCEL_TOP_Delay30 accelConfig->TOP_Delay30
#define ACCEL_TOP_Delay31 accelConfig->TOP_Delay31
#define ACCEL_TOP_Delay32 accelConfig->TOP_Delay32
#define ACCEL_TOP_Delay33 accelConfig->TOP_Delay33
#define ACCEL_TOP_Delay34 accelConfig->TOP_Delay34
#define ACCEL_TOP_Delay35 accelConfig->TOP_Delay35
#define ACCEL_TOP_Delay36 accelConfig->TOP_Delay36
#define ACCEL_TOP_Delay37 accelConfig->TOP_Delay37
#define ACCEL_TOP_Delay38 accelConfig->TOP_Delay38
#define ACCEL_TOP_Delay39 accelConfig->TOP_Delay39
#define ACCEL_TOP_Delay40 accelConfig->TOP_Delay40
#define ACCEL_TOP_Delay41 accelConfig->TOP_Delay41
#define ACCEL_TOP_Delay42 accelConfig->TOP_Delay42
#define ACCEL_TOP_Delay43 accelConfig->TOP_Delay43
#define ACCEL_TOP_Delay44 accelConfig->TOP_Delay44
#define ACCEL_TOP_Delay45 accelConfig->TOP_Delay45
#define ACCEL_TOP_Delay46 accelConfig->TOP_Delay46
#define ACCEL_TOP_Delay47 accelConfig->TOP_Delay47
#define ACCEL_TOP_Delay48 accelConfig->TOP_Delay48
#define ACCEL_TOP_Delay49 accelConfig->TOP_Delay49
#define ACCEL_TOP_Delay50 accelConfig->TOP_Delay50
#define ACCEL_TOP_Delay51 accelConfig->TOP_Delay51
#define ACCEL_TOP_Delay52 accelConfig->TOP_Delay52
#define ACCEL_TOP_Delay53 accelConfig->TOP_Delay53
#define ACCEL_TOP_Delay54 accelConfig->TOP_Delay54
#define ACCEL_TOP_Delay55 accelConfig->TOP_Delay55
#define ACCEL_TOP_Delay56 accelConfig->TOP_Delay56
#define ACCEL_TOP_Delay57 accelConfig->TOP_Delay57
#define ACCEL_TOP_Delay58 accelConfig->TOP_Delay58
#define ACCEL_TOP_Delay59 accelConfig->TOP_Delay59
#define ACCEL_TOP_Delay60 accelConfig->TOP_Delay60
#define ACCEL_TOP_Delay61 accelConfig->TOP_Delay61
#define ACCEL_TOP_Delay62 accelConfig->TOP_Delay62
#define ACCEL_TOP_Delay63 accelConfig->TOP_Delay63
#define ACCEL_TOP_Delay64 accelConfig->TOP_Delay64
#define ACCEL_TOP_Delay65 accelConfig->TOP_Delay65
#define ACCEL_TOP_Delay66 accelConfig->TOP_Delay66
#define ACCEL_TOP_Delay67 accelConfig->TOP_Delay67
#define ACCEL_TOP_Delay68 accelConfig->TOP_Delay68
#define ACCEL_TOP_Delay69 accelConfig->TOP_Delay69
#define ACCEL_TOP_Delay70 accelConfig->TOP_Delay70
#define ACCEL_TOP_Delay71 accelConfig->TOP_Delay71
#define ACCEL_TOP_Delay72 accelConfig->TOP_Delay72

#define ACCEL_TOP_State_s0_reg_currentValue accelState->TOP_State_s0_reg_currentValue
#define ACCEL_TOP_State_s1_reg_currentValue accelState->TOP_State_s1_reg_currentValue
#define ACCEL_TOP_State_s2_reg_currentValue accelState->TOP_State_s2_reg_currentValue
#define ACCEL_TOP_State_s3_reg_currentValue accelState->TOP_State_s3_reg_currentValue
#define ACCEL_TOP_State_s4_reg_currentValue accelState->TOP_State_s4_reg_currentValue
#define ACCEL_TOP_State_s5_reg_currentValue accelState->TOP_State_s5_reg_currentValue
#define ACCEL_TOP_State_s6_reg_currentValue accelState->TOP_State_s6_reg_currentValue
#define ACCEL_TOP_State_s7_reg_currentValue accelState->TOP_State_s7_reg_currentValue

typedef enum{
MergeType_SHA = 0,MergeType_ReadWriteAES256 = 1,MergeType_McEliece = 2} MergeType;

#ifdef __cplusplus
extern "C" {
#endif

static inline void ActivateMergedAccelerator(MergeType type){
   ACCEL_CryptoAlgos_versat_merge_mux_sel = (int) type;
}

#ifdef __cplusplus
}
#endif


#endif // INCLUDED_VERSAT_ACCELERATOR_HEADER

