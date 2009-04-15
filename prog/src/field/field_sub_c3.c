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

typedef struct {
	u16 player_len;
	u16 pos_angle;
	s16 df_len;
	s16 df_angle;
}C3_MOVE_WORK;

static C3_MOVE_WORK C3MoveWork;

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	初期化処理（グリッド無し）
 */
//------------------------------------------------------------------
static void TestC3Create( FIELD_MAIN_WORK * fieldWork, VecFx32 * pos, u16 dir)
{
	static const C3_MOVE_WORK init = {
		 0x1f0,
		 0,
		 1,
		 16
	};
	C3MoveWork = init;
	fieldWork->pcActCont = CreatePlayerAct( fieldWork, fieldWork->heapID );
	SetPlayerActTrans( fieldWork->pcActCont, pos );
	SetPlayerActDirection( fieldWork->pcActCont, &dir );

}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void TestC3Main( FIELD_MAIN_WORK* fieldWork, VecFx32 * pos )
{
	C3_MOVE_WORK * mwk = &C3MoveWork;

#ifdef	PM_DEBUG
	{
		FLDEASYTP_TCHDIR tp_dir = FieldEasyTP_TouchDirGet();
		switch (tp_dir) {
		case FLDEASYTP_TCHDIR_LEFT:		mwk->df_angle -= 4;	break;
		case FLDEASYTP_TCHDIR_RIGHT:	mwk->df_angle += 4;	break;
		case FLDEASYTP_TCHDIR_UP:		mwk->df_len += 1;	break;
		case FLDEASYTP_TCHDIR_DOWN:		mwk->df_len -= 1;	break;
		}
		if (mwk->df_angle <= 0) {mwk->df_angle = 4;}
		if (mwk->df_angle > FX32_ONE / 16) { mwk->df_angle = FX32_ONE / 16; }
		if (mwk->df_len <= 0) {mwk->df_len = 1; }
		if (mwk->df_len > 8) {mwk->df_len = 8; }
		if (tp_dir == FLDEASYTP_TCHDIR_CENTER) {
			TAMADA_Printf("LEN %04x vec:%04x\n", mwk->player_len, mwk->df_len);
			TAMADA_Printf("ANGLE %04x vec:%04x\n", mwk->pos_angle, mwk->df_angle);
		}
	}
#endif

	{
		if (fieldWork->key_cont & PAD_KEY_LEFT) {
			mwk->pos_angle -= mwk->df_angle;
		}
		if (fieldWork->key_cont & PAD_KEY_RIGHT) {
			mwk->pos_angle += mwk->df_angle;
		}
		if (fieldWork->key_cont & PAD_KEY_UP) {
			mwk->player_len -= mwk->df_len;
		}
		if (fieldWork->key_cont & PAD_KEY_DOWN) {
			mwk->player_len += mwk->df_len;
		}
	}

	{
		VecFx32 cam, player_pos;
		FIELD_CAMERA_GetTargetPos( fieldWork->camera_control, &cam);
		CalcPos(&player_pos, &cam, mwk->player_len, mwk->pos_angle);
		SetPlayerActTrans( fieldWork->pcActCont, &player_pos );
	}
	FIELD_CAMERA_SetDirectionOnXZ(fieldWork->camera_control, mwk->pos_angle);
	MainPlayerAct_C3( fieldWork->pcActCont, fieldWork->key_cont, mwk->pos_angle );
	

}

//------------------------------------------------------------------
/**
 * @brief	終了処理（グリッド無し）
 */
//------------------------------------------------------------------
static void TestC3Delete( FIELD_MAIN_WORK* fieldWork )
{
	DeletePlayerAct( fieldWork->pcActCont );
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
