//============================================================================================
/**
 * @file	field_sub_nogrid.c
 * @brief	フィールドメイン処理サブ（グリッド無し用）
 *
 * このソースはfield_main.cにインクルードされています。
 * 最終的にはちゃんと分割管理されますが、実験環境のために
 * しばらくはこの形式です。
 */
//============================================================================================

#include "field_easytp.h"
//------------------------------------------------------------------
/**
 * @brief	初期化処理（グリッド無し）
 */
//------------------------------------------------------------------
static void NoGridCreate( FIELD_MAIN_WORK * fieldWork, VecFx32 * pos, u16 dir)
{
#if 0
	fieldWork->pcActCont = CreatePlayerAct( fieldWork, pos, fieldWork->heapID );
	SetPlayerActTrans( fieldWork->pcActCont, pos );
	SetPlayerActDirection( fieldWork->pcActCont, &dir );
#endif
	fieldWork->pcActCont = CreatePlayerActGrid(fieldWork, pos, fieldWork->heapID);
	SetGridPlayerActTrans( fieldWork->pcActCont, pos);
	SetPlayerActDirection( fieldWork->pcActCont, &dir );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void NoGridMain( FIELD_MAIN_WORK* fieldWork, VecFx32 * pos )
{
	MainPlayerAct_NoGrid( fieldWork->pcActCont, fieldWork->key_cont );
	
	GetPlayerActTrans( fieldWork->pcActCont, pos );
	SetGridPlayerActTrans( fieldWork->pcActCont, pos);
}

//------------------------------------------------------------------
/**
 * @brief	終了処理（グリッド無し）
 */
//------------------------------------------------------------------
static void NoGridDelete( FIELD_MAIN_WORK* fieldWork )
{
	DeletePlayerActGrid(fieldWork->pcActCont);
	//DeletePlayerAct( fieldWork->pcActCont );
}

