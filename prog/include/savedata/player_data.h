//==============================================================================
/**
 * @file	player_data.h
 * @brief	プレイヤー関連セーブデータのヘッダ
 * @author	matsuda
 * @date	2009.01.08(木)
 */
//==============================================================================
#ifndef __PLAYER_DATA_H__
#define __PLAYER_DATA_H__

#include "savedata/save_control.h"	//SAVE_CONTROL_WORK参照のため
#include "gamesystem/playerwork.h"


//==============================================================================
//	外部関数宣言
//==============================================================================
extern void SaveData_PlayerDataUpdate(SAVE_CONTROL_WORK * sv, const PLAYER_WORK *pw);
extern void SaveData_PlayerDataLoad(SAVE_CONTROL_WORK * sv, PLAYER_WORK *pw);


#endif	//__PLAYER_DATA_H__
