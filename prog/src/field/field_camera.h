//============================================================================================
/**
 *	@file	field_camera.h
 *	@brief
 */
//============================================================================================
#pragma once

#include "../../../resource/fldmapdata/camera_data/fieldcameraformat.h"
#include "field_camera_mv_param.h"


//------------------------------------------------------------------
/**
 * @brief	フィールドカメラ制御構造体への不完全型定義
 */
//------------------------------------------------------------------
typedef struct _FIELD_CAMERA	FIELD_CAMERA;

//------------------------------------------------------------------
/**
 * @brief	フィールドカメラ移動コールバック関数
 */
//------------------------------------------------------------------
typedef void (*CAMERA_CALL_BACK)( FIELD_CAMERA* camera, void *work );

//カメラトレースマスク定義
#define CAM_TRACE_MASK_X	(1)
#define CAM_TRACE_MASK_Y	(2)
#define CAM_TRACE_MASK_Z	(4)
#define CAM_TRACE_MASK_ALL	(CAM_TRACE_MASK_X|CAM_TRACE_MASK_Y|CAM_TRACE_MASK_Z)


//-----------------------------------------------------------------------------
/**
 *			フィールドカメラモード
 */
//-----------------------------------------------------------------------------
typedef enum
{
	FIELD_CAMERA_MODE_CALC_CAMERA_POS,		// カメラ座標をターゲット座標とアングルから計算する
	FIELD_CAMERA_MODE_CALC_TARGET_POS,		// ターゲット座標をカメラ座標とアングルから計算する
	FIELD_CAMERA_MODE_DIRECT_POS,					// カメラ座標、ターゲット座標　直接指定

	FIELD_CAMERA_MODE_MAX,								// モード最大数（システム内で使用）
} FIELD_CAMERA_MODE;


//-----------------------------------------------------------------------------
/**
 *			カメラ可動範囲
 */
//-----------------------------------------------------------------------------
// カメラ可動範囲、設定最大数
#define FIELD_CAMERA_AREA_DATA_MAX   ( 4 )

// 範囲判定タイプ
typedef enum
{
// conv.pl用定義 [CAMERA_AREA]
// resource/fldmapdata/camera_scroll/で直値指定しています。
// 基本下に追加してください。
  FIELD_CAMERA_AREA_NONE,     // 範囲なし
  FIELD_CAMERA_AREA_RECT,     // 矩形範囲外判定
  FIELD_CAMERA_AREA_CIRCLE,   // 円範囲外判定
  FIELD_CAMERA_AREA_RECT_IN, // 矩形囲内判定

  FIELD_CAMERA_AREA_MAX,   // システム内で使用
} FIELD_CAMERA_AREA_TYPE;

// 管理座標
typedef enum
{
// conv.pl用定義 [CAMERA_CONT]
// resource/fldmapdata/camera_scroll/で直値指定しています。
// 基本下に追加してください。
  FIELD_CAMERA_AREA_CONT_TARGET,   // カメラターゲットが範囲内にあるか管理
  FIELD_CAMERA_AREA_CONT_CAMERA,   // カメラ座標が範囲内にあるか管理

  FIELD_CAMERA_AREA_CONT_MAX,   // システム内で使用
} FIELD_CAMERA_AREA_CONT;

//-------------------------------------
///	矩形範囲
//=====================================
typedef struct {
  s32 x_min;  // 小数点なし座標
  s32 x_max;
  s32 z_min;
  s32 z_max;
} FIELD_CAMERA_RECT;

//-------------------------------------
///	円範囲
//=====================================
typedef struct {
  fx32 center_x;
  fx32 center_z;
  fx32 r;         // 可動範囲の半径
  u16  min_rot;   // min_rot == max_rot なら ３６０度　可動範囲
  u16  max_rot;   // min_rot > max_rot なら、360度を通過する方が　可動範囲
} FIELD_CAMERA_CIRCLE;

//-------------------------------------
///	カメラ可動範囲情報
//=====================================
typedef struct {

  // 範囲タイプ指定
  FIELD_CAMERA_AREA_TYPE area_type;

  // 管理座標タイプ指定
  FIELD_CAMERA_AREA_CONT area_cont;

  // 範囲情報
  // 16byte
  union
  {
    // XZ平面の矩形範囲
    FIELD_CAMERA_RECT rect;

    // XZ平面の円範囲
    FIELD_CAMERA_CIRCLE circle;
  } ;
  
} FIELD_CAMERA_AREA;


//-------------------------------------
///	カメラ可動範囲セット
//=====================================
typedef struct {
  // 個数
  u32 num;
  FIELD_CAMERA_AREA buff[];
} FIELD_CAMERA_AREA_SET;


//============================================================================================
//	生成・メイン・消去
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
extern FIELD_CAMERA* FIELD_CAMERA_Create(
		u8 type,
		FIELD_CAMERA_MODE mode,
		GFL_G3D_CAMERA * cam,
		const VecFx32 * target,
		HEAPID heapID);

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void FIELD_CAMERA_Delete( FIELD_CAMERA* camera );

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void FIELD_CAMERA_Main( FIELD_CAMERA* camera, u16 key_cont);



//============================================================================================
//============================================================================================


//------------------------------------------------------------------
//	カメラモードの変更
//	  変更時の注意点
//	    各モードへの変更により、無効な情報の引継ぎを回避するため、
//	    以下の情報が初期化されます。
//      【FIELD_CAMERA_MODE_CALC_CAMERA_POS】
//          FIELD_CAMERA_BindCamera()で設定したwatch_camera
//          
//      【FIELD_CAMERA_MODE_CALC_TARGET_POS】
//          FIELD_CAMERA_BindTarget()で設定したwatch_target
//          
//      【FIELD_CAMERA_MODE_DIRECT_POS】
//          アングル情報
//
//------------------------------------------------------------------
// カメラの計算モードを切り替えます。
extern void FIELD_CAMERA_SetMode( FIELD_CAMERA * camera, FIELD_CAMERA_MODE mode );
// カメラモードを取得します。
extern FIELD_CAMERA_MODE FIELD_CAMERA_GetMode( const FIELD_CAMERA * camera );
// カメラモードを変更します。
// 1つ前のターゲット座標＋カメラ座標からmode用の値の計算も行います。
extern void FIELD_CAMERA_ChangeMode( FIELD_CAMERA * camera, FIELD_CAMERA_MODE mode );


//------------------------------------------------------------------
// カメラにZONEのデフォルト情報を設定する。
//  どんな操作をしていても、これを呼べば、
//  そのZONEに入ってきたときのカメラ状態に戻ります。
//------------------------------------------------------------------
extern void FIELD_CAMERA_SetDefaultParameter( FIELD_CAMERA * camera );


//------------------------------------------------------------------
// デフォルトのカメラタイプを変更する。
// 　resource/fldmapdata/camera_dataのエクセルのカメラをしてする値です。
//
// 変えると、カメラパラメータを読み込んで設定しなおします。
//------------------------------------------------------------------
extern void FIELD_CAMERA_SetCameraType( FIELD_CAMERA * camera, u32 camera_type );

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void FIELD_CAMERA_SetNear(FIELD_CAMERA * camera, fx32 near);
extern fx32 FIELD_CAMERA_GetNear(const FIELD_CAMERA * camera);
extern void FIELD_CAMERA_SetFar(FIELD_CAMERA * camera, fx32 far);
extern fx32 FIELD_CAMERA_GetFar(const FIELD_CAMERA * camera);


//-----------------------------------------------------------------------------
/**
 *			angle操作
 *	有効カメラモード
 *					FIELD_CAMERA_MODE_CALC_CAMERA_POS,		// カメラ座標をターゲット座標とアングルから計算する
 *					FIELD_CAMERA_MODE_CALC_TARGET_POS,		// ターゲット座標をカメラ座標とアングルから計算する
 *
 *	『FIELD_CAMERA_MODE_CALC_CAMERA_POS』
 *		ターゲット座標から見た、カメラ座標のアングル設定
 *
 *	『FIELD_CAMERA_MODE_CALC_TARGET_POS』
 *		カメラ座標から見た、ターゲット座標のアングル設定
 */
//-----------------------------------------------------------------------------
// ターゲットを中心とした、アングル操作
// Pitch  垂直方向回転
// Yaw    水平方向回転
extern u16 FIELD_CAMERA_GetAnglePitch(const FIELD_CAMERA * camera );
extern void FIELD_CAMERA_SetAnglePitch(FIELD_CAMERA * camera, u16 angle );
extern u16 FIELD_CAMERA_GetAngleYaw(const FIELD_CAMERA * camera );
extern void FIELD_CAMERA_SetAngleYaw(FIELD_CAMERA * camera, u16 angle );
extern const u16 * FIELD_CAMERA_GetAngleYawAddress( const FIELD_CAMERA *camera );
extern fx32 FIELD_CAMERA_GetAngleLen(const FIELD_CAMERA * camera );
extern void FIELD_CAMERA_SetAngleLen(FIELD_CAMERA * camera, fx32 length );

//----------------------------------------------------------------------------
/**
 *	@brief	カメラ視野角操作
 */
//-----------------------------------------------------------------------------
extern u16 FIELD_CAMERA_GetFovy(const FIELD_CAMERA * camera );
extern void FIELD_CAMERA_SetFovy(FIELD_CAMERA * camera, u16 fovy );

//------------------------------------------------------------------
/**
 * @brief	カメラ注視点の取得
 * @param	camera		FIELDカメラ制御ポインタ
 * @param	pos			カメラ注視点を受け取るVecFx32へのポインタ
 */
//------------------------------------------------------------------
extern void	FIELD_CAMERA_GetTargetPos( const FIELD_CAMERA* camera, VecFx32* pos );

//------------------------------------------------------------------
/**
 * @brief	カメラ注視点の設定
 * @param	camera		FIELDカメラ制御ポインタ
 * @param	pos			カメラ注視点を渡すVecFx32へのポインタ
 *
 *	有効カメラモード
 *					FIELD_CAMERA_MODE_CALC_CAMERA_POS,		// カメラ座標をターゲット座標とアングルから計算する
 *					FIELD_CAMERA_MODE_DIRECT_POS,					// カメラ座標、ターゲット座標　直接指定
 */
//------------------------------------------------------------------
extern void	FIELD_CAMERA_SetTargetPos( FIELD_CAMERA* camera, const VecFx32* target );


//------------------------------------------------------------------
/**
 * @brief	カメラ注視点　補正座標の取得
 * @param	camera		    FIELDカメラ制御ポインタ
 * @param	target_offset	カメラ注視点を補正する座標受け取るVecFx32へのポインタ
 */
//------------------------------------------------------------------
extern void	FIELD_CAMERA_GetTargetOffset( const FIELD_CAMERA* camera, VecFx32* target_offset );


//------------------------------------------------------------------
/**
 * @brief	カメラ注視点　補正座標の取得
 * @param	camera		        FIELDカメラ制御ポインタ
 * @param	target_offset			カメラ注視点を補正する座標渡すVecFx32へのポインタ
 */
//------------------------------------------------------------------
extern void	FIELD_CAMERA_SetTargetOffset( FIELD_CAMERA* camera, const VecFx32* target_offset );

//------------------------------------------------------------------
//  カメラ位置の取得・セット
//  カメラ動作状態によっては正しく反映されていないので注意
//------------------------------------------------------------------
extern void FIELD_CAMERA_GetCameraPos( const FIELD_CAMERA * camera, VecFx32 * camPos);
//
//有効カメラモード
//	FIELD_CAMERA_MODE_CALC_TARGET_POS,		// ターゲット座標をカメラ座標とアングルから計算する
//	FIELD_CAMERA_MODE_DIRECT_POS,					// カメラ座標、ターゲット座標　直接指定
extern void FIELD_CAMERA_SetCameraPos( FIELD_CAMERA * camera, const VecFx32 * camPos);


//------------------------------------------------------------------
//  カメラ上方向
//------------------------------------------------------------------
extern void FIELD_CAMERA_SetCameraUp( FIELD_CAMERA * camera, const VecFx32 * camUp);
extern void FIELD_CAMERA_GetCameraUp( const FIELD_CAMERA * camera, VecFx32 * camUp);

//------------------------------------------------------------------
//------------------------------------------------------------------
extern const GFL_G3D_CAMERA * FIELD_CAMERA_GetCameraPtr(const FIELD_CAMERA * camera);

//------------------------------------------------------------------
//有効カメラモード
//	FIELD_CAMERA_MODE_CALC_CAMERA_POS,		// カメラ座標をターゲット座標とアングルから計算する
//	FIELD_CAMERA_MODE_DIRECT_POS,					// カメラ座標、ターゲット座標　直接指定
//------------------------------------------------------------------
extern void FIELD_CAMERA_BindTarget(FIELD_CAMERA * camera, const VecFx32 * watch_target);
extern void FIELD_CAMERA_BindDefaultTarget(FIELD_CAMERA * camera);
extern void FIELD_CAMERA_FreeTarget(FIELD_CAMERA * camera);
extern const VecFx32 *FIELD_CAMERA_GetWatchTarget(FIELD_CAMERA * camera);
extern BOOL FIELD_CAMERA_IsBindDefaultTarget( const FIELD_CAMERA * camera );

//------------------------------------------------------------------
//	カメラ座標更新の停止（デバック用？）
//	DIRECT_POSモードに変更することで対応可能
//------------------------------------------------------------------
//extern void FIELD_CAMERA_BindNoCamera(FIELD_CAMERA * camera, BOOL flag);

//------------------------------------------------------------------
//	カメラ座標のバインド
//有効カメラモード
//	FIELD_CAMERA_MODE_CALC_TARGET_POS,		// ターゲット座標をカメラ座標とアングルから計算する
//	FIELD_CAMERA_MODE_DIRECT_POS,					// カメラ座標、ターゲット座標　直接指定
//------------------------------------------------------------------
extern void FIELD_CAMERA_BindCamera(FIELD_CAMERA * camera, const VecFx32 * watch_camera);
extern void FIELD_CAMERA_FreeCamera(FIELD_CAMERA * camera);
extern BOOL FIELD_CAMERA_IsBindCamera(const FIELD_CAMERA * camera);



//-----------------------------------------------------------------------------
/**
 *      カメラ可動範囲
 */
//-----------------------------------------------------------------------------
extern void FIELD_CAMERA_SetCameraArea( FIELD_CAMERA * camera, const FIELD_CAMERA_AREA_SET* cp_area );
extern void FIELD_CAMERA_ClearCameraArea( FIELD_CAMERA * camera );
extern u32 FIELD_CAMERA_GetCameraAreaNum( const FIELD_CAMERA * camera );
extern FIELD_CAMERA_AREA_TYPE FIELD_CAMERA_GetCameraAreaType( const FIELD_CAMERA * camera, u32 idx );
extern FIELD_CAMERA_AREA_CONT FIELD_CAMERA_GetCameraAreaCont( const FIELD_CAMERA * camera, u32 idx );

extern void FIELD_CAMERA_LoadCameraArea( FIELD_CAMERA * camera, u32 area_id, HEAPID heapID );

// 可動範囲処理後の、座標を取得
extern void FIELD_CAMERA_GetCameraAreaAfterTargetPos( const FIELD_CAMERA * camera, VecFx32* p_pos );
extern void FIELD_CAMERA_GetCameraAreaAfterCameraPos( const FIELD_CAMERA * camera, VecFx32* p_pos );

//-------------------------------------
///	カメラエリアの動作をフック
//
//  TRUE:カメラエリア動作
//  FALSE:カメラエリア停止
//=====================================
extern void FIELD_CAMERA_SetCameraAreaActive( FIELD_CAMERA * camera, BOOL flag );
extern BOOL FIELD_CAMERA_GetCameraAreaActive( const FIELD_CAMERA * camera );

//-----------------------------------------------------------------------------
/**
 *      線形カメラ関連
 */
//-----------------------------------------------------------------------------
extern void FIELD_CAMERA_SetRecvCamParam(FIELD_CAMERA * camera);
extern void FIELD_CAMERA_ClearRecvCamParam(FIELD_CAMERA * camera);
extern void FIELD_CAMERA_SetLinerParam(FIELD_CAMERA * camera, const FLD_CAM_MV_PARAM *param, const u16 inFrame);
extern void FIELD_CAMERA_RecvLinerParam(FIELD_CAMERA * camera, const FLD_CAM_MV_PARAM_CHK *chk, const u16 inFrame);
extern void FIELD_CAMERA_RecvLinerParamDefault( FIELD_CAMERA * camera, const u16 inFrame);
extern void FIELD_CAMERA_SetLinerParamDirect(FIELD_CAMERA * camera,
                                            const VecFx32 *inCamPos, const VecFx32 *inTrgtPos,
                                            const FLD_CAM_MV_PARAM_CHK *inChk,
                                            const u16 inFrame);
extern void FIELD_CAMERA_SetMvFuncCallBack(FIELD_CAMERA * camera, CAMERA_CALL_BACK func, void *work);
extern void FIELD_CAMERA_ClearMvFuncCallBack(FIELD_CAMERA * camera);
extern BOOL FIELD_CAMERA_CheckMvFunc(FIELD_CAMERA * camera);
extern FLD_CAM_MV_PARAM_CORE *FIELD_CAMERA_GetMoveDstPrmPtr(FIELD_CAMERA * camera);

//-----------------------------------------------------------------------------
/**
 *      カメラトレース関連
 */
//-----------------------------------------------------------------------------
extern void FIELD_CAMERA_RestartTrace(FIELD_CAMERA * camera_ptr);
extern void FIELD_CAMERA_StopTraceRequest(FIELD_CAMERA * camera_ptr);
extern BOOL FIELD_CAMERA_CheckTrace(FIELD_CAMERA * camera_ptr);
extern BOOL FIELD_CAMERA_CheckTraceSys(FIELD_CAMERA * camera_ptr);

// ↓使用に注意が必要
 /*	トレース座標が違う場合に使用すると、カメラが急に飛ぶような動作を行います。
 *	注意して使用してください。
 */
extern void FIELD_CAMERA_StopTraceNow(FIELD_CAMERA * camera_ptr);  

#ifdef  PM_DEBUG
typedef enum{
  FIELD_CAMERA_DEBUG_BIND_NONE,
  FIELD_CAMERA_DEBUG_BIND_CAMERA_POS,
  FIELD_CAMERA_DEBUG_BIND_TARGET_POS,
} FIELD_CAMERA_DEBUG_BIND_TYPE;
//------------------------------------------------------------------
//  デバッグ用：下画面操作とのバインド
//------------------------------------------------------------------
extern void FIELD_CAMERA_DEBUG_BindSubScreen(FIELD_CAMERA * camera, void * param, FIELD_CAMERA_DEBUG_BIND_TYPE type, HEAPID heapID );
extern void FIELD_CAMERA_DEBUG_ReleaseSubScreen(FIELD_CAMERA * camera);
extern void FIELD_CAMERA_DEBUG_GetBindSubScreenTarget( const FIELD_CAMERA * camera, VecFx32* p_target );


//------------------------------------------------------------------
//  デフォルトターゲット操作
//------------------------------------------------------------------
extern void FIELD_CAMERA_DEBUG_SetDefaultTarget( FIELD_CAMERA* camera, const VecFx32* target );
extern const VecFx32* FIELD_CAMERA_DEBUG_GetDefaultTarget( const FIELD_CAMERA* camera );

extern void FIELD_CAMERA_GetInitialParameter( const FIELD_CAMERA* camera, FLD_CAMERA_PARAM * result);


//------------------------------------------------------------------
//  カメラ　デバック表示関数
//------------------------------------------------------------------
extern void FIELD_CAMERA_DEBUG_Draw( const FIELD_CAMERA* camera);




//------------------------------------------------------------------
//  デバッグカメラ詳細操作
/*
 *  NONE：ターゲット、カメラ平行移動
 *	B：カメラ公転（ターゲット座標が変わる）
 *	Y：カメラ自転（カメラ座標が変わる）
 *	A：パース操作
 *	X：ターゲットカメラバインドのON・OFF
 *	
 *	START：バッファリングモード変更
 *
 *	戻り値　TRUE  変更あり    FALSE　変更なし
 */
//------------------------------------------------------------------
extern void FIELD_CAMERA_DEBUG_InitControl( FIELD_CAMERA* camera, HEAPID heapID );
extern void FIELD_CAMERA_DEBUG_ExitControl( FIELD_CAMERA* camera );
extern BOOL FIELD_CAMERA_DEBUG_Control( FIELD_CAMERA* camera, int trg, int cont, int repeat );
extern void FIELD_CAMERA_DEBUG_DrawInfo( FIELD_CAMERA* camera, GFL_BMPWIN* p_win, fx32 map_size_x, fx32 map_size_z );
extern void FIELD_CAMERA_DEBUG_DrawControlHelp( FIELD_CAMERA* camera, GFL_BMPWIN* p_win );

// 線形カメラ用のパラメータをカメラワークに設定
// デバック操作中のパラメータの値が消えるので注意
extern void FIELD_CAMERA_DEBUG_SetUpLinerCameraInfoData( FIELD_CAMERA* camera );
#endif

