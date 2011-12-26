//=============================================================================
/**
 *
 *	@file		t_download.c
 *	@brief  トーナメントダウンロード画面
 *	@author
 *	@date
 *
 *  共通部分は[t_download_sub.*]に移行
 *
 */
//=============================================================================
// 必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/bmp_oam.h"
#include "system/bmp_winframe.h"
#include "system/wipe.h"
#include "system/scroll_bar.h"

// FONT
#include "print/gf_font.h"
#include "font/font.naix"

// PRINT_QUE
#include "print/printsys.h"
#include "print/wordset.h"
#include "ui/print_tool.h"
#include "gamesystem/msgspeed.h"

// 描画設定
#include "t_download_graphic.h"

// 簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"
#include "ui/frame_list.h"

// アプリ共通素材
#include "app/app_menu_common.h"
#include "app/app_taskmenu.h"
#include "app/app_keycursor.h"
#include "app/app_printsys_common.h"

// サウンド
#include "sound/pm_sndsys.h"
#include "t_download_snd_def.h"

// アーカイブ
#include "arc_def.h"
#include "t_download_gra.naix"
#include "wbt_dldata.naix"

#include "t_download_local.h"
#include "app/t_download.h"
#include "t_download_sub.h"

#include "t_download_save.h"

#include "message.naix"

// ダウンロードチェック(local)
#define __DOWNLOAD_CHECK__
#undef  __DOWNLOAD_CHECK__

//=============================================================================
//=============================================================================

FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
#ifdef __DOWNLOAD_CHECK__
enum
{
	UI_TEMPLATE_HEAP_SIZE = 0x50000,  ///< ヒープサイズ(debug)
};
#endif
//
#ifndef __DOWNLOAD_CHECK__
enum
{
	UI_TEMPLATE_HEAP_SIZE = 0x40000,  ///< ヒープサイズ
};
#endif


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

//-------------------
// フラグ
//-------------------
// T_DOWNLOAD_MAIN_WORK.Flag
enum{
	FLG_MW_NOITEM		= (1<<0),		// 項目がない(本来こないはず)
	//
	FLG_MW_INFO_UPDATE	= (1<<9),		// 詳細更新
	FLG_MW_DISP_SEL_1ST	= (1<<10),		// 一度表示したのでスキップ(select)
	FLG_MW_DISP_JOIN_1ST = (1<<11),		// 一度表示したのでスキップ(join)
	//
	FLG_MW_JOIN_END		= (1<<14),		// 参加で終了
	FLG_MW_TITLE_UPDATE	= (1<<15),		// タイトル更新
	//
	FLG_MW_SELECT_SAVE	= (1<<16),		// セレクト画面で実際にセーブ
	FLG_MW_SAVED		= (1<<17),		// セーブ画面でセーブをした
	FLG_MW_SAVE2INFO	= (1<<18),		// セーブ画面から詳細に行った
	//
	FLG_MW_JOIN_UPDATE	= (1<<20),		// 更新
	FLG_MW_SAVE_UPDATE	= (1<<21),		// 更新
	FLG_MW_PAGE_MIN		= (1<<22),		// ページ最小(1ページのみ)
	//
	FLG_MW_FRAME_STAT	= (1<<24),		// フレームの状態(初期化済みかどうか)
};

// debug
// デバッグ
enum{
	FLG_DBG_ON		= (1<<0),	// on
	FLG_DBG_LEFT	= (1<<1),	// 左入力
	FLG_DBG_RIGHT	= (1<<2),	// 右入力
	FLG_DBG_UP		= (1<<3),	// 上入力
	FLG_DBG_DOWN	= (1<<4),	// 下入力
	FLG_DBG_DECIDE	= (1<<5),	// 決定
};

// 文字負荷チェック用
#define __CHK_TD__
#undef  __CHK_TD__

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
static void td_main_print( T_DOWNLOAD_MAIN_WORK *wk );
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
static BOOL td_chk_wait_mes_select( T_DOWNLOAD_MAIN_WORK* wk );
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
//
static void td_debug( T_DOWNLOAD_MAIN_WORK* wk );

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
	
	// WORDSET生成(トーナメント名がwordのdefaultの32を超えるための対応)
	wk->Wordset = WORDSET_CreateEx( WORDSET_DEFAULT_SETNUM, 40, wk->heapID );
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
	wk->TimeIconWork = 0;
	//
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
	
	// セーブ
	TdSave_exit_data( wk );
	
	// 文字送り用
	APP_KEYCURSOR_Delete( wk->MsgCursorWork );
	
	// yes/no
	APP_TASKMENU_RES_Delete( wk->pAppTaskRes );
	
	//メッセージ破棄
	for(i=0;i<T_DOWNLOAD_MSG_MAX;i++){
		if( wk->msg[i] ){
			GFL_MSG_Delete( wk->msg[i] );
			wk->msg[i] = 0;
		}
	}
	
	// time icon
	TdSave_exit_timeicon( wk );
	
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
	BOOL	bflg;
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
				//--------
				// Que待ち
				//--------
				bflg = PRINTSYS_QUE_IsFinished( wk->print_que );
				if(bflg == TRUE){
					OS_Printf("que end(t_download)\n");
					(*seq) ++;
				}
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
	
	// タイムアイコン
	TdSave_main_timeicon( wk );
	
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
	
	// 初期化
	// それほど複雑ではないので、
	// 上、下画面をまとめて処理
	wk->Step 		= STEP_INIT;
	wk->Mode		= wk->Param->status;	// 分岐
	wk->Flag		= 0;
	wk->CmnFlag		= 0;
	wk->ScrollOffy  = 0;
	wk->DispMaxNum	= 0;
	wk->PFlag		= 0;
	wk->SaveFlag	= 0;
	wk->SaveSrc		= 0;
	wk->SaveDest	= 0;
	wk->JoinCurPos	= 0;
	wk->SaveCurPos	= 0;
	//
	for(i=0;i<3;i++){
		wk->AllCurPos[i]		= 0;
		wk->ListCurPos[i]		= 0;
		wk->ListScrollPos[i] 	= 0;
	}
	for(i=0;i<3;i++){
		wk->ItemNum[i] 			= 0;
		wk->PageMaxNum[i] 		= 0;
		wk->CurrentPageNum[i] 	= 0;
	}
	wk->PCount		= 0;
	wk->InfoCurPos	= 0;
	wk->RetMode		= -1;
	wk->NextMode	= 0;
	//
	for(i=0;i<T_DOWNLOAD_CANVAS_MAXNUM;i++){
		wk->PrintUtil[i].win = 0;
	}
	wk->frame_list_work = 0;
	//
	wk->CurrentWbt = 0;
	for(i=0;i<3;i++){
		wk->WbtSaveData[i] = 0;
	}
	// empty用バッファ取得(必ず破棄すること)
	wk->EmptyBuffer = GFL_ARC_LoadDataAlloc( ARCID_T_DOWNLOAD_GRA, NARC_t_download_gra_t_download_pos_tblbin, wk->heapID );
	// testバッファ
	wk->TestBuffer = 0;
	// バッファ
	for(i=0;i<WBTDL_TOTAL_NUM;i++){
		wk->CheckBuffer[i] = 0;
	}
	
	// セーブデータの読み込み(全モード必ず)
	wk->SaveMaxNum = TdSave_init_data( wk );
#ifdef __DOWNLOAD_CHECK__	// debug用
	wk->ListMaxNum = 0;
	// 選択画面からの場合だけデバッグを発動(暫定処理)
	if(wk->Param->status == T_DOWNLOAD_MODE_SELECT){
		wk->TestBuffer = GFL_ARC_LoadDataAlloc( ARCID_WBT_DLDATA, NARC_wbt_dldata_test_all_crc_bin, wk->heapID );
		
		// 12ブロック分上書き
		{
			WBTDL_DATA* wbtdl;
			WBTDL_DATA *adr = &wk->Param->data->data[0];
			wbtdl = (WBTDL_DATA*)wk->TestBuffer;
			for(i=0;i<WBTDL_TOTAL_NUM;i++){
				wk->CheckBuffer[i] = GFL_HEAP_AllocClearMemoryLo( wk->heapID, sizeof(WBTDL_DATA) );
				// wbtdl => check
				GFL_STD_MemCopy32( wbtdl, wk->CheckBuffer[i], sizeof(WBTDL_DATA) );
				wbtdl++;
			}
		}
		wk->ListMaxNum = WBTDL_TOTAL_NUM;
	}
#endif
	// リストデータのチェック
	// =>選択モードの場合のみ
#ifndef __DOWNLOAD_CHECK__
	wk->ListMaxNum = 0;
	if(wk->Mode == T_DOWNLOAD_MODE_SELECT){
		wk->ListMaxNum = TdSave_init_list_data( wk );
	}
#endif
	//--------------------------------
	// データ(bg+string+obj)の初期化
	// (yes/no)winも含む
	//--------------------------------
	TdCmn_init_data( wk );
	//
	//--------------
	// meswin
	//--------------
	// 汎用PrintStream用
	APP_PRINTSYS_COMMON_PrintStreamInit( &wk->PrintSysWork, APP_PRINTSYS_COMMON_TYPE_BOTH );
	// メッセージ送りマーク用
	wk->MsgCursorWork = APP_KEYCURSOR_Create( 15, TRUE, TRUE, wk->heapID );
	
	// 初期化まとめ
	td_sub_init( wk );
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
	//
	// debug
	td_debug( wk );
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
	s32	i;
	
	//-----------------
	// 終了パラメータ
	//------------------
	// ×で終了 => フリームーブとなる
	if(wk->CmnFlag & T_DOWNLOAD_FLG_CROSS_END){
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
	//
	//-------------------
	// カーソル位置(参加)
	//-------------------
	if(wk->Flag & FLG_MW_JOIN_END){
		if(wk->Param->curpos != NULL){
			*wk->Param->curpos = T_DOWNLOAD_CURPOS_OFFSET + wk->JoinCurPos;
		}
	}
	else{
		if(wk->Param->curpos != NULL){
			*wk->Param->curpos = 0;
		}
	}
	// 結果
	if(wk->Param->curpos != NULL){
		OS_Printf("curpos : %d\n",*wk->Param->curpos);
	}else{
		OS_Printf("curpos : null(debug)\n");
	}
	
	// リスト破棄
	TdCmn_exit_framelist( wk );
	
	// 上、下画面をまとめて処理
	// バッファ
	for(i=0;i<WBTDL_TOTAL_NUM;i++){
		if(wk->CheckBuffer[i]){
			GFL_HEAP_FreeMemory( wk->CheckBuffer[i] );
			wk->CheckBuffer[i] = 0;
		}
	}
	// emptyのバッファクリア
	GFL_HEAP_FreeMemory( wk->EmptyBuffer );
	// debugのバッファクリア
	if(wk->TestBuffer){
		GFL_HEAP_FreeMemory( wk->TestBuffer );
	}
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
	BOOL	bflg;
	
	// Queの処理
	PRINTSYS_QUE_Main( wk->print_que );
	//
	for(i=0;i<T_DOWNLOAD_CANVAS_MAXNUM;i++){
		if(wk->PrintUtil[i].win){
			bflg = PRINT_UTIL_Trans( &wk->PrintUtil[i], wk->print_que );
			// 画面の切り替え
			if(wk->PFlag & (1<<i)){
				// キャラの転送終了後スクリーン作成して転送
				GFL_BMPWIN_MakeScreen( wk->PrintUtil[i].win );
				GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->PrintUtil[i].win) );
				wk->PFlag &= ~(1<<i);
			}
		}
	}
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
	s32		i;
	
	// オフセット戻し
	GFL_BG_SetScroll( GFL_BG_FRAME2_M, GFL_BG_SCROLL_Y_SET, 0 );
	
	// ここまでで表示数(DispMaxNum)を決めておくこと
	wk->DispMaxNum = wk->SaveMaxNum;
	//
	// 本来こないはず(念のため)
	wk->Flag &= ~FLG_MW_NOITEM;
	if(wk->DispMaxNum == 0){
		wk->Flag |= FLG_MW_NOITEM;
	}
	// パネル表示
	for(i=0;i<wk->DispMaxNum;i++){
		TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_PANEL_00+i, TRUE );
	}
	// バー非表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_BAR, FALSE );
	// 左上タイトル表示
	TdCmn_disp_title_info( wk, wk->Mode, 0);
	//
	if(wk->Flag & FLG_MW_NOITEM){
		// カーソル非表示
		TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, FALSE );
	}else{
		// カーソル表示
		TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, TRUE );
		// カーソル移動
		TdCmn_disp_panel_cursor( wk, T_DOWNLOAD_PANEL_3, wk->JoinCurPos );
		// 通常アニメ
		TdCmn_set_panel_cursor( wk, 0 );
	}
	// ×表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_CMN, T_DOWNLOAD_CLWKID_CMN_CROSS,  FALSE );
	// 左非表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_CMN, T_DOWNLOAD_CLWKID_CMN_LEFT,  FALSE );
	// 右非表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_CMN, T_DOWNLOAD_CLWKID_CMN_RIGHT, FALSE );
	//
	// 文字列初期化
	TdCmn_init_string2( wk, wk->Mode );
	for(i=0;i<wk->DispMaxNum;i++){
		TdCmn_disp_panel_string( wk, T_DOWNLOAD_DATA_SAVE, i );
	}
	// 項目なしの場合
	if(wk->Flag & FLG_MW_NOITEM){
		wk->Step = STEP_LV5;
	}else{
		//
		// 一度も表示してないときのみ、
		// 「選んでください」を表示
		if(!(wk->Flag & FLG_MW_DISP_JOIN_1ST)){
			// 判定の後に表示(stream)
			TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_01, T_DOWNLOAD_WIN_JOIN_00, 1 );
			// 輝度変更
			TdCmn_set_passive( wk, TRUE );
			// アニメ再開
			TdCmn_stop_anim_speed( wk, TRUE );
			//
			wk->Step = STEP_INIT+10;
			wk->Flag |= FLG_MW_DISP_JOIN_1ST;
		}
		// 判定
		TdCmn_chk_up_disp( wk, FALSE, FALSE, FALSE );
		//
		// 上画面表示
		TdCmn_disp_upmes( wk, T_DOWNLOAD_DATA_SAVE, wk->JoinCurPos );
	}
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
	BOOL	bflg;
	//
	// 処理中は更新しない
	if( PRINTSYS_QUE_IsFinished( wk->print_que ) == FALSE ){
		return;
	}
	// 判定
	// タッチペン
	bflg = td_chk_tp_input_join( wk );
	if(bflg == FALSE){
		// キー
		td_chk_key_input_join( wk );
	}
	//
	// 更新
	if(wk->Flag & FLG_MW_JOIN_UPDATE){
		// カーソル
		TdCmn_disp_panel_cursor( wk, T_DOWNLOAD_PANEL_3, wk->JoinCurPos );
		// 上画面表示
		TdCmn_disp_upmes( wk, T_DOWNLOAD_DATA_SAVE, wk->JoinCurPos );
		//
		wk->Flag &= ~FLG_MW_JOIN_UPDATE;
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
	s16		num;
	s16		flg=0;
	s16		sflg=0;
	s16		mflg=0;
	BOOL	bflg=FALSE;
	s32		ret = TdCmn_chk_tp( wk, 0, wk->Mode );
	
	// パネル
	switch(ret){
		case T_DOWNLOAD_AREA_JOIN_TP_PANEL_00:
		case T_DOWNLOAD_AREA_JOIN_TP_PANEL_01:
		case T_DOWNLOAD_AREA_JOIN_TP_PANEL_02:
			// 範囲内にあるか
			num = ret - T_DOWNLOAD_AREA_JOIN_TP_PANEL_00;
			if(num <= (wk->DispMaxNum-1)){
				mflg = 1;
				wk->JoinCurPos = num;
			}
			break;
	}
	// 判定
	if(mflg == 0){
		// RETURN
		if(ret == T_DOWNLOAD_AREA_JOIN_TP_RETURN){
			TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RETURN, 0 );
			flg = 2;
		}
	}
	// 決定
	if(mflg == 1){
		// 上画面処理
//		TdCmn_chk_up_disp( wk, FALSE, TRUE, TRUE );
		//
		wk->Flag |= FLG_MW_JOIN_UPDATE;
		// 決定アニメ
		TdCmn_set_panel_cursor( wk, 1 );
//		wk->Step = STEP_LV2;
		wk->Step = STEP_LV1+30;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_DECIDE );
		GFL_UI_SetTouchOrKey( GFL_APP_KTST_TOUCH );
	}
	//
	// 更新
	if(flg != 0){
		// 上画面処理
		TdCmn_chk_up_disp( wk, FALSE, TRUE, FALSE );
		//
		bflg = TRUE;
		wk->Step = STEP_LV1+10;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_CANCEL );
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
	s16		sflg=0;
	s16		mflg=0;
	BOOL	bflg=FALSE;
	
	// タッチモード => キーモードにしてskip
	if( GFL_UI_CheckTouchOrKey() == GFL_APP_KTST_TOUCH ){
		// 入力待ち
		if(GFL_UI_KEY_GetTrg() != 0){
			// 上画面処理
			TdCmn_chk_up_disp( wk, TRUE, FALSE, FALSE );
			// SE
			PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
		}
		mflg = 1;
	}
	
	if(mflg == 0){
		// 上
		if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP ){
			if(wk->JoinCurPos != 0){
				// SE
				PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
				//
				wk->JoinCurPos--;
				if(wk->JoinCurPos <= 0){
					wk->JoinCurPos = 0;
				}
				sflg = 1;
			}
		}
		// 下
		else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
			if(wk->JoinCurPos != (wk->DispMaxNum-1)){
				// SE
				PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
				//
				wk->JoinCurPos++;
				if(wk->JoinCurPos >= (wk->DispMaxNum-1)){
					wk->JoinCurPos = (wk->DispMaxNum-1);
				}
				sflg = 1;
			}
		}
		// 決定
		else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE ){
			// 決定アニメ
			TdCmn_set_panel_cursor( wk, 1 );
//			wk->Step = STEP_LV2;
			wk->Step = STEP_LV1+30;
			// SE
			PMSND_PlaySE( SND_T_DOWNLOAD_DECIDE );
			// 上画面処理
//			TdCmn_chk_up_disp( wk, TRUE, FALSE, TRUE );
			GFL_UI_SetTouchOrKey( GFL_APP_KTST_KEY );
		}
		// 戻る
		else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
			TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RETURN, 0 );
			flg = 1;
			//
			// 上画面処理
			TdCmn_chk_up_disp( wk, TRUE, FALSE, FALSE );
		}
	}
	//
	// 更新
	if(sflg == 1){
		wk->Flag |= FLG_MW_JOIN_UPDATE;
		//
		// 上画面処理
		TdCmn_chk_up_disp( wk, TRUE, FALSE, FALSE );
	}
	//
	// 更新
	if(flg == 1){
		bflg = TRUE;
		wk->Step = STEP_LV1+10;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_CANCEL );
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
		// 上画面処理
		TdCmn_chk_up_disp( wk, TRUE, FALSE, FALSE );
	}
	else if( GFL_UI_TP_GetTrg() == TRUE ){
		bflg = TRUE;
		// 上画面処理
		TdCmn_chk_up_disp( wk, FALSE, TRUE, FALSE );
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
	s32		i;
	s32		tmp;
	u32		ret;
	BOOL	bflg;
	
	// 処理
	switch(wk->Step)
	{
		// 初期化
		case STEP_INIT:
			wk->Step = STEP_LV1;
//			break;
		//
		// メッセージ表示待ち(stream)
		case STEP_INIT+10:
			bflg = TdCmn_main_meswin( wk );
			if(bflg == FALSE){
				// 終了処理
				TdCmn_clear_frame( wk, 1 );
				// 輝度戻し
				TdCmn_set_passive( wk, FALSE );
				// アニメ再開
				TdCmn_stop_anim_speed( wk, FALSE );
				// 左上タイトル表示
				TdCmn_disp_title_info( wk, wk->Mode, 0);
				wk->Step = STEP_LV1;
				//
				TdCmn_chk_up_disp( wk, FALSE, FALSE, FALSE );
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
			bflg = TdCmn_chk_anim( wk, 0 );
			if(bflg == TRUE){
				wk->Step = STEP_END;
			}
			break;
		//
		// 左右アニメ待ち
		case STEP_LV1+20:
			// main
			bflg = TdCmn_chk_anim( wk, 1 );
			if(bflg == TRUE){
				wk->CmnFlag |= T_DOWNLOAD_FLG_UPDATE;
				wk->Step = STEP_LV1;
			}
			break;
		// 待ち
		case STEP_LV1+30:
			// 終了してから、上画面を切り替える
			// =>FakeNetModeの時に問題がでてしまうため
			if( PRINTSYS_QUE_IsFinished( wk->print_que ) == TRUE ){
				// 決定時は非表示からタッチの場合がある
				// =>表示タイミングをずらしている
				// 上画面処理
				TdCmn_chk_up_disp( wk, FALSE, FALSE, TRUE );
				wk->Step = STEP_LV2;
			}
			break;
		//
		// 決定
		case STEP_LV2:
			bflg = TdCmn_is_anime_end( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR );
			if(bflg == FALSE){
				// ウインドウ
				TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_00, T_DOWNLOAD_WIN_JOIN_01, 0 );
				// 3択ウインドウ(参加する)
				TdCmn_init_yesnowin( wk, T_DOWNLOAD_YESNO_JOIN );
				// 輝度変更
				TdCmn_set_passive( wk, TRUE );
				// アニメ停止
				TdCmn_stop_anim_speed( wk, TRUE );
				//
				wk->Step = STEP_LV2+10;
			}
			break;
		//
		case STEP_LV2+10:
			// 3択ウインドウの選択処理
			tmp = TdCmn_chk_yesnowin( wk );
			if(tmp != -1){
				// 終了処理
				TdCmn_exit_yesnowin( wk );
				// 輝度戻し
				TdCmn_set_passive( wk, FALSE );
				// アニメ再開
				TdCmn_stop_anim_speed( wk, FALSE );
				// 参加する
				if(tmp == 0){
					TdCmn_clear_frame( wk, 1 );
					wk->Step = STEP_END;
					wk->Flag |= FLG_MW_JOIN_END;
				}
				else if(tmp == 1){
					// 詳細へ移行
					wk->Step = STEP_LV3;
					// カーソル表示(ちらつくので消しておく)
					TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, FALSE );
					for(i=0;i<3;i++){
						// パネル表示
						TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_PANEL_00+i, FALSE );
					}
				}else{
					TdCmn_clear_frame( wk, 1 );
					wk->Step = STEP_LV1;
					// 左上タイトル表示
					TdCmn_disp_title_info( wk, wk->Mode, 0);
					// 通常アニメ
					TdCmn_set_panel_cursor( wk, 0 );
					// 上画面処理
					TdCmn_chk_up_disp( wk, FALSE, FALSE, FALSE );
				}
			}
			break;
		//
		// 詳細画面へ移行
		case STEP_LV3:
			TdCmn_clear_frame( wk, 0 );
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
		//
		// 項目なしの場合(念のため)
		case STEP_LV5:
			bflg = td_chk_wait_mes_join( wk );
			if(bflg == TRUE){
				TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RETURN, 0 );
				wk->Step = STEP_LV1+10;
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
	s32		i;
	
	for(i=0;i<3;i++){
		// パネル表示
		TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_PANEL_00+i, FALSE );
	}
	// 文字列破棄
	TdCmn_exit_string2( wk, wk->Mode );
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
	GFL_BG_SetScroll( GFL_BG_FRAME2_M, GFL_BG_SCROLL_Y_SET, 0 );
	
	// アイコン表示
	for(i=0;i<3;i++){
		TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_ICON_00+i, TRUE );
	}
	// 上画面処理(必ず表示のはず)
	TdCmn_chk_up_disp( wk, FALSE, FALSE, TRUE );
	// カーソル非表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, FALSE );
	
	// 左右矢印非表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_CMN, T_DOWNLOAD_CLWKID_CMN_LEFT,  FALSE );
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_CMN, T_DOWNLOAD_CLWKID_CMN_RIGHT, FALSE );
	// ×非表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_CMN, T_DOWNLOAD_CLWKID_CMN_CROSS,  FALSE );
	
	// BGの表示
	TdCmn_init_sub_bg( wk, wk->Mode );
	//
	// パレットチェンジon
	TdCmn_set_chgpal( wk, TRUE );
	//
	// 文字列初期化
	TdCmn_init_string2( wk, wk->Mode );
	//
	// 初期カーソル位置
	wk->InfoCurPos = 0;
	wk->Flag |= FLG_MW_INFO_UPDATE;
	
	// リスト画面から来た
	if(wk->RetMode == T_DOWNLOAD_MODE_SELECT){
		// パラメータ初期化
		TdCmn_init_info_mode( wk, T_DOWNLOAD_DATA_LIST, wk->AllCurPos[T_DOWNLOAD_LIST_SELECT] );
		// 情報表示
		TdCmn_disp_info_mode( wk, T_DOWNLOAD_DATA_LIST, wk->AllCurPos[T_DOWNLOAD_LIST_SELECT], wk->InfoCurPos );
	}
	// 参加から来た
	else if(wk->RetMode == T_DOWNLOAD_MODE_JOIN){
		// パラメータ初期化
		TdCmn_init_info_mode( wk, T_DOWNLOAD_DATA_SAVE, wk->JoinCurPos );
		// 情報表示
		TdCmn_disp_info_mode( wk, T_DOWNLOAD_DATA_SAVE, wk->JoinCurPos, wk->InfoCurPos );
	}
	// セーブから来た
	else{
		// パラメータ初期化
		TdCmn_init_info_mode( wk, T_DOWNLOAD_DATA_SAVE, wk->SaveCurPos );
		// 情報表示
		TdCmn_disp_info_mode( wk, T_DOWNLOAD_DATA_SAVE, wk->SaveCurPos, wk->InfoCurPos );
	}
	// 左上タイトル表示
	TdCmn_disp_title_info( wk, wk->Mode, 0 );
	// マーク判定
	TdCmn_chk_mark_anim( wk, 1, 0, wk->PageMaxNum[wk->InfoCurPos]-1, wk->CurrentPageNum[wk->InfoCurPos] );
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
	
	// 処理中は更新しない
	if( PRINTSYS_QUE_IsFinished( wk->print_que ) == FALSE ){
		return;
	}
	
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
	if(wk->Flag & FLG_MW_INFO_UPDATE){
		if(wk->RetMode ==  T_DOWNLOAD_MODE_SELECT){
			// 情報表示
			TdCmn_disp_info_mode( wk, T_DOWNLOAD_DATA_LIST, wk->AllCurPos[T_DOWNLOAD_LIST_SELECT], wk->InfoCurPos );
		}
		else if(wk->RetMode == T_DOWNLOAD_MODE_JOIN){
			// 情報表示
			TdCmn_disp_info_mode( wk, T_DOWNLOAD_DATA_SAVE, wk->JoinCurPos, wk->InfoCurPos );
		}
		else{
			// 情報表示
			TdCmn_disp_info_mode( wk, T_DOWNLOAD_DATA_SAVE, wk->SaveCurPos, wk->InfoCurPos );
		}
		// カーソル位置
		TdCmn_chg_info_curpos( wk, wk->InfoCurPos );
		// スクロールバー表示チェック
		TdCmn_chk_info_disp_scrollbar( wk, wk->InfoCurPos );
		// 最小チェック
		wk->Flag &= ~FLG_MW_PAGE_MIN;
		if(wk->PageMaxNum[wk->InfoCurPos] == 1){
			wk->Flag |= FLG_MW_PAGE_MIN;
		}
		wk->Flag &= ~FLG_MW_INFO_UPDATE;
	}
	// タイトル更新
	if(wk->Flag & FLG_MW_TITLE_UPDATE){
		TdCmn_disp_title_info( wk, wk->Mode, wk->InfoCurPos );
		wk->Flag &= ~FLG_MW_TITLE_UPDATE;
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
	s32		flg=0;
	s32		sflg=0;
	s32		num;
	BOOL	bflg=FALSE;
	BOOL	bbflg;
	s32		oldpos = wk->CurrentPageNum[wk->InfoCurPos];
	s32		ret = TdCmn_chk_tp( wk, 0, wk->Mode );		// trg
	s32		cret = TdCmn_chk_tp( wk, 1, wk->Mode );		// cont
	//
	// RETURN
	if(ret == T_DOWNLOAD_AREA_TD_TP_RETURN){
		TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RETURN, 0 );
		flg = 1;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_CANCEL );
	}
	// UP
	else if(ret == T_DOWNLOAD_AREA_TD_TP_UP){
		// ページ最小の時 => タッチなし
		if(!(wk->Flag & FLG_MW_PAGE_MIN)){
			if(wk->CurrentPageNum[wk->InfoCurPos] != 0){
				wk->CurrentPageNum[wk->InfoCurPos]--;
				if(wk->CurrentPageNum[wk->InfoCurPos] <= 0){
					wk->CurrentPageNum[wk->InfoCurPos] = 0;
				}
				TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_UP, 0 );
				flg = 2;
				sflg = 1;
				// SE
				PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
			}
		}
	}
	// DOWN
	else if(ret == T_DOWNLOAD_AREA_TD_TP_DOWN){
		// ページ最小の時 => タッチなし
		if(!(wk->Flag & FLG_MW_PAGE_MIN)){
			if(wk->CurrentPageNum[wk->InfoCurPos] != (wk->PageMaxNum[wk->InfoCurPos]-1)){
				wk->CurrentPageNum[wk->InfoCurPos]++;
				if(wk->CurrentPageNum[wk->InfoCurPos] >= (wk->PageMaxNum[wk->InfoCurPos]-1)){
					wk->CurrentPageNum[wk->InfoCurPos] = (wk->PageMaxNum[wk->InfoCurPos]-1);
				}
				TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_DOWN, 0 );
				flg = 3;
				sflg = 1;
				// SE
				PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
			}
		}
	}
	// ポケモン
	else if(ret == T_DOWNLOAD_AREA_TD_TP_ICON_00){
		wk->InfoCurPos = 0;
		sflg = 1;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
	}
	// 道具
	else if(ret == T_DOWNLOAD_AREA_TD_TP_ICON_01){
		wk->InfoCurPos = 1;
		sflg = 1;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
	}
	// 技
	else if(ret == T_DOWNLOAD_AREA_TD_TP_ICON_02){
		wk->InfoCurPos = 2;
		sflg = 1;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
	}
	//
	else{
		// スクロールバー
		if(cret == T_DOWNLOAD_AREA_TD_TP_SCROLL){
			// 1ページよりある時のみ対応
			if(wk->PageMaxNum[wk->InfoCurPos] > 1){
				sflg = 2;
			}
		}
	}
	// カーソル切り替え
	if(sflg == 1){
		// スクロールバー更新
		TdCmn_set_scrollbar_info( wk, wk->InfoCurPos, wk->CurrentPageNum[wk->InfoCurPos], 0 );
		wk->Flag |= FLG_MW_INFO_UPDATE;
		wk->Flag |= FLG_MW_TITLE_UPDATE;
		// マーク判定
		TdCmn_chk_mark_anim( wk, 1, 0, wk->PageMaxNum[wk->InfoCurPos]-1, wk->CurrentPageNum[wk->InfoCurPos] );
	}
	else if(sflg == 2){
		// ドラッグ中はフリーの座標に表示
		bbflg = TdCmn_set_scrollbar_info( wk, wk->InfoCurPos, 0, 1 );
		// カーソル位置
		num = TdCmn_calc_scrollbar_info( wk, wk->InfoCurPos );
		if(num != -1){
			// 更新があったときのみ
			if(num != oldpos){
				// スクロールバー位置更新
				wk->CurrentPageNum[wk->InfoCurPos] = num;
				wk->Flag |= FLG_MW_INFO_UPDATE;
				// マーク判定
				TdCmn_chk_mark_anim( wk, 1, 0, wk->PageMaxNum[wk->InfoCurPos]-1, wk->CurrentPageNum[wk->InfoCurPos] );
				// 切り替え時のみらしい
				PMSND_PlaySE( SND_T_DOWNLOAD_BAR );
			}
		}
	}
	//
	// 更新
	if(flg != 0){
		bflg = TRUE;
		if(flg == 1){
			wk->Step = STEP_LV1+10;
			// 上画面処理
			// ちらつくのでここでは判定のみ
			GFL_UI_SetTouchOrKey( GFL_APP_KTST_TOUCH );
		}else if(flg == 2){		// up
			wk->Step = STEP_LV1+20;
		}else if(flg == 3){		// down
			wk->Step = STEP_LV1+30;
		}
	}
	
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
	s32		dflg=0;
	BOOL	bflg=FALSE;
	
#ifdef __CHK_TD__
	// debug
	if(wk->DebugFlag & FLG_DBG_ON){
		dflg = 1;
	}
#endif
	
	// 戻る
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
		TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RETURN, 0 );
		flg = 1;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_CANCEL );
	}
	// 左
	else if(( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT )||(dflg == 1)){
		wk->InfoCurPos--;
		if(wk->InfoCurPos < 0){
			wk->InfoCurPos = T_DOWNLOAD_INFO_MAX-1;
		}
		sflg = 1;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
	}
	// 右
	else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT ){
		wk->InfoCurPos++;
		if(wk->InfoCurPos >= T_DOWNLOAD_INFO_MAX){
			wk->InfoCurPos = 0;
		}
		sflg = 1;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
	}
	// 上
	else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP ){
		// ページ最小
		if(!(wk->Flag & FLG_MW_PAGE_MIN)){
			if(wk->CurrentPageNum[wk->InfoCurPos] != 0){
				wk->CurrentPageNum[wk->InfoCurPos]--;
				if(wk->CurrentPageNum[wk->InfoCurPos] <= 0){
					wk->CurrentPageNum[wk->InfoCurPos] = 0;
				}
				TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_UP, 0 );
				flg = 2;
				sflg = 2;
				// SE
				PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
			}
		}
	}
	// 下
	else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
		// ページ最小
		if(!(wk->Flag & FLG_MW_PAGE_MIN)){
			if(wk->CurrentPageNum[wk->InfoCurPos] != (wk->PageMaxNum[wk->InfoCurPos]-1)){
				wk->CurrentPageNum[wk->InfoCurPos]++;
				if(wk->CurrentPageNum[wk->InfoCurPos] >= (wk->PageMaxNum[wk->InfoCurPos]-1)){
					wk->CurrentPageNum[wk->InfoCurPos] = (wk->PageMaxNum[wk->InfoCurPos]-1);
				}
				TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_DOWN, 0 );
				flg = 3;
				sflg = 2;
				// SE
				PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
			}
		}
	}
	// R(最下段)
	else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_R ){
		// ページ最小
		if(!(wk->Flag & FLG_MW_PAGE_MIN)){
			wk->CurrentPageNum[wk->InfoCurPos] = (wk->PageMaxNum[wk->InfoCurPos]-1);
			sflg = 3;
			// SE
			PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
		}
	}
	// L(最上段)
	else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_L ){
		// ページ最小
		if(!(wk->Flag & FLG_MW_PAGE_MIN)){
			wk->CurrentPageNum[wk->InfoCurPos] = 0;
			sflg = 3;
			// SE
			PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
		}
	}
	//
	// カーソル切り替え
	if(sflg != 0){
		// スクロールバー更新
		TdCmn_set_scrollbar_info( wk, wk->InfoCurPos, wk->CurrentPageNum[wk->InfoCurPos], 0 );
		wk->Flag |= FLG_MW_INFO_UPDATE;
		//
		if(sflg == 1){
			wk->Flag |= FLG_MW_TITLE_UPDATE;
		}
		// マーク判定
		if((sflg == 1)||(sflg == 3)){
			TdCmn_chk_mark_anim( wk, 1, 0, wk->PageMaxNum[wk->InfoCurPos]-1, wk->CurrentPageNum[wk->InfoCurPos] );
		}
	}
	// 更新
	if(flg != 0){
		bflg = TRUE;
		if(flg == 1){
			wk->Step = STEP_LV1+10;
			// 上画面処理
			// カーソルがちらつくので最後で処理
			GFL_UI_SetTouchOrKey( GFL_APP_KTST_KEY );
		}else if(flg == 2){			// up
			wk->Step = STEP_LV1+20;
		}else if(flg == 3){			// down
			wk->Step = STEP_LV1+30;
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
	s32		i;
	s32		tmp;
	u32		ret;
	BOOL	bflg;
	
	// 処理
	switch(wk->Step)
	{
		// 初期化
		case STEP_INIT:
			wk->Step = STEP_LV1;
//			break;
		//
		// 本体
		case STEP_LV1:
			td_chk_input_info( wk );
			break;
		//
		// アニメ待ち
		case STEP_LV1+10:
			bflg = TdCmn_chk_anim( wk, 0 );
			if(bflg == TRUE){
				// 戻り先チェック
				if(wk->RetMode == -1){
					wk->Step = STEP_END;
				}else{
					wk->Step = STEP_LV3;
					// obj非表示
					// アイコン非表示
					for(i=0;i<3;i++){
						TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_ICON_00+i, FALSE );
					}
					// 上下矢印非表示
					TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_CMN, T_DOWNLOAD_CLWKID_CMN_UP,   FALSE );
					TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_CMN, T_DOWNLOAD_CLWKID_CMN_DOWN, FALSE );
				}
			}
			break;
		//
		// 上アニメ待ち
		case STEP_LV1+20:
			bflg = TdCmn_chk_anim( wk, 1 );
			if(bflg == TRUE){
				wk->Step = STEP_LV1;
				// マーク判定
				TdCmn_chk_mark_anim( wk, 1, 0, wk->PageMaxNum[wk->InfoCurPos]-1, wk->CurrentPageNum[wk->InfoCurPos] );
			}
			break;
		//
		// 下アニメ待ち
		case STEP_LV1+30:
			bflg = TdCmn_chk_anim( wk, 1 );
			if(bflg == TRUE){
				wk->Step = STEP_LV1;
				// マーク判定
				TdCmn_chk_mark_anim( wk, 1, 0, wk->PageMaxNum[wk->InfoCurPos]-1, wk->CurrentPageNum[wk->InfoCurPos] );
			}
			break;
		//
		// 終了戻り
		case STEP_LV3:
			// 上画面処理
			TdCmn_chk_up_disp( wk, FALSE, FALSE, FALSE );
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
	//
	// アイコン非表示
	for(i=0;i<3;i++){
		TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_ICON_00+i, FALSE );
	}
	// 文字列破棄
	TdCmn_exit_string2( wk, wk->Mode );
	//
	// BG面クリア
	GFL_BG_ClearFrame( GFL_BG_FRAME2_M );
	//
	// 左右矢印非表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_CMN, T_DOWNLOAD_CLWKID_CMN_LEFT,  TRUE );
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_CMN, T_DOWNLOAD_CLWKID_CMN_RIGHT, TRUE );
	// 上下矢印非表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_CMN, T_DOWNLOAD_CLWKID_CMN_UP,   FALSE );
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_CMN, T_DOWNLOAD_CLWKID_CMN_DOWN, FALSE );
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
	wk->DispMaxNum = wk->ListMaxNum;
	//
	// selectの情報をセット
	wk->AllCurPos[T_DOWNLOAD_LIST_CURRENT] 		= wk->AllCurPos[T_DOWNLOAD_LIST_SELECT];
	wk->ListCurPos[T_DOWNLOAD_LIST_CURRENT] 	= wk->ListCurPos[T_DOWNLOAD_LIST_SELECT];
	wk->ListScrollPos[T_DOWNLOAD_LIST_CURRENT] 	= wk->ListScrollPos[T_DOWNLOAD_LIST_SELECT];
	//
	// リスト初期化
	GFL_BG_SetVisible( GFL_BG_FRAME2_M, FALSE );
	wk->Flag &= ~FLG_MW_FRAME_STAT;
	TdCmn_init_framelist( wk, wk->Mode );
	// 初期化状態チェック
	if( FRAMELIST_Init( wk->frame_list_work ) == FALSE ){
		wk->Flag |= FLG_MW_FRAME_STAT;
	}
//	FRAMELIST_SetFrameMax( wk->frame_list_work, 7 );
	// カーソル表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, TRUE );
	// 通常アニメ
	TdCmn_set_panel_cursor( wk, 0 );
	// バー表示
	if(wk->DispMaxNum <= T_DOWNLOAD_DISP_PANEL_MAX){
		TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_BAR, FALSE );
	}else{
		TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_BAR, TRUE );
		// スクロールバーの処理
		TdCmn_main_scrollbar( wk );
	}
	// 左右矢印表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_CMN, T_DOWNLOAD_CLWKID_CMN_LEFT,  TRUE );
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_CMN, T_DOWNLOAD_CLWKID_CMN_RIGHT, TRUE );
	// マーク判定
	TdCmn_chk_mark_anim( wk, 0, 0, wk->DispMaxNum-1, wk->AllCurPos[T_DOWNLOAD_LIST_CURRENT] );
	
	// カーソル移動
	TdCmn_disp_panel_cursor( wk, T_DOWNLOAD_PANEL_FRAME, wk->ListCurPos[T_DOWNLOAD_LIST_CURRENT] );
	// タイトル表示
	TdCmn_disp_title_info( wk, wk->Mode, 0);
	//
	// ×非表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_CMN, T_DOWNLOAD_CLWKID_CMN_CROSS,  FALSE );
	//
	// 上画面処理
	TdCmn_chk_up_disp( wk, FALSE, FALSE, FALSE );
	//
	// 上画面表示
	TdCmn_disp_upmes( wk, T_DOWNLOAD_DATA_LIST, wk->AllCurPos[T_DOWNLOAD_LIST_CURRENT] );
	//
	wk->Step = STEP_INIT+5;
	// 次のとび先指定
	wk->NextMode = STEP_LV1;
	//
	// 3つセーブ済み
	// 一度も表示してない場合のみ、
	//「選択してください」を表示
	if(!(wk->Flag & FLG_MW_DISP_SEL_1ST)){
		// 判定の後に表示(stream)
		TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_01, T_DOWNLOAD_WIN_SEL_DATA, 1 );
		// 輝度変更
		TdCmn_set_passive( wk, TRUE );
		// アニメ停止
		TdCmn_stop_anim_speed( wk, TRUE );
		// 通常
//		wk->Step = STEP_INIT+10;
		wk->NextMode = STEP_INIT+10;
		//
		wk->Flag |= FLG_MW_DISP_SEL_1ST;
	}
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, FALSE );
	// 終了済み
	if(wk->Flag & FLG_MW_FRAME_STAT){
		wk->Step = wk->NextMode;
		GFL_BG_SetVisible( GFL_BG_FRAME2_M, TRUE );
		// カーソル表示と移動
		// キーの場合のみ
		if( GFL_UI_CheckTouchOrKey() == GFL_APP_KTST_KEY ){
			TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, TRUE );
			TdCmn_disp_panel_cursor( wk, T_DOWNLOAD_PANEL_FRAME, wk->ListCurPos[T_DOWNLOAD_LIST_CURRENT] );
		}
	}
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
	
	// 処理中は更新しない
	if( PRINTSYS_QUE_IsFinished( wk->print_que ) == FALSE ){
		return;
	}
	//
	// タッチモード => キーモードにする
	if( GFL_UI_CheckTouchOrKey() == GFL_APP_KTST_TOUCH ){
		// 入力待ち
		if(GFL_UI_KEY_GetTrg() != 0){
			// 上画面処理
			TdCmn_chk_up_disp( wk, FALSE, FALSE, TRUE );
			// SE
			PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
		}
	}
	
	// リスト処理
	ret = TdCmn_main_framelist( wk );
	listflg = TdCmn_is_main_framelist( wk, ret );
	if(listflg != 0){
		flg = 1;
		// 上画面処理
		// => パネルタッチの場合は消さない
		if(listflg != 2){
			TdCmn_chk_up_disp( wk, FALSE, FALSE, FALSE );
		}
	}
	if(listflg == 2){
		// 決定アニメ
		TdCmn_set_panel_cursor( wk, 1 );
		//
//		wk->Step = STEP_LV2;
		wk->Step = STEP_LV1+30;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_DECIDE );
		// 上画面処理
//		TdCmn_chk_up_disp( wk, FALSE, FALSE, TRUE );
		flg = 1;
	}
	// 左右キー
	else if(listflg == 3){
		wk->Step = STEP_LV1+20;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
		flg = 1;
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
		// 上画面表示
		TdCmn_disp_upmes( wk, T_DOWNLOAD_DATA_LIST, wk->AllCurPos[T_DOWNLOAD_LIST_CURRENT] );
		// カーソル移動
		TdCmn_disp_panel_cursor( wk, T_DOWNLOAD_PANEL_FRAME, wk->ListCurPos[T_DOWNLOAD_LIST_CURRENT] );
		// マーク判定
		TdCmn_chk_mark_anim( wk, 0, 0, wk->DispMaxNum-1, wk->AllCurPos[T_DOWNLOAD_LIST_CURRENT] );
		//
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
	s32		flg=0;
	BOOL	bflg=FALSE;
	s32		ret = TdCmn_chk_tp( wk, 0, wk->Mode );
	
	// RETURN
	if(ret == T_DOWNLOAD_AREA_TD_TP_RETURN){
		TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RETURN, 0 );
		flg = 1;
	}
	//
	// 更新
	if(flg != 0){
		bflg = TRUE;
		wk->Step = STEP_LV1+10;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_CANCEL );
		// 上画面処理
		TdCmn_chk_up_disp( wk, FALSE, TRUE, FALSE );
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  メッセージ表示キー入力(select)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval BOOL				(TRUE:更新あり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL td_chk_wait_mes_select( T_DOWNLOAD_MAIN_WORK* wk )
{
	BOOL	bflg=FALSE;
	
	// 入力待ち
	if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_CANCEL|PAD_BUTTON_DECIDE)){
		// 上画面処理
		TdCmn_chk_up_disp( wk, TRUE, FALSE, FALSE );
		bflg = TRUE;
	}
	else if( GFL_UI_TP_GetTrg() == TRUE ){
		// 上画面処理
		TdCmn_chk_up_disp( wk, FALSE, TRUE, FALSE );
		bflg = TRUE;
	}
	//
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
	
	// 戻る
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
		TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RETURN, 0 );
		flg = 1;
	}
	//
	// 更新
	if(flg != 0){
		bflg = TRUE;
		// 上画面処理
		TdCmn_chk_up_disp( wk, TRUE, FALSE, FALSE );
		//
		if(flg == 1){
			wk->Step = STEP_LV1+10;
			// SE
			PMSND_PlaySE( SND_T_DOWNLOAD_CANCEL );
		}else{
			wk->Step = STEP_LV1+20;
			// SE
			PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
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
	s32		flg;
	u32		ret;
	BOOL	bflg;
	
	// 処理
	switch(wk->Step)
	{
		// 初期化
		case STEP_INIT:
			wk->Step = STEP_LV1;
//			break;
		//
		//---------------------------------------
		// フレームの初期化待ち
		// (負荷があると表示が遅れるための処理)
		// (通常時では、ほとんど起こらないはず)
		//----------------------------------------
		case STEP_INIT+5:
			// 初期化状態チェック
			if(!(wk->Flag & FLG_MW_FRAME_STAT)){
				if( FRAMELIST_Init( wk->frame_list_work ) == FALSE ){
					wk->Flag |= FLG_MW_FRAME_STAT;
					wk->Step = wk->NextMode;
					GFL_BG_SetVisible( GFL_BG_FRAME2_M, TRUE );
					// カーソル表示と移動
					// キーの場合のみ
					if( GFL_UI_CheckTouchOrKey() == GFL_APP_KTST_KEY ){
						TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, TRUE );
						TdCmn_disp_panel_cursor( wk, T_DOWNLOAD_PANEL_FRAME, wk->ListCurPos[T_DOWNLOAD_LIST_CURRENT] );
					}
				}
			}else{
				// こないはず(念のため)
				wk->Step = wk->NextMode;
			}
			break;
		
		//------------
		// 開始win待ち(stream)
		//------------
		case STEP_INIT+10:
			bflg = TdCmn_main_meswin( wk );
			if(bflg == FALSE){
				// 終了処理
				TdCmn_clear_frame( wk, 1 );
				// 輝度戻し
				TdCmn_set_passive( wk, FALSE );
				// アニメ再開
				TdCmn_stop_anim_speed( wk, FALSE );
				// 左上タイトル表示
				TdCmn_disp_title_info( wk, wk->Mode, 0);
				wk->Step = STEP_LV1;
				//
				TdCmn_chk_up_disp( wk, FALSE, FALSE, FALSE );
			}
			break;
		//
		//-------
		// 本体
		//-------
		case STEP_LV1:
			td_chk_input_select( wk );
			break;
		//
		//-----------
		// 終了判定(win)
		//-----------
		case STEP_LV1+10:
			bflg = TdCmn_chk_anim( wk, 0 );
			if(bflg == TRUE){
				// ウインドウ(終了しますか)
				TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_00, T_DOWNLOAD_WIN_FINISH, 0 );
				// (yes/no)
				TdCmn_init_yesnowin( wk, T_DOWNLOAD_YESNO_YESNO );
				// 輝度変更
				TdCmn_set_passive( wk, TRUE );
				// アニメ停止
				TdCmn_stop_anim_speed( wk, TRUE );
				wk->Step = STEP_LV1+15;
			}
			break;
		//
		// 終了win待ち
		case STEP_LV1+15:
			// 2択ウインドウの選択処理
			tmp = TdCmn_chk_yesnowin( wk );
			if(tmp != -1){
				// 終了処理
				TdCmn_exit_yesnowin( wk );
				// 輝度戻し
				TdCmn_set_passive( wk, FALSE );
				// アニメ再開
				TdCmn_stop_anim_speed( wk, FALSE );
				// はい=>終了
				if(tmp == 0){
					wk->Step = STEP_END;
				}
				// 戻る
				else{
					TdCmn_clear_frame( wk, 1 );
					wk->Step = STEP_LV1;
					TdCmn_disp_title_info( wk, wk->Mode, 0 );
					// 通常アニメ
					TdCmn_set_panel_cursor( wk, 0 );
					// フラグ戻し
					wk->CmnFlag &= ~(T_DOWNLOAD_FLG_CROSS_END|T_DOWNLOAD_FLG_RET_END);
					// 上画面処理
					TdCmn_chk_up_disp( wk, FALSE, FALSE, FALSE );
				}
			}
			break;
		//
		//----------------
		// 左右アニメ待ち
		//----------------
		case STEP_LV1+20:
			// リスト移動待ち
			ret = TdCmn_main_framelist( wk );
			if(ret == FRAMELIST_RET_NONE){
				bflg = TdCmn_chk_anim( wk, 1 );
				if(bflg == TRUE){
					// マーク判定
					TdCmn_chk_mark_anim( wk, 0, 0, wk->DispMaxNum-1, wk->AllCurPos[T_DOWNLOAD_LIST_CURRENT] );
					//
					wk->Step = STEP_LV1;
				}
			}
			break;
		//
		
		//---------------
		// 決定タッチ処理
		//---------------
		case STEP_LV1+30:
			// 終了してから、上画面を切り替える
			// =>FakeNetModeの時に問題がでてしまうため
			if( PRINTSYS_QUE_IsFinished( wk->print_que ) == TRUE ){
				// 決定時は非表示からタッチの場合がある
				// =>表示タイミングをずらしている
				// 上画面処理
				TdCmn_chk_up_disp( wk, FALSE, FALSE, TRUE );
				wk->Step = STEP_LV2;
			}
			break;
		
		//---------------
		// 決定アニメ待ち
		// (save3択)
		//---------------
		case STEP_LV2:
			bflg = TdCmn_is_anime_end( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR );
			if(bflg == FALSE){
				// ウインドウ
				TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_00, T_DOWNLOAD_WIN_WHAT_00, 0 );
				// 3択ウインドウ(セーブする)
				TdCmn_init_yesnowin( wk, T_DOWNLOAD_YESNO_SAVE );
				// 輝度変更
				TdCmn_set_passive( wk, TRUE );
				// アニメ停止
				TdCmn_stop_anim_speed( wk, TRUE );
				//
				wk->Step = STEP_LV2+10;
			}
			break;
		//
		// 決定ウインドウ待ち(3択)
		case STEP_LV2+10:
			// 3択ウインドウの選択処理
			tmp = TdCmn_chk_yesnowin( wk );
			if(tmp != -1){
				// 終了処理
				TdCmn_exit_yesnowin( wk );
				// セーブ画面
				if(tmp == 0){
					// すでにセーブ済みかどうか
					bflg = TdSave_chk_save_list( wk, wk->AllCurPos[T_DOWNLOAD_LIST_CURRENT] );
					if(bflg == TRUE){
						TdCmn_clear_frame( wk, 1 );
						TdCmn_disp_title_info( wk, wk->Mode, 0 );
						//
						wk->Step = STEP_LV2+30;
					}
					// まだセーブしてない
					else{
						// 空きスロット番号を求める
						flg = TdSave_get_save_pos( wk );
						// 空きがない => 消してね
						if(flg == -1){
							// 輝度戻し
							TdCmn_set_passive( wk, FALSE );
							// アニメ再開
							TdCmn_stop_anim_speed( wk, FALSE );
							wk->Step = STEP_LV4;
							// 上画面処理
							TdCmn_chk_up_disp( wk, FALSE, FALSE, FALSE );
							// カーソル表示(ちらつくので消しておく)
							TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, FALSE );
							TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_BAR, FALSE );
							// カーソル戻し
							wk->SaveCurPos = 0;
						}
						// 空きがあるのですぐセーブ
						else{
							// 輝度戻し=>引き継ぐ
							// アニメ再開
							TdCmn_stop_anim_speed( wk, FALSE );
							//
							wk->Step = STEP_LV4;
							// カーソル表示(ちらつくので消しておく)
							TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, FALSE );
							TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_BAR, FALSE );
							wk->Flag |= FLG_MW_SELECT_SAVE;
						}
					}
				}
				// 詳細画面へ
				else if(tmp == 1){
					// 輝度戻し
					TdCmn_set_passive( wk, FALSE );
					// アニメ再開
					TdCmn_stop_anim_speed( wk, FALSE );
					wk->Step = STEP_LV3;
					// カーソル表示(ちらつくので消しておく)
					TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, FALSE );
				}
				// 戻る
				else{
					// 輝度戻し
					TdCmn_set_passive( wk, FALSE );
					// アニメ再開
					TdCmn_stop_anim_speed( wk, FALSE );
					TdCmn_clear_frame( wk, 1 );
					wk->Step = STEP_LV1;
					TdCmn_disp_title_info( wk, wk->Mode, 0 );
					// 通常アニメ
					TdCmn_set_panel_cursor( wk, 0 );
					// 上画面処理
					TdCmn_chk_up_disp( wk, FALSE, FALSE, FALSE );
				}
			}
			break;
		
		//---------------
		// 決定アニメ待ち
		// (すでにsaveされている)
		//---------------
		case STEP_LV2+30:
			bflg = TdCmn_is_anime_end( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR );
			if(bflg == FALSE){
				// ウインドウ(stream)
				TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_01, T_DOWNLOAD_WIN_SAVED, 1 );
				// 輝度変更
				TdCmn_set_passive( wk, TRUE );
				// アニメ停止
				TdCmn_stop_anim_speed( wk, TRUE );
				wk->Step = STEP_LV2+40;
			}
			break;
		//
		// 決定ウインドウ待ち(stream)
		case STEP_LV2+40:
			bflg = TdCmn_main_meswin( wk );
			if(bflg == FALSE){
				// 終了処理
				TdCmn_clear_frame( wk, 1 );
				// 輝度戻し
				TdCmn_set_passive( wk, FALSE );
				// アニメ再開
				TdCmn_stop_anim_speed( wk, FALSE );
				// 左上タイトル表示
				TdCmn_disp_title_info( wk, wk->Mode, 0);
				wk->Step = STEP_LV1;
				TdCmn_chk_up_disp( wk, FALSE, FALSE, FALSE );
			}
			break;
		//
		//----------------
		// 詳細画面へ移行
		//----------------
		case STEP_LV3:
			TdCmn_clear_frame( wk, 0 );
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
		//----------------------------
		// セーブデータ選択画面へ移行
		//----------------------------
		case STEP_LV4:
			TdCmn_clear_frame( wk, 1 );
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
		//-----
		// END
		//-----
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
	s32		i;
	s32		num;
	
	// オフセット戻し
	GFL_BG_SetScroll( GFL_BG_FRAME2_M, GFL_BG_SCROLL_Y_SET, 0 );
	
	// 表示数セット
	wk->DispMaxNum = wk->SaveMaxNum;
	//
	// アイテムがない
	wk->Flag &= ~FLG_MW_NOITEM;
	if(wk->DispMaxNum == 0){
		wk->Flag |= FLG_MW_NOITEM;
	}
	// バー非表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_BAR, FALSE );
	//
	// ×非表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_CMN, T_DOWNLOAD_CLWKID_CMN_CROSS,  FALSE );
	// 左右矢印非表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_CMN, T_DOWNLOAD_CLWKID_CMN_LEFT,  FALSE );
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_CMN, T_DOWNLOAD_CLWKID_CMN_RIGHT, FALSE );
	//
	// 文字列初期化
	TdCmn_init_string2( wk, wk->Mode );
	for(i=0;i<wk->DispMaxNum;i++){
		TdCmn_disp_panel_string( wk, T_DOWNLOAD_DATA_SAVE, i );
		// パネル表示
		TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_PANEL_00+i, TRUE );
	}
	// 項目がある
	if(!(wk->Flag & FLG_MW_NOITEM)){
		// カーソル表示
		TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, TRUE );
		// カーソル移動
		TdCmn_disp_panel_cursor( wk, T_DOWNLOAD_PANEL_3, wk->SaveCurPos );
		// すぐセーブのときは上画面は更新しない
		// =>選択の状態を表示
		if(!(wk->Flag & FLG_MW_SELECT_SAVE)){
			// 上画面表示
			TdCmn_disp_upmes( wk, T_DOWNLOAD_DATA_SAVE, wk->SaveCurPos );
		}
	}
	// 上画面処理
	TdCmn_chk_up_disp( wk, FALSE, FALSE, FALSE );
	//
	wk->Step = STEP_LV1;
	//
	// 詳細画面からの戻りの場合は、
	// 処理を分岐
	if(!(wk->Flag & FLG_MW_SAVE2INFO)){
		// すぐセーブではない場合
		if(!(wk->Flag & FLG_MW_SELECT_SAVE)){
			// データがいっぱい => 削除が必要
			TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_01, T_DOWNLOAD_WIN_DATA_FULL, 1 );
			// 輝度変更(直接セット)
			TdCmn_set_passive_direct( wk, TRUE );
			// アニメ停止
			TdCmn_stop_anim_speed( wk, TRUE );
			wk->Step = STEP_INIT+10;
		}
		// すぐセーブの場合
		else{
			// ウインドウ
			TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_01, T_DOWNLOAD_WIN_NOW_SAVING, 0 );
			// 輝度変更=>そのまま引き継ぐ
			// アニメ停止
			TdCmn_stop_anim_speed( wk, TRUE );
			wk->Step = STEP_LV4+5;
			// カーソル非表示
			TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, FALSE );
			//----------------
			// セーブ処理
			// (=>メッセージ表示待ち後セーブ)
			//----------------
		}
	}
	// 左上タイトル表示
	// 輝度変更(パレット変え)の影響があるため最後に表示
	TdCmn_disp_title_info( wk, wk->Mode, 0 );
	// フラグクリア
	wk->Flag &= ~FLG_MW_SAVE2INFO;
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
	BOOL	bflg;
	
	// 処理中は更新しない
	if( PRINTSYS_QUE_IsFinished( wk->print_que ) == FALSE ){
		return;
	}
	// 判定
	// タッチペン
	bflg = td_chk_tp_input_save( wk );
	if(bflg == FALSE){
		// キー
		td_chk_key_input_save( wk );
	}
	//
	// 更新
	if(wk->Flag & FLG_MW_SAVE_UPDATE){
		// カーソル
		TdCmn_disp_panel_cursor( wk, T_DOWNLOAD_PANEL_3, wk->SaveCurPos );
		// 上画面表示
		TdCmn_disp_upmes( wk, T_DOWNLOAD_DATA_SAVE, wk->SaveCurPos );
		//
		wk->Flag &= ~FLG_MW_SAVE_UPDATE;
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
	s16		num;
	s16		flg=0;
	s16		sflg=0;
	s16		mflg=0;
	BOOL	bflg=FALSE;
	s32		ret = TdCmn_chk_tp( wk, 0, wk->Mode );
	
	// パネル
	switch(ret){
		case T_DOWNLOAD_AREA_SAVE_TP_PANEL_00:
		case T_DOWNLOAD_AREA_SAVE_TP_PANEL_01:
		case T_DOWNLOAD_AREA_SAVE_TP_PANEL_02:
			// 範囲内にあるか
			num = ret - T_DOWNLOAD_AREA_SAVE_TP_PANEL_00;
			if(num <= (wk->DispMaxNum-1)){
				mflg = 1;
				wk->SaveCurPos = num;
			}
			break;
	}
	if(mflg == 0){
		// RETURN
		if(ret == T_DOWNLOAD_AREA_SAVE_TP_RETURN){
			TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RETURN, 0 );
			flg = 2;
		}
	}
	// 更新
	if(sflg == 1){
		wk->Flag |= FLG_MW_SAVE_UPDATE;
		// 上画面処理
		TdCmn_chk_up_disp( wk, FALSE, TRUE, FALSE );
	}
	// 決定
	if(mflg == 1){
		wk->Flag |= FLG_MW_SAVE_UPDATE;
		// 決定アニメ
		TdCmn_set_panel_cursor( wk, 1 );
//		wk->Step = STEP_LV2;
		wk->Step = STEP_LV1+30;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_DECIDE );
		// 上画面処理
//		TdCmn_chk_up_disp( wk, FALSE, TRUE, TRUE );
		GFL_UI_SetTouchOrKey( GFL_APP_KTST_TOUCH );
	}
	//
	// 更新
	if(flg != 0){
		bflg = TRUE;
		wk->Step = STEP_LV1+10;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_CANCEL );
		// 上画面処理
		TdCmn_chk_up_disp( wk, FALSE, TRUE, FALSE );
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
	s16		sflg=0;
	s16		mflg=0;
	BOOL	bflg=FALSE;
	
	// タッチモード => キーモードにしてskip
	if( GFL_UI_CheckTouchOrKey() == GFL_APP_KTST_TOUCH ){
		// 入力待ち
		if(GFL_UI_KEY_GetTrg() != 0){
			// 上画面処理
			TdCmn_chk_up_disp( wk, TRUE, FALSE, FALSE );
			// SE
			PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
		}
		mflg = 1;
	}
	
	if(mflg == 0){
		// 上
		if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP ){
			if(wk->SaveCurPos != 0){
				// SE
				PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
				//
				wk->SaveCurPos--;
				if(wk->SaveCurPos <= 0){
					wk->SaveCurPos = 0;
				}
				sflg = 1;
			}
		}
		// 下
		else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
			if(wk->SaveCurPos != (wk->DispMaxNum-1)){
				// SE
				PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
				//
				wk->SaveCurPos++;
				if(wk->SaveCurPos >= (wk->DispMaxNum-1)){
					wk->SaveCurPos = (wk->DispMaxNum-1);
				}
				sflg = 1;
			}
		}
		// 決定
		else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE ){
			// 決定アニメ
			TdCmn_set_panel_cursor( wk, 1 );
//			wk->Step = STEP_LV2;
			wk->Step = STEP_LV1+30;
			// SE
			PMSND_PlaySE( SND_T_DOWNLOAD_DECIDE );
			// 上画面処理
//			TdCmn_chk_up_disp( wk, TRUE, FALSE, TRUE );
			GFL_UI_SetTouchOrKey( GFL_APP_KTST_KEY );
		}
		// 戻る
		else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
			TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RETURN, 0 );
			flg = 1;
		}
		//
#ifdef __CHK_TD__
		else{
			if(wk->DebugFlag & FLG_DBG_ON){
				wk->SaveCurPos++;
				if(wk->SaveCurPos > (wk->DispMaxNum-1)){
					wk->SaveCurPos = 0;
				}
				sflg = 1;
			}
		}
#endif
	}
	
	// 更新
	if(sflg == 1){
		wk->Flag |= FLG_MW_SAVE_UPDATE;
		// 上画面処理
		TdCmn_chk_up_disp( wk, TRUE, FALSE, FALSE );
	}
	
	//
	// 更新
	if(flg == 1){
		bflg = TRUE;
		wk->Step = STEP_LV1+10;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_CANCEL );
		// 上画面処理
		TdCmn_chk_up_disp( wk, TRUE, FALSE, FALSE );
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
	//
	// 入力待ち
	if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_CANCEL|PAD_BUTTON_DECIDE)){
		bflg = TRUE;
		// 上画面処理
		TdCmn_chk_up_disp( wk, TRUE, FALSE, FALSE );
	}
	else if( GFL_UI_TP_GetTrg() == TRUE ){
		bflg = TRUE;
		// 上画面処理
		TdCmn_chk_up_disp( wk, FALSE, TRUE, FALSE );
	}
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
	s32		i;
	s32		tmp;
	u32		ret;
	BOOL	bflg;
	
	// 処理
	switch(wk->Step)
	{
		// 初期化
		case STEP_INIT:
			wk->Step = STEP_LV1;
//			break;
		//
		//--------------------
		// データが一杯表示待ち(stream)
		//--------------------
		case STEP_INIT+10:
			bflg = TdCmn_main_meswin( wk );
			if(bflg == FALSE){
				// 終了処理
				TdCmn_clear_frame( wk, 1 );
				// 輝度戻し
				TdCmn_set_passive( wk, FALSE );
				// アニメ再開
				TdCmn_stop_anim_speed( wk, FALSE );
				// 左上タイトル表示
				TdCmn_disp_title_info( wk, wk->Mode, 0);
				wk->Step = STEP_LV1;
				//
				TdCmn_chk_up_disp( wk, FALSE, FALSE, FALSE );
			}
			break;
		//
		//-------
		// 本体
		//-------
		case STEP_LV1:
			td_chk_input_save( wk );
			break;
		//
		//------------
		// 終了win
		//------------
		case STEP_LV1+10:
			bflg = TdCmn_chk_anim( wk, 0 );
			if(bflg == TRUE){
				// セーブ済みの場合=>選択へ戻る
				if(wk->Flag & FLG_MW_SAVED){
					wk->Step = STEP_LV3;
//					wk->Flag &= ~FLG_MW_SAVED;
					// カーソル表示
					TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, FALSE );
					for(i=0;i<3;i++){
						// パネル表示
						TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_PANEL_00+i, FALSE );
					}
				}else{
					// ウインドウ
					// (削除しないで終了しますか)
					TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_00, T_DOWNLOAD_WIN_DEL_STOP, 0 );
					// (yes/no)
					TdCmn_init_yesnowin( wk, T_DOWNLOAD_YESNO_YESNO );
					// 輝度変更
					TdCmn_set_passive( wk, TRUE );
					// アニメ停止
					TdCmn_stop_anim_speed( wk, TRUE );
					//
					wk->Step = STEP_LV1+15;
				}
			}
			break;
		//
		//-----------
		// 終了win待ち
		//------------
		case STEP_LV1+15:
			// 2択ウインドウの選択処理
			tmp = TdCmn_chk_yesnowin( wk );
			if(tmp != -1){
				// 終了処理
				TdCmn_exit_yesnowin( wk );
				// 輝度戻し
				TdCmn_set_passive( wk, FALSE );
				// アニメ停止
				TdCmn_stop_anim_speed( wk, FALSE );
				// はい=>終了
				if(tmp == 0){
					TdCmn_clear_frame( wk, 1 );
					// 戻り先チェック
					if(wk->RetMode == -1){
						wk->Step = STEP_END;
					}else{
						wk->Step = STEP_LV3;
						// カーソル表示
						TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, FALSE );
						for(i=0;i<3;i++){
							// パネル表示
							TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_PANEL_00+i, FALSE );
						}
					}
					// フラグ初期化
					wk->Flag &= ~(FLG_MW_SELECT_SAVE|FLG_MW_SAVED);
				}
				// 戻る
				else{
					TdCmn_clear_frame( wk, 1 );
					wk->Step = STEP_LV1;
					TdCmn_disp_title_info( wk, wk->Mode, 0 );
					// 通常アニメ
					TdCmn_set_panel_cursor( wk, 0 );
					// フラグ戻し
					wk->CmnFlag &= ~(T_DOWNLOAD_FLG_CROSS_END|T_DOWNLOAD_FLG_RET_END);
					// 上画面処理
					TdCmn_chk_up_disp( wk, FALSE, FALSE, FALSE );
				}
			}
			break;
		//
		//-----------------
		// 左右アニメ待ち
		//-----------------
		case STEP_LV1+20:
			// main
			bflg = TdCmn_chk_anim( wk, 1 );
			if(bflg == TRUE){
				// 再計算
				wk->CmnFlag |= T_DOWNLOAD_FLG_UPDATE;
				wk->Step = STEP_LV1;
			}
			break;
		//
		//-----------------
		// 待ち
		//-----------------
		case STEP_LV1+30:
			// 終了してから、上画面を切り替える
			// =>FakeNetModeの時に問題がでてしまうため
			if( PRINTSYS_QUE_IsFinished( wk->print_que ) == TRUE ){
				// 決定時は非表示からタッチの場合がある
				// =>表示タイミングをずらしている
				// 上画面処理
				TdCmn_chk_up_disp( wk, FALSE, FALSE, TRUE );
				wk->Step = STEP_LV2;
			}
			break;
		//
		//-----------------
		// 削除win(第1段階)
		//-----------------
		case STEP_LV2:
			bflg = TdCmn_is_anime_end( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR );
			if(bflg == FALSE){
				// セーブしてない => 削除選択中
				if(!(wk->Flag & FLG_MW_SAVED)){
					// 削除しますか
					TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_00, T_DOWNLOAD_WIN_DEL_00, 0 );
					// (yes/no)
					TdCmn_init_yesnowin( wk, T_DOWNLOAD_YESNO_YESNO );
					// 輝度変更
					TdCmn_set_passive( wk, TRUE );
					// アニメ停止
					TdCmn_stop_anim_speed( wk, TRUE );
					wk->Step = STEP_LV2+10;
				}
				// セーブ済み => 通常選択(詳細 or 戻る)
				else{
					// どうしますか
					TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_00, T_DOWNLOAD_WIN_WHAT_00, 0 );
					// (yes/no)
					TdCmn_init_yesnowin( wk, T_DOWNLOAD_YESNO_SAVE_AFTER );
					// 輝度変更
					TdCmn_set_passive( wk, TRUE );
					// アニメ停止
					TdCmn_stop_anim_speed( wk, TRUE );
					//
					wk->Step = STEP_LV2+20;
				}
			}
			break;
		//
		case STEP_LV2+10:
			// yes/noウインドウの選択処理
			tmp = TdCmn_chk_yesnowin( wk );
			if(tmp != -1){
				// 終了処理
				TdCmn_exit_yesnowin( wk );
				// はい=>削除、第2段階
				if(tmp == 0){
					// 本当に削除しますか
					TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_00, T_DOWNLOAD_WIN_DEL_01, 0 );
					// (yes/no)
					TdCmn_init_yesnowin( wk, T_DOWNLOAD_YESNO_YESNO );
					// 輝度変更=>そのまま引き継ぐ
					wk->Step = STEP_LV2+40;
				}
				// 戻る
				else{
					// 輝度戻し
					TdCmn_set_passive( wk, FALSE );
					// アニメ戻し
					TdCmn_stop_anim_speed( wk, FALSE );
					//
					TdCmn_clear_frame( wk, 1 );
					wk->Step = STEP_LV1;
					TdCmn_disp_title_info( wk, wk->Mode, 0 );
					// 通常アニメ
					TdCmn_set_panel_cursor( wk, 0 );
					// 上画面処理
					TdCmn_chk_up_disp( wk, FALSE, FALSE, FALSE );
				}
			}
			break;
		//
		//------------------------
		// どうする(詳細 or 戻る)
		//------------------------
		case STEP_LV2+20:
			// yes/noウインドウの選択処理
			tmp = TdCmn_chk_yesnowin( wk );
			if(tmp != -1){
				// 終了処理
				TdCmn_exit_yesnowin( wk );
				// はい=>詳細
				if(tmp == 0){
					// 終了処理
					TdCmn_clear_frame( wk, 1 );
					// 輝度戻し
					TdCmn_set_passive( wk, FALSE );
					// アニメ戻し
					TdCmn_stop_anim_speed( wk, FALSE );
					// 左上タイトル表示
//					TdCmn_disp_title_info( wk, wk->Mode, 0);
					wk->Step = STEP_LV3+10;
					// 詳細画面に行く(戻りで必要)
					wk->Flag |= FLG_MW_SAVE2INFO;
					//
					// カーソル非表示
					TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, FALSE );
					for(i=0;i<3;i++){
						// パネル表示
						TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_PANEL_00+i, FALSE );
					}
				}
				// 戻る
				else{
					// 輝度戻し
					TdCmn_set_passive( wk, FALSE );
					// アニメ戻し
					TdCmn_stop_anim_speed( wk, FALSE );
					//
					TdCmn_clear_frame( wk, 1 );
					wk->Step = STEP_LV1;
					TdCmn_disp_title_info( wk, wk->Mode, 0 );
					// 通常アニメ
					TdCmn_set_panel_cursor( wk, 0 );
					// 上画面処理
					TdCmn_chk_up_disp( wk, FALSE, FALSE, FALSE );
				}
			}
			break;
		//
		//----------------
		// 本当に消しますか
		//----------------
		case STEP_LV2+40:
			// yes/noウインドウの選択処理
			tmp = TdCmn_chk_yesnowin( wk );
			if(tmp != -1){
				// 終了処理
				TdCmn_exit_yesnowin( wk );
				// はい=>本当に削除
				if(tmp == 0){
					// いったんクリア
					for(i=0;i<6;i++){
						TdCmn_clear_canvas( wk, T_DOWNLOAD_MSG_UP_TNAME+i );
					}
					// 削除画面
					TdCmn_clear_frame( wk, 1 );
					TdCmn_disp_title_info( wk, wk->Mode, 0 );
					// 削除しました(stream)
					TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_01, T_DOWNLOAD_WIN_DEL_DONE, 1 );
					// 輝度変更=>そのまま
					// アニメ停止
					TdCmn_stop_anim_speed( wk, TRUE );
					// 削除した処理
					TdCmn_set_del_save( wk, TRUE, wk->SaveCurPos );
					// 削除してセーブにつながる処理
					wk->Step = STEP_LV4;
				}
				// 戻る
				else{
					// 輝度戻し
					TdCmn_set_passive( wk, FALSE );
					// アニメ戻し
					TdCmn_stop_anim_speed( wk, FALSE );
					//
					TdCmn_clear_frame( wk, 1 );
					wk->Step = STEP_LV1;
					TdCmn_disp_title_info( wk, wk->Mode, 0 );
					// 通常アニメ
					TdCmn_set_panel_cursor( wk, 0 );
					// 上画面処理
					TdCmn_chk_up_disp( wk, FALSE, FALSE, FALSE );
				}
			}
			break;
		//
		//-----------------
		// 選択画面へ移行
		//-----------------
		case STEP_LV3:
			TdCmn_clear_frame( wk, 0 );	// 他とあわせるため
			// 戻り場所をセット
			wk->RetMode = wk->Mode;
			td_exit_save( wk );
			wk->Step = STEP_INIT;
			wk->Mode = T_DOWNLOAD_MODE_SELECT;
			//
			// 初期化まとめ
			td_sub_init( wk );
			// フラグ初期化
			wk->Flag &= ~(FLG_MW_SELECT_SAVE|FLG_MW_SAVED);
			break;
		//
		//----------------
		// 詳細画面へ移行
		//----------------
		case STEP_LV3+10:
			TdCmn_clear_frame( wk, 0 );
			// 戻り場所をセット
			wk->RetMode = wk->Mode;
			td_exit_save( wk );
			wk->Step = STEP_INIT;
			wk->Mode = T_DOWNLOAD_MODE_INFO;
			//
			// 初期化まとめ
			td_sub_init( wk );
			// フラグ初期化
			// (ここでは、FLG_MW_SAVEDはクリアしない)
			wk->Flag &= ~FLG_MW_SELECT_SAVE;
			break;
		
		//------------------
		// 削除して=>セーブ中(stream)
		//------------------
		case STEP_LV4:
			bflg = TdCmn_main_meswin( wk );
			if(bflg == FALSE){
				// ウインドウ
				TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_01, T_DOWNLOAD_WIN_NOW_SAVING, 0 );
				wk->Step = STEP_LV4+10;
				//----------------
				// セーブ処理
				// => メッセージ終了後セーブ
				//----------------
			}
			break;
		
		//------------------------------
		// セーブ処理
		// (すぐセーブの場合)
		//------------------------------
		case STEP_LV4+5:
			// 処理中は更新しない
			if( PRINTSYS_QUE_IsFinished( wk->print_que ) == FALSE ){
				break;
			}
			// 文字の処理(負荷が高い)をしているとセーブ時に落ちてしまうので、
			// 終了を待ってから処理
			// 空きを見つけてセーブ
			tmp = TdSave_get_save_pos( wk );
			TdSave_init_save( wk, wk->AllCurPos[T_DOWNLOAD_LIST_SELECT], tmp );
			TdSave_init_timeicon( wk );
			wk->Step = STEP_LV5;
			break;
		
		//------------------------------
		// セーブ処理
		// (削除後のセーブ)
		//------------------------------
		case STEP_LV4+10:
			// 処理中は更新しない
			if( PRINTSYS_QUE_IsFinished( wk->print_que ) == FALSE ){
				break;
			}
			// 指定の位置にセーブ
			tmp = wk->SaveCurPos;
			TdSave_init_save( wk, wk->AllCurPos[T_DOWNLOAD_LIST_SELECT], tmp );
			TdSave_init_timeicon( wk );
			wk->Step = STEP_LV5;
			break;
		
		//----------------------
		// セーブ画面(セーブ中)
		//----------------------
		case STEP_LV5:
			bflg = TdSave_main_save( wk );
			if(bflg == FALSE){
				// 後処理
				wk->DispMaxNum = wk->SaveMaxNum;
				wk->SaveCurPos = wk->SaveDest;
				// 終了
				TdSave_exit_timeicon( wk );
				// ウインドウ
				TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_01, T_DOWNLOAD_WIN_SAVE_DONE, 1 );
				wk->Step = STEP_LV5+10;
				// SE
				PMSND_PlaySE( SND_T_DOWNLOAD_DECIDE );
				//
				// 後処理
				// 削除した状況を復帰
				TdCmn_set_del_save( wk, FALSE, wk->SaveCurPos );
				//
				for(i=0;i<wk->DispMaxNum;i++){
					TdCmn_disp_panel_string( wk, T_DOWNLOAD_DATA_SAVE, i );
					// パネル表示
					TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_PANEL_00+i, TRUE );
				}
				// カーソル
				TdCmn_disp_panel_cursor( wk, T_DOWNLOAD_PANEL_3, wk->SaveCurPos );
				// アニメ停止
				TdCmn_stop_anim_speed( wk, TRUE );
				// 上画面表示
				TdCmn_disp_upmes( wk, T_DOWNLOAD_DATA_SAVE, wk->SaveCurPos );
			}
			break;
		//
		// セーブしました(stream)
		case STEP_LV5+10:
			bflg = TdCmn_main_meswin( wk );
			if(bflg == FALSE){
				// 終了処理
				TdCmn_clear_frame( wk, 1 );
				// 輝度戻し
				TdCmn_set_passive( wk, FALSE );
				// アニメ戻し
				TdCmn_stop_anim_speed( wk, FALSE );
				// 左上タイトル表示
				TdCmn_disp_title_info( wk, wk->Mode, 0);
				// 通常アニメ
				TdCmn_set_panel_cursor( wk, 0 );
				wk->Step = STEP_LV1;
				//
				wk->Flag |= FLG_MW_SAVED;
				// 上画面処理
				TdCmn_chk_up_disp( wk, FALSE, FALSE, FALSE );
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
 *	@brief  終了処理(save)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_exit_save( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		i;
	
	for(i=0;i<3;i++){
		// パネル表示
		TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_PANEL_00+i, FALSE );
	}
	// 文字列破棄
	TdCmn_exit_string2( wk, wk->Mode );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  debug
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_debug( T_DOWNLOAD_MAIN_WORK* wk )
{
#ifdef __CHK_TD__
	if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_X) ){
		wk->DebugFlag |= FLG_DBG_ON;
		wk->DebugFlag |= FLG_DBG_UP;
	}
	if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_Y) ){
		wk->DebugFlag &= ~(FLG_DBG_ON|FLG_DBG_UP);
	}
#endif
}

