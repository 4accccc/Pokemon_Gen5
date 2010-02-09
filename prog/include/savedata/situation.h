//==============================================================================
/**
 * @file	situation.h
 * @brief	状況データアクセスのヘッダ
 * @author	matsuda
 * @date	2009.01.08(木)
 */
//==============================================================================
#ifndef __SITUATION_H__
#define __SITUATION_H__

#include "savedata/save_control.h"	//SAVE_CONTROL_WORK参照のため
#include "gamesystem/playerwork.h"
#include "field/location.h"
#include "field/field_status.h"


//==============================================================================
//	型定義
//==============================================================================
///状況データの不定形アクセス型
typedef struct _SITUATION SITUATION;

//==============================================================================
//	構造体定義
//==============================================================================
///PLAYER_WORK構造体からセーブに必要なものだけをまとめた構造体
typedef struct{
	ZONEID zoneID;
	VecFx32 position;
  RAIL_LOCATION railposition;
	s16 direction;
  u8 pos_type;       ///<LOCATION_POS_TYPE
  u8 pad;
}PLAYERWORK_SAVE;


//==============================================================================
//	外部関数宣言
//==============================================================================
extern int Situation_GetWorkSize(void);
extern void Situation_Init(SITUATION * st);
extern LOCATION * Situation_GetStartLocation(SITUATION * st);
extern LOCATION * Situation_GetEntranceLocation(SITUATION * st);
extern LOCATION * Situation_GetSpecialLocation(SITUATION * st);
extern LOCATION * Situation_GetEscapeLocation(SITUATION * st);
extern u16 * Situation_GetWarpID( SITUATION * st );
extern SITUATION * SaveData_GetSituation(SAVE_CONTROL_WORK * sv);
extern void SaveData_SituationDataUpdate(SAVE_CONTROL_WORK *sv, const PLAYER_WORK *pw );
extern void SaveData_SituationDataLoad(SAVE_CONTROL_WORK *sv, PLAYER_WORK *pw);
extern void SaveData_SituationLoad_PlayerWorkSave(SAVE_CONTROL_WORK *sv, PLAYERWORK_SAVE *plsv);


extern void SaveData_SituationDataUpdateStatus(SITUATION * st, const FIELD_STATUS * status );
extern void SaveData_SituationDataLoadStatus(const SITUATION * st, FIELD_STATUS* status);

extern void SaveData_SituationUpdateSeasonID(SITUATION * st, u8 season);
extern void SaveData_SituationLoadSeasonID(SITUATION * st, u8 * season);
extern void SaveData_SituationUpdateWeatherID(SITUATION * st, u8 weather);
extern void SaveData_SituationLoadWeatherID(SITUATION * st, u8 * weather);

extern void SaveData_SituationUpdateEggStepCount(SITUATION * st, u32 count);
extern void SaveData_SituationLoadEggStepCount(SITUATION * st, u32 * count);

#endif	//__SITUATION_H__
