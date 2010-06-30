//============================================================================================
/**
 * @file   proc_playable_end.c
 * @date   2010.06.25
 * @author obata
 * @brief  試遊台の一枚絵表示
 */
//============================================================================================

#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_data.h"

#include "system/main.h"  //HEAPID_～
#include "arc/arc_def.h" //ARCID_～
#include "../../../resource/playable_end/playable_end.naix"


typedef struct{
  HEAPID heapID;
  GAMEDATA* gamedata;
} PLAYABLE_END_WORK;


static void setup_bg_sys( HEAPID heapID );
static void release_bg_sys( void );

static GFL_PROC_RESULT PlayableEndProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT PlayableEndProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT PlayableEndProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static void _init_display( PLAYABLE_END_WORK *work );
static void _release_display( PLAYABLE_END_WORK *work );


const GFL_PROC_DATA PlayableEndProcData = {
  PlayableEndProc_Init,
  PlayableEndProc_Main,
  PlayableEndProc_End,
};


//----------------------------------------------------------------------------------
/**
 * @brief 初期化
 */
//----------------------------------------------------------------------------------
static GFL_PROC_RESULT PlayableEndProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  PLAYABLE_END_WORK* work;

  work = GFL_PROC_AllocWork( proc, sizeof(PLAYABLE_END_WORK), HEAPID_PROC );
  GFL_STD_MemClear32( work, sizeof(PLAYABLE_END_WORK) );
  work->heapID   = HEAPID_PROC;
  work->gamedata = pwk;

  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------------
/**
 * @brief メイン
 */
//----------------------------------------------------------------------------------
static GFL_PROC_RESULT PlayableEndProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  PLAYABLE_END_WORK* work = mywk;

  int trg = GFL_UI_KEY_GetTrg();

  switch( *seq ){
  case 0: 
    _init_display( work );
    (*seq)++;
    break;

  case 1:
    GX_SetVisiblePlane( GX_PLANEMASK_BG0 );
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN, 16, 0, 0 );
    (*seq)++;
    break;

  case 2:
    if( GFL_FADE_CheckFade() == FALSE ) {
      (*seq)++;
    }
    break; 

  case 3:
    if( trg & PAD_BUTTON_A ) {
      (*seq)++;
    }
    break;

  case 4:
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN, 0, 16, 0 );
    (*seq)++;
    break;

  case 5:
    if( GFL_FADE_CheckFade() == FALSE ) {
      (*seq)++;
    }
    break;

  case 6:
    _release_display( work ); 
    (*seq)++;
    break;

  case 7:
    return GFL_PROC_RES_FINISH;
  } 
  return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------------------------
/**
 * @brief 終了
 */
//----------------------------------------------------------------------------------
static GFL_PROC_RESULT PlayableEndProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GFL_PROC_FreeWork( proc );
  return GFL_PROC_RES_FINISH;
} 

/**
 * @brief 画面 初期化処理
 */
static void _init_display( PLAYABLE_END_WORK *work )
{
  GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN );
  GX_SetBankForLCDC( GX_VRAM_LCDC_ALL );
  MI_CpuClearFast( (void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE );
  GX_DisableBankForLCDC(); 
  setup_bg_sys( work->heapID );
}

/**
 * @brief 画面 終了処理
 */
static void _release_display( PLAYABLE_END_WORK *work )
{
  release_bg_sys(); 
}

/**
 * @brief BG 初期化
 */
static void setup_bg_sys( HEAPID heapID )
{
  static const GFL_DISP_VRAM SetBankData = {
    GX_VRAM_BG_128_B,             // メイン2DエンジンのBG
    GX_VRAM_BGEXTPLTT_NONE,       // メイン2DエンジンのBG拡張パレット
    GX_VRAM_SUB_BG_128_C,         // サブ2DエンジンのBG
    GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット
    GX_VRAM_OBJ_64_E,             // メイン2DエンジンのOBJ
    GX_VRAM_OBJEXTPLTT_NONE,      // メイン2DエンジンのOBJ拡張パレット
    GX_VRAM_SUB_OBJ_16_I,         // サブ2DエンジンのOBJ
    GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット
    GX_VRAM_TEX_0_A,              // テクスチャイメージスロット
    GX_VRAM_TEXPLTT_01_FG,        // テクスチャパレットスロット
    GX_OBJVRAMMODE_CHAR_1D_64K,   // メインOBJマッピングモード
    GX_OBJVRAMMODE_CHAR_1D_32K,   // サブOBJマッピングモード
  };

  static const GFL_BG_SYS_HEADER BGsys_data = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
  };

  static const GFL_BG_BGCNT_HEADER header = {
    0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
    GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,
    GX_BG_EXTPLTT_01, 1, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
  };

  GFL_BG_Init( heapID );
  GFL_DISP_SetBank( &SetBankData );
  GFL_BG_SetBGMode( &BGsys_data );
  GFL_BG_SetBGControl( GFL_BG_FRAME0_M, &header, GFL_BG_MODE_TEXT );

  GFL_ARC_UTIL_TransVramPalette( ARCID_PLAYABLE_END, NARC_playable_end_playable_end_NCLR, PALTYPE_MAIN_BG, 0x20*0, 0x20*16, heapID );
  GFL_ARC_UTIL_TransVramBgCharacter( ARCID_PLAYABLE_END, NARC_playable_end_playable_end_NCGR, GFL_BG_FRAME0_M, 0, 0, FALSE, heapID );
  GFL_ARC_UTIL_TransVramScreen( ARCID_PLAYABLE_END, NARC_playable_end_playable_end_NSCR, GFL_BG_FRAME0_M, 0, 0, FALSE, heapID );
}

/**
 * @brief BG 解放
 */
static void release_bg_sys( void )
{
  GFL_BG_Exit();
}
