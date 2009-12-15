//============================================================================
/**
 *  @file   zukan_toroku.c
 *  @brief  図鑑登録
 *  @author Koji Kawada
 *  @data   2009.12.03
 *  @note   ui_template.cを元に作成しました。
 *
 *  モジュール名：ZUKAN_TOROKU
 */
//============================================================================
// 必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "print/gf_font.h"
#include "font/font.naix"

#include "print/printsys.h"

#include "../info/zukan_info.h"
#include "zukan_nickname.h"

#include "zukan_toroku_graphic.h"
#include "app/zukan_toroku.h"

#include "arc_def.h"
#include "font/font.naix"

FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
*  定数定義
*/
//=============================================================================
#define ZUKAN_TOROKU_HEAP_SIZE (0x30000)  ///< ヒープサイズ

typedef enum
{
  ZUKAN_TOROKU_STATE_TOROKU,
  ZUKAN_TOROKU_STATE_NICKNAME,
}
ZUKAN_TOROKU_STATE;

typedef enum
{
  ZUKAN_TOROKU_TOROKU_STEP_FADE_IN,
  ZUKAN_TOROKU_TOROKU_STEP_WAIT_KEY,
}
ZUKAN_TOROKU_TOROKU_STEP;

//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// PROCワーク
//=====================================
typedef struct
{
  HEAPID heap_id;
  ZUKAN_TOROKU_GRAPHIC_WORK* graphic;
  GFL_FONT* font;
  PRINT_QUE* print_que;

  ZUKAN_TOROKU_STATE state;
  s32 step;

  ZUKAN_INFO_WORK* zukan_info_work;
  ZUKAN_NICKNAME_WORK* zukan_nickname_work;
}
ZUKAN_TOROKU_WORK;

//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================



//=============================================================================
/**
*  PROC
*/
//=============================================================================
static GFL_PROC_RESULT Zukan_Toroku_ProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT Zukan_Toroku_ProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT Zukan_Toroku_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );

const GFL_PROC_DATA ZUKAN_TOROKU_ProcData =
{
  Zukan_Toroku_ProcInit,
  Zukan_Toroku_ProcMain,
  Zukan_Toroku_ProcExit,
};

//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
ZUKAN_TOROKU_PARAM* ZUKAN_TOROKU_AllocParam( HEAPID heap_id,
                                             ZUKAN_TOROKU_LAUNCH launch )
{
  ZUKAN_TOROKU_PARAM* param = GFL_HEAP_AllocMemory( heap_id, sizeof(ZUKAN_TOROKU_PARAM) );
  GFL_STD_MemClear( param, sizeof(ZUKAN_TOROKU_PARAM) );
  param->launch = launch;
  return param;
}

void ZUKAN_TOROKU_FreeParam( ZUKAN_TOROKU_PARAM* param )
{
  GFL_HEAP_FreeMemory( param );
}

//-------------------------------------
/// PROCに渡す引数の設定
//=====================================
void ZUKAN_TOROKU_SetParam( ZUKAN_TOROKU_PARAM* param,
                            ZUKAN_TOROKU_LAUNCH launch )
{
  param->launch = launch;
}

ZUKAN_TOROKU_RESULT ZUKAN_TOROKU_GetResult( ZUKAN_TOROKU_PARAM* param )
{
  return param->result; 
}

//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
static GFL_PROC_RESULT Zukan_Toroku_ProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  ZUKAN_TOROKU_PARAM* param = (ZUKAN_TOROKU_PARAM*)pwk;
  ZUKAN_TOROKU_WORK* work;

  // オーバーレイ読み込み
  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common) );

  {
    GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_ZUKAN_TOROKU, ZUKAN_TOROKU_HEAP_SIZE );
    work = GFL_PROC_AllocWork( proc, sizeof(ZUKAN_TOROKU_WORK), HEAPID_ZUKAN_TOROKU );
    GFL_STD_MemClear( work, sizeof(ZUKAN_TOROKU_WORK) );
  }

  work->heap_id = HEAPID_ZUKAN_TOROKU;

  work->graphic = ZUKAN_TOROKU_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, work->heap_id );

  work->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, work->heap_id );

  work->print_que = PRINTSYS_QUE_Create( work->heap_id );

  {
    work->state = ZUKAN_TOROKU_STATE_TOROKU;
    work->step = ZUKAN_TOROKU_TOROKU_STEP_FADE_IN;
  }

  work->zukan_info_work = ZUKAN_INFO_Init( work->heap_id, 0, 1, 2,
                                           ZUKAN_TOROKU_GRAPHIC_GetClunit(work->graphic),
                                           3, work->print_que );

  work->zukan_nickname_work = ZUKAN_NICKNAME_Init( work->heap_id, ZUKAN_TOROKU_GRAPHIC_GetClunit(work->graphic), work->font, work->print_que );
  
  //GX_SetMasterBrightness(0);
  //GXS_SetMasterBrightness(0);
  //GFL_FADE_Init( work->heap_id );  // prog/src/system/gfl_use.cのGFLUser_Mainで常に回しているので既に存在していると思われる
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 4 );  // 真っ白の値or真っ黒の値, 0<画面が見えている値>, 輝度が1変化するのに掛かるフレーム );

  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT Zukan_Toroku_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  ZUKAN_TOROKU_PARAM* param = (ZUKAN_TOROKU_PARAM*)pwk;
  ZUKAN_TOROKU_WORK* work = (ZUKAN_TOROKU_WORK*)mywk;

  //GFL_FADE_Exit();

  ZUKAN_NICKNAME_Exit( work->zukan_nickname_work );
  ZUKAN_INFO_Exit( work->zukan_info_work );

  PRINTSYS_QUE_Delete( work->print_que );

  GFL_FONT_Delete( work->font );

  ZUKAN_TOROKU_GRAPHIC_Exit( work->graphic );

  {
    HEAPID heap_id = work->heap_id;
    GFL_PROC_FreeWork( proc );
    GFL_HEAP_DeleteHeap( heap_id );
  }
  
  // オーバーレイ破棄
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common) );

  return GFL_PROC_RES_FINISH;
} 

static GFL_PROC_RESULT Zukan_Toroku_ProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  ZUKAN_TOROKU_PARAM* param = (ZUKAN_TOROKU_PARAM*)pwk;
  ZUKAN_TOROKU_WORK* work = (ZUKAN_TOROKU_WORK*)mywk;

  ZUKAN_NICKNAME_RESULT zukan_nickname_result;
  GFL_PROC_RESULT proc_res = GFL_PROC_RES_CONTINUE;




  ZUKAN_INFO_Main( work->zukan_info_work );
  zukan_nickname_result = ZUKAN_NICKNAME_Main( work->zukan_nickname_work );
  switch( zukan_nickname_result )
  {
  case ZUKAN_NICKNAME_RESULT_NO:
    param->result = ZUKAN_TOROKU_RESULT_END;
    proc_res = GFL_PROC_RES_FINISH;
    break;
  case ZUKAN_NICKNAME_RESULT_YES:
    param->result = ZUKAN_TOROKU_RESULT_NICKNAME;
    proc_res = GFL_PROC_RES_FINISH;
    break;
  }




  


  //GFL_FADE_Main();

  PRINTSYS_QUE_Main( work->print_que );

  // 2D描画
  ZUKAN_TOROKU_GRAPHIC_2D_Draw( work->graphic );
  // 3D描画
  ZUKAN_TOROKU_GRAPHIC_3D_StartDraw( work->graphic );
  ZUKAN_TOROKU_GRAPHIC_3D_EndDraw( work->graphic );

  return proc_res;
}















#if 0

//=============================================================================
// 下記defineをコメントアウトすると、機能を取り除けます
//=============================================================================
#define ZUKAN_TOROKU_INFOWIN  // INFOWIN

FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
 *  定数定義
 */
//=============================================================================
enum
{ 
  ZUKAN_TOROKU_HEAP_SIZE = 0x30000,  ///< ヒープサイズ
};

//-------------------------------------
/// フレーム
//=====================================
enum
{
  BG_FRAME_BAR_M  = GFL_BG_FRAME1_M,
  BG_FRAME_POKE_M = GFL_BG_FRAME2_M,
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
  PLTID_BG_BACK_M          =  0,
  PLTID_BG_POKE_M          =  1,
  PLTID_BG_TASKMENU_M      = 11,
  PLTID_BG_TOUCHBAR_M      = 13,
  PLTID_BG_INFOWIN_M       = 15,
  //サブBG
  PLTID_BG_BACK_S          =  0,

  //メインOBJ
  PLTID_OBJ_TOUCHBAR_M     =  0,  // 3本使用
  PLTID_OBJ_TYPEICON_M     =  3,  // 3本使用
  PLTID_OBJ_OAM_MAPMODEL_M =  6,  // 1本使用
  PLTID_OBJ_POKEICON_M     =  7,  // 3本使用
  PLTID_OBJ_POKEITEM_M     = 10,  // 1本使用
  PLTID_OBJ_ITEMICON_M     = 11,
  PLTID_OBJ_POKE_M         = 12,
  PLTID_OBJ_BALLICON_M     = 13,  // 1本使用
  PLTID_OBJ_TOWNMAP_M      = 14,
  //サブOBJ
  PLTID_OBJ_PMS_DRAW       =  0,  // 5本使用
};

//=============================================================================
/**
 *  構造体定義
 */
//=============================================================================
//--------------------------------------------------------------
/// BG管理ワーク
//==============================================================
typedef struct 
{
  int dummy;
}
ZUKAN_TOROKU_BG_WORK;

//--------------------------------------------------------------
/// メインワーク
//==============================================================
typedef struct 
{
  HEAPID                   heapID;

  ZUKAN_TOROKU_BG_WORK     wk_bg;

  // 描画設定
  UI_TEMPLATE_GRAPHIC_WORK *graphic;

  // フォント
  GFL_FONT                 *font;

  // プリントキュー
  PRINT_QUE                *print_que;
  GFL_MSGDATA              *msg;
}
ZUKAN_TOROKU_MAIN_WORK;

//=============================================================================
/**
 *  データ定義
 */
//=============================================================================

//=============================================================================
/**
 *  プロトタイプ宣言
 */
//=============================================================================
//-------------------------------------
/// PROC
//=====================================
static GFL_PROC_RESULT Zukan_Toroku_ProcInit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT Zukan_Toroku_ProcMain( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT Zukan_Toroku_ProcExit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );

//-------------------------------------
/// 汎用処理ユーティリティ
//=====================================
static void Zukan_Toroku_BG_LoadResource( ZUKAN_TOROKU_BG_WORK *wk, HEAPID heapID );

//=============================================================================
/**
 *  外部公開
 */
//=============================================================================
const GFL_PROC_DATA ZUKAN_TOROKU_ProcData =
{
  Zukan_Toroku_ProcInit,
  Zukan_Toroku_ProcMain,
  Zukan_Toroku_ProcExit,
};

//=============================================================================
/**
 *  PROC
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *  @brief     PROC 初期化処理
 *
 *  @param[in] proc プロセスシステム
 *  @param[in] seq  シーケンス
 *  @param[in] pwk  ZUKAN_TOROKU_PARAM
 *  @param[in] mywk PROCワーク
 *
 *  @retval    終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Zukan_Toroku_ProcInit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
  ZUKAN_TOROKU_MAIN_WORK *wk;
  ZUKAN_TOROKU_PARAM     *param;

  // オーバーレイ読み込み
  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common) );

  // 引数取得
  param = pwk;

  // ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_ZUKAN_TOROKU, ZUKAN_TOROKU_HEAP_SIZE );
  wk = GFL_PROC_AllocWork( proc, sizeof(ZUKAN_TOROKU_MAIN_WORK), HEAPID_ZUKAN_TOROKU );
  GFL_STD_MemClear( wk, sizeof(ZUKAN_TOROKU_MAIN_WORK) );

  // 初期化
  wk->heapID = HEAPID_ZUKAN_TOROKU;

  // 描画設定初期化
  wk->graphic = UI_TEMPLATE_GRAPHIC_Init( GX_DISP_SELECT_SUB_MAIN, wk->heapID );

  // フォント作成
  wk->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
  GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

  // メッセージ
  wk->msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
  NARC_message_mictest_dat, wk->heapID );

  // PRINT_QUE作成
  wk->print_que = PRINTSYS_QUE_Create( wk->heapID );

  // BGリソース読み込み
  Ui_Test_BG_LoadResource( &wk->wk_bg, wk->heapID );

  // @todo フェードシーケンスがないので
  GX_SetMasterBrightness(0);
  GXS_SetMasterBrightness(0);

  return GFL_PROC_RES_FINISH;
}

//-----------------------------------------------------------------------------
/**
 *  @brief     PROC 終了処理
 *
 *  @param[in] proc プロセスシステム
 *  @param[in] seq  シーケンス
 *  @param[in] pwk  ZUKAN_TOROKU_PARAM
 *  @param[in] mywk PROCワーク
 *
 *  @retval    終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Zukan_Toroku_ProcExit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
  ZUKAN_TOROKU_MAIN_WORK* wk = mywk;

  // メッセージ破棄
  GFL_MSG_Delete( wk->msg );

  // PRINT_QUE
  PRINTSYS_QUE_Delete( wk->print_que );

  // FONT
  GFL_FONT_Delete( wk->font );

  // 描画設定破棄
  UI_TEMPLATE_GRAPHIC_Exit( wk->graphic );

  // PROC用メモリ解放
  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( wk->heapID );

  // オーバーレイ破棄
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));

  return GFL_PROC_RES_FINISH;
}

//-----------------------------------------------------------------------------
/**
 *  @brief     PROC 主処理
 *
 *  @param[in] proc プロセスシステム
 *  @param[in] seq  シーケンス
 *  @param[in] pwk  ZUKAN_TOROKU_PARAM
 *  @param[in] mywk PROCワーク
 *
 *  @retval    終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Zukan_Toroku_ProcMain( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
  ZUKAN_TOROKU_MAIN_WORK* wk = mywk;

  // デバッグボタンでアプリ終了
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DEBUG )
  {
    return GFL_PROC_RES_FINISH;
  }

  // PRINT_QUE
  PRINTSYS_QUE_Main( wk->print_que );

  // 2D描画
  UI_TEMPLATE_GRAPHIC_2D_Draw( wk->graphic );

  // 3D描画
  UI_TEMPLATE_GRAPHIC_3D_StartDraw( wk->graphic );

  UI_TEMPLATE_GRAPHIC_3D_EndDraw( wk->graphic );

  return GFL_PROC_RES_CONTINUE;
}

//=============================================================================
/**
 *  static関数
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *  @brief     BG管理モジュール リソース読み込み
 *
 *  @param[in] wk BG管理ワーク
 *  @param[in] heapID  ヒープID 
 *
 *  @retval    none
 */
//-----------------------------------------------------------------------------
static void Ui_Test_BG_LoadResource( ZUKAN_TOROKU_BG_WORK* wk, HEAPID heapID )
{
  // @TODO とりあえずマイクテストのリソース
  ARCHANDLE *handle;

  handle = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_TOROKU, heapID );

  // 上下画面ＢＧパレット転送
  GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_ui_test_back_bg_down_NCLR, PALTYPE_MAIN_BG, PLTID_BG_BACK_M, 0x20, heapID );
  GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_ui_test_back_bg_up_NCLR, PALTYPE_SUB_BG, PLTID_BG_BACK_S, 0x20, heapID );

  // ----- 下画面 -----
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_ui_test_back_bg_down_NCGR,
                                        BG_FRAME_BACK_S, 0, 0, 0, heapID );
  GFL_ARCHDL_UTIL_TransVramScreen( handle, NARC_ui_test_back_bg_down_NSCR,
                                   BG_FRAME_BACK_S, 0, 0, 0, heapID );

  // ----- 上画面 -----
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_ui_test_back_bg_up_NCGR,
                                        BG_FRAME_BACK_M, 0, 0, 0, heapID );
  GFL_ARCHDL_UTIL_TransVramScreen( handle, NARC_ui_test_back_bg_up_NSCR,
                                   BG_FRAME_BACK_M, 0, 0, 0, heapID );

  GFL_ARC_CloseDataHandle( handle );
}

#if 0 // tabをまだ修正していない
#ifdef ZUKAN_TOROKU_INFOWIN
//=============================================================================
/**
 *		INFOWIN
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	INFOWIN初期化
 *
 *	@param	GAMESYS_WORK *gamesys	ゲームシステム
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void UITemplate_INFOWIN_Init( GAMESYS_WORK *gamesys, HEAPID heapID )
{	
	GAME_COMM_SYS_PTR comm;
	comm	= GAMESYSTEM_GetGameCommSysPtr(gamesys);
	INFOWIN_Init( BG_FRAME_BAR_M, PLTID_BG_INFOWIN_M, comm, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief	INFOWIN破棄
 *
 *	@param	void 
 */
//-----------------------------------------------------------------------------
static void UITemplate_INFOWIN_Exit( void )
{	
	INFOWIN_Exit();
}
//----------------------------------------------------------------------------
/**
 *	@brief	INFWINメイン処理
 *
 *	@param	void 
 */
//-----------------------------------------------------------------------------
static void UITemplate_INFOWIN_Main( void )
{	
	INFOWIN_Update();
}
#endif //ZUKAN_TOROKU_INFOWIN
#endif

#endif
