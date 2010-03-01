//=============================================================================
/**
 *
 *  @file   un_select.c
 *  @brief  国連 フロア選択
 *  @author genya hosaka >> saito
 *  @data   2010.02.05
 *
 */
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/bmp_winframe.h" // for BmpWinFrame_
#include "system/brightness.h" // for

// 簡易会話表示システム
#include "system/pms_draw.h"

#include "gamesystem/msgspeed.h"  // for MSGSPEED_GetWait

// 文字列関連
#include "print/gf_font.h"
#include "print/wordset.h"
#include "print/printsys.h"
#include "font/font.naix"

//INFOWIN
#include "infowin/infowin.h"

//描画設定
#include "un_select_graphic.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"

#include "ui/print_tool.h" // for 

//タッチバー
#include "ui/touchbar.h"

// BGフレームリスト
#include "ui/frame_list.h"

// シーンコントローラー
#include "ui/ui_scene.h"

//タスクメニュー
#include "app/app_taskmenu.h"

//アプリ共通素材
#include "app/app_menu_common.h"

//アーカイブ
#include "arc_def.h"

//外部公開
#include "app/un_select.h"

#include "message.naix"
#include "un_select_gra.naix" // アーカイブ

// サウンド
#include "sound/pm_sndsys.h"

#include "msg/msg_wifi_place_msg_UN.h"  // GMM

//国50音順配列
#include "country_ary.cdat"

//=============================================================================
// 下記defineをコメントアウトすると、機能を取り除けます
//=============================================================================
#define UN_SELECT_TOUCHBAR

FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
 *                定数定義
 */
//=============================================================================





//=============================================================================
/**
 *                定数定義
 */
//=============================================================================

#define   UN_SELECT_HEAP_SIZE (0x30000) ///< ヒープサイズ
#define   CUR_MOVE_RANGE (4)

// パッシブ定数　※上画面はアルファブレンドを使用するので、マスター輝度変更でパッシブ状態にする
#define YESNO_MASK_DISP ( MASK_MAIN_DISPLAY )
#define YESNO_MASK_PLANE ( PLANEMASK_BG3 | PLANEMASK_BG2 | PLANEMASK_OBJ | PLANEMASK_BD )
#define BRIGHT_PASSIVE_SYNC (8)
#define BRIGHT_PASSIVE_VOL (-8)

// フォントカラー
#define FCOL_WP01WN   ( PRINTSYS_LSB_Make(14,15,0) )  // フォントカラー：明日

#define	BLEND_EV1		( 6 )				// ブレンド係数
#define	BLEND_EV2		( 10 )			// ブレンド係数

#define COUNTRY_DISP_OFS  (30)

//ＢＧ書き換え用
#define BASE_OFS  (1*32)    //ＢＧ先頭アドレス１キャラ先から開始
#define MAIN_DISP_COUNTRY_MAX (144)   //メイン画面には144カ国表示
#define SUB_DISP_COUNTRY_MAX (89)   //サブ画面には89カ国表示
//ＯＢＪ
#define UN_OBJ_CHRRES_MAX (1)
#define UN_OBJ_PALRES_MAX (1)
#define UN_OBJ_CELRES_MAX (1)
//#define UN_LISTMAKER_MAX (FLOOR_MARKING_MAX)
#define UN_LISTMARKER_SETUP_MAX  (4+2)
#define UN_LISTMAKER_MAIN_MAX (UN_LISTMARKER_SETUP_MAX+1)
#define UN_LISTMAKER_SUB_MAX (UN_LISTMARKER_SETUP_MAX+1)
#define UN_BUILMAKER_MAX (FLOOR_MARKING_MAX)
#define UN_BUILCURSOR_MAX (1)
#define UN_SCRBER_MAX (1)
#define UN_OBJ_MAX ( /*UN_LISTMAKER_MAX*/UN_LISTMAKER_MAIN_MAX+UN_LISTMAKER_SUB_MAX+UN_BUILMAKER_MAX+UN_BUILCURSOR_MAX+UN_SCRBER_MAX )

#define BUIL_CUR_SIZE (8)      //グラフィック依存
#define SCROLL_BAR_SIZE (16)      //グラフィック依存
#define LIST_MARLER_BASE_POS_MAIN (20) //メイン画面リストマーカー基点位置（ドット）
#define LIST_MARLER_BASE_POS_SUB (192-20) //サブ画面リストマーカー基点位置（ドット）
#define LIST_MARLER_OFS (5*8)     //リスト項目は５キャラ間隔
#define BASE_FLOOR_IDX (3)       //計算基底フロアインデックス

//ビルフロア塗りつぶし開始Ｙドット位置（絶対座標）ここの位置がフロアインデックス0(1カ国目234階)に相当
#define BUIL_FLOOR_YDOT_TOP    (103)
//ビルフロア塗りつぶし開始Ｙドット位置（絶対座標）ここの位置がフロアインデックス232（233カ国目2階）に相当
#define BUIL_FLOOR_YDOT_BOTTOM    (335)

#define SCROLL_BAR_UY (8)
#define SCROLL_BAR_DY (160)

#define NOT_MARKER (0xffff)

enum {
  UN_OBJ_BUIL_CURSOR = 0,           //1
  UN_OBJ_BUIL_MARKER_START = 1,     //20
  UN_OBJ_LIST_MARKER_M_START = 21,  //7
  UN_OBJ_LIST_MARKER_S_START = 28,  //7
  UN_OBJ_SCROLL_BAR = 35,
};

//両用０１白抜

//--------------------------------------------------------------
/// アプリ終了モード
//==============================================================
typedef enum { 
  END_MODE_DECIDE,
  END_MODE_CANCEL
} END_MODE;

//-------------------------------------
/// フレーム
//=====================================
enum
{ 
  BG_FRAME_TEXT_M = GFL_BG_FRAME0_M,
  BG_FRAME_MENU_M = GFL_BG_FRAME1_M,
  BG_FRAME_LIST_M = GFL_BG_FRAME2_M,
  BG_FRAME_BACK_M = GFL_BG_FRAME3_M,

  BG_FRAME_TEXT_S = GFL_BG_FRAME0_S, 
  BG_FRAME_LIST_S = GFL_BG_FRAME2_S,
  BG_FRAME_BACK_S = GFL_BG_FRAME3_S,
};

//-------------------------------------
/// パレット
//=====================================
enum
{ 
  //メインBG
  PLTID_BG_BACK_M       = 0,
//  PLTID_BG_TEXT_M       = 1,
//  PLTID_BG_TEXT_WIN_M   = 2,
  PLTID_BG_TASKMENU_M   = 11,
  PLTID_BG_TOUCHBAR_M   = 13,
  PLTID_BG_TEXT_M       = 14,
  PLTID_BG_TEXT_WIN_M    = 15,
  //サブBG
  PLTID_BG_BACK_S       = 0,
  PLTID_BG_TEXT_S       = 6,

  //メインOBJ
  PLTID_OBJ_TOUCHBAR_M  = 0, // タッチバーパレット　3本使用
  PLTID_OBJ_UN_MS  = 3, // 国連アプリパレット　1本使用 上下で使用

  //サブOBJ
  PLTID_OBJ_PMS_DRAW = 0, // 5本使用
};

//=============================================================================
/**
 *                構造体定義
 */
//=============================================================================
#if 0
//--------------------------------------------------------------
/// BG管理ワーク
//==============================================================
typedef struct 
{
  int dummy;
} UN_SELECT_BG_WORK;
#endif
//--------------------------------------------------------------
/// メッセージ管理ワーク
//==============================================================
typedef struct
{ 
  //[IN]
  HEAPID  heap_id;

  //[PRIVATE]
  GFL_TCBLSYS   *tcblsys;     ///< タスクシステム
  GFL_MSGDATA   *msghandle;   ///< メッセージハンドル
  GFL_FONT      *font;        ///< フォント
  PRINT_QUE     *print_que;   ///< プリントキュー
  WORDSET       *wordset;     ///< ワードセット

  STRBUF        *strbuf;
  STRBUF        *exp_strbuf;

  // ストリーム再生
  PRINT_STREAM* print_stream;
  GFL_BMPWIN    *win_talk;
  GFL_BMPWIN    *win_talk_up;

  STRBUF* name[ UN_LIST_MAX ]; ///< 名前バッファ

} UN_SELECT_MSG_CNT_WORK;

//--------------------------------------------------------------
/// メインワーク
//==============================================================
typedef struct 
{
  HEAPID heap_id;

//  UN_SELECT_BG_WORK       wk_bg;

  //描画設定
  UN_SELECT_GRAPHIC_WORK  *graphic;

#ifdef UN_SELECT_TOUCHBAR
  //タッチバー
  TOUCHBAR_WORK             *touchbar;
  //以下カスタムボタン使用する場合のサンプルリソース
  u32                       ncg_btn;
  u32                       ncl_btn;
  u32                       nce_btn;
#endif //UN_SELECT_TOUCHBAR
  
  //タスクメニュー
  APP_TASKMENU_RES          *menu_res;
  APP_TASKMENU_WORK         *menu;

  UN_SELECT_PARAM* pwk;

  UI_SCENE_CNT_PTR cnt_scene;

  UN_SELECT_MSG_CNT_WORK* cnt_msg;

  FRAMELIST_WORK* lwk;

  int ListSelPos;   //リストで選んだ位置 0～UN_LIST_MAX-1

  BOOL Valid[UN_LIST_MAX]; //0=最上階　232=2Ｆ

  GFL_TCB * htask;		// TCB ( HBLANK )

  //OBJ
//  GFL_CLUNIT * clunit;
	GFL_CLWK * ClWk[UN_OBJ_MAX];
	u32	ChrRes[UN_OBJ_CHRRES_MAX];
	u32	PalRes[UN_OBJ_PALRES_MAX];
	u32	CelRes[UN_OBJ_CELRES_MAX];

  //マーカー表示する２０個の対象フロア
  int MarkerFloor[FLOOR_MARKING_MAX];
  
} UN_SELECT_MAIN_WORK;

// セルアクターデータ
typedef struct {
	GFL_CLWK_DATA	Dat;

	u32	ChrRes;
	u32	PalRes;
	u32	CelRes;

	u16	Pal;
	u16	Disp; //CLSYS_DEFREND_TYPE or CLWK_SETSF_NONE 

}UN_CLWK_DATA;

//=============================================================================
/**
 *              シーン定義
 */
//=============================================================================

// リスト生成
static BOOL SceneListMake( UI_SCENE_CNT_PTR cnt, void* work );

// フロア選択
static BOOL SceneSelectFloor_Init( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL SceneSelectFloor_Main( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL SceneSelectFloor_End( UI_SCENE_CNT_PTR cnt, void* work );

// 確認画面
static BOOL SceneConfirm_Init( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL SceneConfirm_Main( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL SceneConfirm_End( UI_SCENE_CNT_PTR cnt, void* work );

// ジャンプＮＧ
static BOOL SceneJumpNG_Init( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL SceneJumpNG_Main( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL SceneJumpNG_End( UI_SCENE_CNT_PTR cnt, void* work );

static void HBlankTask( GFL_TCB * tcb, void * work );


//ＯＢＪ
static void SetupRes( UN_SELECT_MAIN_WORK *wk );
static void ExitRes( UN_SELECT_MAIN_WORK *wk );
static void MakeAct( UN_SELECT_MAIN_WORK *wk );
static void DelAct( UN_SELECT_MAIN_WORK *wk );
static GFL_CLWK *CreateAct( GFL_CLUNIT *unit, UN_CLWK_DATA *data );

static void MvListMarkerPos( UN_SELECT_MAIN_WORK *wk, const int inAddVal);
static void SetBuilMarkerPos( UN_SELECT_MAIN_WORK *wk );
static void SetScrollBarPos( UN_SELECT_MAIN_WORK *wk, const int inY );
static void SetBuilCurPos( UN_SELECT_MAIN_WORK *wk, const int inY );

static void SetupListMarker( UN_SELECT_MAIN_WORK* wk, int target_item );
static void SetListMarker(UN_SELECT_MAIN_WORK* wk, const BOOL inIsMain, const int inY);
static BOOL SetListMarkerCore(GFL_CLWK **clwk_ary, const int inIdx, const u16 inSetsf, const int inY);
static BOOL IsMarkerFloorValid( UN_SELECT_MAIN_WORK *wk, const int inTargetItem );

static int GetFloorIdxFromCountryCode(const int inCountryCode);

//--------------------------------------------------------------
/// SceneID
//==============================================================
typedef enum
{ 
  UNS_SCENE_ID_LIST_MAKE = 0, ///< リスト生成
  UNS_SCENE_ID_SELECT_FLOOR,  ///< フロア選択
  UNS_SCENE_ID_CONFIRM,       ///< 確認画面
  UNS_SCENE_ID_JUMP_NG,       ///< ジャンプＮＧ

  UNS_SCENE_ID_MAX,
} UNS_SCENE_ID;

//--------------------------------------------------------------
/// SceneFunc
//==============================================================
static const UI_SCENE_FUNC_SET c_scene_func_tbl[ UNS_SCENE_ID_MAX ] = 
{
  // UNS_SCENE_ID_LIST_MAKE
  {
    NULL,
    NULL,
    SceneListMake,
    NULL,
    NULL,
  },
  // UNS_SCENE_ID_SELECT_FLOOR
  {
    SceneSelectFloor_Init,
    NULL,
    SceneSelectFloor_Main,
    NULL,
    SceneSelectFloor_End,
  },
  // UNS_SCENE_ID_CONFIRM
  {
    SceneConfirm_Init,
    NULL,
    SceneConfirm_Main,
    NULL,
    SceneConfirm_End,
  },
  // UNS_SCENE_ID_JUMP_NG
  {
    SceneJumpNG_Init,
    NULL,
    SceneJumpNG_Main,
    NULL,
    SceneJumpNG_End,
  },
};

//=============================================================================
/**
 *              プロトタイプ宣言
 */
//=============================================================================
//-------------------------------------
/// PROC
//=====================================
static GFL_PROC_RESULT UNSelectProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT UNSelectProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT UNSelectProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
//-------------------------------------
/// 汎用処理ユーティリティ
//=====================================
static void UNSelect_BG_LoadResource( UN_SELECT_MAIN_WORK* wk, HEAPID heap_id );

#ifdef UN_SELECT_TOUCHBAR
//-------------------------------------
/// タッチバー
//=====================================
static TOUCHBAR_WORK * UNSelect_TOUCHBAR_Init( GFL_CLUNIT *clunit, HEAPID heap_id );
static void UNSelect_TOUCHBAR_Exit( TOUCHBAR_WORK *touchbar );
static void UNSelect_TOUCHBAR_Main( TOUCHBAR_WORK *touchbar );
#endif //UN_SELECT_TOUCHBAR

//-------------------------------------
/// リストシステムはい、いいえ
//=====================================
static APP_TASKMENU_WORK * UNSelect_TASKMENU_Init( APP_TASKMENU_RES *menu_res, GFL_MSGDATA *msg, HEAPID heap_id );
static void UNSelect_TASKMENU_Exit( APP_TASKMENU_WORK *menu );
static void UNSelect_TASKMENU_Main( APP_TASKMENU_WORK *menu );

//-------------------------------------
// パッシブ
//=====================================
static void PASSIVE_Request( void );
static void PASSIVE_Reset( void );

//-------------------------------------
// メッセージ管理モジュール
//=====================================
static UN_SELECT_MSG_CNT_WORK* MSG_CNT_Create( HEAPID heap_id );
static void MSG_CNT_Delete( UN_SELECT_MSG_CNT_WORK* wk );
static void MSG_CNT_Main( UN_SELECT_MSG_CNT_WORK* wk );
static void MSG_CNT_SetPrint( UN_SELECT_MSG_CNT_WORK* wk, int str_id );
static BOOL MSG_CNT_PrintProc( UN_SELECT_MSG_CNT_WORK* wk );
static void MSG_CNT_PrintClear( UN_SELECT_MSG_CNT_WORK* wk );
static GFL_FONT* MSG_CNT_GetFont( UN_SELECT_MSG_CNT_WORK* wk );
static PRINT_QUE* MSG_CNT_GetPrintQue( UN_SELECT_MSG_CNT_WORK* wk );
static GFL_MSGDATA* MSG_CNT_GetMsgData( UN_SELECT_MSG_CNT_WORK* wk );

// PROTOTYPE
static UN_SELECT_MAIN_WORK* app_init( GFL_PROC* proc, UN_SELECT_PARAM* prm );
static void app_end( UN_SELECT_MAIN_WORK* wk, END_MODE end_mode );

static void LIST_Make( UN_SELECT_MAIN_WORK* wk );
static void LIST_Delete( UN_SELECT_MAIN_WORK* wk );

//=============================================================================
/**
 *                外部公開
 */
//=============================================================================
const GFL_PROC_DATA UNSelectProcData = 
{
  UNSelectProc_Init,
  UNSelectProc_Main,
  UNSelectProc_Exit,
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
 *  @param  *pwk          UN_SELECT_PARAM
 *  @param  *mywk         PROCワーク
 *
 *  @retval 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT UNSelectProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
  UN_SELECT_MAIN_WORK *wk;
  
  GF_ASSERT( pwk );

  switch( *seq )
  {
  case 0:
    wk = app_init( proc, pwk );
  
    // フェードイン リクエスト
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 1 );

    (*seq)++;
    break;

  case 1:
    if( GFL_FADE_CheckFade() == FALSE )
    {
      return GFL_PROC_RES_FINISH;
    }
    break;

  default : GF_ASSERT(0);
  }

  return GFL_PROC_RES_CONTINUE;
}
//-----------------------------------------------------------------------------
/**
 *  @brief  PROC 終了処理
 *
 *  @param  GFL_PROC *procプロセスシステム
 *  @param  *seq          シーケンス
 *  @param  *pwk          UN_SELECT_PARAM
 *  @param  *mywk         PROCワーク
 *
 *  @retval 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT UNSelectProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
  UN_SELECT_MAIN_WORK* wk = mywk;
    
  if( GFL_FADE_CheckFade() == TRUE )
  {
     return GFL_PROC_RES_CONTINUE;
  }

  GFL_TCB_DeleteTask( wk->htask );
  // ブレンド初期化
	G2_BlendNone();
	G2S_BlendNone();
  
  // リスト開放
  LIST_Delete( wk ); 

  // シーンコントーラ削除
  UI_SCENE_CNT_Delete( wk->cnt_scene );

  // メッセージ消去
  MSG_CNT_Delete( wk->cnt_msg );

  //アクター削除
  DelAct( wk );
  //ＯＢＪリソース解放
  ExitRes( wk );

  //TASKMENU リソース破棄
  APP_TASKMENU_RES_Delete( wk->menu_res );  

#ifdef UN_SELECT_TOUCHBAR
  //タッチバー
  UNSelect_TOUCHBAR_Exit( wk->touchbar );
#endif //UN_SELECT_TOUCHBAR

  //描画設定破棄
  UN_SELECT_GRAPHIC_Exit( wk->graphic );

  //PROC用メモリ解放
  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( wk->heap_id );

  //オーバーレイ破棄
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common) );

  HOSAKA_Printf(" PROC終了！ \n");

  return GFL_PROC_RES_FINISH;
}
//-----------------------------------------------------------------------------
/**
 *  @brief  PROC 主処理
 *
 *  @param  GFL_PROC *procプロセスシステム
 *  @param  *seq          シーケンス
 *  @param  *pwk          UN_SELECT_PARAM
 *  @param  *mywk         PROCワーク
 *
 *  @retval 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT UNSelectProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
  UN_SELECT_MAIN_WORK* wk = mywk;
  
  // SCENE
  if( UI_SCENE_CNT_Main( wk->cnt_scene ) )
  {
    return GFL_PROC_RES_FINISH;
  }

  // メッセージ主処理
  MSG_CNT_Main( wk->cnt_msg );

  //2D描画
  UN_SELECT_GRAPHIC_2D_Draw( wk->graphic );

  //3D描画
  UN_SELECT_GRAPHIC_3D_StartDraw( wk->graphic );

  UN_SELECT_GRAPHIC_3D_EndDraw( wk->graphic );

  return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *                static関数
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *  @brief  BG管理モジュール リソース読み込み + ビルライトBG書き換え
 *
 *  @param  UN_SELECT_MAIN_WORK* wk メインワークポインタ
 *  @param  heap_id  ヒープID 
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static void UNSelect_BG_LoadResource( UN_SELECT_MAIN_WORK* wk, HEAPID heap_id )
{
  ARCHANDLE *handle;
  void *mainbg_data;
  void *subbg_data;
  NNSG2dCharacterData* charData_main;
  NNSG2dCharacterData* charData_sub;

  handle  = GFL_ARC_OpenDataHandle( ARCID_UN_SELECT_GRA, heap_id );

  // 上下画面ＢＧパレット転送
  GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_un_select_gra_kokuren_bg_NCLR, PALTYPE_MAIN_BG, PLTID_BG_BACK_M, 7*0x20, heap_id );
  GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_un_select_gra_kokuren_bg_NCLR, PALTYPE_SUB_BG, PLTID_BG_BACK_S, 7*0x20, heap_id );
  
  //  ----- サブ画面 -----
/**
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_un_select_gra_kokuren_bg_NCGR,
            BG_FRAME_BACK_S, 0, 0, 0, heap_id );
*/
  GFL_ARCHDL_UTIL_TransVramScreen(  handle, NARC_un_select_gra_kokuren_bgu_NSCR,
            BG_FRAME_BACK_S, 0, 0, 0, heap_id );

  subbg_data = GFL_ARCHDL_UTIL_LoadBGCharacter(
      handle, NARC_un_select_gra_kokuren_bg_NCGR, FALSE, &charData_sub, heap_id );

  //  ----- メイン画面 -----
/**
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_un_select_gra_kokuren_bg_NCGR,
            BG_FRAME_BACK_M, 0, 0, 0, heap_id );
*/            
  GFL_ARCHDL_UTIL_TransVramScreen(  handle, NARC_un_select_gra_kokuren_bgd_NSCR,
            BG_FRAME_BACK_M, 0, 0, 0, heap_id );

  mainbg_data = GFL_ARCHDL_UTIL_LoadBGCharacter(
      handle, NARC_un_select_gra_kokuren_bg_NCGR, FALSE, &charData_main, heap_id );
  
  // ----- リストバー -----
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_un_select_gra_kokuren_bg_listframe_NCGR,
            BG_FRAME_LIST_M, 0, 0, 0, heap_id );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_un_select_gra_kokuren_bg_listframe_NCGR,
            BG_FRAME_LIST_S, 0, 0, 0, heap_id );

  //BGキャラデータ書き換え
  {
    int i;
    for( i=0; i<UN_LIST_MAX; i++)
    {
      u8 *data;
      u8 chr_idx;
      u8 target_line;
      u8 ofs;
      int adr;
      int target;
      int valid_idx;
///>      valid_idx = (UN_LIST_MAX - i) - 1;
      valid_idx = i;
      target = (UN_LIST_MAX - i) - 1;

      if (i < SUB_DISP_COUNTRY_MAX) data = (u8*)charData_sub->pRawData;
      else data = (u8*)charData_main->pRawData;

      if ( !wk->Valid[valid_idx] )
      {
        //書き換えキャラを選定
        chr_idx = target/8;
        target_line = 7 - (target%8);    //キャラ内書き換え対象ライン（0～7）キャラの下から書き換える
        ofs = target_line * 4;    //一列８ドットは４バイト
        adr = BASE_OFS + (chr_idx * 32) + ofs;
        //４バイト書き換え
        data[adr] = 0xff;
        data[adr+1] = 0xff;
        data[adr+2] = 0xff;
        data[adr+3] = 0xff;
      }
    }
  }

  GFL_BG_LoadCharacter(BG_FRAME_BACK_S, charData_sub->pRawData, charData_sub->szByte, 0);
  GFL_HEAP_FreeMemory( subbg_data );
  GFL_BG_LoadCharacter(BG_FRAME_BACK_M, charData_main->pRawData, charData_main->szByte, 0);
  GFL_HEAP_FreeMemory( mainbg_data );  

  GFL_ARC_CloseDataHandle( handle );
}

#ifdef UN_SELECT_TOUCHBAR
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
 *  @param  heap_id             ヒープID
 *
 *  @return TOUCHBAR_WORK
 */
//-----------------------------------------------------------------------------
static TOUCHBAR_WORK * UNSelect_TOUCHBAR_Init( GFL_CLUNIT *clunit, HEAPID heap_id )
{ 
  TOUCHBAR_WORK* touchbar;

  //アイコンの設定
  //数分作る
  TOUCHBAR_ITEM_ICON touchbar_icon_tbl[]  =
  { 
    { 
      TOUCHBAR_ICON_RETURN,
      { TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_Y },
    },
    { 
      TOUCHBAR_ICON_CUR_L,
      { TOUCHBAR_ICON_X_00, TOUCHBAR_ICON_Y },
    },
    { 
      TOUCHBAR_ICON_CUR_R,
      { TOUCHBAR_ICON_X_02, TOUCHBAR_ICON_Y },
    },
  };

  //設定構造体
  //さきほどの窓情報＋リソース情報をいれる
  TOUCHBAR_SETUP  touchbar_setup;
  GFL_STD_MemClear( &touchbar_setup, sizeof(TOUCHBAR_SETUP) );

  touchbar_setup.p_item   = touchbar_icon_tbl;        //上の窓情報
  touchbar_setup.item_num = NELEMS(touchbar_icon_tbl);//いくつ窓があるか
  touchbar_setup.p_unit   = clunit;                   //OBJ読み込みのためのCLUNIT
  touchbar_setup.bar_frm  = BG_FRAME_MENU_M;            //BG読み込みのためのBG面
  touchbar_setup.bg_plt   = PLTID_BG_TOUCHBAR_M;      //BGﾊﾟﾚｯﾄ
  touchbar_setup.obj_plt  = PLTID_OBJ_TOUCHBAR_M;     //OBJﾊﾟﾚｯﾄ
  touchbar_setup.mapping  = APP_COMMON_MAPPING_32K;  //マッピングモード

  touchbar =  TOUCHBAR_Init( &touchbar_setup, heap_id );

  TOUCHBAR_SetBGPriorityAll( touchbar, 1 ); // BGプライオリティを設定

  //左右ボタンのＳＥを鳴らさないようにする
  TOUCHBAR_SetSE( touchbar, TOUCHBAR_ICON_CUR_L, 0 );
  TOUCHBAR_SetSE( touchbar, TOUCHBAR_ICON_CUR_R, 0 );

  return touchbar;
}
//----------------------------------------------------------------------------
/**
 *  @brief  TOUCHBAR破棄
 *
 *  @param  TOUCHBAR_WORK *touchbar タッチバー
 */
//-----------------------------------------------------------------------------
static void UNSelect_TOUCHBAR_Exit( TOUCHBAR_WORK *touchbar )
{ 
  TOUCHBAR_Exit( touchbar );
}

//----------------------------------------------------------------------------
/**
 *  @brief  TOUCHBARメイン処理
 *
 *  @param  TOUCHBAR_WORK *touchbar タッチバー
 */
//-----------------------------------------------------------------------------
static void UNSelect_TOUCHBAR_Main( TOUCHBAR_WORK *touchbar )
{ 
  TOUCHBAR_Main( touchbar );
}
#endif //UN_SELECT_TOUCHBAR

//----------------------------------------------------------------------------
/**
 *  @brief  TASKMENUの初期化
 *
 *  @param  menu_res  リソース
 */
//-----------------------------------------------------------------------------
static APP_TASKMENU_WORK * UNSelect_TASKMENU_Init( APP_TASKMENU_RES *menu_res, GFL_MSGDATA *msg, HEAPID heap_id )
{ 
  int i;
  APP_TASKMENU_INITWORK init;
  APP_TASKMENU_ITEMWORK item[2];
  APP_TASKMENU_WORK     *menu;  

  //窓の設定
  for( i = 0; i < NELEMS(item); i++ )
  { 
    item[i].str       = GFL_MSG_CreateString( msg, un_reception_msg_02 + i ); //文字列
    item[i].msgColor  = APP_TASKMENU_ITEM_MSGCOLOR; //文字色
    item[i].type      = APP_TASKMENU_WIN_TYPE_NORMAL; //窓の種類
  }

  //初期化
  init.heapId   = heap_id;
  init.itemNum  = NELEMS(item);
  init.itemWork = item;
  init.posType  = ATPT_RIGHT_DOWN;
  init.charPosX = 32;
  init.charPosY = 18;
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
static void UNSelect_TASKMENU_Exit( APP_TASKMENU_WORK *menu )
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
static void UNSelect_TASKMENU_Main( APP_TASKMENU_WORK *menu )
{ 
  APP_TASKMENU_UpdateMenu( menu );
}


//-----------------------------------------------------------------------------
/**
 *  @brief  パッシブリクエスト
 *
 *  @param  void 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void PASSIVE_Request( void )
{
  BrightnessChgReset( YESNO_MASK_DISP );
  ChangeBrightnessRequest( BRIGHT_PASSIVE_SYNC, BRIGHT_PASSIVE_VOL, 0, YESNO_MASK_PLANE, YESNO_MASK_DISP );
}

//-----------------------------------------------------------------------------
/**
 *  @brief  パッシブリセット
 *
 *  @param  void 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void PASSIVE_Reset( void )
{
  BrightnessChgReset( YESNO_MASK_DISP ); // パッシブ解除
  ChangeBrightnessRequest( BRIGHT_PASSIVE_SYNC, 0, BRIGHT_PASSIVE_VOL, YESNO_MASK_PLANE, YESNO_MASK_DISP );
}


//=============================================================================
//
//
// メッセージ管理クラス
//
//
//=============================================================================
enum
{ 
  CGX_BMPWIN_FRAME_POS = 1,
  STRBUF_SIZE = 1600,
};

#define MSG_CNT_SKIP_BTN (PAD_BUTTON_A|PAD_BUTTON_B)

//-----------------------------------------------------------------------------
/**
 *  @brief  メッセージ管理クラス 生成
 *
 *  @param  HEAPID heap_id 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static UN_SELECT_MSG_CNT_WORK* MSG_CNT_Create( HEAPID heap_id )
{
  UN_SELECT_MSG_CNT_WORK* wk;

  // メモリ アロック
  wk = GFL_HEAP_AllocClearMemory( heap_id, sizeof( UN_SELECT_MSG_CNT_WORK ) );

  // メンバ初期化
  wk->heap_id = heap_id;
  
  // メッセージ用フォント転送
  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 0x20*PLTID_BG_TEXT_M, 0x20, heap_id );
  
  // フレームウィンドウ用のキャラを用意
  BmpWinFrame_GraphicSet( BG_FRAME_TEXT_M, CGX_BMPWIN_FRAME_POS, PLTID_BG_TEXT_WIN_M, MENU_TYPE_SYSTEM, heap_id );
  
  // ワードセット生成
  wk->wordset = WORDSET_Create( heap_id );

  //メッセージ
  wk->msghandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
      NARC_message_wifi_place_msg_UN_dat, heap_id );

  //PRINT_QUE作成
  wk->print_que   = PRINTSYS_QUE_Create( heap_id );

  //フォント作成
  wk->font      = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
                        GFL_FONT_LOADTYPE_FILE, FALSE, heap_id );

  // STRBU 生成
  wk->strbuf      = GFL_STR_CreateBuffer( STRBUF_SIZE, heap_id );
  wk->exp_strbuf  = GFL_STR_CreateBuffer( STRBUF_SIZE, heap_id );

  wk->tcblsys = GFL_TCBL_Init( heap_id, heap_id, 1, 0 );
  
  // ウィンドウ生成
  wk->win_talk = GFL_BMPWIN_Create( BG_FRAME_TEXT_M, 1, 19, 30, 4, PLTID_BG_TEXT_M, GFL_BMP_CHRAREA_GET_B );

  //上画面固定メッセージ表示
  {
    const u8 clear_color = 15;
    GFL_BMPWIN* win;

    // ウィンドウ生成
    wk->win_talk_up = GFL_BMPWIN_Create( BG_FRAME_TEXT_S, 1, 1, 30, 2, PLTID_BG_TEXT_S, GFL_BMP_CHRAREA_GET_B );
    
    win = wk->win_talk_up;
    GFL_BMP_Clear(GFL_BMPWIN_GetBmp(win), clear_color);
    GFL_FONTSYS_SetColor(1, 2, clear_color);

    GFL_MSG_GetString( wk->msghandle, un_reception_msg_00, wk->strbuf );

    PRINTSYS_Print( GFL_BMPWIN_GetBmp(win), 4, 0, wk->strbuf, wk->font);

#if 1   //@todo
    GFL_BMPWIN_TransVramCharacter( win );
    GFL_BMPWIN_MakeScreen( win );

      GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(win) );
#else
/**  
    // フレーム生成
    BmpWinFrame_Write( wk->win_talk_up, WINDOW_TRANS_OFF, CGX_BMPWIN_FRAME_POS, GFL_BMPWIN_GetPalette(wk->win_talk_up) );


    // 転送
    { 
      GFL_BMPWIN_TransVramCharacter( win );
      GFL_BMPWIN_MakeScreen( win );

      GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(win) );
    }
*/
#endif    
  }
  
  return wk;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  メッセージ管理クラス 破棄
 *
 *  @param  UN_SELECT_MSG_CNT_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void MSG_CNT_Delete( UN_SELECT_MSG_CNT_WORK* wk )
{
  // ストリーム破棄
  if( wk->print_stream )
  {
    PRINTSYS_PrintStreamDelete( wk->print_stream );
    wk->print_stream = NULL;
  }

  // BMPWIN 破棄
  GFL_BMPWIN_Delete( wk->win_talk_up );
  GFL_BMPWIN_Delete( wk->win_talk );

  // STRBUF 破棄
  GFL_STR_DeleteBuffer( wk->strbuf );
  GFL_STR_DeleteBuffer( wk->exp_strbuf );
  //メッセージ破棄
  GFL_MSG_Delete( wk->msghandle );
  //PRINT_QUE
  PRINTSYS_QUE_Delete( wk->print_que );
  //FONT
  GFL_FONT_Delete( wk->font );
  // TCBL
  GFL_TCBL_Exit( wk->tcblsys );
  // ワードセット 破棄
  WORDSET_Delete( wk->wordset );

  // メモリ破棄
  GFL_HEAP_FreeMemory( wk );
}


//-----------------------------------------------------------------------------
/**
 *  @brief  メッセージ管理クラス 主処理
 *
 *  @param  UN_SELECT_MSG_CNT_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void MSG_CNT_Main( UN_SELECT_MSG_CNT_WORK* wk )
{
  PRINTSYS_QUE_Main( wk->print_que );
  
  GFL_TCBL_Main( wk->tcblsys );
}


// 国名が２行になる場合の文字数の境界（これ以上は改行が必要・これ以下は改行はいっていないはず）
#define MULTI_LINE_WORD_NUM ( 10 )

//-----------------------------------------------------------------------------
/**
 *  @brief  リスト項目描画(国名描画)
 *
 *  @param  UN_SELECT_MSG_CNT_WORK* wk
 *  @param  util 
 *  @pawram itemNum
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void MSG_CNT_DrawListElem( UN_SELECT_MSG_CNT_WORK* wk, PRINT_UTIL* util, PRINT_QUE *que, GFL_FONT *font, u32 itemNum )
{
  int ypos = 12;
  int idx;
  int floor;

  GF_ASSERT(wk);

  // フロア表示
  idx = itemNum;
///>  floor = UN_LIST_MAX - itemNum + 1; //フロアは2Ｆから
  floor = (UN_LIST_MAX+1) - itemNum;
  GFL_MSG_GetString( wk->msghandle, un_reception_msg_04, wk->strbuf );
  WORDSET_RegisterNumber( wk->wordset, 2, floor, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
  //※↑国連だけ特別に半角数字を許可してもらっているらしい。

  WORDSET_ExpandStr( wk->wordset, wk->exp_strbuf, wk->strbuf );

  PRINTTOOL_PrintColor( util, que, 0, 12, wk->exp_strbuf, font, FCOL_WP01WN, PRINTTOOL_MODE_LEFT );

  // 国名 表示(１行・２行対応）
  if(GFL_STR_GetBufferLength(wk->name[idx]) > MULTI_LINE_WORD_NUM){
    ypos = 4;
  }
  PRINTTOOL_PrintColor( util, que, COUNTRY_DISP_OFS, ypos, wk->name[idx], font, FCOL_WP01WN, PRINTTOOL_MODE_LEFT );

  return;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  メッセージ管理クラス メッセージプリント リクエスト
 *
 *  @param  UN_SELECT_MSG_CNT_WORK* wk
 *  @param  str_id 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void MSG_CNT_SetPrint( UN_SELECT_MSG_CNT_WORK* wk, int str_id )
{
  const u8 clear_color = 15;
  GFL_BMPWIN* win;
  int msgspeed;

  GF_ASSERT(wk);
  
  msgspeed  = MSGSPEED_GetWait();
  win       = wk->win_talk;
  
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(win), clear_color);
  GFL_FONTSYS_SetColor(1, 2, clear_color);

  GFL_MSG_GetString( wk->msghandle, str_id, wk->strbuf );

#ifdef PM_DEBUG
  GFL_STR_CheckBufferValid( wk->strbuf ); ///< 破損チェック
#endif
  
  WORDSET_ExpandStr( wk->wordset, wk->exp_strbuf, wk->strbuf );

#ifdef PM_DEBUG
  GFL_STR_CheckBufferValid( wk->strbuf ); ///< 破損チェック
#endif

  wk->print_stream = PRINTSYS_PrintStream( win, 4, 0, wk->exp_strbuf, wk->font, msgspeed,
                                           wk->tcblsys, 0xffff, wk->heap_id, clear_color );
  
  // フレーム生成
  BmpWinFrame_Write( wk->win_talk, WINDOW_TRANS_OFF, CGX_BMPWIN_FRAME_POS, PLTID_BG_TEXT_WIN_M );

  // 転送
  { 
    GFL_BMPWIN_TransVramCharacter( win );
    GFL_BMPWIN_MakeScreen( win );

    GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(win) );
  }

}

//-----------------------------------------------------------------------------
/**
 *  @brief  メッセージ管理クラス メッセージ処理
 *
 *  @param  UN_SELECT_MSG_CNT_WORK* wk 
 *
 *  @retval TRUE:プリント終了
 */
//-----------------------------------------------------------------------------
static BOOL MSG_CNT_PrintProc( UN_SELECT_MSG_CNT_WORK* wk )
{
  PRINTSTREAM_STATE state;

  GF_ASSERT(wk);

//  HOSAKA_Printf("print state= %d \n", state );

  if( wk->print_stream )
  {
    state = PRINTSYS_PrintStreamGetState( wk->print_stream );

    switch( state )
    {
    case PRINTSTREAM_STATE_DONE : // 終了
      PRINTSYS_PrintStreamDelete( wk->print_stream );
      wk->print_stream = NULL;
      return TRUE;

    case PRINTSTREAM_STATE_PAUSE : // 一時停止中
      // キー入力待ち
      if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_DECIDE || (  GFL_UI_KEY_GetTrg() == PAD_BUTTON_CANCEL ) || GFL_UI_TP_GetTrg() )
      {
        PRINTSYS_PrintStreamReleasePause( wk->print_stream );
        GFL_SOUND_PlaySE( SEQ_SE_DECIDE1 );
      }
      break;

    case PRINTSTREAM_STATE_RUNNING :  // 実行中
      // メッセージスキップ
      if( (GFL_UI_KEY_GetCont() & MSG_CNT_SKIP_BTN) || GFL_UI_TP_GetCont() )
      {
        PRINTSYS_PrintStreamShortWait( wk->print_stream, 0 );
      }
      break;

    default : GF_ASSERT(0);
    }

    return FALSE;
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  メッセージフレームをクリア
 *
 *  @param  UN_SELECT_MSG_CNT_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void MSG_CNT_PrintClear( UN_SELECT_MSG_CNT_WORK* wk )
{
  BmpWinFrame_Clear( wk->win_talk, WINDOW_TRANS_ON_V );
}

//-----------------------------------------------------------------------------
/**
 *  @brief  メッセージ管理クラス フォント取得
 *
 *  @param  UN_SELECT_MSG_CNT_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static GFL_FONT* MSG_CNT_GetFont( UN_SELECT_MSG_CNT_WORK* wk )
{
  GF_ASSERT(wk);

  return wk->font;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  メッセージ管理クラス プリントキュー取得
 *
 *  @param  UN_SELECT_MSG_CNT_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static PRINT_QUE* MSG_CNT_GetPrintQue( UN_SELECT_MSG_CNT_WORK* wk )
{
  GF_ASSERT(wk);

  return wk->print_que;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  メッセージ管理クラス メッセージハンドルを取得
 *
 *  @param  UN_SELECT_MSG_CNT_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static GFL_MSGDATA* MSG_CNT_GetMsgData( UN_SELECT_MSG_CNT_WORK* wk )
{
  GF_ASSERT(wk);

  return wk->msghandle;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  ワードセット
 *
 *  @param  UN_SELECT_MSG_CNT_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static WORDSET* MSG_CNT_GetWordset( UN_SELECT_MSG_CNT_WORK* wk )
{
  GF_ASSERT(wk);

  return wk->wordset;
}

//=============================================================================
//
//
// リスト管理クラス
//
//
//=============================================================================
static void ListCallBack_Draw( void * work, u32 itemNum, PRINT_UTIL * util, s16 py, BOOL disp );
static void ListCallBack_Move( void * work, u32 listPos, BOOL flg );
static void ListCallBack_Scroll( void * work, s8 mv );

//--------------------------------------------------------------
/// 
//==============================================================
static const FRAMELIST_CALLBACK FRMListCallBack = {
  ListCallBack_Draw,    // 描画処理
  ListCallBack_Move,    // 移動処理
  ListCallBack_Scroll,  // スクロール
};

//--------------------------------------------------------------
/// 
//==============================================================
static const FRAMELIST_TOUCH_DATA TouchHitTbl[] =
{
  { { 0,    8*5-1,  8*12, 231 }, FRAMELIST_TOUCH_PARAM_ITEM },   // 00: 
  { { 8*5,  8*10-1, 8*12, 231 }, FRAMELIST_TOUCH_PARAM_ITEM },   // 01: 
  { { 8*10, 8*15-1, 8*12, 231 }, FRAMELIST_TOUCH_PARAM_ITEM },   // 02: 
  { { 8*15, 8*20-1, 8*12, 231 }, FRAMELIST_TOUCH_PARAM_ITEM },   // 03: 

  { { SCROLL_BAR_UY, SCROLL_BAR_DY, 240, 255 }, FRAMELIST_TOUCH_PARAM_RAIL },         // 07: レール

  { { 168, 191, 16, 16+3*8-1 }, FRAMELIST_TOUCH_PARAM_PAGE_UP },      // 08: 左
  { { 168, 191, 80, 80+3*8-1 }, FRAMELIST_TOUCH_PARAM_PAGE_DOWN },    // 09: 右

  { { GFL_UI_TP_HIT_END, 0, 0, 0 }, 0 },
};

//-----------------------------------------------------------------------------
/**
 *  @brief  リスト描画
 *
 *  @param  void * work
 *  @param  itemNum
 *  @param  * util
 *  @param  py
 *  @param  disp 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void ListCallBack_Draw( void * work, u32 itemNum, PRINT_UTIL * util, s16 py, BOOL disp )
{ 
  UN_SELECT_MAIN_WORK* wk = work;
  u32 prm;
  
  prm = FRAMELIST_GetItemParam( wk->lwk, itemNum );

  {
    PRINT_QUE * que;
    GFL_FONT* font;
    WORDSET* wset;
    
    que  = FRAMELIST_GetPrintQue( wk->lwk );
    font = MSG_CNT_GetFont( wk->cnt_msg );

    // 各階フロア名描画（国名）
    MSG_CNT_DrawListElem( wk->cnt_msg, util, que, font, itemNum );
  }

  HOSAKA_Printf("draw!\n");
  NOZOMU_Printf("disp %d item %d draw %d\n",disp, itemNum, py);

  //セットする項目は20件リストに入っているかを調べ、該当したら、表示処理を行う
  if ( IsMarkerFloorValid( wk, itemNum ) ) 
  {
    int y = py + (LIST_MARLER_OFS/2); //項目縦幅の真ん中に表示
    SetListMarker(wk, disp, y);
  }
/**
  {
    s16 cur_y;
    cur_y = FRAMELIST_GetScrollBarPY(wk->lwk);
    //スクロールバー移動
    SetScrollBarPos( wk, cur_y );
    //ビルカーソル位置セット
    SetBuilCurPos( wk, cur_y );
  }
*/
}

//-----------------------------------------------------------------------------
/**
 *  @brief
 *
 *  @param  void * work
 *  @param  listPos
 *  @param  flg 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void ListCallBack_Move( void * work, u32 listPos, BOOL flg )
{ 
  UN_SELECT_MAIN_WORK* wk = work;
  HOSAKA_Printf("move!\n");
  {
    s16 cur_y;
    cur_y = FRAMELIST_GetScrollBarPY(wk->lwk);
    //スクロールバー移動
    SetScrollBarPos( wk, cur_y );
    //ビルカーソル位置セット
    SetBuilCurPos( wk, cur_y );
  }
}

//-----------------------------------------------------------------------------
/**
 *  @brief
 *
 *  @param  void * work
 *  @param  mv 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void ListCallBack_Scroll( void * work, s8 mv )
{
  UN_SELECT_MAIN_WORK* wk = work;
  HOSAKA_Printf("scroll!\n");
  //マーカー移動
  MvListMarkerPos( wk, -mv);
  {
    s16 cur_y;
    cur_y = FRAMELIST_GetScrollBarPY(wk->lwk);
    //スクロールバー移動
    SetScrollBarPos( wk, cur_y );
    //ビルカーソル位置セット
    SetBuilCurPos( wk, cur_y );
  }
}

//-----------------------------------------------------------------------------
/**
 *  @brief  リスト生成
 *
 *  @param  UN_SELECT_MAIN_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void LIST_Make( UN_SELECT_MAIN_WORK* wk )
{
  ARCHANDLE* ah;
  int i;

  static FRAMELIST_HEADER header = {
    BG_FRAME_LIST_M,
    BG_FRAME_LIST_S,
    //<<※ここから画像依存
    9,         // 項目フレーム表示開始Ｘ座標 
    0,          // 項目フレーム表示開始Ｙ座標
    20,         // 項目フレーム表示Ｘサイズ
    5,          // 項目フレーム表示Ｙサイズ
    //<<※ここまで画像依存
    1,              // フレーム内に表示するBMPWINの表示Ｘ座標
    0,              // フレーム内に表示するBMPWINの表示Ｙ座標
    18,             // フレーム内に表示するBMPWINの表示Ｘサイズ
    5,              // フレーム内に表示するBMPWINの表示Ｙサイズ
    1,              // フレーム内に表示するBMPWINのパレット

    { 40, 20, 10, 8, 5, 4 },    // スクロール速度 [0] = 最速 ※itemSizYを割り切れる値であること！

    3,              // 選択項目のパレット

    SCROLL_BAR_SIZE,              // スクロールバーのＹサイズ

    UN_LIST_MAX,    // 項目登録数
    2,              // 背景登録数

    0,              // 初期位置
    CUR_MOVE_RANGE,              // カーソル移動範囲
    0,  // 初期スクロール値 　一番上

    TouchHitTbl,      // タッチデータ

    &FRMListCallBack, // コールバック関数
    NULL,
  };
/**
  //受け渡されたフロア数が5Ｆより大きかった場合だった場合初期位置を書き換える
  if ( wk->pwk->InFloor > 5 )
  {
    NOZOMU_Printf("国フロアからアプリがコールされたとみなす\n");
    header.initScroll = (UN_LIST_MAX-1)-(wk->pwk->InFloor-2);
  }
*/
  header.cbWork = wk;

  //初期位置セット
  for (i=0; i<UN_LIST_MAX; i++)
  {
    int floor;
    floor = UN_LIST_MAX - i + 1;
    if( floor == wk->pwk->InFloor )
    {
      if( i > (UN_LIST_MAX-CUR_MOVE_RANGE) )
      {
        header.initPos    = i - ( UN_LIST_MAX-CUR_MOVE_RANGE );
        header.initScroll = i - header.initPos;
      }else{
        header.initScroll = i;
      }
    }
  }

  wk->lwk = FRAMELIST_Create( &header, wk->heap_id );

  // アーカイブ オープン
  ah = GFL_ARC_OpenDataHandle( ARCID_UN_SELECT_GRA, wk->heap_id );

  // 項目背景設定
  FRAMELIST_LoadFrameGraphicAH( wk->lwk, ah, NARC_un_select_gra_kokuren_bg_listframe_NSCR, FALSE, 0 );
  FRAMELIST_LoadFrameGraphicAH( wk->lwk, ah, NARC_un_select_gra_kokuren_bg_listframe2_NSCR, FALSE, 1 );

  // 点滅アニメパレット設定
  FRAMELIST_LoadBlinkPalette( wk->lwk, ah, NARC_un_select_gra_kokuren_bg_NCLR, 2, 3 );

  // アーカイブ クローズ
  GFL_ARC_CloseDataHandle( ah );
        
  // リスト項目生成
  {
    GFL_MSGDATA* mman;
    
    mman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
        NARC_message_wifi_place_msg_UN_dat, wk->heap_id );

    for(i=0; i<UN_LIST_MAX; i++)
    {
      int type;
      int idx;
///>      idx = UN_LIST_MAX - i - 1;
      idx = i;
      if ( wk->Valid[i] == TRUE ) type = 0;
      else type = 1;

      // パラメータは純粋に順列ID
      FRAMELIST_AddItem( wk->lwk, type, idx );

      // 項目あり
      if( type == 0 )
      {
        int country_msg_idx;
        country_msg_idx = g_FloorTable[idx];
        // 項目用文字列 取得
        wk->cnt_msg->name[i] = GFL_MSG_CreateString( mman, country_msg_idx );
      }
      // 項目なし
      else
      {
        // 項目用文字列 取得
        wk->cnt_msg->name[i] = GFL_MSG_CreateString( mman, un_reception_msg_05 );
      }
    }
      
    // メッセージ生成
//    MSG_CNT_CreateNameBuffer( wk->cnt_msg, i, buf );
    
    GFL_MSG_Delete( mman );
  }
}

//-----------------------------------------------------------------------------
/**
 *  @brief  リスト開放
 *
 *  @param  UN_SELECT_MAIN_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void LIST_Delete( UN_SELECT_MAIN_WORK* wk )
{
  int i;
  
  for( i=0; i<UN_LIST_MAX; i++)
  {
    // 項目用文字列 開放
    GFL_STR_DeleteBuffer( wk->cnt_msg->name[i] );
  }
  
  // フレームリスト 開放
  FRAMELIST_Exit( wk->lwk );
}
  
//-----------------------------------------------------------------------------
/**
 *  @brief  PROC 初期化
 *
 *  @param  GFL_PROC* proc
 *  @param  prm 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static UN_SELECT_MAIN_WORK* app_init( GFL_PROC* proc, UN_SELECT_PARAM* prm )
{
  UN_SELECT_MAIN_WORK* wk;

  //オーバーレイ読み込み
  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
  
  //ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_UN_SELECT, UN_SELECT_HEAP_SIZE );
  wk = GFL_PROC_AllocWork( proc, sizeof(UN_SELECT_MAIN_WORK), HEAPID_UN_SELECT );
  GFL_STD_MemClear( wk, sizeof(UN_SELECT_MAIN_WORK) );

  // 初期化
  wk->heap_id = HEAPID_UN_SELECT;
  wk->pwk = prm;

  //解禁国セット
  {
    int i;
    for( i=0; i<UN_LIST_MAX; i++)
    {
      int code_idx;   //国コード-1の値が入る
      int floor_idx;
      code_idx = g_FloorTable[i] - 1;
///>      floor_idx = UN_LIST_MAX - i - 1;
      floor_idx = i;
      if ( prm->OpenCountryFlg[code_idx]) wk->Valid[floor_idx] = TRUE;
      else wk->Valid[floor_idx] = FALSE;
    }
  }
  //マークするフロアをワークに受け取る
  {
    int i;
    int floor_idx;
    for(i=0;i<FLOOR_MARKING_MAX;i++){
      int code;
      code = prm->StayCountry[i];
      if (code != 0) floor_idx = GetFloorIdxFromCountryCode(code);
      else floor_idx = NOT_MARKER;
      wk->MarkerFloor[i] = floor_idx;
    }
  }
  //描画設定初期化
  wk->graphic = UN_SELECT_GRAPHIC_Init( GX_DISP_SELECT_SUB_MAIN, wk->heap_id );

  // メッセージ生成
  wk->cnt_msg = MSG_CNT_Create( wk->heap_id );
  
  // シーンコントローラ作成
  wk->cnt_scene = UI_SCENE_CNT_Create( 
      wk->heap_id, c_scene_func_tbl, UNS_SCENE_ID_MAX, 
      UNS_SCENE_ID_LIST_MAKE, wk );

  //BGリソース読み込み
  UNSelect_BG_LoadResource( wk, wk->heap_id );

#ifdef UN_SELECT_TOUCHBAR
  //タッチバーの初期化
  { 
    GFL_CLUNIT  *clunit = UN_SELECT_GRAPHIC_GetClunit( wk->graphic );
    wk->touchbar  = UNSelect_TOUCHBAR_Init( clunit, wk->heap_id );
  }
#endif //UN_SELECT_TOUCHBAR

  //TASKMENUリソース読み込み＆初期化
  wk->menu_res  = APP_TASKMENU_RES_Create( BG_FRAME_MENU_M, PLTID_BG_TASKMENU_M,
      MSG_CNT_GetFont( wk->cnt_msg ),
      MSG_CNT_GetPrintQue( wk->cnt_msg ),
      wk->heap_id );

  //OBJリソースセットアップ
  SetupRes( wk );

  // リスト生成
  LIST_Make( wk );

  //アクター作成
  MakeAct( wk );

  //ビルマーカー位置決定
  SetBuilMarkerPos( wk );

  {
    //ビルカーソル位置セット
    s16 cur_y;
    cur_y = SCROLL_BAR_DY-(SCROLL_BAR_SIZE/2);
    SetBuilCurPos( wk, cur_y );
  }
  
  //アルファセット
  G2S_SetBlendAlpha(
		GX_BLEND_PLANEMASK_BG2,
		GX_BLEND_PLANEMASK_BG3,
		BLEND_EV1, BLEND_EV2 );

  wk->htask = GFUser_HIntr_CreateTCB( HBlankTask, wk, 0 );

  //通信アイコン
  GFL_NET_WirelessIconEasy_HoldLCD( TRUE, wk->heap_id );
	GFL_NET_ReloadIcon();

  return wk;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  アプリケーション終了処理
 *
 *  @param  UN_SELECT_MAIN_WORK* wk
 *  @param  end_mode 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void app_end( UN_SELECT_MAIN_WORK* wk, END_MODE end_mode )
{
  switch( end_mode )
  {
  case END_MODE_DECIDE:     //フロア選択
    {
      int code;
      int floor;
      int floor_idx;
      //リスト位置をフロアインデックスに変換
///>      floor_idx = (UN_LIST_MAX-1) - wk->ListSelPos;
      floor_idx = wk->ListSelPos;
      //フロア数は2Ｆからなのでインデックスに2を足す
///>      floor = floor_idx+2;
      floor = UN_LIST_MAX - floor_idx;
      //フロアインデックスを国コードに変換
      code = g_FloorTable[floor_idx];

      // データ吐き出し
      wk->pwk->OutFloor = floor;
      wk->pwk->CountryCode = code;
      wk->pwk->Decide = TRUE;
      NOZOMU_Printf("output: Floor = %d Code = %d\n",floor, code);
    }
    break;
  case END_MODE_CANCEL:     //キャンセル
      // データ吐き出し
      wk->pwk->OutFloor = 0;
      wk->pwk->CountryCode = 0;
      wk->pwk->Decide = FALSE;
      NOZOMU_Printf("output: Floor Cansel\n");
    break;
  }
  
  // フェードアウト リクエスト
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 1 );
}


//-----------------------------------------------------------------------------
/**
 *  @brief  SCENE リスト生成 主処理
 *
 *  @param  UI_SCENE_CNT_PTR cnt
 *  @param  work 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneListMake( UI_SCENE_CNT_PTR cnt, void* work )
{
  UN_SELECT_MAIN_WORK* wk = work;
  u8 seq = UI_SCENE_CNT_GetSubSeq( cnt );

  switch( seq )
  {
  case 0:
    // リスト初期化
    if( FRAMELIST_Init( wk->lwk ) == FALSE )
    {
      UI_SCENE_CNT_IncSubSeq( cnt );
    }
    break;
  case 1:
    // フロア選択へ
    UI_SCENE_CNT_SetNextScene( cnt, UNS_SCENE_ID_SELECT_FLOOR );
    HOSAKA_Printf("リスト生成完了\n");
    return TRUE;
  default : GF_ASSERT(0);
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  SCENE フロア選択 初期化処理
 = *
 *  @param  UI_SCENE_CNT_PTR cnt
 *  @param  work 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneSelectFloor_Init( UI_SCENE_CNT_PTR cnt, void* work )
{
  UN_SELECT_MAIN_WORK* wk = work;

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  SCENE フロア選択 主処理
 *
 *  @param  UI_SCENE_CNT_PTR cnt
 *  @param  work 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneSelectFloor_Main( UI_SCENE_CNT_PTR cnt, void* work )
{
  u32 ret;
  UN_SELECT_MAIN_WORK* wk = work;

  // フロア選択処理
  ret = FRAMELIST_Main( wk->lwk );

  if ( (0<=ret)&&(ret<CUR_MOVE_RANGE) )
  {
    int list_pos;
    NOZOMU_Printf("pos=%d\n",ret);
    list_pos = FRAMELIST_GetListPos( wk->lwk );
    NOZOMU_Printf("list_pos=%d\n",list_pos);

    //項目有効判定
    if ( wk->Valid[list_pos] == TRUE )
    {
      int scene;
      int floor;
      wk->ListSelPos = list_pos;
      floor = UN_LIST_MAX - list_pos + 1;
      //選んだフロアがアプリ起動時に受け渡したフロアと同じ場合は飛べないことを警告するメッセージを出す
      if ( wk->pwk->InFloor == floor ) scene = UNS_SCENE_ID_JUMP_NG;
      else scene = UNS_SCENE_ID_CONFIRM;

      UI_SCENE_CNT_SetNextScene( cnt, scene );
      return TRUE;
    }
    else
    {
      //項目無効
      NOZOMU_Printf("Invalid list_pos = %d\n",list_pos);
    }
  }

#ifdef UN_SELECT_TOUCHBAR
  //タッチバーメイン処理
  UNSelect_TOUCHBAR_Main( wk->touchbar );
#endif //UN_SELECT_TOUCHBAR

  switch( TOUCHBAR_GetTrg( wk->touchbar ) )
  {
  case TOUCHBAR_ICON_CUR_L : // ←
    //@TODO
    HOSAKA_Printf("L!\n");
    break;

  case TOUCHBAR_ICON_CUR_R : // →
    //@TODO
    HOSAKA_Printf("R!\n"); 
    break;

  case TOUCHBAR_ICON_RETURN : // リターンボタン
    // 終了
    app_end( wk, END_MODE_CANCEL ); 
    UI_SCENE_CNT_SetNextScene( cnt, UI_SCENE_ID_END );
    return TRUE;

  case TOUCHBAR_SELECT_NONE : // 選択なし
    break;

  default : GF_ASSERT(0);
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  SCENE フロア選択 後処理
 *
 *  @param  UI_SCENE_CNT_PTR cnt
 *  @param  work 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneSelectFloor_End( UI_SCENE_CNT_PTR cnt, void* work )
{
  UN_SELECT_MAIN_WORK* wk = work;

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  SCENE 確認画面 初期化処理
 *
 *  @param  UI_SCENE_CNT_PTR cnt
 *  @param  work 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneConfirm_Init( UI_SCENE_CNT_PTR cnt, void* work )
{
  UN_SELECT_MAIN_WORK* wk = work;
  u8 seq = UI_SCENE_CNT_GetSubSeq( cnt );

  switch(seq)
  {
  case 0:
    // パッシブ状態に遷移
    PASSIVE_Request();

#if 0
    TOUCHBAR_SetVisible( wk->touchbar, TOUCHBAR_ICON_CUR_L, FALSE );
    TOUCHBAR_SetVisible( wk->touchbar, TOUCHBAR_ICON_CUR_R, FALSE );i++;
    ;i++TOUCHBAR_SetVisible( wk->touchbar, TOUCHBAR_ICON_RETURN, FALSE );
#endif
    {
      int code;
      //リスト位置を国コードに変換
///>      code = g_FloorTable[(UN_LIST_MAX-1) - wk->ListSelPos];
      code = g_FloorTable[wk->ListSelPos];
      if ( (0<code)&&(code<UN_LIST_MAX) )
      {
        //国名タグ展開
        WORDSET_RegisterCountryName( wk->cnt_msg->wordset, 0, code );
      }
      else GF_ASSERT_MSG(0,"listpos = %d code = %d",wk->ListSelPos, code);
    }
    MSG_CNT_SetPrint( wk->cnt_msg, un_reception_msg_01 );
    UI_SCENE_CNT_IncSubSeq( cnt );
    break;

  case 1:
    if( MSG_CNT_PrintProc(wk->cnt_msg) )
    {
      UI_SCENE_CNT_IncSubSeq( cnt );
    }
    break;

  case 2:
    // タスクメニュー表示
    wk->menu = UNSelect_TASKMENU_Init( 
        wk->menu_res, 
        MSG_CNT_GetMsgData(wk->cnt_msg), 
        wk->heap_id );

    return TRUE;

  default : GF_ASSERT(0);
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  SCENE 確認画面 主処理
 *
 *  @param  UI_SCENE_CNT_PTR cnt
 *  @param  work 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneConfirm_Main( UI_SCENE_CNT_PTR cnt, void* work )
{
  UN_SELECT_MAIN_WORK* wk = work;

  //タスクメニューメイン処理
  UNSelect_TASKMENU_Main( wk->menu );

  if( APP_TASKMENU_IsFinish( wk->menu ) )
  {
    u8 pos = APP_TASKMENU_GetCursorPos( wk->menu );

    switch( pos )
    {
    case 0 :
      // 終了
      app_end( wk, END_MODE_DECIDE );
      PASSIVE_Reset();
      MSG_CNT_PrintClear( wk->cnt_msg );
      UNSelect_TASKMENU_Exit( wk->menu );
      UI_SCENE_CNT_SetNextScene( cnt, UI_SCENE_ID_END );
      break;

    case 1 :
      // 選択画面に戻る
      PASSIVE_Reset();
      MSG_CNT_PrintClear( wk->cnt_msg );
      UNSelect_TASKMENU_Exit( wk->menu );
      UI_SCENE_CNT_SetNextScene( cnt, UNS_SCENE_ID_SELECT_FLOOR );
      break;

    default : GF_ASSERT(0);
    }

    return TRUE;
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  SCENE 確認画面 後処理
 *
 *  @param  UI_SCENE_CNT_PTR cnt
 *  @param  work 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneConfirm_End( UI_SCENE_CNT_PTR cnt, void* work )
{
  UN_SELECT_MAIN_WORK* wk = work;

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  SCENE ジャンプ不可メッセージ出力
 *
 *  @param  UI_SCENE_CNT_PTR cnt
 *  @param  work 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneJumpNG_Init( UI_SCENE_CNT_PTR cnt, void* work )
{
  UN_SELECT_MAIN_WORK* wk = work;
  u8 seq = UI_SCENE_CNT_GetSubSeq( cnt );

  switch(seq)
  {
  case 0:
    // パッシブ状態に遷移
    PASSIVE_Request();

    MSG_CNT_SetPrint( wk->cnt_msg, un_reception_msg_06 );
    UI_SCENE_CNT_IncSubSeq( cnt );
    break;

  case 1:
    return TRUE;
  default : GF_ASSERT(0);
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  SCENE 確認画面 主処理
 *
 *  @param  UI_SCENE_CNT_PTR cnt
 *  @param  work 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneJumpNG_Main( UI_SCENE_CNT_PTR cnt, void* work )
{
  UN_SELECT_MAIN_WORK* wk = work;

  if( MSG_CNT_PrintProc(wk->cnt_msg) ) return TRUE;

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  SCENE　ジャンプＮＧ 後処理
 *
 *  @param  UI_SCENE_CNT_PTR cnt
 *  @param  work 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneJumpNG_End( UI_SCENE_CNT_PTR cnt, void* work )
{
  UN_SELECT_MAIN_WORK* wk = work;

  // 選択画面に戻る
  PASSIVE_Reset();
  MSG_CNT_PrintClear( wk->cnt_msg );
  UI_SCENE_CNT_SetNextScene( cnt, UNS_SCENE_ID_SELECT_FLOOR );
  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  Ｈブランク処理
 *
 *  @param  tcb
 *  @param  work 
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static void HBlankTask( GFL_TCB * tcb, void * work )
{
	s32	vcount = GX_GetVCount();

#if 1  
  int base = 4*8;

	if( vcount >= base+LIST_MARLER_OFS*3 ){
		G2S_ChangeBlendAlpha( 11, 5 );
	}else if( vcount >= base+LIST_MARLER_OFS*2 ){
		G2S_ChangeBlendAlpha( 10, 6 );
	}else if( vcount >= base+LIST_MARLER_OFS*1 ){
		G2S_ChangeBlendAlpha( 9, 7 );
	}else if( vcount >= base ){
		G2S_ChangeBlendAlpha( 8, 8 );
	}else{
		G2S_ChangeBlendAlpha( 6, 10 );
	}
#else
  if( vcount >= 168 ){
		G2S_ChangeBlendAlpha( 11, 5 );
	}else if( vcount >= 144 ){
		G2S_ChangeBlendAlpha( 10, 6 );
	}else if( vcount >= 120 ){
		G2S_ChangeBlendAlpha( 9, 7 );
	}else if( vcount >= 96 ){
		G2S_ChangeBlendAlpha( 8, 8 );
	}else if( vcount >= 72 ){
		G2S_ChangeBlendAlpha( 6, 10 );
	}else{
		G2S_ChangeBlendAlpha( 4, 12 );
	}
#endif
}

//////////////////////////////////////////////////
//OBJ
#define	RES_NONE	( 0xffffffff )		// リソースが読み込まれていない

#define LIST_MARKER_X (29*8)
#define BUIL_MARKER_X (32+8)
#define BUIL_CUR_X (8)
#define SCROLL_BAR_X (256-10)

#define SOFT_PRI  (0)
#define OBJ_BG_PRI  (2)

//セルアニメオフセット定義
typedef enum{
  ANM_OFS_BUIL_MARKER = 0,
  ANM_OFS_LIST_MARKER,
  ANM_OFS_LIST_CURSOR,
  ANM_OFS_SCROLL_BAR,
}OBJ_ANM_OFS;

//リソースのセットアップ
static void SetupRes( UN_SELECT_MAIN_WORK *wk )
{
  ARCHANDLE *handle;
	u32 * chr;
	u32 * pal;
	u32 * cel;
  int i;

  HEAPID heapID = wk->heap_id;

  // 初期化
	for( i=0; i<UN_OBJ_CHRRES_MAX; i++ ){
		wk->ChrRes[i] = RES_NONE;
	}
	for( i=0; i<UN_OBJ_PALRES_MAX; i++ ){
		wk->PalRes[i] = RES_NONE;
	}
	for( i=0; i<UN_OBJ_CELRES_MAX; i++ ){
		wk->CelRes[i] = RES_NONE;
	}

  handle = GFL_ARC_OpenDataHandle( ARCID_UN_SELECT_GRA, heapID );
	chr = wk->ChrRes;
	pal = wk->PalRes;
	cel = wk->CelRes;
	*chr = GFL_CLGRP_CGR_Register(
					handle, NARC_un_select_gra_kokuren_obj_NCGR,
					FALSE, CLSYS_DRAW_MAX, heapID );
  *pal = GFL_CLGRP_PLTT_Register(
					handle, NARC_un_select_gra_kokuren_obj_NCLR,
					CLSYS_DRAW_MAX, PLTID_OBJ_UN_MS*0x20, heapID ); //両画面
	*cel = GFL_CLGRP_CELLANIM_Register(
					handle,
					NARC_un_select_gra_kokuren_obj_NCER,
					NARC_un_select_gra_kokuren_obj_NANR,
					heapID );
	GFL_ARC_CloseDataHandle( handle );
}

//リソースの解放
static void ExitRes( UN_SELECT_MAIN_WORK *wk )
{
	int	i;
	for( i=0; i<UN_OBJ_CHRRES_MAX; i++ ){
		if( wk->ChrRes[i] != RES_NONE ){
			GFL_CLGRP_CGR_Release( wk->ChrRes[i] );
		}
	}
	for( i=0; i<UN_OBJ_PALRES_MAX; i++ ){
		if( wk->PalRes[i] != RES_NONE ){
	    GFL_CLGRP_PLTT_Release( wk->PalRes[i] );
		}
	}
	for( i=0; i<UN_OBJ_CELRES_MAX; i++ ){
		if( wk->CelRes[i] != RES_NONE ){
	    GFL_CLGRP_CELLANIM_Release( wk->CelRes[i] );
		}
	}
}

static void MakeAct( UN_SELECT_MAIN_WORK *wk )
{
  int i;
  GFL_CLUNIT *unit;
  unit = UN_SELECT_GRAPHIC_GetClunit( wk->graphic );
  GF_ASSERT(unit != NULL);
  
  //初期化
  for( i=0; i<UN_OBJ_MAX; i++ ){
		wk->ClWk[i] = NULL;
	}

  //ビルカーソル
  {
    UN_CLWK_DATA data = {
      { BUIL_CUR_X, 104, ANM_OFS_LIST_CURSOR, SOFT_PRI, OBJ_BG_PRI },
      0,0,0,
      PLTID_OBJ_UN_MS, CLWK_SETSF_NONE
    };
    data.ChrRes = wk->ChrRes[0];
    data.PalRes = wk->PalRes[0];
    data.CelRes = wk->CelRes[0];
    wk->ClWk[UN_OBJ_BUIL_CURSOR] = CreateAct( unit, &data );
  }

  //ビルマーカー
  {
    UN_CLWK_DATA data = {
      { BUIL_MARKER_X, 0, ANM_OFS_BUIL_MARKER, SOFT_PRI, OBJ_BG_PRI },
      0,0,0,
      PLTID_OBJ_UN_MS, CLWK_SETSF_NONE
    };
    data.ChrRes = wk->ChrRes[0];
    data.PalRes = wk->PalRes[0];
    data.CelRes = wk->CelRes[0];
    for (i=0;i<UN_BUILMAKER_MAX;i++)
    {
      wk->ClWk[UN_OBJ_BUIL_MARKER_START+i] = CreateAct( unit, &data );
    }
  }
  //リストマーカーメイン画面
  {
    UN_CLWK_DATA data = {
      { LIST_MARKER_X, 0, ANM_OFS_LIST_MARKER, SOFT_PRI, OBJ_BG_PRI },
      0,0,0,
      PLTID_OBJ_UN_MS, CLSYS_DEFREND_MAIN
    };
    data.ChrRes = wk->ChrRes[0];
    data.PalRes = wk->PalRes[0];
    data.CelRes = wk->CelRes[0];
    for (i=0;i<UN_LISTMAKER_MAIN_MAX;i++)
    {
      GFL_CLWK *clwk;
      clwk = CreateAct( unit, &data );
      //アクター非表示
      GFL_CLACT_WK_SetDrawEnable( clwk, FALSE );
      wk->ClWk[UN_OBJ_LIST_MARKER_M_START+i] = clwk;
    }
  }
  //リストマーカーサブ画面
  {
    UN_CLWK_DATA data = {
      { LIST_MARKER_X, 0, ANM_OFS_LIST_MARKER, SOFT_PRI, OBJ_BG_PRI },
      0,0,0,
      PLTID_OBJ_UN_MS, CLSYS_DEFREND_SUB
    };
    data.ChrRes = wk->ChrRes[0];
    data.PalRes = wk->PalRes[0];
    data.CelRes = wk->CelRes[0];
    for (i=0;i<UN_LISTMAKER_SUB_MAX;i++)
    {
      GFL_CLWK *clwk;
      clwk = CreateAct( unit, &data );
      //半透明
      GFL_CLACT_WK_SetObjMode( clwk, GX_OAM_MODE_XLU );
      //アクター非表示
      GFL_CLACT_WK_SetDrawEnable( clwk, FALSE );
      wk->ClWk[UN_OBJ_LIST_MARKER_S_START+i] = clwk;
    }
  }

  //スクロールバー
  {
    //アプリ開始時は、5Ｆをさす状態なので、
    //Ｙ値にバーが一番下に来るSCROLL_BAR_DY-(SCROLL_BAR_SIZE/2)をセットする
    UN_CLWK_DATA data = {
      { SCROLL_BAR_X, SCROLL_BAR_DY-(SCROLL_BAR_SIZE/2), ANM_OFS_SCROLL_BAR, SOFT_PRI, OBJ_BG_PRI },
      0,0,0,
      PLTID_OBJ_UN_MS, CLSYS_DEFREND_MAIN
    };
    data.ChrRes = wk->ChrRes[0];
    data.PalRes = wk->PalRes[0];
    data.CelRes = wk->CelRes[0];
    wk->ClWk[UN_OBJ_SCROLL_BAR] = CreateAct( unit, &data );
  }
}

static void DelAct( UN_SELECT_MAIN_WORK *wk )
{
  int	i;
	for( i=0; i<UN_OBJ_MAX; i++ ){
    if( wk->ClWk[i] != NULL ){
      GFL_CLACT_WK_Remove( wk->ClWk[i] );
    }
	}
}

//アクターの作成
static GFL_CLWK *CreateAct( GFL_CLUNIT *unit, UN_CLWK_DATA *data )
{
  GFL_CLWK *clwk;
  HEAPID heapID = HEAPID_UN_SELECT;
  clwk = GFL_CLACT_WK_Create( unit, data->ChrRes, data->PalRes, data->CelRes, &data->Dat, data->Disp, heapID );
  GFL_CLACT_WK_SetAutoAnmFlag( clwk, TRUE );
  return clwk;
}

//マーカーの管理位置座標を更新
static void MvListMarkerPos( UN_SELECT_MAIN_WORK *wk, const int inAddVal)
{
  int i;
  GFL_CLACTPOS calc_pos;
  int ofs = (LIST_MARLER_OFS/2)+((5-24%5)*8);
  //(5-24%5)は縦5キャラの項目を埋め尽くしたとき、
  //見切れたリストの上（下）に項目追加しようとしたときのアクターの表示位置オフセット

  for (i=0;i<UN_LISTMAKER_MAIN_MAX;i++)
  {
    int pos;
    //アクター取得
    GFL_CLWK *clwk = wk->ClWk[UN_OBJ_LIST_MARKER_M_START+i];
    //使用してるか？
    if ( GFL_CLACT_WK_GetDrawEnable( clwk ) )
    {
      //座標取得
      GFL_CLACT_WK_GetPos( clwk, &calc_pos, CLSYS_DEFREND_MAIN ); //画面内座標指定
      calc_pos.y += inAddVal;
      pos = calc_pos.y;
      GFL_CLACT_WK_SetPos( clwk, &calc_pos, CLSYS_DEFREND_MAIN ); //画面内座標指定
      //管理座標は画面外（上下少し余裕を持つ範囲外）か？
      if ( (pos<=0-(LIST_MARLER_OFS/2))||(192+ofs<=pos) )
      {
        NOZOMU_Printf("main_vanish pos %d\n",pos);
        //非表示
        GFL_CLACT_WK_SetDrawEnable( clwk, FALSE );
      }
    }
  }

  for (i=0;i<UN_LISTMAKER_SUB_MAX;i++)
  {
    int pos;
    //アクター取得
    GFL_CLWK *clwk = wk->ClWk[UN_OBJ_LIST_MARKER_S_START+i];
    //使用してるか？
    if ( GFL_CLACT_WK_GetDrawEnable( clwk ) )
    {
      //座標取得
      GFL_CLACT_WK_GetPos( clwk, &calc_pos, CLSYS_DEFREND_SUB ); //画面内座標指定
      calc_pos.y += inAddVal;
      pos = calc_pos.y;
      GFL_CLACT_WK_SetPos( clwk, &calc_pos, CLSYS_DEFREND_SUB ); //画面内座標指定
      //管理座標は画面外（上下少し余裕を持つ範囲外）か？
      if ( (pos<=0-ofs)||(192+(LIST_MARLER_OFS/2)<=pos) )
      {
        NOZOMU_Printf("sub_vanish pos %d\n",pos);
        //非表示
        GFL_CLACT_WK_SetDrawEnable( clwk, FALSE );
      }
    }
  }
}


//ビルマーカーの位置をセット
static void SetBuilMarkerPos( UN_SELECT_MAIN_WORK *wk )
{
  int i;
  for (i=0;i<UN_BUILMAKER_MAX;i++)
  {
    GFL_CLACTPOS calc_pos;
    //アクター取得
    GFL_CLWK *clwk = wk->ClWk[UN_OBJ_BUIL_MARKER_START+i];

    if ( wk->MarkerFloor[i] == NOT_MARKER )
    {
      //非表示
      GFL_CLACT_WK_SetDrawEnable( clwk, FALSE );
      continue;
    }

    //座標セット
    GFL_CLACT_WK_GetPos( clwk, &calc_pos, CLWK_SETSF_NONE ); //絶対座標指定
    calc_pos.y = BUIL_FLOOR_YDOT_TOP + wk->MarkerFloor[i];
    GFL_CLACT_WK_SetPos( clwk, &calc_pos, CLWK_SETSF_NONE ); //絶対座標指定
  }
}

//スクロールバーの位置をセット
static void SetScrollBarPos( UN_SELECT_MAIN_WORK *wk, const int inY )
{
  s16 y;
  s16 w;
  GFL_CLACTPOS	pos;
  GFL_CLWK *clwk = wk->ClWk[UN_OBJ_SCROLL_BAR];
  y = inY;
  w = SCROLL_BAR_SIZE/2;
  if( y < SCROLL_BAR_UY+w ){
		y = SCROLL_BAR_UY + w;
	}else if( y > SCROLL_BAR_DY-w ){
		y = SCROLL_BAR_DY - w;
	}
  GFL_CLACT_WK_GetPos( clwk, &pos, CLSYS_DEFREND_MAIN ); //メイン画面内指定
	pos.y = y;
	GFL_CLACT_WK_SetPos( clwk, &pos, CLSYS_DEFREND_MAIN ); //メイン画面内指定

  NOZOMU_Printf("bar_y %d\n",y);
}

//ビルカーソルの位置をセット
static void SetBuilCurPos( UN_SELECT_MAIN_WORK *wk, const int inY )
{
  s16 y;
  s16 w;
  s16 pos_y;
  GFL_CLACTPOS	pos;
  GFL_CLWK *clwk = wk->ClWk[UN_OBJ_BUIL_CURSOR];
  y = inY;
  w = SCROLL_BAR_SIZE/2;
  if( y < SCROLL_BAR_UY+w ){
		y = SCROLL_BAR_UY + w;
	}else if( y > SCROLL_BAR_DY-w ){
		y = SCROLL_BAR_DY - w;
	}

  {
    int numerator;      //スクロールバー位置の割合分子
    int denominator;    //スクロールバー位置の割合分母
    int band;           //ビルカーソル移動範囲
    int val;            //計算結果
    int scr_w = SCROLL_BAR_SIZE/2;
    int cur_w = BUIL_CUR_SIZE/2;
    numerator = y - (SCROLL_BAR_UY+scr_w);
    denominator = (SCROLL_BAR_DY-scr_w) - (SCROLL_BAR_UY+scr_w);
    band = (BUIL_FLOOR_YDOT_BOTTOM-cur_w) - (BUIL_FLOOR_YDOT_TOP+cur_w)+1;
    val = (band * numerator) / denominator;
    pos_y = BUIL_FLOOR_YDOT_TOP + cur_w + val;
  }
  GFL_CLACT_WK_GetPos( clwk, &pos, CLWK_SETSF_NONE ); //絶対座標指定
	pos.y = pos_y;
	GFL_CLACT_WK_SetPos( clwk, &pos, CLWK_SETSF_NONE ); //絶対座標指定
}

static void SetupListMarker( UN_SELECT_MAIN_WORK* wk, int target_item )
{
  int i, j;
  int main_list[UN_LISTMAKER_MAIN_MAX];
  int sub_list[UN_LISTMAKER_SUB_MAX];
  int main_base, sub_base;

  //指定基底位置からリストに表示される対象項目をリストアップ
  {
    int base;
    int idx;
    
    main_base = target_item;
    sub_base = target_item-1;

    //メイン画面
    base = target_item - 1;
    for (i=0;i<UN_LISTMAKER_MAIN_MAX;i++)
    {
      idx = base + i;
      main_list[i] = idx;
    }

    //サブ画面対象インデックス

    base = target_item - (UN_LISTMAKER_SUB_MAX-1);
    for (i=0;i<UN_LISTMAKER_SUB_MAX;i++)
    {
      idx = base + i;
      sub_list[i] = idx;
    }
  }

  //今回の20件のインデックスがリストに入っているかを調べる
  {
    int y;
    int ofs;
    for (i=0;i<FLOOR_MARKING_MAX;i++)
    {
      int floor_idx;
      int item_idx;
      floor_idx = wk->MarkerFloor[i];
      if (floor_idx == NOT_MARKER){
        continue;
      }
///>      item_idx = UN_LIST_MAX - floor_idx - 1;
      item_idx = floor_idx;
      //サブ
      for(j=0;j<UN_LISTMARKER_SETUP_MAX;j++)
      {
        if (item_idx == sub_list[i])
        {
          ofs = (item_idx - sub_base) * LIST_MARLER_OFS;
          y = LIST_MARLER_BASE_POS_SUB + ofs;
          //サブ画面アクターセット
          SetListMarker(wk, FALSE, y);
          break;
        }
      }
      //メイン
      for(j=0;j<UN_LISTMARKER_SETUP_MAX;j++)
      {
        if (item_idx == main_list[i])
        {
          //メイン画面アクターセット
          ofs = (item_idx - main_base) * LIST_MARLER_OFS;
          y = LIST_MARLER_BASE_POS_MAIN + ofs;
          SetListMarker(wk, TRUE, y);
          break;
        }
      }
    }
  }
}

static void SetListMarker(UN_SELECT_MAIN_WORK* wk, const BOOL inIsMain, const int inY)
{
  int i;
  GFL_CLWK **clwk_org;
  u16 setsf;
  int num;
  if (inIsMain)
  {
    setsf = CLSYS_DEFREND_MAIN;
    clwk_org = &wk->ClWk[UN_OBJ_LIST_MARKER_M_START];
    num = UN_LISTMAKER_MAIN_MAX;
  }
  else
  {
    setsf = CLSYS_DEFREND_SUB;
    clwk_org = &wk->ClWk[UN_OBJ_LIST_MARKER_S_START];
    num = UN_LISTMAKER_SUB_MAX;
  }

  for (i=0;i<num;i++)
  {
    BOOL rc;
    rc = SetListMarkerCore(clwk_org, i, setsf, inY);
    if (rc) break;
  }
  GF_ASSERT( i != num);
}

static BOOL SetListMarkerCore(GFL_CLWK **clwk_ary, const int inIdx, const u16 inSetsf, const int inY)
{
  GFL_CLACTPOS pos;
  BOOL rc = FALSE;
  GFL_CLWK *clwk = clwk_ary[inIdx];
  
  if ( GFL_CLACT_WK_GetDrawEnable(clwk) == FALSE )
  {
    //表示
    GFL_CLACT_WK_SetDrawEnable( clwk, TRUE );
    //座標セット
    GFL_CLACT_WK_GetPos( clwk, &pos, inSetsf ); //画面内座標指定
    pos.y = inY;
    GFL_CLACT_WK_SetPos( clwk, &pos, inSetsf ); //画面内座標指定
    rc = TRUE;
  }
  return rc;
}

static BOOL IsMarkerFloorValid( UN_SELECT_MAIN_WORK *wk, const int inTargetItem )
{
  int i;
  //リストインデックスをフロアインデックスに変換
///>  int floor_idx = UN_LIST_MAX - inTargetItem - 1;
  int floor_idx = inTargetItem;
  for (i=0;i<FLOOR_MARKING_MAX;i++)
  {
    if ( wk->MarkerFloor[i] == floor_idx ) return TRUE;
  }

  return FALSE;
}

//国コードからフロアインデックスを取得
static int GetFloorIdxFromCountryCode(const int inCountryCode)
{
  int i;
  for (i=0;i<UN_LIST_MAX;i++)
  {
    if ( inCountryCode == g_FloorTable[i] ) break;
  }

  if ( i >= UN_LIST_MAX )
  {
    GF_ASSERT_MSG( 0,"code not found %d",inCountryCode );
    return 0;
  }
  return i;
}
