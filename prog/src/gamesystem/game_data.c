//=============================================================================
/**
 * @file	game_data.c
 * @brief	ゲーム進行データ管理
 * @author	tamada
 * @date	2008.11.04
 *
 * 基本的にゲーム進行データはセーブデータとして存在するが、
 * 通信時の複数プレイヤー情報やフィールドマップ情報などセーブデータに
 * のるとは限らない情報もある。
 * そのようなセーブデータ・非セーブデータへ並列にアクセスするインターフェイスを
 * 各パートごとに構成するとパートごとにアクセス手法が違ったり、また同じ機能の
 * ものが複数存在するなどプログラム全体の複雑さが増してしまう可能性が高い。
 * それを避けるため、共通インターフェイスを作ることによって簡略化する試み
 * …試みなので途中で方針を変えるかも。
 */
//=============================================================================

#include <gflib.h>

#include "gamesystem/game_data.h"
#include "gamesystem/playerwork.h"
#include "field/eventdata_system.h"
#include "savedata/save_control.h"
#include "savedata/save_tbl.h"
#include "savedata/myitem_savedata.h"
#include "poke_tool/pokeparty.h"
#include "savedata/mystatus.h"
#include "savedata/situation.h"
#include "savedata/player_data.h"
#include "gamesystem/pm_season.h"		//季節定義参照

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	ゲームデータ内容定義
 */
//------------------------------------------------------------------
struct _GAMEDATA{
	SAVE_CONTROL_WORK *sv_control_ptr;		///<セーブデータ管理ワークへのポインタ
	PLAYER_WORK playerWork[PLAYER_MAX];
	EVENTDATA_SYSTEM * evdata;
	LOCATION *start_loc;
	LOCATION *entrance_loc;
	LOCATION *special_loc;
	MYITEM_PTR myitem;			///<手持ちアイテムセーブデータへのポインタ
	POKEPARTY *my_pokeparty;	///<手持ちポケモンセーブデータへのポインタ
	FLDMMDLSYS *fldmmdlsys;
	EVENTWORK *eventwork;
	u8 season_id;				///<季節指定ID
};

//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static void GAMEDATA_SaveDataLoad(GAMEDATA *gamedata);
static void GAMEDATA_SaveDataUpdate(GAMEDATA *gamedata);


//============================================================================================
//
//
//		GAMEDATA全体
//
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
GAMEDATA * GAMEDATA_Create(HEAPID heapID)
{
	int i;
	GAMEDATA * gd;
	SITUATION *st;
	
	gd = GFL_HEAP_AllocMemory(heapID, sizeof(GAMEDATA));
	GFL_STD_MemClear(gd, sizeof(GAMEDATA));
	
	gd->sv_control_ptr = SaveControl_GetPointer();
	
	//季節設定：ゲーム内では1ヶ月＝１シーズン
	{
		RTCDate date;
		GFL_RTC_GetDate(&date);
		gd->season_id = date.month % PMSEASON_TOTAL;
	}

	//状況データ
	st = SaveData_GetSituation(gd->sv_control_ptr);
	gd->start_loc = Situation_GetStartLocation(st);
	gd->entrance_loc = Situation_GetStartLocation(st);
	gd->special_loc = Situation_GetStartLocation(st);
	
	//プレイヤーワーク
	for (i = 0; i < PLAYER_MAX; i++) {
		PLAYERWORK_init(&gd->playerWork[i]);
	}
	//自分自身のプレイヤーワークのマイステータスにセーブデータのマイステータスをコピーしてくる
	//「最初から」で始めたときでもここに来るまでに名前入力がされているので
	//セーブの有無に関係なくコピー
	GFL_STD_MemCopy(SaveData_GetMyStatus(gd->sv_control_ptr), 
		GAMEDATA_GetMyStatus(gd), MyStatus_GetWorkSize());
	
	//イベントデータ
	gd->evdata = EVENTDATA_SYS_Create(heapID);
	EVENTDATA_SYS_Clear(gd->evdata);
	
	//イベントワーク
	gd->eventwork = EVENTWORK_AllocWork( heapID );
	EVENTWORK_Init( gd->eventwork );

	//動作モデル
	gd->fldmmdlsys = FLDMMDLSYS_CreateSystem( heapID, FLDMMDL_MDL_MAX );
	
	gd->myitem = SaveControl_DataPtrGet(gd->sv_control_ptr, GMDATA_ID_MYITEM);
	gd->my_pokeparty = SaveControl_DataPtrGet(gd->sv_control_ptr, GMDATA_ID_MYPOKE);
	
	if(SaveControl_NewDataFlagGet(gd->sv_control_ptr) == FALSE){
	}
	
	GAMEDATA_SaveDataLoad(gd);	//セーブデータから必要なものをロードしてくる
	return gd;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void GAMEDATA_Delete(GAMEDATA * gamedata)
{
	FLDMMDLSYS_FreeSystem(gamedata->fldmmdlsys);
	EVENTWORK_FreeWork(gamedata->eventwork);
	EVENTDATA_SYS_Delete(gamedata->evdata);
	GFL_HEAP_FreeMemory(gamedata);
}
//------------------------------------------------------------------
//------------------------------------------------------------------
PLAYER_WORK * GAMEDATA_GetPlayerWork(GAMEDATA * gamedata, u32 player_id)
{
	GF_ASSERT(player_id < PLAYER_MAX);
	return &gamedata->playerWork[player_id];
}

//------------------------------------------------------------------
//------------------------------------------------------------------
PLAYER_WORK * GAMEDATA_GetMyPlayerWork(GAMEDATA * gamedata)
{
	return &gamedata->playerWork[0];
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	イベント起動データシステムへのポインタを取得する
 * @param	gamedata			GAMEDATAへのポインタ
 * @return	EVENTDATA_SYSTEM	イベント起動データシステムへのポインタ
 */
//------------------------------------------------------------------
EVENTDATA_SYSTEM * GAMEDATA_GetEventData(GAMEDATA * gamedata)
{
	return gamedata->evdata;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
const LOCATION * GAMEDATA_GetStartLocation(const GAMEDATA * gamedata)
{
	return gamedata->start_loc;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void GAMEDATA_SetStartLocation(GAMEDATA * gamedata, const LOCATION * loc)
{
	*(gamedata->start_loc) = *loc;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const LOCATION * GAMEDATA_GetEntranceLocation(const GAMEDATA * gamedata)
{
	return gamedata->entrance_loc;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void GAMEDATA_SetEntranceLocation(GAMEDATA * gamedata, const LOCATION * loc)
{
	*(gamedata->entrance_loc) = *loc;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const LOCATION * GAMEDATA_GetSpecialLocation(const GAMEDATA * gamedata)
{
	return gamedata->special_loc;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void GAMEDATA_SetSpecialLocation(GAMEDATA * gamedata, const LOCATION * loc)
{
	*(gamedata->special_loc) = *loc;
}

//--------------------------------------------------------------
/**
 * @brief   手持ちアイテムへのポインタ取得
 * @param   gamedata		GAMEDATAへのポインタ
 * @retval  手持ちアイテムへのポインタ
 */
//--------------------------------------------------------------
MYITEM_PTR GAMEDATA_GetMyItem(const GAMEDATA * gamedata)
{
	return gamedata->myitem;
}

//--------------------------------------------------------------
/**
 * @brief   手持ちポケモンへのポインタ取得
 * @param   gamedata		GAMEDATAへのポインタ
 * @retval  手持ちポケモンへのポインタ
 */
//--------------------------------------------------------------
POKEPARTY * GAMEDATA_GetMyPokemon(const GAMEDATA * gamedata)
{
	return gamedata->my_pokeparty;
}

//--------------------------------------------------------------
/**
 * @brief   MYSTATUSへのポインタ取得
 * @param   gamedata		GAMEDATAへのポインタ
 * @retval  MYSTATUSへのポインタ
 */
//--------------------------------------------------------------
MYSTATUS * GAMEDATA_GetMyStatus(GAMEDATA * gamedata)
{
	return &((GAMEDATA_GetMyPlayerWork(gamedata))->mystatus);
}

//--------------------------------------------------------------
/**
 * @brief	FLDMMDLSYSへのポインタ取得
 * @param	gamedata	GAMEDATAへのポインタ
 * @retval	FLDMMDLSYSへのポインタ
 */
//--------------------------------------------------------------
FLDMMDLSYS * GAMEDATA_GetFldMMdlSys(GAMEDATA *gamedata)
{
	return gamedata->fldmmdlsys;
}

//--------------------------------------------------------------
/**
 * @brief	EVENTWORKへのポインタ取得
 * @param	gamedata	GAMEDATAへのポインタ
 * @retval	EVENTWORKへのポインタ
 */
//--------------------------------------------------------------
EVENTWORK * GAMEDATA_GetEventWork(GAMEDATA *gamedata)
{
	return gamedata->eventwork;
}

//--------------------------------------------------------------
/**
 * @brief	季節の取得
 * @param   gamedata		GAMEDATAへのポインタ
 * @return	u8	季節ID（gamesystem/pm_season.h参照）
 */
//--------------------------------------------------------------
u8 GAMEDATA_GetSeasonID(const GAMEDATA *gamedata)
{
	return gamedata->season_id;
}

//============================================================================================
//
//
//	PLAYER_WORK用関数
//	※player_work.cを作成して移動させるかも
//
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	PLAYER_WORK初期化
 */
//------------------------------------------------------------------
void PLAYERWORK_init(PLAYER_WORK * player)
{
	player->zoneID = 0;
	player->position.x = 0;
	player->position.y = 0;
	player->position.z = 0;
	player->direction = 0;
	player->mystatus.id = 0;
	player->mystatus.sex = 0;
	player->mystatus.region_code = 0;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void PLAYERWORK_setPosition(PLAYER_WORK * player, const VecFx32 * pos)
{
	player->position = *pos;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const VecFx32 * PLAYERWORK_getPosition(const PLAYER_WORK * player)
{
	return &player->position;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void PLAYERWORK_setZoneID(PLAYER_WORK * player, ZONEID zoneid)
{
	player->zoneID = zoneid;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
ZONEID PLAYERWORK_getZoneID(const PLAYER_WORK * player)
{
	return player->zoneID;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void PLAYERWORK_setDirection(PLAYER_WORK * player, u16 direction)
{
	player->direction = direction;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
u16 PLAYERWORK_getDirection(const PLAYER_WORK * player)
{
	return player->direction;
}

//==============================================================================
//	ゲームデータが持つ情報を元にセーブ
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   セーブデータワークからゲームデータが持つ情報をロードする
 *
 * @param   gamedata		ゲームデータへのポインタ
 */
//--------------------------------------------------------------
static void GAMEDATA_SaveDataLoad(GAMEDATA *gamedata)
{
	if(SaveControl_NewDataFlagGet(gamedata->sv_control_ptr) == TRUE){
		return;	//セーブデータが無いので何も読み込まない
	}
	
	{	//PLAYER_WORK
		PLAYER_WORK *pw;
		pw = GAMEDATA_GetMyPlayerWork(gamedata);
		SaveData_PlayerDataLoad(gamedata->sv_control_ptr, pw);
		SaveData_SituationDataLoad(gamedata->sv_control_ptr, pw);
	}
	
	{	//FLDMMDL
		FLDMMDLSYS *fldmmdlsys = GAMEDATA_GetFldMMdlSys(gamedata);
		FLDMMDL_SAVEDATA *pw = SaveControl_DataPtrGet(gamedata->sv_control_ptr,GMDATA_ID_FLDMMDL);
		FLDMMDL_SAVEDATA_Load( fldmmdlsys, pw );
	}
}

//--------------------------------------------------------------
/**
 * @brief   ゲームデータが持つ情報を元にセーブデータのワークを更新する
 *
 * @param   gamedata		ゲームデータへのポインタ
 *
 * セーブするデータを、セーブワークから直接使用せずに、別ワークにコピーして使用しているものが対象
 */
//--------------------------------------------------------------
static void GAMEDATA_SaveDataUpdate(GAMEDATA *gamedata)
{
	{	//PLAYER_WORK
		PLAYER_WORK *pw = GAMEDATA_GetMyPlayerWork(gamedata);
		SaveData_PlayerDataUpdate(gamedata->sv_control_ptr, pw);
		SaveData_SituationDataUpdate(gamedata->sv_control_ptr, pw);
	}

	{	//FLDMMDL
		FLDMMDLSYS *fldmmdlsys = GAMEDATA_GetFldMMdlSys(gamedata);
		FLDMMDL_SAVEDATA *pw = SaveControl_DataPtrGet(gamedata->sv_control_ptr,GMDATA_ID_FLDMMDL);
		FLDMMDL_SAVEDATA_Save( fldmmdlsys, pw );
	}
}

//--------------------------------------------------------------
/**
 * @brief   ゲームデータが持つ情報を元にセーブを実行
 *
 * @param   gamedata		ゲームデータへのポインタ
 *
 * @retval  セーブ結果
 */
//--------------------------------------------------------------
SAVE_RESULT GAMEDATA_Save(GAMEDATA *gamedata)
{
	//セーブワークの情報を更新
	GAMEDATA_SaveDataUpdate(gamedata);
	
	//セーブ実行
	return SaveControl_Save(gamedata->sv_control_ptr);
}
