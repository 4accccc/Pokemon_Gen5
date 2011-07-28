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
	FLG_MW_ANM_DOWN		= (1<<4),		// 下矢印アニメ
	FLG_MW_ANM_UP		= (1<<5),		// 上矢印アニメ
	//
	FLG_MW_DEL_SAVE		= (1<<8),		// 削除してセーブにつながる
	FLG_MW_INFO_UPDATE	= (1<<9),		// 詳細更新
	//
	FLG_MW_RET_END		= (1<<12),		// RETURNで終了
	FLG_MW_CROSS_END	= (1<<13),		// ×で終了
	FLG_MW_JOIN_END		= (1<<14),		// 参加で終了
	FLG_MW_TITLE_UPDATE	= (1<<15),		// タイトル更新
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
static void td_main_print( T_DOWNLOAD_MAIN_WORK *wk );
//
// アニメ待ち
static BOOL td_chk_anim( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );
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
	wk->ScrollOffy  = 0;
	wk->DispMaxNum	= 0;
	wk->PFlag		= 0;
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
	//
	for(i=0;i<T_DOWNLOAD_CANVAS_MAXNUM;i++){
		wk->PrintUtil[i].win = 0;
	}
	wk->frame_list_work = 0;
	//
	// empty用バッファ取得(必ず破棄すること)
	wk->EmptyBuffer = GFL_ARC_LoadDataAlloc( ARCID_T_DOWNLOAD_GRA, NARC_t_download_gra_t_download_pos_tblbin, wk->heapID );
	
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
	//
	//-------------------
	// カーソル位置(参加)
	//-------------------
	if(wk->Flag & FLG_MW_JOIN_END){
		if(wk->Param->curpos != NULL){
			*wk->Param->curpos = T_DOWNLOAD_CURPOS_OFFSET;	// test
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
	// emptyのバッファクリア
	GFL_HEAP_FreeMemory( wk->EmptyBuffer );
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
			// 画面の切り替えで必要になるかもしれない(保留)
//			if(!(wk->PFlag & (1<<i))){
//				// キャラの転送終了後スクリーン作成して転送
//				GFL_BMPWIN_MakeScreen( wk->PrintUtil[i].win );
//				GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->PrintUtil[i].win) );
//				wk->PFlag |= (1<<i);
//			}
		}
	}
}

//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
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
	// 下アニメ再生中
	else if(wk->Flag & FLG_MW_ANM_DOWN){
		wnum = T_DOWNLOAD_CLWKID_CMN_DOWN;
		flg = 1;
	}
	// 上アニメ再生中
	else if(wk->Flag & FLG_MW_ANM_UP){
		wnum = T_DOWNLOAD_CLWKID_CMN_UP;
		flg = 1;
	}

	// アニメ終了判定を行う
	if(flg == 1){
		bflg = TdCmn_is_anime_end( wk, T_DOWNLOAD_CLWKMODE_CMN, wnum );
		if(bflg == FALSE){
			flg = 2;
			// アニメの終了
			// とりあえず通常に戻しておく
			// =>最終で待つでもよいはず
//			TdCmn_set_cmn_anim( wk, wnum );
			wk->Flag &= ~(1<<wnum);
			//
			// 終了判定ありの場合
			if(mode == 0){
				// 戻るボタン(飛び先変更はある)
				if((wnum == T_DOWNLOAD_CLWKID_CMN_CROSS)||(wnum == T_DOWNLOAD_CLWKID_CMN_RETURN)){
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
			rflg = TRUE;
		}
	}
	//
	return( rflg );
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
	// 左上タイトル表示
	TdCmn_disp_title_info( wk, wk->Mode, 0);
	// 判定の後に表示
	TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_01, T_DOWNLOAD_WIN_JOIN_00 );
	// 輝度変更
	TdCmn_set_passive( wk, TRUE );
	//
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
		// 決定アニメ
		TdCmn_set_framelist_cursor( wk, 1 );
		wk->Step = STEP_LV2;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_DECIDE );
	}
	// 左右キー
	else if(listflg == 3){
		wk->Step = STEP_LV1+20;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
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
	BOOL	bflg=FALSE;
	s32		ret = TdCmn_chk_tp( wk, 0, wk->Mode );
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
//			break;
		//
		// メッセージ表示待ち
		case STEP_INIT+10:
			TdCmn_main_keycursor( wk );
			bflg = td_chk_wait_mes_join( wk );
			if(bflg == TRUE){
				// 終了処理
				TdCmn_clear_frame( wk, 1 );
				// 輝度戻し
				TdCmn_set_passive( wk, FALSE );
				// 左上タイトル表示
				TdCmn_disp_title_info( wk, wk->Mode, 0);
				wk->Step = STEP_LV1;
				// SE
				PMSND_PlaySE( SND_T_DOWNLOAD_DECIDE );
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
		// 決定
		case STEP_LV2:
			bflg = TdCmn_is_anime_end( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR );
			if(bflg == FALSE){
				// ウインドウ
				TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_00, T_DOWNLOAD_WIN_JOIN_01 );
				// 3拓ウインドウ(参加する)
				TdCmn_init_yesnowin( wk, T_DOWNLOAD_YESNO_JOIN );
				// 輝度変更
				TdCmn_set_passive( wk, TRUE );
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
				}else{
					TdCmn_clear_frame( wk, 1 );
					wk->Step = STEP_LV1;
					// 左上タイトル表示
					TdCmn_disp_title_info( wk, wk->Mode, 0);
					// 通常アニメ
					TdCmn_set_framelist_cursor( wk, 0 );
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
	
	// 左右矢印非表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_CMN, T_DOWNLOAD_CLWKID_CMN_LEFT,  FALSE );
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_CMN, T_DOWNLOAD_CLWKID_CMN_RIGHT, FALSE );
	// 上下矢印非表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_CMN, T_DOWNLOAD_CLWKID_CMN_UP,   TRUE );
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_CMN, T_DOWNLOAD_CLWKID_CMN_DOWN, TRUE );
	
	// BGの表示
	TdCmn_init_bg_info( wk );
	//
	// パレットチェンジon
	TdCmn_set_chgpal( wk, TRUE );
	//
	// 文字列初期化
	TdCmn_init_string2( wk );
	//
	// 初期カーソル位置
	wk->InfoCurPos = 0;
	wk->Flag |= FLG_MW_INFO_UPDATE;
	
	// パラメータ初期化
	TdCmn_init_info_mode( wk );
	// 情報表示
	TdCmn_disp_info_mode( wk, wk->InfoCurPos );
	// 左上タイトル表示
	TdCmn_disp_title_info( wk, wk->Mode, 0);
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
	if(wk->Flag & FLG_MW_INFO_UPDATE){
		// 情報表示
		TdCmn_disp_info_mode( wk, wk->InfoCurPos );
		// カーソル位置
		TdCmn_chg_info_curpos( wk, wk->InfoCurPos );
		// カーソル移動
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
	s32		oldpos = wk->CurrentPageNum[wk->InfoCurPos];
	s32		ret = TdCmn_chk_tp( wk, 0, wk->Mode );		// trg
	s32		cret = TdCmn_chk_tp( wk, 1, wk->Mode );		// cont
	//
	// ×
	if(ret == T_DOWNLOAD_AREA_TD_TP_CROSS){
		TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_CROSS );
		flg = 1;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_CANCEL );
	}
	// RETURN
	else if(ret == T_DOWNLOAD_AREA_TD_TP_RETURN){
		TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RETURN );
		flg = 1;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_CANCEL );
	}
	// UP
	else if(ret == T_DOWNLOAD_AREA_TD_TP_UP){
		// リミットチェック
		if(wk->CurrentPageNum[wk->InfoCurPos] != 0){
			wk->CurrentPageNum[wk->InfoCurPos]--;
			if(wk->CurrentPageNum[wk->InfoCurPos] < 0){
				wk->CurrentPageNum[wk->InfoCurPos] = 0;
			}
			TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_UP );
			flg = 2;
			sflg = 1;
			// SE
			PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
		}
	}
	// DOWN
	else if(ret == T_DOWNLOAD_AREA_TD_TP_DOWN){
		// リミットチェック
		if(wk->CurrentPageNum[wk->InfoCurPos] != (wk->PageMaxNum[wk->InfoCurPos]-1)){
			wk->CurrentPageNum[wk->InfoCurPos]++;
			if(wk->CurrentPageNum[wk->InfoCurPos] >= wk->PageMaxNum[wk->InfoCurPos]){
				wk->CurrentPageNum[wk->InfoCurPos] = (wk->PageMaxNum[wk->InfoCurPos]-1);
			}
			TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_DOWN );
			flg = 2;
			sflg = 1;
			// SE
			PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
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
			sflg = 2;
		}
	}
	// カーソル切り替え
	if(sflg == 1){
		// スクロールバー更新
		TdCmn_set_scrollbar_info( wk, wk->InfoCurPos, wk->CurrentPageNum[wk->InfoCurPos] );
		wk->Flag |= FLG_MW_INFO_UPDATE;
		wk->Flag |= FLG_MW_TITLE_UPDATE;
	}
	else if(sflg == 2){
		// カーソル位置
		num = TdCmn_calc_scrollbar_info( wk, wk->InfoCurPos );
		if(num != -1){
			// 更新があったときのみ
			if(num != oldpos){
				// スクロールバー位置更新
				wk->CurrentPageNum[wk->InfoCurPos] = num;
				TdCmn_set_scrollbar_info( wk, wk->InfoCurPos, num );
				wk->Flag |= FLG_MW_INFO_UPDATE;
			}
		}
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
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_CANCEL );
	}
	// 戻る
	else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
		TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RETURN );
		flg = 1;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_CANCEL );
	}
	// 左
	else if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT ){
		wk->InfoCurPos--;
		if(wk->InfoCurPos < 0){
			wk->InfoCurPos = T_DOWNLOAD_INFO_MAX-1;
		}
		sflg = 1;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
	}
	// 右
	else if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT ){
		wk->InfoCurPos++;
		if(wk->InfoCurPos >= T_DOWNLOAD_INFO_MAX){
			wk->InfoCurPos = 0;
		}
		sflg = 1;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
	}
	// 下
	else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
		// リミットチェック
		if(wk->CurrentPageNum[wk->InfoCurPos] != (wk->PageMaxNum[wk->InfoCurPos]-1)){
			wk->CurrentPageNum[wk->InfoCurPos]++;
			if(wk->CurrentPageNum[wk->InfoCurPos] >= wk->PageMaxNum[wk->InfoCurPos]){
				wk->CurrentPageNum[wk->InfoCurPos] = (wk->PageMaxNum[wk->InfoCurPos]-1);
			}
			TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_DOWN );
			flg = 2;
			sflg = 2;
			// SE
			PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
		}
	}
	// 上
	else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP ){
		// リミットチェック
		if(wk->CurrentPageNum[wk->InfoCurPos] != 0){
			wk->CurrentPageNum[wk->InfoCurPos]--;
			if(wk->CurrentPageNum[wk->InfoCurPos] < 0){
				wk->CurrentPageNum[wk->InfoCurPos] = 0;
			}
			TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_UP );
			flg = 2;
			sflg = 2;
			// SE
			PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
		}
	}
	//
	// カーソル切り替え
	if(sflg != 0){
		// スクロールバー更新
		TdCmn_set_scrollbar_info( wk, wk->InfoCurPos, wk->CurrentPageNum[wk->InfoCurPos] );
		wk->Flag |= FLG_MW_INFO_UPDATE;
		//
		if(sflg == 1){
			wk->Flag |= FLG_MW_TITLE_UPDATE;
		}
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
//			break;
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
		// 上下アニメ待ち
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
	//
	// アイコン非表示
	for(i=0;i<3;i++){
		TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_ICON_00+i, FALSE );
	}
	// 文字列破棄
	TdCmn_exit_string2( wk );
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
	// 通常アニメ
	TdCmn_set_framelist_cursor( wk, 0 );
	// バー非表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_BAR, TRUE );
	// カーソル移動
	TdCmn_disp_framelist_cursor( wk, wk->ListCurPos[T_DOWNLOAD_LIST_CURRENT] );
	// タイトル表示
	TdCmn_disp_title_info( wk, wk->Mode, 0);
	//
	// 削除してセーブ
	if(wk->Flag & FLG_MW_DEL_SAVE){
		wk->Flag &= ~FLG_MW_DEL_SAVE;
		// ウインドウ
		TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_01, T_DOWNLOAD_WIN_NOW_SAVING );
		TdCmn_disp_title_info( wk, wk->Mode, 0 );
		// 輝度変更
		TdCmn_set_passive( wk, TRUE );
		//
		wk->Step = STEP_LV5;
	}
	// 3つセーブ済み
	else{
		// 判定の後に表示
		TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_01, T_DOWNLOAD_WIN_SEL_DATA );
		// 輝度変更
		TdCmn_set_passive( wk, TRUE );
		// 通常
		wk->Step = STEP_INIT+10;
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
	
	// リスト処理
	ret = TdCmn_main_framelist( wk );
	listflg = TdCmn_is_main_framelist( wk, ret );
	if(listflg != 0){
		flg = 1;
	}
	if(listflg == 2){
		// 決定アニメ
		TdCmn_set_framelist_cursor( wk, 1 );
		//
		// 分岐テスト(判定が必要)
		// 0:セーブ、1:一杯なので削除へ、2:セーブ済み
		if(wk->AllCurPos[T_DOWNLOAD_LIST_CURRENT] < 2){
			wk->Step = STEP_LV2;
		}else{
			wk->Step = STEP_LV2+30;
		}
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_DECIDE );
	}
	// 左右キー
	else if(listflg == 3){
		wk->Step = STEP_LV1+20;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
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
	s32		flg=0;
	BOOL	bflg=FALSE;
	s32		ret = TdCmn_chk_tp( wk, 0, wk->Mode );
	
	// ×
	if(ret == T_DOWNLOAD_AREA_TD_TP_CROSS){
		TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_CROSS );
		flg = 1;
	}
	// RETURN
	else if(ret == T_DOWNLOAD_AREA_TD_TP_RETURN){
		TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RETURN );
		flg = 1;
	}
	//
	// 更新
	if(flg != 0){
		bflg = TRUE;
		wk->Step = STEP_LV1+10;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_CANCEL );
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
		//------------
		// 開始win待ち
		//------------
		case STEP_INIT+10:
			TdCmn_main_keycursor( wk );
			bflg = td_chk_wait_mes_select( wk );
			if(bflg == TRUE){
				// 終了処理
				TdCmn_clear_frame( wk, 1 );
				// 輝度戻し
				TdCmn_set_passive( wk, FALSE );
				// 左上タイトル表示
				TdCmn_disp_title_info( wk, wk->Mode, 0);
				wk->Step = STEP_LV1;
				// SE
				PMSND_PlaySE( SND_T_DOWNLOAD_DECIDE );
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
			bflg = td_chk_anim( wk, 0 );
			if(bflg == TRUE){
				// ウインドウ(終了しますか)
				TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_00, T_DOWNLOAD_WIN_FINISH );
				// (yes/no)
				TdCmn_init_yesnowin( wk, T_DOWNLOAD_YESNO_YESNO );
				// 輝度変更
				TdCmn_set_passive( wk, TRUE );
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
					TdCmn_set_framelist_cursor( wk, 0 );
					// フラグ戻し
					wk->Flag &= ~(FLG_MW_CROSS_END|FLG_MW_RET_END);
				}
			}
			break;
		//
		//----------------
		// 左右アニメ待ち
		//----------------
		case STEP_LV1+20:
			bflg = td_chk_anim( wk, 1 );
			if(bflg == TRUE){
				wk->Step = STEP_LV1;
			}
			break;
		//
		//---------------
		// 決定アニメ待ち
		// (save3択)
		//---------------
		case STEP_LV2:
			bflg = TdCmn_is_anime_end( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR );
			if(bflg == FALSE){
				// ウインドウ
				TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_00, T_DOWNLOAD_WIN_WHAT_00 );
				// 3択ウインドウ(セーブする)
				TdCmn_init_yesnowin( wk, T_DOWNLOAD_YESNO_SAVE );
				// 輝度変更
				TdCmn_set_passive( wk, TRUE );
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
					// セーブします(テスト)
					if(wk->AllCurPos[T_DOWNLOAD_LIST_CURRENT] == 0){
						TdCmn_clear_frame( wk, 1 );
						// ウインドウ
						TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_01, T_DOWNLOAD_WIN_NOW_SAVING );
						TdCmn_disp_title_info( wk, wk->Mode, 0 );
						// 輝度変更=>そのまま引き継ぐ
						wk->Step = STEP_LV5;
					}else{
						// 輝度戻し
						TdCmn_set_passive( wk, FALSE );
						wk->Step = STEP_LV4;
						// カーソル表示(ちらつくので消しておく)
						TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, FALSE );
					}
				}
				// 詳細画面へ
				else if(tmp == 1){
					// 輝度戻し
					TdCmn_set_passive( wk, FALSE );
					wk->Step = STEP_LV3;
					// カーソル表示(ちらつくので消しておく)
					TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, FALSE );
				}
				// 戻る
				else{
					// 輝度戻し
					TdCmn_set_passive( wk, FALSE );
					TdCmn_clear_frame( wk, 1 );
					wk->Step = STEP_LV1;
					TdCmn_disp_title_info( wk, wk->Mode, 0 );
					// 通常アニメ
					TdCmn_set_framelist_cursor( wk, 0 );
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
				// ウインドウ
				TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_01, T_DOWNLOAD_WIN_SAVED );
				// 輝度変更
				TdCmn_set_passive( wk, TRUE );
				wk->Step = STEP_LV2+40;
			}
			break;
		//
		// 決定ウインドウ待ち
		case STEP_LV2+40:
			TdCmn_main_keycursor( wk );
			bflg = td_chk_wait_mes_select( wk );
			if(bflg == TRUE){
				// 終了処理
				TdCmn_clear_frame( wk, 1 );
				// 輝度戻し
				TdCmn_set_passive( wk, FALSE );
				// 左上タイトル表示
				TdCmn_disp_title_info( wk, wk->Mode, 0);
				wk->Step = STEP_LV1;
				// SE
				PMSND_PlaySE( SND_T_DOWNLOAD_DECIDE );
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
		
		//------------------
		// セーブ画面(判定あり)
		//------------------
		case STEP_LV5:
			TdCmn_main_keycursor( wk );
			bflg = td_chk_wait_mes_select( wk );
			if(bflg == TRUE){
				// ウインドウ
				TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_01, T_DOWNLOAD_WIN_SAVE_DONE );
				wk->Step = STEP_LV5+10;
				// SE
				PMSND_PlaySE( SND_T_DOWNLOAD_DECIDE );
			}
			break;
		//
		// セーブしました
		case STEP_LV5+10:
			TdCmn_main_keycursor( wk );
			bflg = td_chk_wait_mes_select( wk );
			if(bflg == TRUE){
				// 終了処理
				TdCmn_clear_frame( wk, 1 );
				// 輝度戻し
				TdCmn_set_passive( wk, FALSE );
				// 左上タイトル表示
				TdCmn_disp_title_info( wk, wk->Mode, 0);
				wk->Step = STEP_LV1;
				// SE
				PMSND_PlaySE( SND_T_DOWNLOAD_DECIDE );
			}
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
	// 通常アニメ
	TdCmn_set_framelist_cursor( wk, 0 );
	// カーソル移動
	TdCmn_disp_framelist_cursor( wk, wk->ListCurPos[T_DOWNLOAD_LIST_CURRENT] );
	// タイトル表示
	TdCmn_disp_title_info( wk, wk->Mode, 0 );
	//
	// (いっぱいの場合)
	// 判定の後に表示
	TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_01, T_DOWNLOAD_WIN_DATA_FULL );
	// 輝度変更
	TdCmn_set_passive( wk, TRUE );
	//
	wk->Step = STEP_INIT+10;
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
	if(listflg == 2){
		// 決定アニメ
		TdCmn_set_framelist_cursor( wk, 1 );
		//
		wk->Step = STEP_LV2;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_DECIDE );
	}
	// 左右キー
	else if(listflg == 3){
		wk->Step = STEP_LV1+20;
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_SELECT );
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
	s32		ret = TdCmn_chk_tp( wk, 0, wk->Mode );
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
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_CANCEL );
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
		// SE
		PMSND_PlaySE( SND_T_DOWNLOAD_CANCEL );
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
	}
	else if( GFL_UI_TP_GetTrg() == TRUE ){
		bflg = TRUE;
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
		// データが一杯表示待ち
		//--------------------
		case STEP_INIT+10:
			TdCmn_main_keycursor( wk );
			bflg = td_chk_wait_mes_save( wk );
			if(bflg == TRUE){
				// 終了処理
				TdCmn_clear_frame( wk, 1 );
				// 輝度戻し
				TdCmn_set_passive( wk, FALSE );
				// 左上タイトル表示
				TdCmn_disp_title_info( wk, wk->Mode, 0);
				wk->Step = STEP_LV1;
				// SE
				PMSND_PlaySE( SND_T_DOWNLOAD_DECIDE );
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
			bflg = td_chk_anim( wk, 0 );
			if(bflg == TRUE){
				// ウインドウ(終了しますか)
				TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_00, T_DOWNLOAD_WIN_DEL_STOP );
				// (yes/no)
				TdCmn_init_yesnowin( wk, T_DOWNLOAD_YESNO_YESNO );
				// 輝度変更
				TdCmn_set_passive( wk, TRUE );
				wk->Step = STEP_LV1+15;
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
				// はい=>終了
				if(tmp == 0){
					TdCmn_clear_frame( wk, 1 );
					//
					// 戻り先チェック
					if(wk->RetMode == -1){
						wk->Step = STEP_END;
					}else{
						// カーソル表示(ちらつくので消しておく)
						TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, FALSE );
						wk->Step = STEP_LV3;
					}
				}
				// 戻る
				else{
					TdCmn_clear_frame( wk, 1 );
					wk->Step = STEP_LV1;
					TdCmn_disp_title_info( wk, wk->Mode, 0 );
					// 通常アニメ
					TdCmn_set_framelist_cursor( wk, 0 );
					// フラグ戻し
					wk->Flag &= ~(FLG_MW_CROSS_END|FLG_MW_RET_END);
				}
			}
			break;
		//
		//-----------------
		// 左右アニメ待ち
		//-----------------
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
		//-----------------
		// 削除win(第1段階)
		//-----------------
		case STEP_LV2:
			bflg = TdCmn_is_anime_end( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR );
			if(bflg == FALSE){
				// 削除しますか
				TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_00, T_DOWNLOAD_WIN_DEL_00 );
				// (yes/no)
				TdCmn_init_yesnowin( wk, T_DOWNLOAD_YESNO_YESNO );
				// 輝度変更
				TdCmn_set_passive( wk, TRUE );
				wk->Step = STEP_LV2+10;
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
					TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_00, T_DOWNLOAD_WIN_DEL_01 );
					// (yes/no)
					TdCmn_init_yesnowin( wk, T_DOWNLOAD_YESNO_YESNO );
					// 輝度変更=>そのまま引き継ぐ
					wk->Step = STEP_LV2+40;
				}
				// 戻る
				else{
					// 輝度戻し
					TdCmn_set_passive( wk, FALSE );
					//
					TdCmn_clear_frame( wk, 1 );
					wk->Step = STEP_LV1;
					TdCmn_disp_title_info( wk, wk->Mode, 0 );
					// 通常アニメ
					TdCmn_set_framelist_cursor( wk, 0 );
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
					// 削除画面(test)
					TdCmn_clear_frame( wk, 1 );
					TdCmn_disp_title_info( wk, wk->Mode, 0 );
					// 削除しました
					TdCmn_disp_winmes( wk, T_DOWNLOAD_MSG_DOWN_MESWIN_01, T_DOWNLOAD_WIN_DEL_DONE );
					// 輝度変更=>そのまま
					//
					// 削除してセーブにつながる処理
					wk->Flag |= FLG_MW_DEL_SAVE;
					wk->Step = STEP_LV2+50;
				}
				// 戻る
				else{
					// 輝度戻し
					TdCmn_set_passive( wk, FALSE );
					//
					TdCmn_clear_frame( wk, 1 );
					wk->Step = STEP_LV1;
					TdCmn_disp_title_info( wk, wk->Mode, 0 );
					// 通常アニメ
					TdCmn_set_framelist_cursor( wk, 0 );
				}
			}
			break;
		//
		//---------------
		// 削除しました
		//---------------
		case STEP_LV2+50:
			TdCmn_main_keycursor( wk );
			bflg = td_chk_wait_mes_save( wk );
			if(bflg == TRUE){
				// 終了処理
				TdCmn_clear_frame( wk, 1 );
				// 輝度戻し
				TdCmn_set_passive( wk, FALSE );
				// 左上タイトル表示
				TdCmn_disp_title_info( wk, wk->Mode, 0);
				wk->Step = STEP_LV3;
				// SE
				PMSND_PlaySE( SND_T_DOWNLOAD_DECIDE );
			}
			break;
		
		//-----------------
		// 選択画面へ移行
		//-----------------
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

