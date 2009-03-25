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
	fieldWork->camera_control = FIELD_CAMERA_Create( fieldWork->gs, fieldWork->heapID );
	fieldWork->fldActCont = FLD_CreateFieldActSys( fieldWork->gs, fieldWork->heapID );
	//FLDACT_TestSetup( fieldWork->fldActCont );
	fieldWork->pcActCont = CreatePlayerAct( fieldWork->gs, fieldWork->heapID );
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
		GFL_G3D_CAMERA_SetFar(GetG3Dcamera(fieldWork->gs), &far);
	}
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void NoGridMain( FIELD_MAIN_WORK* fieldWork, VecFx32 * pos )
{
	MainPlayerAct_NoGrid( fieldWork->pcActCont, fieldWork->key_cont );
	FLD_MainFieldActSys( fieldWork->fldActCont );
	
	GetPlayerActTrans( fieldWork->pcActCont, pos );

	FLD_SetCameraTrans( fieldWork->camera_control, pos );
	//FLD_SetCameraDirection( fieldWork->camera_control, &dir );

	FIELD_CAMERA_Main( fieldWork->camera_control, fieldWork->key_cont );
#if 0
	{
		GFL_G3D_CAMERA * g3Dcamera = GetG3Dcamera(fieldWork->gs);
		VecFx32 target, c_pos;
		c_pos = *pos;

		VEC_Set(	&target,
					c_pos.x,
					c_pos.y + CAMERA_TARGET_HEIGHT*FX32_ONE,
					c_pos.z);
		c_pos.x = pos->x + 16 * FX_SinIdx(0);
		c_pos.y = pos->y;
		c_pos.z = pos->z + 16 * FX_CosIdx(0);

		GFL_G3D_CAMERA_SetTarget( g3Dcamera, &target );
		GFL_G3D_CAMERA_SetPos( g3Dcamera, &c_pos );
	}
#endif
	if (FieldEasyTP_TouchDirGet() == FLDEASYTP_TCHDIR_DOWN) {
		VecFx32 trans;
		FLDMAPPER_GRIDINFO gridInfo;
		int i;
		GetPlayerActTrans(fieldWork->pcActCont, &trans);
		FLDMAPPER_GetGridInfo( GetFieldG3Dmapper(fieldWork->gs), &trans, &gridInfo);
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
	FIELD_CAMERA_Delete( fieldWork->camera_control );
	//FLDACT_TestRelease( fieldWork->fldActCont );
	FLD_DeleteFieldActSys( fieldWork->fldActCont );
}

