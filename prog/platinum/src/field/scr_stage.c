//============================================================================================
/**
 * @file	scr_stage.c
 * @bfief	スクリプトコマンド：バトルステージ(受付まわり)
 * @author	Satoshi Nohara
 * @date	06.06.13
 */
//============================================================================================
#include "common.h"

#include <nitro/code16.h> 
#include "system/pm_str.h"
#include "system\msgdata.h"			//MSGMAN_GetString
#include "system\wordset.h"			//WordSet_RegistPlayerName
#include "system/bmp_menu.h"
#include "system/bmp_list.h"
#include "system/get_text.h"
#include "system/lib_pack.h"
#include "poketool/poke_number.h"	//PMNumber_GetMode
#include "savedata/sp_ribbon.h"		//SaveData_GetSpRibbon
#include "savedata/frontier_savedata.h"
#include "gflib/strbuf_family.h"	//許可制
//フィールド
#include "fieldsys.h"
#include "field_subproc.h"
//スクリプト
#include "script.h"
#include "scrcmd.h"
#include "scrcmd_def.h"
#include "field/eventflag.h"
#include "field/evwkdef.h"
#include "sysflag.h"
#include "syswork.h"
#include "scr_tool.h"
//バトルステージ
#include "scr_stage.h"
#include "scr_stage_sub.h"
#include "savedata/stage_savedata.h"
#include "../frontier/stage_def.h"
//通信
#include "communication/comm_def.h"
#include "communication/comm_tool.h"
#include "communication/comm_system.h"

//#include "../frontier/comm_command_frontier.h"
#include "comm_command_field.h"
#include "frontier/frontier_tool.h"


//============================================================================================
//
//	構造体宣言
//
//============================================================================================
///ポケモン選択イベントワーク
typedef struct _STAGE_POKESEL_EVENT{
	int	seq;
	u8	plist_type;							//ポケモンリストタイプ
	u8	pos;								//ポケモンリストの現在カーソル位置
	u8	sel[STAGE_ENTRY_POKE_MAX];
	void** sp_wk;
}STAGE_POKESEL_EVENT;

///ポケモン選択イベントシーケンスID
typedef enum{
	STAGE_POKESEL_PLIST_CALL,				//ポケモンリスト呼び出し
	STAGE_POKESEL_PLIST_WAIT,				//ポケモンリスト終了待ち
	STAGE_POKESEL_PST_CALL,					//ポケモンステータス呼び出し
	STAGE_POKESEL_PST_WAIT,					//ポケモンステータス終了待ち
	STAGE_POKESEL_EXIT,						//終了
};


//============================================================================================
//
//	プロトタイプ宣言
//
//============================================================================================
BOOL EvCmdBattleStageTools(VM_MACHINE* core);
static BOOL StageScrTools_CheckEntryPokeNum(u16 num,SAVEDATA *savedata);
BOOL EvCmdBattleStageSetContinueNG( VM_MACHINE * core );

BOOL EvCmdBattleStageCommMonsNo(VM_MACHINE* core);
void EventCall_StageComm( GMEVENT_CONTROL* event, u16 monsno, u16* ret_wk );
static BOOL GMEVENT_StageComm( GMEVENT_CONTROL* event );

//ポケモンリスト＆ポケモンステータス
void EventCmd_StagePokeSelectCall( GMEVENT_CONTROL *event, void** buf, u8 plist_type );
static BOOL BtlStageEv_PokeSelect( GMEVENT_CONTROL *ev );
static int BtlStage_PokeListCall( STAGE_POKESEL_EVENT* wk,FIELDSYS_WORK* fsys,int heapID );
static int BtlStage_PokeListWait( STAGE_POKESEL_EVENT* wk, FIELDSYS_WORK* fsys );
static int BtlStage_PokeStatusCall(STAGE_POKESEL_EVENT* wk,FIELDSYS_WORK* fsys,int heapID );
static int BtlStage_PokeStatusWait( STAGE_POKESEL_EVENT* wk, FIELDSYS_WORK* fsys );


//============================================================================================
//
//	コマンド
//
//============================================================================================

//--------------------------------------------------------------
/**
 *	@brief	バトルステージ用コマンド群呼び出しインターフェース
 *
 *	@param	com_id		u16:コマンドID
 *	@param	retwk_id	u16:返り値を格納するワークのID
 */
//--------------------------------------------------------------
BOOL EvCmdBattleStageTools(VM_MACHINE* core)
{
	u8 plist_type;
	u8 buf8[4];
	u16 buf16[4];
	u16	com_id,param,retwk_id;
	u16* ret_wk,*prm_wk;
	void** buf;
	BTOWER_SCRWORK* wk;
	STAGEDATA* stage_sv;
	STAGESCORE* score_sv;

	com_id	= VMGetU16(core);
	param	= VMGetWorkValue(core);
	retwk_id= VMGetU16(core);
	ret_wk	= GetEventWorkAdrs( core->fsys, retwk_id );

	stage_sv= SaveData_GetStageData( core->fsys->savedata );
	score_sv= SaveData_GetStageScore( core->fsys->savedata );
	buf		= GetEvScriptWorkMemberAdrs( core->fsys, ID_EVSCR_SUBPROC_WORK );

	switch( com_id ){

	//0:参加可能なポケモン数のチェック(シングル専用な処理になっている)
	case STAGE_TOOL_CHK_ENTRY_POKE_NUM:
		*ret_wk = StageScrTools_CheckEntryPokeNum( param, core->fsys->savedata );
		break;

	//1:連勝中か取得
	case STAGE_TOOL_GET_CLEAR_FLAG:
		//WIFIのみ特殊
		if( param == STAGE_TYPE_WIFI_MULTI ){
			*ret_wk = FrontierRecord_Get(SaveData_GetFrontier(core->fsys->savedata), 
										FRID_STAGE_MULTI_WIFI_CLEAR_BIT,
										Frontier_GetFriendIndex(FRID_STAGE_MULTI_WIFI_CLEAR_BIT) );
		}else{
			*ret_wk = (u16)STAGESCORE_GetScoreData( score_sv, STAGESCORE_ID_CLEAR_FLAG, 
													param, 0, NULL);
		}
		break;

	//2:連勝中のポケモンナンバーを取得
	case STAGE_TOOL_GET_CLEAR_MONSNO:
		*ret_wk = FrontierRecord_Get(SaveData_GetFrontier(core->fsys->savedata), 
									StageScr_GetMonsNoRecordID(param),
									Frontier_GetFriendIndex(StageScr_GetMonsNoRecordID(param)) );
		break;

	//3:連勝中フラグクリア、連勝レコードクリア、タイプカウントクリア
	case STAGE_TOOL_SET_NEW_CHALLENGE:
		BattleStageSetNewChallenge( core->fsys->savedata, score_sv, param );
		break;

	//4:ポケモンリスト画面呼び出し
	case STAGE_TOOL_SELECT_POKE:

		//バトルタイプによって分岐
		if( param == STAGE_TYPE_SINGLE ){
			plist_type = PL_TYPE_SINGLE;
		}else if( param == STAGE_TYPE_DOUBLE ){
			plist_type = PL_TYPE_DOUBLE;
		}else if( param == STAGE_TYPE_MULTI ){
			plist_type = PL_TYPE_MULTI;
		}else{
			plist_type = PL_TYPE_MULTI;
		}

		OS_Printf( "stage_type = %d\n", param );
		EventCmd_StagePokeSelectCall( core->event_work, buf, plist_type );
		return 1;

	default:
		OS_Printf( "渡されたcom_id = %d\n", com_id );
		GF_ASSERT( (0) && "com_idが未対応です！" );
		*ret_wk = 0;
		break;
	}

	return 0;
}

//--------------------------------------------------------------
/**
 *	@brief	バトルステージ　参加可能なポケモン数のチェック
 *
 *	@param	num		参加に必要なポケモン数
 *
 *	@retval	true	参加可能
 *	@retval	false	参加不可
 *
 *	タマゴでなくて、伝説系以外が何匹いるか
 */
//--------------------------------------------------------------
static BOOL StageScrTools_CheckEntryPokeNum(u16 num,SAVEDATA *savedata)
{
	u8 i,j,ct,pokenum;
	u16	monsno;
	POKEPARTY* party;
	POKEMON_PARAM* pp;
	
	party	= SaveData_GetTemotiPokemon( savedata );
	pokenum = PokeParty_GetPokeCount( party );

	//手持ちの数より必要なポケモン数が多い時
	if( pokenum < num ){
		return FALSE;
	}

	for( i=0, ct=0; i < pokenum ;i++ ){
		pp		= PokeParty_GetMemberPointer( party, i );
		monsno	= PokeParaGet( pp, ID_PARA_monsno, NULL );		//ポケモンナンバー
		
		//タマゴでないかチェック
		if( PokeParaGet(pp,ID_PARA_tamago_flag,NULL) != 0 ){
			continue;
		}

		//バトルタワーに参加できないポケモンかをチェック
		if( BattleTowerExPokeCheck_MonsNo(monsno) == TRUE ){
			continue;
		}

		ct++;
	}

	//条件を通過したポケモン数をチェック
	if( ct < num ){
		return FALSE;
	}

	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		スクリプトコマンド：バトルステージ継続NGセット
 * @param		core
 */
//--------------------------------------------------------------------------------------------
BOOL EvCmdBattleStageSetContinueNG( VM_MACHINE * core )
{
	STAGESCORE* score_wk;
	u16 type = VMGetWorkValue(core);

	score_wk = SaveData_GetStageScore( core->fsys->savedata );

	BattleStageSetNewChallenge( core->fsys->savedata, score_wk, type );
	return 0;
}


//============================================================================================
//
//	通信
//
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 *	@brief	通信マルチデータ送信
 */
//--------------------------------------------------------------------------------------------
BOOL EvCmdBattleStageCommMonsNo(VM_MACHINE* core)
{
	u16 monsno	= VMGetWorkValue( core );
	u16* ret_wk	= VMGetWork( core );

	OS_Printf( "バトルステージ通信マルチデータ送信\n" );
	OS_Printf( "ポケモンナンバー monsno = %d\n", monsno );
	
	EventCall_StageComm( core->event_work, monsno, ret_wk );
	return 1;
}
	
//--------------------------------------------------------------------------------------------
/**
 * @brief	イベント擬似コマンド：送受信
 *
 * @param	event		イベント制御ワークへのポインタ
 * @param	monsno		ポケモンナンバー
 */
//--------------------------------------------------------------------------------------------
void EventCall_StageComm( GMEVENT_CONTROL* event, u16 monsno, u16* ret_wk )
{
	FLDSCR_STAGE_COMM* comm_wk;

	comm_wk = sys_AllocMemory( HEAPID_WORLD, sizeof(FLDSCR_STAGE_COMM) );
	memset( comm_wk, 0, sizeof(FLDSCR_STAGE_COMM) );

	comm_wk->mine_monsno = monsno;
	comm_wk->ret_wk = ret_wk;
	//CommCommandFrontierInitialize( comm_wk );
	CommCommandFieldInitialize( comm_wk );

	FieldEvent_Call( event, GMEVENT_StageComm, comm_wk );
	return;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	送受信
 *
 * @param	event		GMEVENT_CONTROL型
 *
 * @retval	"FALSE = 実行中"
 * @retval	"TRUE = 実行終了"
 */
//--------------------------------------------------------------------------------------------
static BOOL GMEVENT_StageComm( GMEVENT_CONTROL* event )
{
	FLDSCR_STAGE_COMM* comm_wk = FieldEvent_GetSpecialWork( event );

	switch( comm_wk->seq ){

	case 0:
		if( CommSendData(CF_STAGE_MONSNO,comm_wk,sizeof(FLDSCR_STAGE_COMM)) == TRUE ){
			comm_wk->seq++;
		}
		break;
		
	case 1:
		if( comm_wk->recieve_count >= STAGE_COMM_PLAYER_NUM ){

			//同じポケモンを選んでいるかチェック
			if( comm_wk->mine_monsno == comm_wk->pair_monsno ){
				*comm_wk->ret_wk = 0;
			}else{
				*comm_wk->ret_wk = 1;
			}

			OS_Printf( "comm_wk->mine_monsno = %d\n", comm_wk->mine_monsno );
			OS_Printf( "comm_wk->pair_monsno = %d\n", comm_wk->pair_monsno );
			OS_Printf( "*comm_wk->ret_wk = %d\n", *comm_wk->ret_wk );
			comm_wk->seq++;
		}
		break;

	case 2:
		sys_FreeMemoryEz( comm_wk );
		return TRUE;
	};

	return FALSE;
}


//============================================================================================
//
//	ポケモンリスト＆ポケモンステータス
//
//============================================================================================

//--------------------------------------------------------------
/**
 * @brief	ポケモン選択　サブイベント呼び出し
 *
 * @param	event	GMEVENT_CONTROL*
 *
 * @retval	nothing
 */
//--------------------------------------------------------------
void EventCmd_StagePokeSelectCall( GMEVENT_CONTROL *event, void** buf, u8 plist_type )
{
	FIELDSYS_WORK* fsys		= FieldEvent_GetFieldSysWork( event );
	STAGE_POKESEL_EVENT* wk = sys_AllocMemory( HEAPID_WORLD, sizeof(STAGE_POKESEL_EVENT) );
	MI_CpuClear8( wk, sizeof(STAGE_POKESEL_EVENT) );

	wk->plist_type	= plist_type;
	wk->sp_wk		= buf;

	FieldEvent_Call( fsys->event, BtlStageEv_PokeSelect, wk );
	return;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	ゲームイベントコントローラ　ポケモンリスト＆ステータス
 *
 * @param	ev	GMEVENT_CONTROL *
 *
 * @retval	BOOL	TRUE=イベント終了
 */
//--------------------------------------------------------------------------------------------
static BOOL BtlStageEv_PokeSelect( GMEVENT_CONTROL *ev )
{
	FIELDSYS_WORK* fsys		= FieldEvent_GetFieldSysWork( ev );
	STAGE_POKESEL_EVENT* wk	= FieldEvent_GetSpecialWork( ev );

	switch( wk->seq ){

	//ポケモンリスト呼び出し
	case STAGE_POKESEL_PLIST_CALL:
		wk->seq = BtlStage_PokeListCall( wk, fsys, HEAPID_WORLD );
		break;

	//ポケモンリスト終了待ち
	case STAGE_POKESEL_PLIST_WAIT:
		wk->seq = BtlStage_PokeListWait( wk, fsys );
		break;

	//ポケモンステータス呼び出し
	case STAGE_POKESEL_PST_CALL:
		wk->seq = BtlStage_PokeStatusCall( wk, fsys, HEAPID_WORLD );
		break;

	//ポケモンステータス終了待ち
	case STAGE_POKESEL_PST_WAIT:
		wk->seq = BtlStage_PokeStatusWait( wk, fsys );
		break;

	//終了
	case STAGE_POKESEL_EXIT:
		sys_FreeMemoryEz( wk );		//STAGE_POKESEL_EVENTを開放
		return TRUE;
	}

	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 *	@brief	バトルステージ　ポケモンリスト呼び出し
 */
//--------------------------------------------------------------------------------------------
static int BtlStage_PokeListCall( STAGE_POKESEL_EVENT* wk,FIELDSYS_WORK* fsys,int heapID )
{
	u8 i;

	//PLIST_DATA* pld = PLISTDATA_Create(HEAPID_WORLD, fsys, wk->plist_type, PL_MODE_BATTLE_STAGE);
	PLIST_DATA* pld = sys_AllocMemory( HEAPID_WORLD, sizeof(PLIST_DATA) );
	MI_CpuClearFast( pld, sizeof(PLIST_DATA) );

	//PILSTDATA_Createの中身と同じ
	pld->pp			= SaveData_GetTemotiPokemon( fsys->savedata );
	pld->myitem		= SaveData_GetMyItem( fsys->savedata );
	pld->mailblock	= SaveData_GetMailBlock( fsys->savedata );
	pld->cfg		= SaveData_GetConfig( fsys->savedata );

	//タイプはシングル固定でよさそう(確認する)
	//pld->type		= wk->plist_type;
	pld->type		= PL_TYPE_SINGLE;

	pld->mode		= PL_MODE_BATTLE_STAGE;
	pld->fsys		= fsys;

	//カーソル位置
	pld->ret_sel	= wk->pos;

	//選択している位置(リスト→ステータス→リストで状態を復帰させる)
	for( i=0; i < STAGE_ENTRY_POKE_MAX ;i++ ){
		pld->in_num[i] = wk->sel[i];
	}

	pld->in_min		= 1;			//参加最小数
	pld->in_max		= 1;			//参加最大数

	if( wk->plist_type == PL_TYPE_DOUBLE ){
		pld->in_min = 2;			//参加最小数
		pld->in_max = 2;			//参加最大数
	}

	GameSystem_StartSubProc( fsys, &PokeListProcData, pld );

	*(wk->sp_wk) = pld;

	return STAGE_POKESEL_PLIST_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 *	@brief	ポケモンリスト終了待ち
 */
//--------------------------------------------------------------------------------------------
static int BtlStage_PokeListWait( STAGE_POKESEL_EVENT* wk, FIELDSYS_WORK* fsys )
{
	int	ret;
	PLIST_DATA* pld;

	// サブプロセス終了待ち
	if( FieldEvent_Cmd_WaitSubProcEnd(fsys) ) {
		return STAGE_POKESEL_PLIST_WAIT;
	}

	pld = *(wk->sp_wk);

	//データ取得
	switch( pld->ret_sel ){

	case PL_SEL_POS_EXIT:					//やめる(pldは開放していない)
		return STAGE_POKESEL_EXIT;

	case PL_SEL_POS_ENTER:					//決定(pldは開放していない)
		return STAGE_POKESEL_EXIT;

	default:								//つよさをみる
		break;
	}

	//選択している状態を、ポケモンステータスを呼んだあとに復帰させるために必要
	MI_CpuCopy8( pld->in_num, wk->sel, STAGE_ENTRY_POKE_MAX );	//現在選ばれているポケモンを保存

	//ポジションを保存
	wk->pos = pld->ret_sel;

	sys_FreeMemoryEz(pld);
	*(wk->sp_wk) = NULL;
	return	STAGE_POKESEL_PST_CALL;
}

//--------------------------------------------------------------------------------------------
/**
 *	@brief	バトルステージ　ポケモンステータス呼び出し
 */
//--------------------------------------------------------------------------------------------
static int BtlStage_PokeStatusCall(STAGE_POKESEL_EVENT* wk,FIELDSYS_WORK* fsys,int heapID )
{
	PSTATUS_DATA* psd;
	SAVEDATA* sv;

	static const u8 PST_PageTbl[] = {
		PST_PAGE_INFO,			//「ポケモンじょうほう」
		PST_PAGE_MEMO,			//「トレーナーメモ」
		PST_PAGE_PARAM,			//「ポケモンのうりょく」
		PST_PAGE_CONDITION,		//「コンディション」
		PST_PAGE_B_SKILL,		//「たたかうわざ」
		PST_PAGE_C_SKILL,		//「コンテストわざ」
		PST_PAGE_RIBBON,		//「きねんリボン」
		PST_PAGE_RET,			//「もどる」
		PST_PAGE_MAX
	};
	
	sv = fsys->savedata;

	//ポケモンステータスを呼び出す
	psd = sys_AllocMemoryLo( heapID, sizeof(PSTATUS_DATA) );
	MI_CpuClear8( psd,sizeof(PSTATUS_DATA) );

	psd->cfg		= SaveData_GetConfig( sv );
	psd->ppd		= SaveData_GetTemotiPokemon( sv );
	psd->zukan_mode	= PMNumber_GetMode( sv );
	psd->ev_contest	= PokeStatus_ContestFlagGet( sv );
	psd->ppt		= PST_PP_TYPE_POKEPARTY;
	psd->pos		= wk->pos;
	psd->max		= (u8)PokeParty_GetPokeCount( psd->ppd );
	psd->waza		= 0;
	psd->mode		= PST_MODE_NORMAL;
	psd->ribbon		= SaveData_GetSpRibbon( sv );

	PokeStatus_PageSet( psd, PST_PageTbl );
	PokeStatus_PlayerSet( psd, SaveData_GetMyStatus(sv) );
	
	GameSystem_StartSubProc( fsys, &PokeStatusProcData, psd );
	
	*(wk->sp_wk) = psd;
	return STAGE_POKESEL_PST_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 *	@brief	バトルステージ　ポケモンステータス終了待ち
 */
//--------------------------------------------------------------------------------------------
static int BtlStage_PokeStatusWait( STAGE_POKESEL_EVENT* wk, FIELDSYS_WORK* fsys )
{
	PSTATUS_DATA* psd;

	//サブプロセス終了待ち
	if( FieldEvent_Cmd_WaitSubProcEnd(fsys) ) {
		return STAGE_POKESEL_PST_WAIT;
	}

	psd = *(wk->sp_wk);
	
	//切り替えられたカレントを保存する
	wk->pos = psd->pos;

	sys_FreeMemoryEz( psd );
	*(wk->sp_wk) = NULL;

	return STAGE_POKESEL_PLIST_CALL;
}


