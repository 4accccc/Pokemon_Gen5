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
#include "win_record_gra.naix"		// データは流用

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

// モード
enum{
	MODE_VIEW = 0,						// 表示(削除可能)
	MODE_SELECT,						// 選択(通常)
	MODE_JOIN,							// 参加
	MODE_INFO,							// 詳細(test)
	//
	MODE_MAX,
};

// empty
enum{
	EMPTY_PANEL_OFFSET 	= 5,			// パネルのオフセット
	//
	EMPTY_PANEL_POS4	= 0,			// パネルの座標(4段)
	EMPTY_PANEL_POS3	= 14,			// パネルの座標(3段)
};

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

// シーケンス
enum{
	CMN_LEFT_SEQ 		= 4,			// 左
	CMN_RIGHT_SEQ 		= 5,			// 右
	CMN_CROSS_SEQ 		= 0,			// ×
	CMN_RETURN_SEQ 		= 1,			// 戻る
	//
	CMN_LEFT_ON_SEQ 	= 12,			// 左オン
	CMN_LEFT_OFF_SEQ 	= 18,			// 左オフ
	CMN_RIGHT_ON_SEQ 	= 13,			// 右オン
	CMN_RIGHT_OFF_SEQ 	= 19,			// 右オフ
	//
	CMN_CROSS_ON_SEQ	= 8,			// ×オン
	CMN_RETURN_ON_SEQ	= 9,			// 戻るオン
};

// パネルのシーケンス(仮)
enum{
	PANEL_NO_READ_SEQ	= 0,			// 未読
	PANEL_READ_SEQ		= 1,			// 既読
	PANEL_SAVE_SEQ		= 2,			// セーブ済み
};

// 最大など
enum{
	PANEL_DISP4			= 4,			// パネル表示(4枚)
	PANEL_DISP3			= 3,			// パネル表示(3枚)
};

// いろいろ
enum{
	TD_ACTIVE_RATE		= 0,			// アクティブの割合(戻し)
	TD_PASSIVE_RATE 	= -8,			// パッシブの割合
};

// キー判定
enum{
	KEY_INPUT_BEFORE = 0,				// 前
	KEY_INPUT_AFTER,					// 後
};

// スクロール
enum{
	SCRL_TOP_Y			= 32,			// レール上部のY座標
	SCRL_DOWN_Y			= 160,			// レール下部のY座標
	SCRL_BAR_SIZE		= 16,			// バーのサイズ
};

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
	FLG_MW_DISP3		= (1<<4),		// パネル3個表示
	FLG_MW_UPDATE		= (1<<5),		// 更新
	FLG_MW_NOITEM		= (1<<6),		// パネルなし(全削除状態)
	FLG_MW_NOITEM_MES	= (1<<7),		// ありませんメッセージ表示
	//
	FLG_MW_LEFT_HIDE	= (1<<8),		// 左矢印
	FLG_MW_RIGHT_HIDE	= (1<<9),		// 右矢印
	FLG_MW_CROSS_HIDE	= (1<<10),		// ×
	FLG_MW_RETURN_HIDE	= (1<<11),		// RETURN
	//
	FLG_MW_RET_END		= (1<<12),		// RETURNで終了
	FLG_MW_CROSS_END	= (1<<13),		// ×で終了
	FLG_MW_BAR_UPDATE	= (1<<14),		// バー更新
	FLG_MW_BAR_HIDE		= (1<<15),		// バー非表示
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
//---------------------------------
// 共通ボタン
// (通常、オン、オフ(操作不可)、0)
// 全部は必要ないが、念のため用意しておく
//---------------------------------
static const s16 td_cmn_btn_tbl[] = {
	CMN_LEFT_SEQ,   CMN_LEFT_ON_SEQ,   CMN_LEFT_OFF_SEQ,  0,	// 左
	CMN_RIGHT_SEQ,  CMN_RIGHT_ON_SEQ,  CMN_RIGHT_OFF_SEQ, 0,	// 右
	CMN_CROSS_SEQ,  CMN_CROSS_ON_SEQ,  CMN_CROSS_SEQ,     0,	// ×
	CMN_RETURN_SEQ, CMN_RETURN_ON_SEQ, CMN_RETURN_SEQ,    0,	// 戻る
};

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
static void td_key_view( T_DOWNLOAD_MAIN_WORK* wk );
static void td_key_select( T_DOWNLOAD_MAIN_WORK* wk );
static void td_key_join( T_DOWNLOAD_MAIN_WORK* wk );
//
static void td_main_print( T_DOWNLOAD_MAIN_WORK *wk );
//
// 情報表示
static void td_disp_upinfo( T_DOWNLOAD_MAIN_WORK* wk, const s32 num );
static void td_disp_downinfo( T_DOWNLOAD_MAIN_WORK* wk );
static void td_set_panel( T_DOWNLOAD_MAIN_WORK* wk, const s32 wnum, const s32 num, const s32 smode, const BOOL bflg );
static void td_set_cursor( T_DOWNLOAD_MAIN_WORK* wk, const s32 wnum, const s32 num, const s32 smode, const BOOL bflg );
static void td_param_cmn_update( T_DOWNLOAD_MAIN_WORK* wk );
static void td_param_view_update( T_DOWNLOAD_MAIN_WORK* wk );
static void td_param_select_update( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_select_key_input( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_join_key_input( T_DOWNLOAD_MAIN_WORK* wk );
static void td_set_cmn_anim( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );
static BOOL td_chk_view_key_input( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_anim( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );
static void td_init_param( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_view_tp_input( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_select_tp_input( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_join_tp_input( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_my_tp( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 num );
static BOOL td_set_scrollbar( T_DOWNLOAD_MAIN_WORK* wk );
static void td_calc_scrollbar_pos( T_DOWNLOAD_MAIN_WORK* wk, const s32 num );

//
// バッファ
static s32  td_calc_savebuffer( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_savebuffer( T_DOWNLOAD_MAIN_WORK* wk, const s32 num );
static s32  td_del_savebuffer( T_DOWNLOAD_MAIN_WORK* wk, const s32 wnum );
static void td_chk_delete( T_DOWNLOAD_MAIN_WORK* wk );
//
// item
static void td_set_item( T_DOWNLOAD_MAIN_WORK* wk, const s32 wnum, const s32 num, const s32 mode );
static s32  td_calc_itembuffer( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_add_item( T_DOWNLOAD_MAIN_WORK* wk );
//
// 輝度変更
static void td_chg_brightness( T_DOWNLOAD_MAIN_WORK* wk, const s32 rate );
//
// タッチペン
static void td_get_tp_area( T_DOWNLOAD_MAIN_WORK* wk, const s32 num, GFL_UI_TP_HITTBL *tpbuffer );
//

//------------------
// 関数のテーブル
//------------------
static void (*(Td_KeyFuncTbl[]))(T_DOWNLOAD_MAIN_WORK*) = {
	td_key_view,			// view
	td_key_select,			// select
	td_key_join,			// join
	td_key_view,			// info
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
	
	// メッセージ
	wk->msg[0] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wbt_tournament_dat, wk->heapID );
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
	wk->CurPos		= 0;
	wk->TopNum		= 0;
	wk->SubNum		= 0;
	//
	for(i=0;i<T_DOWNLOAD_SAVE_BUFFER_MAX;i++){
		wk->SaveBuffer[i] = -1;
	}
	// test
	wk->SaveBuffer[0] = 0;
	wk->SaveBuffer[1] = 2;
	wk->SaveBuffer[2] = 3;
	
	for(i=0;i<T_DOWNLOAD_ITEM_BUFFER_MAX;i++){
		td_set_item( wk, i, 0xff, 0 );
	}
	// test
	for(i=0;i<10;i++){
		td_set_item( wk, i, i, 0 );
	}
	
	// empty用バッファ取得(必ず破棄すること)
	wk->EmptyBuffer = GFL_ARC_LoadDataAlloc( ARCID_WIN_RECORD_GRA, NARC_win_record_gra_t_download_pos_tblbin, wk->heapID );
	// パラメータ用のバッファ取得(必ず破棄すること)
	wk->ParamBuffer = GFL_ARC_LoadDataAlloc( ARCID_WIN_RECORD_GRA, NARC_win_record_gra_t_download_param_tblbin, wk->heapID );
	
	//--------------------------------
	// データ(bg+string+obj)の初期化
	// (yes/no)winも含む
	//--------------------------------
	TdCmn_init_data( wk );
	
	//-------------
	// 後処理
	//-------------
	td_disp_downinfo( wk );
	//
	//--------------
	// 内部パラメータセット
	//---------------
	td_init_param( wk );
	//
	wk->Flag |= FLG_MW_UPDATE;
	// 表示モードの場合
	if((wk->Mode == MODE_VIEW)||(wk->Mode == MODE_JOIN)||(wk->Mode == MODE_INFO)){
		wk->Flag |= FLG_MW_DISP3;
		wk->Flag |= (FLG_MW_LEFT_HIDE|FLG_MW_RIGHT_HIDE);
		wk->Flag |= FLG_MW_BAR_HIDE;
		//
		td_param_view_update( wk );
		// バー非表示
		TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_BAR, FALSE );
	}
	// 通常モードの場合
	else{
		wk->Flag &= ~FLG_MW_BAR_HIDE;
		//
		wk->SaveBuffer[0] = -1;
		wk->SaveBuffer[1] = -1;
		wk->SaveBuffer[2] = -1;
		// 必要に応じて
		td_param_select_update( wk );
		// バー非表示
		TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_BAR, TRUE );
	}
	
	//--------------
	// meswin
	//--------------
	// 汎用PrintStream用
	APP_PRINTSYS_COMMON_PrintStreamInit( &wk->PrintSysWork, APP_PRINTSYS_COMMON_TYPE_BOTH );
//	APP_PRINTSYS_COMMON_PrintStreamInit( &wk->PrintSysWork, APP_PRINTSYS_COMMON_TYPE_THROUGH );
	// メッセージ送りマーク用
//	wk->MsgCursorWork = APP_KEYCURSOR_Create( 15, TRUE, TRUE, wk->heapID );
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
#if 1	// 結果
	if(wk->Param->result_work != NULL){
		OS_Printf("result : %d\n",*wk->Param->result_work);
	}else{
		OS_Printf("result : null(debug)\n");
	}
#endif
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
	if((wk->Mode < 0)||(wk->Mode >= MODE_MAX)){
		OS_Printf("mode error\n");
		return;
	}
	// 分岐
	Td_KeyFuncTbl[wk->Mode]( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力処理(View)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_key_view( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		tmp;
	BOOL	bflg;
	
	// 処理
	switch(wk->Step)
	{
		// 初期化
		case STEP_INIT:
			wk->Step = STEP_LV1;
			break;
		//
		// LV1
		case STEP_LV1:
			// アニメチェック
			td_chk_anim( wk, 0 );
			// キー入力(view)
			bflg = td_chk_view_key_input( wk );
			if(bflg == FALSE){
				// タッチペン(view)
				td_chk_view_tp_input( wk );
			}
			// 更新(view)
			td_param_view_update( wk );
			break;
		//
		// LV2(meswin)
		case STEP_LV2:
			// meswin
			if( TdCmn_main_meswin( wk ) == FALSE ){
				// yes/noウインドウ
				TdCmn_init_yesnowin( wk );
				//
				wk->Step = STEP_LV2+10;
			}
			break;
		//
		// LV2+10(yes/no)
		case STEP_LV2+10:
			// yes/noの処理(update)
			if( wk->pAppTask ){
				APP_TASKMENU_UpdateMenu( wk->pAppTask );
			}
			// yes/noウインドウの選択処理
			tmp = TdCmn_chk_yesnowin( wk );
			if(tmp != -1){
				wk->Step = STEP_LV1;
				if(tmp == 0){		// NOの処理
					
				}else if(tmp == 1){	// YESの処理
					// 削除
					td_chk_delete( wk );
				}
				// 輝度戻し(test)
				td_chg_brightness( wk, TD_ACTIVE_RATE );
			}
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
 *	@brief  キー入力処理(Select)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_key_select( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		tmp;
	BOOL	bflg;
	
	// 処理
	switch(wk->Step)
	{
		// 初期化
		case STEP_INIT:
			wk->Step = STEP_LV1;
			break;
		//
		// LV1
		case STEP_LV1:
			// アニメチェック
			td_chk_anim( wk, 0 );
			// キー入力(select)
			bflg = td_chk_select_key_input( wk );
			if(bflg == FALSE){
				td_chk_select_tp_input( wk );
			}
			// 更新(select)
			td_param_select_update( wk );
			break;
		//
		// LV2(meswin)
		case STEP_LV2:
			// meswin
			if( TdCmn_main_meswin( wk ) == FALSE ){
				// yes/noウインドウ
				TdCmn_init_yesnowin( wk );
				//
				wk->Step = STEP_LV2+10;
			}
			break;
		//
		// LV2+10(yes/no)
		case STEP_LV2+10:
			// yes/noの処理(update)
			if( wk->pAppTask ){
				APP_TASKMENU_UpdateMenu( wk->pAppTask );
			}
			// yes/noウインドウの選択処理
			tmp = TdCmn_chk_yesnowin( wk );
			if(tmp != -1){
				wk->Step = STEP_LV1;
				if(tmp == 0){			// NOの処理
				}
				else if(tmp == 1){		// YESの処理
					// 登録
					bflg = td_chk_add_item( wk );
					// セーブできない=>3つセーブされている
					if(bflg == FALSE){
						wk->Step = STEP_LV3;
						wk->CurPos = 0;
						wk->Flag |= FLG_MW_DISP3;
						wk->Flag |= FLG_MW_UPDATE;
						wk->Flag |= (FLG_MW_RIGHT_HIDE|FLG_MW_LEFT_HIDE);
						wk->Flag |= FLG_MW_BAR_HIDE;
					}
				}
				// 輝度戻し(test)
				td_chg_brightness( wk, TD_ACTIVE_RATE );
			}
			break;
		//
		// LV3(上書きセーブ画面)
		case STEP_LV3:
			// アニメチェック
			td_chk_anim( wk, 1 );
			// キー入力(select)
			bflg = td_chk_select_key_input( wk );
			if(bflg == FALSE){
				td_chk_select_tp_input( wk );
			}
			// 更新(select)
			td_param_select_update( wk );
			break;
		//
		// LV4(上書きセーブ)
		case STEP_LV4:
			// meswin
			if( TdCmn_main_meswin( wk ) == FALSE ){
				// yes/noウインドウ
				TdCmn_init_yesnowin( wk );
				//
				wk->Step = STEP_LV4+10;
			}
			break;
		//
		// LV4+10(yes/no)
		case STEP_LV4+10:
			// yes/noの処理(update)
			if( wk->pAppTask ){
				APP_TASKMENU_UpdateMenu( wk->pAppTask );
			}
			// yes/noウインドウの選択処理
			tmp = TdCmn_chk_yesnowin( wk );
			if(tmp != -1){
				if(tmp == 0){			// NOの処理
					wk->Step = STEP_LV3;
				}
				else if(tmp == 1){		// YESの処理
					wk->Flag |= FLG_MW_UPDATE;
					wk->Flag &= ~FLG_MW_DISP3;
					wk->Flag &= ~(FLG_MW_RIGHT_HIDE|FLG_MW_LEFT_HIDE);
					wk->Flag &= ~FLG_MW_BAR_HIDE;
					wk->Step = STEP_LV1;
					// アイテム戻し
					tmp = wk->SaveBuffer[wk->CurPos];
					td_set_item( wk, tmp, tmp, PANEL_NO_READ_SEQ );
					// セット
					tmp = wk->TopNum+wk->SubNum;
					wk->SaveBuffer[wk->CurPos] = tmp;
					td_set_item( wk, tmp, tmp, PANEL_SAVE_SEQ );
				}
				// 輝度戻し(test)
				td_chg_brightness( wk, TD_ACTIVE_RATE );
			}
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
	BOOL	bflg;
	
	// 処理
	switch(wk->Step)
	{
		// 初期化
		case STEP_INIT:
			wk->Step = STEP_LV1;
			break;
		//
		// LV1
		case STEP_LV1:
			// アニメチェック
			td_chk_anim( wk, 0 );
			// キー入力(join)
			bflg = td_chk_join_key_input( wk );
			if(bflg == FALSE){
				// タッチペン(join)
				td_chk_join_tp_input( wk );
			}
			// 更新(view)
			td_param_view_update( wk );
			break;
		//
		// LV2(meswin)
		case STEP_LV2:
			// meswin
			if( TdCmn_main_meswin( wk ) == FALSE ){
				// yes/noウインドウ
				TdCmn_init_yesnowin( wk );
				//
				wk->Step = STEP_LV2+10;
			}
			break;
		//
		// LV2+10(yes/no)
		case STEP_LV2+10:
			// yes/noの処理(update)
			if( wk->pAppTask ){
				APP_TASKMENU_UpdateMenu( wk->pAppTask );
			}
			// yes/noウインドウの選択処理
			tmp = TdCmn_chk_yesnowin( wk );
			if(tmp != -1){
				wk->Step = STEP_LV1;
				if(tmp == 0){		// NOの処理
					
				}else if(tmp == 1){	// YESの処理
					// 削除
//					td_chk_delete( wk );
				}
				// 輝度戻し(test)
				td_chg_brightness( wk, TD_ACTIVE_RATE );
			}
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
 *	@brief  パネル表示まとめ
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			ワーク番号
 *	@param	const s32			直接シーケンス番号
 *	@param	const s32			(0:4行、1:3行)
 *	@param	const BOOL			(TRUE:表示、FALSE:非表示)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_set_panel( T_DOWNLOAD_MAIN_WORK* wk, const s32 wnum, const s32 num, const s32 smode, const BOOL bflg )
{
	s16				buf[4];
	s32				tmp;
	GFL_CLACTPOS	pos;
	STRBUF			*strbuf;
	
	// 表示/非表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, wnum, bflg );
	// bmpoam
	BmpOam_ActorSetDrawEnable( wk->BmpOamWork[wnum], bflg );
	
	// 非表示の時はそのままで終了
	if(bflg == FALSE){
		return;
	}
	// シーケンスの変更
	TdCmn_chg_objseq( wk, T_DOWNLOAD_CLWKMODE_NORMAL, wnum, num );
	
	//----------
	// 数字表示
	//----------
	// test(要修正)
	if(smode == 1){
		tmp = wk->SaveBuffer[wnum];
	}else{
		tmp = wk->ItemBuffer[wk->TopNum+wnum].Num;
	}
	strbuf = GFL_STR_CreateBuffer( 64, wk->heapID );
	STRTOOL_SetNumber( strbuf, tmp, 8, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
	TdCmn_disp_bmpoam( wk, wnum, strbuf );
	//
	// bmpoamは座標も念のため再セットしておく
	buf[0] = TdCmn_get_empty( wk,  EMPTY_PANEL_OFFSET*2 );
	buf[1] = TdCmn_get_empty( wk,  EMPTY_PANEL_OFFSET*2+1 );
	//
	if(smode == 0){
		buf[2] = TdCmn_get_empty( wk,  (EMPTY_PANEL_POS4+wnum)*2 );
		buf[3] = TdCmn_get_empty( wk,  (EMPTY_PANEL_POS4+wnum)*2+1 );
	}else{
		buf[2] = TdCmn_get_empty( wk,  (EMPTY_PANEL_POS3+wnum)*2 );
		buf[3] = TdCmn_get_empty( wk,  (EMPTY_PANEL_POS3+wnum)*2+1 );
	}
	//
	// パネルの座標の取得
//	GFL_CLACT_WK_GetPos( wk->wk_obj.clwk[wnum], &pos, CLSYS_DEFREND_MAIN );
	pos.x = buf[2];
	pos.y = buf[3];
	GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[wnum], &pos, CLSYS_DEFREND_MAIN );
	// 文字列の座標セット
	BmpOam_ActorSetPos( wk->BmpOamWork[wnum],   (pos.x+buf[0]), (pos.y+buf[1]) );
	//
	// 破棄
	GFL_STR_DeleteBuffer( strbuf );
}

//-----------------------------------------------------------------------------------------------------------------
/**
 *	@brief  カーソル表示(仮)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			カーソルID
 *	@param	const s32			位置(0-2)
 *	@param	const s32			(0:4行、1:3行)
 *	@param	const BOOL			(TRUE:表示、FALSE:非表示)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_set_cursor( T_DOWNLOAD_MAIN_WORK* wk, const s32 wnum, const s32 num, const s32 smode, const BOOL bflg )
{
	s16				buf[4];
	GFL_CLACTPOS	pos;
	
	// 表示/非表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, wnum, bflg );
	// 非表示の時はそのままで終了
	if(bflg == FALSE){
		return;
	}
	// シーケンスの変更
//	td_chg_objseq( wk, T_DOWNLOAD_CLWKMODE_NORMAL, wnum, num );
	//
	if(smode == 0){
		buf[2] = TdCmn_get_empty( wk,  (EMPTY_PANEL_POS4+num)*2 );
		buf[3] = TdCmn_get_empty( wk,  (EMPTY_PANEL_POS4+num)*2+1 );
	}else{
		buf[2] = TdCmn_get_empty( wk,  (EMPTY_PANEL_POS3+num)*2 );
		buf[3] = TdCmn_get_empty( wk,  (EMPTY_PANEL_POS3+num)*2+1 );
	}
	// パネルの座標の取得
	pos.x = buf[2]+0x10;
	pos.y = buf[3]+0x10;
	GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[wnum], &pos, CLSYS_DEFREND_MAIN );
}

//-----------------------------------------------------------------------------------------------------------------
/**
 *	@brief  上画面表示まとめ(仮)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_disp_upinfo( T_DOWNLOAD_MAIN_WORK* wk, const s32 num )
{
	s32		i;
	STRBUF	*strbuf;
	
	//---------
	// クリア
	//---------
	for(i=0;i<CVS_UP_MES_MAX;i++){
		TdCmn_clear_canvas( wk, i );
	}
	//------------
	// 各項目表示
	//------------
	// 項目なしの場合は全非表示とする
	if(num != -1){
		// 項目ありの場合はnumから情報を参照
		// タイトル
		TdCmn_disp_string_number( wk, CVS_TITLE, 0, 0, num, 8, PRINTTOOL_MODE_LEFT );
		// 配信日
		TdCmn_disp_string_number( wk, CVS_DATE,  0, 0, 100, 8, PRINTTOOL_MODE_LEFT );
		// ルールタイトル
		TdCmn_disp_string_number( wk, CVS_RULE_TITLE,   0, 0, 200, 8, PRINTTOOL_MODE_LEFT );
		// ルール説明
		TdCmn_disp_string_number( wk, CVS_RULE_EXPLAIN, 0, 0, 300, 8, PRINTTOOL_MODE_LEFT );
		// 内容タイトル
		TdCmn_disp_string_number( wk, CVS_INFO_TITLE,   0, 0, 400, 8, PRINTTOOL_MODE_LEFT );
		// 内容説明
		TdCmn_disp_string_number( wk, CVS_INFO_EXPLAIN, 0, 0, 500, 8, PRINTTOOL_MODE_LEFT );
	}
}

//-----------------------------------------------------------------------------------------------------------------
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
 *	@brief  更新
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_param_cmn_update( T_DOWNLOAD_MAIN_WORK* wk )
{
	BOOL	bflg;
	
	// 左矢印
	bflg = TRUE;
	if(wk->Flag & FLG_MW_LEFT_HIDE){
		bflg = FALSE;
	}
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_CMN, T_DOWNLOAD_CLWKID_CMN_LEFT, bflg );
	//
	// 右矢印
	bflg = TRUE;
	if(wk->Flag & FLG_MW_RIGHT_HIDE){
		bflg = FALSE;
	}
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_CMN, T_DOWNLOAD_CLWKID_CMN_RIGHT, bflg );
	//
	// 項目がない
	if(wk->Flag & FLG_MW_NOITEM){
		if(!(wk->Flag & FLG_MW_NOITEM_MES)){
			TdCmn_disp_centermes( wk, CVS_MAIN_CENTER_WIN, 1 );
			wk->Flag |= FLG_MW_NOITEM_MES;
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  更新
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_param_view_update( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		num;
	s32		i;
	BOOL	bflg;
	
	// 更新
	if(wk->Flag & FLG_MW_UPDATE){
		num = td_calc_savebuffer( wk );
		if(num == 0){
			wk->Flag |= FLG_MW_NOITEM;
		}
		//
		// パネル表示
		for(i=0;i<4;i++){
			bflg = FALSE;
			if(i < num){
				bflg = TRUE;
			}
			td_set_panel( wk, i, PANEL_SAVE_SEQ, 1, bflg );
		}
		// カーソル表示
		bflg = TRUE;
		if(wk->Flag & FLG_MW_NOITEM){
			bflg = FALSE;
		}
		td_set_cursor( wk, T_DOWNLOAD_CLWKID_CURSOR, wk->CurPos, 1, bflg );
		
		//
		// 上画面更新
		if(!( wk->Flag & FLG_MW_NOITEM )){
			td_disp_upinfo( wk, wk->SaveBuffer[wk->CurPos] );
		}else{
			td_disp_upinfo( wk, -1 );
		}
		// 共通
		td_param_cmn_update( wk );
		//
		wk->Flag &= ~FLG_MW_UPDATE;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  更新
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_param_select_update( T_DOWNLOAD_MAIN_WORK* wk )
{
	s16				tmp;
	s32				num;
	s32				i;
	u32				py;
	BOOL			bflg;
	GFL_CLACTPOS	pos;
	
	// 更新
	if(wk->Flag & FLG_MW_UPDATE){
		// 通常の場合
		if(!(wk->Flag & FLG_MW_DISP3)){
			num = td_calc_itembuffer( wk );
			if(num == 0){
				wk->Flag |= FLG_MW_NOITEM;
			}
			// パネル表示
			for(i=0;i<4;i++){
				bflg = FALSE;
				if(i < num){
					bflg = TRUE;
				}
				tmp = wk->ItemBuffer[wk->TopNum+i].Mode;
				td_set_panel( wk, i, tmp, 0, bflg );
			}
			// カーソル表示
			bflg = TRUE;
			if(wk->Flag & FLG_MW_NOITEM){
				bflg = FALSE;
			}
			td_set_cursor( wk, T_DOWNLOAD_CLWKID_CURSOR, wk->SubNum, 0, bflg );
			//
			// 上画面更新
			if(!( wk->Flag & FLG_MW_NOITEM )){
				td_disp_upinfo( wk, wk->ItemBuffer[wk->TopNum+wk->SubNum].Num );
			}else{
				td_disp_upinfo( wk, -1 );
			}
			//
			// バー更新
			bflg = TRUE;
			if(wk->Flag & FLG_MW_BAR_HIDE){
				bflg = FALSE;
			}
			// バー表示/非表示
			TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_BAR, bflg );
			//
			if(wk->Flag & FLG_MW_BAR_UPDATE){
				if(!(wk->Flag & FLG_MW_NOITEM)){
					py = SCROLLBAR_GetPosY( num-1, wk->TopNum+wk->SubNum, SCRL_TOP_Y, SCRL_DOWN_Y, SCRL_BAR_SIZE );
					// 座標の更新
					GFL_CLACT_WK_GetPos( wk->wk_obj.clwk[T_DOWNLOAD_CLWKID_BAR], &pos, CLSYS_DEFREND_MAIN );
//					pos.x = 238;	// test
					pos.y = py;
					GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[T_DOWNLOAD_CLWKID_BAR], &pos, CLSYS_DEFREND_MAIN );
					//
					wk->Flag &= ~FLG_MW_BAR_UPDATE;
				}
			}
		}
		// 3行の場合
		else{
			num = td_calc_savebuffer( wk );
			if(num == 0){
				wk->Flag |= FLG_MW_NOITEM;
			}
			//
			// パネル表示
			for(i=0;i<4;i++){
				bflg = FALSE;
				if(i < num){
					bflg = TRUE;
				}
				td_set_panel( wk, i, PANEL_SAVE_SEQ, 1, bflg );
			}
			// カーソル表示
			bflg = TRUE;
			if(wk->Flag & FLG_MW_NOITEM){
				bflg = FALSE;
			}
			td_set_cursor( wk, T_DOWNLOAD_CLWKID_CURSOR, wk->CurPos, 1, bflg );
			
			//
			// 上画面更新
			if(!( wk->Flag & FLG_MW_NOITEM )){
				td_disp_upinfo( wk, wk->SaveBuffer[wk->CurPos] );
			}else{
				td_disp_upinfo( wk, -1 );
			}
			// バー更新
			bflg = TRUE;
			if(wk->Flag & FLG_MW_BAR_HIDE){
				bflg = FALSE;
			}
			// バー表示/非表示
			TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_BAR, bflg );
		}
		// 共通
		td_param_cmn_update( wk );
		//
		wk->Flag &= ~FLG_MW_UPDATE;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力(view)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval BOOL				(TRUE:更新あり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL td_chk_view_key_input( T_DOWNLOAD_MAIN_WORK* wk )
{
	s16		i;
	s16		j;
	s16		flg=0;
	s32		wnum;
	s32		tmp;
	BOOL	bflg=FALSE;
	
	// 終了判定
	if(wk->Flag & FLG_MW_END){
		return( bflg );
	}
	// アニメ再生中
	if(wk->Flag & FLG_MW_ANM_ALL){
		return( bflg );
	}
	//
	// 上
	if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP ){
		wk->CurPos--;
		if(wk->CurPos < 0){
			wk->CurPos = 0;
		}
		flg = 1;
	}
	// 下
	else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
		j = td_calc_savebuffer( wk );
		// 念のため
		if(j == 0){
			wk->CurPos = 0;
		}
		else{
			wk->CurPos++;
			if(wk->CurPos >= (j-1)){
				wk->CurPos = (j-1);
			}
		}
		flg = 1;
	}
	// 左
	else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT ){
	}
	// 右
	else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT ){
	}
	// 戻る
	else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
		// RETURNアニメ
		td_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RETURN );
	}
	// 決定(削除)
	else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE ){
		// 削除チェック
		if(!(wk->Flag & FLG_MW_NOITEM)){
			// 初期化
			TdCmn_init_meswin( wk, CVS_MAIN_MESWIN );
			// メッセージウインドウ(test)
			TdCmn_set_meswin( wk, CVS_MAIN_MESWIN, 1 );
			//
			// 輝度変更(test)
			td_chg_brightness( wk, TD_PASSIVE_RATE );
			//
			wk->Step = STEP_LV2;
		}
	}
	//
	// 更新
	if(flg == 1){
		wk->Flag |= FLG_MW_UPDATE;
		bflg = TRUE;
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力(select)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval BOOL				(TRUE:更新あり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL td_chk_select_key_input( T_DOWNLOAD_MAIN_WORK* wk )
{
	s16		i;
	s16		j;
	s16		flg=0;
	s32		wnum;
	s32		tmp;
	s32		mode;
	BOOL	bflg=FALSE;
	BOOL	sflg;
	
	// 終了判定
	if(wk->Flag & FLG_MW_END){
		return( bflg );
	}
	// アニメ再生中
	if(wk->Flag & FLG_MW_ANM_ALL){
		return( bflg );
	}
	//
	// 後日、正しく処理すること
	mode = KEY_INPUT_BEFORE;
	if(wk->Step == STEP_LV3){
		mode = KEY_INPUT_AFTER;
	}
	
	//------
	// 前
	//------
	if(mode == KEY_INPUT_BEFORE){
		// 上
		if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP ){
			// すでに0になっている
			// =>上に変動の可能性あり
			if(wk->SubNum == 0){
				wk->TopNum--;
				if(wk->TopNum < 0){
					wk->TopNum = 0;
				}
			}
			wk->SubNum--;
			if(wk->SubNum < 0){
				wk->SubNum = 0;
			}
			flg = 1;
		}
		// 下
		else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
			j = td_calc_itembuffer( wk );
			// 念のため
			if(j == 0){
				wk->SubNum = 0;
			}
			else{
				// すでに3になっている
				// =>下に変動の可能性あり
				if(wk->SubNum == (PANEL_DISP4-1)){
					wk->TopNum++;
					if(wk->TopNum >= (j-4)){
						wk->TopNum = (j-4);
					}
				}
				if(j >= PANEL_DISP4){
					j = PANEL_DISP4;
				}
				wk->SubNum++;
				if(wk->SubNum >= (j-1)){
					wk->SubNum = (j-1);
				}
			}
			flg = 1;
		}
		// 左
		else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT ){
			// 上限に達した
			if(wk->TopNum == 0){
				wk->SubNum = 0;
			}
			wk->TopNum -= PANEL_DISP4;
			if(wk->TopNum < 0){
				wk->TopNum = 0;
			}
			flg = 1;
			// 左アニメ
			td_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_LEFT );
		}
		// 右
		else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT ){
			// 個数を調べる
			j = td_calc_itembuffer( wk );
			// 上限に達している
			if(wk->TopNum == (j-4)){
				wk->SubNum = (PANEL_DISP4-1);
			}
			wk->TopNum += PANEL_DISP4;
			if(wk->TopNum >= (j-4)){
				wk->TopNum = (j-4);
			}
			flg = 1;
			// 右アニメ
			td_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RIGHT );
		}
		// 戻る
		else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
			// RETURNアニメ
			td_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RETURN );
		}
		// 決定(追加)
		else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE ){
			// 含まれているかチェック
			// =>なければ追加
			sflg = td_chk_savebuffer( wk, wk->TopNum+wk->SubNum );
			if(sflg == FALSE){
				// 初期化
				TdCmn_init_meswin( wk, CVS_MAIN_MESWIN );
				// メッセージウインドウ(test)
				TdCmn_set_meswin( wk, CVS_MAIN_MESWIN, 1 );
				// 輝度変更(test)
				td_chg_brightness( wk, TD_PASSIVE_RATE );
				//
				wk->Step = STEP_LV2;
			}
		}
		//
		// 更新
		if(flg == 1){
			wk->Flag |= FLG_MW_UPDATE;
			wk->Flag |= FLG_MW_BAR_UPDATE;
			bflg = TRUE;
		}
	}
	//-----
	// 後
	//-----
	else{
		// 上
		if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP ){
			wk->CurPos--;
			if(wk->CurPos < 0){
				wk->CurPos = 0;
			}
			flg = 1;
		}
		// 下
		else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
			j = td_calc_savebuffer( wk );
			// 念のため
			if(j == 0){
				wk->CurPos = 0;
			}
			else{
				wk->CurPos++;
				if(wk->CurPos >= (j-1)){
					wk->CurPos = (j-1);
				}
			}
			flg = 1;
		}
		// 戻る
		else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
			// RETURNアニメ
			td_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RETURN );
		}
		// 決定(削除)
		else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE ){
			// 初期化
			TdCmn_init_meswin( wk, CVS_MAIN_MESWIN );
			// メッセージウインドウ(test)
			TdCmn_set_meswin( wk, CVS_MAIN_MESWIN, 1 );
			//
			// 輝度変更(test)
			td_chg_brightness( wk, TD_PASSIVE_RATE );
			//
			wk->Step = STEP_LV4;
		}
		//
		// 更新
		if(flg == 1){
			wk->Flag |= FLG_MW_UPDATE;
			bflg = TRUE;
		}
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
static BOOL td_chk_join_key_input( T_DOWNLOAD_MAIN_WORK* wk )
{
	s16		i;
	s16		j;
	s16		flg=0;
	s32		wnum;
	s32		tmp;
	BOOL	bflg=FALSE;
	
	// 終了判定
	if(wk->Flag & FLG_MW_END){
		return( bflg );
	}
	// アニメ再生中
	if(wk->Flag & FLG_MW_ANM_ALL){
		return( bflg );
	}
	//
	// 上
	if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP ){
		wk->CurPos--;
		if(wk->CurPos < 0){
			wk->CurPos = 0;
		}
		flg = 1;
	}
	// 下
	else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
		j = td_calc_savebuffer( wk );
		// 念のため
		if(j == 0){
			wk->CurPos = 0;
		}
		else{
			wk->CurPos++;
			if(wk->CurPos >= (j-1)){
				wk->CurPos = (j-1);
			}
		}
		flg = 1;
	}
	// 左
	else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT ){
	}
	// 右
	else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT ){
	}
	// 戻る
	else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
		// RETURNアニメ
		td_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RETURN );
	}
	// 決定(削除)
	else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE ){
		// 削除チェック
		if(!(wk->Flag & FLG_MW_NOITEM)){
			// 初期化
			TdCmn_init_meswin( wk, CVS_MAIN_MESWIN );
			// メッセージウインドウ(test)
			TdCmn_set_meswin( wk, CVS_MAIN_MESWIN, 1 );
			//
			// 輝度変更(test)
			td_chg_brightness( wk, TD_PASSIVE_RATE );
			//
			wk->Step = STEP_LV2;
		}
	}
	//
	// 更新
	if(flg == 1){
		wk->Flag |= FLG_MW_UPDATE;
		bflg = TRUE;
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  アニメのセット
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				(CMNの番号:フラグと対応)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_set_cmn_anim( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode )
{
	s32		wnum;
	
	// アニメ再生
	wnum = mode;
	TdCmn_chg_objseq( wk, T_DOWNLOAD_CLWKMODE_CMN, wnum, td_cmn_btn_tbl[wnum*4+1] );
	wk->Flag |= (1<<mode);
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
			TdCmn_chg_objseq( wk, T_DOWNLOAD_CLWKMODE_CMN, wnum, td_cmn_btn_tbl[wnum*4] );
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
					wk->Step = STEP_END;
				}
			}
			// 戻る処理
			else if(mode == 1){
				if((wnum == T_DOWNLOAD_CLWKID_CMN_CROSS)||(wnum == T_DOWNLOAD_CLWKID_CMN_RETURN)){
					wk->Step = STEP_LV1;
					wk->Flag |= FLG_MW_UPDATE;
					wk->Flag &= ~FLG_MW_DISP3;
					wk->Flag &= ~(FLG_MW_RIGHT_HIDE|FLG_MW_LEFT_HIDE);
					wk->Flag &= ~FLG_MW_BAR_HIDE;
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
 *	@brief  タッチペン入力(view)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval BOOL					(TRUE:タッチあり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL td_chk_view_tp_input( T_DOWNLOAD_MAIN_WORK* wk )
{
	s16		i;
	s16		flg=0;
	s32		num;
	s32		wnum;
	const GFL_UI_TP_HITTBL	*adr = &wk->HitBuffer[0];
	const int ret  = GFL_UI_TP_HitTrg( adr );
	const int cret = GFL_UI_TP_HitCont( adr );
	// なければ、TP_HIT_NONE
	BOOL	bflg=FALSE;
	BOOL	mflg;
	
	// 終了判定
	if(wk->Flag & FLG_MW_END){
		return( bflg );
	}
	// アニメ再生中
	if(wk->Flag & FLG_MW_ANM_ALL){
		return( bflg );
	}
	
	// ×
	if(ret == T_DOWNLOAD_AREA_TD_TP_CROSS){
		// アニメ再生
		td_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_CROSS );
		flg = 2;
	}
	// RETURN
	else if(ret == T_DOWNLOAD_AREA_TD_TP_RETURN){
		// アニメ再生
		td_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RETURN );
		flg = 2;
	}
	// セーブ領域を調べ、
	// 必要なパネルの分だけチェックする
	if(flg == 0){
		num = td_calc_savebuffer( wk );
		for(i=0;i<num;i++){
			// 個別のタッチペン判定を使用
			mflg = td_chk_my_tp( wk, 0, T_DOWNLOAD_AREA_TD_TP_PANEL3_0+i );
			// 連番なので注意
			if(mflg == TRUE){
				// カーソル位置と同じでないとだめ(仮)
				if(wk->CurPos == i){
					// 初期化
					TdCmn_init_meswin( wk, CVS_MAIN_MESWIN );
					// メッセージウインドウ(test)
					TdCmn_set_meswin( wk, CVS_MAIN_MESWIN, 1 );
					//
					// 輝度変更(test)
					td_chg_brightness( wk, TD_PASSIVE_RATE );
					//
					wk->Step = STEP_LV2;
				}
				// カーソル位置を変更(仮)
				else{
					wk->CurPos = i;
					flg = 1;
				}
				break;
			}
		}
	}
	//
	// 更新
	if(flg != 0){
		wk->Flag |= FLG_MW_UPDATE;
		bflg = TRUE;
	}
	return( bflg );
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
static BOOL td_chk_select_tp_input( T_DOWNLOAD_MAIN_WORK* wk )
{
	s16		i;
	s16		flg=0;
	s32		num;
	s32		wnum;
	s32		mode;
	const GFL_UI_TP_HITTBL	*adr = &wk->HitBuffer[0];
	const int ret  = GFL_UI_TP_HitTrg( adr );
	const int cret = GFL_UI_TP_HitCont( adr );
	// なければ、TP_HIT_NONE
	BOOL	bflg=FALSE;
	BOOL	mflg;
	BOOL	sflg;
	
	// 終了判定
	if(wk->Flag & FLG_MW_END){
		return( bflg );
	}
	// アニメ再生中
	if(wk->Flag & FLG_MW_ANM_ALL){
		return( bflg );
	}
	
	// 後日、正しく処理すること
	mode = KEY_INPUT_BEFORE;
	if(wk->Step == STEP_LV3){
		mode = KEY_INPUT_AFTER;
	}
	//-------
	// 前
	//-------
	if(mode == KEY_INPUT_BEFORE){
		// 左を押した
		if(ret == T_DOWNLOAD_AREA_TD_TP_LEFT){
			// 上限に達した
			if(wk->TopNum == 0){
				wk->SubNum = 0;
			}
			wk->TopNum -= PANEL_DISP4;
			if(wk->TopNum < 0){
				wk->TopNum = 0;
			}
			flg = 1;
			// 左アニメ
			td_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_LEFT );
		}
		// 右を押した
		else if(ret == T_DOWNLOAD_AREA_TD_TP_RIGHT){
			// 個数を調べる
			num = td_calc_itembuffer( wk );
			// 上限に達している
			if(wk->TopNum == (num-4)){
				wk->SubNum = (PANEL_DISP4-1);
			}
			wk->TopNum += PANEL_DISP4;
			if(wk->TopNum >= (num-4)){
				wk->TopNum = (num-4);
			}
			flg = 1;
			// 右アニメ
			td_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RIGHT );
		}
		// ×
		else if(ret == T_DOWNLOAD_AREA_TD_TP_CROSS){
			// アニメ再生
			td_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_CROSS );
			flg = 2;
		}
		// RETURN
		else if(ret == T_DOWNLOAD_AREA_TD_TP_RETURN){
			// アニメ再生
			td_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RETURN );
			flg = 2;
		}
		else{
			// バー
			// contで取得
			if(cret == T_DOWNLOAD_AREA_TD_TP_SCROLL){
				td_set_scrollbar( wk );
				flg = 3;
			}
		}
		// アイテムの個数を調べ、
		// 必要なパネルの分(MAX4)だけチェックする
		if(flg == 0){
			num = td_calc_itembuffer( wk );
			if(num >= PANEL_DISP4){
				num = PANEL_DISP4;
			}
			for(i=0;i<num;i++){
				// 個別のタッチペン判定を使用
				mflg = td_chk_my_tp( wk, 0, T_DOWNLOAD_AREA_TD_TP_PANEL4_0+i );
				// 連番なので注意
				if(mflg == TRUE){
					// カーソル位置と同じでないとだめ(仮)
					if(wk->SubNum == i){
						// 含まれているかチェック
						// =>なければ追加
						sflg = td_chk_savebuffer( wk, wk->TopNum+wk->SubNum );
						if(sflg == FALSE){
							// 初期化
							TdCmn_init_meswin( wk, CVS_MAIN_MESWIN );
							// メッセージウインドウ(test)
							TdCmn_set_meswin( wk, CVS_MAIN_MESWIN, 1 );
							//
							// 輝度変更(test)
							td_chg_brightness( wk, TD_PASSIVE_RATE );
							//
							wk->Step = STEP_LV2;
						}
					}
					// カーソル位置を変更(仮)
					else{
						wk->SubNum = i;
						flg = 1;
					}
					break;
				}
			}
		}
		//
		// 更新
		if(flg != 0){
			wk->Flag |= FLG_MW_UPDATE;
			wk->Flag |= FLG_MW_BAR_UPDATE;
			bflg = TRUE;
		}
	}
	//-------
	// 後
	//-------
	else{
		// ×
		if(ret == T_DOWNLOAD_AREA_TD_TP_CROSS){
			// アニメ再生
			td_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_CROSS );
			flg = 2;
		}
		// RETURN
		else if(ret == T_DOWNLOAD_AREA_TD_TP_RETURN){
			// アニメ再生
			td_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RETURN );
			flg = 2;
		}
		//
		if(flg == 0){
			num = td_calc_savebuffer( wk );
			for(i=0;i<num;i++){
				// 個別のタッチペン判定を使用
				mflg = td_chk_my_tp( wk, 0, T_DOWNLOAD_AREA_TD_TP_PANEL3_0+i );
				// 連番なので注意
				if(mflg == TRUE){
					// カーソル位置と同じでないとだめ(仮)
					if(wk->CurPos == i){
						// 初期化
						TdCmn_init_meswin( wk, CVS_MAIN_MESWIN );
						// メッセージウインドウ(test)
						TdCmn_set_meswin( wk, CVS_MAIN_MESWIN, 1 );
						//
						// 輝度変更(test)
						td_chg_brightness( wk, TD_PASSIVE_RATE );
						//
						wk->Step = STEP_LV4;
					}
					// カーソル位置を変更(仮)
					else{
						wk->CurPos = i;
						flg = 1;
					}
					break;
				}
			}
		}
		// 更新
		if(flg != 0){
			wk->Flag |= FLG_MW_UPDATE;
			bflg = TRUE;
		}
	}
	return( bflg );
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
static BOOL td_chk_join_tp_input( T_DOWNLOAD_MAIN_WORK* wk )
{
	s16		i;
	s16		flg=0;
	s32		num;
	s32		wnum;
	const GFL_UI_TP_HITTBL	*adr = &wk->HitBuffer[0];
	const int ret  = GFL_UI_TP_HitTrg( adr );
	const int cret = GFL_UI_TP_HitCont( adr );
	// なければ、TP_HIT_NONE
	BOOL	bflg=FALSE;
	BOOL	mflg;
	
	// 終了判定
	if(wk->Flag & FLG_MW_END){
		return( bflg );
	}
	// アニメ再生中
	if(wk->Flag & FLG_MW_ANM_ALL){
		return( bflg );
	}
	
	// ×
	if(ret == T_DOWNLOAD_AREA_TD_TP_CROSS){
		// アニメ再生
		td_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_CROSS );
		flg = 2;
	}
	// RETURN
	else if(ret == T_DOWNLOAD_AREA_TD_TP_RETURN){
		// アニメ再生
		td_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RETURN );
		flg = 2;
	}
	// セーブ領域を調べ、
	// 必要なパネルの分だけチェックする
	if(flg == 0){
		num = td_calc_savebuffer( wk );
		for(i=0;i<num;i++){
			// 個別のタッチペン判定を使用
			mflg = td_chk_my_tp( wk, 0, T_DOWNLOAD_AREA_TD_TP_PANEL3_0+i );
			// 連番なので注意
			if(mflg == TRUE){
				// カーソル位置と同じでないとだめ(仮)
				if(wk->CurPos == i){
					// 初期化
					TdCmn_init_meswin( wk, CVS_MAIN_MESWIN );
					// メッセージウインドウ(test)
					TdCmn_set_meswin( wk, CVS_MAIN_MESWIN, 1 );
					//
					// 輝度変更(test)
					td_chg_brightness( wk, TD_PASSIVE_RATE );
					//
					wk->Step = STEP_LV2;
				}
				// カーソル位置を変更(仮)
				else{
					wk->CurPos = i;
					flg = 1;
				}
				break;
			}
		}
	}
	//
	// 更新
	if(flg != 0){
		wk->Flag |= FLG_MW_UPDATE;
		bflg = TRUE;
	}
	return( bflg );
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
 *	@brief  スクロールバーの処理
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval BOOL					(TRUE:変更あり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL td_set_scrollbar( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32				num;
	u32				cnt;
	u32				buf[2];
	BOOL			bflg=FALSE;
	
	// ここにくるまでにスクロールバーの領域を
	// タッチしたかの判定は済んでいる
	// タッチした座標を取得
	if( GFL_UI_TP_GetPointCont( &buf[0], &buf[1] ) == TRUE ){
		// どの段階かが求まる
		num = td_calc_itembuffer( wk );
		cnt = SCROLLBAR_GetCount( num-1, buf[1], SCRL_TOP_Y, SCRL_DOWN_Y, SCRL_BAR_SIZE );
		// 現在の位置計算してセット
		td_calc_scrollbar_pos( wk, cnt );
		wk->Flag |= FLG_MW_UPDATE;
		wk->Flag |= FLG_MW_BAR_UPDATE;
		//
		bflg = TRUE;
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  スクロールバーの位置計算
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				(スクロール値)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_calc_scrollbar_pos( T_DOWNLOAD_MAIN_WORK* wk, const s32 num )
{
	// 怪しい気もするが、
	// ゲージを等分した箇所になるように、
	// yposの値が丸められるため、
	// これでいいようだ..
	//
	// マイナス方向
	if(num < wk->TopNum){
		// 0に固定
		wk->SubNum = 0;
		wk->TopNum = num;
	}
	// プラス方向(4以上)
	else if(num > (wk->TopNum+3)){
		wk->SubNum = 3;
		wk->TopNum = num-3;
	}
	// (0-3)
	else{
		// wk->TopNumは変化なし
		wk->SubNum = num-wk->TopNum;
	}
//	OS_Printf("top,sub :(%d),%d,%d,%d\n",flg,num,wk->TopNum,wk->SubNum);
}

//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief	バッファ計算
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval s32					個数
 */
//-----------------------------------------------------------------------------
static s32 td_calc_savebuffer( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		i;
	s32		j;
	
	// 個数の計算
	j=0;
	for(i=0;i<T_DOWNLOAD_SAVE_BUFFER_MAX;i++){
		if( wk->SaveBuffer[i] != -1 ){
			j++;
		}
	}
	// 個数取得
	return( j );
}

//-----------------------------------------------------------------------------
/**
 *	@brief	バッファにあるかチェック
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			番号
 *
 *	@retval s32					個数
 */
//-----------------------------------------------------------------------------
static BOOL td_chk_savebuffer( T_DOWNLOAD_MAIN_WORK* wk, const s32 num )
{
	s32		i;
	BOOL	bflg=FALSE;
	
	// 個数の計算
	for(i=0;i<T_DOWNLOAD_SAVE_BUFFER_MAX;i++){
		if( wk->SaveBuffer[i] == num){
			bflg = TRUE;
			break;
		}
	}
	//
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief	バッファ計算
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				番号
 *
 *	@retval s32						(0よりでwnumに近い値)
 */
//-----------------------------------------------------------------------------
static s32 td_del_savebuffer( T_DOWNLOAD_MAIN_WORK* wk, const s32 wnum )
{
	s32		i;
	s32		j;
	s32		ret;
	s32		buf[T_DOWNLOAD_SAVE_BUFFER_MAX];
	
	// 削除
	wk->SaveBuffer[wnum] = -1;
	
	// 初期化
	for(i=0;i<T_DOWNLOAD_SAVE_BUFFER_MAX;i++){
		buf[i] = -1;
	}
	// つめる処理
	j=0;
	for(i=0;i<T_DOWNLOAD_SAVE_BUFFER_MAX;i++){
		if(wk->SaveBuffer[i] != -1){
			buf[j] = wk->SaveBuffer[i];
			j++;
		}
	}
	//
	// ここまででbuf[]に値が入っているはず(再セット)
	for(i=0;i<T_DOWNLOAD_SAVE_BUFFER_MAX;i++){
		wk->SaveBuffer[i] = buf[i];
	}
	//
	// 削除され番号に近い値を求める
	if(j > wnum){
		ret = wnum;
	}else{
		ret = j-1;
	}
	return( ret );
}

//-----------------------------------------------------------------------------
/**
 *  @brief  deleteの処理判定
 *
 *  @param  T_DOWNLOAD_MAIN_WORK	*wk
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void td_chk_delete( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		tmp;
	
	tmp = td_del_savebuffer( wk, wk->CurPos );
	if(tmp == -1){		// 項目がない
		wk->CurPos = 0;
		wk->Flag |= FLG_MW_NOITEM;
	}else{				// 項目あり
		wk->CurPos = tmp;
	}
	wk->Flag |= FLG_MW_UPDATE;
}

//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  アイテム情報のセット
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			バッファの番号
 *	@param	const s32			番号(仮)
 *	@param	const s32			状態(未読、既読、セーブ済み)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_set_item( T_DOWNLOAD_MAIN_WORK* wk, const s32 wnum, const s32 num, const s32 mode )
{
	// 情報のセット
	wk->ItemBuffer[wnum].Num 	 = num;
	wk->ItemBuffer[wnum].Mode	 = mode;
}

//-----------------------------------------------------------------------------
/**
 *	@brief	バッファ計算
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval s32						(個数)
 */
//-----------------------------------------------------------------------------
static s32 td_calc_itembuffer( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		i;
	s32		j;
	
	// 個数の計算
	j=0;
	for(i=0;i<T_DOWNLOAD_ITEM_BUFFER_MAX;i++){
		if( wk->ItemBuffer[i].Num != 0xff ){
			j++;
		}
	}
	// 個数取得
	return( j );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  アイテム情報の登録チェック
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval BOOL				(TRUE:登録可、FALSE:不可)
 */
//-----------------------------------------------------------------------------
static BOOL td_chk_add_item( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		tmp;
	BOOL	bflg=FALSE;
	
	// セーブの個数チェック
	tmp = td_calc_savebuffer( wk );
	if(tmp >= T_DOWNLOAD_SAVE_BUFFER_MAX){
		// 最大数を超えている
	}else{
		// アイテムの登録
		td_set_item( wk, wk->TopNum+wk->SubNum, wk->TopNum+wk->SubNum, PANEL_SAVE_SEQ );
		// save領域にセット
		wk->SaveBuffer[tmp] = wk->TopNum+wk->SubNum;
		//
		wk->Flag |= FLG_MW_UPDATE;
		bflg = TRUE;
	}
	//
	return( bflg );
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
	// objはどうしよう..(要相談)
	G2_SetBlendBrightness( GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ, rate );
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

