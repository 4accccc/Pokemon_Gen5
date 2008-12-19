//==============================================================================
/**
 * @file	balloon.c
 * @brief	風船膨らましメイン
 * @author	matsuda
 * @date	2007.11.01(木)
 */
//==============================================================================
#include <gflib.h>
#include <procsys.h>
#include <tcbl.h>
#include "system\main.h"
#include <dwc.h>

#include "balloon_common.h"
#include "net_app/balloon.h"
#include "balloon_game.h"
#include "balloon_entry.h"


//==============================================================================
//	定数定義
//==============================================================================
///メインシーケンス番号
enum{
	MAINSEQ_ENTRY_PROC,
	MAINSEQ_ENTRY_WAIT,
	MAINSEQ_GAME_PROC,
	MAINSEQ_GAME_WAIT,
	MAINSEQ_RESULT_PROC,
	MAINSEQ_RESULT_WAIT,
	MAINSEQ_END_BEFORE_TIMING,
	MAINSEQ_END_BEFORE_TIMING_WAIT,
	MAINSEQ_END,
};

///ゲーム終了時の最後の同期取りに使用する番号
#define BALLOON_END_TIMING_NO		(222)

//==============================================================================
//	GFL_PROCデータ
//==============================================================================
///風船割りゲーム画面プロセス定義データ
static const GFL_PROC_DATA BalloonGameProcData = {
	BalloonGameProc_Init,
	BalloonGameProc_Main,
	BalloonGameProc_End,
};

///風船割りエントリー画面＆結果発表画面プロセス定義データ
static const GFL_PROC_DATA BalloonEntryProcData = {
	BalloonEntryProc_Init,
	BalloonEntryProc_Main,
	BalloonEntryProc_End,
};


//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static void Ballon_ProcWorkInit(BALLOON_SYSTEM_WORK *bsw, BALLOON_PROC_WORK *parent);
static BOOL Ballon_DisconnectErrorCheck( BALLOON_SYSTEM_WORK *bsw );


//==============================================================================
//
//	
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   風船割り：初期化
 *
 * @param   proc		GFL_PROCへのポインタ
 * @param   seq			シーケンスワーク
 *
 * @retval  
 */
//--------------------------------------------------------------
GFL_PROC_RESULT BalloonProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	BALLOON_PROC_WORK *parent = pwk;
	BALLOON_SYSTEM_WORK *bsw;

	//Eメール管理用ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_BALLOON, 0x60000 );

	bsw = GFL_PROC_AllocWork(proc, sizeof(BALLOON_SYSTEM_WORK), HEAPID_BALLOON );
	GFL_STD_MemClear(bsw, sizeof(BALLOON_SYSTEM_WORK));
#ifdef PM_DEBUG
	bsw->debug_offline = parent->debug_offline;
#endif
	Ballon_ProcWorkInit(bsw, parent);
	
	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
/**
 * @brief   風船割り：メイン
 *
 * @param   proc		GFL_PROCへのポインタ
 * @param   seq			シーケンスワーク
 *
 * @retval  
 */
//--------------------------------------------------------------
GFL_PROC_RESULT BalloonProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	BALLOON_SYSTEM_WORK * bsw  = mywk;
	BALLOON_PROC_WORK *parent = pwk;

	// 通信エラー終了チェック
	if( Ballon_DisconnectErrorCheck( bsw ) == TRUE ){
		return GFL_PROC_RES_FINISH;
	}
	
	
	switch(*seq){
	case MAINSEQ_ENTRY_PROC:
	#ifdef PM_DEBUG
		if(parent->debug_offline == TRUE){
			*seq = MAINSEQ_GAME_PROC;
			break;
		}
	#endif
		bsw->mode = BALLOON_MODE_ENTRY;
		GFL_PROC_SysCallProc(NO_OVERLAY_ID, &BalloonEntryProcData, bsw);
		(*seq)++;
		break;
	case MAINSEQ_ENTRY_WAIT:
		(*seq)++;	//ゲーム画面へ
		break;

	case MAINSEQ_GAME_PROC:
		bsw->mode = BALLOON_MODE_GAME;
		GFL_PROC_SysCallProc(NO_OVERLAY_ID, &BalloonGameProcData, bsw);
		(*seq)++;
		break;
	case MAINSEQ_GAME_WAIT:
		(*seq)++;
		break;

	case MAINSEQ_RESULT_PROC:
	#ifdef PM_DEBUG
		if(parent->debug_offline == TRUE){
			*seq = MAINSEQ_END;
			break;
		}
	#endif
		if(parent->vchat){
			GFL_NET_DWC_StopVChat();
		}
		bsw->mode = BALLOON_MODE_RESULT;
		GFL_PROC_SysCallProc(NO_OVERLAY_ID, &BalloonEntryProcData, bsw);
		(*seq)++;
		break;
	case MAINSEQ_RESULT_WAIT:
		if(bsw->replay == TRUE){
			(*seq) = MAINSEQ_ENTRY_PROC;	//MAINSEQ_GAME_PROC;
		}
		else{
			(*seq)++;
		}
		break;
	
	case MAINSEQ_END_BEFORE_TIMING:	//ゲーム終了前の最後の同期取りを行う
	#if WB_FIX
		// 切断エラーを無視する（ブルースクリーンにも飛ばなくなる）
		CommStateSetErrorCheck(FALSE,TRUE);
	#endif
		//同期命令送信
		GFL_NET_HANDLE_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle(), BALLOON_END_TIMING_NO);
		(*seq)++;
		break;
	case MAINSEQ_END_BEFORE_TIMING_WAIT:
	#if WB_FIX
		if((GFL_NET_HANDLE_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(),BALLOON_END_TIMING_NO) == TRUE) 
				|| (CommGetConnectNum() < CommInfoGetEntryNum()) ){	// 同期が取れるか、人数が少なくなったらそのまま抜ける
	#else
		if((GFL_NET_HANDLE_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(),BALLOON_END_TIMING_NO) == TRUE)){
	#endif
			(*seq)++;
		}
		break;

	case MAINSEQ_END:
	default:
		return GFL_PROC_RES_FINISH;
	}
	
	return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   風船割り：終了処理
 *
 * @param   proc		GFL_PROCへのポインタ
 * @param   seq			シーケンスワーク
 *
 * @retval  
 */
//--------------------------------------------------------------
GFL_PROC_RESULT BalloonProc_End(GFL_PROC *proc, int *seq, void * pwk, void * mywk)
{
	BALLOON_SYSTEM_WORK * bsw  = mywk;

	GFL_PROC_FreeWork( proc );				// GFL_PROCワーク開放

	GFL_HEAP_DeleteHeap( HEAPID_BALLOON );

	return GFL_PROC_RES_FINISH;
}


//==============================================================================
//	
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   GFL_PROCワークの初期値設定
 * @param   bsw		GFL_PROCワークへのポインタ
 * @param   parent	BALLOON_PROC_WORKへのポインタ
 */
//--------------------------------------------------------------
static void Ballon_ProcWorkInit(BALLOON_SYSTEM_WORK *bsw, BALLOON_PROC_WORK *parent)
{
	int i, index, my_no, current_id;
	
//	bsw->result_param.p_gadget = &parent->gadget;	// GADGETなくしました tomoya
	
	current_id = GFL_NET_SystemGetCurrentID();
	my_no = 0;
	
	//参加しているnetIDのリストを作成
	index = 0;
#if WB_FIX
	for(i = 0; i < WFLBY_MINIGAME_MAX; i++){
		if(CommInfoGetMyStatus(i) != NULL){
			bsw->player_netid[index] = i;
			if(i == current_id){
				my_no = index;
			}
			index++;
		}
		else{
			bsw->player_netid[i] = 0xffff;
		}
	}
	bsw->player_max = index;
#endif

	bsw->vchat = parent->vchat;
	
#ifdef PM_DEBUG
	if(bsw->debug_offline == TRUE){
		bsw->player_netid[0] = 0;
		bsw->player_max = 1;
	}
	else{
		//エントリー画面＆結果発表画面用の値セット
		MNGM_ENRES_PARAM_Init( &bsw->entry_param, parent->wifi_lobby, parent->p_save, parent->vchat, &parent->lobby_wk );
	}
#else
	MNGM_ENRES_PARAM_Init( &bsw->entry_param, parent->wifi_lobby, parent->p_save, parent->vchat, &parent->lobby_wk );
#endif
}



//----------------------------------------------------------------------------
/**
 *	@brief	通信エラー切断チェック
 *
 *	@param	bsw		ワーク
 *
 *	@retval	TRUE	終了へ
 *	@retval	FALSE	エラーなし
 */
//-----------------------------------------------------------------------------
static BOOL Ballon_DisconnectErrorCheck( BALLOON_SYSTEM_WORK *bsw )
{
#if WB_FIX
	if( bsw->dis_error == FALSE ){
		if( MNGM_ERROR_CheckDisconnect( &bsw->entry_param ) == TRUE ){
			bsw->dis_error = TRUE;	// 切断エラー発生

			// 
		}
	}

	if( bsw->dis_error == TRUE ){
		// 切断エラー中で、サブプロックがない場合直ぐに終了
		if( bsw->sub_proc == NULL ){
			return TRUE;
		}
	}
#endif
	return FALSE;
}
