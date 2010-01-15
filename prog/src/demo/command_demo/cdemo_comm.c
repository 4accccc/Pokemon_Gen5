//============================================================================================
/**
 * @file    cdemo_comm.c
 * @brief   コマンドデモ画面 コマンド処理
 * @author  Hiroyuki Nakamura
 * @date    09.05.07
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"

#include "cdemo_main.h"
#include "cdemo_seq.h"
#include "cdemo_comm.h"

#include "op_demo.naix"


//============================================================================================
//  定数定義
//============================================================================================
//#define NCGR_START_IDX  ( NARC_ld_arceus_ds_2_00000_lz_NCGR ) // キャラ開始位置
//#define NSCR_START_IDX  ( NARC_ld_arceus_ds_2_00000_NSCR )    // スクリーン開始位置
//#define NCLR_START_IDX  ( NARC_ld_arceus_ds_2_00000_NCLR )    // パレット開始位置
#define NCGR_START_IDX  ( 0 ) // キャラ開始位置
#define NSCR_START_IDX  ( 0 )   // スクリーン開始位置
#define NCLR_START_IDX  ( 0 )   // パレット開始位置

typedef int (*pCommandFunc)(CDEMO_WORK*,const int*);  // コマンド関数


//============================================================================================
//  プロトタイプ宣言
//============================================================================================
/*
static void LoadPaletteMain( LDA_WORK * awk );
static void ObjAnmMain( LDA_WORK * awk );
static void PokeWalkObjChange( LEGEND_DEMO_WORK * wk );
*/

static int Comm_Wait( CDEMO_WORK * wk, const int * comm );
static int Comm_BlackIn( CDEMO_WORK * wk, const int * comm );
static int Comm_BlackOut( CDEMO_WORK * wk, const int * comm );
static int Comm_WhiteIn( CDEMO_WORK * wk, const int * comm );
static int Comm_WhiteOut( CDEMO_WORK * wk, const int * comm );
static int Comm_FadeMain( CDEMO_WORK * wk, const int * comm );
static int Comm_BgLoad( CDEMO_WORK * wk, const int * comm );
static int Comm_BgVanish( CDEMO_WORK * wk, const int * comm );
static int Comm_BgPriChg( CDEMO_WORK * wk, const int * comm );
static int Comm_AlphaInit( CDEMO_WORK * wk, const int * comm );
static int Comm_AlphaStart( CDEMO_WORK * wk, const int * comm );
static int Comm_AlphaNone( CDEMO_WORK * wk, const int * comm );
static int Comm_BGMSave( CDEMO_WORK * wk, const int * comm );
static int Comm_BGMStop( CDEMO_WORK * wk, const int * comm );
static int Comm_BGMPlay( CDEMO_WORK * wk, const int * comm );
static int Comm_ObjVanish( CDEMO_WORK * wk, const int * comm );
static int Comm_ObjPut( CDEMO_WORK * wk, const int * comm );
static int Comm_ObjPriChange( CDEMO_WORK * wk, const int * comm );
static int Comm_ObjBgPriChange( CDEMO_WORK * wk, const int * comm );
static int Comm_BgScreenAnime( CDEMO_WORK * wk, const int * comm );

static int Comm_DebugPrint( CDEMO_WORK * wk, const int * comm );


//============================================================================================
//  グローバル
//============================================================================================

// コマンド関数テーブル
static const pCommandFunc CommFunc[] = {
  Comm_Wait,
  Comm_BlackIn,
  Comm_BlackOut,
  Comm_WhiteIn,
  Comm_WhiteOut,
  Comm_FadeMain,

  Comm_BgLoad,
  Comm_BgVanish,
  Comm_BgPriChg,
  Comm_AlphaInit,
  Comm_AlphaStart,
  Comm_AlphaNone,

  Comm_BGMSave,
  Comm_BGMStop,
  Comm_BGMPlay,

  Comm_ObjVanish,
  Comm_ObjPut,
  Comm_ObjBgPriChange,

  Comm_BgScreenAnime,

  Comm_DebugPrint,
};

// コマンドサイズ
static const int CommandSize[] = {
  2,    // 00: ウェイト           COMM_WAIT, wait,

  3,    // 01: ブラックイン         COMM_BLACK_IN, div, sync
  3,    // 02: ブラックアウト       COMM_BLACK_OUT, div, sync
  3,    // 03: ホワイトイン         COMM_WHITE_IN, div, sync
  3,    // 04: ホワイトアウト       COMM_WHITE_OUT, div, sync
  1,    // 05: フェードメイン       COMM_FADE_MAIN,

  3,    // 06: BG読み込み         COMM_BG_LOAD, frm, graphic,
  3,    // 07: BG表示切り替え       COMM_BG_VANISH, frm, flg,
  3,    // 08: BGプライオリティ切り替え   COMM_BG_PRI_CHG, frm, pri,
  4,    // 09: アルファブレンド設定     COMM_ALPHA_INIT, plane, frm, init,
  3,    // 10: アルファブレンド開始     COMM_ALPHA_START, end, cnt,
  1,    // 11: アルファ無効         COMM_ALPHA_NONE,

  1,    // 12: 再生中のBGMを記憶      COMM_BGM_SAVE,
  1,    // 13: 再生中のBGMを停止      COMM_BGM_STOP,
  2,    // 14: BGM再生            COMM_BGM_PLAY, no

  2,    // 15: OBJ表示切り替え        COMM_OBJ_VANISH, flg,
  3,    // 16: OBJ座標設定          COMM_OBJ_PUT, x, y,
  2,    // 17: OBJとBGのプライオリティ設定  COMM_OBJ_BG_PRI_CHANGE, pri,

  4,    //  CDEMOCOMM_BG_FRM_ANIME, gra, max, wait,

  2,    // 18: デバッグプリント       COMM_DEBUG_PRINT, id

  1,    // 終了               COMM_END,
};










int CDEMOCOMM_Main( CDEMO_WORK * wk )
{
  int ret = CommFunc[ wk->commPos[0] ]( wk, wk->commPos );

  if( ret != CDEMOCOMM_RET_TRUE ){
    wk->commPos = &wk->commPos[ CommandSize[wk->commPos[0]] ];
  }

  return ret;
}


//--------------------------------------------------------------------------------------------
/**
 * コマンド：ウェイト
 *
 * @param wk    伝説デモワーク
 * @param comm  コマンドデータ
 *
 * @retval  "COMM_RET_NEXT = 次のコマンドへ"
 * @retval  "COMM_RET_TRUE = コマンド実行中"
 *
 * @li  comm[0] = COMM_WAIT
 * @li  comm[1] = wait
 */
//--------------------------------------------------------------------------------------------
static int Comm_Wait( CDEMO_WORK * wk, const int * comm )
{
  GF_ASSERT_MSG( comm[0] == CDEMOCOMM_WAIT, "COMM_WAIT: comm[0] = %d\n", comm[0] );

  if( wk->cnt == comm[1] ){
    wk->cnt = 0;
    return CDEMOCOMM_RET_NEXT;
  }
  wk->cnt++;
  return CDEMOCOMM_RET_TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * コマンド：ブラックイン
 *
 * @param wk    伝説デモワーク
 * @param comm  コマンドデータ
 *
 * @retval  "COMM_RET_FALSE = コマンド終了"
 *
 * @li  comm[0] = COMM_BLACK_IN
 * @li  comm[1] = div
 * @li  comm[2] = sync
 */
//--------------------------------------------------------------------------------------------
static int Comm_BlackIn( CDEMO_WORK * wk, const int * comm )
{
  GF_ASSERT_MSG( comm[0] == CDEMOCOMM_BLACK_IN, "COMM_BLACK_IN: comm[0] = %d\n", comm[0] );

  CDEMOMAIN_FadeInSet( wk, comm[1], comm[2], CDEMOSEQ_MAIN_MAIN );
  return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * コマンド：ブラックアウト
 *
 * @param wk    伝説デモワーク
 * @param comm  コマンドデータ
 *
 * @retval  "COMM_RET_FALSE = コマンド終了"
 *
 * @li  comm[0] = COMM_BLACK_OUT
 * @li  comm[1] = div
 * @li  comm[2] = sync
 */
//--------------------------------------------------------------------------------------------
static int Comm_BlackOut( CDEMO_WORK * wk, const int * comm )
{
  GF_ASSERT_MSG( comm[0] == CDEMOCOMM_BLACK_OUT, "COMM_BLACK_OUT: comm[0] = %d\n", comm[0] );

  CDEMOMAIN_FadeOutSet( wk, comm[1], comm[2], CDEMOSEQ_MAIN_MAIN );
  return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * コマンド：ホワイトイン
 *
 * @param wk    伝説デモワーク
 * @param comm  コマンドデータ
 *
 * @retval  "COMM_RET_FALSE = コマンド終了"
 *
 * @li  comm[0] = COMM_WHITE_IN
 * @li  comm[1] = div
 * @li  comm[2] = sync
 */
//--------------------------------------------------------------------------------------------
static int Comm_WhiteIn( CDEMO_WORK * wk, const int * comm )
{
  GF_ASSERT_MSG( comm[0] == CDEMOCOMM_WHITE_IN, "COMM_WHITE_IN: comm[0] = %d\n", comm[0] );

  CDEMOMAIN_WhiteInSet( wk, comm[1], comm[2], CDEMOSEQ_MAIN_MAIN );
  return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * コマンド：ホワイトアウト
 *
 * @param wk    伝説デモワーク
 * @param comm  コマンドデータ
 *
 * @retval  "COMM_RET_FALSE = コマンド終了"
 *
 * @li  comm[0] = COMM_WHITE_OUT
 * @li  comm[1] = div
 * @li  comm[2] = sync
 */
//--------------------------------------------------------------------------------------------
static int Comm_WhiteOut( CDEMO_WORK * wk, const int * comm )
{
  GF_ASSERT_MSG( comm[0] == CDEMOCOMM_WHITE_OUT, "COMM_WHITE_OUT: comm[0] = %d\n", comm[0] );

  CDEMOMAIN_WhiteOutSet( wk, comm[1], comm[2], CDEMOSEQ_MAIN_MAIN );
  return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * コマンド：フェードメイン
 *
 * @param wk    伝説デモワーク
 * @param comm  コマンドデータ
 *
 * @retval  "COMM_RET_NEXT = 次のコマンドへ"
 *
 * @li  comm[0] = COMM_FADE_MAIN
 */
//--------------------------------------------------------------------------------------------
static int Comm_FadeMain( CDEMO_WORK * wk, const int * comm )
{
  GF_ASSERT_MSG( comm[0] == CDEMOCOMM_FADE_MAIN, "COMM_FADE_MAIN: comm[0] = %d\n", comm[0] );

  wk->main_seq = CDEMOSEQ_MAIN_FADE;
  wk->next_seq = CDEMOSEQ_MAIN_MAIN;
  return CDEMOCOMM_RET_NEXT;
}

//--------------------------------------------------------------------------------------------
/**
 * コマンド：BG読み込み
 *
 * @param wk    伝説デモワーク
 * @param comm  コマンドデータ
 *
 * @retval  "COMM_RET_FALSE = コマンド終了"
 *
 * @li  comm[0] = COMM_BG_LOAD
 * @li  comm[1] = frm
 * @li  comm[2] = graphic
 */
//--------------------------------------------------------------------------------------------
static int Comm_BgLoad( CDEMO_WORK * wk, const int * comm )
{
  GF_ASSERT_MSG( comm[0] == CDEMOCOMM_BG_LOAD, "COMM_BG_LOAD: comm[0] = %d\n", comm[0] );

/*
  ArcUtil_HDL_BgCharSet(
    awk->ah, NCGR_START_IDX + comm[2],
    awk->bgl, comm[1], 0, 0, TRUE, HEAPID_LEGEND_DEMO );

  LoadPaletteRequest( awk, comm[1], comm[2] );

  ArcUtil_HDL_ScrnSet(
    awk->ah, NSCR_START_IDX + comm[2],
    awk->bgl, comm[1], 0, 0, TRUE, HEAPID_LEGEND_DEMO );
*/
  GFL_ARCHDL_UTIL_TransVramBgCharacter(
    wk->gra_ah, NCGR_START_IDX + comm[2], comm[1], 0, 0, TRUE, HEAPID_COMMAND_DEMO );
  if( comm[2] == 0 ){
    GFL_ARCHDL_UTIL_TransVramScreen(
      wk->gra_ah, NSCR_START_IDX + comm[2], comm[1], 0, 0, FALSE, HEAPID_COMMAND_DEMO );
  }
//  GFL_ARCHDL_UTIL_TransVramPalette(
//    wk->gra_ah, NCLR_START_IDX + comm[2], PALTYPE_MAIN_BG_EX, 0, 0, HEAPID_COMMAND_DEMO );
  CDEMOMAIN_LoadPaletteRequest( wk, comm[1], NCLR_START_IDX + comm[2] );


  return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * コマンド：BG表示切り替え
 *
 * @param wk    伝説デモワーク
 * @param comm  コマンドデータ
 *
 * @retval  "COMM_RET_FALSE = コマンド終了"
 *
 * @li  comm[0] = COMM_BG_VANISH
 * @li  comm[1] = frm
 * @li  comm[2] = flg
 */
//--------------------------------------------------------------------------------------------
static int Comm_BgVanish( CDEMO_WORK * wk, const int * comm )
{
  GF_ASSERT_MSG( comm[0] == CDEMOCOMM_BG_VANISH, "COMM_BG_VANISH: comm[0] = %d\n", comm[0] );

  GFL_BG_SetVisible( comm[1], comm[2] );
  return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * コマンド：BGプライオリティ切り替え
 *
 * @param wk    伝説デモワーク
 * @param comm  コマンドデータ
 *
 * @retval  "COMM_RET_FALSE = コマンド終了"
 *
 * @li  comm[0] = COMM_BG_PRI_CHG
 * @li  comm[1] = frm
 * @li  comm[2] = pri
 */
//--------------------------------------------------------------------------------------------
static int Comm_BgPriChg( CDEMO_WORK * wk, const int * comm )
{
  GF_ASSERT_MSG( comm[0] == CDEMOCOMM_BG_PRI_CHG, "COMM_BG_PRI_CHG: comm[0] = %d\n", comm[0] );

  GFL_BG_SetPriority( comm[1], comm[2] );
  return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * コマンド：アルファブレンド設定
 *
 * @param wk    伝説デモワーク
 * @param comm  コマンドデータ
 *
 * @retval  "COMM_RET_FALSE = コマンド終了"
 *
 * @li  comm[0] = COMM_ALPHA_INIT
 * @li  comm[1] = plane
 * @li  comm[2] = frm
 * @li  comm[3] = init
 */
//--------------------------------------------------------------------------------------------
static int Comm_AlphaInit( CDEMO_WORK * wk, const int * comm )
{
  GF_ASSERT_MSG( comm[0] == CDEMOCOMM_ALPHA_INIT, "COMM_ALPHA_INIT: comm[0] = %d\n", comm[0] );

  if( comm[1] == CDEMO_PRM_BLEND_PLANE_1 ){
    wk->alpha_plane1 = comm[2];
    wk->alpha_ev1    = comm[3];
  }else{
    wk->alpha_plane2 = comm[2];
    wk->alpha_ev2    = comm[3];
  }

  return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * コマンド：アルファブレンド開始
 *
 * @param wk    伝説デモワーク
 * @param comm  コマンドデータ
 *
 * @retval  "COMM_RET_NEXT = 次のコマンドへ"
 *
 * @li  comm[0] = COMM_ALPHA_START
 * @li  comm[1] = end
 * @li  comm[2] = cnt
 */
//--------------------------------------------------------------------------------------------
static int Comm_AlphaStart( CDEMO_WORK * wk, const int * comm )
{
  GF_ASSERT_MSG( comm[0] == CDEMOCOMM_ALPHA_START, "COMM_ALPHA_START: comm[0] = %d\n", comm[0] );

  wk->alpha_end_ev = comm[1];
  wk->alpha_mv_frm = comm[2];

  wk->main_seq = CDEMOSEQ_MAIN_ALPHA_BLEND;
  wk->next_seq = CDEMOSEQ_MAIN_MAIN;

  return CDEMOCOMM_RET_NEXT;
}

//--------------------------------------------------------------------------------------------
/**
 * コマンド：アルファ無効
 *
 * @param wk    伝説デモワーク
 * @param comm  コマンドデータ
 *
 * @retval  "COMM_RET_FALSE = コマンド終了"
 *
 * @li  comm[0] = COMM_ALPHA_NONE
 */
//--------------------------------------------------------------------------------------------
static int Comm_AlphaNone( CDEMO_WORK * wk, const int * comm )
{
  GF_ASSERT_MSG( comm[0] == CDEMOCOMM_ALPHA_NONE, "COMM_ALPHA_NONE: comm[0] = %d\n", comm[0] );

  G2_BlendNone();

  wk->alpha_plane1 = 0;
  wk->alpha_ev1    = 0;
  wk->alpha_plane2 = 0;
  wk->alpha_ev2    = 0;

  return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * コマンド：BGM記憶
 *
 * @param wk    伝説デモワーク
 * @param comm  コマンドデータ
 *
 * @retval  "COMM_RET_FALSE = コマンド終了"
 *
 * @li  comm[0] = COMM_BGM_SAVE
 */
//--------------------------------------------------------------------------------------------
static int Comm_BGMSave( CDEMO_WORK * wk, const int * comm )
{
  GF_ASSERT_MSG( comm[0] == CDEMOCOMM_BGM_SAVE, "COMM_BGM_SAVE: comm[0] = %d\n", comm[0] );

//  awk->bgm = Snd_NowBgmNoGet();

  return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * コマンド：BGM停止
 *
 * @param wk    伝説デモワーク
 * @param comm  コマンドデータ
 *
 * @retval  "COMM_RET_FALSE = コマンド終了"
 *
 * @li  comm[0] = COMM_BGM_STOP
 */
//--------------------------------------------------------------------------------------------
static int Comm_BGMStop( CDEMO_WORK * wk, const int * comm )
{
//  u16 bgm;

  GF_ASSERT_MSG( comm[0] == CDEMOCOMM_BGM_STOP, "COMM_BGM_STOP: comm[0] = %d\n", comm[0] );
/*
  bgm = Snd_NowBgmNoGet();
  Snd_BgmStop( bgm, 0 );
*/
  return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * コマンド：BGM再生
 *
 * @param wk    伝説デモワーク
 * @param comm  コマンドデータ
 *
 * @retval  "COMM_RET_FALSE = コマンド終了"
 *
 * @li  comm[0] = COMM_BGM_PLAY
 * @li  comm[1] = no
 */
//--------------------------------------------------------------------------------------------
static int Comm_BGMPlay( CDEMO_WORK * wk, const int * comm )
{
  GF_ASSERT_MSG( comm[0] == CDEMOCOMM_BGM_PLAY, "COMM_BGM_PLAY: comm[0] = %d\n", comm[0] );
/*
  if( comm[1] == PRM_BGM_PLAY_SAVE ){
    Snd_BgmPlay( awk->bgm );
  }else{
    Snd_BgmPlay( comm[1] );
  }
*/
  return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * コマンド：OBJ表示切り替え
 *
 * @param wk    伝説デモワーク
 * @param comm  コマンドデータ
 *
 * @retval  "COMM_RET_FALSE = コマンド終了"
 *
 * @li  comm[0] = COMM_OBJ_VANISH
 * @li  comm[1] = flg
 */
//--------------------------------------------------------------------------------------------
static int Comm_ObjVanish( CDEMO_WORK * wk, const int * comm )
{
  GF_ASSERT_MSG( comm[0] == CDEMOCOMM_OBJ_VANISH, "COMM_OBJ_VANISH: comm[0] = %d\n", comm[0] );

//  CATS_ObjectEnableCap( awk->cap[PRM_OBJ_ARCEUS], comm[1] );

  return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * コマンド：OBJ座標設定
 *
 * @param wk    伝説デモワーク
 * @param comm  コマンドデータ
 *
 * @retval  "COMM_RET_FALSE = コマンド終了"
 *
 * @li  comm[0] = COMM_OBJ_PUT
 * @li  comm[1] = x
 * @li  comm[2] = y
 */
//--------------------------------------------------------------------------------------------
static int Comm_ObjPut( CDEMO_WORK * wk, const int * comm )
{
  GF_ASSERT_MSG( comm[0] == CDEMOCOMM_OBJ_PUT, "COMM_OBJ_PUT: comm[0] = %d\n", comm[0] );

//  CATS_ObjectPosSetCap( awk->cap[PRM_OBJ_ARCEUS], comm[1], comm[2] );

  return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * コマンド：OBJとBGのプライオリティ設定
 *
 * @param wk    伝説デモワーク
 * @param comm  コマンドデータ
 *
 * @retval  "COMM_RET_FALSE = コマンド終了"
 *
 * @li  comm[0] = COMM_OBJ_BG_PRI_CHANGE
 * @li  comm[1] = pri
 */
//--------------------------------------------------------------------------------------------
static int Comm_ObjBgPriChange( CDEMO_WORK * wk, const int * comm )
{
  GF_ASSERT_MSG( comm[0] == CDEMOCOMM_OBJ_BG_PRI_CHANGE, "COMM_OBJ_BG_PRI_CHANGE: comm[0] = %d\n", comm[0] );

//  CATS_ObjectBGPriSetCap( awk->cap[PRM_OBJ_ARCEUS], comm[1] );

  return CDEMOCOMM_RET_FALSE;
}


/*
  0: コマンド
  1: 開始番号
  2: 枚数
  3: ウェイト
*/
static int Comm_BgScreenAnime( CDEMO_WORK * wk, const int * comm )
{
  GF_ASSERT_MSG( comm[0] == CDEMOCOMM_BG_FRM_ANIME, "COMM_BG_FRM_ANIME: comm[0] = %d\n", comm[0] );

  wk->bgsa_chr  = NCGR_START_IDX + comm[1];
  wk->bgsa_pal  = NCLR_START_IDX + comm[1];
  wk->bgsa_scr  = NSCR_START_IDX + comm[1];
  wk->bgsa_max  = comm[2];
  wk->bgsa_num  = 0;
  wk->bgsa_wait = comm[3];
  wk->bgsa_cnt  = 0;
  wk->bgsa_load = 0;
  wk->bgsa_seq  = 0;

  wk->main_seq = CDEMOSEQ_MAIN_BG_SCRN_ANM;

  return CDEMOCOMM_RET_NEXT;
}


//--------------------------------------------------------------------------------------------
/**
 * コマンド：デバッグプリント
 *
 * @param wk    伝説デモワーク
 * @param comm  コマンドデータ
 *
 * @retval  "COMM_RET_FALSE = コマンド終了"
 *
 * @li  comm[0] = COMM_DEBUG_PRINT
 */
//--------------------------------------------------------------------------------------------
static int Comm_DebugPrint( CDEMO_WORK * wk, const int * comm )
{
#ifdef PM_DEBUG
  GF_ASSERT_MSG( comm[0] == CDEMOCOMM_DEBUG_PRINT, "COMM_DEBUG_PRINT: comm[0] = %d\n", comm[0] );

  {
    RTCTime time;
    RTC_GetTime( &time );
  }
  {
    if( comm[1] == 0 ){
      wk->stick = OS_GetTick();
    }else{
      wk->etick = OS_GetTick();
    }
  }
#endif  // PM_DEBUG

  return CDEMOCOMM_RET_FALSE;
}





























#if 0
#include "common.h"

#include "system/wipe.h"
#include "system/arc_util.h"
#include "system/snd_tool.h"
#include "system/clact_tool.h"
#include "system/gra_tool.h"
#include "poketool/monsno.h"
#include "poketool/poke_tool.h"
#include "poketool/pokeparty.h"
#include "field/pair_poke_info.h"

#include "../../field/pair_poke_idx.h"
#include "../../data/mmodel/mmodel.naix"

#include "ld_main.h"
#include "ld_arceus.h"
#include "ld_arceus.naix"

#include "ld_arceus.dat"


//============================================================================================
//  定数定義
//============================================================================================


typedef int (*pCommandFunc)(LEGEND_DEMO_WORK*,const int*);  // コマンド関数

// コマンド戻り値
enum {
  COMM_RET_TRUE,    // コマンド実行中
  COMM_RET_FALSE,   // コマンド終了   ( 即次のコマンドを再生します )
  COMM_RET_NEXT,    // 次のコマンドへ ( 1sync待ちます )
};

typedef struct {
  u16 buff[256*16];
//  u16 adrs;
  u32 size;
}LDA_PALETTE;

// アルセウスデモワーク
typedef struct {
  GF_BGL_INI * bgl;
  ARCHANDLE * ah;

  // OBJ関連
  CATS_SYS_PTR  csp;
  CATS_RES_PTR  crp;
  CATS_ACT_PTR  cap[PRM_OBJ_MAX];

  const int * comm;

  u16 bgm;
  u16 dmy;

  int alpha_plane1;
  int alpha_ev1;
  int alpha_plane2;
  int alpha_ev2;
  int alpha_end_ev;
  int alpha_mv_frm;

  LDA_PALETTE pltt[3];

#ifdef  PM_DEBUG
  u32 debug_count_frm;
  u32 debug_count;
#endif  // PM_DEBUG

}LDA_WORK;

//#define NSCR_START_IDX  ( NARC_ld_arceus_big10_lz_NSCR )  // スクリーン開始位置
//#define NCGR_START_IDX  ( NARC_ld_arceus_big10_lz_NCGR )  // キャラ開始位置
//#define NCLR_START_IDX  ( NARC_ld_arceus_big10_NCLR )   // パレット開始位置
#define NSCR_START_IDX  ( NARC_ld_arceus_ds_2_00000_lz_NSCR ) // スクリーン開始位置
#define NCGR_START_IDX  ( NARC_ld_arceus_ds_2_00000_lz_NCGR ) // キャラ開始位置
#define NCLR_START_IDX  ( NARC_ld_arceus_ds_2_00000_NCLR )    // パレット開始位置

#define CLA_H_NONE  ( CLACT_U_HEADER_DATA_NONE )  // セルアクターヘッダーデータなし

#define CLACT_ID_COMMON   ( 51433 )       // この画面で使用するセルアクターのID

#define SUB_ACTOR_DISTANCE  ( 512*FX32_ONE )    // サブ画面のOBJ表示Y座標オフセット

// リソースＩＤ
enum {
  CHR_ID_POKEWALK = CLACT_ID_COMMON,
  CHR_ID_MAX = CHR_ID_POKEWALK - CLACT_ID_COMMON + 1,

  PAL_ID_POKEWALK = CLACT_ID_COMMON,
  PAL_ID_MAX = PAL_ID_POKEWALK - CLACT_ID_COMMON + 1,

  CEL_ID_POKEWALK = CLACT_ID_COMMON,
  CEL_ID_MAX = CEL_ID_POKEWALK - CLACT_ID_COMMON + 1,

  ANM_ID_POKEWALK = CLACT_ID_COMMON,
  ANM_ID_MAX = ANM_ID_POKEWALK - CLACT_ID_COMMON + 1,
};

#define POKEWALK_PAL_SIZE ( 0x20 )    // 連れ歩きポケモンのパレットサイズ



//============================================================================================
//  プロトタイプ宣言
//============================================================================================
static void LoadPaletteMain( LDA_WORK * awk );
static void ObjAnmMain( LDA_WORK * awk );
static void PokeWalkObjChange( LEGEND_DEMO_WORK * wk );

static int MainSeq_Init( LEGEND_DEMO_WORK * wk );
static int MainSeq_Release( LEGEND_DEMO_WORK * wk );
static int MainSeq_Fade( LEGEND_DEMO_WORK * wk );
static int MainSeq_AlphaBlend( LEGEND_DEMO_WORK * wk );
static int MainSeq_Main( LEGEND_DEMO_WORK * wk );

static int Comm_Wait( LEGEND_DEMO_WORK * wk, const int * comm );
static int Comm_BlackIn( LEGEND_DEMO_WORK * wk, const int * comm );
static int Comm_BlackOut( LEGEND_DEMO_WORK * wk, const int * comm );
static int Comm_WhiteIn( LEGEND_DEMO_WORK * wk, const int * comm );
static int Comm_WhiteOut( LEGEND_DEMO_WORK * wk, const int * comm );
static int Comm_FadeMain( LEGEND_DEMO_WORK * wk, const int * comm );
static int Comm_BgLoad( LEGEND_DEMO_WORK * wk, const int * comm );
static int Comm_BgVanish( LEGEND_DEMO_WORK * wk, const int * comm );
static int Comm_BgPriChg( LEGEND_DEMO_WORK * wk, const int * comm );
static int Comm_AlphaInit( LEGEND_DEMO_WORK * wk, const int * comm );
static int Comm_AlphaStart( LEGEND_DEMO_WORK * wk, const int * comm );
static int Comm_AlphaNone( LEGEND_DEMO_WORK * wk, const int * comm );
static int Comm_BGMSave( LEGEND_DEMO_WORK * wk, const int * comm );
static int Comm_BGMStop( LEGEND_DEMO_WORK * wk, const int * comm );
static int Comm_BGMPlay( LEGEND_DEMO_WORK * wk, const int * comm );
static int Comm_ObjVanish( LEGEND_DEMO_WORK * wk, const int * comm );
static int Comm_ObjPut( LEGEND_DEMO_WORK * wk, const int * comm );
static int Comm_ObjPriChange( LEGEND_DEMO_WORK * wk, const int * comm );
static int Comm_ObjBgPriChange( LEGEND_DEMO_WORK * wk, const int * comm );

static int Comm_DebugPrint( LEGEND_DEMO_WORK * wk, const int * comm );


//============================================================================================
//  グローバル変数
//============================================================================================

// VRAM割り振り
static const GF_BGL_DISPVRAM VramTbl = {
  GX_VRAM_BG_256_BC,        // メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_0123_E,   // メイン2DエンジンのBG拡張パレット

  GX_VRAM_SUB_BG_32_H,      // サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット

  GX_VRAM_OBJ_128_A,        // メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット

  GX_VRAM_SUB_OBJ_16_I,     // サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット

  GX_VRAM_TEX_NONE,       // テクスチャイメージスロット
  GX_VRAM_TEXPLTT_NONE      // テクスチャパレットスロット
};



// ＯＢＪデータ
static const TCATS_OBJECT_ADD_PARAM_S ClaPrm = {
  0, 0, 0,
  0, 0, 0, NNS_G2D_VRAM_TYPE_2DMAIN,
  { CHR_ID_POKEWALK, PAL_ID_POKEWALK, CEL_ID_POKEWALK, ANM_ID_POKEWALK, CLA_H_NONE, CLA_H_NONE },
  0, 0
};




//--------------------------------------------------------------------------------------------
/**
 * VBLANK関数
 *
 * @param work
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void VBlankFunc( void * work )
{
  LDA_WORK * awk = work;

#ifdef  PM_DEBUG
//  awk->debug_count_frm++;
//  if( awk->debug_count_frm == 60 ){
//    awk->debug_count_frm = 0;
    awk->debug_count++;
//    OS_Printf( "%d sec\n", awk->debug_count );
//  }
#endif  // PM_DEBUG

  LoadPaletteMain( awk );

  GF_BGL_VBlankFunc( awk->bgl );

  CATS_RenderOamTrans();

  OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}

//--------------------------------------------------------------------------------------------
/**
 * VRAM設定
 *
 * @param none
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void VramBankSet(void)
{
  GF_Disp_SetBank( &VramTbl );
}

//--------------------------------------------------------------------------------------------
/**
 * ＢＧ初期化
 *
 * @param awk   アルセウスデモワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BgInit( LDA_WORK * awk )
{
  awk->bgl = GF_BGL_BglIniAlloc( HEAPID_LEGEND_DEMO );

  { // BG SYSTEM
    GF_BGL_SYS_HEADER sysh = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
    };
    GF_BGL_InitBG( &sysh );
  }

  { // メイン画面：
    GF_BGL_BGCNT_HEADER cnth= {
      0, 0, 0x800, 0, GF_BGL_SCRSIZ_256x256, GX_BG_COLORMODE_256,
      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x10000, GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
    };
    GF_BGL_BGControlSet( awk->bgl, GF_BGL_FRAME0_M, &cnth, GF_BGL_MODE_TEXT );
    GF_BGL_ScrClear( awk->bgl, GF_BGL_FRAME0_M );
    GF_BGL_ClearCharSet( GF_BGL_FRAME0_M, 0x40, 0, HEAPID_LEGEND_DEMO );
  }
  { // メイン画面：
    GF_BGL_BGCNT_HEADER cnth= {
      0, 0, 0x800, 0, GF_BGL_SCRSIZ_256x256, GX_BG_COLORMODE_256,
      GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x20000, GX_BG_EXTPLTT_01,
      1, 0, 0, FALSE
    };
    GF_BGL_BGControlSet( awk->bgl, GF_BGL_FRAME1_M, &cnth, GF_BGL_MODE_TEXT );
    GF_BGL_ScrClear( awk->bgl, GF_BGL_FRAME1_M );
    GF_BGL_ClearCharSet( GF_BGL_FRAME1_M, 0x40, 0, HEAPID_LEGEND_DEMO );
  }
  { // メイン画面：
    GF_BGL_BGCNT_HEADER cnth= {
      0, 0, 0x800, 0, GF_BGL_SCRSIZ_256x256, GX_BG_COLORMODE_256,
      GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x30000, GX_BG_EXTPLTT_23,
      2, 0, 0, FALSE
    };
    GF_BGL_BGControlSet( awk->bgl, GF_BGL_FRAME2_M, &cnth, GF_BGL_MODE_TEXT );
    GF_BGL_ScrClear( awk->bgl, GF_BGL_FRAME2_M );
    GF_BGL_ClearCharSet( GF_BGL_FRAME2_M, 0x40, 0, HEAPID_LEGEND_DEMO );
  }

  GF_BGL_BackGroundColorSet( GF_BGL_FRAME0_M, 0 );
  GF_BGL_BackGroundColorSet( GF_BGL_FRAME0_S, 0 );
}

//--------------------------------------------------------------------------------------------
/**
 * ＢＧ解放
 *
 * @param awk   アルセウスデモワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BgExit( LDA_WORK * awk )
{
  GF_BGL_BGControlExit( awk->bgl, GF_BGL_FRAME2_M );
  GF_BGL_BGControlExit( awk->bgl, GF_BGL_FRAME1_M );
  GF_BGL_BGControlExit( awk->bgl, GF_BGL_FRAME0_M );

  sys_FreeMemoryEz( awk->bgl );
}

//--------------------------------------------------------------------------------------------
/**
 * ＯＢＪ初期化
 *
 * @param wk    伝説デモワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ObjInit( LEGEND_DEMO_WORK * wk )
{
/*
  LDA_WORK * awk = wk->work;

  awk->csp = CATS_AllocMemory( HEAPID_LEGEND_DEMO );
  awk->crp = CATS_ResourceCreate( awk->csp );

  {
    TCATS_OAM_INIT  coi = {
      0, 128,
      0, 32,
      0, 128,
      0, 32,
    };
    TCATS_CHAR_MANAGER_MAKE ccmm = {
      1024,
      1024*128,
      1024*16,
      GX_OBJVRAMMODE_CHAR_1D_32K,
      GX_OBJVRAMMODE_CHAR_1D_32K
    };
    CATS_SystemInit( awk->csp, &coi, &ccmm, 32 );
  }
  {
    TCATS_RESOURCE_NUM_LIST crnl = {
      CHR_ID_MAX, PAL_ID_MAX, CEL_ID_MAX, ANM_ID_MAX, 0, 0
    };
    CATS_ClactSetInit( awk->csp, awk->crp, PRM_OBJ_MAX );
    CATS_ResourceManagerInit( awk->csp, awk->crp, &crnl );
  }
  {
    CLACT_U_EASYRENDER_DATA * renddata = CATS_EasyRenderGet( awk->csp );
    CLACT_U_SetSubSurfaceMatrix( renddata, 0, 1024 * FX32_ONE );  // サブにＯＢＪを出さないので適当
  }

  // リソース読み込み
  CATS_LoadResourceCellArcH(
    awk->csp, awk->crp, awk->ah, NARC_ld_arceus_pw_obj_64_lz_NCER, 1, CEL_ID_POKEWALK );
  CATS_LoadResourceCellAnmArcH(
    awk->csp, awk->crp, awk->ah, NARC_ld_arceus_pw_obj_64_lz_NANR, 1, ANM_ID_POKEWALK );
  CATS_LoadResourcePlttArcH(
    awk->csp, awk->crp, awk->ah,
    NARC_ld_arceus_pw_obj_64_NCLR, 0, 1, NNS_G2D_VRAM_TYPE_2DMAIN, PAL_ID_POKEWALK );
  CATS_LoadResourceCharArcH(
    awk->csp, awk->crp, awk->ah,
    NARC_ld_arceus_pw_obj_64_lz_NCGR, 1, NNS_G2D_VRAM_TYPE_2DMAIN, CHR_ID_POKEWALK );

  awk->cap[PRM_OBJ_ARCEUS] = CATS_ObjectAdd_S( awk->csp, awk->crp, &ClaPrm );
  CATS_ObjectEnableCap( awk->cap[PRM_OBJ_ARCEUS], OBJ_OFF );

  PokeWalkObjChange( wk );

  GF_Disp_GX_VisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );    // MAIN DISP OBJ ON
*/
}

//--------------------------------------------------------------------------------------------
/**
 * ＯＢＪ削除
 *
 * @param awk   アルセウスデモワーク
 * @param id    OBJ ID
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ObjDel( LDA_WORK * awk, u32 id )
{
  if( awk->cap[id] != NULL ){
    CATS_ActorPointerDelete_S( awk->cap[id] );
    awk->cap[id] = NULL;
  }
}

//--------------------------------------------------------------------------------------------
/**
 * ＯＢＪ解放
 *
 * @param awk   アルセウスデモワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ObjExit( LDA_WORK * awk )
{
/*
  u32 i;

  for( i=0; i<PRM_OBJ_MAX; i++ ){
    ObjDel( awk, i );
  }
  CATS_ResourceDestructor_S( awk->csp, awk->crp );
  CATS_FreeMemory( awk->csp );
*/
}

//--------------------------------------------------------------------------------------------
/**
 * ＯＢＪアニメメイン
 *
 * @param awk   アルセウスデモワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ObjAnmMain( LDA_WORK * awk )
{
/*
  u32 i;

  for( i=0; i<PRM_OBJ_MAX; i++ ){
    if( awk->cap[i] != NULL ){
      CATS_ObjectUpdateCap( awk->cap[i] );
    }
  }
*/
}

//--------------------------------------------------------------------------------------------
/**
 * ＯＢＪ転送
 *
 * @param buf   転送データ
 * @param adrs  転送先アドレス
 * @param siz   転送サイズ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ObjVramTrans( void * buf, u32 adrs, u32 siz )
{
  DC_FlushRange( buf, siz );
  GX_LoadOBJ( buf, adrs, siz );
}

//--------------------------------------------------------------------------------------------
/**
 * ＯＢＪパレット転送
 *
 * @param buf   転送データ
 * @param adrs  転送先アドレス
 * @param siz   転送サイズ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ObjPlttVramTrans( void * buf, u32 adrs, u32 siz )
{
  DC_FlushRange( buf, siz );
  GX_LoadOBJPltt( buf, adrs, siz );
}


//--------------------------------------------------------------------------------------------
/**
 * 連れ歩きポケモンのアーカイブインデックスを返す
 *
 * @param inMonsNo  モンスターナンバー
 * @param inFormNo  フォルムナンバー
 * @param inSex   性別
 *
 * @return  アーカイブインデックス
 *
 *  pokethlon/poke_clact.cよりコピー
 */
//--------------------------------------------------------------------------------------------
static int GetMonsArcIdx( const int inMonsNo, const u16 inFormNo, const int inSex )
{
  int idx;
  if ( (inMonsNo<=0) || (MONSNO_END<inMonsNo) ){
    idx = 1;
  }else{
    idx = NARC_mmodel_tsure_001_nsbtx + PairPokeIdx_GetIdx(inMonsNo);
    //モンスターナンバーだけからだと不十分なので、ここからさらに調べる（フォルムとか）
    //性別による描き違いをして、かつ複数フォルムナンバー所持ポケモンはいないので、
    //性別違いをまずキーにして分岐
    if (PairPokeIdx_GetSexDif(inMonsNo)){   //性別による描き違いがある
      if(inSex == PARA_FEMALE ){ //メスの場合
        idx += 1; //ベースから1シフトしたところがメスのコード
      }
    }else{    //性別による描き違いがない
      //フォルムを調べる
      int form;
      if ( inFormNo > PairPokeIdx_GetFormNum(inMonsNo) ){
        form = 0;
      }else{
        form = inFormNo;
      }
      idx += form;
    }
  }

  return idx;
}

// 連れ歩きポケモンＯＢＪサイズ
#define POKEWALK_ONE_CELL_SIZE  ( 0x20 * 8 * 8 )

//--------------------------------------------------------------------------------------------
/**
 * 連れ歩きポケモンＯＢＪ切り替え
 *
 * @param wk    伝説デモワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void PokeWalkObjChange( LEGEND_DEMO_WORK * wk )
{
  LDA_WORK * awk;
  u16 sex, form;
  BOOL  rare;
  NNSG3dResTex * tex;
  void * file;
  u8 * raw;

  awk = wk->work;

  { // ポケモンデータ作成
    POKEPARTY * party = SaveData_GetTemotiPokemon( wk->dat->savedata );

    if( PokeParty_GetPokeCount( party ) == 0 ){
      sex  = PARA_UNK;
      form = 0;
      rare = FALSE;
    }else{
      POKEMON_PARAM * pp = PokeParty_GetMemberPointer( party, 0 );

      if( PokeParaGet( pp, ID_PARA_monsno, NULL ) == MONSNO_AUSU ){
        sex  = PokeParaGet( pp, ID_PARA_sex, NULL );
        form = PokeParaGet( pp, ID_PARA_form_no, NULL);
        rare = PokeRareGet( pp );
      }else{
        sex  = PARA_UNK;
        form = 0;
        rare = FALSE;
      }
    }
  }

  { // 連れ歩きポケモンのテクスチャを読み込んでくる
    // アーカイブインデックス取得
    u32 arc = GetMonsArcIdx( MONSNO_AUSU, form, sex );

    file = ArchiveDataLoadMalloc( ARC_MMODEL, arc, HEAPID_LEGEND_DEMO );
  }

  // テクスチャファイルを取得
  tex = NNS_G3dGetTex( file );

  { // キャラ書き換え
    NNSG2dImageProxy * ipc;
    void * buf;
    u32 cgx;
    u32 i;

    ipc = CLACT_ImageProxyGet( awk->cap[PRM_OBJ_ARCEUS]->act );
    cgx = NNS_G2dGetImageLocation( ipc, NNS_G2D_VRAM_TYPE_2DMAIN );

    // テクスチャロウデータ取得
    raw = (u8 *)tex + tex->texInfo.ofsTex;

    buf = sys_AllocMemory( HEAPID_LEGEND_DEMO, POKEWALK_ONE_CELL_SIZE );

    for( i=0; i<8; i++ ){
      // キャラデータ用のバッファにテクスチャのロウデータを1D変換して格納
      ChangesInto_RawData_1D_from_2D( &raw[POKEWALK_ONE_CELL_SIZE*i], 8, 0, 0, 8, 8, buf );
      ObjVramTrans( buf, cgx+(POKEWALK_ONE_CELL_SIZE*i), POKEWALK_ONE_CELL_SIZE );
    }

    sys_FreeMemoryEz( buf );
  }

  { // パレット書き換え
    NNSG2dImagePaletteProxy * ipp;
    u32 pal;

    ipp = CLACT_PaletteProxyGet( awk->cap[PRM_OBJ_ARCEUS]->act );
    pal = NNS_G2dGetImagePaletteLocation( ipp, NNS_G2D_VRAM_TYPE_2DMAIN );

    // パレットロウデータ取得
    raw = (u8 *)tex + tex->plttInfo.ofsPlttData;
    // レアかどうかでオフセット変更
    if( rare == TRUE ){
      raw = &raw[POKEWALK_PAL_SIZE];
    }
    ObjPlttVramTrans( raw, pal, POKEWALK_PAL_SIZE );
  }

  sys_FreeMemoryEz( file );
}






#endif
