//=============================================================================
/**
 *
 *  @file   box_search.c
 *  @brief  新規アプリ用テンプレート
 *  @author Toru=Nagihashi / genya hosaka
 *  @data   2009.09.29
 *
 */
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/wipe.h"

// 簡易会話表示システム
#include "system/pms_draw.h"

//テクスチャをOAMに転送
#include "system/nsbtx_to_clwk.h"

//FONT
#include "print/gf_font.h"
#include "font/font.naix"

//PRINT_QUE
#include "print/printsys.h"

#include "print/wordset.h"

//INFOWIN
#include "infowin/infowin.h"

//描画設定
#include "box_search_graphic.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"

//マップモデルのnsbtx
#include "fieldmap/fldmmdl_mdlres.naix"

//タッチバー
#include "ui/touchbar.h"

//ポケパラ、ポケモンNO
#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

//ポケアイコン
#include "pokeicon/pokeicon.h"

//MCSS
#include "system/mcss.h"
#include "system/mcss_tool.h"

//タスクメニュー
#include "app/app_taskmenu.h"

//どうぐアイコン
#include "item/item.h"
#include "item_icon.naix"

//ポケモンBG,OBJ読みこみ
#include "system/poke2dgra.h"

//アプリ共通素材
#include "app/app_menu_common.h"

// サウンド
#include "sound/pm_sndsys.h"

//アーカイブ
#include "arc_def.h"

//外部公開
#include "box_search.h"


//@TODO BG読み込み とりあえずマイクテストのリソース
#include "message.naix"
//#include "mictest.naix" // アーカイブ
#include "box_gra.naix"
#include "msg/msg_mictest.h"  // GMM
#include "townmap_gra.naix"   // タッチバーカスタムボタン用サンプルにタウンマップリソース

//=============================================================================
// 下記defineをコメントアウトすると、機能を取り除けます
//=============================================================================
#define BOX_SEARCH_TOUCHBAR
//#define BOX_SEARCH_TASKMENU
//#define BOX_SEARCH_POKE_ICON     // ポケアイコン(ポケアイコン用アイテムアイコン込み)
//#define BOX_SEARCH_OAM_MAPMODEL  // マップモデルをOAMで表示    //nsbtx_to_clwkをオーバーレイに移したのでコメントアウトにしました 10/06/11 nagihashi
//#define BOX_SEARCH_PMSDRAW       // 簡易会話表示

FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
 *                定数定義
 */
//=============================================================================
enum
{ 
  BOX_SEARCH_HEAP_SIZE = 0x30000,  ///< ヒープサイズ
};

enum
{
  SEQ_FADEIN_WAIT=0,
  SEQ_MAIN,
  SEQ_FADEOUT,
  SEQ_FADEOUT_WAIT,
  
};

#define BOX_SEARCH_BMPWIN_NUM (14)
//-------------------------------------
/// フレーム
//=====================================
enum
{ 
  BG_FRAME_BAR_M  = GFL_BG_FRAME1_M,
  BG_FRAME_BUTTON_M = GFL_BG_FRAME2_M,
  BG_FRAME_BACK_M = GFL_BG_FRAME3_M,
  BG_FRAME_BACK_S = GFL_BG_FRAME2_S,
  BG_FRAME_TEXT_S = GFL_BG_FRAME0_S, 
};
//-------------------------------------
/// パレット
//=====================================
enum
{ 
  //メインBG
  PLTID_BG_BACK_M       = 0,
  PLTID_BG_POKE_M       = 1,
  PLTID_BG_TASKMENU_M   = 11,
  PLTID_BG_TOUCHBAR_M   = 13,
  PLTID_BG_INFOWIN_M    = 15,
  //サブBG
  PLTID_BG_BACK_S       = 0,

  //メインOBJ
  PLTID_OBJ_TOUCHBAR_M  = 0, // 3本使用
  PLTID_OBJ_TYPEICON_M  = 3, // 3本使用
  PLTID_OBJ_OAM_MAPMODEL_M = 6, // 1本使用
  PLTID_OBJ_POKEICON_M = 7,     // 3本使用
  PLTID_OBJ_POKEITEM_M = 10,    // 1本使用
  PLTID_OBJ_ITEMICON_M = 11,
  PLTID_OBJ_POKE_M = 12,
  PLTID_OBJ_BALLICON_M = 13, // 1本使用
  PLTID_OBJ_TOWNMAP_M = 14,   
  //サブOBJ
  PLTID_OBJ_PMS_DRAW = 0, // 5本使用
};

//=============================================================================
/**
 *                構造体定義
 */
//=============================================================================
//--------------------------------------------------------------
/// BG管理ワーク
//==============================================================
typedef struct 
{
  int dummy;
} BOX_SEARCH_BG_WORK;


#ifdef BOX_SEARCH_PMSDRAW
#define BOX_SEARCH_PMSDRAW_NUM (3) ///< 簡易会話の個数
#endif // BOX_SEARCH_PMSDRAW 

//--------------------------------------------------------------
/// メインワーク
//==============================================================
typedef struct 
{
  HEAPID heapID;

  BOX_SEARCH_BG_WORK       wk_bg;

  //描画設定
  BOX_SEARCH_GRAPHIC_WORK  *graphic;

#ifdef BOX_SEARCH_TOUCHBAR
  //タッチバー
  TOUCHBAR_WORK             *touchbar;
  //以下カスタムボタン使用する場合のサンプルリソース
  u32                       ncg_btn;
  u32                       ncl_btn;
  u32                       nce_btn;
#endif //BOX_SEARCH_TOUCHBAR


  //フォント
  GFL_FONT                  *font;

  //プリントキュー
  PRINT_QUE                 *print_que;
  GFL_MSGDATA               *msg;

#ifdef BOX_SEARCH_TASKMENU
  //タスクメニュー
  APP_TASKMENU_RES          *menu_res;
  APP_TASKMENU_WORK         *menu;
#endif //BOX_SEARCH_TASKMENU



#ifdef BOX_SEARCH_POKE_ICON
  // ポケアイコン
  UI_EASY_CLWK_RES      clres_poke_icon;
  GFL_CLWK                  *clwk_poke_icon;
  // ポケアイコン用アイテムアイコン
  UI_EASY_CLWK_RES      clres_poke_item;
  GFL_CLWK                  *clwk_poke_item;
#endif //BOX_SEARCH_POKE_ICON

#ifdef BOX_SEARCH_OAM_MAPMODEL
  // OAMマップモデル
  UI_EASY_CLWK_RES      clres_oam_mmdl;
  GFL_CLWK                  *clwk_oam_mmdl;
#endif //BOX_SEARCH_OAM_MAPMODEL



#ifdef  BOX_SEARCH_PRINT_TOOL
  //プリントユーティリティ
  PRINT_UTIL                print_util;
  u32                       seq;
#endif  //BOX_SEARCH_PRINT_TOOL

#ifdef BOX_SEARCH_PMSDRAW
  GFL_BMPWIN*               pms_win[ BOX_SEARCH_PMSDRAW_NUM ];
  PMS_DRAW_WORK*            pms_draw;
#endif //BOX_SEARCH_PMSDRAW

  GFL_BMPWIN *bmp_win[BOX_SEARCH_BMPWIN_NUM];
  int subseq;
  int next_subseq;

} BOX_SEARCH_MAIN_WORK;


//=============================================================================
/**
 *              データ定義
 */
//=============================================================================


//=============================================================================
/**
 *              プロトタイプ宣言
 */
//=============================================================================
//-------------------------------------
/// PROC
//=====================================
static GFL_PROC_RESULT BOXSEARCHProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT BOXSEARCHProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT BOXSEARCHProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
//-------------------------------------
/// 汎用処理ユーティリティ
//=====================================
static void BOXSEARCH_BG_LoadResource( BOX_SEARCH_BG_WORK* wk, HEAPID heapID );


#ifdef BOX_SEARCH_TOUCHBAR
//-------------------------------------
/// タッチバー
//=====================================
static TOUCHBAR_WORK * BOXSEARCH_TOUCHBAR_Init( GFL_CLUNIT *clunit, HEAPID heapID );
static void BOXSEARCH_TOUCHBAR_Exit( TOUCHBAR_WORK  *touchbar );
static void BOXSEARCH_TOUCHBAR_Main( TOUCHBAR_WORK  *touchbar );
//以下カスタムボタン使用サンプル用
static TOUCHBAR_WORK * BOXSEARCH_TOUCHBAR_InitEx( BOX_SEARCH_MAIN_WORK *wk, GFL_CLUNIT *clunit, HEAPID heapID );
static void BOXSEARCH_TOUCHBAR_ExitEx( BOX_SEARCH_MAIN_WORK *wk );
#endif //BOX_SEARCH_TOUCHBAR


#ifdef BOX_SEARCH_TASKMENU
//-------------------------------------
/// リストシステムはい、いいえ
//=====================================
static APP_TASKMENU_WORK * BOXSEARCH_TASKMENU_Init( APP_TASKMENU_RES *menu_res, GFL_MSGDATA *msg, HEAPID heapID );
static void BOXSEARCH_TASKMENU_Exit( APP_TASKMENU_WORK *menu );
static void BOXSEARCH_TASKMENU_Main( APP_TASKMENU_WORK *menu );
#endif //BOX_SEARCH_TASKMENU



#ifdef BOX_SEARCH_POKE_ICON
//-------------------------------------
/// ポケアイコン
//=====================================
static void BOXSEARCH_POKE_ICON_CreateCLWK( BOX_SEARCH_MAIN_WORK* wk, u32 mons, u32 form_no, u32 sex, BOOL egg, GFL_CLUNIT* unit, HEAPID heapID );
static void BOXSEARCH_POKE_ICON_DeleteCLWK( BOX_SEARCH_MAIN_WORK* wk );
#endif //BOX_SEARCH_POKE_ICON

#ifdef BOX_SEARCH_OAM_MAPMODEL
//-------------------------------------
/// OAMでマップモデル表示
//=====================================
static void BOXSEARCH_OAM_MAPMODEL_CreateCLWK( BOX_SEARCH_MAIN_WORK *wk, u16 tex_idx, u8 ptn_ofs, GFL_CLUNIT *unit, HEAPID heapID );
static void BOXSEARCH_OAM_MAPMODEL_DeleteCLWK( BOX_SEARCH_MAIN_WORK* wk );
#endif //BOX_SEARCH_OAM_MAPMODEL


#ifdef BOX_SEARCH_PMSDRAW
//-------------------------------------
/// 簡易会話表示
//=====================================
static void BOXSEARCH_PMSDRAW_Init( BOX_SEARCH_MAIN_WORK* wk );
static void BOXSEARCH_PMSDRAW_Exit( BOX_SEARCH_MAIN_WORK* wk );
static void BOXSEARCH_PMSDRAW_Proc( BOX_SEARCH_MAIN_WORK* wk );
#endif // BOX_SEARCH_PMSDRAW


static int Subseq_Start(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_Return(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectPokemonInit(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectPokemon1(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectPokemon2Init(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectPokemon2(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectPokemon3Init(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectPokemon3(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectSeikakuInit(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectSeikaku(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectTokuseiInit(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectTokusei1(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectTokusei2Init(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectTokusei2(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectTokusei3Init(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectTokusei3(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectSexInit(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectSex(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectItemHavaInit(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectItemHava(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectMarkInit(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectMark(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SearchOn(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SearchOff(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SearchReset(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_AnimeWait(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SearchExit(BOX_SEARCH_MAIN_WORK *wk, int seq);

//=============================================================================
// サブシーケンスENUM定義
//=============================================================================
enum{
  SUBSEQ_START=0,             //
  SUBSEQ_RETURN,              //
  SUBSEQ_SELECTPOKEMONINIT,   //
  SUBSEQ_SELECTPOKEMON1,      //
  SUBSEQ_SELECTPOKEMON2INIT,  //
  SUBSEQ_SELECTPOKEMON2,      //
  SUBSEQ_SELECTPOKEMON3INIT,  //
  SUBSEQ_SELECTPOKEMON3,      //
  SUBSEQ_SELECTSEIKAKUINIT,   //
  SUBSEQ_SELECTSEIKAKU,       //
  SUBSEQ_SELECTTOKUSEIINIT,   //
  SUBSEQ_SELECTTOKUSEI1,      //
  SUBSEQ_SELECTTOKUSEI2INIT,  //
  SUBSEQ_SELECTTOKUSEI2,      //
  SUBSEQ_SELECTTOKUSEI3INIT,  //
  SUBSEQ_SELECTTOKUSEI3,      //
  SUBSEQ_SELECTSEXINIT,       //
  SUBSEQ_SELECTSEX,           //
  SUBSEQ_SELECTITEMHAVAINIT,  //
  SUBSEQ_SELECTITEMHAVA,      //
  SUBSEQ_SELECTMARKINIT,      //
  SUBSEQ_SELECTMARK,          //
  SUBSEQ_SEARCHON,            //
  SUBSEQ_SEARCHOFF,           //
  SUBSEQ_SEARCHRESET,         //
  SUBSEQ_ANIMEWAIT,
  SUBSEQ_SEARCHEXIT,          //
};

//=============================================================================
/**
 *                サブシーケンス関数テーブル
 */
//=============================================================================
// 関数配列定義
static int (* const FuncTable[])(BOX_SEARCH_MAIN_WORK *wk, int seq)={
  Subseq_Start,
  Subseq_Return,
  Subseq_SelectPokemonInit,
  Subseq_SelectPokemon1,
  Subseq_SelectPokemon2Init,
  Subseq_SelectPokemon2,
  Subseq_SelectPokemon3Init,
  Subseq_SelectPokemon3,
  Subseq_SelectSeikakuInit,
  Subseq_SelectSeikaku,
  Subseq_SelectTokuseiInit,
  Subseq_SelectTokusei1,
  Subseq_SelectTokusei2Init,
  Subseq_SelectTokusei2,
  Subseq_SelectTokusei3Init,
  Subseq_SelectTokusei3,
  Subseq_SelectSexInit,
  Subseq_SelectSex,
  Subseq_SelectItemHavaInit,
  Subseq_SelectItemHava,
  Subseq_SelectMarkInit,
  Subseq_SelectMark,
  Subseq_SearchOn,
  Subseq_SearchOff,
  Subseq_SearchReset,
  Subseq_AnimeWait,
  Subseq_SearchExit,
};


//=============================================================================
/**
 *                外部公開
 */
//=============================================================================
const GFL_PROC_DATA BOXSEARCHProcData = 
{
  BOXSEARCHProc_Init,
  BOXSEARCHProc_Main,
  BOXSEARCHProc_Exit,
};
//=============================================================================
/**
 *                PROC
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *  @brief  PROC 初期化処理
 *
 *  @param  GFL_PROC *procプロセスシステム
 *  @param  *seq          シーケンス
 *  @param  *pwk          BOX_SEARCH_PARAM
 *  @param  *mywk         PROCワーク
 *
 *  @retval 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BOXSEARCHProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
  BOX_SEARCH_MAIN_WORK *wk;
  BOX_SEARCH_PARAM *param;

  //オーバーレイ読み込み
  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
  
  //引数取得
  param = pwk;

  //ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_BOX_SEARCH, BOX_SEARCH_HEAP_SIZE );
  wk = GFL_PROC_AllocWork( proc, sizeof(BOX_SEARCH_MAIN_WORK), HEAPID_BOX_SEARCH );
  GFL_STD_MemClear( wk, sizeof(BOX_SEARCH_MAIN_WORK) );

  // 初期化
  wk->heapID = HEAPID_BOX_SEARCH;
  
  //描画設定初期化
  wk->graphic = BOX_SEARCH_GRAPHIC_Init( GX_DISP_SELECT_SUB_MAIN, wk->heapID );

  //フォント作成
  wk->font      = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
                        GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

  //メッセージ
  wk->msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
      NARC_message_mictest_dat, wk->heapID );

  //PRINT_QUE作成
  wk->print_que   = PRINTSYS_QUE_Create( wk->heapID );

  //BGリソース読み込み
  BOXSEARCH_BG_LoadResource( &wk->wk_bg, wk->heapID );


#ifdef BOX_SEARCH_TOUCHBAR
  //タッチバーの初期化
  { 
    GFL_CLUNIT  *clunit = BOX_SEARCH_GRAPHIC_GetClunit( wk->graphic );
    wk->touchbar  = BOXSEARCH_TOUCHBAR_InitEx( wk, clunit, wk->heapID );
  }
#endif //BOX_SEARCH_TOUCHBAR


#ifdef BOX_SEARCH_TASKMENU
  //TASKMENUリソース読み込み＆初期化
  wk->menu_res  = APP_TASKMENU_RES_Create( BG_FRAME_BAR_M, PLTID_BG_TASKMENU_M, wk->font, wk->print_que, wk->heapID );
  wk->menu      = BOXSEARCH_TASKMENU_Init( wk->menu_res, wk->msg, wk->heapID );
#endif //BOX_SEARCH_TASKMENU


#ifdef BOX_SEARCH_OAM_MAPMODEL
  //OAMマップモデルの読み込み
  {
    // 主人公は現状 0=後ろ, 1=後ろアニメ1, 2=後ろアニメ2, 3=正面....
    // トレーナーなどは違う可能性があるので注意。
    u16 ptn_ofs = 3;
    GFL_CLUNIT  *clunit = BOX_SEARCH_GRAPHIC_GetClunit( wk->graphic );

    BOXSEARCH_OAM_MAPMODEL_CreateCLWK( wk, NARC_fldmmdl_mdlres_hero_nsbtx, ptn_ofs, clunit, wk->heapID );
  }
#endif //BOX_SEARCH_OAM_MAPMODEL

#ifdef BOX_SEARCH_POKE_ICON
  // ポケアイコンの読み込み
  {
    GFL_CLUNIT  *clunit = BOX_SEARCH_GRAPHIC_GetClunit( wk->graphic );
    BOXSEARCH_POKE_ICON_CreateCLWK( wk, MONSNO_HUSIGIDANE, 0, 0, FALSE, clunit, wk->heapID );
  }
#endif //BOX_SEARCH_POKE_ICON

  // 文字列描画用領域確保
  //SearchBmpwinInit( wk );


#ifdef BOX_SEARCH_PMSDRAW  
  BOXSEARCH_PMSDRAW_Init( wk );
#endif //BOX_SEARCH_PMSDRAW

  //@todo フェードシーケンスがないので
//  GX_SetMasterBrightness(0);
//  GXS_SetMasterBrightness(0);
  WIPE_SYS_Start(
    WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
    WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapID );


  return GFL_PROC_RES_FINISH;
}
//-----------------------------------------------------------------------------
/**
 *  @brief  PROC 終了処理
 *
 *  @param  GFL_PROC *procプロセスシステム
 *  @param  *seq          シーケンス
 *  @param  *pwk          BOX_SEARCH_PARAM
 *  @param  *mywk         PROCワーク
 *
 *  @retval 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BOXSEARCHProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
  BOX_SEARCH_MAIN_WORK* wk = mywk;





#ifdef BOX_SEARCH_OAM_MAPMODEL
  //OAMマップモデルの破棄
  BOXSEARCH_OAM_MAPMODEL_DeleteCLWK( wk );
#endif //BOX_SEARCH_OAM_MAPMODEL

#ifdef BOX_SEARCH_POKE_ICON
   BOXSEARCH_POKE_ICON_DeleteCLWK( wk );
#endif //BOX_SEARCH_POKE_ICON

#ifdef BOX_SEARCH_TASKMENU
  //TASKMENUシステム＆リソース破棄
  BOXSEARCH_TASKMENU_Exit( wk->menu );
  APP_TASKMENU_RES_Delete( wk->menu_res );  
#endif //BOX_SEARCH_TASKMENU


#ifdef BOX_SEARCH_TOUCHBAR
  //タッチバー
  BOXSEARCH_TOUCHBAR_ExitEx( wk );
#endif //BOX_SEARCH_TOUCHBAR


#ifdef BOX_SEARCH_PMSDRAW
  BOXSEARCH_PMSDRAW_Exit( wk );
#endif //BOX_SEARCH_PMSDRAW

  //メッセージ破棄
  GFL_MSG_Delete( wk->msg );

  //PRINT_QUE
  PRINTSYS_QUE_Delete( wk->print_que );

  //FONT
  GFL_FONT_Delete( wk->font );

  //描画設定破棄
  BOX_SEARCH_GRAPHIC_Exit( wk->graphic );

  //PROC用メモリ解放
  {
    HEAPID heapID = wk->heapID;
    GFL_PROC_FreeWork( proc );
    GFL_HEAP_DeleteHeap( heapID );
  }

  //オーバーレイ破棄
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));

  return GFL_PROC_RES_FINISH;
}
//-----------------------------------------------------------------------------
/**
 *  @brief  PROC 主処理
 *
 *  @param  GFL_PROC *procプロセスシステム
 *  @param  *seq          シーケンス
 *  @param  *pwk          BOX_SEARCH_PARAM
 *  @param  *mywk         PROCワーク
 *
 *  @retval 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BOXSEARCHProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
  BOX_SEARCH_MAIN_WORK* wk = mywk;

  switch(*seq){
  case SEQ_FADEIN_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE ){
      *seq = SEQ_MAIN;
    }
    break;
  case SEQ_MAIN:
    // デバッグボタンでアプリ終了
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START )
    {
      *seq = SEQ_FADEOUT;;
    }
    wk->subseq = FuncTable[wk->subseq](wk,wk->subseq);
    if(wk->subseq==SUBSEQ_SEARCHEXIT){
      *seq = SEQ_FADEOUT;;
    }
//    KeyControl( wk );
//    BGControl( wk );
    break;
  case SEQ_FADEOUT:
    WIPE_SYS_Start(
      WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
      WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapID );
    *seq = SEQ_FADEOUT_WAIT;
    break;
  case SEQ_FADEOUT_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE ){
      return GFL_PROC_RES_FINISH;
    }
    break;
  }


#ifdef BOX_SEARCH_TOUCHBAR
  //タッチバーメイン処理
  BOXSEARCH_TOUCHBAR_Main( wk->touchbar );
#endif //BOX_SEARCH_TOUCHBAR


#ifdef BOX_SEARCH_TASKMENU
  //タスクメニューメイン処理
  BOXSEARCH_TASKMENU_Main( wk->menu );
#endif //BOX_SEARCH_TASKMENU

  //PRINT_QUE
  PRINTSYS_QUE_Main( wk->print_que );

#ifdef BOX_SEARCH_PMSDRAW
  BOXSEARCH_PMSDRAW_Proc( wk );
#endif //BOX_SEARCH_PMSDRAW

  //2D描画
  BOX_SEARCH_GRAPHIC_2D_Draw( wk->graphic );

  //3D描画
  BOX_SEARCH_GRAPHIC_3D_StartDraw( wk->graphic );

  BOX_SEARCH_GRAPHIC_3D_EndDraw( wk->graphic );

  return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *                static関数
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *  @brief  BG管理モジュール リソース読み込み
 *
 *  @param  BOX_SEARCH_BG_WORK* wk BG管理ワーク
 *  @param  heapID  ヒープID 
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static void BOXSEARCH_BG_LoadResource( BOX_SEARCH_BG_WORK* wk, HEAPID heapID )
{
  //@TODO とりあえずマイクテストのリソース
  //@TODO 複数枚ロードできるようにして切り離す？
  enum
  {
    RES_ARC = ARCID_BOX2_GRA,

    RES_PAL_S_SIZE = 0x20,
    RES_PAL_M_SIZE = 0x20*4,

    RES_PAL_S_ID = NARC_box_gra_boxsearch_bgu_NCLR,
    RES_CHR_S_ID = NARC_box_gra_boxsearch_bgu_NCGR,
    RES_SCR_S_ID = NARC_box_gra_boxsearchbase_bgu_NSCR,

    RES_PAL_M_ID = NARC_box_gra_boxsearch_bgd_NCLR,
    RES_CHR_M_ID = NARC_box_gra_boxsearch_bgd_NCGR,
    RES_SCR_M2_ID = NARC_box_gra_boxsearch_buton_bgd_NSCR,
    RES_SCR_M3_ID = NARC_box_gra_boxsearchbase_bgd_1_NSCR,

  };

  ARCHANDLE *handle;

  handle  = GFL_ARC_OpenDataHandle( RES_ARC, heapID );

  // 上下画面ＢＧパレット転送
  GFL_ARCHDL_UTIL_TransVramPalette( handle, RES_PAL_M_ID, PALTYPE_MAIN_BG, PLTID_BG_BACK_M, RES_PAL_M_SIZE, heapID );
  GFL_ARCHDL_UTIL_TransVramPalette( handle, RES_PAL_S_ID, PALTYPE_SUB_BG,  PLTID_BG_BACK_S, RES_PAL_S_SIZE, heapID );
  
  //  ----- SUB -----
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, RES_CHR_S_ID, BG_FRAME_BACK_S, 0, 0, 0, heapID );
  GFL_ARCHDL_UTIL_TransVramScreen(      handle, RES_SCR_S_ID, BG_FRAME_BACK_S, 0, 0, 0, heapID ); 

  //  ----- MAIN -----
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, RES_CHR_M_ID,  BG_FRAME_BACK_M, 0, 0, 0, heapID );
  GFL_ARCHDL_UTIL_TransVramScreen(      handle, RES_SCR_M2_ID, BG_FRAME_BUTTON_M, 0, 0, 0, heapID );   
  GFL_ARCHDL_UTIL_TransVramScreen(      handle, RES_SCR_M3_ID, BG_FRAME_BACK_M, 0, 0, 0, heapID );   

  GFL_ARC_CloseDataHandle( handle );
}






#ifdef BOX_SEARCH_TOUCHBAR
//=============================================================================
/**
 *  TOUCHBAR
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  TOUCHBAR初期化
 *
 *  @param  GFL_CLUNIT *clunit  CLUNIT
 *  @param  heapID              ヒープID
 *
 *  @return TOUCHBAR_WORK
 */
//-----------------------------------------------------------------------------
static TOUCHBAR_WORK * BOXSEARCH_TOUCHBAR_Init( GFL_CLUNIT *clunit, HEAPID heapID )
{ 
  //アイコンの設定
  //数分作る
  TOUCHBAR_ITEM_ICON touchbar_icon_tbl[]  =
  { 
    { 
      TOUCHBAR_ICON_RETURN,
      { TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_Y },
    },
    { 
      TOUCHBAR_ICON_CLOSE,
      { TOUCHBAR_ICON_X_06, TOUCHBAR_ICON_Y },
    },
    { 
      TOUCHBAR_ICON_CHECK,
      { TOUCHBAR_ICON_X_05, TOUCHBAR_ICON_Y_CHECK },
    },
  };

  //設定構造体
  //さきほどの窓情報＋リソース情報をいれる
  TOUCHBAR_SETUP  touchbar_setup;
  GFL_STD_MemClear( &touchbar_setup, sizeof(TOUCHBAR_SETUP) );

  touchbar_setup.p_item   = touchbar_icon_tbl;        //上の窓情報
  touchbar_setup.item_num = NELEMS(touchbar_icon_tbl);//いくつ窓があるか
  touchbar_setup.p_unit   = clunit;                   //OBJ読み込みのためのCLUNIT
  touchbar_setup.bar_frm  = BG_FRAME_BAR_M;           //BG読み込みのためのBG面
  touchbar_setup.bg_plt   = PLTID_BG_TOUCHBAR_M;      //BGﾊﾟﾚｯﾄ
  touchbar_setup.obj_plt  = PLTID_OBJ_TOUCHBAR_M;     //OBJﾊﾟﾚｯﾄ
  touchbar_setup.mapping  = APP_COMMON_MAPPING_128K;  //マッピングモード

  return TOUCHBAR_Init( &touchbar_setup, heapID );
}
//----------------------------------------------------------------------------
/**
 *  @brief  TOUCHBAR破棄
 *
 *  @param  TOUCHBAR_WORK *touchbar タッチバー
 */
//-----------------------------------------------------------------------------
static void BOXSEARCH_TOUCHBAR_Exit( TOUCHBAR_WORK  *touchbar )
{ 
  TOUCHBAR_Exit( touchbar );
}

//----------------------------------------------------------------------------
/**
 *  @brief  TOUCHBAR初期化  + カスタムアイコン版
 *
 * @param  wk                   ワーク
 *  @param  GFL_CLUNIT *clunit  CLUNIT
 *  @param  heapID              ヒープID
 *
 *  @return TOUCHBAR_WORK
 */
//-----------------------------------------------------------------------------
static TOUCHBAR_WORK * BOXSEARCH_TOUCHBAR_InitEx( BOX_SEARCH_MAIN_WORK *wk, GFL_CLUNIT *clunit, HEAPID heapID )
{ 

  //リソース読みこみ
  //サンプルとしてタウンマップの拡大縮小アイコンをカスタムボタンに登録する
  { 
    ARCHANDLE *handle;
    //ハンドル
    handle  = GFL_ARC_OpenDataHandle( ARCID_TOWNMAP_GRAPHIC, heapID );
    //リソース読みこみ
    wk->ncg_btn = GFL_CLGRP_CGR_Register( handle,
          NARC_townmap_gra_tmap_obj_d_NCGR, FALSE, CLSYS_DRAW_MAIN, heapID );
    wk->ncl_btn = GFL_CLGRP_PLTT_RegisterEx( handle,
        NARC_townmap_gra_tmap_bg_d_NCLR, CLSYS_DRAW_MAIN, PLTID_OBJ_TOWNMAP_M*0x20,
        2, 2, heapID ); 
    wk->nce_btn = GFL_CLGRP_CELLANIM_Register( handle,
          NARC_townmap_gra_tmap_obj_d_NCER,
          NARC_townmap_gra_tmap_obj_d_NANR, heapID );
  
    GFL_ARC_CloseDataHandle( handle ) ;
  }

  //タッチバーの設定
  { 
  //アイコンの設定
  //数分作る
  TOUCHBAR_SETUP  touchbar_setup;
  TOUCHBAR_ITEM_ICON touchbar_icon_tbl[]  =
  { 
    { 
      TOUCHBAR_ICON_RETURN,
      { TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_Y },
    },
    { 
      TOUCHBAR_ICON_CLOSE,
      { TOUCHBAR_ICON_X_06, TOUCHBAR_ICON_Y },
    },
  };

  //設定構造体
  //さきほどの窓情報＋リソース情報をいれる
  GFL_STD_MemClear( &touchbar_setup, sizeof(TOUCHBAR_SETUP) );

  touchbar_setup.p_item   = touchbar_icon_tbl;        //上の窓情報
  touchbar_setup.item_num = NELEMS(touchbar_icon_tbl);//いくつ窓があるか
  touchbar_setup.p_unit   = clunit;                   //OBJ読み込みのためのCLUNIT
  touchbar_setup.bar_frm  = BG_FRAME_BAR_M;           //BG読み込みのためのBG面
  touchbar_setup.bg_plt   = PLTID_BG_TOUCHBAR_M;      //BGﾊﾟﾚｯﾄ
  touchbar_setup.obj_plt  = PLTID_OBJ_TOUCHBAR_M;     //OBJﾊﾟﾚｯﾄ
  touchbar_setup.mapping  = APP_COMMON_MAPPING_128K;  //マッピングモード

  return TOUCHBAR_Init( &touchbar_setup, heapID );
  }

}
//----------------------------------------------------------------------------
/**
 *  @brief  TOUCHBAR破棄  カスタムボタン版
 *
 *  @param  TOUCHBAR_WORK *touchbar タッチバー
 */
//-----------------------------------------------------------------------------
static void BOXSEARCH_TOUCHBAR_ExitEx( BOX_SEARCH_MAIN_WORK *wk )
{ 
  //タッチバー破棄
  TOUCHBAR_Exit( wk->touchbar );

  //リソース破棄
  { 
    GFL_CLGRP_PLTT_Release( wk->ncl_btn );
    GFL_CLGRP_CGR_Release( wk->ncg_btn );
    GFL_CLGRP_CELLANIM_Release( wk->nce_btn );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  TOUCHBARメイン処理
 *
 *  @param  TOUCHBAR_WORK *touchbar タッチバー
 */
//-----------------------------------------------------------------------------
static void BOXSEARCH_TOUCHBAR_Main( TOUCHBAR_WORK  *touchbar )
{ 
  TOUCHBAR_Main( touchbar );
}
#endif //BOX_SEARCH_TOUCHBAR


#ifdef BOX_SEARCH_TASKMENU
//----------------------------------------------------------------------------
/**
 *  @brief  TASKMENUの初期化
 *
 *  @param  menu_res  リソース
 */
//-----------------------------------------------------------------------------
static APP_TASKMENU_WORK * BOXSEARCH_TASKMENU_Init( APP_TASKMENU_RES *menu_res, GFL_MSGDATA *msg, HEAPID heapID )
{ 
  int i;
  APP_TASKMENU_INITWORK init;
  APP_TASKMENU_ITEMWORK item[3];
  APP_TASKMENU_WORK     *menu;  

  //窓の設定
  for( i = 0; i < NELEMS(item); i++ )
  { 
    item[i].str = GFL_MSG_CreateString( msg, 0 ); //文字列
    item[i].msgColor  = APP_TASKMENU_ITEM_MSGCOLOR; //文字色
    item[i].type      = APP_TASKMENU_WIN_TYPE_NORMAL; //窓の種類
  }

  //初期化
  init.heapId   = heapID;
  init.itemNum  = NELEMS(item);
  init.itemWork = item;
  init.posType  = ATPT_RIGHT_DOWN;
  init.charPosX = 32;
  init.charPosY = 21;
  init.w        = APP_TASKMENU_PLATE_WIDTH;
  init.h        = APP_TASKMENU_PLATE_HEIGHT;

  menu  = APP_TASKMENU_OpenMenu( &init, menu_res );

  //文字列解放
  for( i = 0; i < NELEMS(item); i++ )
  { 
    GFL_STR_DeleteBuffer( item[i].str );
  }

  return menu;

}
//----------------------------------------------------------------------------
/**
 *  @brief  TASKMENUの破棄
 *
 *  @param  APP_TASKMENU_WORK *menu ワーク
 */
//-----------------------------------------------------------------------------
static void BOXSEARCH_TASKMENU_Exit( APP_TASKMENU_WORK *menu )
{ 
  APP_TASKMENU_CloseMenu( menu );
}
//----------------------------------------------------------------------------
/**
 *  @brief  TASKMENUのメイン処理
 *
 *  @param  APP_TASKMENU_WORK *menu ワーク
 */
//-----------------------------------------------------------------------------
static void BOXSEARCH_TASKMENU_Main( APP_TASKMENU_WORK *menu )
{ 
  APP_TASKMENU_UpdateMenu( menu );
}
#endif //BOX_SEARCH_TASKMENU




#ifdef BOX_SEARCH_POKE_ICON
//=============================================================================
/**
 *  POKE_ICON
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *  @brief  ポケアイコン 生成
 *
 *  @param  BOX_SEARCH_MAIN_WORK *wk ワーク
 *  @param  unit  ユニット
 *  @param  heapID  ヒープID  
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static void BOXSEARCH_POKE_ICON_CreateCLWK( BOX_SEARCH_MAIN_WORK* wk, u32 mons, u32 form_no, u32 sex, BOOL egg, GFL_CLUNIT* unit, HEAPID heapID )
{
  // ポケアイコン本体
  {
    UI_EASY_CLWK_RES_PARAM prm;

    prm.draw_type = CLSYS_DRAW_MAIN;
    prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NCLR;
    prm.arc_id    = ARCID_POKEICON;
    prm.pltt_id   = POKEICON_GetPalArcIndex();
  //  prm.ncg_id    = POKEICON_GetCgxArcIndex(ppp);
    prm.ncg_id    = POKEICON_GetCgxArcIndexByMonsNumber( mons, form_no, sex, egg );
    prm.cell_id   = POKEICON_GetCellArcIndex(); 
    prm.anm_id    = POKEICON_GetAnmArcIndex();
    prm.pltt_line = PLTID_OBJ_POKEICON_M;
    prm.pltt_src_ofs  = 0;
    prm.pltt_src_num = 3;
    
    UI_EASY_CLWK_LoadResource( &wk->clres_poke_icon, &prm, unit, heapID );

    // アニメシーケンスで指定( 0=瀕死, 1=HP最大, 2=HP緑, 3=HP黄, 4=HP赤, 5=状態異常 )
    wk->clwk_poke_icon = UI_EASY_CLWK_CreateCLWK( &wk->clres_poke_icon, unit, 48, 32, 1, heapID );

    // 上にアイテムアイコンを描画するので優先度を下げておく
    GFL_CLACT_WK_SetSoftPri( wk->clwk_poke_icon, 1 );

    // オートアニメ ON
    GFL_CLACT_WK_SetAutoAnmFlag( wk->clwk_poke_icon, TRUE );

    {
      u8 pal_num = POKEICON_GetPalNum( mons, form_no, sex, egg );
      GFL_CLACT_WK_SetPlttOffs( wk->clwk_poke_icon, pal_num, CLWK_PLTTOFFS_MODE_OAM_COLOR );
    }
  }

  // ポケアイコン用アイテムアイコン
  {
    UI_EASY_CLWK_RES_PARAM prm;

    prm.draw_type = CLSYS_DRAW_MAIN;
    prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NONE;
    prm.arc_id    = APP_COMMON_GetArcId();
    prm.pltt_id   = APP_COMMON_GetPokeItemIconPltArcIdx();
    prm.ncg_id    = APP_COMMON_GetPokeItemIconCharArcIdx();
    prm.cell_id   = APP_COMMON_GetPokeItemIconCellArcIdx( APP_COMMON_MAPPING_128K );
    prm.anm_id    = APP_COMMON_GetPokeItemIconAnimeArcIdx( APP_COMMON_MAPPING_128K );
    prm.pltt_line = PLTID_OBJ_POKEITEM_M;
    prm.pltt_src_ofs  = 0;
    prm.pltt_src_num = 1;
    
    UI_EASY_CLWK_LoadResource( &wk->clres_poke_item, &prm, unit, heapID );

    // アニメシーケンスで指定( 0=どうぐ, 1=メール )
    // ※位置調整はとりあえずの値です。
    wk->clwk_poke_item = UI_EASY_CLWK_CreateCLWK( &wk->clres_poke_item, unit, 48+1*8+4, 32+4, 0, heapID );
  }

}

//-----------------------------------------------------------------------------
/**
 *  @brief  ポケアイコン 破棄
 *
 *  @param  BOX_SEARCH_MAIN_WORK *wk ワーク
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static void BOXSEARCH_POKE_ICON_DeleteCLWK( BOX_SEARCH_MAIN_WORK* wk )
{ 
  //CLWK破棄
  GFL_CLACT_WK_Remove( wk->clwk_poke_icon );
  //リソース開放
  UI_EASY_CLWK_UnLoadResource( &wk->clres_poke_icon );
  
  //CLWK破棄
  GFL_CLACT_WK_Remove( wk->clwk_poke_item );
  //リソース開放
  UI_EASY_CLWK_UnLoadResource( &wk->clres_poke_item );
}

#endif //BOX_SEARCH_POKE_ICON

#ifdef BOX_SEARCH_OAM_MAPMODEL
//=============================================================================
/**
 *  OAM_MAPMODEL
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *  @brief  OAMマップモデル 作成 (人物OBJ特化仕様)
 *
 *  @param  BOX_SEARCH_MAIN_WORK *wk ワーク
 *  @param  tex_idx   テクスチャのARC内インデックス
 *  @param  ptn_ofs   テクスチャオフセット（アニメパターンのオフセット）
 *  @param  GFL_CLUNIT *unit  CLUNIT  ユニット
 *  @param  heapID            ヒープID
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static void BOXSEARCH_OAM_MAPMODEL_CreateCLWK( BOX_SEARCH_MAIN_WORK *wk, u16 tex_idx, u8 ptn_ofs, GFL_CLUNIT *unit, HEAPID heapID )
{ 
  UI_EASY_CLWK_RES_PARAM prm;

  prm.draw_type = CLSYS_DRAW_MAIN;
  prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NONE;
  prm.arc_id    = APP_COMMON_GetArcId();
  prm.pltt_id   = APP_COMMON_GetNull4x4PltArcIdx();
  prm.ncg_id    = APP_COMMON_GetNull4x4CharArcIdx();
  prm.cell_id   = APP_COMMON_GetNull4x4CellArcIdx( APP_COMMON_MAPPING_128K );
  prm.anm_id    = APP_COMMON_GetNull4x4AnimeArcIdx( APP_COMMON_MAPPING_128K );
  prm.pltt_line = PLTID_OBJ_OAM_MAPMODEL_M;
  prm.pltt_src_ofs  = 0;
  prm.pltt_src_num = 1;

  // リソース読み込み
  UI_EASY_CLWK_LoadResource( &wk->clres_oam_mmdl, &prm, unit, heapID );
  // CLWK生成
  wk->clwk_oam_mmdl = UI_EASY_CLWK_CreateCLWK( &wk->clres_oam_mmdl, unit, 80, 32, 0, heapID );

  // テクスチャを転送
  {
    // 人物は基本 4 x 4
    int sx = NSBTX_DEF_SX;
    int sy = NSBTX_DEF_SY;

    CLWK_TransNSBTX( wk->clwk_oam_mmdl, ARCID_MMDL_RES, tex_idx, ptn_ofs, sx, sy, 0, prm.draw_type, heapID );
  }
}

//-----------------------------------------------------------------------------
/**
 *  @brief  OAMマップモデル 破棄
 *
 *  @param  BOX_SEARCH_MAIN_WORK *wk ワーク
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static void BOXSEARCH_OAM_MAPMODEL_DeleteCLWK( BOX_SEARCH_MAIN_WORK* wk )
{
  //CLWK破棄
  GFL_CLACT_WK_Remove( wk->clwk_oam_mmdl );
  //リソース開放
  UI_EASY_CLWK_UnLoadResource( &wk->clres_oam_mmdl );
}

#endif //BOX_SEARCH_OAM_MAPMODEL




#ifdef  BOX_SEARCH_PRINT_TOOL
//=============================================================================
/**
 *  PRINT_TOOL
 */
//=============================================================================
/*
    ・「HP ??/??」を表示するサンプルです
    ・BMPWINのサイズは 20 x 2 です
    ・現在のHP = 618, 最大HP = 999 とします
    ・サンプルメインが FALSE を返すと終了です
*/

// サンプルメイン
static BOOL PrintTool_MainFunc( BOX_SEARCH_MAIN_WORK * wk )
{
  switch( wk->seq ){
  case 0:
    PrintTool_AddBmpWin( wk );      // BMPWIN作成
    PrintTool_PrintHP( wk );        // ＨＰ表示
    PrintTool_ScreenTrans( wk );    // スクリーン転送
    wk->seq = 1;
    break;

  case 1:
    // プリント終了待ち
    if( PRINTSYS_QUE_IsFinished( wk->que ) == TRUE ){
      wk->seq = 2;
    }
    break;

  case 2:
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
      PrintTool_DelBmpWin( wk );    // BMPWIN削除
      return FALSE;
    }
  }

  PRINT_UTIL_Trans( &wk->print_util, wk->que );
  return TRUE;
}

// BMPWIN作成
static void PrintTool_AddBmpWin( BOX_SEARCH_MAIN_WORK * wk )
{
  wk->print_util.win = GFL_BMPWIN_Create(
                          GFL_BG_FRAME0_M,          // ＢＧフレーム
                          1, 1,                     // 表示座標
                          20, 2,                    // 表示サイズ
                          15,                       // パレット
                          GFL_BMP_CHRAREA_GET_B );  // キャラ取得方向
}

// BMPWIN削除
static void PrintTool_DelBmpWin( BOX_SEARCH_MAIN_WORK * wk )
{
    GFL_BMPWIN_Delete( wk->print_util.win );
}

// BMPWINスクリーン転送
static void PrintTool_ScreenTrans( BOX_SEARCH_MAIN_WORK * wk )
{
  GFL_BMPWIN_MakeScreen( wk->print_util.win );
  GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->print_util.win) );
}

// ＨＰ表示
static void PrintTool_PrintHP( BOX_SEARCH_MAIN_WORK * wk )
{
  STRBUF * strbuf;

  // BMPWIN内の座標(32,0)を基準に右詰めで「ＨＰ」と表示
  PRINTTOOL_Print(
        &wk->print_util,        // プリントユーティル ( BMPWIN )
        wk->que,                // プリントキュー
        32, 0,                  // 表示基準座標
        strbuf,                 // 文字列（すでに「ＨＰ」が展開されているものとする）
        wk->font,               // フォント
        PRINTTOOL_MODE_RIGHT ); // 右詰

  // BMPWIN内の座標(100,0)を「／」の中心にしてＨＰを表示 ( hp / mhp )
  PRINTTOOL_PrintFraction(
        &wk->print_util,        // プリントユーティル ( BMPWIN )
        wk->que,                // プリントキュー
        wk->font,               // フォント
        100, 0,                 // 表示基準座標
        618,                    // hp
        999,                    // mhp
        wk->heapID );           // ヒープＩＤ
}
#endif  //BOX_SEARCH_PRINT_TOOL

#ifdef BOX_SEARCH_PMSDRAW

//-----------------------------------------------------------------------------
/**
 *  @brief  簡易会話表示 初期化処理
 *
 *  @param  BOX_SEARCH_MAIN_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void BOXSEARCH_PMSDRAW_Init( BOX_SEARCH_MAIN_WORK* wk )
{
  GFL_CLUNIT  *clunit;
  
  clunit = BOX_SEARCH_GRAPHIC_GetClunit( wk->graphic );

  wk->pms_draw  = PMS_DRAW_Init( clunit, CLSYS_DRAW_SUB, wk->print_que, wk->font, 
      PLTID_OBJ_PMS_DRAW, BOX_SEARCH_PMSDRAW_NUM ,wk->heapID );
  
  {
    int i;
    PMS_DATA pms;

    // PMS表示用BMPWIN生成
    for( i=0; i<BOX_SEARCH_PMSDRAW_NUM; i++ )
    {
      wk->pms_win[i] = GFL_BMPWIN_Create(
          BG_FRAME_TEXT_S,          // ＢＧフレーム
          2+4*i, 0 + 6 * i,             // 表示座標(キャラ単位)
          28, 4,                    // 表示サイズ
          15,                       // パレット
          GFL_BMP_CHRAREA_GET_B );  // キャラ取得方向
    }
    
    // 1個目 通常+空欄表示
    PMSDAT_SetDebug( &pms );
    PMSDAT_SetWord( &pms, 1, PMS_WORD_NULL );
    PMS_DRAW_Print( wk->pms_draw, wk->pms_win[0], &pms ,0 );

    // 2個目 デコメ
    PMSDAT_SetDeco( &pms, 1, PMS_DECOID_HERO );
    PMS_DRAW_Print( wk->pms_draw, wk->pms_win[1], &pms ,1 );
    
    // 3個目 デコメ二個表示
    PMSDAT_SetDeco( &pms, 0, PMS_DECOID_TANKS );
    PMSDAT_SetDeco( &pms, 1, PMS_DECOID_LOVE );
    PMS_DRAW_Print( wk->pms_draw, wk->pms_win[2], &pms ,2 );
    
    // 1の要素を2にコピー(移動表現などにご使用ください。)
    PMS_DRAW_Copy( wk->pms_draw, 1, 2 );
  }
}

//-----------------------------------------------------------------------------
/**
 *  @brief  簡易会話表示 後処理
 *
 *  @param  BOX_SEARCH_MAIN_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void BOXSEARCH_PMSDRAW_Exit( BOX_SEARCH_MAIN_WORK* wk )
{
  PMS_DRAW_Exit( wk->pms_draw );
  {
    int i;
    for( i=0; i<BOX_SEARCH_PMSDRAW_NUM; i++ )
    {
      GFL_BMPWIN_Delete( wk->pms_win[i] );
    }
  }
}

//-----------------------------------------------------------------------------
/**
 *  @brief  簡易会話表示 主処理
 *
 *  @param  BOX_SEARCH_MAIN_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void BOXSEARCH_PMSDRAW_Proc( BOX_SEARCH_MAIN_WORK* wk )
{
#if 1
  // SELECTでクリア
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
  {
    int i;
    for( i=0; i<BOX_SEARCH_PMSDRAW_NUM; i++ )
    {
      PMS_DRAW_Clear( wk->pms_draw, i, TRUE );
    }
  }
  // STARTでランダム挿入（二重登録するとアサート）
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START )
  {
    int i;
    for( i=0; i<BOX_SEARCH_PMSDRAW_NUM; i++ )
    {
      PMS_DATA pms;
      PMSDAT_SetDebugRandom( &pms );
      PMSDAT_SetDeco( &pms, 0, GFUser_GetPublicRand(10)+1 );
      PMS_DRAW_Print( wk->pms_draw, wk->pms_win[i], &pms ,i );
    }
  }
#endif

  PMS_DRAW_Main( wk->pms_draw );
}

#endif // BOX_SEARCH_PMSDRAW


//----------------------------------------------------------------------------------
/**
 * @brief 文字列描画用領域確保
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void   SearchBmpwinInit( BOX_SEARCH_MAIN_WORK *wk )
{
  int i;
  for(i=0;i<BOX_SEARCH_BMPWIN_NUM;i++){
    wk->bmp_win[i] = GFL_BMPWIN_Create(
        BG_FRAME_TEXT_S,          // ＢＧフレーム
        2+4*i, 0 + 6 * i,             // 表示座標(キャラ単位)
        28, 4,                    // 表示サイズ
        15,                       // パレット
        GFL_BMP_CHRAREA_GET_B );  // キャラ取得方向
  }
}



static int Subseq_Start(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_Return(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_SelectPokemonInit(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_SelectPokemon1(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_SelectPokemon2Init(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_SelectPokemon2(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_SelectPokemon3Init(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_SelectPokemon3(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_SelectSeikakuInit(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_SelectSeikaku(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_SelectTokuseiInit(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_SelectTokusei1(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_SelectTokusei2Init(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_SelectTokusei2(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_SelectTokusei3Init(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_SelectTokusei3(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_SelectSexInit(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_SelectSex(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_SelectItemHavaInit(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_SelectItemHava(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_SelectMarkInit(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_SelectMark(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_SearchOn(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_SearchOff(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_SearchReset(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_AnimeWait(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_SearchExit(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
