//============================================================================================
/**
 * @file	player_data.c
 * @date	2006.06.01
 * @author	tamada
 * @brief	プレイヤー関連セーブデータ
 */
//============================================================================================
#include <gflib.h>
#include "savedata/config.h"
#include "savedata/playerdata_local.h"
#include "savedata/mystatus.h"
#include "mystatus_local.h"
#include "playtime_local.h"
#include "savedata/playtime.h"
#include "savedata/save_tbl.h"
#include "gamesystem/playerwork.h"
#include "savedata/player_data.h"


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	プレイヤーセーブデータの定義
 */
//------------------------------------------------------------------
struct PLAYER_DATA {
	CONFIG config;
	MYSTATUS mystatus;
	PLAYTIME playtime;
	u8 now_save_mode_setup;    ///<TRUE:初回セットアップしかしていない状態(FALSE:正規のセーブ移行はずっとFALSE状態)
	u8 padding[3];
};

//============================================================================================
//
//		主にセーブシステムから呼ばれる関数
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	セーブデータサイズの取得
 * @return	int		プレイヤーセーブデータのサイズ
 */
//------------------------------------------------------------------
int PLAYERDATA_GetWorkSize(void)
{
	return sizeof(PLAYER_DATA);
}

//------------------------------------------------------------------
/**
 * @brief	セーブデータ初期化
 * @param	pd		プレイヤーセーブデータへのポインタ
 */
//------------------------------------------------------------------
void PLAYERDATA_Init(PLAYER_DATA * pd)
{
	GFL_STD_MemClear32(pd, sizeof(PLAYER_DATA));
	/* 以下に個別部分の初期化処理をおく */
	CONFIG_Init(&pd->config);
	MyStatus_Init(&pd->mystatus);
	PLAYTIME_Init(&pd->playtime);
	pd->now_save_mode_setup = FALSE;
}

//============================================================================================
//
//	セーブデータ取得のための関数
//
//============================================================================================
//---------------------------------------------------------------------------
/**
 * @brief	自分状態データへのポインタ取得
 * @param	sv			セーブデータ保持ワークへのポインタ
 * @return	MYSTATUS	自分状態保持ワークへのポインタ
 */
//---------------------------------------------------------------------------
MYSTATUS * SaveData_GetMyStatus(SAVE_CONTROL_WORK * sv)
{
	PLAYER_DATA * pd;
	pd = SaveControl_DataPtrGet(sv, GMDATA_ID_PLAYER_DATA);
	return &pd->mystatus;
}

//---------------------------------------------------------------------------
/**
 * @brief	設定情報データへのポインタ取得
 * @param	sv			セーブデータ保持ワークへのポインタ
 * @return	CONFIG		設定情報保持ワークへのポインタ
 */
//---------------------------------------------------------------------------
CONFIG * SaveData_GetConfig(SAVE_CONTROL_WORK * sv)
{
	PLAYER_DATA * pd;
	pd = SaveControl_DataPtrGet(sv, GMDATA_ID_PLAYER_DATA);
	return &pd->config;
}

//---------------------------------------------------------------------------
/**
 * @brief	プレイ時間データへのポインタを取得
 * @param	sv			セーブデータ保持ワークへのポインタ
 * @return	playtime	プレイ時間データへのポインタ
 */
//---------------------------------------------------------------------------
PLAYTIME * SaveData_GetPlayTime(SAVE_CONTROL_WORK * sv)
{
	PLAYER_DATA * pd;
	pd = SaveControl_DataPtrGet(sv, GMDATA_ID_PLAYER_DATA);
	return &pd->playtime;
}

//==================================================================
/**
 * 現在のセーブ状態が初回セットアップしかされていない状況なのか調べる
 *
 * @param   sv		
 *
 * @retval  BOOL		TRUE:初回セットアップしかされていない
 *                      ※初回移行、一度でもセーブすれば以後、ずっとFALSEになる
 */
//==================================================================
BOOL SaveData_GetNowSaveModeSetup(SAVE_CONTROL_WORK *sv)
{
	PLAYER_DATA * pd = SaveControl_DataPtrGet(sv, GMDATA_ID_PLAYER_DATA);
	return pd->now_save_mode_setup;
}

//==================================================================
/**
 * 現在のセーブ状態を「初回セットアップしかされていない」状況にする
 *
 * @param   sv		
 */
//==================================================================
void SaveData_SetNowSaveModeSetupON(SAVE_CONTROL_WORK *sv)
{
	PLAYER_DATA * pd = SaveControl_DataPtrGet(sv, GMDATA_ID_PLAYER_DATA);
	pd->now_save_mode_setup = TRUE;
}

//==================================================================
/**
 * 初回セットアップフラグをOFFにし、現在のセーブ状態を普通にする
 *
 * @param   sv		
 */
//==================================================================
void SaveData_SetNowSaveModeSetupOFF(SAVE_CONTROL_WORK *sv)
{
	PLAYER_DATA * pd = SaveControl_DataPtrGet(sv, GMDATA_ID_PLAYER_DATA);
	pd->now_save_mode_setup = FALSE;
}


//==============================================================================
//	
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   PLAYER_WORK構造体のデータを元にPLAYER_DATAを更新する
 *
 * @param   pw		PLAYER_DATA構造体
 *
 * フィールドのセーブ前などにこれを呼び出す必要がある
 */
//--------------------------------------------------------------
void SaveData_PlayerDataUpdate(SAVE_CONTROL_WORK * sv, const PLAYER_WORK *pw)
{
	PLAYER_DATA * pd = SaveControl_DataPtrGet(sv, GMDATA_ID_PLAYER_DATA);

	pd->mystatus = pw->mystatus;
}

//--------------------------------------------------------------
/**
 * @brief   セーブデータからPLAYER_WORK構造体の情報をロードする
 *
 * @param   pw		PLAYER_DATA構造体
 */
//--------------------------------------------------------------
void SaveData_PlayerDataLoad(SAVE_CONTROL_WORK * sv, PLAYER_WORK *pw)
{
	PLAYER_DATA * pd = SaveControl_DataPtrGet(sv, GMDATA_ID_PLAYER_DATA);

	pw->mystatus = pd->mystatus;
}
