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
	{
		u16 len;
		fx32 height;
		FLD_GetCameraLength(fieldWork->camera_control, &len);
		len += 0x0080;
		FLD_SetCameraLength(fieldWork->camera_control, len);
		FLD_GetCameraHeight(fieldWork->camera_control, &height);
		height += 0x0003a000;
		FLD_SetCameraHeight(fieldWork->camera_control, height);
	}
	{
		fx32 far = 4096 << FX32_SHIFT;
		GFL_G3D_CAMERA_SetFar(fieldWork->g3Dcamera, &far);
	}
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void NoGridMain( FIELD_MAIN_WORK* fieldWork, VecFx32 * pos )
{
	MainPlayerAct_NoGrid( fieldWork->pcActCont, fieldWork->key_cont );
	FLD_MainFieldActSys( fieldWork->fldActCont );
	
	GetPlayerActTrans( fieldWork->pcActCont, pos );

	FIELD_CAMERA_SetPos( fieldWork->camera_control, pos );

	FIELD_CAMERA_DEBUG_Control( fieldWork->camera_control, fieldWork->key_cont );

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

