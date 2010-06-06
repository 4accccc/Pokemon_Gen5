//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_camera_local.h
 *	@brief  フィールドカメラ外部非公開　定義
 *	@author	tomoya takahashi
 *	@date		2010.04.19
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include <gflib.h>

#ifdef PM_DEBUG
// カメラパラメータ　デバック表示用
#include "debug_message.naix"

#include "print/wordset.h"
#include "print/gf_font.h"
#include "print/printsys.h"

#include "fld_wipe_3dobj.h"

#include "font/font.naix"


//#define DEBUG_CAMERA_ALLCONTROL_MODE_2  // これは、自機とのバインドをカットしたときに、ターゲットオフセットを変更しないモード



#endif

//============================================================================================
//============================================================================================

#define FIELD_CAMERA_DELAY	(6)
#define FIELD_CAMERA_TRACE_BUFF	(FIELD_CAMERA_DELAY+1)

/*---------------------------------------------------------------------------*
	カメラトレース構造体
 *---------------------------------------------------------------------------*/
typedef struct 
{
	int bufsize;
	int CamPoint;
	int TargetPoint;
	int Delay;			//遅延時間
	BOOL UpdateFlg;		//更新開始フラグ
	BOOL ValidX;		//X遅延を有効にするかのフラグ
	BOOL ValidY;		//Y遅延を有効にするかのフラグ
	BOOL ValidZ;		//Z遅延を有効にするかのフラグ

  VecFx32 * targetBuffer;
  VecFx32 * camPosBuffer;

  BOOL Valid;
  BOOL StopReq;

}CAMERA_TRACE;

//------------------------------------------------------------------
/**
 * @brief	フィールドカメラタイプ
 */
//------------------------------------------------------------------
typedef enum {
	FIELD_CAMERA_TYPE_GRID,
	FIELD_CAMERA_TYPE_H01,
	FIELD_CAMERA_TYPE_C03,

  FIELD_CAMERA_TYPE_POKECEN,
  FIELD_CAMERA_TYPE_H01P01,

  FIELD_CAMERA_TYPE_PLP01,

	FIELD_CAMERA_TYPE_MAX,
}FIELD_CAMERA_TYPE;


//-----------------------------------------------------------------------------
/**
 *			カメラ可動範囲
 */
//-----------------------------------------------------------------------------

//------------------------------------------------------------------
/**
 * @brief	カメラ線形移動管理構造体
 */
//------------------------------------------------------------------

//カメラの復帰情報
typedef struct FLD_CAM_PUSH_PARAM_tag
{
  FIELD_CAMERA_MODE BeforeMode;   //プッシュする前のモード
//  FIELD_CAMERA_MODE PushMode;     //復帰情報をプッシュするときのモード
  FLD_CAM_MV_PARAM_CORE RecvParam;  //復帰情報
}FLD_CAM_PUSH_PARAM;

typedef struct FLD_MOVE_CAM_DATA_tag
{
  FLD_CAM_PUSH_PARAM PushParam;

  FLD_CAM_MV_PARAM_CORE SrcParam;   //移動前カメラ情報
  FLD_CAM_MV_PARAM_CORE DstParam;   //移動後カメラ情報
  u16 CostFrm;              //移動にかかるフレーム数
  u16 NowFrm;               //現在のフレーム
  BOOL PushFlg;
  BOOL Valid;   //線形補間中か？
  FLD_CAM_MV_PARAM_CHK Chk;
  void *CallBackWorkPtr;
}FLD_MOVE_CAM_DATA;

//------------------------------------------------------------------
/**
 * @brief	型宣言
 */
//------------------------------------------------------------------
struct _FIELD_CAMERA {
	HEAPID				heapID;			///<使用するヒープ指定ID
  ARCHANDLE * init_param_handle;
  ARCHANDLE * camera_area_handle;
	GFL_G3D_CAMERA * g3Dcamera;			///<カメラ構造体へのポインタ

	FIELD_CAMERA_TYPE	type;			///<カメラのタイプ指定
	FIELD_CAMERA_MODE mode;			///<カメラモード

  const VecFx32 * default_target;
	const VecFx32 *		watch_target;	///<追随する注視点へのポインタ
  const VecFx32 *   watch_camera; ///<追随するカメラ位置へのポインタ

  VecFx32       camPos;             ///<カメラ位置用ワーク
	VecFx32				campos_offset;			///<カメラ位置用補正座標
  VecFx32       campos_write;       ///<カメラ位置用実際に画面反映した値

	VecFx32				target;			        ///<注視点用ワーク
	VecFx32				target_write;			  ///<注視点用実際に画面反映した値
	VecFx32				target_offset;			///<注視点用補正座標
  VecFx32       target_before;

  u16         angle_pitch;
  u16         angle_yaw;
  fx32        angle_len;

  u16         fovy;
  u16         global_angle_yaw;  ///<カメラ座標、ターゲット座標計算後のカメラアングルYaw　カメラモードにも左右されず、常にFIELD_CAMERA_MODE_CALC_CAMERA_POSのアングルを返す
  u16         global_angle_pitch;

  // カメラ可動範囲
  FIELD_CAMERA_AREA camera_area[FIELD_CAMERA_AREA_DATA_MAX];  
  u8                camera_area_num;
  u8                camera_area_active;
  u16               camera_area_id;


  CAMERA_TRACE * Trace;

  FLD_MOVE_CAM_DATA MoveData;
  CAMERA_CALL_BACK CallBack;
  
#ifdef PM_DEBUG
  // デバック用


  u16 debug_subscreen_type;
  u16 debug_trace_off;

  // ターゲット操作用
  u16         debug_target_pitch;
  u16         debug_target_yaw;
  fx32        debug_target_len;
  VecFx32     debug_target;

  fx32        debug_far;

  u16 debug_save_camera_mode;
  u16 debug_save_buffer_mode;


  // デバック表示用
  WORDSET*    p_debug_wordset;
  GFL_FONT*   p_debug_font;
  GFL_MSGDATA*  p_debug_msgdata;
  STRBUF*     p_debug_strbuff;
  STRBUF*     p_debug_strbuff_tmp;

  // デバックコントロール用ワーク
  VecFx32 debug_camera;
  u32 debug_control_on;

  // ワイプ表示
  FLD_WIPEOBJ* p_debug_wipe;
  
  // 基本ターゲットオフセット
  VecFx32 debug_default_target_offs;
#endif
};


//-----------------------------------------------------------------------------
/**
 *			内部公開関数
 */
//-----------------------------------------------------------------------------

extern void FIELD_CAMERA_LOCAL_calcAnglePos( const VecFx32* cp_target, VecFx32* p_pos, u16 yaw, u16 pitch, fx32 len );
extern void FIELD_CAMERA_LOCAL_modeChange_CalcVecAngel( const VecFx32* cp_vec, u16* p_pitch, u16* p_yaw, fx32* p_len );
extern void FIELD_CAMERA_LOCAL_ControlParameter( FIELD_CAMERA * camera, u16 key_cont );





#ifdef _cplusplus
}	// extern "C"{
#endif



