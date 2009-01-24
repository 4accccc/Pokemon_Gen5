//============================================================================================
/**
 * @file	field_sub_c3.c
 * @brief	フィールドメイン処理サブ（グリッド無し、C3実験用）
 *
 * このソースはfieldmap.cにインクルードされています。
 * 最終的にはちゃんと分割管理されますが、実験環境のために
 * しばらくはこの形式です。
 */
//============================================================================================

#include "field_easytp.h"


static void CalcPos(VecFx32 * pos, const VecFx32 * center, u16 len, u16 dir);

static u16 player_len;
static s16 mv_value;
static u16 pos_angle;
//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	初期化処理（グリッド無し）
 */
//------------------------------------------------------------------
static void TestC3Create( FIELD_MAIN_WORK * fieldWork, VecFx32 * pos, u16 dir)
{
	fieldWork->camera_control = FLD_CreateCamera( fieldWork->gs, fieldWork->heapID );
	fieldWork->fldActCont = FLD_CreateFieldActSys( fieldWork->gs, fieldWork->heapID );
	//FLDACT_TestSetup( fieldWork->fldActCont );
	fieldWork->pcActCont = CreatePlayerAct( fieldWork->gs, fieldWork->heapID );
	SetPlayerActTrans( fieldWork->pcActCont, pos );
	SetPlayerActDirection( fieldWork->pcActCont, &dir );

	{
		FIELD_CAMERA * cam = fieldWork->camera_control;
		u16 len = 0x0308;
		u16 dir = 0;
		fx32 height = 0x2000;
		VecFx32 trans = {0x2f6f36, 0, 0x301402};

		FLD_SetCameraLength(cam, 0x0308);
		FLD_SetCameraDirection(cam, &dir);
		FLD_SetCameraHeight(cam, height);
		FLD_SetCameraTrans(cam, &trans);

		player_len = 0x200;
		mv_value = 16;
	}
	if (0){
		u16 len;
		fx32 height;
		FLD_GetCameraLength(fieldWork->camera_control, &len);
		len += 0x0080;
		FLD_SetCameraLength(fieldWork->camera_control, len);

		FLD_GetCameraHeight(fieldWork->camera_control, &height);
		height += 0x0003a000;
		FLD_SetCameraHeight(fieldWork->camera_control, height);
	}
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void TestC3Main( FIELD_MAIN_WORK* fieldWork, VecFx32 * pos )
{
	FLDEASYTP_TCHDIR tp_dir = FieldEasyTP_TouchDirGet();

	{
		switch (tp_dir) {
		case FLDEASYTP_TCHDIR_LEFT:		mv_value -= 8;	break;
		case FLDEASYTP_TCHDIR_RIGHT:	mv_value += 8;	break;
		case FLDEASYTP_TCHDIR_UP:		player_len += 8;	break;
		case FLDEASYTP_TCHDIR_DOWN:		player_len -= 8;	break;
		}
		if (mv_value <= 0) {mv_value = 8;}
		if (mv_value > FX32_ONE / 16) { mv_value = FX32_ONE / 16; }
	}
	{
		u16 dir;
		FLD_GetCameraDirection(fieldWork->camera_control, &dir);
		if (fieldWork->key_cont & PAD_KEY_LEFT) {
			dir -= mv_value;
		}
		if (fieldWork->key_cont & PAD_KEY_RIGHT) {
			dir += mv_value;
		}
		FLD_SetCameraDirection(fieldWork->camera_control, &dir);
	}
	{
		VecFx32 cam, player_pos;
		u16 dir;
		FLD_GetCameraDirection( fieldWork->camera_control, &dir);
		FLD_GetCameraTrans( fieldWork->camera_control, &cam);
		CalcPos(&player_pos, &cam, player_len, dir);
		SetPlayerActTrans( fieldWork->pcActCont, &player_pos );
	}
	//MainPlayerAct_NoGrid( fieldWork->pcActCont, fieldWork->key_cont );
	FLD_MainFieldActSys( fieldWork->fldActCont );
	
	GetPlayerActTrans( fieldWork->pcActCont, pos );

	//FLD_SetCameraTrans( fieldWork->camera_control, pos );
	//FLD_SetCameraDirection( fieldWork->camera_control, &dir );

	FLD_MainCamera( fieldWork->camera_control, fieldWork->key_cont );
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
	if (tp_dir == FLDEASYTP_TCHDIR_DOWN) {
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
static void TestC3Delete( FIELD_MAIN_WORK* fieldWork )
{
	DeletePlayerAct( fieldWork->pcActCont );
	FLD_DeleteCamera( fieldWork->camera_control );
	//FLDACT_TestRelease( fieldWork->fldActCont );
	FLD_DeleteFieldActSys( fieldWork->fldActCont );
}

//============================================================================================
//============================================================================================
#define HEIGHT	0x2000
//------------------------------------------------------------------
//------------------------------------------------------------------
static void CalcPos(VecFx32 * pos, const VecFx32 * center, u16 len, u16 dir)
{
	pos->x = center->x + len * FX_SinIdx(dir);
	pos->y = center->y + HEIGHT;
	pos->z = center->z + len * FX_CosIdx(dir);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
