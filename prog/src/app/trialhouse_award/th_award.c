//============================================================================
/**
 *  @file   th_award.c
 *  @brief  トライアルハウス検定結果
 *  @author Koji Kawada
 *  @data   2010.03.04
 *  @note   
 *  モジュール名：TH_AWARD
 */
//============================================================================
#define HEAPID_TH_AWARD (HEAPID_AWARD)


// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "print/gf_font.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "pokeicon/pokeicon.h"
#include "ui/ui_easy_clwk.h"
#include "ui/nogear_subscreen.h"

#include "savedata/trialhouse_save.h"
#include "../../savedata/trialhouse_save_local.h"

#include "th_award_graphic.h"
#include "app/th_award.h"

// アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "script_message.naix"
#include "msg/script/msg_trialhouse.h"
#include "trialhouse_gra.naix"

// サウンド

// オーバーレイ
FS_EXTERN_OVERLAY(ui_common);


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
#define HEAP_SIZE              (0x50000)               ///< ヒープサイズ

// メインBGフレーム
#define BG_FRAME_M_REAR        (GFL_BG_FRAME1_M)
#define BG_FRAME_M_FRONT       (GFL_BG_FRAME2_M)
#define BG_FRAME_M_TEXT        (GFL_BG_FRAME3_M)

// メインBGフレームのプライオリティ
#define BG_FRAME_PRI_M_REAR    (2)
#define BG_FRAME_PRI_M_FRONT   (1)
#define BG_FRAME_PRI_M_TEXT    (0)

// メインBGパレット
// 本数
enum
{
  BG_PAL_NUM_M_GRA           = 2,
  BG_PAL_NUM_M_TEXT          = 1,
};
// 位置
enum
{
  BG_PAL_POS_M_GRA          = 0,
  BG_PAL_POS_M_TEXT         = 2,
};

// メインOBJパレット
// 本数
enum
{
  OBJ_PAL_NUM_M_STAR        = 1,
  OBJ_PAL_NUM_M_POKEICON    = 3,
};
// 位置
enum
{
  OBJ_PAL_POS_M_STAR        = 0,
  OBJ_PAL_POS_M_POKEICON    = 1,
};

// ProcMainのシーケンス
enum
{
  SEQ_START          = 0,
  SEQ_FADE_IN,
  SEQ_MAIN,
  SEQ_FADE_OUT,
  SEQ_END,
};


// ポケモン数
#define POKEMON_NUM_MAX        (4)

static const GFL_CLACTPOS pokemon_pos_single[POKEMON_NUM_MAX] =
{
  { 100, 100 },
  { 100, 100 },
  { 100, 100 },
  {   0,   0 },  // 使用しない
};
static const GFL_CLACTPOS pokemon_pos_double[POKEMON_NUM_MAX] =
{
  { 100, 100 },
  { 100, 100 },
  { 100, 100 },
  { 100, 100 },
};


// テキスト
enum
{
  TEXT_DUMMY,
  TEXT_TITLE,
  TEXT_POINT,
  TEXT_RANK,
  TEXT_MAX,
};
static const u8 bmpwin_setup[TEXT_MAX][7] =
{
  { BG_FRAME_M_TEXT,  0,  0,  1,  1, BG_PAL_POS_M_TEXT, GFL_BMP_CHRAREA_GET_F },
  { BG_FRAME_M_TEXT,  1,  4, 30,  2, BG_PAL_POS_M_TEXT, GFL_BMP_CHRAREA_GET_F },
  { BG_FRAME_M_TEXT,  4,  8, 24,  2, BG_PAL_POS_M_TEXT, GFL_BMP_CHRAREA_GET_F },
  { BG_FRAME_M_TEXT,  4, 14, 24,  2, BG_PAL_POS_M_TEXT, GFL_BMP_CHRAREA_GET_F },
};

static const u8 text_pos_y[TEXT_MAX] =
{
  0,
  0,
  0,
  0,
};

#define TEXT_COLOR_L (1)
#define TEXT_COLOR_S (2)
#define TEXT_COLOR_B (0)


// OBJ
enum
{
  OBJ_RES_NCG,
  OBJ_RES_NCL,
  OBJ_RES_NCE,
  OBJ_RES_MAX
};
#define STAR_NUM_MAX       (7)

static const GFL_CLACTPOS star_pos[STAR_NUM_MAX] =
{
  {  48, 114 },
  {  69, 107 },
  {  92, 103 },
  { 116, 101 },
  { 140, 103 },
  { 163, 107 },
  { 184, 114 },
};


// フェード
#define FADE_IN_WAIT  (0)
#define FADE_OUT_WAIT (0)


// テキスト
#define TEXT_TITLE_NAME_LEN_MAX    (17)  // EOMを含めた文字数
#define TEXT_POINT_NUMBER_LEN_MAX  (5)   // EOMを含めた文字数
#define TEXT_TITLE_SPACE   (8)  // タイトル名と検定結果の間のスペースの幅
#define TEXT_POINT_SPACE   (8)  // 点数とポイントの間のスペースの幅

// ポイント
#define POINT_NUMBER_MAX (9999)  // ここまで(含む)表示可能。これより大きい数字はこの数字で表示する
#define POINT_NUMBER_MIN (0)     // ここまで(含む)表示可能。これより小さい数字はこの数字で表示する

// ランク
typedef enum
{
  RANK_MASTER,
  RANK_ELITE,
  RANK_HYPER,
  RANK_SUPER,
  RANK_NORMAL,
  RANK_NOVICE,
  RANK_BEGINNER,
  RANK_MAX,
}
RANK;
typedef struct
{
  u32 str_id;
  u8  star_num;
  u16 min;  // min<= <=max
  u16 max;
}
RANK_INFO;
static const RANK_INFO rank_info_tbl[RANK_MAX] =
{
  { msg_trialhouse_str15, 7,             6000, POINT_NUMBER_MAX },
  { msg_trialhouse_str16, 6,             5000,             5999 },
  { msg_trialhouse_str17, 5,             4000,             4999 },
  { msg_trialhouse_str18, 4,             3000,             3999 },
  { msg_trialhouse_str19, 3,             2000,             2999 },
  { msg_trialhouse_str20, 2,             1000,             1999 },
  { msg_trialhouse_str21, 1, POINT_NUMBER_MIN,              999 },
};


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// PROC パラメータ
//=====================================
struct _TH_AWARD_PARAM
{
  u8                   sex;           ///< [in] プレイヤーの性別  // PM_MALE or PM_FEMALE  // include/pm_version.h
  const THSV_WORK*     thsv;          ///< [in] トライアルハウスセーブデータ
  BOOL                 b_download;    ///< [in] ダウンロードデータを表示するときTRUE
};

//-------------------------------------
/// PROC ワーク
//=====================================
typedef struct
{
  // ヒープ、パラメータなど
  HEAPID                      heap_id;
  TH_AWARD_PARAM*             param;
  
  // グラフィック、フォントなど
  TH_AWARD_GRAPHIC_WORK*      graphic;
  GFL_FONT*                   font;
  PRINT_QUE*                  print_que;

  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;

  // ポケモン
  u8                          pokemon_num;
  UI_EASY_CLWK_RES            pokeicon_res[POKEMON_NUM_MAX];
  GFL_CLWK*                   pokeicon_clwk[POKEMON_NUM_MAX];  

  // テキスト
  GFL_MSGDATA*                msgdata;
  GFL_BMPWIN*                 text_bmpwin[TEXT_MAX];
  BOOL                        text_finish[TEXT_MAX];  // bmpwinの転送が済んでいればTRUE
  
  // スター
  u8                          star_num;
  u32                         obj_res[OBJ_RES_MAX];
  GFL_CLWK*                   star_clwk[STAR_NUM_MAX];
}
TH_AWARD_WORK;


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void Th_Award_VBlankFunc( GFL_TCB* tcb, void* wk );

// BG
static void Th_Award_BgInit( TH_AWARD_WORK* work );
static void Th_Award_BgExit( TH_AWARD_WORK* work );

// ポケアイコン
static void Th_Award_PokeiconInit( TH_AWARD_WORK* work );
static void Th_Award_PokeiconExit( TH_AWARD_WORK* work );

// スター
static void Th_Award_StarInit( TH_AWARD_WORK* work );
static void Th_Award_StarExit( TH_AWARD_WORK* work );

// テキスト
static void Th_Award_TextInit( TH_AWARD_WORK* work );
static void Th_Award_TextExit( TH_AWARD_WORK* work );
static void Th_Award_TextMain( TH_AWARD_WORK* work );

// ランク
static RANK Th_Award_GetRank( u16 point );


//=============================================================================
/**
*  PROC
*/
//=============================================================================
static GFL_PROC_RESULT Th_Award_ProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT Th_Award_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT Th_Award_ProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk );

const GFL_PROC_DATA    TH_AWARD_ProcData =
{
  Th_Award_ProcInit,
  Th_Award_ProcMain,
  Th_Award_ProcExit,
};


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
//------------------------------------------------------------------
/**
 *  @brief           パラメータ生成
 *
 *  @param[in]       heap_id       ヒープID
 *  @param[in]       sex           プレイヤーの性別  // PM_MALE or PM_FEMALE  // include/pm_version.h
 *  @param[in]       thsv          THSV_WORK
 *  @param[in]       b_download    ダウンロードデータを表示するときTRUE
 *
 *  @retval          TH_AWARD_PARAM
 */
//------------------------------------------------------------------
TH_AWARD_PARAM*  TH_AWARD_AllocParam(
                                HEAPID               heap_id,
                                u8                   sex,           // PM_MALE or PM_FEMALE  // include/pm_version.h
                                const THSV_WORK*     thsv,
                                BOOL                 b_download )
{
  TH_AWARD_PARAM* param = GFL_HEAP_AllocMemory( heap_id, sizeof( TH_AWARD_PARAM ) );
  TH_AWARD_InitParam(
      param,
      sex,
      thsv,
      b_download );
  return param;
}

//------------------------------------------------------------------
/**
 *  @brief           パラメータ解放
 *
 *  @param[in,out]   param      パラメータ生成で生成したもの
 *
 *  @retval          
 */
//------------------------------------------------------------------
void            TH_AWARD_FreeParam(
                            TH_AWARD_PARAM*  param )
{
  GFL_HEAP_FreeMemory( param );
}

//------------------------------------------------------------------
/**
 *  @brief           パラメータを設定する
 *
 *  @param[in,out]   param         TH_AWARD_PARAM
 *  @param[in]       sex           プレイヤーの性別  // PM_MALE or PM_FEMALE  // include/pm_version.h
 *  @param[in]       thsv          THSV_WORK
 *  @param[in]       b_download    ダウンロードデータを表示するときTRUE
 *
 *  @retval          
 */
//------------------------------------------------------------------
void  TH_AWARD_InitParam(
                  TH_AWARD_PARAM*      param,
                  u8                   sex,           // PM_MALE or PM_FEMALE  // include/pm_version.h
                  const THSV_WORK*     thsv,
                  BOOL                 b_download )
{
  param->sex          = sex;
  param->thsv         = thsv;
  param->b_download   = b_download;
}


//=============================================================================
/**
*  ローカル関数定義(PROC)
*/
//=============================================================================
//-------------------------------------
/// PROC 初期化処理
//=====================================
static GFL_PROC_RESULT Th_Award_ProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  TH_AWARD_WORK*     work;

  // オーバーレイ
  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common) );

  // ヒープ、パラメータなど
  {
    GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_TH_AWARD, HEAP_SIZE );
    work = GFL_PROC_AllocWork( proc, sizeof(TH_AWARD_WORK), HEAPID_TH_AWARD );
    GFL_STD_MemClear( work, sizeof(TH_AWARD_WORK) );
    
    work->heap_id       = HEAPID_TH_AWARD;
    work->param         = (TH_AWARD_PARAM*)pwk;
  }

  // グラフィック、フォントなど
  {
    work->graphic       = TH_AWARD_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, work->heap_id );
    work->font          = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, work->heap_id );
    work->print_que     = PRINTSYS_QUE_Create( work->heap_id );
  }

  // メインBG
  GFL_BG_SetPriority( BG_FRAME_M_REAR,   BG_FRAME_PRI_M_REAR  );
  GFL_BG_SetPriority( BG_FRAME_M_FRONT,  BG_FRAME_PRI_M_FRONT );
  GFL_BG_SetPriority( BG_FRAME_M_TEXT,   BG_FRAME_PRI_M_TEXT  );

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( Th_Award_VBlankFunc, work, 1 );

  // 生成
  Th_Award_BgInit( work );
  Th_Award_PokeiconInit( work );
  Th_Award_StarInit( work );
  Th_Award_TextInit( work );

  // サブBG
  NOGEAR_SUBSCREEN_Init();
  NOGEAR_SUBSCREEN_Trans( work->heap_id, work->param->sex );

  // フェードイン(黒→見える)
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, FADE_IN_WAIT );

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 終了処理
//=====================================
static GFL_PROC_RESULT Th_Award_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  TH_AWARD_WORK*     work     = (TH_AWARD_WORK*)mywk;

  // サブBG
  NOGEAR_SUBSCREEN_Exit();

  // 破棄
  Th_Award_TextExit( work );
  Th_Award_StarExit( work );
  Th_Award_PokeiconExit( work );
  Th_Award_BgExit( work );

  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // グラフィック、フォントなど
  {
    PRINTSYS_QUE_Delete( work->print_que );
    GFL_FONT_Delete( work->font );
    TH_AWARD_GRAPHIC_Exit( work->graphic );
  }

  // ヒープ、パラメータなど
  {
    GFL_PROC_FreeWork( proc );
    GFL_HEAP_DeleteHeap( HEAPID_TH_AWARD );
  }

  // オーバーレイ
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common) );

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 主処理
//=====================================
static GFL_PROC_RESULT Th_Award_ProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  TH_AWARD_WORK*     work     = (TH_AWARD_WORK*)mywk;

  switch(*seq)
  {
  case SEQ_START:
    {
      *seq = SEQ_FADE_IN;
    }
    break;
  case SEQ_FADE_IN:
    {
      if( !GFL_FADE_CheckFade() )
      {
        *seq = SEQ_MAIN;
      }
    }
    break;
  case SEQ_MAIN:
    {
      BOOL b_end = FALSE;
      u32 x, y;
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
      {
        b_end = TRUE;
      }
      else if( GFL_UI_TP_GetPointTrg( &x, &y ) )
      {
        b_end = TRUE;
      }

      if( b_end )
      {
        *seq = SEQ_FADE_OUT;
        // フェードアウト(見える→黒)
        GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, FADE_OUT_WAIT );
      }
    }
    break;
  case SEQ_FADE_OUT:
    {
      if( !GFL_FADE_CheckFade() )
      {
        *seq = SEQ_END;
      }
    }
    break;
  case SEQ_END:
    {
      return GFL_PROC_RES_FINISH;
    }
    break;
  }

  // メイン
  Th_Award_TextMain( work );

#if 0
  {
    // スターの位置調整
    GFL_CLACTPOS pos;
    GFL_CLACT_WK_GetPos( work->star_clwk[0], &pos, CLSYS_DEFREND_MAIN );
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
    {
      OS_Printf( "( %d, %d )\n", pos.x, pos.y );
    }
    if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )     pos.y -= 1;
    if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )   pos.y += 1;
    if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )   pos.x -= 1;
    if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )  pos.x += 1;
    GFL_CLACT_WK_SetPos( work->star_clwk[0], &pos, CLSYS_DEFREND_MAIN );
  }
#endif

#if 1
  {
    // ポケアイコンの位置調整
    static u8 no = 0;
    GFL_CLACTPOS pos;

    if( no >= work->pokemon_num ) no = 0;
    GFL_CLACT_WK_GetPos( work->pokeicon_clwk[no], &pos, CLSYS_DEFREND_MAIN );
    
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y )
    {
      no++;
      if( no >= work->pokemon_num ) no = 0;
      GFL_CLACT_WK_GetPos( work->pokeicon_clwk[no], &pos, CLSYS_DEFREND_MAIN );
    }
    
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
    {
      OS_Printf( "pokeicon=%d ( %d, %d )\n", no, pos.x, pos.y );
    }
    if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )     pos.y -= 1;
    if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )   pos.y += 1;
    if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )   pos.x -= 1;
    if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )  pos.x += 1;
    GFL_CLACT_WK_SetPos( work->pokeicon_clwk[no], &pos, CLSYS_DEFREND_MAIN );
  }
#endif

  // 2D描画
  TH_AWARD_GRAPHIC_2D_Draw( work->graphic );

  // 3D描画
  //TH_AWARD_GRAPHIC_3D_StartDraw( work->graphic );
  //TH_AWARD_GRAPHIC_3D_EndDraw( work->graphic );

  return GFL_PROC_RES_CONTINUE;
}

//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// VBlank関数
//=====================================
static void Th_Award_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  TH_AWARD_WORK* work = (TH_AWARD_WORK*)wk;
}

//-------------------------------------
/// BG
//=====================================
static void Th_Award_BgInit( TH_AWARD_WORK* work )
{
  ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_TRIALHOUSE_GRA, work->heap_id );

  GFL_ARCHDL_UTIL_TransVramPalette(
      handle,
      NARC_trialhouse_gra_trial_bg_NCLR,
      PALTYPE_MAIN_BG,
      BG_PAL_POS_M_GRA * 0x20,
      BG_PAL_NUM_M_GRA * 0x20,
      work->heap_id );

  GFL_ARCHDL_UTIL_TransVramBgCharacter(
      handle,
      NARC_trialhouse_gra_trial_bg_NCGR,
      BG_FRAME_M_FRONT,  // BG_FRAME_M_REAR もこのキャラクタを利用する
			0,
      0,  // 全転送
      FALSE,
      work->heap_id );

  GFL_ARCHDL_UTIL_TransVramScreen(
      handle,
      NARC_trialhouse_gra_trial_bg1_NSCR,
      BG_FRAME_M_FRONT,
      0,
      0,  // 全転送
      FALSE,
      work->heap_id );

  GFL_ARCHDL_UTIL_TransVramScreen(
      handle,
      NARC_trialhouse_gra_trial_bg2_NSCR,
      BG_FRAME_M_REAR,
      0,
      0,  // 全転送
      FALSE,
      work->heap_id );

  GFL_ARC_CloseDataHandle( handle );

  GFL_BG_LoadScreenReq( BG_FRAME_M_FRONT );
  GFL_BG_LoadScreenReq( BG_FRAME_M_REAR );
}
static void Th_Award_BgExit( TH_AWARD_WORK* work )
{
  // 何もしない
}

//-------------------------------------
/// ポケアイコン
//=====================================
static void Th_Award_PokeiconInit( TH_AWARD_WORK* work )
{
  const TH_SV_COMMON_WK*  common_data;

  // NULL、ゼロ初期化
  work->pokemon_num = 0;

  // セーブデータ
  // ダウンロードデータ
  if( work->param->b_download )
  {
    common_data   = &(work->param->thsv->CommonData[1]);
  }
  // ＲＯＭデータ
  else
  {
    common_data   = &(work->param->thsv->CommonData[0]);
  }

  if( common_data->Valid == 0 ) return;  // 無効データ

  // ポケモン数


}
static void Th_Award_PokeiconExit( TH_AWARD_WORK* work )
{
}

//-------------------------------------
/// スター
//=====================================
static void Th_Award_StarInit( TH_AWARD_WORK* work )
{
  const TH_SV_COMMON_WK*  common_data;
  
  // NULL、ゼロ初期化
  work->star_num = 0;

  // セーブデータ
  // ダウンロードデータ
  if( work->param->b_download )
  {
    common_data   = &(work->param->thsv->CommonData[1]);
  }
  // ＲＯＭデータ
  else
  {
    common_data   = &(work->param->thsv->CommonData[0]);
  }

  if( common_data->Valid == 0 ) return;  // 無効データ

  // スターの個数
  {
    RANK     rank         = Th_Award_GetRank( common_data->Point );
    work->star_num = rank_info_tbl[rank].star_num;

    if( work->star_num == 0 ) return;  // スターなし
  }

  // リソース読み込み
  {
    ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_TRIALHOUSE_GRA, work->heap_id );
    work->obj_res[OBJ_RES_NCL] = GFL_CLGRP_PLTT_RegisterEx( 
                                     handle,
                                     NARC_trialhouse_gra_trial_obj_NCLR,
                                     CLSYS_DRAW_MAIN,
                                     OBJ_PAL_POS_M_STAR*0x20,
                                     0,
                                     OBJ_PAL_NUM_M_STAR,
                                     work->heap_id );	
    work->obj_res[OBJ_RES_NCG] = GFL_CLGRP_CGR_Register(
                                     handle,
                                     NARC_trialhouse_gra_trial_obj_NCGR,
                                     FALSE,
                                     CLSYS_DRAW_MAIN,
                                     work->heap_id );
    work->obj_res[OBJ_RES_NCE] = GFL_CLGRP_CELLANIM_Register(
                                     handle,
                                     NARC_trialhouse_gra_trial_obj_NCER,
                                     NARC_trialhouse_gra_trial_obj_NANR,
                                     work->heap_id );
    GFL_ARC_CloseDataHandle( handle );
  }

  // CLWK作成
  {
    u8 i;
    GFL_CLWK_DATA cldata;
    GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
    for( i=0; i<work->star_num; i++ )
    {
      cldata.pos_x = star_pos[i].x;
      cldata.pos_y = star_pos[i].y;
      work->star_clwk[i] = GFL_CLACT_WK_Create(
                             TH_AWARD_GRAPHIC_GetClunit( work->graphic ),
                             work->obj_res[OBJ_RES_NCG], work->obj_res[OBJ_RES_NCL], work->obj_res[OBJ_RES_NCE],
                             &cldata, CLSYS_DEFREND_MAIN, work->heap_id );
      GFL_CLACT_WK_SetAnmSeq( work->star_clwk[i], 0 );
      GFL_CLACT_WK_SetAutoAnmFlag( work->star_clwk[i], TRUE );
    }
  }
}
static void Th_Award_StarExit( TH_AWARD_WORK* work )
{
  if( work->star_num > 0 )
  {
    u8 i;
    for( i=0; i<work->star_num; i++ )
    {
      GFL_CLACT_WK_Remove( work->star_clwk[i] );
    }

    GFL_CLGRP_CELLANIM_Release( work->obj_res[OBJ_RES_NCE] );
    GFL_CLGRP_CGR_Release( work->obj_res[OBJ_RES_NCG] );
    GFL_CLGRP_PLTT_Release( work->obj_res[OBJ_RES_NCL] );
  }
}

//-------------------------------------
/// テキスト
//=====================================
static void Th_Award_TextInit( TH_AWARD_WORK* work )
{
  u8 i;

  const TH_SV_COMMON_WK*  common_data;
  const STRCODE*          sv_title_name;  // work->param->b_downloadがTRUEのときのみ有効

  // NULL、ゼロ初期化
  work->msgdata = NULL;
  for( i=0; i<TEXT_MAX; i++ )
  {
    work->text_bmpwin[i] = NULL;
    work->text_finish[i] = FALSE;
  }
  work->text_finish[TEXT_DUMMY] = TRUE;  // ダミーは済んでいることにしておく

  // セーブデータ
  // ダウンロードデータ
  if( work->param->b_download )
  {
    common_data   = &(work->param->thsv->CommonData[1]);
    sv_title_name = (const STRCODE*)(work->param->thsv->Name);
  }
  // ＲＯＭデータ
  else
  {
    common_data   = &(work->param->thsv->CommonData[0]);
  }

  if( common_data->Valid == 0 ) return;  // 無効データ

  // パレット
  GFL_ARC_UTIL_TransVramPaletteEx(
      ARCID_FONT,
      NARC_font_default_nclr,
      PALTYPE_MAIN_BG,
      0,
      BG_PAL_POS_M_TEXT * 0x20,
      BG_PAL_NUM_M_TEXT * 0x20,
      work->heap_id );

  // メッセージ
  work->msgdata = GFL_MSG_Create(
                      GFL_MSG_LOAD_NORMAL,
                      ARCID_SCRIPT_MESSAGE,
                      NARC_script_message_trialhouse_dat,
                      work->heap_id );

  // ビットマップウィンドウ
  for( i=0; i<TEXT_MAX; i++ )
  {
    work->text_bmpwin[i] = GFL_BMPWIN_Create(
                                     bmpwin_setup[i][0],
                                     bmpwin_setup[i][1], bmpwin_setup[i][2], bmpwin_setup[i][3], bmpwin_setup[i][4],
                                     bmpwin_setup[i][5], bmpwin_setup[i][6] );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_bmpwin[i]), 0 );
    GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[i] );
  }

  // テキスト
  // タイトル
  {
    STRBUF* title_name_strbuf;
    STRBUF* title_result_strbuf;

    u16 title_name_str_width;
    u16 title_result_str_width;
    u16 title_str_width;
    u16 title_bmp_width;
    u16 title_name_x;
    u16 title_result_x;

    // ダウンロードデータ
    if( work->param->b_download )
    {
      title_name_strbuf = GFL_STR_CreateBuffer( TEXT_TITLE_NAME_LEN_MAX, work->heap_id );
      GFL_STR_SetStringCodeOrderLength( title_name_strbuf, sv_title_name, TEXT_TITLE_NAME_LEN_MAX );
    }
    // ＲＯＭデータ
    else
    {
      title_name_strbuf = GFL_MSG_CreateString( work->msgdata, (common_data->IsDouble==0)?(msg_trialhouse_str11):(msg_trialhouse_str12) );
    }

    title_result_strbuf = GFL_MSG_CreateString( work->msgdata, msg_trialhouse_str13 );

    title_name_str_width    = (u16)( PRINTSYS_GetStrWidth( title_name_strbuf, work->font, 0 ) );
    title_result_str_width  = (u16)( PRINTSYS_GetStrWidth( title_result_strbuf, work->font, 0 ) );
    title_str_width         = title_name_str_width + TEXT_TITLE_SPACE + title_result_str_width;

    title_bmp_width         = GFL_BMP_GetSizeX( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_TITLE]) );

    if( title_bmp_width >= title_str_width )
    {
      title_name_x = ( title_bmp_width - title_str_width ) / 2;
    }
    else
    {
      title_name_x = 0;
    }
    title_result_x = title_name_x + title_name_str_width + TEXT_TITLE_SPACE;

    PRINTSYS_PrintQueColor(
        work->print_que,
        GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_TITLE]),
        title_name_x, text_pos_y[TEXT_TITLE],
        title_name_strbuf,
        work->font,
        PRINTSYS_LSB_Make(TEXT_COLOR_L,TEXT_COLOR_S,TEXT_COLOR_B) );

    PRINTSYS_PrintQueColor(
        work->print_que,
        GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_TITLE]),
        title_result_x, text_pos_y[TEXT_TITLE],
        title_result_strbuf,
        work->font,
        PRINTSYS_LSB_Make(TEXT_COLOR_L,TEXT_COLOR_S,TEXT_COLOR_B) );

    GFL_STR_DeleteBuffer( title_result_strbuf );
    GFL_STR_DeleteBuffer( title_name_strbuf );
  }

  // ポイント
  {
    u16 point = MATH_CLAMP( common_data->Point, POINT_NUMBER_MIN, POINT_NUMBER_MAX );

    WORDSET* wordset;
    STRBUF*  number_src_strbuf;
    STRBUF*  number_strbuf;
    STRBUF*  point_strbuf;

    u16 number_str_width;
    u16 point_str_width;
    u16 str_width;
    u16 bmp_width;
    u16 number_x;
    u16 point_x;

    wordset             = WORDSET_Create( work->heap_id );
    number_src_strbuf   = GFL_MSG_CreateString( work->msgdata, msg_trialhouse_str22 );
    number_strbuf       = GFL_STR_CreateBuffer( TEXT_POINT_NUMBER_LEN_MAX, work->heap_id );
    WORDSET_RegisterNumber( wordset, 0, (s32)point, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
    WORDSET_ExpandStr( wordset, number_strbuf, number_src_strbuf );
    
    point_strbuf        = GFL_MSG_CreateString( work->msgdata, msg_trialhouse_str14 );
    
    number_str_width    = (u16)( PRINTSYS_GetStrWidth( number_strbuf, work->font, 0 ) );
    point_str_width     = (u16)( PRINTSYS_GetStrWidth( point_strbuf, work->font, 0 ) );
    str_width           = number_str_width + TEXT_POINT_SPACE + point_str_width;

    bmp_width           = GFL_BMP_GetSizeX( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_POINT]) );

    if( bmp_width >= str_width )
    {
      number_x = ( bmp_width - str_width ) / 2;
    }
    else
    {
      number_x = 0;
    }
    point_x = number_x + number_str_width + TEXT_POINT_SPACE;

    PRINTSYS_PrintQueColor(
        work->print_que,
        GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_POINT]),
        number_x, text_pos_y[TEXT_POINT],
        number_strbuf,
        work->font,
        PRINTSYS_LSB_Make(TEXT_COLOR_L,TEXT_COLOR_S,TEXT_COLOR_B) );

    PRINTSYS_PrintQueColor(
        work->print_que,
        GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_POINT]),
        point_x, text_pos_y[TEXT_POINT],
        point_strbuf,
        work->font,
        PRINTSYS_LSB_Make(TEXT_COLOR_L,TEXT_COLOR_S,TEXT_COLOR_B) );

    GFL_STR_DeleteBuffer( point_strbuf );
    GFL_STR_DeleteBuffer( number_strbuf );
    GFL_STR_DeleteBuffer( number_src_strbuf );
    WORDSET_Delete( wordset );
  }

  // ランク
  {
    RANK     rank         = Th_Award_GetRank( common_data->Point );
    STRBUF*  strbuf       = GFL_MSG_CreateString( work->msgdata, rank_info_tbl[rank].str_id );
    u16      str_width    = (u16)( PRINTSYS_GetStrWidth( strbuf, work->font, 0 ) );
    u16      bmp_width    = GFL_BMP_GetSizeX( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_RANK]) );
    u16      x            = ( bmp_width - str_width ) / 2;
    PRINTSYS_PrintQueColor(
        work->print_que,
        GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_RANK]),
        x, text_pos_y[TEXT_RANK],
        strbuf,
        work->font,
        PRINTSYS_LSB_Make(TEXT_COLOR_L,TEXT_COLOR_S,TEXT_COLOR_B) );
    GFL_STR_DeleteBuffer( strbuf );
  }

  // 既に済んでいるかもしれないので、Mainを1度呼んでおく
  Th_Award_TextMain( work );
}
static void Th_Award_TextExit( TH_AWARD_WORK* work )
{
  u8 i;

  for( i=0; i<TEXT_MAX; i++ )
  {
    if( work->text_bmpwin[i] ) GFL_BMPWIN_Delete( work->text_bmpwin[i] );
  }
  
  if( work->msgdata ) GFL_MSG_Delete( work->msgdata );
}
static void Th_Award_TextMain( TH_AWARD_WORK* work )
{
  u8 i;

  for( i=0; i<TEXT_MAX; i++ )
  {
    if( !(work->text_finish[i]) )
    {
      if( !PRINTSYS_QUE_IsExistTarget( work->print_que, GFL_BMPWIN_GetBmp(work->text_bmpwin[i]) ) )
      {
        GFL_BMPWIN_MakeTransWindow_VBlank( work->text_bmpwin[i] );
        work->text_finish[i] = TRUE;
      }
    }
  }
}

//-------------------------------------
/// ランク
//=====================================
static RANK Th_Award_GetRank( u16 point )
{
  u8 i;
  for( i=0; i<RANK_MAX -1; i++ )
  {
    if( point >= rank_info_tbl[i].min ) break;
  }
  return i;
}
