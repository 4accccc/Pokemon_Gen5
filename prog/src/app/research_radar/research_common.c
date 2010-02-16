/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー 共通定義
 * @file   research_common.c
 * @author obata
 * @date   2010.02.13
 */
///////////////////////////////////////////////////////////////////////////////// 
#include <gflib.h>


//===============================================================================
// ■VRAM-Bank 設定
//===============================================================================
const GFL_DISP_VRAM VRAMBankSettings =
{
  GX_VRAM_BG_128_A,            // MAIN-BG
  GX_VRAM_BGEXTPLTT_NONE,      // MAIN-BG-EXP-PLT
  GX_VRAM_SUB_BG_128_C,        // SUB--BG
  GX_VRAM_SUB_BGEXTPLTT_NONE,  // SUB--BG-EXP-PLT
  GX_VRAM_OBJ_64_E,            // MAIN-OBJ
  GX_VRAM_OBJEXTPLTT_NONE,     // MAIN-OBJ-EXP-PLT
  GX_VRAM_SUB_OBJ_16_I,        // SUB--OBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE, // SUB--OBJ-EXP-PLT
  GX_VRAM_TEX_0_B,             // TEX-IMG
  GX_VRAM_TEXPLTT_0_G,         // TEX-PLT
  GX_OBJVRAMMODE_CHAR_1D_32K,  // MAIN-OBJ-MAPPING-MODE
  GX_OBJVRAMMODE_CHAR_1D_32K,  // SUB--OBJ-MAPPING-MODE
}; 

//===============================================================================
// ■BGモード設定
//===============================================================================
// 2D
const GFL_BG_SYS_HEADER BGSysHeader2D = 
{
  GX_DISPMODE_GRAPHICS,   // 表示モード指定
  GX_BGMODE_0,            // ＢＧモード指定(メインスクリーン)
  GX_BGMODE_0,            // ＢＧモード指定(サブスクリーン)
  GX_BG0_AS_2D            // ＢＧ０の２Ｄ、３Ｄモード選択
}; 
// 3D
const GFL_BG_SYS_HEADER BGSysHeader3D = 
{
  GX_DISPMODE_GRAPHICS,   // 表示モード指定
  GX_BGMODE_0,            // ＢＧモード指定(メインスクリーン)
  GX_BGMODE_0,            // ＢＧモード指定(サブスクリーン)
  GX_BG0_AS_3D            // ＢＧ０の２Ｄ、３Ｄモード選択
};
