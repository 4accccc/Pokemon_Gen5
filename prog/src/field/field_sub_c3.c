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
#include "field_player_nogrid.h"

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
#if 0
	fieldWork->field_player = CreatePlayerAct( fieldWork, pos, fieldWork->heapID );
	SetPlayerActTrans( fieldWork->field_player, pos );
	FIELD_PLAYER_SetDir( fieldWork->field_player, &dir );
#endif
	FIELD_PLAYER_SetPos( fieldWork->field_player, pos);
	FIELD_PLAYER_SetDir( fieldWork->field_player, dir );

}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void TestC3Main( FIELD_MAIN_WORK* fieldWork, VecFx32 * pos )
{
	C3_MOVE_WORK * mwk = &C3MoveWork;

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
		//SetPlayerActTrans( fieldWork->field_player, &player_pos );
		FIELD_PLAYER_SetPos( fieldWork->field_player, &player_pos);
	}
	FIELD_CAMERA_SetDirectionOnXZ(fieldWork->camera_control, mwk->pos_angle);
	FIELD_PLAYER_C3_Move( fieldWork->field_player, fieldWork->key_cont, mwk->pos_angle );
}

//------------------------------------------------------------------
/**
 * @brief	終了処理（グリッド無し）
 */
//------------------------------------------------------------------
static void TestC3Delete( FIELD_MAIN_WORK* fieldWork )
{
	//DeletePlayerAct( fieldWork->field_player );
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
