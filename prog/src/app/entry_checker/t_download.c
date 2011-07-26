//=============================================================================
/**
 *
 *	@file		t_download.c
 *	@brief  トーナメントダウンロード画面
 *	@author
 *	@date
 *
 *  共通部分は[t_download_sub.*]に以降
 *
 */
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/bmp_oam.h"
#include "system/bmp_winframe.h"

//FONT
#include "print/gf_font.h"
#include "font/font.naix"

//PRINT_QUE
#include "print/printsys.h"

#include "print/wordset.h"

//描画設定
#include "t_download_graphic.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"
#include "ui/frame_list.h"

// print
#include "ui/print_tool.h"
#include "gamesystem/msgspeed.h"

//アプリ共通素材
#include "app/app_menu_common.h"
#include "app/app_taskmenu.h"

// サウンド
#include "sound/pm_sndsys.h"

//アーカイブ
#include "arc_def.h"
#include "t_download_gra.naix"

#include "system/wipe.h"
//
#include "system/scroll_bar.h"

#include "app/app_keycursor.h"
#include "app/app_printsys_common.h"

#include "t_download_local.h"
#include "app/t_download.h"
#include "t_download_sub.h"

#include "message.naix"

//=============================================================================
//=============================================================================

FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
enum
{
	UI_TEMPLATE_HEAP_SIZE = 0x30000,  ///< ヒープサイズ
};

// 段階
enum{
	STEP_INIT	= 0,				// 初期化
	STEP_LV1 	= 100,
	STEP_LV2 	= 200,
	STEP_LV3 	= 300,
	STEP_LV4 	= 400,
	STEP_LV5 	= 500,
	STEP_LV6 	= 600,
	STEP_LV7 	= 700,
	//
	STEP_END	= 10000,
};

#if 0
// モード(途中で切り替わることもある)
enum{
	MODE_SAVE = 0,						// セーブ画面
	MODE_SELECT,						// 選択画面
	MODE_JOIN,							// 参加画面
	MODE_INFO,							// 詳細画面
	//
	MODE_MAX,
};
#endif

// canvas
enum{
	// 上側
	CVS_TITLE = 0,						// タイトル
	CVS_DATE,							// 配信日
	CVS_RULE_TITLE,						// ルールタイトル
	CVS_RULE_EXPLAIN,					// ルール説明
	CVS_INFO_TITLE,						// 内容タイトル
	CVS_INFO_EXPLAIN,					// 内容説明
	//
	CVS_UP_MES_MAX,						// 上側メッセージ最大
	//
	// 下側
	CVS_MAIN_TITLE = CVS_UP_MES_MAX,	// タイトル
	CVS_MAIN_MESWIN,					// メッセージwin
	CVS_MAIN_CENTER_WIN,				// 中央win
	//
	CVS_MAIN_MES_MAX,					// 下側メッセージ最大
};

// いろいろ
enum{
	TD_ACTIVE_RATE		= 0,			// アクティブの割合(戻し)
	TD_PASSIVE_RATE 	= -8,			// パッシブの割合
};

// スクロール
//enum{
//	SCRL_TOP_Y			= 32,			// レール上部のY座標
//	SCRL_DOWN_Y			= 160,			// レール下部のY座標
//	SCRL_BAR_SIZE		= 16,			// バーのサイズ
//};

//-------------------
// フラグ
//-------------------
// T_DOWNLOAD_MAIN_WORK.Flag
enum{
	FLG_MW_ANM_LEFT		= (1<<0),		// 左矢印アニメ
	FLG_MW_ANM_RIGHT	= (1<<1),		// 右矢印アニメ
	FLG_MW_ANM_CROSS	= (1<<2),		// ×アニメ
	FLG_MW_ANM_RETURN	= (1<<3),		// 戻るアニメ
	//
	FLG_MW_UPDATE		= (1<<5),		// 更新
	FLG_MW_NOITEM		= (1<<6),		// パネルなし(全削除状態)
	//
	FLG_MW_CROSS_HIDE	= (1<<10),		// ×
	FLG_MW_RETURN_HIDE	= (1<<11),		// RETURN
	//
	FLG_MW_RET_END		= (1<<12),		// RETURNで終了
	FLG_MW_CROSS_END	= (1<<13),		// ×で終了
	FLG_MW_BAR_UPDATE	= (1<<14),		// バー更新
	//
	FLG_MW_END			= (1<<20),		// 終了
	//
	// アニメまとめ
	FLG_MW_ANM_ALL		= (FLG_MW_ANM_LEFT|FLG_MW_ANM_RIGHT|FLG_MW_ANM_CROSS|FLG_MW_ANM_RETURN),
};

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================
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
static GFL_PROC_RESULT TDownloadProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT TDownloadProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT TDownloadProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );

//-------------------------------------
///	以下に追加
//=====================================
static void td_init( T_DOWNLOAD_MAIN_WORK* wk );
static void td_main( T_DOWNLOAD_MAIN_WORK* wk );
static void td_exit( T_DOWNLOAD_MAIN_WORK* wk );
static void td_down( T_DOWNLOAD_MAIN_WORK* wk );
//
static void td_sub_init( T_DOWNLOAD_MAIN_WORK* wk );
//
static void td_main_print( T_DOWNLOAD_MAIN_WORK *wk );
//
// 情報表示
static void td_disp_downinfo( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_anim( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );
static void td_init_param( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_my_tp( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 num );

//
// バッファ
//static s32  td_calc_savebuffer( T_DOWNLOAD_MAIN_WORK* wk );
//static BOOL td_chk_savebuffer( T_DOWNLOAD_MAIN_WORK* wk, const s32 num );
//static s32  td_del_savebuffer( T_DOWNLOAD_MAIN_WORK* wk, const s32 wnum );
//static void td_chk_delete( T_DOWNLOAD_MAIN_WORK* wk );
// item
//static void td_set_item( T_DOWNLOAD_MAIN_WORK* wk, const s32 wnum, const s32 num, const s32 mode );
//static s32  td_calc_itembuffer( T_DOWNLOAD_MAIN_WORK* wk );
//static BOOL td_chk_add_item( T_DOWNLOAD_MAIN_WORK* wk );
//
// 輝度変更
static void td_chg_brightness( T_DOWNLOAD_MAIN_WORK* wk, const s32 rate );
//
// タッチペン
static void td_get_tp_area( T_DOWNLOAD_MAIN_WORK* wk, const s32 num, GFL_UI_TP_HITTBL *tpbuffer );
//
//------------------
//------------------
//--------
// join
//--------
static void td_init_join( T_DOWNLOAD_MAIN_WORK* wk );
static void td_chk_input_join( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_tp_input_join( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_key_input_join( T_DOWNLOAD_MAIN_WORK* wk );
static void td_key_join( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_wait_mes_join( T_DOWNLOAD_MAIN_WORK* wk );
static void td_exit_join( T_DOWNLOAD_MAIN_WORK* wk );

//--------
// info
//--------
static void td_init_info( T_DOWNLOAD_MAIN_WORK* wk );
static void td_chk_input_info( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_tp_input_info( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_key_input_info( T_DOWNLOAD_MAIN_WORK* wk );
static void td_key_info( T_DOWNLOAD_MAIN_WORK* wk );
static void td_exit_info( T_DOWNLOAD_MAIN_WORK* wk );

//--------
// select
//--------
static void td_init_select( T_DOWNLOAD_MAIN_WORK* wk );
static void td_chk_input_select( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_tp_input_select( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_key_input_select( T_DOWNLOAD_MAIN_WORK* wk );
static void td_key_select( T_DOWNLOAD_MAIN_WORK* wk );
static void td_exit_select( T_DOWNLOAD_MAIN_WORK* wk );

//--------
// save
//--------
static void td_init_save( T_DOWNLOAD_MAIN_WORK* wk );
static void td_chk_input_save( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_tp_input_save( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_key_input_save( T_DOWNLOAD_MAIN_WORK* wk );
static void td_key_save( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_wait_mes_save( T_DOWNLOAD_MAIN_WORK* wk );
static void td_exit_save( T_DOWNLOAD_MAIN_WORK* wk );

//------------------
// 関数のテーブル
//------------------
// 初期化
static void (*(Td_InitFuncTbl[]))(T_DOWNLOAD_MAIN_WORK*) = {
	td_init_save,			// save
	td_init_select,			// select
	td_init_join,			// join
	td_init_info,			// info
};

// 入力
static void (*(Td_KeyFuncTbl[]))(T_DOWNLOAD_MAIN_WORK*) = {
	td_key_save,			// save
	td_key_select,			// select
	td_key_join,			// join
	td_key_info,			// info
};

//=============================================================================
/**
 *								外部公開
 */
//=============================================================================
const GFL_PROC_DATA TDownloadProcData = 
{
	TDownloadProc_Init,
	TDownloadProc_Main,
	TDownloadProc_Exit,
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
 *	@param	*pwk					T_DOWNLOAD_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT TDownloadProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	T_DOWNLOAD_MAIN_WORK 	*wk;
	
	//オーバーレイ読み込み
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
	
	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_ENTRY_CHECKER, UI_TEMPLATE_HEAP_SIZE );
	wk = GFL_PROC_AllocWork( proc, sizeof(T_DOWNLOAD_MAIN_WORK), HEAPID_ENTRY_CHECKER );
	GFL_STD_MemClear( wk, sizeof(T_DOWNLOAD_MAIN_WORK) );
	
	// 初期化
	wk->heapID = HEAPID_ENTRY_CHECKER;
	
	// 参照用
	wk->Param = pwk;
	
	//描画設定初期化(上:サブ、下:メイン)	
	wk->graphic	= T_DOWNLOAD_GRAPHIC_Init( GX_DISP_SELECT_SUB_MAIN, wk->heapID );
	
	//フォント作成
	wk->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );
	
	// WORDSET生成
	wk->Wordset = WORDSET_Create( wk->heapID );
	
	// メッセージ
	wk->msg[0] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wbt_download_dat, wk->heapID );
	wk->msg[1] = 0;
	
	//PRINT_QUE作成
	wk->print_que = PRINTSYS_QUE_Create( wk->heapID );
	//
	// 会話ウインドウ用
	wk->MsgTcblSys = GFL_TCBL_Init( wk->heapID, wk->heapID, 32 , 32 );
	wk->PrintStream = 0;
	wk->MsgBuffer 	= 0;
	
	//----------------
	// 処理を記述
	//----------------
	td_init( wk );
	
	// ワイプ
	WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
					WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapID );
	
	return GFL_PROC_RES_FINISH;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 終了処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					T_DOWNLOAD_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT TDownloadProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	s32		i;
	T_DOWNLOAD_MAIN_WORK* wk = mywk;
	
	//----------------
	// 処理を記述
	//----------------
	td_exit( wk );
	
	// yes/no
	APP_TASKMENU_RES_Delete( wk->pAppTaskRes );
	
	//メッセージ破棄
	for(i=0;i<T_DOWNLOAD_MSG_MAX;i++){
		if( wk->msg[i] ){
			GFL_MSG_Delete( wk->msg[i] );
			wk->msg[i] = 0;
		}
	}
	
	//PRINT_QUE
	PRINTSYS_QUE_Delete( wk->print_que );
	//
	// WORDSET破棄
	WORDSET_Delete( wk->Wordset );
	
	//FONT
	GFL_FONT_Delete( wk->font );
	// 会話ウインドウ用
	GFL_TCBL_Exit( wk->MsgTcblSys );
	
	//描画設定破棄
	T_DOWNLOAD_GRAPHIC_Exit( wk->graphic );
	
	//PROC用メモリ解放
	{
		HEAPID heapID = wk->heapID;
		GFL_PROC_FreeWork( proc );
		GFL_HEAP_DeleteHeap( heapID );
	}
	//オーバーレイ破棄
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));
	//
	return GFL_PROC_RES_FINISH;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 主処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					T_DOWNLOAD_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT TDownloadProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	T_DOWNLOAD_MAIN_WORK* wk = mywk;
	
	switch ( *seq )
	{
		case 0:
			if (WIPE_SYS_EndCheck())
			{
				(*seq) ++;
			}
			break;
		case 1:
			// 終了判定
			if(wk->Step == STEP_END){
				(*seq) ++;
			}
			break;
		case 2:
			WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
							WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapID );
			(*seq) ++;
			break;
		//
		// ここで最終終了待ちをしているようだ
		case 3:
			if (WIPE_SYS_EndCheck())
			{
				return GFL_PROC_RES_FINISH;
			}
			break;
		default:
			break;
	}
	
	//----------------
	// 処理を追加
	//----------------
	td_main( wk );
	
	// 会話ウインドウ用
	GFL_TCBL_Main( wk->MsgTcblSys );
	
	//PRINT_QUE
	td_main_print( wk );
	
	//2D描画
	T_DOWNLOAD_GRAPHIC_2D_Draw( wk->graphic );
	
	//3D描画
	T_DOWNLOAD_GRAPHIC_3D_StartDraw( wk->graphic );
	
	T_DOWNLOAD_GRAPHIC_3D_EndDraw( wk->graphic );
	
	return GFL_PROC_RES_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(init)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_init( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32			i;
	
	// バッファを確保
	wk->MsgBuffer = GFL_STR_CreateBuffer( 256, wk->heapID );
	
	// 初期化
	// それほど複雑ではないので、
	// 上、下画面をまとめて処理
	wk->Step 		= STEP_INIT;
	wk->Mode		= wk->Param->status;	// 分岐
	wk->Flag		= 0;
	wk->CmnFlag		= 0;
//	wk->CurPos		= 0;
//	wk->TopNum		= 0;
//	wk->SubNum		= 0;
	wk->ScrollOffy  = 0;
	wk->DispMaxNum	= 0;
	//
	for(i=0;i<3;i++){
		wk->AllCurPos[i]		= 0;
		wk->ListCurPos[i]		= 0;
		wk->ListScrollPos[i] 	= 0;
	}
	wk->PCount		= 0;
	wk->InfoCurPos	= 0;
	wk->RetMode		= -1;
	//
//	for(i=0;i<T_DOWNLOAD_SAVE_BUFFER_MAX;i++){
//		wk->SaveBuffer[i] = -1;
//	}
	wk->frame_list_work = 0;
	//
	// test
//	wk->SaveBuffer[0] = 0;
//	wk->SaveBuffer[1] = 2;
//	wk->SaveBuffer[2] = 3;
	
#if 0
	for(i=0;i<T_DOWNLOAD_ITEM_BUFFER_MAX;i++){
		td_set_item( wk, i, 0xff, 0 );
	}
	// test
	for(i=0;i<10;i++){
		td_set_item( wk, i, i, 0 );
	}
#endif

	// empty用バッファ取得(必ず破棄すること)
	wk->EmptyBuffer = GFL_ARC_LoadDataAlloc( ARCID_T_DOWNLOAD_GRA, NARC_t_download_gra_t_download_pos_tblbin, wk->heapID );
	// パラメータ用のバッファ取得(必ず破棄すること)
	wk->ParamBuffer = GFL_ARC_LoadDataAlloc( ARCID_T_DOWNLOAD_GRA, NARC_t_download_gra_t_download_param_tblbin, wk->heapID );
	
	//--------------------------------
	// データ(bg+string+obj)の初期化
	// (yes/no)winも含む
	//--------------------------------
	TdCmn_init_data( wk );
	
	//-------------
	// 後処理
	//-------------
//	td_disp_downinfo( wk );
	//
	//--------------
	// 内部パラメータセット
	//---------------
	td_init_param( wk );
	//
	wk->Flag |= FLG_MW_UPDATE;
#if 0
	// 表示モードの場合
	if((wk->Mode == MODE_SAVE)||(wk->Mode == MODE_JOIN)||(wk->Mode == MODE_INFO)){
//		wk->Flag |= (FLG_MW_LEFT_HIDE|FLG_MW_RIGHT_HIDE);
	}
	// 通常モードの場合
	else{
		wk->SaveBuffer[0] = -1;
		wk->SaveBuffer[1] = -1;
		wk->SaveBuffer[2] = -1;
	}
#endif
	//--------------
	// meswin
	//--------------
	// 汎用PrintStream用
	APP_PRINTSYS_COMMON_PrintStreamInit( &wk->PrintSysWork, APP_PRINTSYS_COMMON_TYPE_BOTH );
//	APP_PRINTSYS_COMMON_PrintStreamInit( &wk->PrintSysWork, APP_PRINTSYS_COMMON_TYPE_THROUGH );
	// メッセージ送りマーク用
//	wk->MsgCursorWork = APP_KEYCURSOR_Create( 15, TRUE, TRUE, wk->heapID );
	
	// 初期化まとめ
	td_sub_init( wk );
	//
	// 上画面表示(共通なので別としておく)
	TdCmn_disp_upmes( wk, 0 );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(main)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_main( T_DOWNLOAD_MAIN_WORK* wk )
{
	// 下画面
	td_down( wk );
	//
	// スクロール
	TdCmn_main_bg_scroll( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(exit)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_exit( T_DOWNLOAD_MAIN_WORK* wk )
{
	//-----------------
	// 終了パラメータ
	//------------------
	// ×で終了 => フリームーブとなる
	if(wk->Flag & FLG_MW_CROSS_END){
		if(wk->Param->result_work != NULL){
			*wk->Param->result_work = TRUE;
		}
	}
	// RETURNで終了 => パソコン画面へ
	else{
		if(wk->Param->result_work != NULL){
			*wk->Param->result_work = FALSE;
		}
	}
	// 結果
	if(wk->Param->result_work != NULL){
		OS_Printf("result : %d\n",*wk->Param->result_work);
	}else{
		OS_Printf("result : null(debug)\n");
	}
	// リスト破棄
	TdCmn_exit_framelist( wk );
	
	// 上、下画面をまとめて処理
	// emptyのバッファクリア
	GFL_HEAP_FreeMemory( wk->EmptyBuffer );
	// paramのバッファクリア
	GFL_HEAP_FreeMemory( wk->ParamBuffer );
	//	
	// データ(string+obj)破棄
	// (yes/no)winも含む
	TdCmn_exit_data( wk );
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(init)(下画面)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_sub_init( T_DOWNLOAD_MAIN_WORK* wk )
{
	// 念のため
	if((wk->Mode < 0)||(wk->Mode >= T_DOWNLOAD_MODE_MAX)){
		OS_Printf("mode error\n");
		return;
	}
	// 分岐
	Td_InitFuncTbl[wk->Mode]( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(main)(下画面)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_down( T_DOWNLOAD_MAIN_WORK* wk )
{
	// 念のため
	if((wk->Mode < 0)||(wk->Mode >= T_DOWNLOAD_MODE_MAX)){
		OS_Printf("mode error\n");
		return;
	}
	// 分岐
	Td_KeyFuncTbl[wk->Mode]( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  Queの処理
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_main_print( T_DOWNLOAD_MAIN_WORK *wk )
{
	s32		i;
	
	// Queの処理
	PRINTSYS_QUE_Main( wk->print_que );
	//
	for(i=0;i<T_DOWNLOAD_CANVAS_MAXNUM;i++){
		PRINT_UTIL_Trans( &wk->PrintUtil[i], wk->print_que );
	}
}

//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  下画面表示まとめ(仮)(現在、タイトル文字のみ)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_disp_downinfo( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		buf[2];
	s32		i;
	STRBUF	*strbuf;
	
	// 下側の個数を求める
//	buf[0] = CVS_MAIN_MES_MAX-CVS_UP_MES_MAX;
	buf[0] = 1;
	//---------
	// クリア
	//---------
	for(i=0;i<buf[0];i++){
		TdCmn_clear_canvas( wk, CVS_UP_MES_MAX+i );
	}
	//------------
	// 各項目表示
	//------------
	// タイトル
	TdCmn_disp_string_number( wk, CVS_MAIN_TITLE, 0, 0, 1000, 8, PRINTTOOL_MODE_LEFT );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  アニメの終了判定
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				(0:とび先終了判定あり、1:アニメ終了判定のみ)
 *
 *	@retval BOOL					(TRUE:終了、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL td_chk_anim( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode )
{
	s16		flg=0;
	s16		wnum=0;
	BOOL	bflg=FALSE;
	BOOL	rflg=FALSE;
	
	// 左アニメ再生中
	if(wk->Flag & FLG_MW_ANM_LEFT){
		wnum = T_DOWNLOAD_CLWKID_CMN_LEFT;
		flg = 1;
	}
	// 右アニメ再生中
	else if(wk->Flag & FLG_MW_ANM_RIGHT){
		wnum = T_DOWNLOAD_CLWKID_CMN_RIGHT;
		flg = 1;
	}
	// ×アニメ再生中
	else if(wk->Flag & FLG_MW_ANM_CROSS){
		wnum = T_DOWNLOAD_CLWKID_CMN_CROSS;
		flg = 1;
	}
	// 戻るアニメ再生中
	else if(wk->Flag & FLG_MW_ANM_RETURN){
		wnum = T_DOWNLOAD_CLWKID_CMN_RETURN;
		flg = 1;
	}
	// アニメ終了判定を行う
	if(flg == 1){
		bflg = TdCmn_is_anime_end( wk, T_DOWNLOAD_CLWKMODE_CMN, wnum );
		if(bflg == FALSE){
			flg = 2;
			// アニメの終了
			// とりあえず通常に戻しておく
			TdCmn_set_cmn_anim( wk, wnum );
			wk->Flag &= ~(1<<wnum);
			//
			// 終了判定ありの場合
			if(mode == 0){
				// 戻るボタン(飛び先変更はある)
				if((wnum == T_DOWNLOAD_CLWKID_CMN_CROSS)||(wnum == T_DOWNLOAD_CLWKID_CMN_RETURN)){
					wk->Flag |= FLG_MW_END;
					//
					// ×ボタン終了
					if(wnum == T_DOWNLOAD_CLWKID_CMN_CROSS){
						wk->Flag |= FLG_MW_CROSS_END;
					}
					// RETURNボタン終了
					else if(wnum == T_DOWNLOAD_CLWKID_CMN_RETURN){
						wk->Flag |= FLG_MW_RET_END;
					}
					// 終了処理
//					wk->Step = STEP_END;
				}
			}
			// 戻る処理
			else if(mode == 1){
				if((wnum == T_DOWNLOAD_CLWKID_CMN_CROSS)||(wnum == T_DOWNLOAD_CLWKID_CMN_RETURN)){
//					wk->Step = STEP_LV1;
					wk->Flag |= FLG_MW_UPDATE;
//					wk->Flag &= ~(FLG_MW_RIGHT_HIDE|FLG_MW_LEFT_HIDE);
				}
			}
			rflg = TRUE;
		}
	}
	//
	return( rflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  パラメータ初期化(ここでパラメータを参照)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_init_param( T_DOWNLOAD_MAIN_WORK* wk )
{
	s16					i;
	GFL_UI_TP_HITTBL	*adr = &wk->HitBuffer[0];
	//
	// タッチペン領域計算(cmnのみ)
	for(i=0;i<T_DOWNLOAD_AREA_TD_TP_CMN_MAX;i++){
		td_get_tp_area( wk, i, adr );
		adr++;
	}
	// 終了コード
	adr->rect.top		= GFL_UI_TP_HIT_END;
	adr->rect.bottom	= 0;
	adr->rect.left		= 0;
	adr->rect.right		= 0;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タッチペン入力(trg)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				(0:Trg、1:Cont)
 *	@param	const s32				エリア番号(T_DOWNLOAD_AREA_TD_TP..)
 *
 *	@retval BOOL					(TRUE:タッチあり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL td_chk_my_tp( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 num )
{
	s16		sbuf[4];
	BOOL	bflg;
	u32		buf[2];
	//
	// パネルのように、重複するエリアの場合、
	// 先に登録したものが優先されるので、
	// あらかじめテーブルに登録する方法だと処理(準備)が複雑になるので
	// 個別のエリアを参照できるのも容易しておく
	// (共有は従来の方法で対応)
	if(mode == 0){		// trg
		bflg = GFL_UI_TP_GetPointTrg(  &buf[0], &buf[1] );
	}else{				// cont
		bflg = GFL_UI_TP_GetPointCont( &buf[0], &buf[1] );
	}
	if(bflg == TRUE){
		sbuf[0] = wk->ParamBuffer[4*num];
		sbuf[1] = wk->ParamBuffer[4*num+1];
		sbuf[2] = wk->ParamBuffer[4*num+2];
		sbuf[3] = wk->ParamBuffer[4*num+3];
		//
		// x方向
		if((buf[0] >= sbuf[0])&&(buf[0] <= (sbuf[0]+sbuf[2]))){
			// y方向
			if((buf[1] >= sbuf[1])&&(buf[1] <= (sbuf[1]+sbuf[3]))){
				return( TRUE );
			}
		}
	}
	return( FALSE );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  輝度の変更
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				(割合(-16 .. +16))
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_chg_brightness( T_DOWNLOAD_MAIN_WORK* wk, const s32 rate )
{
	G2_SetBlendBrightness( GX_BLEND_PLANEMASK_BG1 |GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ, rate );
}

//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  タッチペン用データセット
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				領域の番号
 *	@param	GFL_UI_TP_HITTBL		タッチペン判定用
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_get_tp_area( T_DOWNLOAD_MAIN_WORK* wk, const s32 num, GFL_UI_TP_HITTBL *tpbuffer )
{
	s16		i;
	s16		buf[4];
	
	// 領域を取得
	// (左上(x,y)、サイズ(w,h)の4個で1つのデータ)
	for(i=0;i<4;i++){
		buf[i] = wk->ParamBuffer[4*num+i];
	}
	// ParamBufferは、(x,y,width,height)なので加算
	buf[2] += buf[0];
	buf[3] += buf[1];
	
	// 領域チェック
	for(i=0;i<4;i++){
		if(buf[i] < 0){
			buf[i] = 0;
		}
		// 判定はu8なのでmaxを255としておく
		if(buf[i] > 255){
			buf[i] = 255;
		}
	}
	// 領域
	tpbuffer[0].rect.left	= (u8)buf[0];
	tpbuffer[0].rect.top    = (u8)buf[1];
	tpbuffer[0].rect.right	= (u8)buf[2];
	tpbuffer[0].rect.bottom	= (u8)buf[3];
}

//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  初期化(Join)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_init_join( T_DOWNLOAD_MAIN_WORK* wk )
{
	// ここまでで表示数(DispMaxNum)を決めておくこと
	wk->DispMaxNum = 3;	// test
	//
	// saveの情報をセット(初期化でもいいかもしれない)
	wk->AllCurPos[T_DOWNLOAD_LIST_CURRENT] 		= wk->AllCurPos[T_DOWNLOAD_LIST_SAVE];
	wk->ListCurPos[T_DOWNLOAD_LIST_CURRENT] 	= wk->ListCurPos[T_DOWNLOAD_LIST_SAVE];
	wk->ListScrollPos[T_DOWNLOAD_LIST_CURRENT] 	= wk->ListScrollPos[T_DOWNLOAD_LIST_SAVE];
	
	// リスト初期化
	TdCmn_init_framelist( wk, wk->Mode );
	// バー非表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_BAR, FALSE );
	// カーソル表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, TRUE );
	//
	// 判定の後に表示
	TdCmn_disp_winmes( wk, CVS_MAIN_CENTER_WIN, T_DOWNLOAD_WIN_JOIN_00 );
	// 輝度変更
	td_chg_brightness( wk, TD_PASSIVE_RATE );
	//
	// カーソル位置更新
//	wk->CmnFlag |= T_DOWNLOAD_FLG_UPDATE;
	// カーソル移動
	TdCmn_disp_framelist_cursor( wk, wk->ListCurPos[T_DOWNLOAD_LIST_CURRENT] );
	//
	wk->Step = STEP_INIT+10;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  入力処理まとめ(Join)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_chk_input_join( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		flg=0;
	u32		ret;
	s32		listflg=0;
	BOOL	bflg;
	
	// リスト処理
	ret = TdCmn_main_framelist( wk );
	listflg = TdCmn_is_main_framelist( wk, ret );
	if(listflg != 0){
		flg = 1;
	}
	if(listflg == 2){
		// ウインドウ
		TdCmn_disp_winmes( wk, CVS_MAIN_MESWIN, T_DOWNLOAD_WIN_JOIN_01 );
		// 3拓ウインドウ(参加する)
		TdCmn_init_yesnowin( wk, T_DOWNLOAD_YESNO_JOIN );
		// 輝度変更
		td_chg_brightness( wk, TD_PASSIVE_RATE );
		//
		wk->Step = STEP_LV2;
	}
	// 左右キー
	else if(listflg == 3){
			wk->Step = STEP_LV1+20;
	}
	//
	// 判定
	// (ほとんどがリストのシステムでやってくれる)
	if(flg == 0){
		// タッチペン
		bflg = td_chk_tp_input_join( wk );
		if(bflg == FALSE){
			// キー
			td_chk_key_input_join( wk );
		}
	}
	//
	// 更新
	if(wk->CmnFlag & T_DOWNLOAD_FLG_UPDATE){
		// カーソル移動
		TdCmn_disp_framelist_cursor( wk, wk->ListCurPos[T_DOWNLOAD_LIST_CURRENT] );
		wk->CmnFlag &= ~T_DOWNLOAD_FLG_UPDATE;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タッチペン入力(join)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval BOOL					(TRUE:タッチあり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL td_chk_tp_input_join( T_DOWNLOAD_MAIN_WORK* wk )
{
	s16		flg=0;
	const GFL_UI_TP_HITTBL	*adr = &wk->HitBuffer[0];
	const int ret  = GFL_UI_TP_HitTrg( adr );		// なければ、TP_HIT_NONE
	BOOL	bflg=FALSE;
	
	// ×
	if(ret == T_DOWNLOAD_AREA_TD_TP_CROSS){
		TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_CROSS );
		flg = 2;
	}
	// RETURN
	else if(ret == T_DOWNLOAD_AREA_TD_TP_RETURN){
		TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RETURN );
		flg = 2;
	}
	//
	// 更新
	if(flg != 0){
		bflg = TRUE;
		wk->Step = STEP_LV1+10;
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力(join)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval BOOL				(TRUE:更新あり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL td_chk_key_input_join( T_DOWNLOAD_MAIN_WORK* wk )
{
	s16		flg=0;
	BOOL	bflg=FALSE;
	
	// ×
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
		TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_CROSS );
		flg = 1;
	}
	// 戻る
	else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
		TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RETURN );
		flg = 1;
	}
	//
	// 更新
	if(flg == 1){
		bflg = TRUE;
		wk->Step = STEP_LV1+10;
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  メッセージ表示キー入力(join)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval BOOL				(TRUE:更新あり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL td_chk_wait_mes_join( T_DOWNLOAD_MAIN_WORK* wk )
{
	BOOL	bflg=FALSE;
	
	// 入力待ち
	if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_CANCEL|PAD_BUTTON_DECIDE)){
		bflg = TRUE;
	}
	else if( GFL_UI_TP_GetTrg() == TRUE ){
		bflg = TRUE;
	}
	//
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力処理(Join)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_key_join( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		tmp;
	u32		ret;
	BOOL	bflg;
	
	// 処理
	switch(wk->Step)
	{
		// 初期化
		case STEP_INIT:
			wk->Step = STEP_LV1;
			break;
		//
		// メッセージ表示待ち
		case STEP_INIT+10:
			bflg = td_chk_wait_mes_join( wk );
			if(bflg == TRUE){
				// 終了処理
				TdCmn_clear_frame( wk );
				// 輝度戻し
				td_chg_brightness( wk, TD_ACTIVE_RATE );
				//
				wk->Step = STEP_LV1;
			}
			break;
		//
		// 本体
		case STEP_LV1:
			td_chk_input_join( wk );
			break;
		//
		// アニメ待ち
		case STEP_LV1+10:
			bflg = td_chk_anim( wk, 0 );
			if(bflg == TRUE){
				wk->Step = STEP_END;
			}
			break;
		//
		// 左右アニメ待ち
		case STEP_LV1+20:
			// main
			ret = TdCmn_main_framelist( wk );
			if(ret == FRAMELIST_RET_NONE){
				bflg = td_chk_anim( wk, 1 );
				if(bflg == TRUE){
					// 再計算
					TdCmn_calc_framelist( wk );
					wk->CmnFlag |= T_DOWNLOAD_FLG_UPDATE;
					wk->Step = STEP_LV1;
				}
			}
			break;
		//
		// LV2
		case STEP_LV2:
			// 3択ウインドウの選択処理
			tmp = TdCmn_chk_yesnowin( wk );
			if(tmp != -1){
				// 終了処理
				TdCmn_exit_yesnowin( wk );
				TdCmn_clear_frame( wk );
				// 輝度戻し
				td_chg_brightness( wk, TD_ACTIVE_RATE );
				if(tmp == 1){
					// 詳細へ移行
					wk->Step = STEP_LV3;
					// カーソル表示(ちらつくので消しておく)
					TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, FALSE );
				}else{
					wk->Step = STEP_LV1;
				}
			}
			break;
		//
		// 詳細画面へ移行
		case STEP_LV3:
			// 戻り場所をセット
			wk->RetMode = wk->Mode;
			td_exit_join( wk );
			wk->Step = STEP_INIT;
			wk->Mode = T_DOWNLOAD_MODE_INFO;
			//
			// 初期化まとめ
			td_sub_init( wk );
			break;
		//
		// END
		case STEP_END:
			break;
		//
		default:
			break;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  終了処理(Join)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *	@param	const s32			(-1：なし、戻り先の番号)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_exit_join( T_DOWNLOAD_MAIN_WORK* wk )
{
	// リストの破棄
	TdCmn_exit_framelist( wk );
	
	// saveの情報をセット
	wk->AllCurPos[T_DOWNLOAD_LIST_SAVE] 	= wk->AllCurPos[T_DOWNLOAD_LIST_CURRENT];
	wk->ListCurPos[T_DOWNLOAD_LIST_SAVE] 	= wk->ListCurPos[T_DOWNLOAD_LIST_CURRENT];
	wk->ListScrollPos[T_DOWNLOAD_LIST_SAVE] = wk->ListScrollPos[T_DOWNLOAD_LIST_CURRENT];
}

//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  詳細初期化(Info)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_init_info( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		i;
	
	// frame_listを使っていない面では必要
	// オフセット戻し
	GFL_BG_SetScrollReq( GFL_BG_FRAME2_M, GFL_BG_SCROLL_Y_SET, 0 );
	
	// アイコン表示
	for(i=0;i<3;i++){
		TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_ICON_00+i, TRUE );
	}
	// カーソル非表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, FALSE );
	
	// バー非表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_BAR, TRUE );
	
	// BGの表示
	TdCmn_init_bg_info( wk );
	//
	// パレットチェンジon
	TdCmn_set_chgpal( wk, TRUE );
	
	// 初期カーソル位置
	wk->InfoCurPos = 0;
	wk->CmnFlag |= T_DOWNLOAD_FLG_INFOUPDATE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  入力処理まとめ(Info)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_chk_input_info( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		flg=0;
	BOOL	bflg;
	
	// 判定
	// ここだけは、自前で処理する必要がある
	if(flg == 0){
		// タッチペン
		bflg = td_chk_tp_input_info( wk );
		if(bflg == FALSE){
			// キー
			td_chk_key_input_info( wk );
		}
	}
	//
	// 更新
	if(wk->CmnFlag & T_DOWNLOAD_FLG_INFOUPDATE){
		TdCmn_chg_info_curpos( wk, wk->InfoCurPos );
		// カーソル移動
		wk->CmnFlag &= ~T_DOWNLOAD_FLG_INFOUPDATE;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タッチペン入力(Info)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval BOOL					(TRUE:タッチあり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL td_chk_tp_input_info( T_DOWNLOAD_MAIN_WORK* wk )
{
	BOOL	bflg=FALSE;
	
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力(Info)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval BOOL					(TRUE:タッチあり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL td_chk_key_input_info( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		flg=0;
	s32		sflg=0;
	BOOL	bflg=FALSE;
	
	// ×
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
		TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_CROSS );
		flg = 1;
	}
	// 戻る
	else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
		TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RETURN );
		flg = 1;
	}
	// 左
	else if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT ){
		wk->InfoCurPos--;
		if(wk->InfoCurPos < 0){
			wk->InfoCurPos = 2;
		}
		sflg = 1;
	}
	// 右
	else if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT ){
		wk->InfoCurPos++;
		if(wk->InfoCurPos > 2){
			wk->InfoCurPos = 0;
		}
		sflg = 1;
	}
	//
	// カーソル切り替え
	if(sflg != 0){
		wk->CmnFlag |= T_DOWNLOAD_FLG_INFOUPDATE;
	}
	// 更新
	if(flg != 0){
		bflg = TRUE;
		if(flg == 1){
			wk->Step = STEP_LV1+10;
		}else{
			wk->Step = STEP_LV1+20;
		}
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力処理(Info)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_key_info( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		tmp;
	u32		ret;
	BOOL	bflg;
	
	// 処理
	switch(wk->Step)
	{
		// 初期化
		case STEP_INIT:
			wk->Step = STEP_LV1;
			break;
		//
		// 本体
		case STEP_LV1:
			td_chk_input_info( wk );
			break;
		//
		// アニメ待ち
		case STEP_LV1+10:
			bflg = td_chk_anim( wk, 0 );
			if(bflg == TRUE){
				// 戻り先チェック
				if(wk->RetMode == -1){
					wk->Step = STEP_END;
				}else{
					wk->Step = STEP_LV3;
				}
			}
			break;
		//
		// 左右アニメ待ち
		case STEP_LV1+20:
			bflg = td_chk_anim( wk, 1 );
			if(bflg == TRUE){
				wk->Step = STEP_LV1;
			}
			break;
		//
		// 終了戻り
		case STEP_LV3:
			// 終了戻り
			td_exit_info( wk );
			wk->Step = STEP_INIT;
			wk->Mode = wk->RetMode;
			wk->RetMode = -1;
			//
			// 初期化まとめ
			td_sub_init( wk );
			break;
		//
		// END
		case STEP_END:
			break;
		//
		default:
			break;
	}
	//
	// パレットチェンジ(詳細のobj用)
	TdCmn_main_chgpal( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  終了(Info)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_exit_info( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		i;
	
	// パレットチェンジoff
	TdCmn_set_chgpal( wk, FALSE );
	
	// BG面クリア
	GFL_BG_ClearFrame( GFL_BG_FRAME2_M );
	//
	// アイコン非表示
	for(i=0;i<3;i++){
		TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_ICON_00+i, FALSE );
	}
}

//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  選択初期化(select)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_init_select( T_DOWNLOAD_MAIN_WORK* wk )
{
	// ここまでで表示数(DispMaxNum)を決めておくこと
	wk->DispMaxNum = 10;	// test
	// selectの情報をセット
	wk->AllCurPos[T_DOWNLOAD_LIST_CURRENT] 		= wk->AllCurPos[T_DOWNLOAD_LIST_SELECT];
	wk->ListCurPos[T_DOWNLOAD_LIST_CURRENT] 	= wk->ListCurPos[T_DOWNLOAD_LIST_SELECT];
	wk->ListScrollPos[T_DOWNLOAD_LIST_CURRENT] 	= wk->ListScrollPos[T_DOWNLOAD_LIST_SELECT];
	//
	// リスト初期化
	TdCmn_init_framelist( wk, wk->Mode );
	// カーソル表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, TRUE );
	// バー非表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_BAR, TRUE );
	// カーソル位置更新
//	wk->CmnFlag |= T_DOWNLOAD_FLG_UPDATE;
	// カーソル移動
	TdCmn_disp_framelist_cursor( wk, wk->ListCurPos[T_DOWNLOAD_LIST_CURRENT] );
	// 通常
	wk->Step = STEP_INIT;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  入力処理まとめ(select)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_chk_input_select( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		flg=0;
	u32		ret;
	s32		listflg=0;
	BOOL	bflg;
	
	// リスト処理
	ret = TdCmn_main_framelist( wk );
	listflg = TdCmn_is_main_framelist( wk, ret );
	if(listflg != 0){
		flg = 1;
	}
	if(listflg == 2){
		// ウインドウ
		TdCmn_disp_winmes( wk, CVS_MAIN_MESWIN, T_DOWNLOAD_WIN_WHAT_00 );
		// 3拓ウインドウ(セーブする)
		TdCmn_init_yesnowin( wk, T_DOWNLOAD_YESNO_SAVE );
		// 輝度変更
		td_chg_brightness( wk, TD_PASSIVE_RATE );
		//
		wk->Step = STEP_LV2;
	}
	// 左右キー
	else if(listflg == 3){
		wk->Step = STEP_LV1+20;
	}
	//
	// 判定
	// (ほとんどがリストのシステムでやってくれる)
	if(flg == 0){
		// タッチペン
		bflg = td_chk_tp_input_select( wk );
		if(bflg == FALSE){
			// キー
			td_chk_key_input_select( wk );
		}
	}
	//
	// 更新
	if(wk->CmnFlag & T_DOWNLOAD_FLG_UPDATE){
		// カーソル移動
		TdCmn_disp_framelist_cursor( wk, wk->ListCurPos[T_DOWNLOAD_LIST_CURRENT] );
		wk->CmnFlag &= ~T_DOWNLOAD_FLG_UPDATE;
	}
	// スクロールバーの処理
	TdCmn_main_scrollbar( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タッチペン入力(select)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval BOOL					(TRUE:タッチあり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL td_chk_tp_input_select( T_DOWNLOAD_MAIN_WORK* wk )
{
	BOOL	bflg=FALSE;
	
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力(select)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval BOOL					(TRUE:タッチあり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL td_chk_key_input_select( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		flg=0;
	BOOL	bflg=FALSE;
	
	// ×
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
		TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_CROSS );
		flg = 1;
	}
	// 戻る
	else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
		TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RETURN );
		flg = 1;
	}
	// 左
	else if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT ){
		TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_LEFT );
		flg = 2;
	}
	// 右
	else if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT ){
		TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RIGHT );
		flg = 2;
	}
	//
	// 更新
	if(flg != 0){
		bflg = TRUE;
		if(flg == 1){
			wk->Step = STEP_LV1+10;
		}else{
			wk->Step = STEP_LV1+20;
		}
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力処理(select)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_key_select( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		tmp;
	u32		ret;
	BOOL	bflg;
	
	// 処理
	switch(wk->Step)
	{
		// 初期化
		case STEP_INIT:
			wk->Step = STEP_LV1;
			break;
		//
		// 本体
		case STEP_LV1:
			td_chk_input_select( wk );
			break;
		//
		// アニメ待ち
		case STEP_LV1+10:
			bflg = td_chk_anim( wk, 0 );
			if(bflg == TRUE){
				wk->Step = STEP_END;
			}
			break;
		//
		// 左右アニメ待ち
		case STEP_LV1+20:
			bflg = td_chk_anim( wk, 1 );
			if(bflg == TRUE){
				wk->Step = STEP_LV1;
			}
			break;
		//
		// LV2
		case STEP_LV2:
			// 3択ウインドウの選択処理
			tmp = TdCmn_chk_yesnowin( wk );
			if(tmp != -1){
				// 終了処理
				TdCmn_exit_yesnowin( wk );
				TdCmn_clear_frame( wk );
				// 輝度戻し
				td_chg_brightness( wk, TD_ACTIVE_RATE );
				// セーブ画面
				if(tmp == 0){
					wk->Step = STEP_LV4;
					// カーソル表示(ちらつくので消しておく)
					TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, FALSE );
				}
				// 詳細画面へ
				else if(tmp == 1){
					wk->Step = STEP_LV3;
					// カーソル表示(ちらつくので消しておく)
					TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, FALSE );
				}
				// 戻る
				else{
					wk->Step = STEP_LV1;
				}
			}
			break;
		//
		// 詳細画面へ移行
		case STEP_LV3:
			// 戻り場所をセット
			wk->RetMode = wk->Mode;
			td_exit_select( wk );
			wk->Step = STEP_INIT;
			wk->Mode = T_DOWNLOAD_MODE_INFO;
			//
			// 初期化まとめ
			td_sub_init( wk );
			break;
		//
		// セーブ画面へ移行
		case STEP_LV4:
			// 戻り場所をセット
			wk->RetMode = wk->Mode;
			td_exit_select( wk );
			wk->Step = STEP_INIT;
			wk->Mode = T_DOWNLOAD_MODE_SAVE;
			//
			// 初期化まとめ
			td_sub_init( wk );
			break;
		//
		// END
		case STEP_END:
			break;
		//
		default:
			break;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  終了処理(select)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_exit_select( T_DOWNLOAD_MAIN_WORK* wk )
{
	// リストの破棄
	TdCmn_exit_framelist( wk );
	
	// BG面クリア
	GFL_BG_ClearFrame( GFL_BG_FRAME2_M );
	//
	// selectの情報をセット
	wk->AllCurPos[T_DOWNLOAD_LIST_SELECT] 		= wk->AllCurPos[T_DOWNLOAD_LIST_CURRENT];
	wk->ListCurPos[T_DOWNLOAD_LIST_SELECT] 		= wk->ListCurPos[T_DOWNLOAD_LIST_CURRENT];
	wk->ListScrollPos[T_DOWNLOAD_LIST_SELECT] 	= wk->ListScrollPos[T_DOWNLOAD_LIST_CURRENT];
}

//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  初期化(Save)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_init_save( T_DOWNLOAD_MAIN_WORK* wk )
{
	// ここまでで表示数(DispMaxNum)を決めておくこと
	wk->DispMaxNum = 3;	// test
	// saveの情報をセット(初期化でもいいかもしれない)
	wk->AllCurPos[T_DOWNLOAD_LIST_CURRENT] 		= wk->AllCurPos[T_DOWNLOAD_LIST_SAVE];
	wk->ListCurPos[T_DOWNLOAD_LIST_CURRENT] 	= wk->ListCurPos[T_DOWNLOAD_LIST_SAVE];
	wk->ListScrollPos[T_DOWNLOAD_LIST_CURRENT] 	= wk->ListScrollPos[T_DOWNLOAD_LIST_SAVE];
	//
	// リスト初期化
	TdCmn_init_framelist( wk, wk->Mode );
	// バー非表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_BAR, FALSE );
	// カーソル表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, TRUE );
	// カーソル位置更新
//	wk->CmnFlag |= T_DOWNLOAD_FLG_UPDATE;
	// カーソル移動
	TdCmn_disp_framelist_cursor( wk, wk->ListCurPos[T_DOWNLOAD_LIST_CURRENT] );
	// 判定の後に表示
//	TdCmn_disp_winmes( wk, CVS_MAIN_CENTER_WIN, 0 );
	// 輝度変更
//	td_chg_brightness( wk, TD_PASSIVE_RATE );
	//
//	wk->Step = STEP_INIT+10;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  入力処理まとめ(Save)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_chk_input_save( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		flg=0;
	u32		ret;
	s32		listflg=0;
	BOOL	bflg;
	
	// リスト処理
	ret = TdCmn_main_framelist( wk );
	listflg = TdCmn_is_main_framelist( wk, ret );
	if(listflg != 0){
		flg = 1;
	}
//	if(listflg == 2){
//		// ウインドウ
//		TdCmn_disp_winmes( wk, CVS_MAIN_MESWIN, 1 );
//		// 3拓ウインドウ
//		TdCmn_init_yesnowin( wk );
//		// 輝度変更
//		td_chg_brightness( wk, TD_PASSIVE_RATE );
//		//
//		wk->Step = STEP_LV2;
//	}
	// 左右キー
//	else if(listflg == 3){
	if(listflg == 3){
			wk->Step = STEP_LV1+20;
	}
	//
	// 判定
	// (ほとんどがリストのシステムでやってくれる)
	if(flg == 0){
		// タッチペン
		bflg = td_chk_tp_input_save( wk );
		if(bflg == FALSE){
			// キー
			td_chk_key_input_save( wk );
		}
	}
	//
	// 更新
	if(wk->CmnFlag & T_DOWNLOAD_FLG_UPDATE){
		// カーソル移動
		TdCmn_disp_framelist_cursor( wk, wk->ListCurPos[T_DOWNLOAD_LIST_CURRENT] );
		wk->CmnFlag &= ~T_DOWNLOAD_FLG_UPDATE;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タッチペン入力(Save)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval BOOL					(TRUE:タッチあり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL td_chk_tp_input_save( T_DOWNLOAD_MAIN_WORK* wk )
{
	s16		flg=0;
	const GFL_UI_TP_HITTBL	*adr = &wk->HitBuffer[0];
	const int ret  = GFL_UI_TP_HitTrg( adr );		// なければ、TP_HIT_NONE
	BOOL	bflg=FALSE;
	// ×
	if(ret == T_DOWNLOAD_AREA_TD_TP_CROSS){
		TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_CROSS );
		flg = 2;
	}
	// RETURN
	else if(ret == T_DOWNLOAD_AREA_TD_TP_RETURN){
		TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RETURN );
		flg = 2;
	}
	//
	// 更新
	if(flg != 0){
		bflg = TRUE;
		wk->Step = STEP_LV1+10;
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力(save)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval BOOL				(TRUE:更新あり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL td_chk_key_input_save( T_DOWNLOAD_MAIN_WORK* wk )
{
	s16		flg=0;
	BOOL	bflg=FALSE;
	// ×
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
		TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_CROSS );
		flg = 1;
	}
	// 戻る
	else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
		TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RETURN );
		flg = 1;
	}
	//
	// 更新
	if(flg == 1){
		bflg = TRUE;
		wk->Step = STEP_LV1+10;
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  メッセージ表示キー入力(save)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval BOOL				(TRUE:更新あり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL td_chk_wait_mes_save( T_DOWNLOAD_MAIN_WORK* wk )
{
	BOOL	bflg=FALSE;
#if 0
	// 入力待ち
	if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_CANCEL|PAD_BUTTON_DECIDE)){
		bflg = TRUE;
	}
	else if( GFL_UI_TP_GetTrg() == TRUE ){
		bflg = TRUE;
	}
#endif
	//
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力処理(save)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_key_save( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		tmp;
	u32		ret;
	BOOL	bflg;
	
	// 処理
	switch(wk->Step)
	{
		// 初期化
		case STEP_INIT:
			wk->Step = STEP_LV1;
			break;
		//
		// メッセージ表示待ち
		case STEP_INIT+10:
//			bflg = td_chk_wait_mes_join( wk );
//			if(bflg == TRUE){
//				// 終了処理
//				TdCmn_clear_frame( wk );
//				// 輝度戻し
//				td_chg_brightness( wk, TD_ACTIVE_RATE );
//				//
				wk->Step = STEP_LV1;
//			}
			break;
		//
		// 本体
		case STEP_LV1:
			td_chk_input_save( wk );
			break;
		//
		// アニメ待ち
		case STEP_LV1+10:
			bflg = td_chk_anim( wk, 0 );
			if(bflg == TRUE){
				// 戻り先チェック
				if(wk->RetMode == -1){
					wk->Step = STEP_END;
				}else{
					// カーソル表示(ちらつくので消しておく)
					TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, FALSE );
					wk->Step = STEP_LV3;
				}
			}
			break;
		//
		// 左右アニメ待ち
		case STEP_LV1+20:
			// main
			ret = TdCmn_main_framelist( wk );
			if(ret == FRAMELIST_RET_NONE){
				bflg = td_chk_anim( wk, 1 );
				if(bflg == TRUE){
					// 再計算
					TdCmn_calc_framelist( wk );
					wk->CmnFlag |= T_DOWNLOAD_FLG_UPDATE;
					wk->Step = STEP_LV1;
				}
			}
			break;
		//
		// LV2
		case STEP_LV2:
			// 3択ウインドウの選択処理
			tmp = TdCmn_chk_yesnowin( wk );
			if(tmp != -1){
				// 終了処理
				TdCmn_exit_yesnowin( wk );
				TdCmn_clear_frame( wk );
				//
				wk->Step = STEP_LV1;
				// 輝度戻し
				td_chg_brightness( wk, TD_ACTIVE_RATE );
			}
			break;
		//
		// 選択画面へ移行
		case STEP_LV3:
			// 戻り場所をセット
			wk->RetMode = wk->Mode;
			td_exit_save( wk );
			wk->Step = STEP_INIT;
			wk->Mode = T_DOWNLOAD_MODE_SELECT;
			//
			// 初期化まとめ
			td_sub_init( wk );
			break;
		//
		// END
		case STEP_END:
			break;
		//
		default:
			break;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  終了処理(save)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_exit_save( T_DOWNLOAD_MAIN_WORK* wk )
{
	// リストの破棄
	TdCmn_exit_framelist( wk );
	
	// BG面クリア
	GFL_BG_ClearFrame( GFL_BG_FRAME2_M );
	//
	// saveに情報をセット
	wk->AllCurPos[T_DOWNLOAD_LIST_SAVE] 		= wk->AllCurPos[T_DOWNLOAD_LIST_CURRENT];
	wk->ListCurPos[T_DOWNLOAD_LIST_SAVE] 		= wk->ListCurPos[T_DOWNLOAD_LIST_CURRENT];
	wk->ListScrollPos[T_DOWNLOAD_LIST_SAVE] 	= wk->ListScrollPos[T_DOWNLOAD_LIST_CURRENT];
}

