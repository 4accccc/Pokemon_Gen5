//==============================================================================
/**
 * @file	br_manager.c
 * @brief	バトルレコーダー
 * @author	goto
 * @date	2007.07.26(木)
 *
 * ここに色々な解説等を書いてもよい
 *
 */
//==============================================================================
#include <dwc.h>

#include "common.h"
#include "system/procsys.h"
#include "system/arc_tool.h"
#include "system/arc_util.h"
#include "system/pm_str.h"
#include "system/clact_tool.h"
#include "system/msgdata_util.h"
#include "system/wordset.h"
#include "msgdata/msg.naix"
#include "system/brightness.h"
#include "system/fontproc.h"
#include "system/lib_pack.h"
#include "gflib/msg_print.h"
#include "gflib/touchpanel.h"
#include "poketool/poke_tool.h"
#include "poketool/pokeicon.h"
#include "poketool/boxdata.h"
#include "system/snd_tool.h"
#include "gflib/strbuf_family.h"
#include "system/pm_overlay.h"
#include "system/wipe.h"

#include "src/field/syswork.h"
#include "src/field/sysflag.h"

#include "br_private.h"


//--------------------------------------------------------------
//	オーバーレイID
//--------------------------------------------------------------
FS_EXTERN_OVERLAY(gds_comm);


typedef struct {
	
	int				seq;
	int				state;			// 状態
	PROC*			p_subproc;		// サブプロセス
	
	SAVEDATA*		save;
	BATTLE_PARAM*	bp;
	

	BR_WORK*		br_work;		// バトルレコーダー用ワーク
	
	POST_MESSAGE	message;		// 連絡
	
	FIELDSYS_WORK*	fsys;
	
} BR_MANAGER_SYS;


enum {
	
	eBRM_BATTLE_RECORDER	= 0,		///< バトルレコーダー
	eBRM_BATTLE_REC_PLAYER,				///< 録画再生
};

//==============================================================
// Prototype
//==============================================================
static void ManagerInit_Common( PROC * proc, int mode );

static PROC_RESULT BR_Manager_Proc_Init( PROC * proc, int * seq );
static PROC_RESULT BR_Manager_Proc_Init_GDS( PROC * proc, int * seq );
static PROC_RESULT BR_Manager_Proc_Init_GDS_BV( PROC * proc, int * seq );
static PROC_RESULT BR_Manager_Proc_Init_GDS_BV_RANK( PROC * proc, int * seq );
static PROC_RESULT BR_Manager_Proc_Init_GDS_RANK( PROC * proc, int * seq );
static PROC_RESULT BR_Manager_Proc_Init_GDS_DRESS( PROC * proc, int * seq );
static PROC_RESULT BR_Manager_Proc_Init_GDS_BOX( PROC * proc, int * seq );

static PROC_RESULT BR_Manager_Proc_Main( PROC * proc, int * seq );
static PROC_RESULT BR_Manager_Proc_Exit( PROC * proc, int * seq );
static BOOL BRM_BattleRecorder( BR_MANAGER_SYS* wk, int heap );
static BOOL BRM_BattleRecPlayer( BR_MANAGER_SYS* wk, int heap );


//--------------------------------------------------------------
/**
 * @brief	子プロックから取得されるので、間違ってもmanager_procを渡さない事
 *
 * @param	proc	
 *
 * @retval	BR_WORK*	
 *
 */
//--------------------------------------------------------------
BR_WORK* BR_PROC_GetWork( PROC * proc )
{
	BR_WORK* ret_wk;
	
	BR_MANAGER_SYS* mana_wk = PROC_GetParentWork( proc );
	
	return mana_wk->br_work;
}


//--------------------------------------------------------------
/**
 * @brief	親への連絡
 *
 * @param	proc	
 * @param	bFlag	
 * @param	state	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void BR_PostMessage( POST_MESSAGE* message, BOOL bFlag, int state )
{
	message->bFlag = bFlag;
	message->state = state;	
}

//--------------------------------------------------------------
/**
 * @brief	ノートの設定
 *
 * @param	save	
 * @param	heap	
 * @param	id	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
static void GDS_NoteDataSet( SAVEDATA* save, int heap, u32 id )
{
	FNOTE_DATA *f_note = SaveData_GetFNote( save );
	void *buf = FNOTE_SioIDOnlyDataMake( heap, id );	
	FNOTE_DataSave( f_note, buf, FNOTE_TYPE_SIO );	
}

//--------------------------------------------------------------
/**
 * @brief	初期化共通部分
 *
 * @param	proc	
 * @param	mode	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
static void ManagerInit_Common( PROC * proc, int mode )
{
	BR_MANAGER_SYS* wk;
	
	wk = PROC_AllocWork( proc, sizeof( BR_MANAGER_SYS ), HEAPID_BASE_APP );
	MI_CpuFill8( wk, 0, sizeof( BR_MANAGER_SYS ) );
	
	sys_CreateHeap( HEAPID_BASE_APP, HEAPID_BR_MANAGER, 9000 );
	
	wk->fsys		= PROC_GetParentWork( proc );
	wk->save		= wk->fsys->savedata;
	wk->br_work		= sys_AllocMemory( HEAPID_BASE_APP, sizeof( BR_WORK ) );
	MI_CpuFill8( wk->br_work, 0, sizeof( BR_WORK ) );
	
	wk->br_work->message	= &wk->message;
	wk->br_work->save		= wk->save;
	
	wk->br_work->mode		= mode;
	wk->br_work->menu_data_mem[ wk->br_work->tag_man.stack_s ] = BattleRecorder_MenuDataGetEx( wk->br_work, wk->br_work->mode );
	
	{
		int id;
		switch( mode ){
		case BR_MODE_GDS_BV:
			id = FNOTE_ID_PL_GDS_VIDEO;
			break;
		case BR_MODE_GDS_BV_RANK:
			id = FNOTE_ID_PL_GDS_RANKING;
			break;
		case BR_MODE_GDS_RANK:
			id = FNOTE_ID_PL_GDS_RANKING;
			break;
		case BR_MODE_GDS_DRESS:
			id = FNOTE_ID_PL_GDS_DRESS;
			break;
		case BR_MODE_GDS_BOX:
			id = FNOTE_ID_PL_GDS_BOX;
			break;
		default:
			return;
		}		
		GDS_NoteDataSet( wk->save, HEAPID_BASE_APP, id );
	}
}

//--------------------------------------------------------------
/**
 * @brief	プロック初期化
 *
 * @param	proc	
 * @param	seq	
 *
 * @retval	static PROC_RESULT	
 *
 */
//--------------------------------------------------------------
static PROC_RESULT BR_Manager_Proc_Init( PROC * proc, int * seq )
{
	ManagerInit_Common( proc, BR_MODE_BROWSE );
	
	return PROC_RES_FINISH;
}

static PROC_RESULT BR_Manager_Proc_Init_GDS( PROC * proc, int * seq )
{
	ManagerInit_Common( proc, BR_MODE_GDS );
		
	return PROC_RES_FINISH;
}

static PROC_RESULT BR_Manager_Proc_Init_GDS_BV( PROC * proc, int * seq )
{
	ManagerInit_Common( proc, BR_MODE_GDS_BV );
		
	return PROC_RES_FINISH;
}

static PROC_RESULT BR_Manager_Proc_Init_GDS_BV_RANK( PROC * proc, int * seq )
{
	ManagerInit_Common( proc, BR_MODE_GDS_BV_RANK );
		
	return PROC_RES_FINISH;
}

static PROC_RESULT BR_Manager_Proc_Init_GDS_RANK( PROC * proc, int * seq )
{
	ManagerInit_Common( proc, BR_MODE_GDS_RANK );
		
	return PROC_RES_FINISH;
}

static PROC_RESULT BR_Manager_Proc_Init_GDS_DRESS( PROC * proc, int * seq )
{
	ManagerInit_Common( proc, BR_MODE_GDS_DRESS );
		
	return PROC_RES_FINISH;
}

static PROC_RESULT BR_Manager_Proc_Init_GDS_BOX( PROC * proc, int * seq )
{
	ManagerInit_Common( proc, BR_MODE_GDS_BOX );
		
	return PROC_RES_FINISH;
}


//--------------------------------------------------------------
/**
 * @brief	
 *
 * @param	proc	
 * @param	seq	
 *
 * @retval	static PROC_RESULT	
 *
 */
//--------------------------------------------------------------
static PROC_RESULT BR_Manager_Proc_Main( PROC * proc, int * seq )
{
	BOOL bEnd;
	BR_MANAGER_SYS* wk = PROC_GetWork( proc );
	
	switch ( *seq ){
	case eBRM_BATTLE_RECORDER:
	
		bEnd = BRM_BattleRecorder( wk, HEAPID_BASE_APP );
	
		if ( bEnd ){
			
			if ( wk->message.bFlag == TRUE ){
				*seq = eBRM_BATTLE_REC_PLAYER;
				wk->seq = 0;
			}
			else {
				return PROC_RES_FINISH;
			}
		}
		break;
		
	case eBRM_BATTLE_REC_PLAYER:
	
		bEnd = BRM_BattleRecPlayer( wk, HEAPID_BASE_APP );
		
		if ( bEnd ){
			*seq = eBRM_BATTLE_RECORDER;
			wk->seq = 0;
		}	
		break;
	}
	
	return PROC_RES_CONTINUE;
}


//--------------------------------------------------------------
/**
 * @brief	終了
 *
 * @param	proc	
 * @param	seq	
 *
 * @retval	static PROC_RESULT	
 *
 */
//--------------------------------------------------------------
static PROC_RESULT BR_Manager_Proc_Exit( PROC * proc, int * seq )
{
	BR_MANAGER_SYS* wk = PROC_GetWork( proc );

	///< 録画データの解放 特に問題なければ、確保しっぱなしで戦闘ルーチンをまわしたりするため
	if ( BattleRec_DataExistCheck() == TRUE ){
		BattleRec_Exit();
		OS_Printf( " ****** バトルレコーダー終了まで brs が開いていた\n" );
	}
	
	sys_FreeMemoryEz( wk->br_work );
	sys_FreeMemoryEz( wk );
	
	sys_DeleteHeap( HEAPID_BR_MANAGER );
	
	return PROC_RES_FINISH;
}


//--------------------------------------------------------------
/**
 * @brief	バトルレコーダー起動
 *
 * @param	wk	
 * @param	heap	
 *
 * @retval	static BOOL	
 *
 */
//--------------------------------------------------------------
static BOOL BRM_BattleRecorder( BR_MANAGER_SYS* wk, int heap )
{
	switch ( wk->seq ){
	case 0:
		if ( wk->br_work->mode == BR_MODE_BROWSE ){
			wk->p_subproc = PROC_Create( &BattleRecorder_BrowseProcData, wk, heap );
		}
		else {
			wk->p_subproc = PROC_Create( &BattleRecorder_GDSProcData, wk, heap );
		}
		wk->seq++;
		break;
	default:
		if( ProcMain( wk->p_subproc ) == FALSE ){
			break;
		}
		PROC_Delete( wk->p_subproc );
		
		return TRUE;
	}
	
	return FALSE;
}


//--------------------------------------------------------------
/**
 * @brief	バトル録画プレイ
 *
 * @param	wk	
 * @param	heap	
 *
 * @retval	static BOOL	
 *
 */
//--------------------------------------------------------------
static BOOL BRM_BattleRecPlayer( BR_MANAGER_SYS* wk, int heap )
{
	if ( wk->br_work->mode != BR_MODE_BROWSE ){	DWC_ProcessInet(); }	///< wi-fi接続維持のため(DWC_ProcessInet();)
	
	switch ( wk->seq ){
	case 0:
		if(wk->br_work->mode != BR_MODE_BROWSE){
			//戦闘画面に突入する為、オーバーレイが被っているgds_commは一時解放する
			Overlay_UnloadID(FS_OVERLAY_ID(gds_comm));	//2008.02.23(土) matsuda
			DpwCommonOverlayEnd();
		}
		
		if ( BattleRec_DataExistCheck() == FALSE )	///< すでに読まれているかチェック
		{
			LOAD_RESULT ret;			
			BattleRec_Init( wk->save, heap, &ret );			
			if ( ret != LOAD_RESULT_OK ){				
				GF_ASSERT( 0 );
			}
			else {
				OS_Printf( "バトルレコーダー 録画データあり\n" );
				BattleRec_Exit();
				wk->seq++;
			}
		}
		else {
			wk->seq++;
		}
		break;
	
	case 1:
		{
			LOAD_RESULT ret;
			//対戦録画データのロード
			
			wk->bp = BattleParam_Create( heap, FIGHT_TYPE_1vs1 );
			
			if ( BattleRec_DataExistCheck() == FALSE ){
				BattleRec_Load( wk->save, heap, &ret, wk->bp, wk->br_work->ex_param1 );
			}
			else {
				BattleRec_BattleParamCreate( wk->bp, wk->save );
			//	BattleRec_Load( wk->save, heap, &ret, wk->bp, 0 );
				ret = LOAD_RESULT_OK;
			}

			wk->bp->bag_cursor	= MyItem_BagCursorAlloc( heap );
			wk->bp->record		= SaveData_GetRecord( wk->save );
			
			if ( ret != LOAD_RESULT_OK ){
				BattleParam_Delete( wk->bp );		//BATTLE_PARAMの開放
				wk->seq = 0;
				return TRUE;
			}
			else {
				wk->seq++;
			}
		}
		break;
		
	case 2:
		///< トレーナーＩＤで曲を分ける
		{			
			if ( wk->bp->trainer_data[ 1 ].tr_type == TRTYPE_KUROTUGU
			||	 wk->bp->trainer_data[ 1 ].tr_type == TRTYPE_FBOSS1
			||	 wk->bp->trainer_data[ 1 ].tr_type == TRTYPE_FBOSS2
			||	 wk->bp->trainer_data[ 1 ].tr_type == TRTYPE_FBOSS3
			||	 wk->bp->trainer_data[ 1 ].tr_type == TRTYPE_FBOSS4 ){
				Snd_DataSetByScene( SND_SCENE_BATTLE, SEQ_PL_BA_BRAIN, 1 );	//バトル曲再生
				OS_Printf( "ブレーンの曲\n" );
			}
			else {
				Snd_DataSetByScene( SND_SCENE_BATTLE, SEQ_BA_TRAIN, 1 );	//バトル曲再生
				OS_Printf( "通常曲\n" );
			}
		}		
		wk->p_subproc = PROC_Create( &TestBattleProcData, wk->bp, heap );
		wk->seq++;
		break;
		
	default:
		if( ProcMain( wk->p_subproc ) == FALSE ){
			break;
		}
		sys_FreeMemoryEz( wk->bp->bag_cursor );
		BattleParam_Delete( wk->bp );		//BATTLE_PARAMの開放
//		BattleRec_Exit();					//解放しない。読み込んだものをそのまま使いたいので。
		PROC_Delete( wk->p_subproc );
		{
			u16 bgm_no;
			//サウンドデータセット(シーンが変更されない時は何もしない)
			Snd_SceneSet( SND_SCENE_DUMMY );
			bgm_no = Snd_FieldBgmNoGet( wk->fsys, wk->fsys->location->zone_id );
			Snd_ZoneBgmSet( Snd_FieldBgmNoGetNonBasicBank( wk->fsys, wk->fsys->location->zone_id ) );//zone set

			Snd_DataSetByScene( SND_SCENE_FIELD, bgm_no, 1 );
		}
		wk->seq = 0;
		
		if(wk->br_work->mode != BR_MODE_BROWSE){
			//gds_comm復帰
			DpwCommonOverlayStart();
			Overlay_Load(FS_OVERLAY_ID(gds_comm), OVERLAY_LOAD_NOT_SYNCHRONIZE);
		}
		return TRUE;
	}
	
	return FALSE;
}


// -----------------------------------------
//
//	□ プロック
//
// -----------------------------------------
const PROC_DATA BattleRecorder_ManagerProcData = {
	BR_Manager_Proc_Init,
	BR_Manager_Proc_Main,
	BR_Manager_Proc_Exit,
	NO_OVERLAY_ID,	
};

const PROC_DATA BattleRecorder_ManagerProcDataGDS = {
	BR_Manager_Proc_Init_GDS,
	BR_Manager_Proc_Main,
	BR_Manager_Proc_Exit,	
	NO_OVERLAY_ID,	
};

const PROC_DATA BattleRecorder_ManagerProcDataGDS_BV = {
	BR_Manager_Proc_Init_GDS_BV,
	BR_Manager_Proc_Main,
	BR_Manager_Proc_Exit,	
	NO_OVERLAY_ID,	
};

const PROC_DATA BattleRecorder_ManagerProcDataGDS_BV_RANK = {
	BR_Manager_Proc_Init_GDS_BV_RANK,
	BR_Manager_Proc_Main,
	BR_Manager_Proc_Exit,	
	NO_OVERLAY_ID,	
};

const PROC_DATA BattleRecorder_ManagerProcDataGDS_RANK = {
	BR_Manager_Proc_Init_GDS_RANK,
	BR_Manager_Proc_Main,
	BR_Manager_Proc_Exit,	
	NO_OVERLAY_ID,	
};

const PROC_DATA BattleRecorder_ManagerProcDataGDS_DRESS = {
	BR_Manager_Proc_Init_GDS_DRESS,
	BR_Manager_Proc_Main,
	BR_Manager_Proc_Exit,	
	NO_OVERLAY_ID,	
};

const PROC_DATA BattleRecorder_ManagerProcDataGDS_BOX = {
	BR_Manager_Proc_Init_GDS_BOX,
	BR_Manager_Proc_Main,
	BR_Manager_Proc_Exit,	
	NO_OVERLAY_ID,	
};


///< プロックテーブル
static const PROC_DATA* BR_ProcData_Table[] = {	
	&BattleRecorder_ManagerProcData,
	&BattleRecorder_ManagerProcDataGDS,
	&BattleRecorder_ManagerProcDataGDS_BV,
	&BattleRecorder_ManagerProcDataGDS_BV_RANK,
	&BattleRecorder_ManagerProcDataGDS_RANK,
	&BattleRecorder_ManagerProcDataGDS_DRESS,
	&BattleRecorder_ManagerProcDataGDS_BOX,
};

//--------------------------------------------------------------
/**
 * @brief	モード毎のプロックデータの取得
 *
 * @param	mode	
 *
 * @retval	const PROC_DATA*	
 *
 */
//--------------------------------------------------------------
const PROC_DATA* BattleRecoder_ProcDataGet( int mode )
{
	const PROC_DATA* pData;
	
	pData = BR_ProcData_Table[ mode ];
	
	return pData;
}


//--------------------------------------------------------------
/**
 * @brief	ゲームクリアしたか？
 *
 * @param	wk	
 *
 * @retval	BOOL	TRUE = クリア
 *
 */
//--------------------------------------------------------------
BOOL BR_IsGameClear( BR_WORK* wk )
{
	EVENTWORK* ev;
	
	ev = SaveData_GetEventWork( wk->save );
	
	return SysFlag_ArriveGet( ev, FLAG_ARRIVE_D32R0101 );
#if 0
	
	return SysFlag_GameClearCheck( ev );
#endif
}
