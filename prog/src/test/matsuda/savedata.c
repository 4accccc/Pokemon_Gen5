//==============================================================================
/**
 * @file	savedata.c
 * @brief	セーブデータ管理
 * @author	matsuda
 * @date	2008.08.26(火)
 */
//==============================================================================
#include "gflib.h"

#include "savedata.h"
#include "savedata_local.h"
#include "savedata.h"


//セーブデータ内容を扱うのに必要な関数参照のためのヘッダ
#include "contest_savedata.h"
//#include "misc.h"
//#include "misc_local.h"


//=============================================================================
//=============================================================================

//============================================================================================
//
//
//		ロムデータ
//
//
//============================================================================================
//---------------------------------------------------------------------------
/**
 * @brief	セーブデータ内容定義
 */
//---------------------------------------------------------------------------
const SAVEDATA_TABLE SaveDataTable[] = {
//ノーマルデータグループ
	{	//コンテスト
		GMDATA_ID_CONTEST,
		SVBLK_ID_NORMAL,
		(FUNC_GET_SIZE)CONDATA_GetWorkSize,
		(FUNC_INIT_WORK)CONDATA_Init,
	},
#if 0
	{	//その他データ
		GMDATA_ID_MISC,
		SVBLK_ID_NORMAL,
		(FUNC_GET_SIZE)MISC_GetWorkSize,
		(FUNC_INIT_WORK)MISC_Init,
	},
#endif


//ボックスデータグループ
#if 0
	{	//ポケモンボックスのデータ
		GMDATA_ID_BOXDATA,
		SVBLK_ID_BOX,
		(FUNC_GET_SIZE)BOXDAT_GetTotalSize,
		(FUNC_INIT_WORK)BOXDAT_Init,
	},
#endif
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
const int SaveDataTableMax = NELEMS(SaveDataTable);



//============================================================================================
//
//
//
//============================================================================================
//---------------------------------------------------------------------------
/**
 * @brief	ボックスデータのポインタ取得
 * @param	sv			セーブデータ構造へのポインタ
 * @return	BOX_DATA	ボックスデータへのポインタ
 */
//---------------------------------------------------------------------------
#if 0
BOX_DATA * SaveData_GetBoxData(SAVEDATA * sv)
{
	return SaveData_Get(sv, GMDATA_ID_BOXDATA);
}
#endif

//============================================================================================
//
//
//			特殊セーブデータ
//
//
//============================================================================================
//---------------------------------------------------------------------------
/**
 * @brief	特殊セーブデータの定義テーブル
 */
//---------------------------------------------------------------------------
const EXSAVEDATA_TABLE ExtraSaveDataTable[] = {
	{
		// 殿堂入りデータ
		EXDATA_ID_DENDOU,
		SAVE_PAGE_MAX + 0,
		(FUNC_GET_SIZE)DendouData_GetWorkSize,
		(FUNC_INIT_WORK)DendouData_Init,
	},

	{	//バトルフロンティア外部データ(進行に関係ない記録のみ)
		EXDATA_ID_FRONTIER,
		SAVE_PAGE_MAX + 3,
		(FUNC_GET_SIZE)FrontierEx_GetWorkSize,
		(FUNC_INIT_WORK)FrontierEx_Init,
	},
	
	{	//戦闘録画：自分
		EXDATA_ID_BATTLE_REC_MINE,
		SAVE_PAGE_MAX + 4,
		(FUNC_GET_SIZE)BattleRec_GetWorkSize,
		(FUNC_INIT_WORK)BattleRec_WorkInit,
	},
	{	//戦闘録画：ダウンロード0番
		EXDATA_ID_BATTLE_REC_DL_0,
		SAVE_PAGE_MAX + 6,
		(FUNC_GET_SIZE)BattleRec_GetWorkSize,
		(FUNC_INIT_WORK)BattleRec_WorkInit,
	},
	{	//戦闘録画：ダウンロード1番
		EXDATA_ID_BATTLE_REC_DL_1,
		SAVE_PAGE_MAX + 8,
		(FUNC_GET_SIZE)BattleRec_GetWorkSize,
		(FUNC_INIT_WORK)BattleRec_WorkInit,
	},
	{	//戦闘録画：ダウンロード2番
		EXDATA_ID_BATTLE_REC_DL_2,
		SAVE_PAGE_MAX + 10,
		(FUNC_GET_SIZE)BattleRec_GetWorkSize,
		(FUNC_INIT_WORK)BattleRec_WorkInit,
	},
};
//---------------------------------------------------------------------------
/**
 * @brief	特殊セーブデータのテーブル数
 */
//---------------------------------------------------------------------------
const int ExtraSaveDataTableMax = NELEMS(ExtraSaveDataTable);



//============================================================================================
//
//
//			個別の特殊セーブデータアクセス関数
//
//
//============================================================================================
//---------------------------------------------------------------------------
/**
 * @brief	特殊セーブデータロード：殿堂入りデータ
 * @param	sv			セーブデータ構造へのポインタ
 * @param	heap_id		ワークを取得するヒープのID
 * @param	result
 * @return	void *	取得したセーブデータを載せたワークへのポインタ
 */
//---------------------------------------------------------------------------
DENDOU_SAVEDATA * SaveData_Extra_LoadDendouData(SAVEDATA * sv, int heap_id, LOAD_RESULT * result)
{
	return SaveData_Extra_LoadAlloc(sv, heap_id, EXDATA_ID_DENDOU, result);
}

//---------------------------------------------------------------------------
/**
 * @brief	特殊セーブデータセーブ：殿堂入りデータ
 * @param	sv			セーブデータ構造へのポインタ
 * @param	data		殿堂入りデータへのポインタ
 * @retval	SAVE_RESULT_OK		セーブ成功
 * @retval	SAVE_RESULT_NG		セーブ失敗
 */
//---------------------------------------------------------------------------
SAVE_RESULT SaveData_Extra_SaveDendouData(SAVEDATA * sv, DENDOU_SAVEDATA * data)
{
	return SaveData_Extra_Save(sv, EXDATA_ID_DENDOU, data);
}


//---------------------------------------------------------------------------
/**
 * @brief	特殊セーブデータロード：戦闘録画
 * @param	sv			セーブデータ構造へのポインタ
 * @param	heap_id		ワークを取得するヒープのID
 * @param	result
 * @param   num			LOADDATA_MYREC, LOADDATA_DOWNLOAD0, ...
 * @return	void *	取得したセーブデータを載せたワークへのポインタ
 */
//---------------------------------------------------------------------------
BATTLE_REC_SAVEDATA * SaveData_Extra_LoadBattleRecData(SAVEDATA * sv, int heap_id, LOAD_RESULT * result, int num)
{
	BOOL old;
	return SaveData_Extra_Mirror_LoadAlloc(sv, heap_id, EXDATA_ID_BATTLE_REC_MINE + num, result, &old);
}

//---------------------------------------------------------------------------
/**
 * @brief	特殊セーブデータセーブ：戦闘録画
 * @param	sv			セーブデータ構造へのポインタ
 * @param	data		戦闘録画データへのポインタ(CHECK_TAIL_DATA構造体分を含めたものである
 * 						必要があるのでSaveData_Extra_LoadBattleRecData関数の戻り値で手に入れた
 *						バッファポインタを渡す必要がある。
 * @param   num			LOADDATA_MYREC, LOADDATA_DOWNLOAD0, ...
 *
 * @retval	SAVE_RESULT_OK		セーブ成功
 * @retval	SAVE_RESULT_NG		セーブ失敗
 */
//---------------------------------------------------------------------------
SAVE_RESULT SaveData_Extra_SaveBattleRecData(SAVEDATA * sv, BATTLE_REC_SAVEDATA * data, int num)
{
	return SaveData_Extra_Mirror_Save(sv, EXDATA_ID_BATTLE_REC_MINE + num, data);
}

//---------------------------------------------------------------------------
/**
 * @brief	特殊セーブデータロード：フロンティア外部データ
 * @param	sv			セーブデータ構造へのポインタ
 * @param	heap_id		ワークを取得するヒープのID
 * @param	result
 * @return	void *	取得したセーブデータを載せたワークへのポインタ
 */
//---------------------------------------------------------------------------
FRONTIER_EX_SAVEDATA * SaveData_Extra_LoadFrontierEx(SAVEDATA * sv, int heap_id, LOAD_RESULT * result)
{
	BOOL old;
	return SaveData_Extra_Mirror_LoadAlloc(sv, heap_id, EXDATA_ID_FRONTIER, result, &old);
}

//---------------------------------------------------------------------------
/**
 * @brief	特殊セーブデータセーブ：フロンティア外部データ
 * @param	sv			セーブデータ構造へのポインタ
 * @param	data		戦闘録画データへのポインタ(CHECK_TAIL_DATA構造体分を含めたものである
 * 						必要があるのでSaveData_Extra_LoadFrontierEx関数の戻り値で手に入れた
 *						バッファポインタを渡す必要がある。
 *
 * @retval	SAVE_RESULT_OK		セーブ成功
 * @retval	SAVE_RESULT_NG		セーブ失敗
 */
//---------------------------------------------------------------------------
SAVE_RESULT SaveData_Extra_SaveFrontierEx(SAVEDATA * sv, FRONTIER_EX_SAVEDATA * data)
{
	return SaveData_Extra_Mirror_Save(sv, EXDATA_ID_FRONTIER, data);
}


