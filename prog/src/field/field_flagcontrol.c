//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_flagcontrol.h
 *	@brief  フィールド　フラグ管理
 *	@author	tomoya takahashi
 *	@date		2009.11.19
 *
 *	モジュール名：FIELD_FLAGCONT
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "gflib.h"

#include "field_flagcontrol.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------


//----------------------------------------------------------------------------
/**
 *	@brief  フラグ初期化：歩いてゾーンをまたいだとき  (必ず呼ばれる)
 *
 *	@param	gdata ゲームデータ
 *	@param  fieldWork フィールドマップワーク
 */
//-----------------------------------------------------------------------------
void FIELD_FLAGCONT_INIT_WalkStepOver(GAMEDATA * gdata, FIELDMAP_WORK* fieldWork)
{

/* PLATUNUM
	//--冒険ノート用更新処理
	FldFlgInit_FnoteTownDataSet( fsys );

	//自転車ゲート通過したフラグのクリア(ゲート内でのみ有効)
	SysFlag_BicycleGatePassage( SaveData_GetEventWork(fsys->savedata), SYSFLAG_MODE_RESET );

	//かいりきフラグのクリア
	SysFlag_KairikiReset( SaveData_GetEventWork(fsys->savedata) );
	//到着フラグのセット
	ARRIVEDATA_SetArriveFlag(fsys, fsys->location->zone_id);
	//ビードロクリア
	EncDataSave_SetVidro( EncDataSave_GetSaveDataPtr(fsys->savedata), VIDRO_NONE );
	//エンカウント歩数クリア
	fsys->encount.walk_count = 0;
	//サファリにいないときの処理
	if ( !SysFlag_SafariCheck(SaveData_GetEventWork(fsys->savedata)) ){
		//移動ポケモン処理(サファリ内ゾーン切り替えのときは履歴を積まない)
		ENC_SV_PTR data;
		data = EncDataSave_GetSaveDataPtr(fsys->savedata);
		MP_UpdatePlayerZoneHist(data, fsys->location->zone_id);		//ゾーン履歴を更新
		//歩いて移動した場合は移動ポケモンの移動処理
		MP_MovePokemonNeighboring(data);//移動ポケモン隣接移動
	}
//*/
}

//----------------------------------------------------------------------------
/**
 *	@brief  フラグ初期化：ワイプの入るマップジャンプのとき  (必ず呼ばれる)
 *
 *	@param	fsys  フィールドマップワーク
 */
//-----------------------------------------------------------------------------
void FIELD_FLAGCONT_INIT_MapJump(GAMEDATA * gdata)
{
/* PLATINUM
	if (fsys->scope_mode_flag == TRUE) {
		//望遠鏡モードの場合は更新処理を行わない
		return;
	}

	//--冒険ノート用更新処理
	if( FldFlgInit_FnoteTownDataSet( fsys ) == FALSE ){
		FldFlgInit_FnoteMapChgDataSet( fsys );
	}

	//自転車ゲート通過したフラグのクリア(ゲート内でのみ有効)
	SysFlag_BicycleGatePassage( SaveData_GetEventWork(fsys->savedata), SYSFLAG_MODE_RESET );

	//サイクリングロードモードのクリア
	SysFlag_CyclingRoadReset( SaveData_GetEventWork(fsys->savedata) );
	//かいりきフラグのクリア
	SysFlag_KairikiReset( SaveData_GetEventWork(fsys->savedata) );
	//到着フラグのセット
	ARRIVEDATA_SetArriveFlag(fsys, fsys->location->zone_id);
	//ビードロクリア
	EncDataSave_SetVidro( EncDataSave_GetSaveDataPtr(fsys->savedata), VIDRO_NONE );
	//エンカウント歩数クリア
	fsys->encount.walk_count = 0;
	//移動ポケモン処理
	{
		ENC_SV_PTR data;
		data = EncDataSave_GetSaveDataPtr(fsys->savedata);
		MP_UpdatePlayerZoneHist(data, fsys->location->zone_id);		//ゾーン履歴を更新
	}
	//ダンジョンでない場合、フラッシュ状態フラグと霧払い状態フラグをクリア
	if (!ZoneData_IsDungeon(fsys->location->zone_id)) {
		EVENTWORK * ev = SaveData_GetEventWork(fsys->savedata);
		SysFlag_FlashReset(ev);
		SysFlag_KiribaraiReset(ev);
	}
	//自機状態の設定
	{
		PLAYER_SAVE_DATA * jikisave = Situation_GetPlayerSaveData(SaveData_GetSituation(fsys->savedata));
		if (jikisave->form == HERO_FORM_CYCLE
				&& ZoneData_GetEnableBicycleFlag(fsys->location->zone_id) == FALSE) {
			jikisave->form = HERO_FORM_NORMAL;
		} else if (jikisave->form == HERO_FORM_SWIM) {
			jikisave->form = HERO_FORM_NORMAL;
		}
	}

	//--タウンマップ足跡
	if (ZoneData_IsSinouField(fsys->location->zone_id)) {
		//フィールド以外では足跡の更新はない！
		TOWN_MAP_FOOTMARK * footmark;
		footmark = Situation_GetTMFootMark(SaveData_GetSituation(fsys->savedata));
		TMFootMark_SetNoDir(footmark, fsys->location->grid_x, fsys->location->grid_z);
	}
//*/
}

//----------------------------------------------------------------------------
/**
 * @brief	フラグ初期化：空を飛ぶを使ったとき固有で行うもの
 *
 * @param	fsys		フィールドシステムポインタ
 *
 */
//-----------------------------------------------------------------------------

void FIELD_FLAGCONT_INIT_FlySky(GAMEDATA * gdata)
{
/*  PLATINUM
	//サファリフラグクリア
	SysFlag_SafariReset( SaveData_GetEventWork(fsys->savedata) );
	//移動ポケモン全部ジャンプ
	MP_JumpMovePokemonAll(EncDataSave_GetSaveDataPtr(fsys->savedata));
//*/
}

//----------------------------------------------------------------------------
/**
 * @brief	フラグ初期化：テレポートを使ったとき固有で行うもの
 *
 * @param	fsys		フィールドシステムポインタ
 */
//-----------------------------------------------------------------------------
void FIELD_FLAGCONT_INIT_Teleport(GAMEDATA * gdata)
{
/*  PLATINUM
	//サファリフラグクリア
	SysFlag_SafariReset( SaveData_GetEventWork(fsys->savedata) );
	//移動ポケモン全部ジャンプ
	MP_JumpMovePokemonAll(EncDataSave_GetSaveDataPtr(fsys->savedata));
//*/
}

//----------------------------------------------------------------------------
/**
 * @brief	フラグ初期化：あなぬけのひも・あなをほるを使ったとき固有で行うもの
 *
 * @param	fsys		フィールドシステムポインタ
 */
//-----------------------------------------------------------------------------

void FIELD_FLAGCONT_INIT_Escape(GAMEDATA * gdata)
{
/*  PLATINUM
	//サファリフラグクリア
	SysFlag_SafariReset( SaveData_GetEventWork(fsys->savedata) );
//*/
}


//----------------------------------------------------------------------------
/**
 * @brief	フラグ初期化：ゲームオーバー
 *
 * @param	fsys		フィールドシステムポインタ
 */
//-----------------------------------------------------------------------------
void FIELD_FLAGCONT_INIT_GameOver(GAMEDATA * gdata)
{
/*  PLATINUM
	EVENTWORK* ev = SaveData_GetEventWork( fsys->savedata );

	//連れ歩きフラグ、トレーナーID格納ワークのクリア
	SysFlag_PairReset( ev );
	SysWork_PairTrainerIDSet( ev, 0 );
//*/
}


//----------------------------------------------------------------------------
/**
 * @brief	フラグ初期化：コンティニュー
 *
 * @param	fsys		フィールドシステムポインタ
 */
//-----------------------------------------------------------------------------
void FIELD_FLAGCONT_INIT_Continue(GAMEDATA * gdata)
{
/*
	//移動ポケモン全部ジャンプ
	MP_JumpMovePokemonAll(EncDataSave_GetSaveDataPtr(fsys->savedata));
//*/
}

