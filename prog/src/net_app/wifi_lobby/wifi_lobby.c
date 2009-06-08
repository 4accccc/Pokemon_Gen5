//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		wifi_lobby.c
 *	@brief		WiFiロビーメインシステム
 *	@author		tomoya takahashi
 *	@data		2007.09.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

//#include "system/pm_overlay.h"

#include "savedata/save_control.h"

#include "net_app/wifi_lobby.h"

#include "net\network_define.h"

//#include "wifi/dwc_overlay.h"

#include "wflby_system.h"
#include "wflby_apl.h"

#include <ppwlobby/ppw_lobby.h>

#include "net_app\wifi_lobby.h"
#include "system/main.h"
#include "system/gfl_use.h"
#include "net_old/net_old.h"

//-----------------------------------------------------------------------------
/**
 *					コーディング規約
 *		●関数名
 *				１文字目は大文字それ以降は小文字にする
 *		●変数名
 *				・変数共通
 *						constには c_ を付ける
 *						staticには s_ を付ける
 *						ポインタには p_ を付ける
 *						全て合わさると csp_ となる
 *				・グローバル変数
 *						１文字目は大文字
 *				・関数内変数
 *						小文字と”＿”と数字を使用する 関数の引数もこれと同じ
*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	WiFiロビーメインワーク
//=====================================
typedef struct {
	SAVE_CONTROL_WORK*		p_save;		// セーブデータ
	WFLBY_SYSTEM*	p_commsys;	// WiFiロビー共通処理システム
	WFLBY_APL*		p_apl;		// WiFiロビーアプリ管理システム
	GFL_TCB* p_vtcb;				// VBLANKTCB
	GAME_COMM_SYS_PTR p_game_comm;
} WFLBY_WK;



//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
static void WFLBY_VBlankFunc( GFL_TCB* p_tcb, void* p_work );


//==============================================================================
//	データ
//==============================================================================
const GFL_PROC_DATA WFLBY_PROC = {	
	WFLBYProc_Init,
	WFLBYProc_Main,
	WFLBYProc_Exit,
};

//==============================================================================
//	
//==============================================================================
FS_EXTERN_OVERLAY(dpw_common);


//----------------------------------------------------------------------------
/**
 *	@brief	初期化処理
 *
 *	@param	p_proc		ワーク
 *	@param	p_seq		シーケンス
 *
 *	@retval	GFL_PROC_RES_CONTINUE = 0,		///<動作継続中
 *	@retval	GFL_PROC_RES_FINISH,			///<動作終了
 */
//-----------------------------------------------------------------------------
GFL_PROC_RESULT WFLBYProc_Init( GFL_PROC* p_proc, int* p_seq , void * pwk, void * mywk)
{
	WFLBY_WK* p_wk;
	WFLBY_PROC_PARAM* p_param;

	{
		// オーバーレイ
		FS_EXTERN_OVERLAY(wifi2dmap);
		GFL_OVERLAY_Load( FS_OVERLAY_ID(wifi2dmap) );
	
	#if WB_FIX
		// WiFiオーバーレイ開始
		DwcOverlayStart();
	#endif

		// 世界交換（ghttpライブラリのため）
	#if WB_FIX
		DpwCommonOverlayStart();
	#else
		GFL_OVERLAY_Load( FS_OVERLAY_ID(dpw_common) );
    NetOld_Load();
	#endif
	}
	
	// ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WFLOBBY, 0x5000 );

	// ワーク作成
	p_wk = GFL_PROC_AllocWork( p_proc, sizeof(WFLBY_WK), HEAPID_WFLOBBY );
	GFL_STD_MemFill( p_wk, 0, sizeof(WFLBY_WK) );

	// パラメータワーク取得
	p_param = pwk;

	// セーブデータ取得
	p_wk->p_save = p_param->p_save;
	p_wk->p_game_comm = p_param->p_game_comm;

	// 共通処理システム作成
	p_wk->p_commsys = WFLBY_SYSTEM_Init( p_wk->p_save, HEAPID_WFLOBBY );
	p_wk->p_apl = WFLBY_APL_Init( p_param->check_skip, p_param->p_save, 
			p_param->p_wflby_counter, p_wk->p_commsys, HEAPID_WFLOBBY, p_param->p_game_comm );

	// VブランクHブランク関数設定
	//sys_HBlankIntrStop();	//HBlank割り込み停止


	// 常時動作VBLANKタスク生成
	p_wk->p_vtcb = GFUser_VIntr_CreateTCB( WFLBY_VBlankFunc, p_wk, 0 );

	// プロック開始
	WFLBY_APL_Start( p_wk->p_apl );
	
	return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	メイン処理
 *
 *	@param	p_proc		ワーク
 *	@param	p_seq		シーケンス
 *
 *	@retval	GFL_PROC_RES_CONTINUE = 0,		///<動作継続中
 *	@retval	GFL_PROC_RES_FINISH,			///<動作終了
 */
//-----------------------------------------------------------------------------
GFL_PROC_RESULT WFLBYProc_Main( GFL_PROC* p_proc, int* p_seq, void * pwk, void * mywk )
{
	WFLBY_WK* p_wk;
	WFLBY_APL_RET apl_ret;

	p_wk = mywk;

	// アプリメイン
	apl_ret = WFLBY_APL_Main( p_wk->p_apl );

	// 共通処理メイン
	// 位置はアプリのあと固定！！！！
	WFLBY_SYSTEM_Main( p_wk->p_commsys );

	// 終了チェック
	if( apl_ret == WFLBY_APL_RET_END ){
		return GFL_PROC_RES_FINISH;
	}
	return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	破棄処理
 *
 *	@param	p_proc		ワーク
 *	@param	p_seq		シーケンス
 *
 *	@retval	GFL_PROC_RES_CONTINUE = 0,		///<動作継続中
 *	@retval	GFL_PROC_RES_FINISH,			///<動作終了
 */
//-----------------------------------------------------------------------------
GFL_PROC_RESULT WFLBYProc_Exit( GFL_PROC* p_proc, int* p_seq, void * pwk, void * mywk )
{
	WFLBY_WK* p_wk;

	p_wk = mywk;

	// 常時動作VブランクTCB破棄
	GFL_TCB_DeleteTask( p_wk->p_vtcb );
	
	// VブランクHブランク関数設定
//	sys_VBlankFuncChange( NULL, NULL );	// VBlankセット
	//sys_HBlankIntrStop();	//HBlank割り込み停止

	// アプリシステム破棄
	WFLBY_APL_Exit( p_wk->p_apl );

	// 共通処理システム破棄
	WFLBY_SYSTEM_Exit( p_wk->p_commsys );

	// ワーク破棄
	GFL_PROC_FreeWork( p_proc );

	// ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_WFLOBBY );

	// ２Dマップシステムをオーバーレイを破棄
	{
		FS_EXTERN_OVERLAY(wifi2dmap);
		GFL_OVERLAY_Unload( FS_OVERLAY_ID(wifi2dmap) );
	
		// 世界交換（ghttpライブラリのため）
	#if WB_FIX
		DpwCommonOverlayEnd();
	#else
		GFL_OVERLAY_Unload( FS_OVERLAY_ID(dpw_common) );
    NetOld_Unload();
	#endif
	
	#if WB_FIX
		// WiFiオーバーレイ終了
		DwcOverlayEnd();
	#endif
	}

	return GFL_PROC_RES_FINISH;
}





//-----------------------------------------------------------------------------
/**
 *				プライベート関数
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief	VBlank関数
 *
 *	@param	p_tcb		TCBポインタ
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_VBlankFunc( GFL_TCB* p_tcb, void* p_work )
{
	WFLBY_WK* p_wk;
	p_wk = p_work;

	WFLBY_APL_VBlank( p_wk->p_apl );
	WFLBY_SYSTEM_VBlank( p_wk->p_commsys );
}

