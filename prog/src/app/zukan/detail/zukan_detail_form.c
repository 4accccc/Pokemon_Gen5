//============================================================================
/**
 *  @file   zukan_detail_form.c
 *  @brief  図鑑詳細画面の姿画面
 *  @author Koji Kawada
 *  @data   2010.02.02
 *  @note   
 *  モジュール名：ZUKAN_DETAIL_FORM
 */
//============================================================================
#define DEBUG_KAWADA


// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/mcss.h"
#include "system/mcss_tool.h"
#include "ui/ui_easy_clwk.h"
#include "pokeicon/pokeicon.h"
#include "gamesystem/gamedata_def.h"
#include "savedata/zukan_savedata.h"
#include "poke_tool/poke_personal.h"
#include "system/bmp_winframe.h"
#include "print/printsys.h"
#include "print/gf_font.h"
#include "print/wordset.h"
#include "print/global_msg.h"
#include "sound/pm_sndsys.h"

#include "zukan_detail_def.h"
#include "zukan_detail_common.h"
#include "zukan_detail_graphic.h"
#include "zukan_detail_form.h"

// アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "message.naix"
#include "msg/msg_zkn.h"
#include "zukan_gra.naix"

// サウンド

// オーバーレイ
FS_EXTERN_OVERLAY(ui_common);


/*
表示ルール


1 フォルム違いがなくて、オスメスもないとき

ミュウのすがた
ミュウのすがた  (いろちがい)


2 フォルム違いがなくて、オスメス(オスとメスでグラフィックに
  違いがあろうとなかろうと、オスとメスを表示する)があるとき

オスのすがた
オスのすがた  (いろちがい)
メスのすがた
メスのすがた  (いろちがい)

注意！：オスメスどちらか片方したいないときもある。
例えば、オスしかいないときは

オスのすがた
オスのすがた  (いろちがい)

となる。


3 フォルム違いがあるとき(オスメスはあろうとなかろうと、無視する)

ノーマルフォルム
ノーマルフォルム  (いろちがい)
アタックフォルム
アタックフォルム  (いろちがい)
ディフェンスフォルム
ディフェンスフォルム  (いろちがい)
スピードフォルム
スピードフォルム  (いろちがい)


*/


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
// メインBGフレーム
#define BG_FRAME_M_POKE          (GFL_BG_FRAME0_M)
#define BG_FRAME_M_TEXT          (GFL_BG_FRAME2_M)
#define BG_FRAME_M_REAR          (GFL_BG_FRAME3_M)
// メインBGフレームのプライオリティ
#define BG_FRAME_PRI_M_POKE      (1)
#define BG_FRAME_PRI_M_TEXT      (2)
#define BG_FRAME_PRI_M_REAR      (3)

// メインBGパレット
// 本数
enum
{
  BG_PAL_NUM_M_TEXT              = 1,
  BG_PAL_NUM_M_REAR              = ZKNDTL_COMMON_REAR_BG_PAL_NUM,
};
// 位置
enum
{
  BG_PAL_POS_M_TEXT              = 0,
  BG_PAL_POS_M_REAR              = 1,
};

// メインOBJパレット
// 本数
enum
{
  OBJ_PAL_NUM_M_             = 0,
};
// 位置
enum
{
  OBJ_PAL_POS_M_             = 0,
};


// サブBGフレーム
#define BG_FRAME_S_TEXT           (GFL_BG_FRAME2_S)
#define BG_FRAME_S_PANEL          (GFL_BG_FRAME3_S)
#define BG_FRAME_S_REAR           (GFL_BG_FRAME0_S)
// サブBGフレームのプライオリティ
#define BG_FRAME_PRI_S_TEXT       (1)
#define BG_FRAME_PRI_S_PANEL      (2)
#define BG_FRAME_PRI_S_REAR       (3)

// サブBGパレット
enum
{
  BG_PAL_NUM_S_TEXT              = 1,
  BG_PAL_NUM_S_PANEL             = 1,
  BG_PAL_NUM_S_REAR              = ZKNDTL_COMMON_REAR_BG_PAL_NUM,
};
// 位置
enum
{
  BG_PAL_POS_S_TEXT              = 0,
  BG_PAL_POS_S_PANEL             = 1,
  BG_PAL_POS_S_REAR              = 2,
};

// サブOBJパレット
// 本数
enum
{
  OBJ_PAL_NUM_S_POKEICON         = 3,
};
// 位置
enum
{
  OBJ_PAL_POS_S_POKEICON         = 0,
};


// ProcMainのシーケンス
enum
{
  SEQ_START      = 0,
  SEQ_PREPARE_0,
  SEQ_PREPARE,
  SEQ_FADE_IN,
  SEQ_MAIN,
  SEQ_POST,
  SEQ_FADE_OUT,
  SEQ_END,
};

// 終了命令
typedef enum
{
  END_CMD_NONE,
  END_CMD_INSIDE,
  END_CMD_OUTSIDE,
}
END_CMD;

// 状態
typedef enum
{
  STATE_TOP,
//  STATE_TOP_TO_EXCHANGE,
  STATE_EXCHANGE,
//  STATE_EXCHANGE_TO_TOP,
}
STATE;

// MCSSポケモン
typedef enum
{
  POKE_CURR_F,    // 今のフォルム(前向き)
  POKE_CURR_B,    // 今のフォルム(後向き)
  POKE_COMP_F,    // 比較フォルム(前向き)
  POKE_COMP_B,    // 比較フォルム(後向き)
  POKE_MAX,
}
POKE_INDEX;

// ポケモンの大きさ
#define POKE_SCALE    (16.0f)
#define POKE_SIZE_MAX (96.0f)

// ポケモンの位置
typedef enum
{
  POKE_CURR_POS_CENTER,  // 今のフォルムのポケモンの位置
  POKE_CURR_POS_LEFT, 
  POKE_COMP_RPOS,        // 比較フォルムのポケモンの位置(POKE_CURR_POS_???を基準とした相対位置)
  POKE_POS_MAX,
}
POKE_POS_INDEX;

static VecFx32 poke_pos[POKE_POS_MAX] =
{
  { FX_F32_TO_FX32(  0.0f), FX_F32_TO_FX32(-24.0f), FX_F32_TO_FX32(0.0f) },
  { FX_F32_TO_FX32(-64.0f), FX_F32_TO_FX32(-24.0f), FX_F32_TO_FX32(0.0f) },
  { FX_F32_TO_FX32(128.0f), FX_F32_TO_FX32(  0.0f), FX_F32_TO_FX32(0.0f) },
};

// ポケアイコンの位置
static const u8 pokeicon_pos[2] = { 128, 128 };

// 違う姿
typedef enum
{
  LOOKS_DIFF_ONE,   // 性別なし、フォルムなし
  LOOKS_DIFF_SEX,   // 性別あり、フォルムなし
  LOOKS_DIFF_FORM,  // 性別不問、フォルムあり
}
LOOKS_DIFF;

typedef enum
{
  COLOR_DIFF_NONE,     // 色違いなし
  COLOR_DIFF_NORMAL,   // 色違いありで通常色
  COLOR_DIFF_SPECIAL,  // 色違いありで特別色
}
COLOR_DIFF;

enum
{
  SEX_MALE,
  SEX_FEMALE,
};

// 違う姿の数
#define DIFF_MAX (64)  // これで足りるか？

// テキスト
enum
{
  TEXT_POKENAME,
  TEXT_UP_LABEL,
  TEXT_UP_NUM,
  TEXT_POKE_CURR,
  TEXT_POKE_COMP,
  TEXT_MAX,
};
// 文字数
#define STRBUF_NUM_LENGTH       (  8 )  // ??  // 例：見つけた姿の数  99  ←この例の99の文字数
#define STRBUF_POKENAME_LENGTH  ( 64 )  // [ポケモン種族名]のすがた  // buflen.hで確認する必要あり！

#define TEXT_POKE_POS_Y_1_LINE   (  9 )
#define TEXT_POKE_POS_Y_2_LINE_0 (  1 )
#define TEXT_POKE_POS_Y_2_LINE_1 ( 17 )


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
// 違う姿
typedef struct
{
  LOOKS_DIFF looks_diff;
  COLOR_DIFF color_diff;
  u16        other_diff;  // LOOKS_DIFF_ONEのとき、使用しない
                          // LOOKS_DIFF_SEXのとき、SEX_MALE or SEX_FEMALE
                          // LOOKS_DIFF_FORMのとき、formno
}
DIFF_INFO;


//-------------------------------------
/// PROC ワーク
//=====================================
typedef struct
{
  // 他のところから借用するもの
  GFL_CLUNIT*                 clunit;
  GFL_FONT*                   font;
  PRINT_QUE*                  print_que;

  // ここで作成するもの
  MCSS_SYS_WORK*              mcss_sys_wk;
  MCSS_WORK*                  poke_wk[POKE_MAX];  // NULLのときなし

  UI_EASY_CLWK_RES            pokeicon_res;   // pokeicon_clwkがNULLのとき、使用していない
  GFL_CLWK*                   pokeicon_clwk;  // NULLのときなし

  DIFF_INFO                   diff_info_list[DIFF_MAX];  // ここには今のフォルムも含んでいる
  u16                         diff_num;  // 最大DIFF_MAX
  u16                         diff_sugata_num;  // diff_num = diff_sugata_num + diff_irochigai_num
  u16                         diff_irochigai_num;
  u16                         diff_curr_no;  // diff_info_list[diff_curr_no]が今のフォルム
  u16                         diff_comp_no;  // diff_info_list[diff_comp_no]が比較フォルム

  GFL_BMPWIN*                 text_bmpwin[TEXT_MAX];
  GFL_MSGDATA*                text_msgdata;

  GFL_ARCUTIL_TRANSINFO       panel_s_tinfo;

  ZKNDTL_COMMON_REAR_WORK*    rear_wk_m;
  ZKNDTL_COMMON_REAR_WORK*    rear_wk_s;

  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;

  // フェード
  ZKNDTL_COMMON_FADE_WORK*    fade_wk_m;
  ZKNDTL_COMMON_FADE_WORK*    fade_wk_s;

  // 終了命令
  END_CMD                     end_cmd;
  
  // 状態
  STATE                       state;
}
ZUKAN_DETAIL_FORM_WORK;


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void Zukan_Detail_Form_VBlankFunc( GFL_TCB* tcb, void* wk );

// テキスト
static void Zukan_Detail_Form_CreateTextBase( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_DeleteTextBase( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_WriteUpText( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_WritePokeText( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                GFL_BMPWIN* bmpwin, u16 diff_no );
static void Zukan_Detail_Form_WritePokeCurrText( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_WritePokeCompText( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_ScrollPokeText( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// MCSSポケモン
static void Zukan_Detail_Form_McssSysInit( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_McssSysExit( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static MCSS_WORK* PokeInit( MCSS_SYS_WORK* mcss_sys_wk,
                      int mons_no, int form_no, int sex, int rare, BOOL egg, int dir );
static void PokeExit( MCSS_SYS_WORK* mcss_sys_wk, MCSS_WORK* poke_wk );
// マクロ
#define BLOCK_POKE_EXIT(sys,wk)                     \
    {                                               \
      if( wk ) PokeExit( sys, wk );                 \
      wk = NULL;                                    \
    }
// NULLを代入し忘れないようにマクロを使うようにしておく
static void PokeAdjustOfsPos( MCSS_WORK* poke_wk );
static void PokeGetCompareRelativePosition( MCSS_WORK* poke_wk, VecFx32* pos );

// ポケアイコン
static GFL_CLWK* PokeiconInit( UI_EASY_CLWK_RES* res, GFL_CLUNIT* clunit, HEAPID heap_id, u32 mons, u32 form_no, BOOL egg );
static void PokeiconExit( UI_EASY_CLWK_RES* res, GFL_CLWK* clwk );
// マクロ
#define BLOCK_POKEICON_EXIT(res,clwk)                     \
    {                                                     \
      if( clwk ) PokeiconExit( res, clwk );               \
      clwk = NULL;                                        \
    }
// NULLを代入し忘れないようにマクロを使うようにしておく

// 違う姿情報を取得する
static void Zukan_Detail_Form_GetDiffInfo( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// 違う姿情報からMCSSポケモンを生成する
static void Zukan_Detail_Form_PokeInitFromDiffInfo( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                POKE_INDEX poke_f, POKE_INDEX poke_b, POKE_POS_INDEX pos, u16 diff_no );
// 違う姿情報からポケアイコンを生成する
static void Zukan_Detail_Form_PokeiconInitFromDiffInfo( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                u16 diff_no );
// 違う姿情報からポケモンの姿の説明STRBUFを得る
static void Zukan_Detail_Form_GetPokeTextStrbufFromDiffInfo( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                STRBUF** a_strbuf0, STRBUF** a_strbuf1, u16 diff_no );

// ポケモン変更
static void Zukan_Detail_Form_ChangePoke( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
// 比較フォルム変更
static void Zukan_Detail_Form_ChangeCompareForm( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// ポケアイコン変更
static void Zukan_Detail_Form_ChangePokeicon( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// 入力
static void Zukan_Detail_Form_Input( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// スクロールバーを移動させる
static void Zukan_Detail_Form_MoveScrollBar( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// 状態を遷移させる
static void Zukan_Detail_Form_ChangeState( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                STATE state );


//=============================================================================
/**
*  PROC
*/
//=============================================================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Form_ProcInit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn );
static ZKNDTL_PROC_RESULT Zukan_Detail_Form_ProcExit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn );
static ZKNDTL_PROC_RESULT Zukan_Detail_Form_ProcMain( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_CommandFunc( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn, ZKNDTL_COMMAND cmd );
static void Zukan_Detail_Form_Draw3DFunc( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn );

const ZKNDTL_PROC_DATA    ZUKAN_DETAIL_FORM_ProcData =
{
  Zukan_Detail_Form_ProcInit,
  Zukan_Detail_Form_ProcMain,
  Zukan_Detail_Form_ProcExit,
  Zukan_Detail_Form_CommandFunc,
  Zukan_Detail_Form_Draw3DFunc,
};


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
//------------------------------------------------------------------
/**
 *  @brief           
 *
 *  @param[in,out]   
 *
 *  @retval          
 */
//------------------------------------------------------------------
void             ZUKAN_DETAIL_FORM_InitParam(
                            ZUKAN_DETAIL_FORM_PARAM*  param,
                            HEAPID                    heap_id )
{
  param->heap_id = heap_id;
}


//=============================================================================
/**
*  ローカル関数定義(PROC)
*/
//=============================================================================
//-------------------------------------
/// PROC 初期化処理
//=====================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Form_ProcInit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_FORM_PARAM*    param    = (ZUKAN_DETAIL_FORM_PARAM*)pwk;
  ZUKAN_DETAIL_FORM_WORK*     work;

#ifdef DEBUG_KAWADA
  {
    OS_Printf( "Zukan_Detail_Form_ProcInit\n" );
  }
#endif

  // オーバーレイ
  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common) );

  // ヒープ
  {
    work = ZKNDTL_PROC_AllocWork(proc, sizeof(ZUKAN_DETAIL_FORM_WORK), param->heap_id);
    GFL_STD_MemClear( work, sizeof(ZUKAN_DETAIL_FORM_WORK) ); 
  }

  // 他のところから借用するもの
  {
    ZUKAN_DETAIL_GRAPHIC_WORK* graphic = ZKNDTL_COMMON_GetGraphic(cmn);
    work->clunit      = ZUKAN_DETAIL_GRAPHIC_GetClunit(graphic);
    work->font        = ZKNDTL_COMMON_GetFont(cmn);
    work->print_que   = ZKNDTL_COMMON_GetPrintQue(cmn);
  }

  // NULL初期化
  {
    u8 i;
    for( i=0; i<POKE_MAX; i++ )
    {
      work->poke_wk[i] = NULL;
    }

    work->pokeicon_clwk = NULL;
  }

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( Zukan_Detail_Form_VBlankFunc, work, 1 );

  // フェード
  {
    work->fade_wk_m = ZKNDTL_COMMON_FadeInit( param->heap_id );
    work->fade_wk_s = ZKNDTL_COMMON_FadeInit( param->heap_id );
   
    ZKNDTL_COMMON_FadeSetPlaneDefault( work->fade_wk_m );
    ZKNDTL_COMMON_FadeSetPlaneDefault( work->fade_wk_s );

    ZKNDTL_COMMON_FadeSetBlackImmediately( ZKNDTL_COMMON_FADE_DISP_M, work->fade_wk_m );
    ZKNDTL_COMMON_FadeSetBlackImmediately( ZKNDTL_COMMON_FADE_DISP_S, work->fade_wk_s );
  }

  // 終了情報
  work->end_cmd = END_CMD_NONE;

  // 状態
  work->state = STATE_TOP;

  return ZKNDTL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 終了処理
//=====================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Form_ProcExit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_FORM_PARAM*    param    = (ZUKAN_DETAIL_FORM_PARAM*)pwk;
  ZUKAN_DETAIL_FORM_WORK*     work     = (ZUKAN_DETAIL_FORM_WORK*)mywk;

  // 最背面
  ZKNDTL_COMMON_RearExit( work->rear_wk_s );
  ZKNDTL_COMMON_RearExit( work->rear_wk_m );

  // BGパネル 
  ZKNDTL_COMMON_PanelDelete(
           BG_FRAME_S_PANEL,
           work->panel_s_tinfo );

  // ポケアイコン
  BLOCK_POKEICON_EXIT( &work->pokeicon_res, work->pokeicon_clwk )

  // テキスト
  Zukan_Detail_Form_DeleteTextBase( param, work, cmn );

  // フェード
  {
    ZKNDTL_COMMON_FadeExit( work->fade_wk_s );
    ZKNDTL_COMMON_FadeExit( work->fade_wk_m );
  }

  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // ヒープ
  ZKNDTL_PROC_FreeWork( proc );

  // オーバーレイ
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common) );

#ifdef DEBUG_KAWADA
  {
    OS_Printf( "Zukan_Detail_Form_ProcExit\n" );
  }
#endif

  return ZKNDTL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 主処理
//=====================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Form_ProcMain( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_FORM_PARAM*    param    = (ZUKAN_DETAIL_FORM_PARAM*)pwk;
  ZUKAN_DETAIL_FORM_WORK*     work     = (ZUKAN_DETAIL_FORM_WORK*)mywk;

  ZUKAN_DETAIL_GRAPHIC_WORK*   graphic  = ZKNDTL_COMMON_GetGraphic(cmn);

  ZUKAN_DETAIL_TOUCHBAR_WORK*  touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);
  ZUKAN_DETAIL_HEADBAR_WORK*   headbar  = ZKNDTL_COMMON_GetHeadbar(cmn);
 
  switch(*seq)
  {
  case SEQ_START:
    {
      *seq = SEQ_PREPARE_0;

      // 切り替え
      {
        // グラフィックスモード設定
        GX_SetGraphicsMode( GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_3D );
      }

      // BG
      {
        // クリア
        u8 i;
        for( i=GFL_BG_FRAME0_M; i<=GFL_BG_FRAME3_S; i++ )
        {
          if(    i != ZKNDTL_BG_FRAME_M_TOUCHBAR
              && i != ZKNDTL_BG_FRAME_S_HEADBAR )
          {
            GFL_BG_SetScroll( i, GFL_BG_SCROLL_X_SET, 0 );
            GFL_BG_SetScroll( i, GFL_BG_SCROLL_Y_SET, 0 );
            GFL_BG_ClearFrame(i);
          }
        }

        // メインBG
        GFL_BG_SetPriority( BG_FRAME_M_POKE,  BG_FRAME_PRI_M_POKE );
        GFL_BG_SetPriority( BG_FRAME_M_TEXT,  BG_FRAME_PRI_M_TEXT );
        GFL_BG_SetPriority( BG_FRAME_M_REAR,  BG_FRAME_PRI_M_REAR );
        
        // サブBG
        GFL_BG_SetPriority( BG_FRAME_S_TEXT,  BG_FRAME_PRI_S_TEXT  );
        GFL_BG_SetPriority( BG_FRAME_S_REAR,  BG_FRAME_PRI_S_REAR  );
        GFL_BG_SetPriority( BG_FRAME_S_PANEL, BG_FRAME_PRI_S_PANEL );
      }

      // 3D
      ZUKAN_DETAIL_GRAPHIC_Init3D( graphic, param->heap_id );

      // MCSSポケモン
      Zukan_Detail_Form_McssSysInit( param, work, cmn );

      // テキスト
      Zukan_Detail_Form_CreateTextBase( param, work, cmn );

      // 違う姿情報を取得する
      Zukan_Detail_Form_GetDiffInfo( param, work, cmn );
     
      // 最初のポケモンを表示する
      Zukan_Detail_Form_ChangePoke( param, work, cmn );

      // ポケモンを非表示にし黒にする
      MCSS_SetVanishFlag( work->poke_wk[POKE_CURR_F] );
      MCSS_SetPaletteFade( work->poke_wk[POKE_CURR_F], 16, 16, 0, 0x0000 );
      if( work->poke_wk[POKE_COMP_F] )
      {
        MCSS_SetVanishFlag( work->poke_wk[POKE_COMP_F] );
        MCSS_SetPaletteFade( work->poke_wk[POKE_COMP_F], 16, 16, 0, 0x0000 );
      }

      // BGパネル 
      work->panel_s_tinfo = ZKNDTL_COMMON_PanelCreate(
                              ZKNDTL_COMMON_PANEL_CREATE_TYPE_TRANS_PAL_CHARA,
                              param->heap_id,
                              BG_FRAME_S_PANEL,
                              BG_PAL_NUM_S_PANEL,
                              BG_PAL_POS_S_PANEL,
                              0,
                              ARCID_ZUKAN_GRA,
                              NARC_zukan_gra_info_info_bgu_NCLR,
                              NARC_zukan_gra_info_info_bgu_NCGR,
                              NARC_zukan_gra_info_formewin_bgu_NSCR,
                              0 );

      // 最背面
      work->rear_wk_m = ZKNDTL_COMMON_RearInit( param->heap_id, ZKNDTL_COMMON_REAR_TYPE_FORM,
          BG_FRAME_M_REAR, BG_PAL_POS_M_REAR +0, BG_PAL_POS_M_REAR +1 );
      work->rear_wk_s = ZKNDTL_COMMON_RearInit( param->heap_id, ZKNDTL_COMMON_REAR_TYPE_FORM,
          BG_FRAME_S_REAR, BG_PAL_POS_S_REAR +0, BG_PAL_POS_S_REAR +1 );
    }
    break;
  case SEQ_PREPARE_0:
    {
      // MCSSポケモン
      // ポケモンが黒になるのを待つために、1フレーム余計に待つ
      *seq = SEQ_PREPARE;
    }
    break;
  case SEQ_PREPARE:
    {
      // MCSSポケモン
      // ポケモンが黒になるのを待つ
      BOOL poke_fade = TRUE;
      if( MCSS_CheckExecutePaletteFade( work->poke_wk[POKE_CURR_F] ) )
      {
        poke_fade = FALSE;
      }
      if( work->poke_wk[POKE_COMP_F] )
      {
        if( MCSS_CheckExecutePaletteFade( work->poke_wk[POKE_COMP_F] ) )
        {
          poke_fade = FALSE;
        }
      }

      if( poke_fade )
      {
        *seq = SEQ_FADE_IN;

        // フェード
        ZKNDTL_COMMON_FadeSetBlackToColorless( work->fade_wk_m );
        ZKNDTL_COMMON_FadeSetBlackToColorless( work->fade_wk_s );

        // タッチバー
        if( ZUKAN_DETAIL_TOUCHBAR_GetState( touchbar ) != ZUKAN_DETAIL_TOUCHBAR_STATE_APPEAR )
        {
          ZUKAN_DETAIL_TOUCHBAR_SetType(
              touchbar,
              ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL,
              ZUKAN_DETAIL_TOUCHBAR_DISP_FORM );
          ZUKAN_DETAIL_TOUCHBAR_Appear( touchbar, ZUKAN_DETAIL_TOUCHBAR_SPEED_OUTSIDE );
        }
        // タイトルバー
        if( ZUKAN_DETAIL_HEADBAR_GetState( headbar ) != ZUKAN_DETAIL_HEADBAR_STATE_APPEAR )
        {
          ZUKAN_DETAIL_HEADBAR_SetType(
              headbar,
              ZUKAN_DETAIL_HEADBAR_TYPE_FORM );
          ZUKAN_DETAIL_HEADBAR_Appear( headbar );
        }

        // MCSSポケモン
        // ポケモンを表示しフェードイン(黒→カラー)
        MCSS_ResetVanishFlag( work->poke_wk[POKE_CURR_F] );
        MCSS_SetPaletteFade( work->poke_wk[POKE_CURR_F], 16, 0, ZKNDTL_COMMON_FADE_BRIGHT_WAIT, 0x0000 );
        if( work->poke_wk[POKE_COMP_F] )
        {
          MCSS_ResetVanishFlag( work->poke_wk[POKE_COMP_F] );
          MCSS_SetPaletteFade( work->poke_wk[POKE_COMP_F], 16, 0, ZKNDTL_COMMON_FADE_BRIGHT_WAIT, 0x0000 );
        }
      }
    }
    break;
  case SEQ_FADE_IN:
    {
      // MCSSポケモン
      // ポケモンがカラーになるのを待つ
      BOOL poke_fade = TRUE;
      if( MCSS_CheckExecutePaletteFade( work->poke_wk[POKE_CURR_F] ) )
      {
        poke_fade = FALSE;
      }
      if( work->poke_wk[POKE_COMP_F] )
      {
        if( MCSS_CheckExecutePaletteFade( work->poke_wk[POKE_COMP_F] ) )
        {
          poke_fade = FALSE;
        }
      }

      if(    (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_m ))
          && (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_s ))
          && ZUKAN_DETAIL_TOUCHBAR_GetState( touchbar ) == ZUKAN_DETAIL_TOUCHBAR_STATE_APPEAR
          && ZUKAN_DETAIL_HEADBAR_GetState( headbar ) == ZUKAN_DETAIL_HEADBAR_STATE_APPEAR
          && poke_fade )
      {
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );

        *seq = SEQ_MAIN;
      }
    }
    break;
  case SEQ_MAIN:
    {
      if( work->end_cmd != END_CMD_NONE )
      {
        *seq = SEQ_POST;

        // ポケモンをカラーにする
        {
          u8 i;
          for( i=0; i<POKE_MAX; i++ )
          {
            if( work->poke_wk[i] )
              MCSS_SetPaletteFade( work->poke_wk[i], 0, 0, 0, 0x0000 );
          }
        }
      }
      else
      {
        // 入力
        Zukan_Detail_Form_Input( param, work, cmn );
      }
    }
    break;
  case SEQ_POST:
    {
      // MCSSポケモン
      // ポケモンがカラーになるのを待つ
      BOOL poke_fade = TRUE;
      {
        u8 i;
        for( i=0; i<POKE_MAX; i++ )
        {
          if( work->poke_wk[i] )
          {
            if( MCSS_CheckExecutePaletteFade( work->poke_wk[i] ) )
            {
              poke_fade = FALSE;
              break;
            }
          }
        }
      }

      if( poke_fade )
      {
        *seq = SEQ_FADE_OUT;

        // フェード
        ZKNDTL_COMMON_FadeSetColorlessToBlack( work->fade_wk_m );
        ZKNDTL_COMMON_FadeSetColorlessToBlack( work->fade_wk_s );
        
        // タイトルバー
        ZUKAN_DETAIL_HEADBAR_Disappear( headbar );
        
        if( work->end_cmd == END_CMD_OUTSIDE )
        {
          // タッチバー
          ZUKAN_DETAIL_TOUCHBAR_Disappear( touchbar, ZUKAN_DETAIL_TOUCHBAR_SPEED_OUTSIDE );
        }

        // MCSSポケモン
        // ポケモンを表示しフェードアウト(カラー→黒)
        {
          u8 i;
          for( i=0; i<POKE_MAX; i++ )
          {
            if( work->poke_wk[i] )
            {
              MCSS_SetPaletteFade( work->poke_wk[i], 0, 16, ZKNDTL_COMMON_FADE_BRIGHT_WAIT, 0x0000 );
            }
          }
        }
      }
    }
    break;
  case SEQ_FADE_OUT:
    {
      // MCSSポケモン
      // ポケモンが黒になるのを待つ
      BOOL poke_fade = TRUE;
      {
        u8 i;
        for( i=0; i<POKE_MAX; i++ )
        {
          if( work->poke_wk[i] )
          {
            if( MCSS_CheckExecutePaletteFade( work->poke_wk[i] ) )
            {
              poke_fade = FALSE;
              break;
            }
          }
        }
      }

      if(    (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_m ))
          && (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_s ))
          && ZUKAN_DETAIL_HEADBAR_GetState( headbar ) == ZUKAN_DETAIL_HEADBAR_STATE_DISAPPEAR
          && poke_fade )
      {
        if( work->end_cmd == END_CMD_OUTSIDE )
        {
          if( ZUKAN_DETAIL_TOUCHBAR_GetState( touchbar ) == ZUKAN_DETAIL_TOUCHBAR_STATE_DISAPPEAR )
          {
            *seq = SEQ_END;
          }
        }
        else
        {
          *seq = SEQ_END;
        }
      }
    }
    break;
  case SEQ_END:
    {
      // MCSSポケモン
      {
        u8 i;
        for( i=0; i<POKE_MAX; i++ )
        {
          BLOCK_POKE_EXIT( work->mcss_sys_wk, work->poke_wk[i] )
        }
      }
      Zukan_Detail_Form_McssSysExit( param, work, cmn );

      // 3D
      ZUKAN_DETAIL_GRAPHIC_Exit3D( graphic );

      // 切り替え
      {
        // グラフィックスモード設定
        GX_SetGraphicsMode( GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D );
      }

      return ZKNDTL_PROC_RES_FINISH;
    }
    break;
  }

  // MCSSポケモン
  if( work->mcss_sys_wk )
  {
    MCSS_Main( work->mcss_sys_wk );
  } 

  // 最背面
  if( *seq >= SEQ_PREPARE )
  {
    ZKNDTL_COMMON_RearMain( work->rear_wk_m );
    ZKNDTL_COMMON_RearMain( work->rear_wk_s );
  }

  // フェード
  ZKNDTL_COMMON_FadeMain( work->fade_wk_m, work->fade_wk_s );

  return ZKNDTL_PROC_RES_CONTINUE;
}

//-------------------------------------
/// PROC 命令処理
//=====================================
static void Zukan_Detail_Form_CommandFunc( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn, ZKNDTL_COMMAND cmd )
{
  if( mywk )
  {
    ZUKAN_DETAIL_FORM_PARAM*    param    = (ZUKAN_DETAIL_FORM_PARAM*)pwk;
    ZUKAN_DETAIL_FORM_WORK*     work     = (ZUKAN_DETAIL_FORM_WORK*)mywk;

    ZUKAN_DETAIL_TOUCHBAR_WORK*  touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);
  
    switch( cmd )
    {
    case ZKNDTL_CMD_NONE:
      {
      }
      break;
    case ZKNDTL_CMD_CLOSE:
    case ZKNDTL_CMD_RETURN:
      {
        work->end_cmd = END_CMD_OUTSIDE;
      }
      break;
    case ZKNDTL_CMD_INFO:
    case ZKNDTL_CMD_MAP:
    case ZKNDTL_CMD_VOICE:
      {
        work->end_cmd = END_CMD_INSIDE;
      }
      break;
    case ZKNDTL_CMD_CUR_D:
    case ZKNDTL_CMD_FORM_CUR_D:
      {
        u16 monsno_curr;
        u16 monsno_go;
        monsno_curr = ZKNDTL_COMMON_GetCurrPoke(cmn);
        ZKNDTL_COMMON_GoToNextPoke(cmn);
        monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
        if( monsno_curr != monsno_go )
        {
          Zukan_Detail_Form_ChangePoke(param, work, cmn);
        }
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    case ZKNDTL_CMD_CUR_U:
    case ZKNDTL_CMD_FORM_CUR_U:
      {
        u16 monsno_curr;
        u16 monsno_go;
        monsno_curr = ZKNDTL_COMMON_GetCurrPoke(cmn);
        ZKNDTL_COMMON_GoToPrevPoke(cmn);
        monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
        if( monsno_curr != monsno_go )
        {
          Zukan_Detail_Form_ChangePoke(param, work, cmn);
        }
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    case ZKNDTL_CMD_CHECK:
      {
        GAMEDATA* gamedata = ZKNDTL_COMMON_GetGamedata(cmn);
        ZUKAN_SAVEDATA* zukan_savedata = GAMEDATA_GetZukanSave( gamedata );
        u16 monsno_curr;
        monsno_curr = ZKNDTL_COMMON_GetCurrPoke(cmn);
        ZUKANSAVE_SetShortcutMons( zukan_savedata, monsno_curr );
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    case ZKNDTL_CMD_FORM_CUR_R:
      {
        if( work->diff_num >= 3 )
        {
          work->diff_comp_no++;
          work->diff_comp_no %= work->diff_num;
          if( work->diff_curr_no == work->diff_comp_no )
          {
            work->diff_comp_no++;
            work->diff_comp_no %= work->diff_num;
          }

          Zukan_Detail_Form_ChangeCompareForm( param, work, cmn );
          Zukan_Detail_Form_MoveScrollBar( param, work, cmn );
        }
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    case ZKNDTL_CMD_FORM_RETURN:
      {
        Zukan_Detail_Form_ChangeState( param, work, cmn, STATE_TOP );
      }
      break;
    case ZKNDTL_CMD_FORM_CUR_L:
      {
        if( work->diff_num >= 3 )
        {
          work->diff_comp_no += work->diff_num -1;
          work->diff_comp_no %= work->diff_num;
          if( work->diff_curr_no == work->diff_comp_no )
          {
            work->diff_comp_no += work->diff_num -1;
            work->diff_comp_no %= work->diff_num;
          }

          Zukan_Detail_Form_ChangeCompareForm( param, work, cmn );
          Zukan_Detail_Form_MoveScrollBar( param, work, cmn );
        }
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    case ZKNDTL_CMD_FORM_EXCHANGE:
      {
        if( work->diff_num >= 2 )
        {
          u16 no;
          MCSS_WORK* mw;

          // 位置入れ替え
          MCSS_SetPosition( work->poke_wk[POKE_COMP_F], &poke_pos[POKE_CURR_POS_LEFT] );
          MCSS_SetPosition( work->poke_wk[POKE_COMP_B], &poke_pos[POKE_CURR_POS_LEFT] );
          {
            VecFx32 p;
            PokeGetCompareRelativePosition( work->poke_wk[POKE_COMP_F], &p );
            
            MCSS_SetPosition( work->poke_wk[POKE_CURR_F], &p );
            MCSS_SetPosition( work->poke_wk[POKE_CURR_B], &p );
          }
          
          // 番号、ポインタ入れ替え
          no = work->diff_curr_no;
          work->diff_curr_no = work->diff_comp_no;
          work->diff_comp_no = no;

          mw = work->poke_wk[POKE_CURR_F];
          work->poke_wk[POKE_CURR_F] = work->poke_wk[POKE_COMP_F];
          work->poke_wk[POKE_COMP_F] = mw;

          mw = work->poke_wk[POKE_CURR_B];
          work->poke_wk[POKE_CURR_B] = work->poke_wk[POKE_COMP_B];
          work->poke_wk[POKE_COMP_B] = mw;

          // ポケアイコン変更
          Zukan_Detail_Form_ChangePokeicon( param, work, cmn );

          // テキスト
          // テキストはOBJではなくBGなので、入れ替えできないので丸々書き直し
          Zukan_Detail_Form_WritePokeCurrText( param, work, cmn );
          Zukan_Detail_Form_WritePokeCompText( param, work, cmn );
        }
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    default:
      {
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    }
  }
}

//-------------------------------------
/// PROC 3D描画
//=====================================
static void Zukan_Detail_Form_Draw3DFunc( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn )
{
  if( mywk )
  {
    ZUKAN_DETAIL_FORM_PARAM*    param    = (ZUKAN_DETAIL_FORM_PARAM*)pwk;
    ZUKAN_DETAIL_FORM_WORK*     work     = (ZUKAN_DETAIL_FORM_WORK*)mywk;

    if( work->mcss_sys_wk )
      MCSS_Draw( work->mcss_sys_wk );
  }
}


//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// VBlank関数
//=====================================
static void Zukan_Detail_Form_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  ZUKAN_DETAIL_FORM_WORK*     work     = (ZUKAN_DETAIL_FORM_WORK*)wk;
}

//-------------------------------------
/// 違う姿情報からポケモンの姿の説明STRBUFを得る
    // 呼び出し元で解放すること(NULLのときはなし)
//=====================================
static void Zukan_Detail_Form_GetPokeTextStrbufFromDiffInfo( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                STRBUF** a_strbuf0, STRBUF** a_strbuf1, u16 diff_no )
{
  STRBUF* strbuf0 = NULL;
  STRBUF* strbuf1 = NULL;
  
  u16 monsno = ZKNDTL_COMMON_GetCurrPoke(cmn);

  switch( work->diff_info_list[diff_no].looks_diff )
  {
  case LOOKS_DIFF_ONE:
    {
      WORDSET* wordset;
      STRBUF* src_strbuf;
      wordset = WORDSET_Create( param->heap_id );
      src_strbuf = GFL_MSG_CreateString( work->text_msgdata, ZKN_FORMNAME_10 );
      strbuf0 = GFL_STR_CreateBuffer( STRBUF_POKENAME_LENGTH, param->heap_id );
      WORDSET_RegisterPokeMonsNameNo( wordset, 0, monsno );
      WORDSET_ExpandStr( wordset, strbuf0, src_strbuf );
      GFL_STR_DeleteBuffer( src_strbuf );
      WORDSET_Delete( wordset );
    }
    break;
  case LOOKS_DIFF_SEX:
    {
      switch( work->diff_info_list[diff_no].other_diff )
      {
      case SEX_MALE:
        {
          strbuf0 = GFL_MSG_CreateString( work->text_msgdata, ZNK_ZUKAN_GRAPH_00 );
        }
        break;
      case SEX_FEMALE:
        {
          strbuf0 = GFL_MSG_CreateString( work->text_msgdata, ZNK_ZUKAN_GRAPH_01 );
        }
        break;
      }
    }
    break;
  case LOOKS_DIFF_FORM:
    {
      u16 form_no = work->diff_info_list[diff_no].other_diff;

      strbuf0 = GFL_MSG_CreateString( work->text_msgdata, ZNK_ZUKAN_GRAPH_07 );
    }
    break;
  }

  switch( work->diff_info_list[diff_no].color_diff )
  {
  case COLOR_DIFF_NONE:
    {
      // 何もしない
    }
    break;
  case COLOR_DIFF_NORMAL:
    {
      // 何もしない
    }
    break;
  case COLOR_DIFF_SPECIAL:
    {
      strbuf1 = GFL_MSG_CreateString( work->text_msgdata, ZKN_FORME_TEXT_05 );
    }
    break;
  }

  *a_strbuf0 = strbuf0;
  *a_strbuf1 = strbuf1;
}

//-------------------------------------
/// テキスト
//=====================================
static void Zukan_Detail_Form_CreateTextBase( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u8 bmpwin_setup[TEXT_MAX][7] =
  {
    { BG_FRAME_S_TEXT, 19, 16,  8,  2, BG_PAL_POS_S_TEXT, GFL_BMP_CHRAREA_GET_B },
    { BG_FRAME_S_TEXT,  3, 19, 23,  4, BG_PAL_POS_S_TEXT, GFL_BMP_CHRAREA_GET_B },
    { BG_FRAME_S_TEXT, 27, 19,  2,  4, BG_PAL_POS_S_TEXT, GFL_BMP_CHRAREA_GET_B },
    { BG_FRAME_M_TEXT,  0,  1, 16,  4, BG_PAL_POS_M_TEXT, GFL_BMP_CHRAREA_GET_B },
    { BG_FRAME_M_TEXT, 16,  1, 16,  4, BG_PAL_POS_M_TEXT, GFL_BMP_CHRAREA_GET_B },
  };

  // パレット
  GFL_ARC_UTIL_TransVramPaletteEx( ARCID_FONT, NARC_font_default_nclr,
       PALTYPE_SUB_BG,
       0,
       BG_PAL_POS_S_TEXT * 0x20,
       BG_PAL_NUM_S_TEXT * 0x20,
       param->heap_id );

  GFL_ARC_UTIL_TransVramPaletteEx( ARCID_FONT, NARC_font_default_nclr,
       PALTYPE_MAIN_BG,
       0,
       BG_PAL_POS_S_TEXT * 0x20,
       BG_PAL_NUM_S_TEXT * 0x20,
       param->heap_id );

  // ビットマップウィンドウ
  {
    u8 i;
    for( i=0; i<TEXT_MAX; i++ )
    {
      work->text_bmpwin[i] = GFL_BMPWIN_Create( bmpwin_setup[i][0],
                                         bmpwin_setup[i][1], bmpwin_setup[i][2], bmpwin_setup[i][3], bmpwin_setup[i][4],
                                         bmpwin_setup[i][5],
                                         bmpwin_setup[i][6] );

      // クリア
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->text_bmpwin[i] ), 0 );
      GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[i] );
    }
  }

  // メッセージ
  work->text_msgdata = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL,
                           ARCID_MESSAGE, NARC_message_zkn_dat,
                           param->heap_id );
}
static void Zukan_Detail_Form_DeleteTextBase( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u8 i;
  for( i=0; i<TEXT_MAX; i++ )
  {
    GFL_BMPWIN_Delete( work->text_bmpwin[i] );
  }

  GFL_MSG_Delete( work->text_msgdata );
}
static void Zukan_Detail_Form_WriteUpText( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u8 i;

  // 前回のテキストをクリア
  {
    for( i=TEXT_POKENAME; i<=TEXT_UP_NUM; i++ )
    {
      // クリア
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->text_bmpwin[i] ), 0 );
      GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[i] );
    }
  }

  // 今回のテキストを描画
  {
    u16 monsno = ZKNDTL_COMMON_GetCurrPoke(cmn);
    u16 label_num_y[2] = { 1, 17 };
    
    {
      GFL_BMP_DATA*	bmp_data = GFL_BMPWIN_GetBmp( work->text_bmpwin[TEXT_POKENAME] );
      
      STRBUF* strbuf = GFL_MSG_CreateString( GlobalMsg_PokeName, monsno );

      u16 str_width = (u16)( PRINTSYS_GetStrWidth( strbuf, work->font, 0 ) );
      u16 bmp_width = GFL_BMP_GetSizeX( bmp_data );
      u16 x = ( bmp_width - str_width ) / 2;

/*
      u16 str_height = (u16)( PRINTSYS_GetStrHeight( strbuf, work->font ) );
      u16 bmp_height = GFL_BMP_GetSizeY( bmp_data );
      u16 y = ( bmp_height - str_height ) / 2;
*/
      u16 y = 1;  // 計算で出した位置は中心より上に見えるので

      PRINTSYS_PrintQueColor( work->print_que, bmp_data,
                              x, y, strbuf, work->font, PRINTSYS_LSB_Make(15,2,0) );

      GFL_STR_DeleteBuffer( strbuf );
    }

    {
      STRBUF* strbuf;
      GFL_BMP_DATA*	bmp_data = GFL_BMPWIN_GetBmp( work->text_bmpwin[TEXT_UP_LABEL] );
      
      strbuf = GFL_MSG_CreateString( work->text_msgdata, ZKN_FORME_TEXT_01 );
      PRINTSYS_PrintQueColor( work->print_que, bmp_data,
                              0, label_num_y[0], strbuf, work->font, PRINTSYS_LSB_Make(15,2,0) );
      GFL_STR_DeleteBuffer( strbuf );

      strbuf = GFL_MSG_CreateString( work->text_msgdata, ZKN_FORME_TEXT_02 );
      PRINTSYS_PrintQueColor( work->print_que, bmp_data,
                              0, label_num_y[1], strbuf, work->font, PRINTSYS_LSB_Make(15,2,0) );
      GFL_STR_DeleteBuffer( strbuf );
    }

    {
      WORDSET* wordset;
      STRBUF* src_strbuf;
      STRBUF* strbuf;
      GFL_BMP_DATA*	bmp_data = GFL_BMPWIN_GetBmp( work->text_bmpwin[TEXT_UP_NUM] );

      wordset = WORDSET_Create( param->heap_id );
      src_strbuf = GFL_MSG_CreateString( work->text_msgdata, ZKN_FORME_TEXT_03 );
      strbuf = GFL_STR_CreateBuffer( STRBUF_NUM_LENGTH, param->heap_id );
      WORDSET_RegisterNumber( wordset, 0, work->diff_sugata_num, 2, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
      WORDSET_ExpandStr( wordset, strbuf, src_strbuf );
      PRINTSYS_PrintQueColor( work->print_que, bmp_data,
                              0, label_num_y[0], strbuf, work->font, PRINTSYS_LSB_Make(15,2,0) );
      GFL_STR_DeleteBuffer( strbuf );
      GFL_STR_DeleteBuffer( src_strbuf );
      WORDSET_Delete( wordset );

      wordset = WORDSET_Create( param->heap_id );
      src_strbuf = GFL_MSG_CreateString( work->text_msgdata, ZKN_FORME_TEXT_04 );
      strbuf = GFL_STR_CreateBuffer( STRBUF_NUM_LENGTH, param->heap_id );
      WORDSET_RegisterNumber( wordset, 0, work->diff_irochigai_num, 2, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
      WORDSET_ExpandStr( wordset, strbuf, src_strbuf );
      PRINTSYS_PrintQueColor( work->print_que, bmp_data,
                              0, label_num_y[1], strbuf, work->font, PRINTSYS_LSB_Make(15,2,0) );
      GFL_STR_DeleteBuffer( strbuf );
      GFL_STR_DeleteBuffer( src_strbuf );
      WORDSET_Delete( wordset );
    }

    for( i=TEXT_POKENAME; i<=TEXT_UP_NUM; i++ )
    {
      // スクリーン転送
      GFL_BMPWIN_MakeTransWindow_VBlank( work->text_bmpwin[i] );
    }
  }
}

static void Zukan_Detail_Form_WritePokeText( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                GFL_BMPWIN* bmpwin, u16 diff_no )
{
  GFL_BMP_DATA*	bmp_data = GFL_BMPWIN_GetBmp( bmpwin );
  u16 bmp_width = GFL_BMP_GetSizeX( bmp_data );

  // 今回のテキストを描画
  {
    STRBUF* strbuf0;
    STRBUF* strbuf1;

    u16 str_width;
    u16 x;
    u16 y[2];

    Zukan_Detail_Form_GetPokeTextStrbufFromDiffInfo( param, work, cmn,
         &strbuf0, &strbuf1, diff_no );

    if( strbuf1 )
    {
      y[0] = TEXT_POKE_POS_Y_2_LINE_0;
      y[1] = TEXT_POKE_POS_Y_2_LINE_1;
    }
    else
    {
      y[0] = TEXT_POKE_POS_Y_1_LINE;
    }

    str_width = (u16)( PRINTSYS_GetStrWidth( strbuf0, work->font, 0 ) );
    x = ( bmp_width - str_width ) / 2;
    PRINTSYS_PrintQueColor( work->print_que, bmp_data,
                            x, y[0], strbuf0, work->font, PRINTSYS_LSB_Make(15,2,0) );

    if( strbuf1 )
    {
      str_width = (u16)( PRINTSYS_GetStrWidth( strbuf1, work->font, 0 ) );
      x = ( bmp_width - str_width ) / 2;
      PRINTSYS_PrintQueColor( work->print_que, bmp_data,
                              x, y[1], strbuf1, work->font, PRINTSYS_LSB_Make(15,2,0) );
    }

    if( strbuf0 ) GFL_STR_DeleteBuffer( strbuf0 );
    if( strbuf1 ) GFL_STR_DeleteBuffer( strbuf1 );

    // スクリーン転送
    GFL_BMPWIN_MakeTransWindow_VBlank( bmpwin );
  }
}
static void Zukan_Detail_Form_WritePokeCurrText( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 前回のテキストをクリア
  {
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->text_bmpwin[TEXT_POKE_CURR] ), 0 );
    GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[TEXT_POKE_CURR] );
  }

  // 今回のテキストを描画
  Zukan_Detail_Form_WritePokeText( param, work, cmn,
               work->text_bmpwin[TEXT_POKE_CURR], work->diff_curr_no );
 }
static void Zukan_Detail_Form_WritePokeCompText( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 前回のテキストをクリア
  {
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->text_bmpwin[TEXT_POKE_COMP] ), 0 );
    GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[TEXT_POKE_COMP] );
  }

  // 今回のテキストを描画
  switch(work->state)
  {
  case STATE_TOP:
    {
      // 描画しない
    }
    break;
  case STATE_EXCHANGE:
    {
      if( work->diff_num >= 2 )
      {
        Zukan_Detail_Form_WritePokeText( param, work, cmn,
               work->text_bmpwin[TEXT_POKE_COMP], work->diff_comp_no );
      }
      // 2姿以上なければ、描画しない
    }
    break;
  }
}
static void Zukan_Detail_Form_ScrollPokeText( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  switch(work->state)
  {
  case STATE_TOP:
    {
      GFL_BG_SetScroll( BG_FRAME_M_TEXT, GFL_BG_SCROLL_X_SET, -64 );
    }
    break;
  case STATE_EXCHANGE:
    {
      if( work->diff_num >= 2 )
      {
        GFL_BG_SetScroll( BG_FRAME_M_TEXT, GFL_BG_SCROLL_X_SET, 0 );
      }
      else
      {
        GFL_BG_SetScroll( BG_FRAME_M_TEXT, GFL_BG_SCROLL_X_SET, -64 );
      }
    }
    break;
  }
}

//-------------------------------------
/// MCSSポケモン
//=====================================
static void Zukan_Detail_Form_McssSysInit( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  work->mcss_sys_wk = MCSS_Init( POKE_MAX, param->heap_id );
  MCSS_SetTextureTransAdrs( work->mcss_sys_wk, 0 );  // 他に一緒に出す3Dはないので
  MCSS_SetOrthoMode( work->mcss_sys_wk );
}
static void Zukan_Detail_Form_McssSysExit( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  MCSS_Exit( work->mcss_sys_wk );
}

static MCSS_WORK* PokeInit( MCSS_SYS_WORK* mcss_sys_wk,
                      int mons_no, int form_no, int sex, int rare, BOOL egg, int dir )
{
  VecFx32 scale =
  {
    FX_F32_TO_FX32(POKE_SCALE),
    FX_F32_TO_FX32(POKE_SCALE),
    FX32_ONE, 
  };

  MCSS_ADD_WORK add_wk;
  MCSS_WORK*    poke_wk;

  MCSS_TOOL_MakeMAWParam( mons_no, form_no, sex, rare, egg, &add_wk, dir );
  poke_wk = MCSS_Add( mcss_sys_wk, 0, 0, 0, &add_wk );
  
  MCSS_SetAnmStopFlag( poke_wk );
  MCSS_SetScale( poke_wk, &scale );
  PokeAdjustOfsPos( poke_wk );

  return poke_wk;
}
static void PokeExit( MCSS_SYS_WORK* mcss_sys_wk, MCSS_WORK* poke_wk )
{
  MCSS_SetVanishFlag( poke_wk );
  MCSS_Del( mcss_sys_wk, poke_wk );
}
static void PokeAdjustOfsPos( MCSS_WORK* poke_wk )
{
  f32 size_y = (f32)MCSS_GetSizeY( poke_wk );
  f32 ofs_y;
  VecFx32 ofs;
  if( size_y > POKE_SIZE_MAX ) size_y = POKE_SIZE_MAX;
  ofs_y = ( POKE_SIZE_MAX - size_y ) / 2.0f;
  ofs.x = 0;  ofs.y = FX_F32_TO_FX32(ofs_y);  ofs.z = 0;
  MCSS_SetOfsPosition( poke_wk, &ofs );
}

static void PokeGetCompareRelativePosition( MCSS_WORK* poke_wk, VecFx32* pos )
{
  VecFx32 origin;
  MCSS_GetPosition( poke_wk, &origin );

  pos->x = origin.x + poke_pos[POKE_COMP_RPOS].x;
  pos->y = origin.y + poke_pos[POKE_COMP_RPOS].y;
  pos->z = origin.z + poke_pos[POKE_COMP_RPOS].z;
}

//-------------------------------------
/// ポケアイコン
//=====================================
static GFL_CLWK* PokeiconInit( UI_EASY_CLWK_RES* res, GFL_CLUNIT* clunit, HEAPID heap_id, u32 mons, u32 form_no, BOOL egg )
{
  GFL_CLWK* clwk;
  
  UI_EASY_CLWK_RES_PARAM prm;
  prm.draw_type    = CLSYS_DRAW_SUB;
  prm.comp_flg     = UI_EASY_CLWK_RES_COMP_NCLR;
  prm.arc_id       = ARCID_POKEICON;
  prm.pltt_id      = POKEICON_GetPalArcIndex();
  prm.ncg_id       = POKEICON_GetCgxArcIndexByMonsNumber( mons, form_no, egg );
  prm.cell_id      = POKEICON_GetCellArcIndex(); 
  prm.anm_id       = POKEICON_GetAnmArcIndex();
  prm.pltt_line    = OBJ_PAL_POS_S_POKEICON;
  prm.pltt_src_ofs = 0;
  prm.pltt_src_num = OBJ_PAL_NUM_S_POKEICON;
 
  UI_EASY_CLWK_LoadResource( res, &prm, clunit, heap_id );
  
  // アニメシーケンスで指定( 0=瀕死, 1=HP最大, 2=HP緑, 3=HP黄, 4=HP赤, 5=状態異常 )
  clwk = UI_EASY_CLWK_CreateCLWK( res, clunit, pokeicon_pos[0], pokeicon_pos[1], 1, heap_id );

  // 上にアイテムアイコンを描画するので優先度を下げておく
  GFL_CLACT_WK_SetSoftPri( clwk, 1 );

  // オートアニメ OFF
  GFL_CLACT_WK_SetAutoAnmFlag( clwk, FALSE );

  {
    u8 pal_num = POKEICON_GetPalNum( mons, form_no, egg );
    GFL_CLACT_WK_SetPlttOffs( clwk, pal_num, CLWK_PLTTOFFS_MODE_OAM_COLOR );
  }

  GFL_CLACT_WK_SetObjMode( clwk, GX_OAM_MODE_XLU );  // BGとともにこのOBJも暗くしたいので

  return clwk;
}
static void PokeiconExit( UI_EASY_CLWK_RES* res, GFL_CLWK* clwk )
{
  // CLWK破棄
  GFL_CLACT_WK_Remove( clwk );
  // リソース開放
  UI_EASY_CLWK_UnLoadResource( res );
}

//-------------------------------------
/// 違う姿情報を取得する
//=====================================
static void Zukan_Detail_Form_GetDiffInfo( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u16 monsno = ZKNDTL_COMMON_GetCurrPoke(cmn);

  GAMEDATA* gamedata = ZKNDTL_COMMON_GetGamedata(cmn);
  ZUKAN_SAVEDATA* zkn_sv = GAMEDATA_GetZukanSave( gamedata );

  // 今だけのダミーデータ作成
  {
    u16 diff_count = 0;
    
    POKEMON_PERSONAL_DATA* ppd = POKE_PERSONAL_OpenHandle( monsno, 0, param->heap_id );
    u16 form_max = (u16)POKE_PERSONAL_GetParam( ppd, POKEPER_ID_form_max );
    u32 sex      = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_sex );
    POKE_PERSONAL_CloseHandle( ppd );

    if( monsno == 151 )
    {
      work->diff_info_list[diff_count].looks_diff = LOOKS_DIFF_ONE;
      work->diff_info_list[diff_count].color_diff = COLOR_DIFF_NONE;
      diff_count++;
    }
    else
    {
      if( form_max <= 1 && sex != 0 )
      {
        work->diff_info_list[diff_count].looks_diff = LOOKS_DIFF_SEX;
        work->diff_info_list[diff_count].color_diff = COLOR_DIFF_NORMAL;
        work->diff_info_list[diff_count].other_diff = SEX_MALE;
        diff_count++;

        work->diff_info_list[diff_count].looks_diff = LOOKS_DIFF_SEX;
        work->diff_info_list[diff_count].color_diff = COLOR_DIFF_SPECIAL;
        work->diff_info_list[diff_count].other_diff = SEX_MALE;
        diff_count++;

        work->diff_info_list[diff_count].looks_diff = LOOKS_DIFF_SEX;
        work->diff_info_list[diff_count].color_diff = COLOR_DIFF_NORMAL;
        work->diff_info_list[diff_count].other_diff = SEX_FEMALE;
        diff_count++;

        work->diff_info_list[diff_count].looks_diff = LOOKS_DIFF_SEX;
        work->diff_info_list[diff_count].color_diff = COLOR_DIFF_SPECIAL;
        work->diff_info_list[diff_count].other_diff = SEX_FEMALE;
        diff_count++;
      }
      else if( form_max > 1 )
      {
        u16 i;
        for( i=0; i<form_max; i++ )
        {
          GF_ASSERT_MSG( diff_count < DIFF_MAX,  "ZUKAN_DETAIL_FORM : 違う姿の数が多いです。\n" );
  
          work->diff_info_list[diff_count].looks_diff = LOOKS_DIFF_FORM;
          work->diff_info_list[diff_count].color_diff = COLOR_DIFF_NORMAL;
          work->diff_info_list[diff_count].other_diff = i;
          diff_count++;

          work->diff_info_list[diff_count].looks_diff = LOOKS_DIFF_FORM;
          work->diff_info_list[diff_count].color_diff = COLOR_DIFF_SPECIAL;
          work->diff_info_list[diff_count].other_diff = i;
          diff_count++;
        }
      }
      else
      {
        work->diff_info_list[diff_count].looks_diff = LOOKS_DIFF_ONE;
        work->diff_info_list[diff_count].color_diff = COLOR_DIFF_NORMAL;
        diff_count++;

        work->diff_info_list[diff_count].looks_diff = LOOKS_DIFF_ONE;
        work->diff_info_list[diff_count].color_diff = COLOR_DIFF_SPECIAL;
        diff_count++;
      }
    }

    work->diff_num = diff_count;
    work->diff_curr_no = 0;
    work->diff_comp_no = 1 % diff_count;
  }

  // 姿の数、色違いの数
  {
    u8 i;

    work->diff_sugata_num = 0;
    work->diff_irochigai_num = 0;

    for( i=0; i<work->diff_num; i++ )
    {
      if( work->diff_info_list[i].color_diff == COLOR_DIFF_SPECIAL )
      {
        work->diff_irochigai_num++;
      }
    }

    work->diff_sugata_num = work->diff_num - work->diff_irochigai_num;
  }
}

//-------------------------------------
/// 違う姿情報からMCSSポケモンを生成する
//=====================================
static void Zukan_Detail_Form_PokeInitFromDiffInfo( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                POKE_INDEX poke_f, POKE_INDEX poke_b, POKE_POS_INDEX pos, u16 diff_no )
{
  int mons_no;
  int form_no;
  int sex;
  int rare;

  mons_no = ZKNDTL_COMMON_GetCurrPoke(cmn);

  switch( work->diff_info_list[diff_no].looks_diff )
  {
  case LOOKS_DIFF_ONE:
    {
      form_no = 0;
      sex = 0;
    }
    break;
  case LOOKS_DIFF_SEX:
    {
      form_no = 0;

      switch( work->diff_info_list[diff_no].other_diff )
      {
      case SEX_MALE:
        {
          sex = 0;
        }
        break;
      case SEX_FEMALE:
        {
          //sex = 1;
          sex = 0;
        }
        break;
      }
    }
    break;
  case LOOKS_DIFF_FORM:
    {
      sex = 0;

      form_no = work->diff_info_list[diff_no].other_diff;
    }
    break;
  }

  switch( work->diff_info_list[diff_no].color_diff )
  {
  case COLOR_DIFF_NONE:
    {
      rare = 0;
    }
    break;
  case COLOR_DIFF_NORMAL:
    {
      rare = 0;
    }
    break;
  case COLOR_DIFF_SPECIAL:
    {
      //rare = 1;
      rare = 0;
    }
    break;
  }

  work->poke_wk[poke_f] = PokeInit( work->mcss_sys_wk,
                              mons_no, form_no, sex, rare, FALSE, MCSS_DIR_FRONT );
  work->poke_wk[poke_b] = PokeInit( work->mcss_sys_wk,
                              mons_no, form_no, sex, rare, FALSE, MCSS_DIR_BACK );

  {
    VecFx32 p;
    if( pos == POKE_COMP_RPOS )
    {
      PokeGetCompareRelativePosition( work->poke_wk[POKE_CURR_F], &p );
    }
    else
    {
      p = poke_pos[pos];
    }
    MCSS_SetPosition( work->poke_wk[poke_f], &p );
    MCSS_SetPosition( work->poke_wk[poke_b], &p );
  }

  MCSS_SetVanishFlag( work->poke_wk[poke_b] );
}

//-------------------------------------
/// 違う姿情報からポケアイコンを生成する
//=====================================
static void Zukan_Detail_Form_PokeiconInitFromDiffInfo( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                u16 diff_no )
{
  u32 mons;
  u32 form_no;

  mons = ZKNDTL_COMMON_GetCurrPoke(cmn);

  switch( work->diff_info_list[diff_no].looks_diff )
  {
  case LOOKS_DIFF_ONE:
    {
      form_no = 0;
    }
    break;
  case LOOKS_DIFF_SEX:
    {
      form_no = 0;
    }
    break;
  case LOOKS_DIFF_FORM:
    {
      form_no = work->diff_info_list[diff_no].other_diff;
    }
    break;
  }

  work->pokeicon_clwk = PokeiconInit( &work->pokeicon_res, work->clunit, param->heap_id, mons, form_no, 0 );
}

//-------------------------------------
/// ポケモン変更
//=====================================
static void Zukan_Detail_Form_ChangePoke( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 前のを破棄
  u8 i;
  for( i=0; i<POKE_MAX; i++ )
  {
    BLOCK_POKE_EXIT( work->mcss_sys_wk, work->poke_wk[i] )
  }

  // 次のを生成
  Zukan_Detail_Form_GetDiffInfo( param, work, cmn );

  {
    POKE_POS_INDEX pos = POKE_CURR_POS_CENTER;
    if( work->state == STATE_EXCHANGE && work->diff_num >= 2 ) 
    {
      pos = POKE_CURR_POS_LEFT;
    }
    Zukan_Detail_Form_PokeInitFromDiffInfo( param, work, cmn,
        POKE_CURR_F, POKE_CURR_B, pos, work->diff_curr_no );
  }
  if( work->diff_num >= 2 )
  {
    Zukan_Detail_Form_PokeInitFromDiffInfo( param, work, cmn,
        POKE_COMP_F, POKE_COMP_B, POKE_COMP_RPOS, work->diff_comp_no );
  }

  // ポケアイコン変更
  Zukan_Detail_Form_ChangePokeicon( param, work, cmn );

  // テキスト
  // 変更されたポケモンで、上画面のテキストを書く
  Zukan_Detail_Form_WriteUpText( param, work, cmn );
  // 変更されたポケモンで、テキストをスクロールさせて、今のフォルムのテキストを書き、比較フォルムのテキストを書くor消す
  Zukan_Detail_Form_ScrollPokeText( param, work, cmn );
  Zukan_Detail_Form_WritePokeCurrText( param, work, cmn );
  Zukan_Detail_Form_WritePokeCompText( param, work, cmn );
}

//-------------------------------------
/// 比較フォルム変更
//=====================================
static void Zukan_Detail_Form_ChangeCompareForm( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 前のを破棄
  BLOCK_POKE_EXIT( work->mcss_sys_wk, work->poke_wk[POKE_COMP_F] )
  BLOCK_POKE_EXIT( work->mcss_sys_wk, work->poke_wk[POKE_COMP_B] )

  // 次のを生成
  Zukan_Detail_Form_PokeInitFromDiffInfo( param, work, cmn,
      POKE_COMP_F, POKE_COMP_B, POKE_COMP_RPOS, work->diff_comp_no );

  // テキスト
  // 変更された比較フォルムでテキストを書く
  Zukan_Detail_Form_WritePokeCompText( param, work, cmn );
}

//-------------------------------------
/// ポケアイコン変更
//=====================================
static void Zukan_Detail_Form_ChangePokeicon( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 前のを破棄
  BLOCK_POKEICON_EXIT( &work->pokeicon_res, work->pokeicon_clwk )
  
  // 次のを生成
  Zukan_Detail_Form_PokeiconInitFromDiffInfo( param, work, cmn, work->diff_curr_no );
}

//-------------------------------------
/// 入力
//=====================================
static void Zukan_Detail_Form_Input( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  switch(work->state)
  {
  case STATE_TOP:
    {
      u32 x, y;
      BOOL change_state = FALSE;
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
      {
        change_state = TRUE;
      }
      else
      {
		    if( GFL_UI_TP_GetPointTrg( &x, &y ) )
        {
          if( 0<=y&&y<168) change_state = TRUE;
        }
      }
      if( change_state )
      {
        Zukan_Detail_Form_ChangeState( param, work, cmn, STATE_EXCHANGE );
        PMSND_PlaySE( SEQ_SE_DECIDE1 );
      }
    }
    break;
  case STATE_EXCHANGE:
    {
    }
    break;
  }
}

//-------------------------------------
/// スクロールバーを移動させる
//=====================================
static void Zukan_Detail_Form_MoveScrollBar( param, work, cmn )
{
}

//-------------------------------------
/// 状態を遷移させる
//=====================================
static void Zukan_Detail_Form_ChangeState( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                STATE state )
{
  ZUKAN_DETAIL_TOUCHBAR_WORK*  touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);
 
  // 遷移前の状態 
  switch(work->state)
  {
  case STATE_TOP:
    {
      if( work->diff_num >= 2 )
      {
        VecFx32 p;

        MCSS_SetPosition( work->poke_wk[POKE_CURR_F], &poke_pos[POKE_CURR_POS_LEFT] );
        MCSS_SetPosition( work->poke_wk[POKE_CURR_B], &poke_pos[POKE_CURR_POS_LEFT] );

        PokeGetCompareRelativePosition( work->poke_wk[POKE_CURR_F], &p );
        MCSS_SetPosition( work->poke_wk[POKE_COMP_F], &p );
        MCSS_SetPosition( work->poke_wk[POKE_COMP_B], &p );
      }

      // タッチバー
      ZUKAN_DETAIL_TOUCHBAR_SetType(
          touchbar,
          ZUKAN_DETAIL_TOUCHBAR_TYPE_FORM,
          ZUKAN_DETAIL_TOUCHBAR_DISP_FORM );
    }
    break;
  case STATE_EXCHANGE:
    {
      if( work->diff_num >= 2 )
      {
        VecFx32 p;

        MCSS_SetPosition( work->poke_wk[POKE_CURR_F], &poke_pos[POKE_CURR_POS_CENTER] );
        MCSS_SetPosition( work->poke_wk[POKE_CURR_B], &poke_pos[POKE_CURR_POS_CENTER] );
      
        PokeGetCompareRelativePosition( work->poke_wk[POKE_CURR_F], &p );
        MCSS_SetPosition( work->poke_wk[POKE_COMP_F], &p );
        MCSS_SetPosition( work->poke_wk[POKE_COMP_B], &p );
      }

      // タッチバー
      ZUKAN_DETAIL_TOUCHBAR_SetType(
          touchbar,
          ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL,
          ZUKAN_DETAIL_TOUCHBAR_DISP_FORM );
    }
    break;
  }

  // 遷移後の状態
  work->state = state;

  // テキスト
  // 遷移された状態で、テキストをスクロールさせて、比較フォルムのテキストを書くor消す
  Zukan_Detail_Form_ScrollPokeText( param, work, cmn );
  Zukan_Detail_Form_WritePokeCompText( param, work, cmn );
}

//=============================================================================
/**
 * 
 */
//=============================================================================
// マクロ
#undef BLOCK_POKE_EXIT
#undef BLOCK_POKEICON_EXIT

