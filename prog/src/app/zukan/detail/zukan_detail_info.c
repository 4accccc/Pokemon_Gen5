//============================================================================
/**
 *  @file   zukan_detail_info.c
 *  @brief  図鑑詳細画面の説明画面
 *  @author Koji Kawada
 *  @data   2010.02.02
 *  @note   
 *  モジュール名：ZUKAN_DETAIL_INFO
 */
//============================================================================
#define DEBUG_KAWADA


// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "gamesystem/gamedata_def.h"
#include "savedata/zukan_savedata.h"
#include "sound/pm_sndsys.h"

#include "../info/zukan_info.h"

#include "zukan_detail_def.h"
#include "zukan_detail_common.h"
#include "zukan_detail_info.h"

// アーカイブ
#include "arc_def.h"
#include "zukan_gra.naix"

// サウンド

// オーバーレイ
FS_EXTERN_OVERLAY(ui_common);
FS_EXTERN_OVERLAY(zukan_toroku);


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
// メインOBJパレット
// 本数
enum
{
  OBJ_PAL_NUM_M_LANG_BTN             = 3,
};
// 位置
enum
{
  OBJ_PAL_POS_M_LANG_BTN             = 5,
};

// ProcMainのシーケンス
enum
{
  SEQ_START      = 0,
  SEQ_PREPARE,
  SEQ_FADE_IN,
  SEQ_MAIN,
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

// 言語ボタン
#define LEFT_SPACE (2)
static const u8 lang_button_rect[ZUKAN_INFO_LANG_MAX][8] =
{
  // place_x,         place_y, touch_x,         touch_y, touch_w, touch_h, active_anmseq, push_anmseq
  {  LEFT_SPACE+   0,      91, LEFT_SPACE+   0,      91,      16,      15,             8,          15 },
  {  LEFT_SPACE+  15,      91, LEFT_SPACE+  17,      91,      20,      15,             9,          16 },
  {  LEFT_SPACE+  36,      91, LEFT_SPACE+  38,      91,      20,      15,            10,          16 },
  {  LEFT_SPACE+  57,      91, LEFT_SPACE+  59,      91,      20,      15,            11,          17 },
  {  LEFT_SPACE+  78,      91, LEFT_SPACE+  80,      91,      20,      15,            12,          18 },
  {  LEFT_SPACE+  99,      91, LEFT_SPACE+ 101,      91,      20,      15,            13,          19 },
};
#undef LEFT_SPACE

// 言語ボタンの状態
typedef enum
{
  LANG_BTN_STATE_ACTIVE,      // 押すことができる
  LANG_BTN_STATE_PUSH_START,  // 押した瞬間(1フレーム)
  LANG_BTN_STATE_PUSH_ANIME,  // 押したアニメ中
  LANG_BTN_STATE_PUSH_END,    // 押したアニメが終了した瞬間(1フレーム)
}
LANG_BUTTON_STATE;


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
// 言語ボタン
typedef struct
{
  GFL_CLWK*           clwk;
  u8                  x;  // touch座標
  u8                  y;
  u8                  w;  // touchサイズ
  u8                  h;
  u16                 active_anmseq;
  u16                 push_anmseq;
  LANG_BUTTON_STATE   state;
}
LANG_BUTTON;

//-------------------------------------
/// PROC ワーク
//=====================================
typedef struct
{
  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;

  // 図鑑情報
  ZUKAN_INFO_WORK*            info_wk_m;
  ZUKAN_INFO_WORK*            info_wk_s;

  // フェード
  ZKNDTL_COMMON_FADE_WORK*    fade_wk_m;
  ZKNDTL_COMMON_FADE_WORK*    fade_wk_s;

  // 終了命令
  END_CMD                     end_cmd;

  // 言語
  LANG_BUTTON                 lang_btn[ZUKAN_INFO_LANG_MAX];
  ZUKAN_INFO_LANG             lang;  // 上画面に今表示している言語
  u32                         lang_ncl;
  u32                         lang_ncg;
  u32                         lang_nce;
}
ZUKAN_DETAIL_INFO_WORK;


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void Zukan_Detail_Info_VBlankFunc( GFL_TCB* tcb, void* wk );

// 言語ボタンのタッチ
static void Zukan_Detail_Info_TouchLangButton( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// ポケモンを変更する
static void Zukan_Detail_Info_ChangePoke( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn );
// 言語を変更する
static void Zukan_Detail_Info_ChangeLang( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn,
           ZUKAN_INFO_LANG lang);

// 言語ボタン
static void Zukan_Detail_Info_CreateLangButton( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Info_DeleteLangButton( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Info_MainLangButton( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Info_SetupLangButtonDrawEnable( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                BOOL e, BOOL fra, BOOL ger, BOOL ita, BOOL spa, BOOL kor );


//=============================================================================
/**
*  PROC
*/
//=============================================================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Info_ProcInit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn );
static ZKNDTL_PROC_RESULT Zukan_Detail_Info_ProcExit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn );
static ZKNDTL_PROC_RESULT Zukan_Detail_Info_ProcMain( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Info_CommandFunc( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn, ZKNDTL_COMMAND cmd );

const ZKNDTL_PROC_DATA    ZUKAN_DETAIL_INFO_ProcData =
{
  Zukan_Detail_Info_ProcInit,
  Zukan_Detail_Info_ProcMain,
  Zukan_Detail_Info_ProcExit,
  Zukan_Detail_Info_CommandFunc,
  NULL,
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
void             ZUKAN_DETAIL_INFO_InitParam(
                            ZUKAN_DETAIL_INFO_PARAM*  param,
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
static ZKNDTL_PROC_RESULT Zukan_Detail_Info_ProcInit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_INFO_PARAM*    param    = (ZUKAN_DETAIL_INFO_PARAM*)pwk;
  ZUKAN_DETAIL_INFO_WORK*     work;

#ifdef DEBUG_KAWADA
  {
    OS_Printf( "Zukan_Detail_Info_ProcInit\n" );
  }
#endif

  // オーバーレイ
  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common) );
  GFL_OVERLAY_Load( FS_OVERLAY_ID(zukan_toroku) );

  // ヒープ
  {
    work = ZKNDTL_PROC_AllocWork(proc, sizeof(ZUKAN_DETAIL_INFO_WORK), param->heap_id);
    GFL_STD_MemClear( work, sizeof(ZUKAN_DETAIL_INFO_WORK) ); 
  }
  
  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( Zukan_Detail_Info_VBlankFunc, work, 1 );

  // 言語
  work->lang = ZUKAN_INFO_LANG_NONE;

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

  return ZKNDTL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 終了処理
//=====================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Info_ProcExit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_INFO_PARAM*    param    = (ZUKAN_DETAIL_INFO_PARAM*)pwk;
  ZUKAN_DETAIL_INFO_WORK*     work     = (ZUKAN_DETAIL_INFO_WORK*)mywk;

  // 図鑑情報
  ZUKAN_INFO_Exit( work->info_wk_s );
  ZUKAN_INFO_Exit( work->info_wk_m );

  // 言語ボタン
  Zukan_Detail_Info_DeleteLangButton( param, work, cmn );

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
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(zukan_toroku) );
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common) );

#ifdef DEBUG_KAWADA
  {
    OS_Printf( "Zukan_Detail_Info_ProcExit\n" );
  }
#endif

  return ZKNDTL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 主処理
//=====================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Info_ProcMain( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_INFO_PARAM*    param    = (ZUKAN_DETAIL_INFO_PARAM*)pwk;
  ZUKAN_DETAIL_INFO_WORK*     work     = (ZUKAN_DETAIL_INFO_WORK*)mywk;

  ZUKAN_DETAIL_TOUCHBAR_WORK* touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);
  ZUKAN_DETAIL_HEADBAR_WORK*  headbar  = ZKNDTL_COMMON_GetHeadbar(cmn);
  
  switch(*seq)
  {
  case SEQ_START:
    {
      *seq = SEQ_PREPARE;

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
      }

      // 言語ボタン
      Zukan_Detail_Info_CreateLangButton( param, work, cmn );

      // 画面生成
      {
        ZUKAN_SAVEDATA* zkn_sv = GAMEDATA_GetZukanSave( ZKNDTL_COMMON_GetGamedata(cmn) );
        GFL_CLUNIT*     clunit = ZUKAN_DETAIL_GRAPHIC_GetClunit( ZKNDTL_COMMON_GetGraphic(cmn) );
        u16 monsno = ZKNDTL_COMMON_GetCurrPoke(cmn);
        u16 formno = 0;
        BOOL b_get = ZUKANSAVE_GetPokeGetFlag( zkn_sv, monsno );
        BOOL lang_exist[ZUKAN_INFO_LANG_MAX] = { 1, 1, 1, 0, 1, 1 };
  
        // 図鑑情報
        {
          work->info_wk_m = ZUKAN_INFO_InitFromMonsno(
                        param->heap_id,
                        monsno,
                        formno,
                        b_get,
                        ZUKAN_INFO_LAUNCH_LIST,
                        ZUKAN_INFO_DISP_M,
                        1,
                        clunit,
                        ZKNDTL_COMMON_GetFont(cmn),
                        ZKNDTL_COMMON_GetPrintQue(cmn) );

          work->info_wk_s = ZUKAN_INFO_InitFromMonsno(
                        param->heap_id,
                        monsno,
                        formno,
                        b_get,
                        ZUKAN_INFO_LAUNCH_LIST,
                        ZUKAN_INFO_DISP_S,
                        1,
                        clunit,
                        ZKNDTL_COMMON_GetFont(cmn),
                        ZKNDTL_COMMON_GetPrintQue(cmn) );
        }
      
        // 言語ボタンの表示非表示切り替え
        {
          Zukan_Detail_Info_SetupLangButtonDrawEnable( param, work, cmn,
              lang_exist[0], lang_exist[1], lang_exist[2], lang_exist[3], lang_exist[4], lang_exist[5] );
        }
      }
    }
    break;
  case SEQ_PREPARE:
    {
      *seq = SEQ_FADE_IN;

      // フェード
      ZKNDTL_COMMON_FadeSetBlackToColorless( work->fade_wk_m );
      
      // タッチバー
      if( ZUKAN_DETAIL_TOUCHBAR_GetState( touchbar ) != ZUKAN_DETAIL_TOUCHBAR_STATE_APPEAR )
      {
        ZUKAN_DETAIL_TOUCHBAR_SetType(
            touchbar,
            ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL,
            ZUKAN_DETAIL_TOUCHBAR_DISP_INFO );
        ZUKAN_DETAIL_TOUCHBAR_Appear( touchbar, ZUKAN_DETAIL_TOUCHBAR_SPEED_OUTSIDE );
      }
      // タイトルバー
      if( ZUKAN_DETAIL_HEADBAR_GetState( headbar ) != ZUKAN_DETAIL_HEADBAR_STATE_APPEAR )
      {
        ZUKAN_DETAIL_HEADBAR_SetType(
            headbar,
            ZUKAN_DETAIL_HEADBAR_TYPE_INFO );
        ZUKAN_DETAIL_HEADBAR_Appear( headbar );
      }
    }
    break;
  case SEQ_FADE_IN:
    {
      if(    (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_m ))
          && ZUKAN_DETAIL_TOUCHBAR_GetState( touchbar ) == ZUKAN_DETAIL_TOUCHBAR_STATE_APPEAR
          && ZUKAN_DETAIL_HEADBAR_GetState( headbar ) == ZUKAN_DETAIL_HEADBAR_STATE_APPEAR )
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
        *seq = SEQ_FADE_OUT;

        // フェード
        {
          ZKNDTL_COMMON_FadeSetColorlessToBlack( work->fade_wk_m );

          if( ZKNDTL_COMMON_FadeIsColorless( work->fade_wk_s ) )
          {
            ZKNDTL_COMMON_FadeSetColorlessToBlack( work->fade_wk_s );
          }
        } 
        
        // タイトルバー
        ZUKAN_DETAIL_HEADBAR_Disappear( headbar );
        
        if( work->end_cmd == END_CMD_OUTSIDE )
        {
          // タッチバー
          ZUKAN_DETAIL_TOUCHBAR_Disappear( touchbar, ZUKAN_DETAIL_TOUCHBAR_SPEED_OUTSIDE );
        }
      }
      else
      {
        // 言語ボタンのタッチ
        Zukan_Detail_Info_TouchLangButton( param, work, cmn ); 
      }
    }
    break;
  case SEQ_FADE_OUT:
    {
      if(    (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_m ))
          && (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_s ))
          && ZUKAN_DETAIL_HEADBAR_GetState( headbar ) == ZUKAN_DETAIL_HEADBAR_STATE_DISAPPEAR )
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
      return ZKNDTL_PROC_RES_FINISH;
    }
    break;
  }

  if( *seq >= SEQ_FADE_IN )
  {
    // 言語ボタン
    Zukan_Detail_Info_MainLangButton( param, work, cmn );
   
    // 図鑑情報
    ZUKAN_INFO_Main( work->info_wk_m );
    ZUKAN_INFO_Main( work->info_wk_s );
  }

  // フェード
  ZKNDTL_COMMON_FadeMain( work->fade_wk_m, work->fade_wk_s );

  return ZKNDTL_PROC_RES_CONTINUE;
}

//-------------------------------------
/// PROC 命令処理
//=====================================
static void Zukan_Detail_Info_CommandFunc( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn, ZKNDTL_COMMAND cmd )
{
  if( mywk )
  {
    ZUKAN_DETAIL_INFO_PARAM*    param    = (ZUKAN_DETAIL_INFO_PARAM*)pwk;
    ZUKAN_DETAIL_INFO_WORK*     work     = (ZUKAN_DETAIL_INFO_WORK*)mywk;

    ZUKAN_DETAIL_TOUCHBAR_WORK* touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);
  
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
    case ZKNDTL_CMD_MAP:
    case ZKNDTL_CMD_VOICE:
    case ZKNDTL_CMD_FORM:
      {
        work->end_cmd = END_CMD_INSIDE;
      }
      break;
    case ZKNDTL_CMD_CUR_D:
      {
        u16 monsno_curr;
        u16 monsno_go;
        monsno_curr = ZKNDTL_COMMON_GetCurrPoke(cmn);
        ZKNDTL_COMMON_GoToNextPoke(cmn);
        monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
        if( monsno_curr != monsno_go )
        {
          Zukan_Detail_Info_ChangePoke(param, work, cmn);
        }
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    case ZKNDTL_CMD_CUR_U:
      {
        u16 monsno_curr;
        u16 monsno_go;
        monsno_curr = ZKNDTL_COMMON_GetCurrPoke(cmn);
        ZKNDTL_COMMON_GoToPrevPoke(cmn);
        monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
        if( monsno_curr != monsno_go )
        {
          Zukan_Detail_Info_ChangePoke(param, work, cmn);
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
    default:
      {
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    }
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
static void Zukan_Detail_Info_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  ZUKAN_DETAIL_INFO_WORK*     work     = (ZUKAN_DETAIL_INFO_WORK*)wk;
}

//-------------------------------------
/// 言語ボタンのタッチ
//=====================================
static void Zukan_Detail_Info_TouchLangButton( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u32 x, y;

  // タッチ判定
  if( GFL_UI_TP_GetPointTrg(&x, &y) )
  {
    u8 i;
    for( i=0; i<ZUKAN_INFO_LANG_MAX; i++ )
    {
      if( GFL_CLACT_WK_GetDrawEnable( work->lang_btn[i].clwk ) )
      {
        if(    work->lang_btn[i].x <= x && x < work->lang_btn[i].x + work->lang_btn[i].w
            && work->lang_btn[i].y <= y && y < work->lang_btn[i].y + work->lang_btn[i].h )
        {
          if( work->lang != i )
          {
            work->lang_btn[i].state = LANG_BTN_STATE_PUSH_START;
            GFL_CLACT_WK_SetAnmSeq( work->lang_btn[i].clwk, work->lang_btn[i].push_anmseq );
            PMSND_PlaySE( SEQ_SE_DECIDE1 );

            // 言語を変更する
            Zukan_Detail_Info_ChangeLang( param, work, cmn, i );
          }
          break;
        }
      }
    }
  }
}

//-------------------------------------
/// ポケモンを変更する
//=====================================
static void Zukan_Detail_Info_ChangePoke( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_SAVEDATA* zkn_sv = GAMEDATA_GetZukanSave( ZKNDTL_COMMON_GetGamedata(cmn) );
  u16 monsno = ZKNDTL_COMMON_GetCurrPoke(cmn);

  u32  formno;
  u32  sex;
  BOOL rare;
  u32  personal_rnd = 0;

  BOOL b_get;
  BOOL lang_exist[ZUKAN_INFO_LANG_MAX];
  BOOL b_zenkoku;  // TRUEのとき全国図鑑

  // 現在表示する姿
  ZUKANSAVE_GetDrawData(  zkn_sv, monsno, &sex, &rare, &formno, param->heap_id );
  if( monsno == MONSNO_PATTIIRU )  // personal_rndはmons_no==MONSNO_PATTIIRUのときのみ使用する
  {
    personal_rnd = ZUKANSAVE_GetPokeRandomFlag( zkn_sv, ZUKANSAVE_RANDOM_PACHI );
  }

  // このポケモンの情報
  b_get = ZUKANSAVE_GetPokeGetFlag( zkn_sv, monsno );

  {
    // カントリーコードはどこに定義されている？
    lang_exist[ZUKAN_INFO_LANG_E]   = ZUKANSAVE_GetTextVersionUpFlag( zkn_sv, monsno, 1 );
    lang_exist[ZUKAN_INFO_LANG_FRA] = ZUKANSAVE_GetTextVersionUpFlag( zkn_sv, monsno, 2 );
    lang_exist[ZUKAN_INFO_LANG_GER] = ZUKANSAVE_GetTextVersionUpFlag( zkn_sv, monsno, 3 );
    lang_exist[ZUKAN_INFO_LANG_ITA] = ZUKANSAVE_GetTextVersionUpFlag( zkn_sv, monsno, 4 );
    lang_exist[ZUKAN_INFO_LANG_SPA] = ZUKANSAVE_GetTextVersionUpFlag( zkn_sv, monsno, 5 );
    lang_exist[ZUKAN_INFO_LANG_KOR] = ZUKANSAVE_GetTextVersionUpFlag( zkn_sv, monsno, 7 );
  }

  b_zenkoku = ZUKANSAVE_GetZukanMode( zkn_sv );
  
#if 1
  // 全言語チェック用
  {
    u8 i;
    for( i=0; i<ZUKAN_INFO_LANG_MAX; i++ )
    {
      lang_exist[i] = TRUE;
    }
  }
#endif

  // 図鑑情報
  {
    BOOL b_sub_off = TRUE;
    ZUKAN_INFO_ChangePoke( work->info_wk_m, monsno, formno, b_get );

    if( work->lang != ZUKAN_INFO_LANG_NONE )
    {
      if( lang_exist[work->lang] )
      {
        b_sub_off = FALSE;
      }
      else
      {
        work->lang = ZUKAN_INFO_LANG_NONE;
      }
    }

    if( b_sub_off )
    {
      ZUKAN_INFO_ChangePoke( work->info_wk_s, monsno, formno, b_get );

      // 直ちに黒にする
      ZKNDTL_COMMON_FadeSetBlackImmediately( ZKNDTL_COMMON_FADE_DISP_S, work->fade_wk_s );
    }
    else
    {
      ZUKAN_INFO_ChangePokeAndLang( work->info_wk_s, monsno, formno, b_get,
                work->lang );
    }
  }

  // 言語ボタンの表示非表示切り替え
  {
    Zukan_Detail_Info_SetupLangButtonDrawEnable( param, work, cmn,
        lang_exist[0], lang_exist[1], lang_exist[2], lang_exist[3], lang_exist[4], lang_exist[5] );
  }
}

//-------------------------------------
/// 言語を変更する
//=====================================
static void Zukan_Detail_Info_ChangeLang( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn,
           ZUKAN_INFO_LANG lang)
{
  if( work->lang == lang ) return;
 
  ZUKAN_INFO_ChangeLang( work->info_wk_s, lang );

  if( work->lang == ZUKAN_INFO_LANG_NONE )
  {
    // 直ちに見えるようにする
    ZKNDTL_COMMON_FadeSetColorlessImmediately( ZKNDTL_COMMON_FADE_DISP_S, work->fade_wk_s );
  }

  work->lang = lang;
}

//-------------------------------------
/// 言語ボタン
//=====================================
static void Zukan_Detail_Info_CreateLangButton( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_GRAPHIC_WORK* graphic = ZKNDTL_COMMON_GetGraphic(cmn);
  GFL_CLUNIT* clunit = ZUKAN_DETAIL_GRAPHIC_GetClunit(graphic);

  // メイン or サブ
  CLSYS_DRAW_TYPE draw_type = CLSYS_DRAW_MAIN;
  CLSYS_DEFREND_TYPE def_type = CLSYS_DEFREND_MAIN;

  // リソース読み込み
  {
    ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, param->heap_id );
    work->lang_ncl = GFL_CLGRP_PLTT_RegisterEx(
                            handle,
                            NARC_zukan_gra_info_info_obj_NCLR,
                            draw_type,
                            OBJ_PAL_POS_M_LANG_BTN * 0x20, 0, OBJ_PAL_NUM_M_LANG_BTN,
                            param->heap_id );	
    work->lang_ncg = GFL_CLGRP_CGR_Register(
                            handle,
                            NARC_zukan_gra_info_info_obj_NCGR,
                            FALSE,
                            draw_type,
                            param->heap_id );
    work->lang_nce = GFL_CLGRP_CELLANIM_Register(
                            handle,
                            NARC_zukan_gra_info_info_obj_NCER,
                            NARC_zukan_gra_info_info_obj_NANR,
                            param->heap_id );
    GFL_ARC_CloseDataHandle( handle );
  }

  // CLWK作成
  {
    u8 i;
    for( i=0; i<ZUKAN_INFO_LANG_MAX; i++ )
    {
      GFL_CLWK_DATA   cldata;
		  GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
      
      work->lang_btn[i].x             = lang_button_rect[i][2];
      work->lang_btn[i].y             = lang_button_rect[i][3];
      work->lang_btn[i].w             = lang_button_rect[i][4];
      work->lang_btn[i].h             = lang_button_rect[i][5];
      work->lang_btn[i].active_anmseq = lang_button_rect[i][6];
      work->lang_btn[i].push_anmseq   = lang_button_rect[i][7];
      work->lang_btn[i].state         = LANG_BTN_STATE_ACTIVE;

      cldata.pos_x = lang_button_rect[i][0];
      cldata.pos_y = lang_button_rect[i][1];
      cldata.anmseq = work->lang_btn[i].active_anmseq;

		  work->lang_btn[i].clwk = GFL_CLACT_WK_Create( clunit,
                             work->lang_ncg, work->lang_ncl, work->lang_nce,
                             &cldata, def_type, param->heap_id );
		  GFL_CLACT_WK_SetAutoAnmFlag( work->lang_btn[i].clwk, TRUE );
      GFL_CLACT_WK_SetObjMode( work->lang_btn[i].clwk, GX_OAM_MODE_XLU );  // BGとともにこのOBJも暗くしたいので
    }
  }
}
static void Zukan_Detail_Info_DeleteLangButton( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
	// CLWK破棄
  {
    u8 i;
    for( i=0; i<ZUKAN_INFO_LANG_MAX; i++ )
    {
	    GFL_CLACT_WK_Remove( work->lang_btn[i].clwk );
    }
  }

  // リソース破棄
  {	
    GFL_CLGRP_PLTT_Release( work->lang_ncl );
    GFL_CLGRP_CGR_Release( work->lang_ncg );
    GFL_CLGRP_CELLANIM_Release( work->lang_nce );
  }
}
static void Zukan_Detail_Info_MainLangButton( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u8 i;
  for( i=0; i<ZUKAN_INFO_LANG_MAX; i++ )
  {
    switch( work->lang_btn[i].state )
    {
    case LANG_BTN_STATE_ACTIVE:
      {
        // 何もしない
      }
      break;
    case LANG_BTN_STATE_PUSH_START:  // この関数の呼び出し位置によっては、この状態には1フレームもなっていないかも。
      {
        work->lang_btn[i].state = LANG_BTN_STATE_PUSH_ANIME;
      }
      break;
    case LANG_BTN_STATE_PUSH_ANIME:
      {
        if( !GFL_CLACT_WK_CheckAnmActive( work->lang_btn[i].clwk ) )
        {
          work->lang_btn[i].state = LANG_BTN_STATE_PUSH_END;
        } 
      }
      break;
    case LANG_BTN_STATE_PUSH_END:
      {
        GFL_CLACT_WK_SetAnmSeq( work->lang_btn[i].clwk, work->lang_btn[i].active_anmseq );
        work->lang_btn[i].state = LANG_BTN_STATE_ACTIVE;
      }
      break;
    }
  }
}
static void Zukan_Detail_Info_SetupLangButtonDrawEnable( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                BOOL e, BOOL fra, BOOL ger, BOOL ita, BOOL spa, BOOL kor )
{
  u8 i;

  GFL_CLACT_WK_SetDrawEnable( work->lang_btn[ZUKAN_INFO_LANG_E].clwk,   e   );
  GFL_CLACT_WK_SetDrawEnable( work->lang_btn[ZUKAN_INFO_LANG_FRA].clwk, fra );
  GFL_CLACT_WK_SetDrawEnable( work->lang_btn[ZUKAN_INFO_LANG_GER].clwk, ger );
  GFL_CLACT_WK_SetDrawEnable( work->lang_btn[ZUKAN_INFO_LANG_ITA].clwk, ita );
  GFL_CLACT_WK_SetDrawEnable( work->lang_btn[ZUKAN_INFO_LANG_SPA].clwk, spa );
  GFL_CLACT_WK_SetDrawEnable( work->lang_btn[ZUKAN_INFO_LANG_KOR].clwk, kor );

  for( i=0; i<ZUKAN_INFO_LANG_MAX; i++ )
  {
    work->lang_btn[i].state         = LANG_BTN_STATE_ACTIVE;
    GFL_CLACT_WK_SetAnmSeq( work->lang_btn[i].clwk, work->lang_btn[i].active_anmseq );
  }
}

