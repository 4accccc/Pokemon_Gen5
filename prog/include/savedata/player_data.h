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

//--------------------------------------------------------------
//  初回セーブセットアップ管理　
//      ※check 下記の関数はsave_control.c, save_control_intr.cでしか使用しないはず。
//              マスターアップ前にも他の箇所で意図せず使用されていないか確認する
//--------------------------------------------------------------
extern BOOL SaveData_GetNowSaveModeSetup(SAVE_CONTROL_WORK *sv);
extern void SaveData_SetNowSaveModeSetupON(SAVE_CONTROL_WORK *sv);
extern void SaveData_SetNowSaveModeSetupOFF(SAVE_CONTROL_WORK *sv);


#endif	//__PLAYER_DATA_H__
