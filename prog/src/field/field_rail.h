//============================================================================================
/**
 * @file  field_rail.h
 * @brief ノーグリッドでの自機・カメラ制御構造
 * @author  tamada, tomoya
 * @date  2009.05.18
 *
 */
//============================================================================================

#pragma once

#include "field_camera.h"
#include "field/field_rail_loader.h"
#include "field/rail_location.h"
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct _FIELD_RAIL_MAN FIELD_RAIL_MAN;

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct _FIELD_RAIL FIELD_RAIL;

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct _RAIL_POINT RAIL_POINT;

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct _RAIL_LINE RAIL_LINE;



//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
#define RAIL_CAMERA_SET_WORK_SIZ	(32)
typedef struct _RAIL_CAMERA_SET{
  u32 func_index;
	union
	{
		struct
		{
			u32 param0;
			u32 param1;
			u32 param2;
			u32 param3;
			u32 param4;
			u32 param5;
			u32 param6;
			u32 param7;
		};
		u8 work[RAIL_CAMERA_SET_WORK_SIZ];
	};
}RAIL_CAMERA_SET;

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
#define RAIL_LINEPOS_SET_WORK_SIZ	(32)
typedef struct _RAIL_LINEPOS_SET{
  u32 func_index;
  u32 func_dist_index;
	union
	{
		struct
		{
			u32 param0;
			u32 param1;
			u32 param2;
			u32 param3;
			u32 param4;
			u32 param5;
			u32 param6;
			u32 param7;
		};
		u8 work[RAIL_CAMERA_SET_WORK_SIZ];
	};
}RAIL_LINEPOS_SET;



//------------------------------------------------------------------
//------------------------------------------------------------------
typedef void (RAIL_POS_FUNC)(const FIELD_RAIL * man, VecFx32 * pos);
typedef void (RAIL_CAMERA_FUNC)(const FIELD_RAIL_MAN * man);
typedef fx32 (RAIL_LINE_DIST_FUNC)(const RAIL_POINT * point_s, const RAIL_POINT * point_e, const RAIL_LINEPOS_SET * line_pos_set );

//------------------------------------------------------------------
/**
 * @brief キーの定義
 *
 * どの入力でどういう方向への移動になるか？の結びつけのための定義
 */
//------------------------------------------------------------------
typedef enum {
  RAIL_KEY_NULL = 0,
  RAIL_KEY_UP,
  RAIL_KEY_RIGHT,
  RAIL_KEY_DOWN,
  RAIL_KEY_LEFT,

  RAIL_KEY_MAX
}RAIL_KEY;

//------------------------------------------------------------------
/**
 * @brief レールタイプ
 *
 * LOCATIONなどの内容が、POINTなのかLINEなのかを判断するための定義
 */
//------------------------------------------------------------------
typedef enum {  
  FIELD_RAIL_TYPE_POINT = 0,
  FIELD_RAIL_TYPE_LINE,

  FIELD_RAIL_TYPE_MAX
}FIELD_RAIL_TYPE;


//------------------------------------------------------------------
/**
 * @brief テーブルインデックス無効値
 */
//------------------------------------------------------------------
#define RAIL_TBL_NULL	( 0xffffffff )


//-----------------------------------------------------------------------------
/**
 *		名称サイズ
 */
//-----------------------------------------------------------------------------
#define RAIL_NAME_BUF_SIZ	( 48 )

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
enum {  
  RAIL_CONNECT_LINE_MAX = 4,
  RAIL_CONNECT_POINT_MAX = 2,
};


//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
struct _RAIL_POINT {  
  //keys[n]にマッチしたらlines[n]に移動する
	u32 lines[RAIL_CONNECT_LINE_MAX];
  u32 keys[RAIL_CONNECT_LINE_MAX];

  ///POINTの位置座標
  VecFx32 pos;

	u32 camera_set;
  char name[ RAIL_NAME_BUF_SIZ ];
};


//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
struct _RAIL_LINE {
  //point_s --> point_eへの動きをkey方向を正として制御する
	u32 point_s;
	u32 point_e;
  u32 key;
	u32 line_pos_set;

	u32 camera_set;


  char name[ RAIL_NAME_BUF_SIZ ];
  //いずれ必要になるが今考えると混乱する
  //fx32 width_margin;  ///<LINEに対して動ける幅
  //u32 width_divider;
};


//------------------------------------------------------------------
/// レール情報
//------------------------------------------------------------------
struct _RAIL_SETTING{
  u16 point_count;
  u16 line_count;
  u16 camera_count;
  u16 linepos_count;
  u16 camera_func_count;
  u16 linepos_func_count;
  u16 line_dist_func_count;
	u16 dummy00;
  const RAIL_POINT* point_table;
  const RAIL_LINE*	line_table;
	const RAIL_CAMERA_SET*	camera_table;
	const RAIL_LINEPOS_SET* linepos_table;
	RAIL_CAMERA_FUNC*const*	camera_func;
	RAIL_POS_FUNC*const*		line_pos_func;
	RAIL_LINE_DIST_FUNC*const* line_dist_func;

  s32   ofs_max;      // 幅分割数
  fx32  ofs_unit;     // 幅の移動単位
} ;



//============================================================================================
//
//
//  レール制御のための関数群
//
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
extern FIELD_RAIL_MAN * FIELD_RAIL_MAN_Create(HEAPID heapID, FIELD_CAMERA * camera);

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void FIELD_RAIL_MAN_Delete(FIELD_RAIL_MAN * man);

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void FIELD_RAIL_MAN_Update(FIELD_RAIL_MAN * man, int key_cont);

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void FIELD_RAIL_MAN_Load(FIELD_RAIL_MAN * man, const RAIL_SETTING * setting);
extern void FIELD_RAIL_MAN_GetLocation(const FIELD_RAIL_MAN * man, RAIL_LOCATION * location);
extern void FIELD_RAIL_MAN_SetLocation(FIELD_RAIL_MAN * man, const RAIL_LOCATION * location);


//------------------------------------------------------------------
//  たぶんデバッグ用途のみ
//------------------------------------------------------------------
extern void FIELD_RAIL_MAN_SetActiveFlag(FIELD_RAIL_MAN * man, BOOL flag);
extern BOOL FIELD_RAIL_MAN_GetActiveFlag(const FIELD_RAIL_MAN *man);

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void FIELD_RAIL_MAN_GetPos(const FIELD_RAIL_MAN * man, VecFx32 * pos);

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void FIELD_RAIL_MAN_UpdateCamera(const FIELD_RAIL_MAN * man);

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void FIELD_RAIL_MAN_GetDirection(const FIELD_RAIL_MAN * man, VecFx32 * dir);
