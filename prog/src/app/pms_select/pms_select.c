//=============================================================================
/**
 *
 *	@file		pms_select.c
 *	@brief  登録済み簡易会話選択画面
 *	@author	hosaka genya
 *	@data		2009.10.20
 *
 */
//=============================================================================

//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "savedata/save_control.h"

// 簡易会話表示システム
#include "system/pms_draw.h"

//FONT
#include "print/gf_font.h"
#include "font/font.naix"

#include "system/brightness.h"

//PRINT_QUE
#include "print/printsys.h"
#include "print/wordset.h"

//描画設定
#include "pms_select_graphic.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"

#include "ui/ui_scene.h"

//タッチバー
#include "ui/touchbar.h"

//タスクメニュー
#include "app/app_taskmenu.h"

//アプリ共通素材
#include "app/app_menu_common.h"

//アーカイブ
#include "arc_def.h"
#include "pmsi.naix"
#include "townmap_gra.naix"		// タッチバーカスタムボタン用サンプルにタウンマップリソース
#include "message.naix"
#include "msg/msg_pms_input.h"

//SE
#include "sound/pm_sndsys.h"
#include "pms_select_sound_def.h"

// 入力画面
#include "app/pms_input.h"

//外部公開
#include "app/pms_select.h"

//=============================================================================
// 下記defineをコメントアウトすると、機能を取り除けます
//=============================================================================
#define PMS_SELECT_TOUCHBAR
#define PMS_SELECT_TASKMENU
#define PMS_SELECT_PMSDRAW       // 簡易会話表示

FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================

enum
{ 
  PMS_SELECT_HEAP_SIZE = 0x1c000,  ///< ヒープサイズ

  SELECT_ID_NULL = 255,
};

//--------------------------------------------------------------
///	OBJ
//==============================================================
enum
{ 
  LIST_BAR_STD_PX = 8 * 30,
  LIST_BAR_STD_PY = 8 * 2,
  LIST_BAR_ANM_IDX = 16, // バーアイコンのアニメIDX

  LIST_BAR_SY = 26,
  LIST_BAR_SY_CENTER = LIST_BAR_SY / 2,

  LIST_BAR_TOUCH_MIN = LIST_BAR_STD_PY,
  LIST_BAR_TOUCH_MAX = GX_LCD_SIZE_Y - 8*4,

  LIST_BAR_AREA_MIN  = LIST_BAR_TOUCH_MIN,
  LIST_BAR_AREA_MAX  = LIST_BAR_TOUCH_MAX - LIST_BAR_SY_CENTER,
  LIST_BAR_AREA_SIZE = LIST_BAR_AREA_MAX - LIST_BAR_AREA_MIN,

  LIST_BAR_ENABLE_LISTNUM = 3,
};

//--------------------------------------------------------------
///	BG
//==============================================================
enum
{ 
  PLATE_BG_SX = 29,
  PLATE_BG_SY = 6,
  PLATE_BG_START_PY = 1,
  PLATE_BG_PLTT_OFS_BASE = 0x2,
};

//--------------------------------------------------------------
///	シーケンス
//==============================================================

//--------------------------------------------------------------
///	SceneFunc
//==============================================================
// プレート選択
static BOOL ScenePlateSelect_Init( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL ScenePlateSelect_Main( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL ScenePlateSelect_End( UI_SCENE_CNT_PTR cnt, void* work );
// コマンド選択
static BOOL SceneCmdSelect_Init( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL SceneCmdSelect_Main( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL SceneCmdSelect_End( UI_SCENE_CNT_PTR cnt, void* work );
// 入力画面呼び出し＆復帰
static BOOL SceneCallEdit_Main( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL SceneCallEdit_End( UI_SCENE_CNT_PTR cnt, void* work );

//--------------------------------------------------------------
///	SceneID
//==============================================================
typedef enum
{ 
  PMSS_SCENE_ID_PLATE_SELECT = 0, ///< プレート選択
  PMSS_SCENE_ID_CMD_SELECT,       ///< メニュー
  PMSS_SCENE_ID_CALL_EDIT,        ///< 入力画面呼び出し＆復帰

  PMSS_SCENE_ID_MAX,
} PMSS_SCENE;

//--------------------------------------------------------------
///	SceneTable
//==============================================================
static const UI_SCENE_FUNC_SET c_scene_func_tbl[ PMSS_SCENE_ID_MAX ] = 
{ 
  // PMSS_SCENE_ID_PLATE_SELECT 
  {
    ScenePlateSelect_Init, 
    NULL,
    ScenePlateSelect_Main, 
    NULL, 
    ScenePlateSelect_End,
  },
  // PMSS_SCENE_ID_CMD_SELECT
  {
    SceneCmdSelect_Init,
    NULL,
    SceneCmdSelect_Main, 
    NULL,
    SceneCmdSelect_End,
  },
  // PMSS_SCENE_ID_CALL_EDIT
  {
    NULL, NULL,
    SceneCallEdit_Main,
    NULL, 
    SceneCallEdit_End,
  },
};

//--------------------------------------------------------------
///	タスクメニューID
//==============================================================
enum
{ 
  TASKMENU_ID_DECIDE,
  TASKMENU_ID_CALL_EDIT,
  TASKMENU_ID_CANCEL,
};

//-------------------------------------
///	フレーム
//=====================================
enum
{	
  // MAIN
	BG_FRAME_MENU_M	  = GFL_BG_FRAME0_M,
	BG_FRAME_BAR_M	  = GFL_BG_FRAME1_M,
//	BG_FRAME_TEXT_M	= GFL_BG_FRAME1_M,
	BG_FRAME_TEXT_M	  = GFL_BG_FRAME2_M,
	BG_FRAME_BACK_M	  = GFL_BG_FRAME3_M,
  // SUB
	BG_FRAME_BACK_S	= GFL_BG_FRAME2_S,
  BG_FRAME_TEXT_S = GFL_BG_FRAME0_S, 

  BG_MENU_BRIGHT = -8,
  BG_MENU_MASK_PLANE = (PLANEMASK_BG1|PLANEMASK_BG2|PLANEMASK_BG3|PLANEMASK_OBJ),
  BG_MENU_MASK_DISP = (MASK_MAIN_DISPLAY),
};
//-------------------------------------
///	パレット
//=====================================
enum
{	
	//メインBG
	PLTID_BG_COMMON_M			= 0,
  PLTID_BG_MSG_M        = 7,
	PLTID_BG_TASKMENU_M		= 12,
	PLTID_BG_TOUCHBAR_M		= 14,

	//サブBG
	PLTID_BG_BACK_S				=	0,

	//メインOBJ
	PLTID_OBJ_COMMON_M    = 0, // 3本使用
  PLTID_OBJ_TOUCHBAR_M	= 3, // 3本使用
  PLTID_OBJ_PMS_DRAW    = 6, // 5本使用 
	//サブOBJ
};

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================
//--------------------------------------------------------------
///	BG管理ワーク
//==============================================================
typedef struct 
{
  GFL_ARCUTIL_TRANSINFO m_back_chr_pos;
  void*             plateScrnWork;
  NNSG2dScreenData* platescrnData;
  u16   transAnmCnt;
  GXRgb transCol;
} PMS_SELECT_BG_WORK;

#ifdef PMS_SELECT_PMSDRAW
#define PMS_SELECT_PMSDRAW_NUM (4) ///< 簡易会話の個数
#endif // PMS_SELECT_PMSDRAW 

//--------------------------------------------------------------
///	メインワーク
//==============================================================
typedef struct 
{
  //[IN]
  PMS_SELECT_PARAM* out_param;

  //[PRIVATE]
  HEAPID heapID;

  GFL_CLWK*               clwk_bar;
  UI_EASY_CLWK_RES        clwkres;

  SAVE_CONTROL_WORK*      save_ctrl;
  PMSW_SAVEDATA*          pmsw_save;

	PMS_SELECT_BG_WORK			wk_bg;

	//描画設定
	PMS_SELECT_GRAPHIC_WORK	*graphic;

#ifdef PMS_SELECT_TOUCHBAR
	//タッチバー
	TOUCHBAR_WORK							*touchbar;
#endif //PMS_SELECT_TOUCHBAR

	//フォント
	GFL_FONT									*font;

	//プリントキュー
	PRINT_QUE									*print_que;
	GFL_MSGDATA								*msg;

#ifdef PMS_SELECT_TASKMENU
	//タスクメニュー
	APP_TASKMENU_RES					*menu_res;
	APP_TASKMENU_WORK					*menu;
#endif //PMS_SELECT_TASKMENU

#ifdef PMS_SELECT_PMSDRAW
  GFL_BMPWIN*               pms_win[ PMS_SELECT_PMSDRAW_NUM ];
  BOOL                      pms_win_tranReq[ PMS_SELECT_PMSDRAW_NUM ];
  PMS_DRAW_WORK*            pms_draw;
#endif //PMS_SELECT_PMSDRAW

  UI_SCENE_CNT_PTR          cntScene;
  void*                     subproc_work;   ///< サブPROC用ワーク保存領域
  u8    select_id;        ///< 選択したID
  u8    list_head_id;     ///< リストの先頭ID
  u8    list_max;         ///< リストの項目数
  u8    b_listbar : 1;    ///< リストバーをタッチしているかフラグ
  u8    padding_bit : 7;

  BOOL bProcChange; ///< PROC切替フラグ

} PMS_SELECT_MAIN_WORK;


//=============================================================================
/**
 *							データ定義
 */
//=============================================================================


//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
static GFL_PROC_RESULT PMSSelectProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT PMSSelectProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT PMSSelectProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
//-------------------------------------
///	汎用処理ユーティリティ
//=====================================
static void PMSSelect_GRAPHIC_Load( PMS_SELECT_MAIN_WORK* wk );
static void PMSSelect_GRAPHIC_UnLoad( PMS_SELECT_MAIN_WORK* wk );
static void PMSSelect_BG_LoadResource( PMS_SELECT_BG_WORK* wk, HEAPID heapID );
static void PMSSelect_BG_UnLoadResource( PMS_SELECT_BG_WORK* wk );
static void PMSSelect_BG_PlateTrans( PMS_SELECT_BG_WORK* wk, u8 view_pos_id, u32 sentence_type, BOOL is_select );
static void PMSSelect_BG_PlateMain( PMS_SELECT_BG_WORK* wk );
static void PMSSelect_BG_PlateAnimeReset( PMS_SELECT_BG_WORK* wk );
static void PMSSelect_BG_PlateFutterTrans( PMS_SELECT_BG_WORK* wk, u32 sentence_type );

#ifdef PMS_SELECT_TOUCHBAR
//-------------------------------------
///	タッチバー
//=====================================
static void PMSSelect_TOUCHBAR_Main( TOUCHBAR_WORK	*touchbar );
//以下カスタムボタン使用サンプル用
static TOUCHBAR_WORK * PMSSelect_TOUCHBAR_Init( PMS_SELECT_MAIN_WORK *wk, GFL_CLUNIT *clunit, HEAPID heapID );
static void PMSSelect_TOUCHBAR_Exit( PMS_SELECT_MAIN_WORK *wk );
#endif //PMS_SELECT_TOUCHBAR

#ifdef PMS_SELECT_TASKMENU
//-------------------------------------
///	リストシステムはい、いいえ
//=====================================
static APP_TASKMENU_WORK * PMSSelect_TASKMENU_Init( APP_TASKMENU_RES *menu_res, GFL_MSGDATA *msg, HEAPID heapID );
static void PMSSelect_TASKMENU_Exit( APP_TASKMENU_WORK *menu );
static void PMSSelect_TASKMENU_Main( APP_TASKMENU_WORK *menu );
#endif //PMS_SELECT_TASKMENU

#ifdef PMS_SELECT_PMSDRAW
//-------------------------------------
///	簡易会話表示
//=====================================
static void PMSSelect_PMSDRAW_Init( PMS_SELECT_MAIN_WORK* wk );
static void PMSSelect_PMSDRAW_Exit( PMS_SELECT_MAIN_WORK* wk );
static void PMSSelect_PMSDRAW_Proc( PMS_SELECT_MAIN_WORK* wk );
#endif // PMS_SELECT_PMSDRAW

static void PLATE_CNT_Setup( PMS_SELECT_MAIN_WORK* wk );
static void PLATE_CNT_UpdateAll( PMS_SELECT_MAIN_WORK* wk );
static void PLATE_CNT_UpdateCore( PMS_SELECT_MAIN_WORK* wk, BOOL is_check_print_end );
static void PLATE_CNT_Main( PMS_SELECT_MAIN_WORK* wk );
static void PLATE_CNT_Trans( PMS_SELECT_MAIN_WORK* wk );
static BOOL PLATE_UNIT_Trans( PRINT_QUE* print_que, GFL_BMPWIN* win, BOOL *transReq );
static void PLATE_UNIT_DrawLastMessage( PMS_SELECT_MAIN_WORK* wk, u8 view_pos_id, u8 data_id, BOOL is_select );
static void PLATE_UNIT_DrawNormal( PMS_SELECT_MAIN_WORK* wk, u8 view_pos_id, u8 data_id, BOOL is_select );

static int TOUCH_GetListPos( void );


//=============================================================================
/**
 *								外部公開
 */
//=============================================================================
const GFL_PROC_DATA ProcData_PMSSelect = 
{
	PMSSelectProc_Init,
	PMSSelectProc_Main,
	PMSSelectProc_Exit,
};

//=============================================================================
/**
 *								PROC
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 初期化処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					PMS_SELECT_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT PMSSelectProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	PMS_SELECT_MAIN_WORK *wk;
	PMS_SELECT_PARAM *param;

	//オーバーレイ読み込み
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common) );
  
  // 速度設定
  GFL_UI_KEY_SetRepeatSpeed( 4, 8 );
	
	//引数取得
	param	= pwk;

	//ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_UI_DEBUG, PMS_SELECT_HEAP_SIZE );
  wk = GFL_PROC_AllocWork( proc, sizeof(PMS_SELECT_MAIN_WORK), HEAPID_UI_DEBUG );
  GFL_STD_MemClear( wk, sizeof(PMS_SELECT_MAIN_WORK) );

  // 初期化
  wk->heapID    = HEAPID_UI_DEBUG;
  wk->save_ctrl = param->save_ctrl;
  wk->pmsw_save = SaveData_GetPMSW( param->save_ctrl );
  wk->out_param = param;
	
	//フォント作成
	wk->font			= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
												GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

	//メッセージ
	wk->msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_pms_input_dat, wk->heapID );

	//PRINT_QUE作成
	wk->print_que		= PRINTSYS_QUE_Create( wk->heapID );

  // グラフィックロード
  PMSSelect_GRAPHIC_Load( wk );
  
  // 初回判定
  {
    PMS_DATA* data;
    int first_scene;

    data = PMSW_GetDataEntry( wk->pmsw_save, 0 );

    if( PMSDAT_IsComplete( data, wk->heapID ) == FALSE )
    {
      first_scene = PMSS_SCENE_ID_CALL_EDIT; // いきなり編集画面へ

      // 真っ暗にしておく
      GX_SetMasterBrightness(16);
      GXS_SetMasterBrightness(16);

      HOSAKA_Printf("初回なので直接編集画面へ\n");
    }
    else
    {
      first_scene = PMSS_SCENE_ID_PLATE_SELECT; 

      //@TODO	フェードシーケンスがないので
      GX_SetMasterBrightness(0);
      GXS_SetMasterBrightness(0);
    }
  
    // SCENE
	  wk->cntScene = UI_SCENE_CNT_Create( wk->heapID, c_scene_func_tbl, PMSS_SCENE_ID_MAX, first_scene, wk );
  }
  

  return GFL_PROC_RES_FINISH;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 終了処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					PMS_SELECT_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT PMSSelectProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	PMS_SELECT_MAIN_WORK* wk = mywk;

  PMSSelect_GRAPHIC_UnLoad( wk );

	//メッセージ破棄
	GFL_MSG_Delete( wk->msg );

	//PRINT_QUE
	PRINTSYS_QUE_Delete( wk->print_que );

	//FONT
	GFL_FONT_Delete( wk->font );

  // SCENE
  UI_SCENE_CNT_Delete( wk->cntScene );
  
  // 元にもどす
  GFL_UI_KEY_SetRepeatSpeed( 8, 15 );

	//PROC用メモリ解放
  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( wk->heapID );
	
  //オーバーレイ破棄
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));

  return GFL_PROC_RES_FINISH;
}
//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 主処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					PMS_SELECT_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT PMSSelectProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	PMS_SELECT_MAIN_WORK* wk = mywk;
  
  if( wk->bProcChange )
  {
	  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
    wk->bProcChange = FALSE;
    HOSAKA_Printf("return! \n");
  }
	
  // SCENE
  if( UI_SCENE_CNT_Main( wk->cntScene ) )
  {
    return GFL_PROC_RES_FINISH;
  }
    
  // PROC
  if( wk->bProcChange )
  {
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common) );
    HOSAKA_Printf("call! \n");
    return GFL_PROC_RES_CONTINUE;
  }

	// PRINT_QUE
	PRINTSYS_QUE_Main( wk->print_que );
  
  // PLATE転送
  PLATE_CNT_Trans( wk );

#ifdef PMS_SELECT_PMSDRAW
  PMSSelect_PMSDRAW_Proc( wk );
#endif //PMS_SELECT_PMSDRAW

	// 2D描画
	PMS_SELECT_GRAPHIC_2D_Draw( wk->graphic );

  return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *								static関数
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  グラフィック ロード
 *	
 *	@param	PMS_SELECT_MAIN_WORK* wk 
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void PMSSelect_GRAPHIC_Load( PMS_SELECT_MAIN_WORK* wk )
{ 
	//描画設定初期化
	wk->graphic	= PMS_SELECT_GRAPHIC_Init( GX_DISP_SELECT_SUB_MAIN, wk->heapID );
  
  // OBJリストバー アイコン読み込み
  {
    UI_EASY_CLWK_RES_PARAM prm;
		GFL_CLUNIT	*clunit	= PMS_SELECT_GRAPHIC_GetClunit( wk->graphic );

    prm.draw_type = CLSYS_DRAW_MAIN;
    prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NONE;
    prm.arc_id    = ARCID_PMSI_GRAPHIC;
    prm.pltt_id   = NARC_pmsi_pms2_obj_main_NCLR;
    prm.ncg_id    = NARC_pmsi_pms2_obj_main_NCGR;
    prm.cell_id   = NARC_pmsi_pms2_obj_main_NCER;
    prm.anm_id    = NARC_pmsi_pms2_obj_main_NANR;
    prm.pltt_line = PLTID_OBJ_COMMON_M;
    prm.pltt_src_ofs  = 0;
    prm.pltt_src_num  = 3;

    UI_EASY_CLWK_LoadResource( &wk->clwkres, &prm, clunit, wk->heapID );

    wk->clwk_bar = UI_EASY_CLWK_CreateCLWK( &wk->clwkres, clunit, LIST_BAR_STD_PX, LIST_BAR_STD_PY, LIST_BAR_ANM_IDX ,wk->heapID );

    // タスクメニューの下
    GFL_CLACT_WK_SetBgPri( wk->clwk_bar, 1 );
  }

	//BGリソース読み込み
	PMSSelect_BG_LoadResource( &wk->wk_bg, wk->heapID );

#ifdef PMS_SELECT_TOUCHBAR
	//タッチバーの初期化
	{	
		GFL_CLUNIT	*clunit	= PMS_SELECT_GRAPHIC_GetClunit( wk->graphic );
		wk->touchbar	= PMSSelect_TOUCHBAR_Init( wk, clunit, wk->heapID );
	}
#endif //PMS_SELECT_TOUCHBAR

#ifdef PMS_SELECT_TASKMENU
	//TASKMENUリソース読み込み＆初期化
	wk->menu_res	= APP_TASKMENU_RES_Create( BG_FRAME_MENU_M, PLTID_BG_TASKMENU_M, wk->font, wk->print_que, wk->heapID );
#endif //PMS_SELECT_TASKMENU

#ifdef PMS_SELECT_PMSDRAW  
  PMSSelect_PMSDRAW_Init( wk );
#endif //PMS_SELECT_PMSDRAW

}

//-----------------------------------------------------------------------------
/**
 *	@brief  グラフィック アンロード
 *
 *	@param	PMS_SELECT_MAIN_WORK* wk 
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void PMSSelect_GRAPHIC_UnLoad( PMS_SELECT_MAIN_WORK* wk )
{
  UI_EASY_CLWK_UnLoadResource( &wk->clwkres );

#ifdef PMS_SELECT_TOUCHBAR
	//タッチバー
	PMSSelect_TOUCHBAR_Exit( wk );
#endif //PMS_SELECT_TOUCHBAR

#ifdef PMS_SELECT_TASKMENU
	//TASKMENUシステム＆リソース破棄
	APP_TASKMENU_RES_Delete( wk->menu_res );	
#endif //PMS_SELECT_TASKMENU

#ifdef PMS_SELECT_PMSDRAW
  PMSSelect_PMSDRAW_Exit( wk );
#endif //PMS_SELECT_PMSDRAW
	
  //BGリソース破棄
	PMSSelect_BG_UnLoadResource( &wk->wk_bg );

	//描画設定破棄
	PMS_SELECT_GRAPHIC_Exit( wk->graphic );
}
//-----------------------------------------------------------------------------
/**
 *	@brief  BG管理モジュール リソース読み込み
 *
 *	@param	PMS_SELECT_BG_WORK* wk BG管理ワーク
 *	@param	heapID  ヒープID 
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void PMSSelect_BG_LoadResource( PMS_SELECT_BG_WORK* wk, HEAPID heapID )
{
	ARCHANDLE	*handle;
	
  handle	= GFL_ARC_OpenDataHandle( ARCID_PMSI_GRAPHIC, heapID );

	// 上下画面ＢＧパレット転送
	GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_pmsi_pms2_bg_main_NCLR, PALTYPE_MAIN_BG, 0x20*PLTID_BG_COMMON_M, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_pmsi_pms2_bg_main_NCLR, PALTYPE_SUB_BG,  0x20*PLTID_BG_BACK_S, 0, heapID );
	
  // 会話フォントパレット転送
//	GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 0x20*PLTID_BG_MSG_M, 0x20, heapID );
	
  //	----- 上画面 -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, NARC_pmsi_pms2_bg_back_NCGR,
						BG_FRAME_BACK_S, 0, 0, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(	handle, NARC_pmsi_pms2_bg_back_NSCR,
						BG_FRAME_BACK_S, 0, 0, 0, heapID );

	//	----- 下画面 -----

  // 背景
	wk->m_back_chr_pos = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan(	handle, NARC_pmsi_pms2_bg_back_NCGR, BG_FRAME_BACK_M, 
            0, 0, heapID );

	GF_ASSERT( wk->m_back_chr_pos != GFL_ARCUTIL_TRANSINFO_FAIL );

	GFL_ARCHDL_UTIL_TransVramScreen(	handle, NARC_pmsi_pms2_bg_back01_NSCR, BG_FRAME_BACK_M,
      GFL_ARCUTIL_TRANSINFO_GetPos( wk->m_back_chr_pos ), 0, 0, heapID );
	
  // プレート
  GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, NARC_pmsi_pms2_bg_main_NCGR,
						BG_FRAME_TEXT_M, 0, 0, 0, heapID );

  wk->plateScrnWork = GFL_ARCHDL_UTIL_LoadScreen( handle, NARC_pmsi_pms2_bg_main1_NSCR, FALSE,
                      &wk->platescrnData, heapID );

	GFL_ARC_CloseDataHandle( handle );

  // メニューのBGにもバーをロードしておく
  

}

//-----------------------------------------------------------------------------
/**
 *	@brief  BG開放
 *
 *	@param	PMS_SELECT_BG_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void PMSSelect_BG_UnLoadResource( PMS_SELECT_BG_WORK* wk )
{
  // 背面BG開放
	GFL_BG_FreeCharacterArea( BG_FRAME_BACK_M,
			GFL_ARCUTIL_TRANSINFO_GetPos(wk->m_back_chr_pos),
			GFL_ARCUTIL_TRANSINFO_GetSize(wk->m_back_chr_pos));

  GFL_HEAP_FreeMemory( wk->plateScrnWork );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BG プレート転送
 *
 *	@param	PMS_SELECT_BG_WORK* wk 
 *	@param	view_pos_id
 *	@param	sentence_type
 *	@param	is_select 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void PMSSelect_BG_PlateTrans( PMS_SELECT_BG_WORK* wk, u8 view_pos_id, u32 sentence_type, BOOL is_select )
{
  int i;
  int sx = wk->platescrnData->screenWidth/8;
  int sy = wk->platescrnData->screenHeight/8;

  u16* buff = (u16*)&wk->platescrnData->rawData;

  // 選択色
  if( is_select )
  {
    sentence_type = PMS_TYPE_MAX;
  }

  // パレット指定
  for( i=0; i<(sx*sy); i++ )
  {
    // スクリーンデータの上位4bitでパレット指定
    buff[i] = ( buff[i] & 0xFFF ) + 0x1000 * ( sentence_type + PLATE_BG_PLTT_OFS_BASE );
  }

  //@TODO 枠だけ転送すれば良い。
  GFL_BG_WriteScreenExpand( BG_FRAME_TEXT_M, 
    0, view_pos_id * PLATE_BG_SY + 1,
    sx, PLATE_BG_SY,
    &wk->platescrnData->rawData,
    0,  0 * PLATE_BG_SY,
    sx, sy );

  GFL_BG_LoadScreenV_Req( BG_FRAME_TEXT_M );
}

//プレートのアニメ。sin使うので0～0xFFFFのループ
#define PLATE_PLTT_ANIME_VALUE (0x400)

#define PLATE_PLTT_ANIME_S_R (0)
#define PLATE_PLTT_ANIME_S_G (30)
#define PLATE_PLTT_ANIME_S_B (29)

#define PLATE_PLTT_ANIME_E_R (24)
#define PLATE_PLTT_ANIME_E_G (31)
#define PLATE_PLTT_ANIME_E_B (31)

//プレートのアニメする色
#define PLATE_PLTT_ANIME_COL (0x5)
// パレット列
#define PLATE_PLTT_ANIME_NO  (0x7)

// BGアニメ
static void _UpdatePalletAnime( u16 *anmCnt , u16 *transBuf , u8 pltNo )
{
  //プレートアニメ
  if( *anmCnt + PLATE_PLTT_ANIME_VALUE >= 0x10000 )
  {
    *anmCnt = *anmCnt + PLATE_PLTT_ANIME_VALUE-0x10000;
  }
  else
  {
    *anmCnt += PLATE_PLTT_ANIME_VALUE;
  }
  {
    //1～0に変換
    const fx16 cos = (FX_CosIdx(*anmCnt)+FX16_ONE)/2;
    const u8 r = PLATE_PLTT_ANIME_S_R + (((PLATE_PLTT_ANIME_E_R-PLATE_PLTT_ANIME_S_R)*cos)>>FX16_SHIFT);
    const u8 g = PLATE_PLTT_ANIME_S_G + (((PLATE_PLTT_ANIME_E_G-PLATE_PLTT_ANIME_S_G)*cos)>>FX16_SHIFT);
    const u8 b = PLATE_PLTT_ANIME_S_B + (((PLATE_PLTT_ANIME_E_B-PLATE_PLTT_ANIME_S_B)*cos)>>FX16_SHIFT);
    BOOL result;
    
    *transBuf = GX_RGB(r, g, b);
    
    result = NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG_PLTT_MAIN ,
                                        pltNo * 32 + PLATE_PLTT_ANIME_COL*2 ,
                                        transBuf , 2 );

    GF_ASSERT( result == TRUE );
  }
}


//-----------------------------------------------------------------------------
/**
 *	@brief  BGプレート 主処理
 *
 *	@param	PMS_SELECT_BG_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void PMSSelect_BG_PlateMain( PMS_SELECT_BG_WORK* wk )
{
  _UpdatePalletAnime( &wk->transAnmCnt, &wk->transCol, 7 );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BGアニメーションカウンタをリセット
 *
 *	@param	PMS_SELECT_BG_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void PMSSelect_BG_PlateAnimeReset( PMS_SELECT_BG_WORK* wk )
{
  wk->transAnmCnt = 0;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  プレートの下枠をBG最上部に転送リクエスト
 *
 *	@param	PMS_SELECT_BG_WORK* wk
 *	@param	sentence_type 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void PMSSelect_BG_PlateFutterTrans( PMS_SELECT_BG_WORK* wk, u32 sentence_type )
{
  int i;
  int sx = wk->platescrnData->screenWidth/8;
  int sy = wk->platescrnData->screenHeight/8;

  u16* buff = (u16*)&wk->platescrnData->rawData;

  //@TODO スクリーンバッファ全体を置き換える必要はない

  // パレット指定
  for( i=0; i<(sx*sy); i++ )
  {
    // スクリーンデータの上位4bitでパレット指定
    buff[i] = ( buff[i] & 0xFFF ) + 0x1000 * ( sentence_type + PLATE_BG_PLTT_OFS_BASE );
  }
  
  GFL_BG_WriteScreenExpand( BG_FRAME_TEXT_M, 
    0, 0,
    sx, 1,
    &wk->platescrnData->rawData,
    0,  PLATE_BG_SY-1,
    sx, sy );

  GFL_BG_LoadScreenV_Req( BG_FRAME_TEXT_M );
}

#ifdef PMS_SELECT_TOUCHBAR
//=============================================================================
/**
 *	TOUCHBAR
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief	TOUCHBAR初期化
 *
 * @param  wk										ワーク
 *	@param	GFL_CLUNIT *clunit	CLUNIT
 *	@param	heapID							ヒープID
 *
 *	@return	TOUCHBAR_WORK
 */
//-----------------------------------------------------------------------------
static TOUCHBAR_WORK * PMSSelect_TOUCHBAR_Init( PMS_SELECT_MAIN_WORK *wk, GFL_CLUNIT *clunit, HEAPID heapID )
{	
	//タッチバーの設定
	
  //アイコンの設定
	//数分作る
	TOUCHBAR_SETUP	touchbar_setup = {0};

	TOUCHBAR_ITEM_ICON touchbar_icon_tbl[]	=
	{	
		{	
			TOUCHBAR_ICON_RETURN,
			{	TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_Y },
		},
	};

	//設定構造体
	//さきほどの窓情報＋リソース情報をいれる
	touchbar_setup.p_item		= touchbar_icon_tbl;				//上の窓情報
	touchbar_setup.item_num	= NELEMS(touchbar_icon_tbl);//いくつ窓があるか
	touchbar_setup.p_unit		= clunit;										//OBJ読み込みのためのCLUNIT
	touchbar_setup.bar_frm	= BG_FRAME_BAR_M;						//BG読み込みのためのBG面
	touchbar_setup.bg_plt		= PLTID_BG_TOUCHBAR_M;			//BGﾊﾟﾚｯﾄ
	touchbar_setup.obj_plt	= PLTID_OBJ_TOUCHBAR_M;			//OBJﾊﾟﾚｯﾄ
	touchbar_setup.mapping	= APP_COMMON_MAPPING_64K;	//マッピングモード

	return TOUCHBAR_Init( &touchbar_setup, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief	TOUCHBAR破棄	カスタムボタン版
 *
 *	@param	TOUCHBAR_WORK	*touchbar タッチバー
 */
//-----------------------------------------------------------------------------
static void PMSSelect_TOUCHBAR_Exit( PMS_SELECT_MAIN_WORK *wk )
{	
	//タッチバー破棄
	TOUCHBAR_Exit( wk->touchbar );
}

//----------------------------------------------------------------------------
/**
 *	@brief	TOUCHBARメイン処理
 *
 *	@param	TOUCHBAR_WORK	*touchbar タッチバー
 */
//-----------------------------------------------------------------------------
static void PMSSelect_TOUCHBAR_Main( TOUCHBAR_WORK	*touchbar )
{	
	TOUCHBAR_Main( touchbar );
}
#endif //PMS_SELECT_TOUCHBAR

#ifdef PMS_SELECT_TASKMENU
//----------------------------------------------------------------------------
/**
 *	@brief	TASKMENUの初期化
 *
 *	@param	menu_res	リソース
 */
//-----------------------------------------------------------------------------
static APP_TASKMENU_WORK * PMSSelect_TASKMENU_Init( APP_TASKMENU_RES *menu_res, GFL_MSGDATA *msg, HEAPID heapID )
{	
	int i;
	APP_TASKMENU_INITWORK	init;
	APP_TASKMENU_ITEMWORK	item[3];
	APP_TASKMENU_WORK			*menu;	

  static u8 strtbl[] = {
    str_decide,
    str_edit,
    str_cancel,
  };

	//窓の設定
	for( i = 0; i < NELEMS(item); i++ )
	{	
		item[i].str	      = GFL_MSG_CreateString( msg, strtbl[i] );	//文字列
		item[i].msgColor	= APP_TASKMENU_ITEM_MSGCOLOR;	//文字色
		item[i].type			= APP_TASKMENU_WIN_TYPE_NORMAL;	//窓の種類
	}
	
  // 「もどる」記号
  item[2].type  = APP_TASKMENU_WIN_TYPE_RETURN,	//もどる記号が入った窓（←┘）←こんなの

	//初期化
	init.heapId		= heapID;
	init.itemNum	= NELEMS(item);
	init.itemWork = item;
	init.posType	= ATPT_RIGHT_DOWN;
	init.charPosX	= 32;
	init.charPosY = 24;
	init.w				= APP_TASKMENU_PLATE_WIDTH;
	init.h				= APP_TASKMENU_PLATE_HEIGHT;

	menu	= APP_TASKMENU_OpenMenu( &init, menu_res );

	//文字列解放
	for( i = 0; i < NELEMS(item); i++ )
	{	
		GFL_STR_DeleteBuffer( item[i].str );
	}

	return menu;

}
//----------------------------------------------------------------------------
/**
 *	@brief	TASKMENUの破棄
 *
 *	@param	APP_TASKMENU_WORK *menu	ワーク
 */
//-----------------------------------------------------------------------------
static void PMSSelect_TASKMENU_Exit( APP_TASKMENU_WORK *menu )
{	
	APP_TASKMENU_CloseMenu( menu );
}
//----------------------------------------------------------------------------
/**
 *	@brief	TASKMENUのメイン処理
 *
 *	@param	APP_TASKMENU_WORK *menu	ワーク
 */
//-----------------------------------------------------------------------------
static void PMSSelect_TASKMENU_Main( APP_TASKMENU_WORK *menu )
{	
	APP_TASKMENU_UpdateMenu( menu );
}
#endif //PMS_SELECT_TASKMENU

#ifdef PMS_SELECT_PMSDRAW

//-----------------------------------------------------------------------------
/**
 *	@brief  簡易会話表示 初期化処理
 *
 *	@param	PMS_SELECT_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void PMSSelect_PMSDRAW_Init( PMS_SELECT_MAIN_WORK* wk )
{
  int i;
	GFL_CLUNIT	*clunit;
  
  clunit = PMS_SELECT_GRAPHIC_GetClunit( wk->graphic );

  wk->pms_draw  = PMS_DRAW_Init( clunit, CLSYS_DRAW_MAIN, wk->print_que, wk->font, 
      PLTID_OBJ_PMS_DRAW, PMS_SELECT_PMSDRAW_NUM ,wk->heapID );

  HOSAKA_Printf("list_head_id=%d select_id=%d \n", wk->list_head_id, wk->select_id );
  
  // リストの数を測定
  wk->list_max = 0;
  for( i=0; i<PMS_DATA_ENTRY_MAX; i++ )
  {
    PMS_DATA* pms;
    
    pms = PMSW_GetDataEntry( wk->pmsw_save, i );
      
    wk->list_max++; // 新規項目を含めるのでこのタイミング

    if( PMSDAT_IsComplete( pms, wk->heapID ) == FALSE )
    {
      HOSAKA_Printf( "list_max=%d \n", wk->list_max );
      break;
    }
  }
  // バーが出現設定
  GFL_CLACT_WK_SetDrawEnable( wk->clwk_bar, ( wk->list_max > LIST_BAR_ENABLE_LISTNUM ) );

  // PMS表示用BMPWIN生成
  for( i=0; i<PMS_SELECT_PMSDRAW_NUM; i++ )
  {
    wk->pms_win[i] = GFL_BMPWIN_Create(
        BG_FRAME_TEXT_M,					// ＢＧフレーム
        2, 2 + 6 * i,			      	// 表示座標(キャラ単位)
        25, 4,    							  // 表示サイズ
        PLTID_BG_MSG_M,					// パレット
        GFL_BMP_CHRAREA_GET_B );	// キャラ取得方向
  }

  // 簡易会話描画 初期化
  PLATE_CNT_Setup( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  簡易会話表示 後処理
 *
 *	@param	PMS_SELECT_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void PMSSelect_PMSDRAW_Exit( PMS_SELECT_MAIN_WORK* wk )
{
  PMS_DRAW_Exit( wk->pms_draw );
  {
    int i;
    for( i=0; i<PMS_SELECT_PMSDRAW_NUM; i++ )
    {
      GFL_BMPWIN_Delete( wk->pms_win[i] );
    }
  }
}

//-----------------------------------------------------------------------------
/**
 *	@brief  簡易会話表示 主処理
 *
 *	@param	PMS_SELECT_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void PMSSelect_PMSDRAW_Proc( PMS_SELECT_MAIN_WORK* wk )
{
  PMS_DRAW_Main( wk->pms_draw );
}

#endif // PMS_SELECT_PMSDRAW

//-----------------------------------------------------------------------------
/**
 *	@brief  プレート管理 全プレートユニットを初期化
 *
 *	@param	PMS_SELECT_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void PLATE_CNT_Setup( PMS_SELECT_MAIN_WORK* wk )
{
  PLATE_CNT_UpdateCore( wk, FALSE );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  プレート管理 全プレートユニットを更新
 *
 *	@param	PMS_SELECT_MAIN_WORK* wk 
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void PLATE_CNT_UpdateAll( PMS_SELECT_MAIN_WORK* wk )
{
  PLATE_CNT_UpdateCore( wk, TRUE );
  PMSSelect_BG_PlateAnimeReset( &wk->wk_bg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  プレート管理 全プレートユニットを更新 コア
 *
 *	@param	PMS_SELECT_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void PLATE_CNT_UpdateCore( PMS_SELECT_MAIN_WORK* wk, BOOL is_check_print_end )
{ 
  int i;
    
  // まだ文字転送が終了していないものがあるときは更新しない
  if( is_check_print_end && PMS_DRAW_IsPrintEnd( wk->pms_draw ) == FALSE )
  {
    return;
  }

  // 一端スクリーンをクリア
  GFL_BG_ClearScreenCodeVReq( BG_FRAME_TEXT_M, 0 );

  // リストが先頭でなければ最上部にフッタを表示
  if( wk->list_head_id > 0 )
  {
    PMS_DATA* pms;
    u8 futter_id;
    u32 sentence_type;
    
    futter_id = wk->list_head_id - 1;
    pms = PMSW_GetDataEntry( wk->pmsw_save, futter_id );
    sentence_type = PMSDAT_GetSentenceType( pms );

    PMSSelect_BG_PlateFutterTrans( &wk->wk_bg, sentence_type );
  }

  // 簡易会話を全て描画
  for( i=0; (wk->list_head_id+i) < wk->list_max && i < PMS_SELECT_PMSDRAW_NUM; i++ )
  {
    u8 data_id = wk->list_head_id + i;
    BOOL is_select = ( data_id == wk->select_id );
    
    if( data_id == wk->list_max-1 )
    {
      // 「あたらしい　メッセージを　ついかする」
      PLATE_UNIT_DrawLastMessage( wk, i, NULL, is_select );
    }
    else
    {
      // 簡易会話描画
      PLATE_UNIT_DrawNormal( wk, i, data_id, is_select );
    }
  }
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キー移動よるバーアイコンの位置変更
 *
 *	@param	PMS_SELECT_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void LIST_BAR_SetPosFromKey( PMS_SELECT_MAIN_WORK* wk )
{
  int pos_id;
  s16 clpos;
  int fx_elem_size;
  
  // バーが出現していない場合は処理なし
  if( GFL_CLACT_WK_GetDrawEnable( wk->clwk_bar ) == FALSE )
  {
    return;
  }

  pos_id = wk->select_id;
  
  // MIN
  if( pos_id == 0 )
  {
    clpos = LIST_BAR_AREA_MIN;
  }
  // MAX
  else if( pos_id == wk->list_max-1 )
  {
    clpos = LIST_BAR_AREA_MAX;
  }
  // OTHER
  else
  {
    // タッチ座標からCLWKの座標を算出
    fx_elem_size     = FX32_CONST(LIST_BAR_AREA_SIZE) / (wk->list_max-1);

    GF_ASSERT( fx_elem_size ); // ZERO DIV

    clpos = LIST_BAR_AREA_MIN + ( (fx_elem_size * pos_id) >> FX32_SHIFT );
   
    GF_ASSERT( clpos < LIST_BAR_AREA_MAX );
  }

  GFL_CLACT_WK_SetWldTypePos( wk->clwk_bar, clpos, CLSYS_MAT_Y );

  OS_Printf("pos_id=%d, list_max=%d, area_size=%d, fx_elem_size=%f, clpos=%d \n",
                 pos_id, wk->list_max, LIST_BAR_AREA_SIZE, FX_FX32_TO_F32(fx_elem_size), clpos );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タッチ移動によるリストの位置変更
 *
 *	@param	PMS_SELECT_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL LIST_BAR_SetPosFromTouch( PMS_SELECT_MAIN_WORK* wk )
{
  u32 tx, ty;

  // バーが出現していない場合は処理なし
  if( GFL_CLACT_WK_GetDrawEnable( wk->clwk_bar ) == FALSE )
  {
    return FALSE;
  }

  // トリガで範囲を取得し、それ以降は範囲を見ない
  if( GFL_UI_TP_GetPointTrg( &tx, &ty ) == FALSE )
  {
    return FALSE;
  }

  if( tx > LIST_BAR_STD_PX && tx < GX_LCD_SIZE_X )
  { 
    if( ty < LIST_BAR_TOUCH_MAX && ty > LIST_BAR_TOUCH_MIN ) 
    {
      wk->b_listbar = TRUE;
      wk->select_id = SELECT_ID_NULL; ///< 選択を無効化
      return TRUE;
    }
  }
  
  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	u32 py
 *	@param	list_max
 *	@param	out_pos_id
 *	@param	out_pos_dot 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void LIST_BAR_GetPosDataByTouch( u32 py, int list_max, u16* out_pos_id, s16* out_pos_dot )
{ 
  fx32 fx_elem_size;
  
  // 範囲制限
  py = MATH_CLAMP( py, LIST_BAR_TOUCH_MIN, LIST_BAR_TOUCH_MAX );
  
  // タッチ座標から選択IDを算出
  fx_elem_size     = FX32_CONST(LIST_BAR_AREA_SIZE) / (list_max-1);

  GF_ASSERT( fx_elem_size ); // ZERO DIV
  
  // 出力
  *out_pos_id = FX_Div( FX32_CONST(py - LIST_BAR_AREA_MIN), fx_elem_size) >> FX32_SHIFT;
  *out_pos_dot = LIST_BAR_AREA_MIN + ( (fx_elem_size * (*out_pos_id) ) >> FX32_SHIFT );
}

//-----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	PMS_SELECT_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL LIST_BAR_CheckTouching( PMS_SELECT_MAIN_WORK* wk )
{
  u32 tx, ty;
  u16 pos_id;
  s16 clpos;
  int select_id_pre;

  GF_ASSERT( wk->b_listbar );

  // 継続タッチチェック＆タッチ座標取得
  if( GFL_UI_TP_GetPointCont( &tx, &ty ) == FALSE )
  {
    wk->b_listbar = FALSE;
    return FALSE;
  }

  LIST_BAR_GetPosDataByTouch( ty, wk->list_max, &pos_id, &clpos );
  select_id_pre = wk->list_head_id;

  // 選択IDに変更があった場合はOBJを移動
  if( pos_id != select_id_pre )
  {
    // 出力
    GFL_CLACT_WK_SetWldTypePos( wk->clwk_bar, clpos, CLSYS_MAT_Y );
    wk->list_head_id = pos_id;
    wk->list_head_id = MATH_CLAMP( wk->list_head_id, 0, (PMS_DATA_ENTRY_MAX-(PMS_SELECT_PMSDRAW_NUM-1)) ); // 実質3個表示なので
    
    OS_Printf("list_head_id=%d pos_id=%d \n", wk->list_head_id, pos_id );

    return TRUE;
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  プレート操作 主処理
 *
 *	@param	PMS_SELECT_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void PLATE_CNT_Main( PMS_SELECT_MAIN_WORK* wk )
{
  // セレクト無効状態でキー入力されたら
  if( GFL_UI_KEY_GetTrg() && wk->select_id == SELECT_ID_NULL )
  {
    //@TODO このままだと、初回移動時に若干ずれてしまう。座標からselect_idを求めれば良いが。
    if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
    {
      wk->select_id = wk->list_head_id + (PMS_SELECT_PMSDRAW_NUM-2); //末尾を選択
    }
    else
    {
      wk->select_id = wk->list_head_id; //先頭を選択
    }

    PLATE_CNT_UpdateAll( wk );
  }

  // キー入力
  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP )
  {
    if( wk->select_id > 0 )
    {
      wk->select_id--;

      // リストの先頭を選択していた場合はリストごと移動
      if( wk->select_id+1 == wk->list_head_id )
      {
        wk->list_head_id--;

        GF_ASSERT( wk->list_head_id >= 0 );
      }

      LIST_BAR_SetPosFromKey( wk );

      //@TODO 重いようなら、
      //1:ページ全体が更新されない時はUpdateAllをスクリーンのカラーを置換する処理にする
      //2:移動の時はコピーを使う。
      PLATE_CNT_UpdateAll( wk );

      GFL_SOUND_PlaySE( SE_MOVE_CURSOR );
      
      HOSAKA_Printf("list_head_id=%d select_id=%d \n", wk->list_head_id, wk->select_id );
    }
  }
  else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN )
  {
    if( wk->select_id < wk->list_max-1 )
    {
      wk->select_id++;

      // リストの末尾を選択していた場合はリストごと移動
      if( ( wk->select_id == wk->list_head_id + PMS_SELECT_PMSDRAW_NUM - 1 ) )
      {
        wk->list_head_id++;

        GF_ASSERT( wk->list_head_id >= 0 );
      }

      LIST_BAR_SetPosFromKey( wk );
        
      PLATE_CNT_UpdateAll( wk );

      GFL_SOUND_PlaySE( SE_MOVE_CURSOR );
     
      HOSAKA_Printf("list_head_id=%d select_id=%d \n", wk->list_head_id, wk->select_id );
    }
  }

  // タッチ入力
  {
    int list_pos = TOUCH_GetListPos();

    // タッチしたプレートを選択
    if( list_pos != -1 )
    {
      wk->select_id =  wk->list_head_id + list_pos;

      HOSAKA_Printf("list_pos = %d, select_id = %d \n", list_pos, wk->select_id );
      
      PLATE_CNT_UpdateAll( wk );
    }
    // バーをタッチでプレート移動
    else
    {
      // リストバーが動作していなければ開始判定
      if( wk->b_listbar == FALSE )
      {
        LIST_BAR_SetPosFromTouch( wk );
      }
      
      // 継続チェック
      if( wk->b_listbar == TRUE )
      {
        if( LIST_BAR_CheckTouching( wk ) )
        {
          // 移動があった場合は更新
          PLATE_CNT_UpdateAll( wk );
        }
      }
    }
  }

}

//-----------------------------------------------------------------------------
/**
 *	@brief  全プレートを転送
 *
 *	@param	PMS_SELECT_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void PLATE_CNT_Trans( PMS_SELECT_MAIN_WORK* wk )
{
  int i;

  for( i=0; i < PMS_SELECT_PMSDRAW_NUM; i++ )
  {
    PLATE_UNIT_Trans( wk->print_que, wk->pms_win[ i ], &wk->pms_win_tranReq[ i ] );
  }
}

//-----------------------------------------------------------------------------
/**
 *	@brief  プレート転送
 *
 *	@param	PRINT_QUE* print_que
 *	@param	win
 *	@param	transReq 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL PLATE_UNIT_Trans( PRINT_QUE* print_que, GFL_BMPWIN* win, BOOL *transReq )
{ 
  GFL_BMP_DATA* bmp = GFL_BMPWIN_GetBmp( win );

  if( *transReq == TRUE && !PRINTSYS_QUE_IsExistTarget( print_que, bmp ) )
  {
    GFL_BMPWIN_MakeTransWindow( win );
//    GFL_BMPWIN_TransVramCharacter( win );
    *transReq = FALSE;
    return TRUE;
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  あたらしい　メッセージを　ついかする
 *
 *	@param	PMS_SELECT_MAIN_WORK* wk
 *	@param	view_pos_id
 *	@param	data_id
 *	@param	is_select 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void PLATE_UNIT_DrawLastMessage( PMS_SELECT_MAIN_WORK* wk, u8 view_pos_id, u8 data_id, BOOL is_select )
{
  PRINTSYS_LSB color;
  STRBUF* buf;

  GF_ASSERT( view_pos_id < PMS_SELECT_PMSDRAW_NUM );
  
  // 簡易会話を表示中ならば消す
  if( PMS_DRAW_IsPrinting( wk->pms_draw, view_pos_id ) )
  {
    PMS_DRAW_Clear( wk->pms_draw, view_pos_id, FALSE );
  }

  buf = GFL_MSG_CreateString( wk->msg, pms_input01_01 );
  
  // プレート描画
  PMSSelect_BG_PlateTrans( &wk->wk_bg, view_pos_id, 0, is_select );
  
#if 0
  // 選択状態によって文字描画エリアのカラーを指定
  if( is_select )
  {
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->pms_win[ view_pos_id ] ), 0x2 );
    color = PRINTSYS_LSB_Make( 0xe, 0xf, 0x2 );
  }
  else
#endif
  {
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->pms_win[ view_pos_id ] ), 0x1 );
    color = PRINTSYS_LSB_Make( 0xe, 0xf, 0x1 );
  }

  PRINTSYS_PrintQueColor( wk->print_que, GFL_BMPWIN_GetBmp(wk->pms_win[ view_pos_id ]), 0, 0, buf, wk->font, color );

  GFL_STR_DeleteBuffer( buf );

  wk->pms_win_tranReq[ view_pos_id ] = TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  プレート描画
 *
 *	@param	PMS_SELECT_MAIN_WORK* wk
 *	@param	view_pos_id
 *	@param	data_id 
 *
 *	@retval 
 */
//-----------------------------------------------------------------------------
static void PLATE_UNIT_DrawNormal( PMS_SELECT_MAIN_WORK* wk, u8 view_pos_id, u8 data_id, BOOL is_select )
{
  PMS_DATA* pms;
  
  GF_ASSERT( view_pos_id < PMS_SELECT_PMSDRAW_NUM );

  // 表示中ならば消す
  if( PMS_DRAW_IsPrinting( wk->pms_draw, view_pos_id ) )
  {
    PMS_DRAW_Clear( wk->pms_draw, view_pos_id, FALSE );
  }
     
  pms = PMSW_GetDataEntry( wk->pmsw_save, data_id );

  // パレットによってプレートの背景色を変更
  {
    u32 sentence_type = PMSDAT_GetSentenceType( pms );
    PMSSelect_BG_PlateTrans( &wk->wk_bg, view_pos_id, sentence_type, is_select );
  }

#if 0
  // 選択状態によって文字描画エリアの色指定
  if( is_select )
  {
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->pms_win[ view_pos_id ] ), 0x2 );
    PMS_DRAW_SetPrintColor( wk->pms_draw, PRINTSYS_LSB_Make( 0xe, 0xf, 0x2 ) );
  }
  else
#endif
  {
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->pms_win[ view_pos_id ] ), 0x1 );
    PMS_DRAW_SetPrintColor( wk->pms_draw, PRINTSYS_LSB_Make( 0xe, 0xf, 0x1 ) );
  }

  //  簡易会話表示
  if( PMSDAT_IsComplete( pms, wk->heapID ) == TRUE )
  {
    PMS_DRAW_Print( wk->pms_draw, wk->pms_win[ view_pos_id ], pms, view_pos_id );
    wk->pms_win_tranReq[ view_pos_id ] = TRUE;
  }
  
}

//-----------------------------------------------------------------------------
/**
 *	@brief  リストの範囲をタッチしているか？
 *
 *	@param	void
 *
 *	@retval TRUE : している
 */
//-----------------------------------------------------------------------------
static int TOUCH_GetListPos( void )
{
  u32 px, py;

  if( GFL_UI_TP_GetPointTrg( &px, &py ) )
  {
    if( px < GX_LCD_SIZE_X - (8*3) && py > PLATE_BG_START_PY * 8 )
    {
        // キャラに変換
        py /= 8;
        // 先頭までずらし
        py -= PLATE_BG_START_PY;
        // リストの場所を取得
        py /= PLATE_BG_SY;

        // タッチ可能なのは3項目
        if( py < PMS_SELECT_PMSDRAW_NUM-1 )
        {
          return py;
        }
    }
  }

  return -1;
}

//=============================================================================
// SCENE
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  プレート選択 初期化
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL ScenePlateSelect_Init( UI_SCENE_CNT_PTR cnt, void* work )
{
  PMS_SELECT_MAIN_WORK* wk = work;

  // 全て表示
  TOUCHBAR_SetVisibleAll( wk->touchbar, TRUE );

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  プレート選択主処理
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL ScenePlateSelect_Main( UI_SCENE_CNT_PTR cnt, void* work )
{ 
  PMS_SELECT_MAIN_WORK* wk = work;
  TOUCHBAR_ICON result;

  PMSSelect_BG_PlateMain( &wk->wk_bg );

	//タッチバーメイン処理
	PMSSelect_TOUCHBAR_Main( wk->touchbar );

  // タッチバー入力状態取得
  result = TOUCHBAR_GetTrg( wk->touchbar );

  // タッチバー入力判定
  switch( result )
  {
  case TOUCHBAR_ICON_RETURN :
    // キャンセルフラグON
    wk->out_param->out_cancel_flag  = TRUE;
    wk->out_param->out_pms_data     = NULL;
    UI_SCENE_CNT_SetNextScene( cnt, UI_SCENE_ID_END );
    return TRUE;

  // @TODO 左右と項目アイコン

  case TOUCHBAR_ICON_CUTSOM1 :
  case TOUCHBAR_SELECT_NONE :
    break;

  default : GF_ASSERT(0);
  }

  // プレート操作
  PLATE_CNT_Main( wk );
    
  // メニューを開く
  if( ( TOUCH_GetListPos() != -1 ) || (GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE ) )
  {
    PMS_DATA* pms;

    GFL_SOUND_PlaySE( SE_DECIDE );
    
    pms = PMSW_GetDataEntry( wk->pmsw_save, wk->select_id );

    if( PMSDAT_IsComplete( pms, wk->heapID ) )
    {
      UI_SCENE_CNT_SetNextScene( cnt, PMSS_SCENE_ID_CMD_SELECT );
    }
    else
    {
      // 選択領域が新規なら直接ジャンプ
      UI_SCENE_CNT_SetNextScene( cnt, PMSS_SCENE_ID_CALL_EDIT );
    }

    return TRUE;
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  プレート選択 後処理
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL ScenePlateSelect_End( UI_SCENE_CNT_PTR cnt, void* work )
{
  PMS_SELECT_MAIN_WORK* wk = work;
  
  // 全て消す
  TOUCHBAR_SetVisibleAll( wk->touchbar, FALSE );

  return TRUE;
}
  
//-----------------------------------------------------------------------------
/**
 *	@brief  プレートに対するコマンド選択 初期化
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneCmdSelect_Init( UI_SCENE_CNT_PTR cnt, void* work )
{
  PMS_SELECT_MAIN_WORK* wk = work;

  wk->menu = PMSSelect_TASKMENU_Init( wk->menu_res, wk->msg, wk->heapID );

  
  SetBrightness( BG_MENU_BRIGHT, BG_MENU_MASK_PLANE, BG_MENU_MASK_DISP );

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  プレートに対するコマンド選択 主処理
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneCmdSelect_Main( UI_SCENE_CNT_PTR cnt, void* work )
{
  PMS_SELECT_MAIN_WORK* wk = work;
	
  //タスクメニューメイン処理
	PMSSelect_TASKMENU_Main( wk->menu );

  if( APP_TASKMENU_IsFinish( wk->menu ) )
  {
    u8 pos = APP_TASKMENU_GetCursorPos( wk->menu );

    switch( pos )
    {
    case TASKMENU_ID_DECIDE :
      // けってい → 終了
      UI_SCENE_CNT_SetNextScene( cnt, UI_SCENE_ID_END );
      // 選択されたPMS_DATAを保存
      wk->out_param->out_pms_data     = PMSW_GetDataEntry( wk->pmsw_save, wk->select_id );
      wk->out_param->out_cancel_flag  = FALSE;
      break;
    case TASKMENU_ID_CALL_EDIT :
      // へんしゅう → 入力画面呼び出し
      UI_SCENE_CNT_SetNextScene( cnt, PMSS_SCENE_ID_CALL_EDIT );
      break;
    case TASKMENU_ID_CANCEL :
      // キャンセル → 選択に戻る
      UI_SCENE_CNT_SetNextScene( cnt, PMSS_SCENE_ID_PLATE_SELECT );
      break;
    default : GF_ASSERT(0);
    }

    return TRUE;
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  プレートに対するコマンド選択 後処理
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneCmdSelect_End( UI_SCENE_CNT_PTR cnt, void* work )
{ 
  PMS_SELECT_MAIN_WORK* wk = work;
  
  SetBrightness( 0, BG_MENU_MASK_PLANE, BG_MENU_MASK_DISP );

	PMSSelect_TASKMENU_Exit( wk->menu );

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  へんしゅう→簡易会話入力
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneCallEdit_Main( UI_SCENE_CNT_PTR cnt, void* work )
{
  PMSI_PARAM* pmsi;
  PMS_SELECT_MAIN_WORK* wk = work;
  u8 seq = UI_SCENE_CNT_GetSubSeq( cnt );

  switch( seq )
  {
  case 0:
    GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0);
    UI_SCENE_CNT_IncSubSeq( cnt );
    break;
  case 1:
    if( GFL_FADE_CheckFade() == FALSE )
    {
      PMSSelect_GRAPHIC_UnLoad( wk );

#if PM_DEBUG
      if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
      {
        PMS_DATA data;
        PMSDAT_Init( &data, 0 );
        pmsi = PMSI_PARAM_Create(PMSI_MODE_SENTENCE, PMSI_GUIDANCE_DEFAULT, &data, TRUE, wk->save_ctrl, wk->heapID );
      }
      else
#endif
      {
        PMS_DATA* data;

        pmsi = PMSI_PARAM_Create(PMSI_MODE_SENTENCE, PMSI_GUIDANCE_DEFAULT, NULL, TRUE, wk->save_ctrl, wk->heapID );
        data = PMSW_GetDataEntry( wk->pmsw_save, wk->select_id );

        PMSI_PARAM_SetInitializeDataSentence( pmsi, data );
      }

      if( pmsi )

      // PROC切替 入力画面呼び出し
      GFL_PROC_SysCallProc( NO_OVERLAY_ID, &ProcData_PMSInput, pmsi );
      wk->subproc_work = pmsi;

      // PROC切替フラグON
      wk->bProcChange = TRUE;
      return TRUE;
    }
    break;

  default : GF_ASSERT(0);
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  編集画面終了
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneCallEdit_End( UI_SCENE_CNT_PTR cnt, void* work )
{ 
  PMS_SELECT_MAIN_WORK* wk = work;
  u8 seq = UI_SCENE_CNT_GetSubSeq( cnt );

  switch( seq )
  {
  case 0:

    if( PMSI_PARAM_CheckModified( wk->subproc_work ) )
    {
      PMS_DATA pms;

      // 編集したメッセージをセーブ
      PMSI_PARAM_GetInputDataSentence( wk->subproc_work, &pms );

      PMSW_SetDataEntry( wk->pmsw_save, wk->select_id, &pms );
    }

    // パラメータ削除
    PMSI_PARAM_Delete( wk->subproc_work );

    // グラフィックリロード
    PMSSelect_GRAPHIC_Load( wk );

    GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0);
    UI_SCENE_CNT_IncSubSeq( cnt );
    break;

  case 1:
    // フェード待ち
    if( GFL_FADE_CheckFade() == FALSE )
    {
      // 次のシーンをセット
      UI_SCENE_CNT_SetNextScene( cnt, PMSS_SCENE_ID_PLATE_SELECT );
      return TRUE;
    }
    break;

  default : GF_ASSERT(0);
  }

  return FALSE;
}


