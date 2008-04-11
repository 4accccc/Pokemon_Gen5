//============================================================================================
/**
 * @file	fssc_factory.c
 * @bfief	フロンティアシステムスクリプトコマンド：ファクトリー
 * @author	Satoshi Nohara
 * @date	07.04.10
 */
//============================================================================================
#include "common.h"
#include <nitro/code16.h> 
#include "system/pm_str.h"
#include "system\msgdata.h"						//MSGMAN_GetString
#include "system/brightness.h"					//ChangeBrightnessRequest
#include "system\wordset.h"						//WordSet_RegistPlayerName
#include "system/bmp_menu.h"
#include "system/bmp_list.h"
#include "system/get_text.h"
#include "system/window.h"
#include "system/fontproc.h"
#include "system/wipe.h"
#include "system/lib_pack.h"
#include "system/procsys.h"
#include "system/snd_tool.h"

#include "savedata/misc.h"
#include "savedata/b_tower.h"
#include "msgdata\msg.naix"						//NARC_msg_??_dat

#include "field/eventflag.h"
#include "field/evwkdef.h"
#include "gflib/strbuf_family.h"				//許可制

#include "../field/field_battle.h"				//BattleParam_IsWinResult
#include "../field/field_subproc.h"				//TestBattleProcData
#include "../application/factory.h"

#include "frontier_types.h"
#include "frontier_main.h"
#include "frontier_map.h"
#include "frontier_scr.h"
#include "frontier_tool.h"
#include "frontier_scrcmd.h"
#include "frontier_scrcmd_sub.h"
#include "factory_tool.h"
#include "factory_def.h"
#include "fss_task.h"
#include "fssc_factory_sub.h"
#include "comm_command_frontier.h"

#include "field/eventflag.h"

#include "graphic/frontier_bg_def.h"


//============================================================================================
//
//	定義
//
//============================================================================================
#define FACTORY_SCROLL_MAP_RAIL_Y	(1)			//マップのレールスクロール速度(256にあうようにする)
#define FACTORY_RAIL_TCB_PRI		(5)			//マップのレールTCBのプライオリティ


//============================================================================================
//
//	プロトタイプ宣言
//
//============================================================================================
BOOL FSSC_FactoryWorkAlloc( FSS_TASK * core );
BOOL FSSC_FactoryWorkInit( FSS_TASK * core );
BOOL FSSC_FactoryWorkFree( FSS_TASK * core );
BOOL FSSC_FactoryRentalCall( FSS_TASK * core );
BOOL FSSC_FactoryCallGetResult( FSS_TASK * core );
BOOL FSSC_FactoryBattleCall( FSS_TASK * core );
BOOL FSSC_FactoryTradeCall( FSS_TASK * core );
BOOL FSSC_FactoryRentalPokePartySet( FSS_TASK * core );
BOOL FSSC_FactoryBtlAfterPokePartySet( FSS_TASK * core );
BOOL FSSC_FactoryTradePokeChange( FSS_TASK * core );
BOOL FSSC_FactoryTradeAfterPokePartySet( FSS_TASK * core );
BOOL FSSC_FactoryScrWork( FSS_TASK * core );
BOOL FSSC_FactoryLoseCheck( FSS_TASK * core );
BOOL FSSC_FactorySendBuf( FSS_TASK * core );
BOOL FSSC_FactoryRecvBuf( FSS_TASK * core );
static BOOL WaitFactoryRecvBuf( FSS_TASK * core );
void Factory_MapRailMoveMain( TCB_PTR tcb, void * work );
BOOL FSSC_FactoryTalkMsgAppear(FSS_TASK* core);

//渡したワークの解放処理
static void FactoryFreeMemory( void *parent_work );


//============================================================================================
//
//	コマンド
//
//============================================================================================

//--------------------------------------------------------------
/**
 * @brief	ファクトリーワーク確保
 *
 * @param	core	仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
 */
//--------------------------------------------------------------
BOOL FSSC_FactoryWorkAlloc( FSS_TASK * core ) 
{
	FACTORY_SCRWORK* wk;
	FRONTIER_EX_PARAM* ex_param;
	u16 init = FSSTGetWorkValue( core );
	u16 type = FSSTGetWorkValue( core );
	u16 level = FSSTGetWorkValue( core );

	ex_param = Frontier_ExParamGet( core->fss->fmain );

	wk = FactoryScr_WorkAlloc( ex_param->savedata, init, type, level );
	Frontier_SysWorkSet( core->fss->fmain, wk );

	OS_Printf( "スクリプトファクトリーワーク確保\n" );
	return 0;
}

//--------------------------------------------------------------
/**
 * @brief	ファクトリーワーク初期化
 *
 * @param	core	仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
 */
//--------------------------------------------------------------
BOOL FSSC_FactoryWorkInit( FSS_TASK * core ) 
{
	FACTORY_SCRWORK* bf_scr_wk;
	u16 init = FSSTGetWorkValue( core );

	bf_scr_wk =  Frontier_SysWorkGet( core->fss->fmain );

	FactoryScr_WorkInit( bf_scr_wk, init );

	OS_Printf( "スクリプトファクトリーワーク初期化\n" );
	return 0;
}

//--------------------------------------------------------------
/**
 * @brief	ファクトリーワーク削除
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
 */
//--------------------------------------------------------------
BOOL FSSC_FactoryWorkFree( FSS_TASK * core ) 
{
	FACTORY_SCRWORK* bf_scr_wk;
	bf_scr_wk =  Frontier_SysWorkGet( core->fss->fmain );
	FactoryScr_WorkRelease( bf_scr_wk );
	OS_Printf( "スクリプトファクトリーワーク開放\n" );
	return 0;
}

//--------------------------------------------------------------
/**
 * @brief	スクリプトコマンド：ファクトリーレンタル呼び出し
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
 */
//--------------------------------------------------------------
BOOL FSSC_FactoryRentalCall( FSS_TASK * core)
{
	int i;
	FACTORY_SCRWORK* bf_scr_wk;
	FACTORY_CALL_WORK* factory_call;
	FRONTIER_EX_PARAM* ex_param = Frontier_ExParamGet( core->fss->fmain );

	//オーバーレイID宣言
	FS_EXTERN_OVERLAY( factory );
	
	//ファクトリープロセスデータ
	static const PROC_DATA FactoryProc = {	
		FactoryProc_Init,
		FactoryProc_Main,
		FactoryProc_End,
		FS_OVERLAY_ID( factory ),
	};

	bf_scr_wk =  Frontier_SysWorkGet( core->fss->fmain );

	OS_Printf( "ファクトリー呼び出し\n" );

	factory_call = sys_AllocMemory( HEAPID_WORLD, sizeof(FACTORY_CALL_WORK) );
	MI_CpuClear8( factory_call, sizeof(FACTORY_CALL_WORK) );
	factory_call->sv			= ex_param->savedata;
	factory_call->type			= bf_scr_wk->type;
	factory_call->level			= bf_scr_wk->level;
	factory_call->mode			= FACTORY_MODE_RENTAL;
	factory_call->p_m_party		= bf_scr_wk->p_m_party;
	factory_call->p_e_party		= bf_scr_wk->p_e_party;
	factory_call->p_work		= bf_scr_wk;

    Frontier_SubProcSet( core->fss->fmain, &FactoryProc, factory_call, FALSE, FactoryFreeMemory);
	return 1;
}

//--------------------------------------------------------------
/**
 * @brief	スクリプトコマンド：ファクトリー戦闘結果取得して開放
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
 */
//--------------------------------------------------------------
BOOL FSSC_FactoryCallGetResult( FSS_TASK * core)
{
	FACTORY_SCRWORK* bf_scr_wk;
	BATTLE_PARAM* param;

	bf_scr_wk =  Frontier_SysWorkGet( core->fss->fmain );
	param = bf_scr_wk->p_work;

	bf_scr_wk->winlose_flag = BattleParam_IsWinResult( param->win_lose_flag );
	OS_Printf( "bf_scr_wk->winlose_flag = %d\n", bf_scr_wk->winlose_flag );

	//BATTLE_PARAMの開放
	BattleParam_Delete( param );
	return 0;
}

//--------------------------------------------------------------
/**
 * @brief	スクリプトコマンド：戦闘呼び出し
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"1"
 */
//--------------------------------------------------------------
BOOL FSSC_FactoryBattleCall( FSS_TASK * core)
{
	BATTLE_PARAM* param;
	FACTORY_SCRWORK* bf_scr_wk;
	FRONTIER_EX_PARAM* ex_param = Frontier_ExParamGet( core->fss->fmain );

	bf_scr_wk =  Frontier_SysWorkGet( core->fss->fmain );

	//バトルデータ初期化
	param = BtlFactory_CreateBattleParam( bf_scr_wk, ex_param );
	bf_scr_wk->p_work = param;

	//戦闘切り替え
//	Snd_DataSetByScene( SND_SCENE_BATTLE, SEQ_BA_TRAIN, 1 );		//バトル曲再生
    Frontier_SubProcSet(core->fss->fmain, &TestBattleProcData, param, FALSE, NULL );
	OS_Printf( "スクリプトファクトリー戦闘呼び出し\n" );			//field_encount.c
	return 1;
}

//--------------------------------------------------------------
/**
 * @brief	スクリプトコマンド：ファクトリー交換呼び出し
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
 */
//--------------------------------------------------------------
BOOL FSSC_FactoryTradeCall( FSS_TASK * core )
{
	int i;
	FACTORY_SCRWORK* bf_scr_wk;
	FACTORY_CALL_WORK* factory_call;
	FRONTIER_EX_PARAM* ex_param = Frontier_ExParamGet( core->fss->fmain );

	//オーバーレイID宣言
	FS_EXTERN_OVERLAY( factory );
	
	//ファクトリープロセスデータ
	static const PROC_DATA FactoryProc = {	
		FactoryProc_Init,
		FactoryProc_Main,
		FactoryProc_End,
		FS_OVERLAY_ID( factory ),
	};

	bf_scr_wk =  Frontier_SysWorkGet( core->fss->fmain );

	OS_Printf( "ファクトリー交換呼び出し\n" );

	//戦闘呼び出し時に、
	//POKEPARRY[0-2]に、手持ちがセットされている
	//POKEPARRY[3-5]に、敵ポケモンがセットされている
		
	//クリアしてから渡す
	for( i=0; i < FACTORY_RET_WORK_MAX ;i++ ){
		bf_scr_wk->ret_work[i] = 0;
	}

	factory_call = sys_AllocMemory( HEAPID_WORLD, sizeof(FACTORY_CALL_WORK) );
	MI_CpuClear8( factory_call, sizeof(FACTORY_CALL_WORK) );
	factory_call->sv			= ex_param->savedata;
	factory_call->type			= bf_scr_wk->type;
	factory_call->level			= bf_scr_wk->level;
	factory_call->mode			= FACTORY_MODE_TRADE;
	factory_call->p_m_party		= bf_scr_wk->p_m_party;
	factory_call->p_e_party		= bf_scr_wk->p_e_party;
	factory_call->p_work		= bf_scr_wk;

    Frontier_SubProcSet( core->fss->fmain, &FactoryProc, factory_call, FALSE, FactoryFreeMemory);
	return 1;
}

//ファクトリー画面で渡したワークの解放処理
static void FactoryFreeMemory( void *parent_work )
{
	int i;
	FACTORY_CALL_WORK* factory_call = parent_work;

	//結果を取得
	FactoryScr_GetResult( factory_call->p_work, parent_work );

	//FACTORY_CALL_WORKの削除
	sys_FreeMemoryEz( parent_work );
	return;
}

//--------------------------------------------------------------
/**
 * @brief	スクリプトコマンド：レンタルPOKEPARTYのセット
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
 */
//--------------------------------------------------------------
BOOL FSSC_FactoryRentalPokePartySet( FSS_TASK * core )
{
	FACTORY_SCRWORK* bf_scr_wk;
	bf_scr_wk =  Frontier_SysWorkGet( core->fss->fmain );
	FactoryScr_RentalPartySet( bf_scr_wk );
	return 0;
}

//--------------------------------------------------------------
/**
 * @brief	スクリプトコマンド：バトル後のPOKEPARTYのセット
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
 */
//--------------------------------------------------------------
BOOL FSSC_FactoryBtlAfterPokePartySet( FSS_TASK * core )
{
	FACTORY_SCRWORK* bf_scr_wk;
	bf_scr_wk =  Frontier_SysWorkGet( core->fss->fmain );
	FactoryScr_BtlAfterPartySet( bf_scr_wk );
	return 0;
}

//--------------------------------------------------------------
/**
 * @brief	スクリプトコマンド：トレードした後のポケモン変更
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
 */
//--------------------------------------------------------------
BOOL FSSC_FactoryTradePokeChange( FSS_TASK * core )
{
	FACTORY_SCRWORK* bf_scr_wk;
	bf_scr_wk =  Frontier_SysWorkGet( core->fss->fmain );
	FactoryScr_TradePokeChange( bf_scr_wk );
	return 0;
}

//--------------------------------------------------------------
/**
 * @brief	スクリプトコマンド：トレードした後のポケモンのPOKEPARTYのセット
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
 */
//--------------------------------------------------------------
BOOL FSSC_FactoryTradeAfterPokePartySet( FSS_TASK * core )
{
	FACTORY_SCRWORK* bf_scr_wk;
	bf_scr_wk =  Frontier_SysWorkGet( core->fss->fmain );
	FactoryScr_TradeAfterPartySet( bf_scr_wk );
	return 0;
}

//--------------------------------------------------------------
/**
 * @brief	スクリプトコマンド：FACTORY_SCR_WORKのセット、ゲット
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
 */
//--------------------------------------------------------------
BOOL FSSC_FactoryScrWork( FSS_TASK * core )
{
	FMAP_PTR fmap;
	STRBUF* strbuf;
	POKEMON_PARAM* pp;
	FACTORY_SCRWORK* bf_scr_wk;
	u8 e_max,m_max;
	u32 type_tbl[9][2];
	u32 type,type_num;
	int i,j;
	u8 code		= FSSTGetU8( core );
	u8 param1	= FSSTGetU8( core );
	u8 param2	= FSSTGetU8( core );
	u16* work	= FSSTGetWork( core );

	bf_scr_wk	= Frontier_SysWorkGet( core->fss->fmain );
	fmap		= FSS_GetFMapAdrs( core->fss );

	switch( code ){

	//モード(レンタル、交換)のセット
	case FA_ID_SET_MODE:
		bf_scr_wk->mode = param1;
		break;

	//レベル(LV50、オープン)のセット
	case FA_ID_SET_LEVEL:
		bf_scr_wk->level = param1;
		break;

	//タイプ(シングル、ダブル、マルチ、wifiマルチ)のセット
	case FA_ID_SET_TYPE:
		bf_scr_wk->type = param1;
		break;

	//ret_workの取得
	case FA_ID_GET_RET_WORK:
		*work = bf_scr_wk->ret_work[param1];
		break;

	//連勝数の取得
	case FA_ID_GET_RENSYOU:
		*work = bf_scr_wk->rensyou;
		break;

	//連勝数のインクリメント
	case FA_ID_INC_RENSYOU:
		if( bf_scr_wk->rensyou < FACTORY_RENSYOU_MAX ){
			bf_scr_wk->rensyou++;
		}
		break;

	//リセットしてタイトルに戻る
	case FA_ID_SYSTEM_RESET:
		OS_ResetSystem( 0 );
		break;

	//------------------------------------
	//	FACTORYDATAにアクセス
	//------------------------------------
	//セーブデータが有効かどうか返す
	case FA_ID_IS_SAVE_DATA_ENABLE:
		*work = FACTORYDATA_GetSaveFlag( bf_scr_wk->factory_savedata );
		break;

	//休む時に現在のプレイ状況をセーブに書き出す
	case FA_ID_SAVE_REST_PLAY_DATA:
		FactoryScr_SaveRestPlayData( bf_scr_wk );
		break;

	//------------------------------------
	//	
	//------------------------------------

	//ラウンド数をインクリメント
	case FA_ID_INC_ROUND:
		*work = FactoryScr_IncRound( bf_scr_wk );
		break;

	//モンスターナンバーを取得
	case FA_ID_GET_MONSNO:
		*work = bf_scr_wk->enemy_poke[param1].mons_no;
		break;

	//技ナンバーを取得
	case FA_ID_GET_WAZANO:
		*work = bf_scr_wk->enemy_poke[param1].waza[param2];
		break;

	//ポケモンタイプを取得
	case FA_ID_GET_POKE_TYPE:
		pp = PokemonParam_AllocWork( HEAPID_WORLD );
		//バトルタワー用ポケモンデータからPOKEMON_PARAMを生成
		Frontier_PokeParaMake( &bf_scr_wk->enemy_poke[param1], pp, Factory_GetLevel(bf_scr_wk) );
		*work = PokeParaGet( pp, ID_PARA_type1, NULL );
		sys_FreeMemoryEz( pp );
		break;

	//最も多いタイプを取得
	case FA_ID_GET_LARGE_TYPE:
		e_max = Factory_GetEnemyPokeNum( bf_scr_wk->type, FACTORY_FLAG_TOTAL );

		//初期化
		for( i=0; i < e_max ;i++ ){
			type_tbl[i][0] = 0xff;
			type_tbl[i][1] = 0xff;
		}

		pp = PokemonParam_AllocWork( HEAPID_WORLD );
		for( i=0; i < e_max ;i++ ){

			//バトルタワー用ポケモンデータからPOKEMON_PARAMを生成
			Frontier_PokeParaMake( &bf_scr_wk->enemy_poke[i], pp, Factory_GetLevel(bf_scr_wk) );

			//タイプ取得
			type = PokeParaGet( pp, ID_PARA_type1, NULL );

			for( j=i; j > 0 ;j-- ){
				if( type == type_tbl[j-1][0] ){
					type_tbl[j-1][0]++;
					break;
				}
			}

			//同じタイプがなかった時
			if( j == 0 ){
				type_tbl[i][0] = type;
				type_tbl[i][1]++;;
			}
		}
		sys_FreeMemoryEz( pp );

		//最も数が多いタイプを探す
		type = type_tbl[0][0];
		type_num = type_tbl[0][1];
		for( i=0; i < e_max ;i++ ){
			if( type_num < type_tbl[i][1] ){
				type = type_tbl[i][0];
				type_num = type_tbl[i][1];
			}
		}

		//1つも同じタイプが存在しない時
		if( type_num == 1 ){
			*work = 0xff;
		}else{
			*work = type;
		}
		break;

	//周回数を取得
	case FA_ID_GET_LAP:
		*work = FactoryScr_CommGetLap( bf_scr_wk );
		break;

	//敵トレーナーのOBJコードを取得
	case FA_ID_GET_TR_OBJ_CODE:
		*work = FactoryScr_GetEnemyObjCode( bf_scr_wk, param1 );
		break;

	//敗戦した時の処理
	case FA_ID_SET_LOSE:
		FactoryScr_SetLose( bf_scr_wk );
		break;

	//7連勝(クリア)した時の処理
	case FA_ID_SET_CLEAR:
		FactoryScr_SetClear( bf_scr_wk );
		break;

	//ラウンド数を取得
	case FA_ID_GET_ROUND:
		*work = FactoryScr_GetRound( bf_scr_wk );
		break;

	//リタイヤフラグを取得
	case FA_ID_GET_RETIRE_FLAG:
		*work = bf_scr_wk->pair_retire_flag;
		break;

	//交換したいかフラグを取得
	case FA_ID_GET_TRADE_YESNO_FLAG:
		*work = bf_scr_wk->pair_trade_yesno_flag;
		break;

	//通信設定
	case FA_ID_COMM_COMMAND_INITIALIZE:
		CommCommandFrontierInitialize( bf_scr_wk );
		break;

	//通信タイプかチェック
	case FA_ID_CHECK_COMM_TYPE:
		*work = Factory_CommCheck( bf_scr_wk->type );
		break;

	//タイプ取得
	case FA_ID_GET_TYPE:
		*work = bf_scr_wk->type;
		break;

	//バトル部屋の地面のパレットを変更する(evyを変更していく)
	case FA_ID_BTL_ROOM_PAL_CHG:
#if 0
		{
			void *buf;
			NNSG2dPaletteData *dat;
			u8 evy;
			FMAP_PTR fmap = FSS_GetFMapAdrs( core->fss );

			//係数(0～16) (変更の度合い、0(元の色)～2,3..(中間色)～16(指定した色)
			evy = param1;
			SoftFadePfd( fmap->pfd, FADE_MAIN_BG_EX3, 0, 1, evy, 0x0000 );
			SoftFadePfd( fmap->pfd, FADE_MAIN_BG_EX3, 1, 1, evy, 0x62cd );
			SoftFadePfd( fmap->pfd, FADE_MAIN_BG_EX3, 2, 1, evy, 0x6aee );
			SoftFadePfd( fmap->pfd, FADE_MAIN_BG_EX3, 3, 1, evy, 0x774e );
			SoftFadePfd( fmap->pfd, FADE_MAIN_BG_EX3, 4, 1, evy, 0x66ed );
		}
#else
		OS_Printf( "切り替えるパレットナンバー = %d\n", param1 );
		GF_BGL_ScrPalChange( fmap->bgl, FRMAP_FRAME_MAP, 3, 10, 26, 11, param1 );
		GF_BGL_LoadScreenV_Req( fmap->bgl, FRMAP_FRAME_MAP );			//スクリーンデータ転送
#endif
		break;

	//マップのレール移動TCB追加
	case FA_ID_ADD_MAP_RAIL_TCB:
		bf_scr_wk->rail_move_tcb = TCB_Add( Factory_MapRailMoveMain, FSS_GetFMapAdrs(core->fss), 
											FACTORY_RAIL_TCB_PRI );
		break;

	//マップのレール移動TCB削除
	case FA_ID_DEL_MAP_RAIL_TCB:
		if( bf_scr_wk->rail_move_tcb != NULL ){
			TCB_Delete( bf_scr_wk->rail_move_tcb );
			bf_scr_wk->rail_move_tcb = NULL;
		}
		break;

	//戦闘前会話のためにB_TOWER_PARTNER_DATAのセット
	case FA_ID_SET_B_TOWER_PARTNER_DATA:
		FSRomBattleTowerTrainerDataMake2( &(bf_scr_wk->tr_data[0]), 
						bf_scr_wk->tr_index[bf_scr_wk->round], HEAPID_WORLD, ARC_PL_BTD_TR );
		FSRomBattleTowerTrainerDataMake2( &(bf_scr_wk->tr_data[1]),
						bf_scr_wk->tr_index[bf_scr_wk->round+FACTORY_LAP_ENEMY_MAX], HEAPID_WORLD,
						ARC_PL_BTD_TR );
		break;

	//主人公の手持ちポケモン名をセット
	case FA_ID_BRAIN_PLAYER_POKE_NAME:
		m_max = Factory_GetMinePokeNum( bf_scr_wk->type );
		for( i=0; i < m_max; i++ ){
			pp = PokeParty_GetMemberPointer( bf_scr_wk->p_m_party, i );
			WORDSET_RegisterPokeMonsName( core->fss->wordset, i, PPPPointerGet(pp) );
		}
		break;

	//貰えるバトルポイントを取得
	case FA_ID_GET_BP_POINT:
		*work = FactoryScr_GetAddBtlPoint( bf_scr_wk );
		break;

	};

	return 0;
}

//--------------------------------------------------------------
/**
 * @brief	エフェクト面(レール)を縦にスクロールさせつづける
 *
 * @param	tcb		TCB_PTR型
 * @param	work	ワーク
 *
 * @retval	none
 */
//--------------------------------------------------------------
void Factory_MapRailMoveMain( TCB_PTR tcb, void * work )
{
	int y;
	FMAP_PTR fmap = work;
	
	y = GF_BGL_ScrollGetY( fmap->bgl, FRMAP_FRAME_EFF );

	if( y >= 255 ){
		GF_BGL_ScrollReq(fmap->bgl,FRMAP_FRAME_EFF,GF_BGL_SCROLL_Y_SET,0);
	}else{
		GF_BGL_ScrollReq(fmap->bgl,FRMAP_FRAME_EFF,GF_BGL_SCROLL_Y_INC,FACTORY_SCROLL_MAP_RAIL_Y);
	}
	return;
}

//--------------------------------------------------------------
/**
 * トレーナー敗北チェック
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
 */
//--------------------------------------------------------------
BOOL FSSC_FactoryLoseCheck( FSS_TASK * core )
{
	FACTORY_SCRWORK* bf_scr_wk;
	u16* work = FSSTGetWork( core );

	bf_scr_wk =  Frontier_SysWorkGet( core->fss->fmain );

	*work = bf_scr_wk->winlose_flag;
	OS_Printf( "敗北チェック*work TRUE勝利 FALSE敗北 = %d\n", *work );
	return 0;
}


//==============================================================================================
//
//	送信、受信の呼び出し
//
//==============================================================================================

//--------------------------------------------------------------
/**
 * 送信
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
 */
//--------------------------------------------------------------
BOOL FSSC_FactorySendBuf( FSS_TASK * core )
{
	FACTORY_SCRWORK* bf_scr_wk;
	u16 type	= FSSTGetWorkValue( core );
	u16 param	= FSSTGetWorkValue( core );
	u16* ret_wk	= FSSTGetWork( core );

	bf_scr_wk =  Frontier_SysWorkGet( core->fss->fmain );

	*ret_wk = FactoryScr_CommSetSendBuf( bf_scr_wk, type, param );
	return 1;
}

//--------------------------------------------------------------
/**
 * 受信
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
 */
//--------------------------------------------------------------
BOOL FSSC_FactoryRecvBuf( FSS_TASK * core )
{
	u16 wk_id = FSSTGetU16( core );

	//仮想マシンの汎用レジスタにワークのIDを格納
	core->reg[0] = wk_id;

	FSST_SetWait( core, WaitFactoryRecvBuf );
	return 1;
}

//return 1 = 終了
static BOOL WaitFactoryRecvBuf( FSS_TASK * core )
{
	FACTORY_SCRWORK* bf_scr_wk;
	u16 type = FSS_GetEventWorkValue( core, core->reg[0] );		//注意！

	bf_scr_wk =  Frontier_SysWorkGet( core->fss->fmain );

	if( bf_scr_wk->recieve_count >= FACTORY_COMM_PLAYER_NUM ){
		bf_scr_wk->recieve_count = 0;
		return 1;
	}

	return 0;
}

//--------------------------------------------------------------
/**
 *	@brief	バトルファクトリー対戦前メッセージ専用表示
 */
//--------------------------------------------------------------
BOOL FSSC_FactoryTalkMsgAppear(FSS_TASK* core)
{
	u16	*msg;
	FACTORY_SCRWORK* ba_scr_wk;
	FRONTIER_EX_PARAM* ex_param = Frontier_ExParamGet( core->fss->fmain );
	u16	tr_idx = FSSTGetU8(core);	//一人目か二人目か？

	ba_scr_wk =  Frontier_SysWorkGet( core->fss->fmain );
	if(ba_scr_wk == NULL){
		return 0;
	}

	msg = ba_scr_wk->tr_data[tr_idx].bt_trd.appear_word;

	FrontierTalkMsgSub( core, msg );
	return 1;
}


