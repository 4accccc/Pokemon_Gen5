//******************************************************************************
/**
 * 
 * @file	guru2.c
 * @brief	ぐるぐる交換
 * @author	kagaya
 * @data	05.07.13
 *
 */
//******************************************************************************
#include "guru2_local.h"

#include "..\..\field\fieldsys.h"
#include "..\..\field\field_subproc.h"

//==============================================================================
//	define
//==============================================================================
//--------------------------------------------------------------
///	管理シーケンス
//--------------------------------------------------------------
enum
{
	SEQNO_G2P_INIT = 0,
	SEQNO_G2P_RECEIPT,
	SEQNO_G2P_SELECT,
	SEQNO_G2P_STATUS,
	SEQNO_G2P_MAIN,
	SEQNO_G2P_CANCEL,
	SEQNO_G2P_OYA_CANCEL_START,
	SEQNO_G2P_OYA_CANCEL_TIMING_WAIT,
	SEQNO_G2P_END,
};

//==============================================================================
//	struct
//==============================================================================
//--------------------------------------------------------------
///	GURU2_CALL_WORK
//--------------------------------------------------------------
typedef struct
{
	int seq_no;
	GURU2_PARAM param;
	GURU2PROC_WORK *g2p;
	FIELDSYS_WORK *fsys;
	PLIST_DATA *plist;
	PSTATUS_DATA *psd;
}GURU2_CALL_WORK;

//==============================================================================
//	proto
//==============================================================================
static const PROC_DATA Guru2Receipt_Proc;
static const PROC_DATA Guru2Main_Proc;

//==============================================================================
//	ぐるぐる交換
//==============================================================================
//--------------------------------------------------------------
/**
 * ぐるぐる交換　プロセス　初期化
 * @param	param	GURU2_PARAM
 * @param	heap_id	ヒープID
 * @retval	GURU2PROC_WORK	GURU2PROC_WORK
 */
//--------------------------------------------------------------
GURU2PROC_WORK * Guru2_ProcInit( GURU2_PARAM *param, u32 heap_id )
{
	GURU2PROC_WORK *g2p;
	
	g2p = sys_AllocMemory( heap_id, sizeof(GURU2PROC_WORK) );
	memset( g2p, 0, sizeof(GURU2PROC_WORK) );
	
	//外部データセット
	g2p->param = *param;
	
	//通信初期化
	g2p->g2c = Guru2Comm_WorkInit( g2p, heap_id );
	
	return( g2p );
}

//--------------------------------------------------------------
/**
 * ぐるぐる交換　プロセス　終了
 * @param	g2p		GURU2PROC_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
void Guru2_ProcDelete( GURU2PROC_WORK *g2p )
{
	//ワーク反映
	
	//ユニオンルーム通信へ
	Guru2Comm_CommUnionRoomChange( g2p->g2c );
	
	//通信ワーク削除
	Guru2Comm_WorkDelete( g2p->g2c );
	
	//リソース開放
	sys_FreeMemoryEz( g2p );
}

//==============================================================================
//	パーツ
//==============================================================================
//--------------------------------------------------------------
/**
 * 受付終了　戻り値チェック
 * @param	g2p	GURU2PROC_WORK *
 * @retval	BOOL	TRUE=継続 FALSE=終了
 */
//--------------------------------------------------------------
BOOL Guru2_ReceiptRetCheck( GURU2PROC_WORK *g2p )
{
	return( g2p->receipt_ret );
}

//==============================================================================
//	イベント
//==============================================================================
//--------------------------------------------------------------
/**
 * ぐるぐる交換処理用ワーク作成
 * @param	FIELDSYS_WORK *fsys
 * @retval	void*	
 */
//--------------------------------------------------------------
void * EventCmd_Guru2ProcWorkAlloc( FIELDSYS_WORK *fsys )
{
	GURU2_CALL_WORK *work;
	
	work = sys_AllocMemory( HEAPID_WORLD, sizeof(GURU2_CALL_WORK) );
	memset( work, 0, sizeof(GURU2_CALL_WORK) );
	
	work->fsys = fsys;
	work->param.sv = fsys->savedata;
	work->param.union_view = fsys->union_view;
	work->param.config = SaveData_GetConfig( fsys->savedata );
	work->param.record = SaveData_GetRecord( fsys->savedata );
	work->param.win_type = CONFIG_GetWindowType( work->param.config );
	work->param.fsys = fsys;
	return( work );
}


//--------------------------------------------------------------
/**
 * ぐるぐる交換イベント処理
 * @param	wk		EventCmd_Guru2ProcWorkAlloc()の戻り値　終了時開放
 * @retval	BOOL	TRUE=終了
 */
//--------------------------------------------------------------
BOOL EventCmd_Guru2Proc( void *wk )
{
	GURU2_CALL_WORK *work = wk;
	FIELDSYS_WORK *fsys = work->fsys;
	
	switch( work->seq_no ){
	case SEQNO_G2P_INIT:	//初期化
		work->g2p = Guru2_ProcInit( &work->param, HEAPID_WORLD );
		work->seq_no = SEQNO_G2P_RECEIPT;
		
		GameSystem_StartSubProc( fsys, &Guru2Receipt_Proc, work->g2p );
		break;
	case SEQNO_G2P_RECEIPT:	//受付
		if( FieldEvent_Cmd_WaitSubProcEnd(fsys) ){
			break;
		}
		
		if( Guru2_ReceiptRetCheck(work->g2p) == FALSE ){
			work->seq_no = SEQNO_G2P_END;
			break;
		}
		
		work->plist = Guru2ListEvent_SetProc( fsys );
		work->g2p->guru2_mode = GURU2MODE_POKESEL;
		work->seq_no = SEQNO_G2P_SELECT;
		break;
	case SEQNO_G2P_SELECT: //ポケモン選択
		if( FieldEvent_Cmd_WaitSubProcEnd(fsys) ){
			break;
		}
		
		{
			int ret = work->plist->ret_sel;
			
			sys_FreeMemoryEz( work->plist );		//リストワーク開放
			
			if( ret == PL_SEL_POS_EXIT ){			//キャンセル
				work->seq_no = SEQNO_G2P_CANCEL;
				break;
			}
			
			if( work->plist->ret_mode == PL_RET_STATUS ){	//ステータス閲覧
				work->psd = PSTATUS_Temoti_Create(
					fsys, HEAPID_BASE_APP, PST_MODE_NORMAL );
				work->psd->pos = ret;
				FieldPokeStatus_SetProc( fsys, work->psd );	
				work->seq_no = SEQNO_G2P_STATUS;
				break;
			}
			
			work->g2p->trade_no = ret;
		}
		
		GameSystem_StartSubProc( fsys, &Guru2Main_Proc, work->g2p );
		work->g2p->guru2_mode = GURU2MODE_GAME_MAIN;
		work->seq_no = SEQNO_G2P_MAIN;
		break;
	case SEQNO_G2P_STATUS:	//ポケモンステータス
		if( FieldEvent_Cmd_WaitSubProcEnd(fsys) ){
			break;
		}
		
		sys_FreeMemoryEz( work->psd );
		work->plist = Guru2ListEvent_SetProc( fsys );
		work->seq_no = SEQNO_G2P_SELECT;
		break;
	case SEQNO_G2P_MAIN:	//ぐるぐるメイン
		if( FieldEvent_Cmd_WaitSubProcEnd(fsys) ){
			break;
		}
		
		work->seq_no = SEQNO_G2P_END;
		break;
	case SEQNO_G2P_CANCEL:	//キャンセル
		if( CommGetCurrentID() == 0 ){	//親
			if( Guru2Comm_PokeSelCancelCheck(work->g2p->g2c) ){ //全キャンセル
				work->seq_no = SEQNO_G2P_END;
			}else{
				u16 bit = G2COMM_GMSBIT_CANCEL;
				Guru2Comm_SendData(
					work->g2p->g2c, G2COMM_GM_SIGNAL, &bit, 2 );
				work->g2p->guru2_mode = GURU2MODE_POKESEL_CANCEL;
				work->seq_no = SEQNO_G2P_OYA_CANCEL_START;
			}
		}else{
			int oya_end = work->g2p->g2c->comm_psel_oya_end_flag;
			
			if( oya_end == G2C_OYA_END_FLAG_NON ){
				Guru2Comm_SendData(
					work->g2p->g2c, G2COMM_GM_CANCEL, NULL, 0 );
				work->g2p->guru2_mode = GURU2MODE_POKESEL_CANCEL;
				work->seq_no = SEQNO_G2P_END;
				break;
			}
			
			if( oya_end == G2C_OYA_END_FLAG_EXE ){	//親切断実行中
				break;
			}
			
			work->seq_no = SEQNO_G2P_END;	//親切断済み
		}
		
		break;
	case SEQNO_G2P_OYA_CANCEL_START:		//親キャンセル相手受け取り待ち
		CommTimingSyncStart( COMM_GURU2_TIMINGSYNC_NO );
		work->seq_no = SEQNO_G2P_OYA_CANCEL_TIMING_WAIT;
		break;
	case SEQNO_G2P_OYA_CANCEL_TIMING_WAIT:	//親キャンセル通信終了待ち
		if( CommIsTimingSync(COMM_GURU2_TIMINGSYNC_NO) ){
			work->seq_no = SEQNO_G2P_END;
		}
		break;
	case SEQNO_G2P_END:		//終了
		Guru2_ProcDelete( work->g2p );
		sys_FreeMemoryEz( wk );
		return( TRUE );
	}
	
	return( FALSE );
}

//==============================================================================
//	data
//==============================================================================
// オーバーレイID宣言
FS_EXTERN_OVERLAY(guru2);

//--------------------------------------------------------------
///	ぐるぐる受付プロセスデータ
//--------------------------------------------------------------
static const PROC_DATA Guru2Receipt_Proc = {
	Guru2Receipt_Init,
	Guru2Receipt_Main,
	Guru2Receipt_End,
	FS_OVERLAY_ID(guru2),
};

//--------------------------------------------------------------
///	ぐるぐるメインプロセスデータ
//--------------------------------------------------------------
static const PROC_DATA Guru2Main_Proc = {
	Guru2Main_Init,
	Guru2Main_Main,
	Guru2Main_End,
	FS_OVERLAY_ID(guru2),
};

