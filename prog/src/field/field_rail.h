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
//------------------------------------------------------------------
typedef void (RAIL_POS_FUNC)(const FIELD_RAIL * man, VecFx32 * pos);
typedef void (RAIL_CAMERA_FUNC)(const FIELD_RAIL_MAN * man);


//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
typedef struct _RAIL_CAMERA_SET{
  RAIL_CAMERA_FUNC * func;
  u32 param0;
  u32 param1;
  u32 param2;
  u32 param3;
  u32 param4;
  u32 param5;
}RAIL_CAMERA_SET;

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
typedef struct _RAIL_LINEPOS_SET{
  RAIL_POS_FUNC * func;
  u32 param0;
  u32 param1;
  u32 param2;
  u32 param3;
}RAIL_LINEPOS_SET;

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
  const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];

  ///POINTの位置座標
  VecFx32 pos;

  const RAIL_CAMERA_SET * camera_set;
  const char * name;
};


//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
struct _RAIL_LINE {
  //point_s --> point_eへの動きをkey方向を正として制御する
  const RAIL_POINT * point_s;
  const RAIL_POINT * point_e;
  RAIL_KEY key;
  const RAIL_LINEPOS_SET * line_pos_set;

  const RAIL_CAMERA_SET * camera_set;

  const char * name;
  //いずれ必要になるが今考えると混乱する
  //fx32 width_margin;  ///<LINEに対して動ける幅
  //u32 width_divider;
};


//------------------------------------------------------------------
/// レール情報
//------------------------------------------------------------------
typedef struct {
  u16 point_count;
  u16 line_count;
  const RAIL_POINT * const * point_table;
  const RAIL_LINE * const * line_table;

  s32   ofs_max;      // 幅分割数
  fx32  ofs_unit;     // 幅の移動単位
} RAIL_SETTING;



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


//------------------------------------------------------------------
//  LINE移動関数
//------------------------------------------------------------------
extern void FIELD_RAIL_POSFUNC_StraitLine(const FIELD_RAIL * rail, VecFx32 * pos);
extern void FIELD_RAIL_POSFUNC_CurveLine(const FIELD_RAIL * rail, VecFx32 * pos);

extern const RAIL_LINEPOS_SET RAIL_LINEPOS_SET_Default;
extern const RAIL_CAMERA_SET RAIL_CAMERA_SET_Default;
extern void FIELD_RAIL_CAMERAFUNC_FixPosCamera(const FIELD_RAIL_MAN* man);
extern void FIELD_RAIL_CAMERAFUNC_FixAngleCamera(const FIELD_RAIL_MAN* man);
extern void FIELD_RAIL_CAMERAFUNC_OfsAngleCamera(const FIELD_RAIL_MAN* man);
extern void FIELD_RAIL_CAMERAFUNC_FixAllCamera(const FIELD_RAIL_MAN* man);


//-----------------------------------------------------------------------------
//  円動作
//-----------------------------------------------------------------------------
extern void FIELD_RAIL_POSFUNC_CircleCamera( const FIELD_RAIL_MAN * man );
extern void FIELD_RAIL_POSFUNC_FixLenCircleCamera( const FIELD_RAIL_MAN * man );

