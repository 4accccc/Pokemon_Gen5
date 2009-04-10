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
static s16 v_angle;
static u16 pos_angle;
static u16 v_len;
//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	初期化処理（グリッド無し）
 */
//------------------------------------------------------------------
static void TestC3Create( FIELD_MAIN_WORK * fieldWork, VecFx32 * pos, u16 dir)
{
	fieldWork->fldActCont = FLD_CreateFieldActSys( fieldWork, fieldWork->heapID );
	//FLDACT_TestSetup( fieldWork->fldActCont );
	fieldWork->pcActCont = CreatePlayerAct( fieldWork, fieldWork->heapID );
	SetPlayerActTrans( fieldWork->pcActCont, pos );
	SetPlayerActDirection( fieldWork->pcActCont, &dir );

	{
		FIELD_CAMERA * cam = fieldWork->camera_control;
		u16 len = 0x0340;
		u16 dir = 0;
		fx32 height = 0x7c000;
		VecFx32 trans = {0x2f6f36, 0, 0x301402};

		FLD_SetCameraLength(cam, 0x0308);
		FLD_SetCameraDirection(cam, &dir);
		FLD_SetCameraHeight(cam, height);
		FIELD_CAMERA_SetPos(cam, &trans);

		player_len = 0x1f0;
		v_len = 1;
		v_angle = 16;
		pos_angle = 0;
	}
	{
		//fx32 far = 1024 << FX32_SHIFT;
		fx32 far = (512 + 256 + 128) << FX32_SHIFT;
		GFL_G3D_CAMERA_SetFar(fieldWork->g3Dcamera, &far);
	}
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void TestC3Main( FIELD_MAIN_WORK* fieldWork, VecFx32 * pos )
{
	FLDEASYTP_TCHDIR tp_dir = FieldEasyTP_TouchDirGet();

	{
		switch (tp_dir) {
		case FLDEASYTP_TCHDIR_LEFT:		v_angle -= 4;	break;
		case FLDEASYTP_TCHDIR_RIGHT:	v_angle += 4;	break;
		case FLDEASYTP_TCHDIR_UP:		v_len += 1;	break;
		case FLDEASYTP_TCHDIR_DOWN:		v_len -= 1;	break;
		}
		if (v_angle <= 0) {v_angle = 4;}
		if (v_angle > FX32_ONE / 16) { v_angle = FX32_ONE / 16; }
		if (v_len <= 0) {v_len = 1; }
		if (v_len > 8) {v_len = 8; }
	}
	{
		u16 dir;
		FLD_GetCameraDirection(fieldWork->camera_control, &dir);
		if (fieldWork->key_cont & PAD_KEY_LEFT) {
			pos_angle -= v_angle;
			dir -= v_angle;
		}
		if (fieldWork->key_cont & PAD_KEY_RIGHT) {
			pos_angle += v_angle;
			dir += v_angle;
		}
		FLD_SetCameraDirection(fieldWork->camera_control, &dir);
	}
	{
		if (fieldWork->key_cont & PAD_KEY_UP) {
			player_len -= v_len;
		}
		if (fieldWork->key_cont & PAD_KEY_DOWN) {
			player_len += v_len;
		}
	}
	{
		VecFx32 cam, player_pos;
		FIELD_CAMERA_GetPos( fieldWork->camera_control, &cam);
		CalcPos(&player_pos, &cam, player_len, pos_angle);
		SetPlayerActTrans( fieldWork->pcActCont, &player_pos );
	}
	MainPlayerAct_C3( fieldWork->pcActCont, fieldWork->key_cont, pos_angle );
	FLD_MainFieldActSys( fieldWork->fldActCont );
	
	FIELD_CAMERA_DEBUG_Control( fieldWork->camera_control, fieldWork->key_cont );

	if (tp_dir == FLDEASYTP_TCHDIR_CENTER) {
		TAMADA_Printf("LEN %04x vec:%04x\n", player_len, v_len);
		TAMADA_Printf("ANGLE %04x vec:%04x\n", pos_angle, v_angle);

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
