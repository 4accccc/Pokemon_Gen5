//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		fld_scenearea.h
 *	@brief  フィールド　特殊シーン領域管理
 *	@author	tomoya takahashi
 *	@data		2009.05.22
 *
 *	モジュール名：FLD_SCENEAREA
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif


#include "gflib.h"
#include "field/fieldmap_proc.h"
#include "field_camera.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
// シーンに持たせるパラメータサイズの最大値
#define FLD_SCENEAREA_SCENEPARAM_SIZE  ( 32 )

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	シーンエリア
//=====================================
typedef struct _FLD_SCENEAREA FLD_SCENEAREA;



//-------------------------------------
///	シーンエリア情報
//=====================================
typedef struct _FLD_SCENEAREA_DATA FLD_SCENEAREA_DATA;

// コールバック関数の型
// エリア範囲チェック   戻り値：TRUE  エリア内  FALSE　エリア外
typedef BOOL (FLD_SCENEAREA_CHECK_AREA_FUNC)( const FLD_SCENEAREA* cp_sys, const FLD_SCENEAREA_DATA* cp_data, const VecFx32* cp_pos );
typedef void (FLD_SCENEAREA_UPDATA_FUNC)( const FLD_SCENEAREA* cp_sys, const FLD_SCENEAREA_DATA* cp_data, const VecFx32* cp_pos );

// 構造体定義
struct _FLD_SCENEAREA_DATA{

  // シーンパラメータ
  u32 area[FLD_SCENEAREA_SCENEPARAM_SIZE/4];

  // コールバック関数
  FLD_SCENEAREA_CHECK_AREA_FUNC* p_checkArea;   // 範囲内チェック
  FLD_SCENEAREA_UPDATA_FUNC* p_updata;      // 情報の更新（エリア内の場合マイフレーム行う処理）
  FLD_SCENEAREA_UPDATA_FUNC* p_inside;      // 範囲に入った瞬間の処理
  FLD_SCENEAREA_UPDATA_FUNC* p_outside;     // 範囲を抜けた瞬間の処理
};


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

extern FLD_SCENEAREA* FLD_SCENEAREA_Create( u32 heapID, FIELD_CAMERA * p_camera );
extern void FLD_SCENEAREA_Delete( FLD_SCENEAREA* p_sys );

extern void FLD_SCENEAREA_Load( FLD_SCENEAREA* p_sys, const FLD_SCENEAREA_DATA* cp_data, u32 datanum );
extern void FLD_SCENEAREA_Release( FLD_SCENEAREA* p_sys );
extern void FLD_SCENEAREA_Updata( FLD_SCENEAREA* p_sys, const VecFx32* cp_pos );

// 各種情報の受け渡し
extern FIELD_CAMERA* FLD_SCENEAREA_GetFieldCamera( const FLD_SCENEAREA* cp_sys );

#ifdef _cplusplus
}	// extern "C"{
#endif



