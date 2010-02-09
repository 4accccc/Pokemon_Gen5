//============================================================================================
/**
 * @file	situation.c
 * @brief	状況データアクセス
 * @author	tamada GAME FREAK inc.
 * @date	2005.12.03
 */
//============================================================================================
#include <gflib.h>
#include "savedata/save_tbl.h"
#include "savedata/situation.h"
#include "gamesystem/playerwork.h"
#include "field/field_status_local.h"


//============================================================================================
//============================================================================================
//----------------------------------------------------------
/**
 * @brief	状況データ型定義
 */
//----------------------------------------------------------
struct _SITUATION {
	LOCATION start_loc;     ///<
	LOCATION entrance_loc;  ///<直前の出入口LOCATION
	LOCATION special_loc;   ///<特殊接続先LOCATION
  LOCATION escape_loc;    ///<脱出先LOCATION
  u16 warp_id;            ///<ワープ飛び先指定ID
  u8  fs_flash;           ///<フラッシュフラグ          
  u8  season_id;          ///<季節
  u8  weather_id;         ///<天気
  u8  pad[3];             ///<padding
  u32 egg_step_count;     ///<タマゴ孵化カウンタ

	//PLAYER_WORKからセーブに必要なものだけを抜粋
	PLAYERWORK_SAVE plsv;
};


//============================================================================================
//
//	セーブデータシステムが依存する関数
//
//============================================================================================
//----------------------------------------------------------
/**
 * @brief	状況ワークのサイズ取得
 * @return	int		サイズ（バイト単位）
 */
//----------------------------------------------------------
int Situation_GetWorkSize(void)
{
	return sizeof(SITUATION);
}

//============================================================================================
//
//	状況データ操作のための関数
//
//============================================================================================
//----------------------------------------------------------
/**
 * @brief	状況データの初期化
 * @param	st	状況ワークへのポインタ
 */
//----------------------------------------------------------
void Situation_Init(SITUATION * st)
{
	GFL_STD_MemClear(st, sizeof(SITUATION));
}

//----------------------------------------------------------
/**
 * @brief	現在のLOCATIONを返す
 * @param	st	状況ワークへのポインタ
 * @return	LOCATIONへのポインタ
 */
//----------------------------------------------------------
LOCATION * Situation_GetStartLocation(SITUATION * st)
{
	return &st->start_loc;
}

//----------------------------------------------------------
/**
 * @brief	直前に入った出入口を示すLOCATIONへのポインタ
 * @param	st	状況ワークへのポインタ
 * @return	LOCATIONへのポインタ
 */
//----------------------------------------------------------
LOCATION * Situation_GetEntranceLocation(SITUATION * st)
{
	return &st->entrance_loc;
}

//----------------------------------------------------------
/**
 * @brief	特殊接続情報の取得
 * @param	st	状況ワークへのポインタ
 * @return	LOCATION	特殊接続情報へのポインタ
 */
//----------------------------------------------------------
LOCATION * Situation_GetSpecialLocation(SITUATION * st)
{
	return &st->special_loc;
}

//----------------------------------------------------------
/**
 * @brief	脱出用接続情報の取得
 * @param	st	状況ワークへのポインタ
 * @return	LOCATION	脱出用接続情報へのポインタ
 */
//----------------------------------------------------------
LOCATION * Situation_GetEscapeLocation(SITUATION * st)
{
	return &st->escape_loc;
}

//----------------------------------------------------------
/**
 * @brief ワープ接続先IDの取得
 * @param	st	状況ワークへのポインタ
 * @return  u16 * ワープID格納エリアへのポインタ
 */
//----------------------------------------------------------
u16 * Situation_GetWarpID(SITUATION * st)
{
  return &st->warp_id;
}

//============================================================================================
//
//	セーブデータ取得のための関数
//
//============================================================================================
//----------------------------------------------------------
/**
 * @brief	状況データへのポインタ取得
 * @param	sv			セーブデータ保持ワークへのポインタ
 * @return	SITUATION	状況保持ワークへのポインタ
 */
//----------------------------------------------------------
SITUATION * SaveData_GetSituation(SAVE_CONTROL_WORK * sv)
{
	return SaveControl_DataPtrGet(sv, GMDATA_ID_SITUATION);
}


//==============================================================================
//	
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   PLAYER_WORK構造体のデータを元にSITUATIONを更新する
 *
 * @param   sv		セーブデータ保持ワークへのポインタ
 * @param   pw		PLAYER_DATA
 */
//--------------------------------------------------------------
void SaveData_SituationDataUpdate(SAVE_CONTROL_WORK *sv, const PLAYER_WORK *pw)
{
	SITUATION *st = SaveData_GetSituation(sv);
	
	st->plsv.zoneID = pw->zoneID;
	st->plsv.position = pw->position;
	st->plsv.railposition = pw->railposition;
	st->plsv.direction = pw->direction;
  st->plsv.pos_type = pw->pos_type;
}

//--------------------------------------------------------------
/**
 * @brief   セーブデータからPALYER_WORKが必要な情報をロードしてくる
 *
 * @param   sv		セーブデータ保持ワークへのポインタ
 * @param   pw		PLAYER_DATA
 */
//--------------------------------------------------------------
void SaveData_SituationDataLoad(SAVE_CONTROL_WORK *sv, PLAYER_WORK *pw)
{
	SITUATION *st = SaveData_GetSituation(sv);
	
	pw->zoneID = st->plsv.zoneID;
	pw->position = st->plsv.position;
	pw->railposition = st->plsv.railposition;
	pw->direction = st->plsv.direction;
	pw->pos_type = st->plsv.pos_type;
}

//--------------------------------------------------------------
/**
 * @brief   SITUATIONセーブデータからPLAYERWORK_SAVEのみロードする
 *
 * @param   sv			
 * @param   plsv		代入先
 */
//--------------------------------------------------------------
void SaveData_SituationLoad_PlayerWorkSave(SAVE_CONTROL_WORK *sv, PLAYERWORK_SAVE *plsv)
{
	SITUATION *st = SaveData_GetSituation(sv);
	
	*plsv = st->plsv;
}


//----------------------------------------------------------------------------
/**
 *	@brief  フィールドステータス  のセーブ情報を更新
 *
 *	@param	sv
 *	@param	status 
 */
//-----------------------------------------------------------------------------
void SaveData_SituationDataUpdateStatus(SITUATION * st, const FIELD_STATUS * status )
{
  // フラッシュフラグ
  st->fs_flash = FIELD_STATUS_IsFieldSkillFlash( status );
}

//----------------------------------------------------------------------------
/**
 *	@brief  フィールドステータス　のセーブ情報を取得
 */
//-----------------------------------------------------------------------------
void SaveData_SituationDataLoadStatus(const SITUATION * st, FIELD_STATUS* status)
{
  // フラッシュフラグ
  FIELD_STATUS_SetFieldSkillFlash( status, st->fs_flash );
}



//----------------------------------------------------------------------------
/**
 *	@brief  季節 のセーブ情報を更新
 *
 *	@param	sv
 *	@param	u8
 */
//-----------------------------------------------------------------------------
void SaveData_SituationUpdateSeasonID(SITUATION * st, u8 season)
{
  st->season_id = season;
}

//----------------------------------------------------------------------------
/**
 *	@brief  季節 のセーブ情報を取得
 */
//-----------------------------------------------------------------------------
void SaveData_SituationLoadSeasonID(SITUATION * st, u8 * season)
{
  *season = st->season_id;
}

//----------------------------------------------------------------------------
/**
 *	@brief  天気IDを保存
 *
 *	@param	st        セーブワーク
 *	@param	weather   天気ID
 */
//-----------------------------------------------------------------------------
void SaveData_SituationUpdateWeatherID(SITUATION * st, u8 weather)
{
  st->weather_id = weather;
}

//----------------------------------------------------------------------------
/**
 *	@brieif 天気IDを取得
 *
 *	@param	st        セーブワーク
 *	@param	weather   天気ID
 */
//-----------------------------------------------------------------------------
void SaveData_SituationLoadWeatherID(SITUATION * st, u8 * weather)
{
  *weather = st->weather_id;
}

//----------------------------------------------------------------------------
/**
 *	@brieif タマゴ孵化カウンタを保存
 *
 *	@param	st        セーブワーク
 *	@param	count     カウンタの値
 */
//-----------------------------------------------------------------------------
void SaveData_SituationUpdateEggStepCount(SITUATION * st, u32 count)
{
  st->egg_step_count = count;
}

//----------------------------------------------------------------------------
/**
 *	@brieif タマゴ孵化カウンタを取得
 *
 *	@param	st        セーブワーク
 *	@param	count     カウンタの値の格納先
 */
//-----------------------------------------------------------------------------
void SaveData_SituationLoadEggStepCount(SITUATION * st, u32 * count)
{
  *count = st->egg_step_count;
}
