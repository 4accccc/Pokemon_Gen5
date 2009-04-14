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
	fieldWork->fldActCont = FLD_CreateFieldActSys( fieldWork, fieldWork->heapID );
	fieldWork->pcActCont = CreatePlayerAct( fieldWork, fieldWork->heapID );
	SetPlayerActTrans( fieldWork->pcActCont, pos );
	SetPlayerActDirection( fieldWork->pcActCont, &dir );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void NoGridMain( FIELD_MAIN_WORK* fieldWork, VecFx32 * pos )
{
	MainPlayerAct_NoGrid( fieldWork->pcActCont, fieldWork->key_cont );
	FLD_MainFieldActSys( fieldWork->fldActCont );
	
	GetPlayerActTrans( fieldWork->pcActCont, pos );

	if (FieldEasyTP_TouchDirGet() == FLDEASYTP_TCHDIR_DOWN) {
		VecFx32 trans;
		FLDMAPPER_GRIDINFO gridInfo;
		int i;
		GetPlayerActTrans(fieldWork->pcActCont, &trans);
		FLDMAPPER_GetGridInfo( GetFieldG3Dmapper(fieldWork), &trans, &gridInfo);
		OS_Printf("gridInfo.count = %d\n", gridInfo.count);
		for (i = 0; i < gridInfo.count; i++) {
			OS_Printf("[%02d]%08x\n",i, gridInfo.gridData[i].height);
		}
	}
}

//------------------------------------------------------------------
/**
 * @brief	終了処理（グリッド無し）
 */
//------------------------------------------------------------------
static void NoGridDelete( FIELD_MAIN_WORK* fieldWork )
{
	DeletePlayerAct( fieldWork->pcActCont );
	FLD_DeleteFieldActSys( fieldWork->fldActCont );
}

