//======================================================================
/**
 * @file	fieldmap_ctrl_c3.c
 * @brief	フィールドマップ　コントロール　C3マップ処理
 * @author	tamada
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "fieldmap.h"
#include "field_player_nogrid.h"

#include "field_easytp.h"

#include "fieldmap_ctrl_c3.h"

//======================================================================
//	define
//======================================================================
#define HEIGHT	0x2000

//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
///	C3_MOVE_WORK
//--------------------------------------------------------------
typedef struct {
	u16 player_len;
	u16 pos_angle;
	s16 df_len;
	s16 df_angle;
}C3_MOVE_WORK;

//======================================================================
//	proto
//======================================================================
static void mapCtrlC3_Create(
		FIELDMAP_WORK *fieldWork, VecFx32 *pos, u16 dir );
static void mapCtrlC3_Delete( FIELDMAP_WORK *fieldWork );
static void mapCtrlC3_Main( FIELDMAP_WORK *fieldWork, VecFx32 *pos );

static void CalcPos( VecFx32 *pos, const VecFx32 *center, u16 len, u16 dir );

//======================================================================
//	フィールドマップ　コントロール　C3
//======================================================================
//--------------------------------------------------------------
///	マップコントロール　グリッド移動
//--------------------------------------------------------------
const DEPEND_FUNCTIONS FieldMapCtrl_C3Functions =
{
	FLDMAP_CTRLTYPE_NOGRID,
	mapCtrlC3_Create,
	mapCtrlC3_Main,
	mapCtrlC3_Delete,
};

//--------------------------------------------------------------
/**
 * フィールドマップ　C3処理　初期化
 * @param	fieldWork	FIELDMAP_WORK
 * @param	pos	自機初期位置
 * @param	dir 自機初期方向
 */
//--------------------------------------------------------------
static void mapCtrlC3_Create(
		FIELDMAP_WORK *fieldWork, VecFx32 *pos, u16 dir )
{
	HEAPID heapID;
	C3_MOVE_WORK *work;
	FIELD_PLAYER *fld_player;
	static const C3_MOVE_WORK init = {
		 0x1f0,
		 0,
		 1,
		 16
	};
	
	heapID = FIELDMAP_GetHeapID( fieldWork );
	work = GFL_HEAP_AllocClearMemory( heapID, sizeof(C3_MOVE_WORK) );
	*work = init;
	FIELDMAP_SetMapCtrlWork( fieldWork, work );
	
	{	//ビルボード設定
		VecFx32 scale = {
			FX32_ONE+(FX32_ONE/2)+(FX32_ONE/4),
			FX32_ONE+(FX32_ONE/2)+(FX32_ONE/4),
			FX32_ONE+(FX32_ONE/2)+(FX32_ONE/4),
		};
		GFL_BBDACT_SYS *bbdActSys = FIELDMAP_GetBbdActSys( fieldWork );
		GFL_BBD_SetScale( GFL_BBDACT_GetBBDSystem(bbdActSys), &scale );
	}
	fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
	FIELD_PLAYER_SetPos( fld_player, pos);
	FIELD_PLAYER_SetDir( fld_player, dir );
}

//--------------------------------------------------------------
/**
 * フィールドマップ　C3処理　削除
 * @param fieldWork FIELDMAP_WORK
 * @retval nothing
 */
//--------------------------------------------------------------
static void mapCtrlC3_Delete( FIELDMAP_WORK *fieldWork )
{
	C3_MOVE_WORK *work = FIELDMAP_GetMapCtrlWork( fieldWork );
	GFL_HEAP_FreeMemory( work );
}

//--------------------------------------------------------------
/**
 * フィールドマップ　C3処理　メイン
 * @param	fieldWork	FIELDMAP_WORK
 * @param	pos
 * @retval nothing
 */
//--------------------------------------------------------------
static void mapCtrlC3_Main( FIELDMAP_WORK *fieldWork, VecFx32 *pos )
{
	int key_cont = FIELDMAP_GetKeyCont( fieldWork );
	C3_MOVE_WORK *mwk = FIELDMAP_GetMapCtrlWork( fieldWork );
	
	{
		s16 df_angle;
		s16 df_len;

		if( key_cont & PAD_BUTTON_B ){
			df_angle	= mwk->df_angle*4;
			df_len		= mwk->df_len*4;
		}else{
			df_angle	= mwk->df_angle;
			df_len		= mwk->df_len;
		}
		
		if (key_cont & PAD_KEY_LEFT) {
			mwk->pos_angle -= df_angle;
		}
		if (key_cont & PAD_KEY_RIGHT) {
			mwk->pos_angle += df_angle;
		}
		if (key_cont & PAD_KEY_UP) {
			mwk->player_len -= df_len;
		}
		if (key_cont & PAD_KEY_DOWN) {
			mwk->player_len += df_len;
		}
	}

	{
		VecFx32 cam, player_pos;
		FIELD_PLAYER *fld_player;
		FIELD_CAMERA *camera_control;
		fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
		camera_control = FIELDMAP_GetFieldCamera( fieldWork );
		FIELD_CAMERA_GetTargetPos( camera_control, &cam);
		CalcPos( &player_pos, &cam, mwk->player_len, mwk->pos_angle );
		//SetPlayerActTrans( fieldWork->field_player, &player_pos );
		FIELD_PLAYER_SetPos( fld_player, &player_pos);
		FIELD_CAMERA_SetAngleY( camera_control, mwk->pos_angle );
		FIELD_PLAYER_C3_Move( fld_player, key_cont, mwk->pos_angle );
	}
}

//======================================================================
//	C3　サブ
//======================================================================
//--------------------------------------------------------------
/**
 * 座標計算
 * @param pos 座標
 * @param center 中心位置
 * @param len 距離
 * @param dir 方向
 * @retval nothing
 */
//--------------------------------------------------------------
static void CalcPos(VecFx32 * pos, const VecFx32 * center, u16 len, u16 dir)
{
	pos->x = center->x + len * FX_SinIdx(dir);
	pos->y = center->y + HEIGHT;
	pos->z = center->z + len * FX_CosIdx(dir);
}
