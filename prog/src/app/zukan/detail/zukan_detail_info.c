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

#define DEF_SCMD_CHANGE  // これが定義されているとき、CMDが発行されるまで待たずにSCMDが発行されたときに変更を開始する

//#define DEBUG_KAWADA


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
#include "message.naix"
#include "font/font.naix"
#include "zukan_gra.naix"

// サウンド

// オーバーレイ
FS_EXTERN_OVERLAY(ui_common);


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
// メインOBJパレット
/*
タッチバーで読み込んだパレットを流用するので、ここでは読み込まない
// 本数
enum
{
  OBJ_PAL_NUM_M_LANG_BTN             = 3,
};
// 位置
enum
{
  OBJ_PAL_POS_M_LANG_BTN             = 6,
};
*/

// 言語ボタンのパレットアニメ用にパレットを変更する
/*
タッチバーで読み込んだパレットを流用するので、ここでは読み込まない
#define OBJ_PAL_OFFSET_LANG_ANIME_NONE  (0)
#define OBJ_PAL_OFFSET_LANG_ANIME_EXEC  (2)
*/
#define OBJ_PAL_POS_M_LANG_ANIME_NONE   (ZKNDTL_OBJ_PAL_POS_M_TOUCHBAR + ZKND_TBAR_OBJ_PLT_NUM)  // zukan/detail/zukan_detail_touchbar.c OBJ_PAL_POS_M_CUSTOM から  // パレットを直で変えるってできないっぽいから、結局これは使用せず、zukan/detail/zukan_detail_touchbar.c OBJ_PAL_OFFSET_GENERAL_ANIME_NONEを流用することにした。
#define OBJ_PAL_POS_M_LANG_ANIME_EXEC   (6)  // zukan/info/zukan_info.c ZUKAN_INFO_BG_PAL_POS_MAX から  // パレットを直で変えるってできないっぽいから、結局これは使用せず、zukan/detail/zukan_detail_touchbar.c OBJ_PAL_OFFSET_GENERAL_ANIME_EXECを流用することにした。

#define OBJ_PAL_OFFSET_LANG_ANIME_NONE  (0)  // zukan/detail/zukan_detail_touchbar.c OBJ_PAL_OFFSET_GENERAL_ANIME_NONE から
#define OBJ_PAL_OFFSET_LANG_ANIME_EXEC  (2)  // zukan/detail/zukan_detail_touchbar.c OBJ_PAL_OFFSET_GENERAL_ANIME_EXEC から
// zukan/detail/zukan_detail_touchbar.c OBJ_PAL_OFFSET_GENERAL_ANIME_NONEやOBJ_PAL_OFFSET_GENERAL_ANIME_EXECを利用するようにしたので、このソース中のパレットアニメ関連の記述は削除しても構わない。

#define RES_PAL_POS_LANG_ANIME_START    (1)  // リソースのパレットの列番号
#define RES_PAL_POS_LANG_ANIME_END      (3)  // リソースのパレットの列番号
#define LANG_ANIME_ADD              (0x400)  // FX_CosIdxを使用するので0<= <0x10000

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
static const u8 lang_button_rect[ZUKAN_INFO_LANG_MAX][9] =
{
  // place_x,         place_y, touch_x,         touch_y, touch_w, touch_h, active_anmseq, push_anmseq, lang_code
  {  LEFT_SPACE+   0,      91, LEFT_SPACE+   0,      91,      16,      15,             8,          15, LANG_ENGLISH  },
  {  LEFT_SPACE+  15,      91, LEFT_SPACE+  17,      91,      20,      15,             9,          16, LANG_FRANCE   },
  {  LEFT_SPACE+  36,      91, LEFT_SPACE+  38,      91,      20,      15,            10,          27, LANG_GERMANY  },
  {  LEFT_SPACE+  57,      91, LEFT_SPACE+  59,      91,      20,      15,            11,          17, LANG_ITALY    },
  {  LEFT_SPACE+  78,      91, LEFT_SPACE+  80,      91,      20,      15,            12,          18, LANG_SPAIN    },
  {  LEFT_SPACE+  99,      91, LEFT_SPACE+ 101,      91,      20,      15,            13,          19, LANG_KOREA    },
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
  u8                  lang_code;  // 言語コード(カントリーコード、国コードと呼んでいるところもある)  // pm_version.hのLANG_ENGLISH, LANG_FRANCEなど
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

  // 入力可不可
  BOOL                        input_enable;  // 入力可のときTRUE

  // 言語
  LANG_BUTTON                 lang_btn[ZUKAN_INFO_LANG_MAX];
  ZUKAN_INFO_LANG             lang_btn_push;  // 今押しアニメしているボタン(ZUKAN_INFO_LANG_NONEのときなし)
  ZUKAN_INFO_LANG             lang;  // 上画面に今表示している言語
  u32                         lang_ncl;  // タッチバーで読み込んだパレットを流用するので、ここでは読み込まない
  u32                         lang_ncg;
  u32                         lang_nce;

  // 言語ボタンのパレットアニメ
  //u16           pal_anime_lang_default[0x10];  // pal_anime_lang_defaultを割り当てると押しアニメしなくなってしまったので、pal_anime_lang_pltt_offs_reqを使うことにした。
  u16           pal_anime_lang_start[0x10];
  u16           pal_anime_lang_end[0x10];
  u16           pal_anime_lang_now[0x10];
  int           pal_anime_lang_count;
  
  BOOL          pal_anime_lang_pltt_offs_req;  // 押しアニメしているときは、パレットアニメをしないように、パレットアニメの開始を遅らせる

  // フォント
  GFL_FONT*     font;  // 独自にフォントを用意する(全文字をメモリに常駐させておく)

  // メッセージデータ
  GFL_MSGDATA*  msgdata_kind;    // 上画面と下画面で2度読みしないように
  GFL_MSGDATA*  msgdata_height;
  GFL_MSGDATA*  msgdata_weight;
}
ZUKAN_DETAIL_INFO_WORK;


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void Zukan_Detail_Info_VBlankFunc( GFL_TCB* tcb, void* wk );

// 言語ボタンのタッチ入力
static BOOL Zukan_Detail_Info_TouchLangButton( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn );
// 言語ボタンのキー入力
static BOOL Zukan_Detail_Info_KeyLangButton( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn );

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

// 現在表示するポケモンの情報を得る
static void Zukan_Detail_Info_GetCurrPokeInfo(
                ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                u16*  a_monsno,
                u16*  a_formno,
                u16*  a_sex,
                u16*  a_rare,
                u32*  a_personal_rnd,
                BOOL* a_b_get_flag,
                BOOL* a_lang_exist );  // a_lang_exist[ZUKAN_INFO_LANG_MAX]

// 言語ボタンのパレットアニメ
static void Zukan_Detail_Info_AnimeBaseInitLang( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Info_AnimeBaseExitLang( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Info_AnimeInitLang( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Info_AnimeExitLang( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Info_AnimeMainLang( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn );


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
  GFL_OVERLAY_Load( FS_OVERLAY_ID(zukan_info) );

  // ヒープ
  {
    work = ZKNDTL_PROC_AllocWork(proc, sizeof(ZUKAN_DETAIL_INFO_WORK), param->heap_id);
    GFL_STD_MemClear( work, sizeof(ZUKAN_DETAIL_INFO_WORK) ); 
  }
  
  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( Zukan_Detail_Info_VBlankFunc, work, 1 );

  // 言語
  work->lang_btn_push = ZUKAN_INFO_LANG_NONE;
  work->lang = ZUKAN_INFO_LANG_NONE;

  work->pal_anime_lang_pltt_offs_req = FALSE;

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

  // 入力可不可
  work->input_enable = TRUE;

  return ZKNDTL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 終了処理
//=====================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Info_ProcExit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_INFO_PARAM*    param    = (ZUKAN_DETAIL_INFO_PARAM*)pwk;
  ZUKAN_DETAIL_INFO_WORK*     work     = (ZUKAN_DETAIL_INFO_WORK*)mywk;

  // 言語ボタンのパレットアニメ
  Zukan_Detail_Info_AnimeExitLang( param, work, cmn );
  Zukan_Detail_Info_AnimeBaseExitLang( param, work, cmn );

  // 図鑑情報
  ZUKAN_INFO_Exit( work->info_wk_s );
  ZUKAN_INFO_Exit( work->info_wk_m );

  // 言語ボタン
  Zukan_Detail_Info_DeleteLangButton( param, work, cmn );

  // フォント
  {
    // 独自にフォントを用意していたので削除
    GFL_FONT_Delete( work->font );
  }

  // メッセージデータ
  {
    // 上画面と下画面で2度読みしないように、ここで読み込んでいたので削除
    GFL_MSG_Delete( work->msgdata_weight );
    GFL_MSG_Delete( work->msgdata_height );
    GFL_MSG_Delete( work->msgdata_kind );
  }

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
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(zukan_info) );
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

      // フォント
      {
        // 独自にフォントを用意しておく
        work->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_MEMORY, FALSE, param->heap_id );
      }

      // メッセージデータ
      {
        // 上画面と下画面で2度読みしないように、ここで読み込んでおく
        work->msgdata_kind   = GFL_MSG_Create( GFL_MSG_LOAD_FAST, ARCID_MESSAGE, NARC_message_zkn_type_dat, param->heap_id );
        work->msgdata_height = GFL_MSG_Create( GFL_MSG_LOAD_FAST, ARCID_MESSAGE, NARC_message_zkn_height_dat, param->heap_id );
        work->msgdata_weight = GFL_MSG_Create( GFL_MSG_LOAD_FAST, ARCID_MESSAGE, NARC_message_zkn_weight_dat, param->heap_id );
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
      }

    // タッチバーで読み込んだパレットを流用するので、ここでは読み込まない
    // というのがあるので、タッチバーを先に生成しておく。
    {
      // タッチバー
      if( ZUKAN_DETAIL_TOUCHBAR_GetState( touchbar ) != ZUKAN_DETAIL_TOUCHBAR_STATE_APPEAR )
      {
        ZUKAN_DETAIL_TOUCHBAR_SetType(
            touchbar,
            ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL,
            ZUKAN_DETAIL_TOUCHBAR_DISP_INFO,
            (ZKNDTL_COMMON_GetPokeNum(cmn)>1)?TRUE:FALSE ); 
        ZUKAN_DETAIL_TOUCHBAR_Appear( touchbar, ZUKAN_DETAIL_TOUCHBAR_SPEED_OUTSIDE );
      }
      else
      {
        ZUKAN_DETAIL_TOUCHBAR_SetDispOfGeneral(
            touchbar,
            ZUKAN_DETAIL_TOUCHBAR_DISP_INFO );
      }
      ZUKAN_DETAIL_TOUCHBAR_SetUserActiveWhole( touchbar, FALSE );  // ZUKAN_DETAIL_TOUCHBAR_SetTypeのときはUnlock状態なので
      {
        GAMEDATA* gamedata = ZKNDTL_COMMON_GetGamedata(cmn);
        ZUKAN_DETAIL_TOUCHBAR_SetCheckFlipOfGeneral(
            touchbar,
            GAMEDATA_GetShortCut( gamedata, SHORTCUT_ID_ZUKAN_INFO ) );
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

      // 言語ボタン
      Zukan_Detail_Info_CreateLangButton( param, work, cmn );

      // 画面生成
      {
        ZUKAN_SAVEDATA* zkn_sv = GAMEDATA_GetZukanSave( ZKNDTL_COMMON_GetGamedata(cmn) );
        GFL_CLUNIT*     clunit = ZUKAN_DETAIL_GRAPHIC_GetClunit( ZKNDTL_COMMON_GetGraphic(cmn) );
        
        BOOL b_zenkoku_flag = ZUKANSAVE_GetZukanMode( zkn_sv );  // TRUEのとき全国図鑑として扱っている
        // 全国図鑑を持っていても、地方図鑑として扱っていたらFALSEになる。
        
        u16  monsno;
        u16  formno;
        u16  sex;
        u16  rare;
        u32  personal_rnd;
        BOOL b_get_flag;
        BOOL lang_exist[ZUKAN_INFO_LANG_MAX];

        // 現在表示するポケモンの情報を得る
        Zukan_Detail_Info_GetCurrPokeInfo(
                param, work, cmn,
                &monsno,
                &formno,
                &sex,
                &rare,
                &personal_rnd,
                &b_get_flag,
                lang_exist );
        
        // 図鑑情報
        {
          work->info_wk_m = ZUKAN_INFO_InitFromMonsno(
                        param->heap_id,
                        monsno,
                        formno,
                        sex,
                        rare,
                        personal_rnd,
                        b_zenkoku_flag,
                        b_get_flag,
                        ZUKAN_INFO_LAUNCH_LIST,
                        ZUKAN_INFO_DISP_M,
                        1,
                        clunit,
                        //ZKNDTL_COMMON_GetFont(cmn),
                        work->font,  // 独自に用意したフォントを使う
                        ZKNDTL_COMMON_GetPrintQue(cmn),
                        work->msgdata_kind, 
                        work->msgdata_height, 
                        work->msgdata_weight );

          work->info_wk_s = ZUKAN_INFO_InitFromMonsno(
                        param->heap_id,
                        monsno,
                        formno,
                        sex,
                        rare,
                        personal_rnd,
                        b_zenkoku_flag,
                        b_get_flag,
                        ZUKAN_INFO_LAUNCH_LIST,
                        ZUKAN_INFO_DISP_S,
                        1,
                        clunit,
                        //ZKNDTL_COMMON_GetFont(cmn),
                        work->font,  // 独自に用意したフォントを使う
                        ZKNDTL_COMMON_GetPrintQue(cmn),
                        work->msgdata_kind, 
                        work->msgdata_height, 
                        work->msgdata_weight );
        }
     
        // 直ちに何も表示しないとき用の背景のみを表示する
        ZUKAN_INFO_DisplayBackNone( work->info_wk_s );

        // 言語ボタンの表示非表示切り替え
        {
          Zukan_Detail_Info_SetupLangButtonDrawEnable( param, work, cmn,
              lang_exist[0], lang_exist[1], lang_exist[2], lang_exist[3], lang_exist[4], lang_exist[5] );
        }

        // 言語ボタンのパレットアニメ
        Zukan_Detail_Info_AnimeBaseInitLang( param, work, cmn );
        Zukan_Detail_Info_AnimeInitLang( param, work, cmn );
      }
    }
    break;
  case SEQ_PREPARE:
    {
      *seq = SEQ_FADE_IN;

      // フェード
      ZKNDTL_COMMON_FadeSetBlackToColorless( work->fade_wk_m );
      ZKNDTL_COMMON_FadeSetBlackToColorless( work->fade_wk_s );  // 何も表示しないとき用の背景のみを表示するモードを用意したので

/*
タッチバーで読み込んだパレットを流用するので、ここでは読み込まない
というのがあるので、タッチバーを先に生成しておく。
      // タッチバー
      if( ZUKAN_DETAIL_TOUCHBAR_GetState( touchbar ) != ZUKAN_DETAIL_TOUCHBAR_STATE_APPEAR )
      {
        ZUKAN_DETAIL_TOUCHBAR_SetType(
            touchbar,
            ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL,
            ZUKAN_DETAIL_TOUCHBAR_DISP_INFO );
        ZUKAN_DETAIL_TOUCHBAR_Appear( touchbar, ZUKAN_DETAIL_TOUCHBAR_SPEED_OUTSIDE );
      }
      else
      {
        ZUKAN_DETAIL_TOUCHBAR_SetDispOfGeneral(
            touchbar,
            ZUKAN_DETAIL_TOUCHBAR_DISP_INFO );
      }
      ZUKAN_DETAIL_TOUCHBAR_SetUserActiveWhole( touchbar, FALSE );  // ZUKAN_DETAIL_TOUCHBAR_SetTypeのときはUnlock状態なので
      {
        GAMEDATA* gamedata = ZKNDTL_COMMON_GetGamedata(cmn);
        ZUKAN_DETAIL_TOUCHBAR_SetCheckFlipOfGeneral(
            touchbar,
            GAMEDATA_GetShortCut( gamedata, SHORTCUT_ID_ZUKAN_INFO ) );
      }
      // タイトルバー
      if( ZUKAN_DETAIL_HEADBAR_GetState( headbar ) != ZUKAN_DETAIL_HEADBAR_STATE_APPEAR )
      {
        ZUKAN_DETAIL_HEADBAR_SetType(
            headbar,
            ZUKAN_DETAIL_HEADBAR_TYPE_INFO );
        ZUKAN_DETAIL_HEADBAR_Appear( headbar );
      }
*/

    }
    break;
  case SEQ_FADE_IN:
    {
      if(    (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_m ))
          && ZUKAN_DETAIL_TOUCHBAR_GetState( touchbar ) == ZUKAN_DETAIL_TOUCHBAR_STATE_APPEAR
          && ZUKAN_DETAIL_HEADBAR_GetState( headbar ) == ZUKAN_DETAIL_HEADBAR_STATE_APPEAR )
      {
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
        ZUKAN_DETAIL_TOUCHBAR_SetUserActiveWhole( touchbar, TRUE );  // ZUKAN_DETAIL_TOUCHBAR_SetTypeのときはUnlock状態なので

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

          //if( ZKNDTL_COMMON_FadeIsColorless( work->fade_wk_s ) )
          //{
          //  ZKNDTL_COMMON_FadeSetColorlessToBlack( work->fade_wk_s );
          //}
          ZKNDTL_COMMON_FadeSetColorlessToBlack( work->fade_wk_s );  // 何も表示しないとき用の背景のみを表示するモードを用意したので
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
        if( work->input_enable )
        {
          // 言語ボタンのタッチ入力
          BOOL is_input = Zukan_Detail_Info_TouchLangButton( param, work, cmn ); 
        
          // 言語ボタンのキー入力
          if( !is_input )
          {
            Zukan_Detail_Info_KeyLangButton( param, work, cmn );
          }
        }
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

    // 言語ボタンのパレットアニメ
    if( work->lang_btn_push == ZUKAN_INFO_LANG_NONE )  // 押しアニメしているときは、パレットアニメをしないように
    {
      Zukan_Detail_Info_AnimeMainLang( param, work, cmn );
    }
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
 
    BOOL b_valid_cmd = FALSE;  // cmdがZKNDTL_CMD_NONE以外のZKNDTL_CMD_???のときTRUE。ZKNDTL_CMD_NONEやZKNDTL_SCMD_???のときFALSE。

    // 入力不可
    switch( cmd )
    {
    case ZKNDTL_SCMD_CLOSE:
    case ZKNDTL_SCMD_RETURN:
    case ZKNDTL_SCMD_MAP:
    case ZKNDTL_SCMD_VOICE:
    case ZKNDTL_SCMD_FORM:
    case ZKNDTL_SCMD_CUR_D:
    case ZKNDTL_SCMD_CUR_U:
    case ZKNDTL_SCMD_CHECK:
      {
        work->input_enable = FALSE;
      }
      break;
    }
    // 入力可
    switch( cmd )
    {
    case ZKNDTL_CMD_CLOSE:
    case ZKNDTL_CMD_RETURN:
    case ZKNDTL_CMD_MAP:
    case ZKNDTL_CMD_VOICE:
    case ZKNDTL_CMD_FORM:
    case ZKNDTL_CMD_CUR_D:
    case ZKNDTL_CMD_CUR_U:
    case ZKNDTL_CMD_CHECK:
      {
        work->input_enable = TRUE;
        b_valid_cmd = TRUE;
      }
      break;
    }


#ifdef DEF_SCMD_CHANGE
    switch( cmd )
    {
    case ZKNDTL_SCMD_CUR_D:
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
      }
      break;
    case ZKNDTL_SCMD_CUR_U:
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
      }
      break;
    }
#endif


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
#ifndef DEF_SCMD_CHANGE
        u16 monsno_curr;
        u16 monsno_go;
        monsno_curr = ZKNDTL_COMMON_GetCurrPoke(cmn);
        ZKNDTL_COMMON_GoToNextPoke(cmn);
        monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
        if( monsno_curr != monsno_go )
        {
          Zukan_Detail_Info_ChangePoke(param, work, cmn);
        }
#endif
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    case ZKNDTL_CMD_CUR_U:
      {
#ifndef DEF_SCMD_CHANGE
        u16 monsno_curr;
        u16 monsno_go;
        monsno_curr = ZKNDTL_COMMON_GetCurrPoke(cmn);
        ZKNDTL_COMMON_GoToPrevPoke(cmn);
        monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
        if( monsno_curr != monsno_go )
        {
          Zukan_Detail_Info_ChangePoke(param, work, cmn);
        }
#endif
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    case ZKNDTL_CMD_CHECK:
      {
        GAMEDATA* gamedata = ZKNDTL_COMMON_GetGamedata(cmn);
        GAMEDATA_SetShortCut(
            gamedata,
            SHORTCUT_ID_ZUKAN_INFO,
            ZUKAN_DETAIL_TOUCHBAR_GetCheckFlipOfGeneral( touchbar ) );
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    default:
      {
        if( b_valid_cmd )
        {
          ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
        }
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
/// 言語ボタンのタッチ入力
//=====================================
static BOOL Zukan_Detail_Info_TouchLangButton( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_TOUCHBAR_WORK* touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);
  BOOL is_input = FALSE;

  if( work->lang_btn_push == ZUKAN_INFO_LANG_NONE )
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
              work->lang_btn_push = i;
              is_input = TRUE;
            }
            break;
          }
        }
      }
    }
  }

  if( is_input )
  {
    // 言語を変更する
    Zukan_Detail_Info_ChangeLang( param, work, cmn, work->lang_btn_push );
    
    work->lang_btn[work->lang_btn_push].state = LANG_BTN_STATE_PUSH_START;
    GFL_CLACT_WK_SetAnmSeq( work->lang_btn[work->lang_btn_push].clwk, work->lang_btn[work->lang_btn_push].push_anmseq );
    PMSND_PlaySE( SEQ_SE_SELECT3 );
    GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
    ZUKAN_DETAIL_TOUCHBAR_SetUserActiveWhole( touchbar, FALSE );
  }

  return is_input;
}

//-------------------------------------
/// 言語ボタンのキー入力
//=====================================
static BOOL Zukan_Detail_Info_KeyLangButton( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_TOUCHBAR_WORK* touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);
  BOOL is_input = FALSE;
  
  if( work->lang_btn_push == ZUKAN_INFO_LANG_NONE )
  {
    u8 i;

    // キー判定
    int rept = GFL_UI_KEY_GetRepeat();
    int trg  = GFL_UI_KEY_GetTrg();
 
    if( work->lang == ZUKAN_INFO_LANG_NONE )  // まだどれも選んでいないとき
    {
      if(    ( rept & ( PAD_BUTTON_R | PAD_BUTTON_L ) )
          || ( trg & PAD_BUTTON_A ) )
      {
        for( i=0; i<ZUKAN_INFO_LANG_MAX; i++ )  // 一番左にある言語ボタンを選ぶ
        {
          if( GFL_CLACT_WK_GetDrawEnable( work->lang_btn[i].clwk ) )
          {
            work->lang_btn_push = i;
            is_input = TRUE;
            break;
          }
        }
      }
    }
    else  // 既にどれかを選んでいるとき
    {
      if(    ( rept & PAD_BUTTON_R )
          || ( trg & PAD_BUTTON_A ) )
      {
        i = 1;
        while( i < ZUKAN_INFO_LANG_MAX )
        {
          u8 j = ( work->lang + i ) % ZUKAN_INFO_LANG_MAX;
          if( GFL_CLACT_WK_GetDrawEnable( work->lang_btn[j].clwk ) )
          {
            work->lang_btn_push = j;
            is_input = TRUE;
            break;
          }
          i++;
        }
      }
      else if( rept & PAD_BUTTON_L )
      {
        i = 1;
        while( i < ZUKAN_INFO_LANG_MAX )
        {
          u8 j = ( work->lang + ZUKAN_INFO_LANG_MAX - i ) % ZUKAN_INFO_LANG_MAX;
          if( GFL_CLACT_WK_GetDrawEnable( work->lang_btn[j].clwk ) )
          {
            work->lang_btn_push = j;
            is_input = TRUE;
            break;
          }
          i++;
        }
      }
    }
  }

  if( is_input )
  {
    // 言語を変更する
    Zukan_Detail_Info_ChangeLang( param, work, cmn, work->lang_btn_push );
    
    work->lang_btn[work->lang_btn_push].state = LANG_BTN_STATE_PUSH_START;
    GFL_CLACT_WK_SetAnmSeq( work->lang_btn[work->lang_btn_push].clwk, work->lang_btn[work->lang_btn_push].push_anmseq );
    PMSND_PlaySE( SEQ_SE_SELECT3 );
    GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
    ZUKAN_DETAIL_TOUCHBAR_SetUserActiveWhole( touchbar, FALSE );
  }
  
  return is_input;
}

//-------------------------------------
/// ポケモンを変更する
//=====================================
static void Zukan_Detail_Info_ChangePoke( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u16  monsno;
  u16  formno;
  u16  sex;
  u16  rare;
  u32  personal_rnd;
  BOOL b_get_flag;
  BOOL lang_exist[ZUKAN_INFO_LANG_MAX];

  // 現在表示するポケモンの情報を得る
  Zukan_Detail_Info_GetCurrPokeInfo(
                param, work, cmn,
                &monsno,
                &formno,
                &sex,
                &rare,
                &personal_rnd,
                &b_get_flag,
                lang_exist );

  // 図鑑情報
  {
    BOOL b_sub_off = TRUE;

#ifdef DEBUG_KAWADA
    if( monsno == 202 ) b_get_flag = FALSE;
#endif

    ZUKAN_INFO_ChangePoke( work->info_wk_m, monsno, formno, sex, rare, personal_rnd, b_get_flag );

    if( work->lang != ZUKAN_INFO_LANG_NONE )
    {
      if( lang_exist[work->lang] )
      {
        b_sub_off = FALSE;
      }
      else
      {
        // 言語ボタンのパレットアニメ
        Zukan_Detail_Info_AnimeExitLang( param, work, cmn );

        work->lang = ZUKAN_INFO_LANG_NONE;
      }
    }

    if( b_sub_off )
    {
      //何も表示しないのでポケモンを変更する必要もない。ZUKAN_INFO_ChangePoke( work->info_wk_s, monsno, formno, sex, rare, personal_rnd, b_get_flag );

      //// 直ちに黒にする
      //ZKNDTL_COMMON_FadeSetBlackImmediately( ZKNDTL_COMMON_FADE_DISP_S, work->fade_wk_s );
      // 直ちに何も表示しないとき用の背景のみを表示する
      ZUKAN_INFO_DisplayBackNone( work->info_wk_s );
    }
    else
    {
      ZUKAN_INFO_ChangePokeAndLang( work->info_wk_s, monsno, formno, sex, rare, personal_rnd, b_get_flag, work->lang );
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
 
  if( work->lang == ZUKAN_INFO_LANG_NONE )
  {
    u16  monsno;
    u16  formno;
    u16  sex;
    u16  rare;
    u32  personal_rnd;
    BOOL b_get_flag;
    BOOL lang_exist[ZUKAN_INFO_LANG_MAX];

    // 現在表示するポケモンの情報を得る
    Zukan_Detail_Info_GetCurrPokeInfo(
                  param, work, cmn,
                  &monsno,
                  &formno,
                  &sex,
                  &rare,
                  &personal_rnd,
                  &b_get_flag,
                  lang_exist );

#ifdef DEBUG_KAWADA
    if( monsno == 202 ) b_get_flag = FALSE;
#endif

    ZUKAN_INFO_ChangePokeAndLang( work->info_wk_s, monsno, formno, sex, rare, personal_rnd, b_get_flag, lang );  //何も表示していないときはポケモンを更新していないので、ポケモンを変更する必要がある。
    
    //// 直ちに見えるようにする
    //ZKNDTL_COMMON_FadeSetColorlessImmediately( ZKNDTL_COMMON_FADE_DISP_S, work->fade_wk_s );
    // 直ちに通常の表示にする
    ZUKAN_INFO_DisplayNormal( work->info_wk_s );
  }
  else
  {
    ZUKAN_INFO_ChangeLang( work->info_wk_s, lang );
  }

  // 言語ボタンのパレットアニメ
  Zukan_Detail_Info_AnimeExitLang( param, work, cmn );

  work->lang = lang;
  
  // 言語ボタンのパレットアニメ
  Zukan_Detail_Info_AnimeInitLang( param, work, cmn );
}

//-------------------------------------
/// 言語ボタン
//=====================================
static void Zukan_Detail_Info_CreateLangButton( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_TOUCHBAR_WORK* touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);

  ZUKAN_DETAIL_GRAPHIC_WORK* graphic = ZKNDTL_COMMON_GetGraphic(cmn);
  GFL_CLUNIT* clunit = ZUKAN_DETAIL_GRAPHIC_GetClunit(graphic);

  // メイン or サブ
  CLSYS_DRAW_TYPE draw_type = CLSYS_DRAW_MAIN;
  CLSYS_DEFREND_TYPE def_type = CLSYS_DEFREND_MAIN;

  // リソース読み込み
  {
    ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, param->heap_id );
/*
タッチバーで読み込んだパレットを流用するので、ここでは読み込まない
    work->lang_ncl = GFL_CLGRP_PLTT_RegisterEx(
                            handle,
                            NARC_zukan_gra_info_info_obj_NCLR,
                            draw_type,
                            OBJ_PAL_POS_M_LANG_BTN * 0x20, 0, OBJ_PAL_NUM_M_LANG_BTN,
                            param->heap_id );
*/
    work->lang_ncl = ZUKAN_DETAIL_TOUCHBAR_GetCustomIconPlttRegIdx( touchbar );  // タッチバーで読み込んだパレットを流用するので、ここでは読み込まない
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
      work->lang_btn[i].lang_code     = lang_button_rect[i][8];
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
/*
タッチバーで読み込んだパレットを流用するので、ここでは読み込まない
    GFL_CLGRP_PLTT_Release( work->lang_ncl );
*/
    GFL_CLGRP_CGR_Release( work->lang_ncg );
    GFL_CLGRP_CELLANIM_Release( work->lang_nce );
  }
}
static void Zukan_Detail_Info_MainLangButton( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_TOUCHBAR_WORK* touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);
  
  if( work->lang_btn_push != ZUKAN_INFO_LANG_NONE )
  {
    u8 i = work->lang_btn_push;

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

        work->lang_btn_push = ZUKAN_INFO_LANG_NONE;
        ZUKAN_DETAIL_TOUCHBAR_SetUserActiveWhole( touchbar, TRUE );
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
  
  work->lang_btn_push = ZUKAN_INFO_LANG_NONE;
}

//-------------------------------------
/// 現在表示するポケモンの情報を得る
//=====================================
static void Zukan_Detail_Info_GetCurrPokeInfo(
                ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                u16*  a_monsno,
                u16*  a_formno,
                u16*  a_sex,
                u16*  a_rare,
                u32*  a_personal_rnd,
                BOOL* a_b_get_flag,
                BOOL* a_lang_exist )  // a_lang_exist[ZUKAN_INFO_LANG_MAX]
{
  ZUKAN_SAVEDATA* zkn_sv = GAMEDATA_GetZukanSave( ZKNDTL_COMMON_GetGamedata(cmn) );

  u16  monsno = ZKNDTL_COMMON_GetCurrPoke(cmn);

  u32  formno;
  u32  sex;
  BOOL rare;
  u32  personal_rnd = 0;

  BOOL b_get_flag;
  BOOL lang_exist[ZUKAN_INFO_LANG_MAX];

  // 現在表示する姿
  ZUKANSAVE_GetDrawData(  zkn_sv, monsno, &sex, &rare, &formno, param->heap_id );
  if( monsno == MONSNO_PATTIIRU )  // personal_rndはmonsno==MONSNO_PATTIIRUのときのみ使用する
  {
    personal_rnd = ZUKANSAVE_GetPokeRandomFlag( zkn_sv, ZUKANSAVE_RANDOM_PACHI );
  }

  b_get_flag = ZUKANSAVE_GetPokeGetFlag( zkn_sv, monsno );

  {
    u8 i;
    for( i=0; i<ZUKAN_INFO_LANG_MAX; i++ )
    {
      lang_exist[i] = ZUKANSAVE_GetTextVersionUpFlag( zkn_sv, monsno, work->lang_btn[i].lang_code );  // カントリーコードはpm_version.hのLANG_JAPAN, LANG_ENGLISHなどの言語コード指定の値
    }
  }
 
#if 0 
  // 全言語チェック用
  {
    u8 i;
    for( i=0; i<ZUKAN_INFO_LANG_MAX; i++ )
    {
      lang_exist[i] = TRUE;
    }
  }
#endif

  *a_monsno        = monsno;
  *a_formno        = (u16)formno;
  *a_sex           = (u16)sex;
  *a_rare          = (u16)rare;
  *a_personal_rnd  = personal_rnd;
  *a_b_get_flag      = b_get_flag;
  {
    u8 i;
    for( i=0; i<ZUKAN_INFO_LANG_MAX; i++ )
    {
      a_lang_exist[i] = lang_exist[i];
    }
  }
}

//-------------------------------------
/// 言語ボタンのパレットアニメ
//=====================================
static void Zukan_Detail_Info_AnimeBaseInitLang( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  NNSG2dPaletteData* pal_data;
  void* buf = GFL_ARC_UTIL_LoadPalette( ARCID_ZUKAN_GRA, NARC_zukan_gra_info_info_obj_NCLR, &pal_data, param->heap_id );
  u16* raw_data = pal_data->pRawData;
  //GFL_STD_MemCopy( &raw_data[OBJ_PAL_OFFSET_LANG_ANIME_NONE*0x10], work->pal_anime_lang_default, 0x20 );
  GFL_STD_MemCopy( &raw_data[RES_PAL_POS_LANG_ANIME_START*0x10], work->pal_anime_lang_start, 0x20 );
  GFL_STD_MemCopy( &raw_data[RES_PAL_POS_LANG_ANIME_END*0x10], work->pal_anime_lang_end, 0x20 );
  GFL_HEAP_FreeMemory( buf );
  work->pal_anime_lang_count = 0;

  // 初期化
  {
    GFL_STD_MemCopy( work->pal_anime_lang_start, work->pal_anime_lang_now, 0x20 );

    NNS_GfdRegisterNewVramTransferTask(
        NNS_GFD_DST_2D_OBJ_PLTT_MAIN,
/*
タッチバーで読み込んだパレットを流用するので、ここでは読み込まない
        ( OBJ_PAL_POS_M_LANG_BTN + OBJ_PAL_OFFSET_LANG_ANIME_EXEC ) * 0x20,
*/
        OBJ_PAL_POS_M_LANG_ANIME_EXEC * 0x20,
        work->pal_anime_lang_now,
        0x20 );
  }
}
static void Zukan_Detail_Info_AnimeBaseExitLang( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 何もしない
}
static void Zukan_Detail_Info_AnimeInitLang( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  if( work->lang != ZUKAN_INFO_LANG_NONE )
  {
    // pal_anime_lang_defaultを割り当てると押しアニメしなくなってしまったので、pal_anime_lang_pltt_offs_reqを使うことにした。
    /*
    {
      // 押しアニメしているときは、defaultの色にしておく
      GFL_STD_MemCopy( work->pal_anime_lang_default, work->pal_anime_lang_now, 0x20 );
      NNS_GfdRegisterNewVramTransferTask(
          NNS_GFD_DST_2D_OBJ_PLTT_MAIN,
          ( OBJ_PAL_POS_M_LANG_BTN + OBJ_PAL_OFFSET_LANG_ANIME_EXEC ) * 0x20,
          work->pal_anime_lang_now,
          0x20 );
    }
    */
    //GFL_CLACT_WK_SetPlttOffs( work->lang_btn[work->lang].clwk, OBJ_PAL_OFFSET_LANG_ANIME_EXEC, CLWK_PLTTOFFS_MODE_OAM_COLOR );
    work->pal_anime_lang_count = 0;
    work->pal_anime_lang_pltt_offs_req = TRUE;  // 押しアニメしているときは、パレットアニメをしないように、パレットアニメの開始を遅らせる
  }
}
static void Zukan_Detail_Info_AnimeExitLang( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  if( work->lang != ZUKAN_INFO_LANG_NONE )
  {
/*
タッチバーで読み込んだパレットを流用するので、ここでは読み込まない 
    GFL_CLACT_WK_SetPlttOffs( work->lang_btn[work->lang].clwk, OBJ_PAL_OFFSET_LANG_ANIME_NONE, CLWK_PLTTOFFS_MODE_OAM_COLOR );
*/
    //GFL_CLACT_WK_SetPlttOffs( work->lang_btn[work->lang].clwk, OBJ_PAL_POS_M_LANG_ANIME_NONE, CLWK_PLTTOFFS_MODE_PLTT_TOP );パレットを直で変えるってできないっぽい
    GFL_CLACT_WK_SetPlttOffs( work->lang_btn[work->lang].clwk, OBJ_PAL_OFFSET_LANG_ANIME_NONE, CLWK_PLTTOFFS_MODE_OAM_COLOR );
    work->pal_anime_lang_pltt_offs_req = FALSE;
  }
}
static void Zukan_Detail_Info_AnimeMainLang( ZUKAN_DETAIL_INFO_PARAM* param, ZUKAN_DETAIL_INFO_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u8 i;
  fx16 cos;

  if( work->pal_anime_lang_pltt_offs_req )  // 押しアニメしているときは、パレットアニメをしないように、パレットアニメの開始を遅らせる
  {
    if( work->lang != ZUKAN_INFO_LANG_NONE )
    {
/*
タッチバーで読み込んだパレットを流用するので、ここでは読み込まない 
      GFL_CLACT_WK_SetPlttOffs( work->lang_btn[work->lang].clwk, OBJ_PAL_OFFSET_LANG_ANIME_EXEC, CLWK_PLTTOFFS_MODE_OAM_COLOR );
*/
      //GFL_CLACT_WK_SetPlttOffs( work->lang_btn[work->lang].clwk, OBJ_PAL_POS_M_LANG_ANIME_EXEC, CLWK_PLTTOFFS_MODE_PLTT_TOP );パレットを直で変えるってできないっぽい
      GFL_CLACT_WK_SetPlttOffs( work->lang_btn[work->lang].clwk, OBJ_PAL_OFFSET_LANG_ANIME_EXEC, CLWK_PLTTOFFS_MODE_OAM_COLOR );
    }
    work->pal_anime_lang_pltt_offs_req = FALSE;
  }

  if( work->pal_anime_lang_count + LANG_ANIME_ADD >= 0x10000 )
  {
    work->pal_anime_lang_count = work->pal_anime_lang_count + LANG_ANIME_ADD - 0x10000;
  }
  else
  {
    work->pal_anime_lang_count = work->pal_anime_lang_count + LANG_ANIME_ADD;
  }
  cos = ( FX_CosIdx( work->pal_anime_lang_count ) + FX16_ONE ) / 2;  // 0<= <=1にしておく

  for( i=0; i<0x10; i++ )
  {
    u8 s_r = ( work->pal_anime_lang_start[i] & GX_RGB_R_MASK ) >> GX_RGB_R_SHIFT;
    u8 s_g = ( work->pal_anime_lang_start[i] & GX_RGB_G_MASK ) >> GX_RGB_G_SHIFT;
    u8 s_b = ( work->pal_anime_lang_start[i] & GX_RGB_B_MASK ) >> GX_RGB_B_SHIFT;
    u8 e_r = ( work->pal_anime_lang_end[i]   & GX_RGB_R_MASK ) >> GX_RGB_R_SHIFT;
    u8 e_g = ( work->pal_anime_lang_end[i]   & GX_RGB_G_MASK ) >> GX_RGB_G_SHIFT;
    u8 e_b = ( work->pal_anime_lang_end[i]   & GX_RGB_B_MASK ) >> GX_RGB_B_SHIFT;

    u8 r = s_r + (((e_r-s_r)*cos)>>FX16_SHIFT);
    u8 g = s_g + (((e_g-s_g)*cos)>>FX16_SHIFT);
    u8 b = s_b + (((e_b-s_b)*cos)>>FX16_SHIFT);

    work->pal_anime_lang_now[i] = GX_RGB( r, g, b );
  }

  {
    NNS_GfdRegisterNewVramTransferTask(
        NNS_GFD_DST_2D_OBJ_PLTT_MAIN,
/*
タッチバーで読み込んだパレットを流用するので、ここでは読み込まない
        ( OBJ_PAL_POS_M_LANG_BTN + OBJ_PAL_OFFSET_LANG_ANIME_EXEC ) * 0x20,
*/
        OBJ_PAL_POS_M_LANG_ANIME_EXEC * 0x20,
        work->pal_anime_lang_now,
        0x20 );
  }
}

