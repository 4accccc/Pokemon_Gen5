//============================================================================================
/**
 * @file	field_camera.c
 * @brief	フィールドカメラ制御
 * @author	tamada GAME FREAK inc.
 */
//============================================================================================
#include <gflib.h>
#include <calctool.h>

#include "field/field_const.h"

#include "field_g3d_mapper.h"
#include "field_common.h"
#include "field_camera.h"
	
#include "arc_def.h"
#include "arc/fieldmap/field_camera_data.naix"


#ifdef PM_DEBUG
// カメラパラメータ　デバック表示用
#include "message.naix"

#include "print/wordset.h"
#include "print/gf_font.h"
#include "print/printsys.h"

#include "fld_wipe_3dobj.h"

#include "msg/msg_d_tomoya.h"

#include "font/font.naix"
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
  VecFx32       campos_write;       ///<カメラ位置用実際に画面反映した値

	VecFx32				target;			        ///<注視点用ワーク
	VecFx32				target_write;			  ///<注視点用実際に画面反映した値
	VecFx32				target_offset;			///<注視点用補正座標
  VecFx32       target_before;

  u16         angle_pitch;
  u16         angle_yaw;
  fx32        angle_len;

  u16         fovy;
  u16         globl_angle_yaw;  ///<カメラ座標、ターゲット座標計算後のカメラアングルYaw　カメラモードにも左右されず、常にFIELD_CAMERA_MODE_CALC_CAMERA_POSのアングルを返す

  // カメラ可動範囲
  FIELD_CAMERA_AREA camera_area[FIELD_CAMERA_AREA_DATA_MAX];  
  u16               camera_area_num;
  u16               camera_area_active;
  
#ifdef PM_DEBUG
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
  
#endif

  CAMERA_TRACE * Trace;

  FLD_MOVE_CAM_DATA MoveData;
  CAMERA_CALL_BACK CallBack;
};


//============================================================================================
//============================================================================================



static void createTraceData(const int inHistNum, const int inDelay,
						const int inTraceMask, const int inHeapID,
						FIELD_CAMERA * ioCamera);
static void deleteTraceData(FIELD_CAMERA * camera_ptr);
static void updateTraceData(CAMERA_TRACE * trace,
    const VecFx32 * inTarget, const VecFx32 * inCamPos, VecFx32 * outTarget, VecFx32 * outCamPos);
static void traceUpdate(FIELD_CAMERA * camera);


//------------------------------------------------------------------
//------------------------------------------------------------------
static void loadCameraParameters(FIELD_CAMERA * camera, const VecFx32* watch_target);
static void loadInitialParameter( const FIELD_CAMERA* camera, FLD_CAMERA_PARAM * result );

static void ControlParameter( FIELD_CAMERA * camera, u16 key_cont );
static void ControlParameter_CalcCamera( FIELD_CAMERA * camera, u16 key_cont );
static void ControlParameter_CalcTarget( FIELD_CAMERA * camera, u16 key_cont );
static void ControlParameter_Direct( FIELD_CAMERA * camera, u16 key_cont );

static void ControlParameterInit_CalcCamera( FIELD_CAMERA * camera );
static void ControlParameterInit_CalcTarget( FIELD_CAMERA * camera );
static void ControlParameterInit_Direct( FIELD_CAMERA * camera );


//------------------------------------------------------------------
//------------------------------------------------------------------
static void modeChange_CalcVecAngel( const VecFx32* cp_vec, u16* p_pitch, u16* p_yaw, fx32* p_len );
static void modeChange_SetVecAngel( FIELD_CAMERA * camera, const VecFx32* cp_vec );
static void modeChange_CalcCameraPos( FIELD_CAMERA * camera );
static void modeChange_CalcTargetPos( FIELD_CAMERA * camera );
static void modeChange_CalcDirectPos( FIELD_CAMERA * camera );


//------------------------------------------------------------------
//------------------------------------------------------------------
static void updateGlobalAngleYaw( FIELD_CAMERA * camera, const VecFx32* cp_targetPos, const VecFx32* cp_cameraPos );


//------------------------------------------------------------------
// カメラ可動エリア判定
//------------------------------------------------------------------
static void cameraArea_UpdateTarget( const FIELD_CAMERA_AREA * cp_area, VecFx32* p_target );
static void cameraArea_UpdateCamPos( const FIELD_CAMERA_AREA * cp_area, VecFx32* p_camera );
static BOOL cameraArea_IsAreaRect( const FIELD_CAMERA_AREA* cp_area, const VecFx32* cp_pos, VecFx32* p_pos );
static BOOL cameraArea_IsAreaCircle( const FIELD_CAMERA_AREA* cp_area, const VecFx32* cp_pos, VecFx32* p_pos );
static BOOL cameraArea_IsAreaRectIn( const FIELD_CAMERA_AREA* cp_area, const VecFx32* cp_pos, VecFx32* p_pos );

static BOOL (*pIsAreaFunc[])( const FIELD_CAMERA_AREA* cp_area, const VecFx32* cp_pos, VecFx32* p_pos ) = 
{
  NULL,
  cameraArea_IsAreaRect,
  cameraArea_IsAreaCircle,
  cameraArea_IsAreaRectIn,
};

//------------------------------------------------------------------
// 線形カメラ移動関連処理
//------------------------------------------------------------------
static void SetNowCameraParam(const FIELD_CAMERA * inCamera, FLD_CAM_MV_PARAM_CORE *outParam);
#if 0
static void RecvModeCamera(FIELD_CAMERA * camera);
static void RecvModeTarget(FIELD_CAMERA * camera);
static void RecvModeDirect(FIELD_CAMERA * camera);
#endif
static void LinerMoveFunc(FIELD_CAMERA * camera, void *work);
static void DirectLinerMoveFunc(FIELD_CAMERA * camera, void *work);

static void VecSubFunc(
    const VecFx32 *inSrc, const VecFx32 *inDst, const u16 inCostFrm, const u16 inNowFrm, VecFx32 *outVec);
static fx32 SubFuncFx(const fx32 *inSrc, const fx32 *inDst, const u16 inCostFrm, const u16 inNowFrm);
static u16 SubFuncU16(const u16 *inSrc, const u16 *inDst, const u16 inCostFrm, const u16 inNowFrm);


#ifdef PM_DEBUG

static void DumpCameraParam(FIELD_CAMERA * camera);

#endif //PM_DEBUG

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	初期化
 */
//------------------------------------------------------------------
FIELD_CAMERA* FIELD_CAMERA_Create(
		u8 type,
		FIELD_CAMERA_MODE mode,
		GFL_G3D_CAMERA * cam,
		const VecFx32 * target,
		HEAPID heapID)
{
	FIELD_CAMERA* camera = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_CAMERA) );

	enum { CAMERA_LENGTH = 16 };

	GF_ASSERT(cam != NULL);
	TAMADA_Printf("FIELD CAMERA TYPE = %d\n", type);
	camera->type = type;
	camera->mode = mode;
	camera->g3Dcamera = cam;
  camera->watch_camera = NULL;
	camera->heapID = heapID;

  VEC_Set( &camera->camPos, 0, 0, 0 );
	VEC_Set( &camera->target, 0, 0, 0 );
  VEC_Set( &camera->target_before, 0, 0, 0 );

  camera->angle_yaw = 0;
  camera->angle_pitch = 0;
  camera->angle_len = 0x0078;

  camera->camera_area_active = TRUE;


  // パースをカメラの情報から取得
  if( GFL_G3D_CAMERA_GetProjectionType( cam ) == GFL_G3D_PRJPERS ){
    fx32 fovyCos;
    GFL_G3D_CAMERA_GetfovyCos( cam, &fovyCos );
    camera->fovy = FX_AcosIdx( fovyCos );
  }

  // ハンドルオープン
  {
    camera->init_param_handle   = GFL_ARC_OpenDataHandle(ARCID_FIELD_CAMERA, camera->heapID);
    camera->camera_area_handle  = GFL_ARC_OpenDataHandle(ARCID_FLD_CAMSCRLL, camera->heapID);
  }


  loadCameraParameters(camera, target);

	createTraceData(	FIELD_CAMERA_TRACE_BUFF, FIELD_CAMERA_DELAY,
							CAM_TRACE_MASK_Y, heapID, camera);

  //コールバック関数クリア
  camera->CallBack = NULL;

	return camera;
}
//------------------------------------------------------------------
/**
 * @brief	終了
 */
//------------------------------------------------------------------
void	FIELD_CAMERA_Delete( FIELD_CAMERA* camera )
{
  // ハンドルクローズ
  {
    GFL_ARC_CloseDataHandle(camera->init_param_handle);
    GFL_ARC_CloseDataHandle(camera->camera_area_handle);
  }
  
  deleteTraceData( camera );
	GFL_HEAP_FreeMemory( camera );
}

//------------------------------------------------------------------
/**
 * @brief	メイン
 */
//------------------------------------------------------------------
void FIELD_CAMERA_Main( FIELD_CAMERA* camera, u16 key_cont)
{
  if (camera->CallBack != NULL){
    camera->CallBack(camera, camera->MoveData.CallBackWorkPtr);
  }
  ControlParameter(camera, key_cont);
}


//----------------------------------------------------------------------------
/**
 *	@brief	カメラモード　設定
 *	
 *	@param	camera		カメラ
 *	@param	mode			モード
 *
 *
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
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_SetMode( FIELD_CAMERA * camera, FIELD_CAMERA_MODE mode )
{
  static void (*pInit[])( FIELD_CAMERA* camera ) = 
  {
    ControlParameterInit_CalcCamera,
    ControlParameterInit_CalcTarget,
    ControlParameterInit_Direct,
  };
  
	GF_ASSERT( camera );
	GF_ASSERT( mode < FIELD_CAMERA_MODE_MAX );
	camera->mode = mode;

  // 各モードの初期化処理
  pInit[ mode ]( camera );
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラモード　取得
 *
 *	@param	camera	カメラ
 *
 *	@return	カメラモード
 */
//-----------------------------------------------------------------------------
FIELD_CAMERA_MODE FIELD_CAMERA_GetMode( const FIELD_CAMERA * camera )
{
	GF_ASSERT( camera );
	return camera->mode;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラモード　モードの変更
 *
 *	@param	camera
 *	@param	mode 
 *
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
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_ChangeMode( FIELD_CAMERA * camera, FIELD_CAMERA_MODE mode )
{
  static void (*pFunc[ FIELD_CAMERA_MODE_MAX ])( FIELD_CAMERA* camera ) = {
    modeChange_CalcCameraPos,
    modeChange_CalcTargetPos,
    modeChange_CalcDirectPos,
  };
  
	GF_ASSERT( camera );
  GF_ASSERT( mode < FIELD_CAMERA_MODE_MAX );
  
  if( camera->mode != mode )
  {
    pFunc[ mode ]( camera );

    FIELD_CAMERA_SetMode( camera, mode );
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  カメラにZONEのデフォルトパラメータを設定する。
 *
 *	@param	camera  カメラ
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_SetDefaultParameter( FIELD_CAMERA * camera )
{
  // カメラモードを変更
  FIELD_CAMERA_SetMode( camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );
  
  // ゾーンのカメラを読み込み
  loadCameraParameters( camera, camera->default_target );

  // その他パラメータも初期化
  camera->camera_area_active = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラのデフォルトタイプを変更します。
 *
 *	@param	camera        カメラ
 *	@param	camera_type   タイプ（resource/fldmapdata/camera_dataのエクセルのカメラをしてする値です。）
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_SetCameraType( FIELD_CAMERA * camera, u32 camera_type )
{
  camera->type = camera_type;
  FIELD_CAMERA_SetDefaultParameter( camera );
}


//============================================================================================
//
//
//		カメラ初期化用関数
//
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static void loadCameraParameters(FIELD_CAMERA * camera, const VecFx32* watch_target)
{
  FLD_CAMERA_PARAM param;

  // パラメータ読み込み
  loadInitialParameter( camera, &param );
  
#if 1
  TOMOYA_Printf("FIELD CAMERA INIT INFO\n");
  TOMOYA_Printf("FIELD CAMERA TYPE =%d\n",camera->type);
  TOMOYA_Printf("dist = %08x\n", param.Distance);
  TOMOYA_Printf("angle = %08x,%08x,%08x\n", param.Angle.x, param.Angle.y, param.Angle.z );
  TOMOYA_Printf("viewtype = %d, persp = %08x\n", param.View, param.PerspWay);
  TOMOYA_Printf("near=%8x, far =%08x\n", param.Near, param.Far);
  TOMOYA_Printf("playerLink%d\n", param.PlayerLink  );
  TOMOYA_Printf("shift = %08x,%08x,%08x\n", param.Shift.x, param.Shift.y, param.Shift.z );
  TOMOYA_Printf("now near = %d\n", FX_Whole( FIELD_CAMERA_GetNear(camera) ) );
  TOMOYA_Printf("now far = %d\n", FX_Whole( FIELD_CAMERA_GetFar(camera) ) );
#endif

#ifdef PM_DEBUG
  if( param.depthType == DEPTH_TYPE_ZBUF)
  {
    camera->debug_save_buffer_mode = 2; // CAMERA_DEBUG_BUFFER_MODE_ZBUFF_AUTO
  }
  else
  {
    camera->debug_save_buffer_mode = 3; // CAMERA_DEBUG_BUFFER_MODE_WBUFF_AUTO
  }
#endif

  camera->angle_len = param.Distance * FX32_ONE;
  camera->angle_pitch = param.Angle.x;
  camera->angle_yaw = param.Angle.y;
  //camera->viewtype = param.ViewType;
  camera->fovy = param.PerspWay;
  FIELD_CAMERA_SetFar( camera, param.Far );
  FIELD_CAMERA_SetNear(camera, param.Near );
  switch (param.depthType)
  {
  case DEPTH_TYPE_ZBUF:
    GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_Z );
    break;
  case DEPTH_TYPE_WBUF:
    GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_W );
    break;
  }
  if( GFL_G3D_CAMERA_GetProjectionType(camera->g3Dcamera) == GFL_G3D_PRJPERS ){
    GFL_G3D_CAMERA_SetfovySin( camera->g3Dcamera, FX_SinIdx( camera->fovy ) );
    GFL_G3D_CAMERA_SetfovyCos( camera->g3Dcamera, FX_CosIdx( camera->fovy ) );
  }

  // プレイヤーとのバインドチェック
  if( param.PlayerLink )
  {
    // バインドする場合
    // watch_targetをセット
    // Shiftはtarget_offsetに格納
    camera->default_target  = watch_target;
    camera->watch_target    = watch_target;

    camera->target        = *watch_target;
    camera->target_offset = param.Shift;
  }
  else
  {
    // バインドしない場合
    // watch_targetをクリア
    // Shiftはtargetに格納
    camera->default_target  = FALSE;
    camera->watch_target    = FALSE;

    camera->target = param.Shift;
    VEC_Set( &camera->target_offset, 0,0,0 );
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  デフォルトパラメータの取得
 *
 *	@param	camera
 *	@param	result 
 */
//-----------------------------------------------------------------------------
static void loadInitialParameter( const FIELD_CAMERA* camera, FLD_CAMERA_PARAM * result )
{
  enum {
    FILE_ID = NARC_field_camera_data_field_camera_bin,
  };
  u16 size = GFL_ARC_GetDataSizeByHandle(camera->init_param_handle, FILE_ID);
  if ( camera->type * sizeof(FLD_CAMERA_PARAM) >= size )
  {
    OS_TPrintf("カメラタイプ（%d）が指定できません\n", camera->type);
    GF_ASSERT(0);
  }
  GFL_ARC_LoadDataOfsByHandle(camera->init_param_handle, FILE_ID,
      camera->type * sizeof(FLD_CAMERA_PARAM), sizeof(FLD_CAMERA_PARAM), result);

}

//============================================================================================
//
//
//		カメラ状態アップデート用関数
//
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	注視点を保持する座標ワークを監視、追随する
 */
//------------------------------------------------------------------
static void updateTargetBinding(FIELD_CAMERA * camera)
{
	if (camera->watch_target) {
		(camera->target) = *(camera->watch_target);
	}
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static BOOL updateCamPosBinding(FIELD_CAMERA * camera)
{
  if (camera->watch_camera)
  {
    (camera->camPos) = *(camera->watch_camera);
    return TRUE;
  }
  return FALSE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void calcAnglePos( const VecFx32* cp_target, VecFx32* p_pos, u16 yaw, u16 pitch, fx32 len )
{
  enum {  PITCH_LIMIT = 0x200 };
  VecFx32 cameraPos;
	fx16 sinYaw = FX_SinIdx(yaw);
	fx16 cosYaw = FX_CosIdx(yaw);
	fx16 sinPitch = FX_SinIdx(pitch);
	fx16 cosPitch = FX_CosIdx(pitch);

	if(cosPitch < 0){ cosPitch = -cosPitch; }	// 裏周りしないようにマイナス値はプラス値に補正
	if(cosPitch < PITCH_LIMIT){ cosPitch = PITCH_LIMIT; }	// 0値近辺は不正表示になるため補正

	// カメラの座標計算
  {
    VEC_Set( &cameraPos, FX_Mul(sinYaw, cosPitch), sinPitch, FX_Mul(cosYaw, cosPitch) );
    VEC_Normalize(&cameraPos, &cameraPos);
    VEC_MultAdd( len, &cameraPos, cp_target, &cameraPos );
  }

  //cameraPos = cameraPos * length + camera->target
  *p_pos = cameraPos;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void updateAngleCameraPos(FIELD_CAMERA * camera)
{
  int i;
  // 
  camera->target_write = camera->target;

  if( camera->camera_area_active )
  {
    for( i=0; i<camera->camera_area_num; i++ )
    {
      cameraArea_UpdateTarget( &camera->camera_area[i], &camera->target_write );
    }
  }
  
  // カメラポジション計算
	calcAnglePos( &camera->target_write, &camera->camPos,
			camera->angle_yaw, camera->angle_pitch, camera->angle_len );

  // 
  camera->campos_write = camera->camPos;

  if( camera->camera_area_active )
  {
    for( i=0; i<camera->camera_area_num; i++ )
    {
      cameraArea_UpdateCamPos( &camera->camera_area[i], &camera->campos_write );
    }
  }
}
static void updateAngleTargetPos(FIELD_CAMERA * camera)
{ 
  int i;
  // 
  camera->campos_write = camera->camPos;

  if( camera->camera_area_active )
  {
    for( i=0; i<camera->camera_area_num; i++ )
    {
      cameraArea_UpdateCamPos( &camera->camera_area[i], &camera->campos_write );
    }
  }
    

  // ターゲットポジション計算
	calcAnglePos( &camera->campos_write, &camera->target,
			camera->angle_yaw, camera->angle_pitch, camera->angle_len );

  // 
  camera->target_write = camera->target;

  if( camera->camera_area_active )
  {
    for( i=0; i<camera->camera_area_num; i++ )
    {
      cameraArea_UpdateTarget( &camera->camera_area[i], &camera->target_write );
    }
  }
}

	
//------------------------------------------------------------------
//------------------------------------------------------------------
static void updateCameraArea(FIELD_CAMERA * camera)
{
  int i;
  
  // 
  camera->campos_write = camera->camPos;

  if( camera->camera_area_active )
  {
    for( i=0; i<camera->camera_area_num; i++ )
    {
      cameraArea_UpdateCamPos( &camera->camera_area[i], &camera->campos_write );
    }
  }

  // 
  camera->target_write = camera->target;
  if( camera->camera_area_active )
  {
    for( i=0; i<camera->camera_area_num; i++ )
    {
      cameraArea_UpdateTarget( &camera->camera_area[i], &camera->target_write );
    }
  }
}
	
//------------------------------------------------------------------
//------------------------------------------------------------------
static void updateG3Dcamera(FIELD_CAMERA * camera)
{
	VecFx32 cameraTarget;

#ifndef PM_DEBUG
	// カメラターゲット補正
  VEC_Add( &camera->target_write, &camera->target_offset, &cameraTarget );
  

	GFL_G3D_CAMERA_SetTarget( camera->g3Dcamera, &cameraTarget );
	GFL_G3D_CAMERA_SetPos( camera->g3Dcamera, &camera->campos_write );

  traceUpdate( camera );
  updateGlobalAngleYaw( camera, &cameraTarget, &camera->campos_write );
#else

  if( camera->debug_subscreen_type != FIELD_CAMERA_DEBUG_BIND_TARGET_POS )
  {
    // 通常のカメラターゲット
    // カメラターゲット補正
    VEC_Add( &camera->target_write, &camera->target_offset, &cameraTarget );

    GFL_G3D_CAMERA_SetTarget( camera->g3Dcamera, &cameraTarget );
	  GFL_G3D_CAMERA_SetPos( camera->g3Dcamera, &camera->campos_write );

    updateGlobalAngleYaw( camera, &cameraTarget, &camera->campos_write );

		if( !camera->debug_trace_off )
		{
			traceUpdate( camera );
		}
  }
  else
  {
    // ターゲットを変更
    calcAnglePos( &camera->camPos, &camera->debug_target, camera->debug_target_yaw,
        camera->debug_target_pitch, camera->debug_target_len );

    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_R )
    {
      VecFx32 target_offset;
      
      OS_TPrintf( "DEBUG Camera Target x[0x%x] y[0x%x] z[0x%x]\n", camera->debug_target.x, camera->debug_target.y, camera->debug_target.z  );
      OS_TPrintf( "DEBUG Camera Pos x[0x%x] y[0x%x] z[0x%x]\n", camera->camPos.x, camera->camPos.y, camera->camPos.z  );

      VEC_Subtract( &camera->debug_target, &camera->target, &target_offset );
      OS_TPrintf( "DEBUG Scroll Camera Target Offset x[0x%x] y[0x%x] z[0x%x]\n", target_offset.x, target_offset.y, target_offset.z  );
    }

    GFL_G3D_CAMERA_SetTarget( camera->g3Dcamera, &camera->debug_target );
    GFL_G3D_CAMERA_SetPos( camera->g3Dcamera, &camera->camPos );

    updateGlobalAngleYaw( camera, &cameraTarget, &camera->campos_write );
  }

  // Far,Fovy座標の設定
  if( camera->debug_subscreen_type != FIELD_CAMERA_DEBUG_BIND_NONE ){
    GFL_G3D_CAMERA_SetFar( camera->g3Dcamera, &camera->debug_far );
    if( GFL_G3D_CAMERA_GetProjectionType(camera->g3Dcamera) == GFL_G3D_PRJPERS ){
      GFL_G3D_CAMERA_SetfovySin( camera->g3Dcamera, FX_SinIdx( camera->fovy ) );
      GFL_G3D_CAMERA_SetfovyCos( camera->g3Dcamera, FX_CosIdx( camera->fovy ) );
    }
  }

#endif


}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void ControlParameter(FIELD_CAMERA * camera, u16 key_cont)
{
	static void (*const cp_Control[FIELD_CAMERA_MODE_MAX])( FIELD_CAMERA * camera, u16 key_cont ) = 
	{
		ControlParameter_CalcCamera,
		ControlParameter_CalcTarget,
		ControlParameter_Direct,
	};
	GF_ASSERT( camera->mode < FIELD_CAMERA_MODE_MAX );
	cp_Control[ camera->mode ]( camera, key_cont );
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラ管理	カメラ座標計算モード
 */
//-----------------------------------------------------------------------------
static void ControlParameter_CalcCamera( FIELD_CAMERA * camera, u16 key_cont )
{
	updateTargetBinding(camera);
  updateAngleCameraPos(camera);
  updateG3Dcamera(camera);
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラ管理	ターゲット座標計算モード
 */
//-----------------------------------------------------------------------------
static void ControlParameter_CalcTarget( FIELD_CAMERA * camera, u16 key_cont )
{
	updateCamPosBinding( camera );
  updateAngleTargetPos(camera);
  updateG3Dcamera(camera);
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラ管理	ダイレクト座標モード
 */
//-----------------------------------------------------------------------------
static void ControlParameter_Direct( FIELD_CAMERA * camera, u16 key_cont )
{
	updateTargetBinding(camera);
	updateCamPosBinding( camera );
  updateCameraArea( camera );
  updateG3Dcamera(camera);
}



//----------------------------------------------------------------------------
/**
 *	@brief  カメラ管理初期化　カメラ座標計算　
 */
//-----------------------------------------------------------------------------
static void ControlParameterInit_CalcCamera( FIELD_CAMERA * camera )
{
  // watch_cameraとのバインドを解除
  camera->watch_camera = NULL;
  //OS_TPrintf( "move change CalcCamera watch_camera set NULL\n" );
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ管理初期化  ターゲット座標計算
 */
//-----------------------------------------------------------------------------
static void ControlParameterInit_CalcTarget( FIELD_CAMERA * camera )
{
  // watch_targetとのバインドを解除
  camera->watch_target = NULL;
  //OS_TPrintf( "move change CalcTarget watch_target set NULL\n" );
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ管理処理  ダイレクト座標計算
 *
 *	@param	camera 
 */
//-----------------------------------------------------------------------------
static void ControlParameterInit_Direct( FIELD_CAMERA * camera )
{
  // アングル情報の破棄
  camera->angle_pitch = 0;
  camera->angle_yaw = 0;
  camera->angle_len = 0;
  //OS_TPrintf( "move change Direct Angle set 0\n" );
}



//------------------------------------------------------------------
//------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  アングルを求める
 */
//-----------------------------------------------------------------------------
static void modeChange_CalcVecAngel( const VecFx32* cp_vec, u16* p_pitch, u16* p_yaw, fx32* p_len )
{
  VecFx32 dist, xz_dist;
  fx32 len;
  fx32 xz_len;
  u16 pitch, yaw;
  
  // 距離
  len = VEC_Mag( cp_vec );
  
  // 計算正規化
  VEC_Normalize( cp_vec, &dist );
  
  // 平面と高さの比率を求める
  VEC_Set( &xz_dist, dist.x, 0, dist.z );
  xz_len = VEC_Mag( &xz_dist );

  // pitck
  pitch = FX_Atan2Idx( dist.y, xz_len );

  // yaw
  yaw = FX_Atan2Idx( dist.x, dist.z );

  *p_pitch = pitch;
  *p_yaw = yaw;
  *p_len = len;
}
//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	camera
 *	@param	cp_vec 
 */
//-----------------------------------------------------------------------------
static void modeChange_SetVecAngel( FIELD_CAMERA * camera, const VecFx32* cp_vec )
{
  u16 pitch, yaw;
  fx32 len;

  modeChange_CalcVecAngel( cp_vec, &pitch, &yaw, &len );
  // 設定
  camera->angle_pitch   = pitch;
  camera->angle_yaw     = yaw;
  camera->angle_len     = len;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラモードをカメラ座標計算モードに変更
 *
 *	@param	camera  カメラワーク
 */
//-----------------------------------------------------------------------------
static void modeChange_CalcCameraPos( FIELD_CAMERA * camera )
{
  VecFx32 dist;

  // 1つ前が、ターゲット計算モードの場合
  if( camera->mode == FIELD_CAMERA_MODE_CALC_TARGET_POS )
  {
    // アングルの反転でＯＫ
    camera->angle_yaw += 0x8000;
    camera->angle_pitch += 0x8000;
  }
  // 直値の場合
  else
  {
    // ターゲットからカメラまでのベクトルを求める
    VEC_Subtract( &camera->camPos, &camera->target, &dist );

    modeChange_SetVecAngel( camera, &dist );
  }
  
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラモードをターゲット座標計算モードに変更
 *
 *	@param	camera  カメラワーク
 */
//-----------------------------------------------------------------------------
static void modeChange_CalcTargetPos( FIELD_CAMERA * camera )
{
  VecFx32 dist;

  // 1つ前が、カメラ計算モードの場合
  if( camera->mode == FIELD_CAMERA_MODE_CALC_CAMERA_POS )
  {
    // アングルの反転でＯＫ
    camera->angle_yaw += 0x8000;
    camera->angle_pitch += 0x8000;
  }
  // 直値の場合
  else
  {
    // カメラからターゲットまでのベクトルを求める
    VEC_Subtract( &camera->target, &camera->camPos, &dist );

    modeChange_SetVecAngel( camera, &dist );
  }
  
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラモードを直接指定モードに変更
 *
 *	@param	camera  カメラワーク
 */
//-----------------------------------------------------------------------------
static void modeChange_CalcDirectPos( FIELD_CAMERA * camera )
{
  // 何もする必要なし
}



//----------------------------------------------------------------------------
/**
 *	@brief  ターゲット座標とカメラ座標から、グローバルアングルを設定
 *
 *	@param	camera          カメラワーク
 *	@param	cp_targetPos    ターゲット座標
 *	@param	cp_camraPos     カメラ座標
 */
//-----------------------------------------------------------------------------
static void updateGlobalAngleYaw( FIELD_CAMERA * camera, const VecFx32* cp_targetPos, const VecFx32* cp_cameraPos )
{
  VecFx32 way;

  VEC_Subtract( cp_cameraPos, cp_targetPos, &way );
  VEC_Normalize( &way, &way );

  camera->globl_angle_yaw = FX_Atan2Idx( way.x, way.z );
}




//============================================================================================
//
//
//			カメラ操作用外部公開関数
//
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
const GFL_G3D_CAMERA * FIELD_CAMERA_GetCameraPtr(const FIELD_CAMERA * camera)
{
	return camera->g3Dcamera;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_CAMERA_BindTarget(FIELD_CAMERA * camera, const VecFx32 * watch_target)
{
	GF_ASSERT( (camera->mode == FIELD_CAMERA_MODE_CALC_CAMERA_POS)||(camera->mode == FIELD_CAMERA_MODE_CALC_TARGET_POS) );
	camera->watch_target = watch_target;
  camera->target_before = *watch_target;
  camera->target = *watch_target;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_CAMERA_FreeTarget(FIELD_CAMERA * camera)
{
	camera->watch_target = NULL;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const VecFx32 *FIELD_CAMERA_GetWatchTarget(FIELD_CAMERA * camera)
{
	return camera->watch_target;
}


//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_CAMERA_BindDefaultTarget(FIELD_CAMERA * camera)
{
  camera->watch_target = camera->default_target;
}

//----------------------------------------------------------------------------
/**
 *	@brief	デフォルトターゲットがあるかチェック
 *
 *	@retval	TRUE	ある
 *	@retval	FALSE	ない
 */
//-----------------------------------------------------------------------------
BOOL FIELD_CAMERA_IsBindDefaultTarget( const FIELD_CAMERA * camera )
{
	if( camera->default_target )
	{
		return TRUE;
	}
	return FALSE;
}


//------------------------------------------------------------------
//------------------------------------------------------------------
/*
void FIELD_CAMERA_BindNoCamera(FIELD_CAMERA * camera, BOOL flag)
{
	if(flag)
	{
		camera->watch_camera = &camera->camPos;
	}
	else
	{
		camera->watch_camera = NULL;
	}
}
//*/
//----------------------------------------------------------------------------
/**
 *	@brief	カメラ座標のバインド
 *
 *	@param	camera				カメラ
 *	@param	watch_camera	カメラついづい座標
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_BindCamera(FIELD_CAMERA * camera, const VecFx32 * watch_camera)
{
	GF_ASSERT( camera );
	GF_ASSERT( (camera->mode == FIELD_CAMERA_MODE_CALC_TARGET_POS) || (camera->mode == FIELD_CAMERA_MODE_DIRECT_POS) );
	camera->watch_camera = watch_camera;
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラ座標のバインド　終了
 *
 *	@param	camera	カメラ
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_FreeCamera(FIELD_CAMERA * camera)
{
	GF_ASSERT( camera );
	camera->watch_camera = NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ座標のバインドがあるかチェック
 *
 *	@param	camera
 *
 *	@retval TRUE  ある
 *	@retval FALSE  ない
 */
//-----------------------------------------------------------------------------
BOOL FIELD_CAMERA_IsBindCamera(const FIELD_CAMERA * camera)
{
	GF_ASSERT( camera );
	if( camera->watch_camera )
  {
    return TRUE;
  }
  return FALSE;
}


//------------------------------------------------------------------
/**
 * @brief	nearクリップ面までの距離を設定
 * @param	camera		FIELDカメラ制御ポインタ
 * @param	near		設定する距離（fx32)
 */
//------------------------------------------------------------------
void FIELD_CAMERA_SetNear(FIELD_CAMERA * camera, fx32 near)
{
	GFL_G3D_CAMERA_SetNear(camera->g3Dcamera, &near);
}

//------------------------------------------------------------------
/**
 * @brief	nearクリップ面までの距離を取得
 * @param	camera		FIELDカメラ制御ポインタ
 * @return	fx32		取得した距離
 */
//------------------------------------------------------------------
fx32 FIELD_CAMERA_GetNear(const FIELD_CAMERA * camera)
{
	fx32 near;
	GFL_G3D_CAMERA_GetNear( camera->g3Dcamera, &near);
	return near;
}

//------------------------------------------------------------------
/**
 * @brief	farクリップ面までの距離を設定
 * @param	camera		FIELDカメラ制御ポインタ
 * @param	far		設定する距離（fx32)
 */
//------------------------------------------------------------------
void FIELD_CAMERA_SetFar(FIELD_CAMERA * camera, fx32 far)
{
	GFL_G3D_CAMERA_SetFar( camera->g3Dcamera, &far );
}

//------------------------------------------------------------------
/**
 * @brief	farクリップ面までの距離を取得
 * @param	camera		FIELDカメラ制御ポインタ
 * @return	fx32		取得した距離
 */
//------------------------------------------------------------------
fx32 FIELD_CAMERA_GetFar(const FIELD_CAMERA * camera)
{
	fx32 far;
	GFL_G3D_CAMERA_GetFar( camera->g3Dcamera, &far);
	return far;
}

//------------------------------------------------------------------
/**
 * @brief	カメラ位置の取得
 * @param	camera		FIELDカメラ制御ポインタ
 * @param	pos			カメラ位置を渡すVecFx32へのポインタ
 */
//------------------------------------------------------------------
void	FIELD_CAMERA_SetTargetPos( FIELD_CAMERA* camera, const VecFx32* target )
{
	GF_ASSERT( (camera->mode == FIELD_CAMERA_MODE_DIRECT_POS) || (camera->mode == FIELD_CAMERA_MODE_CALC_CAMERA_POS) );
	camera->target = *target;
  camera->target_before = * target;
}
//------------------------------------------------------------------
/**
 * @brief	カメラ位置の取得
 * @param	camera		FIELDカメラ制御ポインタ
 * @param	pos			カメラ位置を受け取るVecFx32へのポインタ
 */
//------------------------------------------------------------------
void	FIELD_CAMERA_GetTargetPos( const FIELD_CAMERA* camera, VecFx32* pos )
{
	*pos = camera->target;
}

//------------------------------------------------------------------
/**
 * @brief	カメラ注視点　補正座標の取得
 * @param	camera		    FIELDカメラ制御ポインタ
 * @param	target_offset	カメラ注視点を補正する座標受け取るVecFx32へのポインタ
 */
//------------------------------------------------------------------
void	FIELD_CAMERA_GetTargetOffset( const FIELD_CAMERA* camera, VecFx32* target_offset )
{
  *target_offset = camera->target_offset;
}


//------------------------------------------------------------------
/**
 * @brief	カメラ注視点　補正座標の取得
 * @param	camera		        FIELDカメラ制御ポインタ
 * @param	target_offset			カメラ注視点を補正する座標渡すVecFx32へのポインタ
 */
//------------------------------------------------------------------
void	FIELD_CAMERA_SetTargetOffset( FIELD_CAMERA* camera, const VecFx32* target_offset )
{
  camera->target_offset = *target_offset;
}

//------------------------------------------------------------------
/**
 * @brief カメラ位置の取得
 * @param	camera		        FIELDカメラ制御ポインタ
 * @param camPos            取得位置を格納するVecFx32へのポインタ
 */
//------------------------------------------------------------------
void FIELD_CAMERA_GetCameraPos( const FIELD_CAMERA * camera, VecFx32 * camPos)
{
  *camPos = camera->camPos;
}
//------------------------------------------------------------------
/**
 * @brief カメラ位置のセット
 * @param	camera		        FIELDカメラ制御ポインタ
 * @param camPos            設定する位置情報
 *
 * FIELD_CAMERA内部でカメラ位置が制御されているときには
 * 反映されない場合もあるので注意
 */
//------------------------------------------------------------------
void FIELD_CAMERA_SetCameraPos( FIELD_CAMERA * camera, const VecFx32 * camPos)
{
	GF_ASSERT( (camera->mode == FIELD_CAMERA_MODE_DIRECT_POS) || (camera->mode == FIELD_CAMERA_MODE_CALC_TARGET_POS) );
  camera->camPos = *camPos;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ上方向を設定
 *
 *	@param	camera    カメラ
 *	@param	camUp     カメラ上方向
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_SetCameraUp( FIELD_CAMERA * camera, const VecFx32 * camUp)
{
  GFL_G3D_CAMERA_SetCamUp( camera->g3Dcamera, camUp );
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ上方向の取得
 *
 *	@param	camera  カメラ
 *	@param	camUp   上方向
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_GetCameraUp( const FIELD_CAMERA * camera, VecFx32 * camUp)
{
  GFL_G3D_CAMERA_GetCamUp( camera->g3Dcamera, camUp );
}


//----------------------------------------------------------------------------
/**
 *	@brief	カメラangle　垂直方向回転
 */
//-----------------------------------------------------------------------------
u16 FIELD_CAMERA_GetAnglePitch(const FIELD_CAMERA * camera )
{
	return camera->angle_pitch;
}
void FIELD_CAMERA_SetAnglePitch(FIELD_CAMERA * camera, u16 angle )
{
	GF_ASSERT( (camera->mode == FIELD_CAMERA_MODE_CALC_CAMERA_POS)||(camera->mode == FIELD_CAMERA_MODE_CALC_TARGET_POS) );
	camera->angle_pitch = angle;
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラangle　水平方向回転
 */
//-----------------------------------------------------------------------------
u16 FIELD_CAMERA_GetAngleYaw(const FIELD_CAMERA * camera )
{
	return camera->angle_yaw;
}

void FIELD_CAMERA_SetAngleYaw(FIELD_CAMERA * camera, u16 angle )
{
	GF_ASSERT( (camera->mode == FIELD_CAMERA_MODE_CALC_CAMERA_POS)||(camera->mode == FIELD_CAMERA_MODE_CALC_TARGET_POS) );
	camera->angle_yaw = angle;
}

const u16 * FIELD_CAMERA_GetAngleYawAddress( const FIELD_CAMERA *camera )
{
  return( &camera->globl_angle_yaw );
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラangle　ターゲットからの距離操作
 */
//-----------------------------------------------------------------------------
fx32 FIELD_CAMERA_GetAngleLen(const FIELD_CAMERA * camera )
{
	return camera->angle_len;	
}
void FIELD_CAMERA_SetAngleLen(FIELD_CAMERA * camera, fx32 length )
{
	GF_ASSERT( (camera->mode == FIELD_CAMERA_MODE_CALC_CAMERA_POS)||(camera->mode == FIELD_CAMERA_MODE_CALC_TARGET_POS) );
	camera->angle_len = length;
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラ視野角操作　視野角取得
 *	@param  camera    カメラポインタ
 *
 *	@return u16       視野角
 */
//-----------------------------------------------------------------------------
u16 FIELD_CAMERA_GetFovy(const FIELD_CAMERA * camera )
{
  // パースをカメラの情報から取得
  if( GFL_G3D_CAMERA_GetProjectionType(camera->g3Dcamera) == GFL_G3D_PRJPERS ){
    return camera->fovy;
  }else{
    return 0;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief	カメラ視野角操作　視野角セット
 *	@param  camera    カメラポインタ
 *	@param  fovy       視野角
 *
 *	@return none
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_SetFovy(FIELD_CAMERA * camera, u16 fovy )
{
	camera->fovy = fovy;
  GFL_G3D_CAMERA_SetfovySin( camera->g3Dcamera, FX_SinIdx( camera->fovy ) );
  GFL_G3D_CAMERA_SetfovyCos( camera->g3Dcamera, FX_CosIdx( camera->fovy ) );
}

#ifdef  PM_DEBUG
#include "test/camera_adjust_view.h"
static fx32	fldWipeScale = 0;
//------------------------------------------------------------------
//  デバッグ用：下画面操作とのバインド
//------------------------------------------------------------------
void FIELD_CAMERA_DEBUG_BindSubScreen(FIELD_CAMERA * camera, void * param, FIELD_CAMERA_DEBUG_BIND_TYPE type, HEAPID heapID)
{ 
  GFL_CAMADJUST * gflCamAdjust = param;

  // ワイプ表示初期化
  camera->p_debug_wipe = FLD_WIPEOBJ_Create( heapID );

  // ファー座標を取得
  GFL_G3D_CAMERA_GetFar( camera->g3Dcamera, &camera->debug_far );

  camera->debug_subscreen_type = type;
  camera->debug_trace_off = TRUE;
  if( type == FIELD_CAMERA_DEBUG_BIND_CAMERA_POS )
  {
    camera->debug_save_camera_mode = camera->mode;
	  FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );
    GFL_CAMADJUST_SetCameraParam(	gflCamAdjust,
																	&camera->angle_yaw, 
																	&camera->angle_pitch, 
																	&camera->angle_len, 
																	&camera->fovy, 
																	&camera->debug_far);  
    GFL_CAMADJUST_SetWipeParam(		gflCamAdjust, &fldWipeScale );
  }
  else if( type == FIELD_CAMERA_DEBUG_BIND_TARGET_POS )
  {
    camera->debug_save_camera_mode = camera->mode;
	  FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_CALC_TARGET_POS );
    camera->debug_target_yaw    = camera->angle_yaw;
    camera->debug_target_pitch  = camera->angle_pitch;
    camera->debug_target_len    = camera->angle_len;
    GFL_CAMADJUST_SetCameraParam(	gflCamAdjust,
																	&camera->debug_target_yaw, 
																	&camera->debug_target_pitch, 
																	&camera->debug_target_len, 
																	&camera->fovy, 
																	&camera->debug_far);  
    GFL_CAMADJUST_SetWipeParam(		gflCamAdjust, &fldWipeScale );
  }
}

void FIELD_CAMERA_DEBUG_ReleaseSubScreen(FIELD_CAMERA * camera)
{ 

  if( camera->debug_subscreen_type == FIELD_CAMERA_DEBUG_BIND_TARGET_POS ){

    OS_TPrintf( "DEBUG Camera Target x[0x%x] y[0x%x] z[0x%x]\n", camera->debug_target.x, camera->debug_target.y, camera->debug_target.z  );
    OS_TPrintf( "DEBUG Camera Pos x[0x%x] y[0x%x] z[0x%x]\n", camera->camPos.x, camera->camPos.y, camera->camPos.z  );

    VEC_Subtract( &camera->debug_target, &camera->target, &camera->target_offset );
    OS_TPrintf( "DEBUG Scroll Camera Target Offset x[0x%x] y[0x%x] z[0x%x]\n", camera->target_offset.x, camera->target_offset.y, camera->target_offset.z  );
  }

  camera->debug_subscreen_type = FIELD_CAMERA_DEBUG_BIND_NONE;
  camera->debug_trace_off = FALSE;

  FIELD_CAMERA_ChangeMode( camera, camera->debug_save_camera_mode );

  // ワイプ表示破棄
  FLD_WIPEOBJ_Delete( camera->p_debug_wipe );
  camera->p_debug_wipe = NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief	操作しているターゲットを取得
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_DEBUG_GetBindSubScreenTarget( const FIELD_CAMERA * camera, VecFx32* p_target )
{
	*p_target = camera->debug_target;
}


//----------------------------------------------------------------------------
/**
 *	@brief	デフォルトターゲットの設定
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_DEBUG_SetDefaultTarget( FIELD_CAMERA* camera, const VecFx32* target )
{
	camera->default_target = target;
}

//----------------------------------------------------------------------------
/**
 *	@brief	デフォルトターゲットの取得
 */
//-----------------------------------------------------------------------------
const VecFx32* FIELD_CAMERA_DEBUG_GetDefaultTarget( const FIELD_CAMERA* camera )
{
	return camera->default_target;
}

//----------------------------------------------------------------------------
/**
 *	@brief	デフォルトパラメータの取得
 */
//----------------------------------------------------------------------------- 
void FIELD_CAMERA_GetInitialParameter( const FIELD_CAMERA* camera, FLD_CAMERA_PARAM * result)
{
  // パラメータ読み込み
  loadInitialParameter( camera, result );
}


//----------------------------------------------------------------------------
/**
 *	@brief  カメラデバック３D描画
 *
 *	@param	camera
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_DEBUG_Draw( const FIELD_CAMERA* camera)
{ 
  if(camera->p_debug_wipe)
  {
    FLD_WIPEOBJ_Main( camera->p_debug_wipe, fldWipeScale, 20 );
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  デバックコントロール　情報管理
 */
//-----------------------------------------------------------------------------
static void DEBUG_ControlWork( FIELD_CAMERA* camera )
{
  // 現状のfield_cameraを維持したまま、カメラの平行移動などを行うために、
  // 実際に、gflibのカメラに渡したいカメラ座標から、
  // FIELD_CAMERA_MODE_CALC_CAMERA_POS用のアングルや、
  // ターゲットオフセットを割り出す。
  
  // アングルからデバックカメラ座標を計算
  calcAnglePos( &camera->debug_target, &camera->debug_camera, camera->debug_target_yaw, camera->debug_target_pitch, camera->debug_target_len );
  
  // デバックターゲット
  // デバックカメラの位置から、
  // ターゲットオフセットと
  // アングルを求める
  {
    VecFx32 move_camera;
    
    // ターゲットオフセット
    if( camera->watch_target != NULL )
    {
      VEC_Subtract( &camera->debug_target, &camera->target_write, &camera->target_offset );
    }
    else
    {
      VEC_Set( &camera->target, camera->debug_target.x, camera->debug_target.y, camera->debug_target.z );
      VEC_Set( &camera->target_offset, 0,0,0 );
    }

    // カメラアングル
    VEC_Subtract( &camera->debug_camera, &camera->target_write, &move_camera ); 
    modeChange_CalcVecAngel( &move_camera, &camera->angle_pitch, &camera->angle_yaw, &camera->angle_len );
  }
  
  // カメラ情報を設定
  ControlParameter( camera, 0 );
}




//-----------------------------------------------------------------------------
/**
 *
 */
//-----------------------------------------------------------------------------
#define CAMERA_DEBUG_ANGLE_MOVE  (64)
#define CAMERA_DEBUG_PAERS_MOVE  (64)
#define CAMERA_DEBUG_NEARFAR_MOVE  (FX32_CONST(2))
#define CAMERA_DEBUG_POS_MOVE  (FX32_ONE)
#define CAMERA_DEBUG_LEN_MOVE  (FX32_ONE)
enum
{
  CAMERA_DEBUG_BUFFER_MODE_ZBUFF_MANUAL,
  CAMERA_DEBUG_BUFFER_MODE_WBUFF_MANUAL,
  CAMERA_DEBUG_BUFFER_MODE_ZBUFF_AUTO,
  CAMERA_DEBUG_BUFFER_MODE_WBUFF_AUTO,

  CAMERA_DEBUG_BUFFER_MODE_MAX,
};
//----------------------------------------------------------------------------
/**
 *	@brief  デバックコントロール初期化
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_DEBUG_InitControl( FIELD_CAMERA* camera, HEAPID heapID )
{
  GF_ASSERT( !camera->p_debug_wordset );
  GF_ASSERT( !camera->p_debug_msgdata );

  // trace OFF
  camera->debug_trace_off = TRUE;

  // ワードセット作成
  camera->p_debug_wordset = WORDSET_Create( heapID );
  camera->p_debug_msgdata = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_d_tomoya_dat, heapID );

  camera->p_debug_strbuff   = GFL_STR_CreateBuffer( 512, heapID );
  camera->p_debug_strbuff_tmp = GFL_STR_CreateBuffer( 512, heapID );

  // フォントデータ
  camera->p_debug_font = GFL_FONT_Create(
    ARCID_FONT, NARC_font_large_gftr,
    GFL_FONT_LOADTYPE_FILE, FALSE, heapID );

  GFL_UI_KEY_SetRepeatSpeed( 1,8 );

  // 今の設定カメラパラメータを取得
  {
    VecFx32 move_camera;
    GFL_G3D_CAMERA_GetTarget( camera->g3Dcamera, &camera->debug_target );
    GFL_G3D_CAMERA_GetPos( camera->g3Dcamera, &camera->debug_camera );

    // アングルを求る
    VEC_Subtract( &camera->debug_camera, &camera->debug_target, &move_camera );
    modeChange_CalcVecAngel( &move_camera, &camera->debug_target_pitch, &camera->debug_target_yaw, &camera->debug_target_len );
  }
  
  DEBUG_ControlWork( camera );

}

//----------------------------------------------------------------------------
/**
 *	@brief  デバックコントロール破棄
 *
 *	@param	camera 
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_DEBUG_ExitControl( FIELD_CAMERA* camera )
{
  // フォントデータ
  GFL_FONT_Delete( camera->p_debug_font );
  camera->p_debug_font = NULL;


  GFL_MSG_Delete( camera->p_debug_msgdata );
  camera->p_debug_msgdata = NULL;

  WORDSET_Delete( camera->p_debug_wordset );
  camera->p_debug_wordset = NULL;

  GFL_STR_DeleteBuffer( camera->p_debug_strbuff );
  camera->p_debug_strbuff = NULL;
  GFL_STR_DeleteBuffer( camera->p_debug_strbuff_tmp );
  camera->p_debug_strbuff_tmp = NULL;

  GFL_UI_KEY_SetRepeatSpeed( 8,15 );

  // trace ON
  camera->debug_trace_off = FALSE;
}



//----------------------------------------------------------------------------
/**
 *	@brief  アングル動作の有無チェック
 *
 *	@param	pitch 
 *
 *	@retval TRUE    ＯＫ
 *	@retval FALSE   ＮＧ
 */
//-----------------------------------------------------------------------------
static BOOL DEBUG_is_PitchAngleMove( u16 pitch )
{
  if( (((pitch - CAMERA_DEBUG_ANGLE_MOVE) > GFL_CALC_GET_ROTA_NUM(270)) && ((pitch - CAMERA_DEBUG_ANGLE_MOVE) < 0x10000)) ||
      (((pitch - CAMERA_DEBUG_ANGLE_MOVE) >= GFL_CALC_GET_ROTA_NUM(0)) && ((pitch - CAMERA_DEBUG_ANGLE_MOVE) < GFL_CALC_GET_ROTA_NUM(90))) )
  {
    return TRUE;
  }
  return FALSE;
}



//----------------------------------------------------------------------------
/**
 *	@brief  フィールドカメラ  コントロール詳細
 *
 *	@param	camera      カメラ  
 *	@param	trg         キー情報
 *	@param	cont        
 *	@param	repeat      
 *
 *  NONE：ターゲット、カメラ平行移動
 *	B：カメラ公転（ターゲット座標が変わる）
 *	Y：カメラ自転（カメラ座標が変わる）
 *	A：パース操作
 *	X：ターゲットカメラバインドのON・OFF
 *	
 *	START：バッファリングモード変更
 */
//-----------------------------------------------------------------------------
BOOL FIELD_CAMERA_DEBUG_Control( FIELD_CAMERA* camera, int trg, int cont, int repeat )
{
  BOOL ret = FALSE;
  
  FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );

  // カメラ公転（ターゲット座標変更　カメラオフセットを動かす）
  if( cont & PAD_BUTTON_B )
  {
    u16 pitch, yaw;

    // ターゲットを動かすためのアングルに変換
    pitch = camera->debug_target_pitch;
    yaw   = camera->debug_target_yaw;
     
    if( repeat & PAD_KEY_UP )
    {
      if( DEBUG_is_PitchAngleMove(pitch - CAMERA_DEBUG_ANGLE_MOVE) )
      {
        pitch -= CAMERA_DEBUG_ANGLE_MOVE;
        ret = TRUE;
      }
    }
    else if( repeat & PAD_KEY_DOWN )
    {
      if( DEBUG_is_PitchAngleMove(pitch + CAMERA_DEBUG_ANGLE_MOVE) )
      {
        pitch += CAMERA_DEBUG_ANGLE_MOVE;
        ret = TRUE;
      }
    }

    if( repeat & PAD_KEY_LEFT )
    {
      yaw += CAMERA_DEBUG_ANGLE_MOVE;
      ret = TRUE;
    }
    else if( repeat & PAD_KEY_RIGHT )
    {
      yaw -= CAMERA_DEBUG_ANGLE_MOVE;
      ret = TRUE;
    }

    if( repeat & PAD_BUTTON_L )
    {
      camera->debug_target_len -= CAMERA_DEBUG_LEN_MOVE;
      ret = TRUE;
    }
    else if( repeat & PAD_BUTTON_R )
    {
      camera->debug_target_len += CAMERA_DEBUG_LEN_MOVE;
      ret = TRUE;
    }


    // 変更地を保存
    camera->debug_target_pitch  = pitch;
    camera->debug_target_yaw    = yaw;

    // 変更座標を求める
    if( ret )
    {
      pitch += 0x8000;  // 方向を反転
      yaw   += 0x8000;
      
      // ターゲットの座標を求める
      calcAnglePos( &camera->debug_camera, &camera->debug_target, yaw, pitch, camera->debug_target_len );
    }
  }
  // カメラ自転（カメラ座標変更）
  else if( cont & PAD_BUTTON_Y )
  {

    if( repeat & PAD_KEY_UP )
    {
      if( DEBUG_is_PitchAngleMove(camera->debug_target_pitch + CAMERA_DEBUG_ANGLE_MOVE) )
      {
        camera->debug_target_pitch += CAMERA_DEBUG_ANGLE_MOVE;
        ret = TRUE;
      }
    }
    else if( repeat & PAD_KEY_DOWN )
    {
      if( DEBUG_is_PitchAngleMove(camera->angle_pitch - CAMERA_DEBUG_ANGLE_MOVE) )
      {
        camera->debug_target_pitch -= CAMERA_DEBUG_ANGLE_MOVE;
        ret = TRUE;
      }
    }

    if( repeat & PAD_KEY_LEFT )
    {
      camera->debug_target_yaw -= CAMERA_DEBUG_ANGLE_MOVE;
      ret = TRUE;
    }
    else if( repeat & PAD_KEY_RIGHT )
    {
      camera->debug_target_yaw += CAMERA_DEBUG_ANGLE_MOVE;
      ret = TRUE;
    }

  }
  // パース操作
  else if( cont & PAD_BUTTON_A )
  {
    if( repeat & PAD_KEY_UP )
    {
      if( (camera->fovy + CAMERA_DEBUG_PAERS_MOVE) < GFL_CALC_GET_ROTA_NUM(90) )
      {
        camera->fovy += CAMERA_DEBUG_PAERS_MOVE;
        ret = TRUE;
      }
    }
    else if( repeat & PAD_KEY_DOWN )
    {
      if( (camera->fovy - CAMERA_DEBUG_PAERS_MOVE) > 0 )
      {
        camera->fovy -= CAMERA_DEBUG_PAERS_MOVE;
        ret = TRUE;
      }
    }
    // 左右では、ドット崩れのない座標を求める 
    else if( (repeat & PAD_KEY_LEFT) || (repeat & PAD_KEY_RIGHT) )
    {
      fx32 dist;

      if( repeat & PAD_KEY_LEFT )
      {
        if( (camera->fovy + CAMERA_DEBUG_PAERS_MOVE) < GFL_CALC_GET_ROTA_NUM(90) )
        {
          camera->fovy += CAMERA_DEBUG_PAERS_MOVE;
          ret = TRUE;
        }
      }
      else if( repeat & PAD_KEY_RIGHT )
      {  
        if( (camera->fovy - CAMERA_DEBUG_PAERS_MOVE) > 0 )
        {
          camera->fovy -= CAMERA_DEBUG_PAERS_MOVE;
          ret = TRUE;
        }
      }

      //カメラの中心にある2D画像が、崩れずに表示できる距離を求める
      //つぶれずに表示するためには、1Dot　＝　1Unitで計算すればよい
      //カメラが画面中心を見つめるとき、画面半分のサイズは、96Dot
      //表示させたい、カメラのパースを考え、96Dot　＝　96Unitになる距離にすればよい
      dist = FX_Div( FX_Mul( FX_CosIdx( camera->fovy ), FX_F32_TO_FX32(96) ),	
                 FX_SinIdx( camera->fovy ));					

      camera->debug_target_len = dist; 
    }

    if( ret )
    {
      GFL_G3D_CAMERA_SetfovySin( camera->g3Dcamera, FX_SinIdx( camera->fovy ) );
      GFL_G3D_CAMERA_SetfovyCos( camera->g3Dcamera, FX_CosIdx( camera->fovy ) );
    }
  }
  // Near Far操作
  else if( cont & PAD_BUTTON_DEBUG )
  {
    fx32 near, far;

    GFL_G3D_CAMERA_GetNear(camera->g3Dcamera, &near);
    GFL_G3D_CAMERA_GetFar(camera->g3Dcamera, &far);

    if( repeat & PAD_KEY_UP )
    {
      if( (near + CAMERA_DEBUG_NEARFAR_MOVE) < far )
      {
        near += CAMERA_DEBUG_NEARFAR_MOVE;
        ret = TRUE;
      }
    }
    else if( repeat & PAD_KEY_DOWN )
    {
      if( (near - CAMERA_DEBUG_NEARFAR_MOVE) > 0 )
      {
        near -= CAMERA_DEBUG_NEARFAR_MOVE;
        ret = TRUE;
      }
    }

    if( repeat & PAD_KEY_LEFT )
    {
      if( (far - CAMERA_DEBUG_NEARFAR_MOVE) > near )
      {
        far -= CAMERA_DEBUG_NEARFAR_MOVE;
        ret = TRUE;
      }
    }
    else if( repeat & PAD_KEY_RIGHT )
    {
      far += CAMERA_DEBUG_NEARFAR_MOVE;
      ret = TRUE;
    }

    if( ret )
    {
      GFL_G3D_CAMERA_SetNear(camera->g3Dcamera, &near);
      GFL_G3D_CAMERA_SetFar(camera->g3Dcamera, &far);
    }
  }
  // 平行移動
  else 
  {
    // ターゲットオフセットを変更
    if( repeat & PAD_KEY_UP )
    {
      camera->debug_target.z -= CAMERA_DEBUG_POS_MOVE;
      ret = TRUE;
    }
    else if( repeat & PAD_KEY_DOWN )
    {
      camera->debug_target.z += CAMERA_DEBUG_POS_MOVE;
      ret = TRUE;
    }
    if( repeat & PAD_KEY_LEFT )
    {
      camera->debug_target.x -= CAMERA_DEBUG_POS_MOVE;
      ret = TRUE;
    }
    else if( repeat & PAD_KEY_RIGHT )
    {
      camera->debug_target.x += CAMERA_DEBUG_POS_MOVE;
      ret = TRUE;
    }
    if( repeat & PAD_BUTTON_L )
    {
      camera->debug_target.y -= CAMERA_DEBUG_POS_MOVE;
      ret = TRUE;
    }
    else if( repeat & PAD_BUTTON_R )
    {
      camera->debug_target.y += CAMERA_DEBUG_POS_MOVE;
      ret = TRUE;
    }

  }
   
  //  ターゲットのバインドOn・Off
  if( trg & PAD_BUTTON_X )
  {
    if( camera->watch_target )
    {
      FIELD_CAMERA_FreeTarget( camera );
    }
    else
    {
      FIELD_CAMERA_BindDefaultTarget( camera );
    }
    ret = TRUE;
  }

  // バッファリングモード変更
  if( trg & PAD_BUTTON_START )
  {
    camera->debug_save_buffer_mode = (camera->debug_save_buffer_mode + 1) % CAMERA_DEBUG_BUFFER_MODE_MAX;
    
    switch( camera->debug_save_buffer_mode )
    {
    case CAMERA_DEBUG_BUFFER_MODE_ZBUFF_MANUAL:
      GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_MANUAL, GX_BUFFERMODE_Z );
      ret = TRUE;
      break;
    case CAMERA_DEBUG_BUFFER_MODE_WBUFF_MANUAL:
      GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_MANUAL, GX_BUFFERMODE_W );
      ret = TRUE;
      break;
    case CAMERA_DEBUG_BUFFER_MODE_ZBUFF_AUTO:
      GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_Z );
      ret = TRUE;
      break;
    case CAMERA_DEBUG_BUFFER_MODE_WBUFF_AUTO:
      GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_W );
      ret = TRUE;
      break;
    }
  }

  DEBUG_ControlWork( camera );

  return ret;
}


//----------------------------------------------------------------------------
/**
 *	@brief  カメラ情報を書き込む
 *
 *	@param	camera    カメラ
 *	@param	p_win     ウィンドウ
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_DEBUG_DrawInfo( FIELD_CAMERA* camera, GFL_BMPWIN* p_win, fx32 map_size_x, fx32 map_size_z )
{
  //クリア
  
  // カメラアングル
  {
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 0, camera->angle_pitch, 4, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 1, camera->angle_yaw, 4, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 2, FX_Whole(camera->angle_len), 4, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );

    GFL_MSG_GetString( camera->p_debug_msgdata, CAMERA_DELICATE_ANGLE, camera->p_debug_strbuff_tmp );

    WORDSET_ExpandStr( camera->p_debug_wordset, camera->p_debug_strbuff, camera->p_debug_strbuff_tmp );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), 0, 0, camera->p_debug_strbuff, camera->p_debug_font );
  }

  // 座標
  {
    VecFx32 camerapos, target;

    GFL_G3D_CAMERA_GetTarget( camera->g3Dcamera, &target );
    GFL_G3D_CAMERA_GetPos( camera->g3Dcamera, &camerapos );
    
    // カメラ
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 0, FX_Whole(camerapos.x), 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 1, FX_Whole(camerapos.y), 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 2, FX_Whole(camerapos.z), 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 3, FX_Whole(target.x), 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 4, FX_Whole(target.y), 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 5, FX_Whole(target.z), 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );

    GFL_MSG_GetString( camera->p_debug_msgdata, CAMERA_DELICATE_, camera->p_debug_strbuff_tmp );

    WORDSET_ExpandStr( camera->p_debug_wordset, camera->p_debug_strbuff, camera->p_debug_strbuff_tmp );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), 0, 32, camera->p_debug_strbuff, camera->p_debug_font );
  }

  // ターゲットオフセット
  {
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 0, FX_Whole(camera->target_offset.x), 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 1, FX_Whole(camera->target_offset.y), 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 2, FX_Whole(camera->target_offset.z), 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );

    GFL_MSG_GetString( camera->p_debug_msgdata, CAMERA_DELICATE_OFFSET, camera->p_debug_strbuff_tmp );

    WORDSET_ExpandStr( camera->p_debug_wordset, camera->p_debug_strbuff, camera->p_debug_strbuff_tmp );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), 0, 64, camera->p_debug_strbuff, camera->p_debug_font );
  }

  // パース
  // Near Far
  {
    fx32 near, far;
    
    GFL_G3D_CAMERA_GetNear(camera->g3Dcamera, &near);
    GFL_G3D_CAMERA_GetFar(camera->g3Dcamera, &far);

    OS_TPrintf( "far = 0x%x\n", far );
    
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 0, camera->fovy, 4, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 1, FX_Whole(near), 4, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 2, FX_Whole(far), 4, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );

    GFL_MSG_GetString( camera->p_debug_msgdata, CAMERA_DELICATE_PARS, camera->p_debug_strbuff_tmp );

    WORDSET_ExpandStr( camera->p_debug_wordset, camera->p_debug_strbuff, camera->p_debug_strbuff_tmp );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), 0, 96, camera->p_debug_strbuff, camera->p_debug_font );
  }
  

  // バッファリング
  {
    GFL_MSG_GetString( camera->p_debug_msgdata, CAMERA_DELICATE_BUFF + camera->debug_save_buffer_mode, camera->p_debug_strbuff_tmp );

    WORDSET_ExpandStr( camera->p_debug_wordset, camera->p_debug_strbuff, camera->p_debug_strbuff_tmp );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), 0, 112, camera->p_debug_strbuff, camera->p_debug_font );
  }

  // 主人公追尾
  {
    if( camera->watch_target )
    {
      GFL_MSG_GetString( camera->p_debug_msgdata, CAMERA_DELICATE_BIND, camera->p_debug_strbuff_tmp );
    }
    else
    {
      GFL_MSG_GetString( camera->p_debug_msgdata, CAMERA_DELICATE_BIND0002, camera->p_debug_strbuff_tmp );
    }

    WORDSET_ExpandStr( camera->p_debug_wordset, camera->p_debug_strbuff, camera->p_debug_strbuff_tmp );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), 0, 128, camera->p_debug_strbuff, camera->p_debug_font );
  }

  // Maya Camera Anime Trans
  // 小数点　第2位まで表示
  {
    f32 f_x, f_y, f_z;
    s32 x, y, z;
    u16 rot_x, rot_y, rot_z;
    fx32 xz_dist;
    VecFx32 camerapos, target, xz_vec;

    GFL_G3D_CAMERA_GetTarget( camera->g3Dcamera, &target );
    GFL_G3D_CAMERA_GetPos( camera->g3Dcamera, &camerapos );

    {
      fx32 half_x, half_z;
      half_x = FX_Div( map_size_x, 2<<FX32_SHIFT );
      half_z = FX_Div( map_size_z, 2<<FX32_SHIFT );
      OS_Printf( "Maya Trans x[%f] y[%f] z[%f]\n", FX_FX32_TO_F32(camerapos.x - half_x), FX_FX32_TO_F32(camerapos.y), FX_FX32_TO_F32(camerapos.z - half_z) );
    }
    
    // カメラ座標を表示
    // 小数点以下無視
    x = FX_Whole( camerapos.x ) - (FX_Whole(map_size_x)/2);
    y = FX_Whole( camerapos.y );
    z = FX_Whole( camerapos.z ) - (FX_Whole(map_size_z)/2);

    f_x = FX_FX32_TO_F32(camerapos.x - FX_Floor(camerapos.x));
    f_y = FX_FX32_TO_F32(camerapos.y - FX_Floor(camerapos.y));
    f_z = FX_FX32_TO_F32(camerapos.z - FX_Floor(camerapos.z));

    /// 整数部
    WORDSET_RegisterNumber( camera->p_debug_wordset, 0, x, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
    WORDSET_RegisterNumber( camera->p_debug_wordset, 1, y, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
    WORDSET_RegisterNumber( camera->p_debug_wordset, 2, z, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );

    /// 小数部
    x = (s32)(f_x * 100.0f);
    y = (s32)(f_y * 100.0f);
    z = (s32)(f_z * 100.0f);
    WORDSET_RegisterNumber( camera->p_debug_wordset, 3, x, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    WORDSET_RegisterNumber( camera->p_debug_wordset, 4, y, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    WORDSET_RegisterNumber( camera->p_debug_wordset, 5, z, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );

    GFL_MSG_GetString( camera->p_debug_msgdata, CAMERA_MAYA_TRANS, camera->p_debug_strbuff_tmp );

    WORDSET_ExpandStr( camera->p_debug_wordset, camera->p_debug_strbuff, camera->p_debug_strbuff_tmp );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), 0, 144, camera->p_debug_strbuff, camera->p_debug_font );

    // カメラからターゲットへのベクトルを生成
    VEC_Subtract( &target, &camerapos, &target );

    // XZ平面の距離を求める
    VEC_Set( &xz_vec, target.x, 0, target.z );
    xz_dist = VEC_Mag( &xz_vec );

    // 角度を求める
    // XZ平面の角度
    // 平面とY方向の角度
    rot_y = FX_Atan2Idx( -target.x, -target.z );
    rot_x = FX_Atan2Idx( target.y, xz_dist );
    rot_z = 0;

    f_x = (f32)(rot_x / (f32)0x10000) * 360.0f;
    f_y = (f32)(rot_y / (f32)0x10000) * 360.0f;

    {
      OS_Printf( "Maya Rotate x[%f] y[%f]\n", f_x, f_y );
    }
 
    x = (s32)f_x;
    y = (s32)f_y;
    z = 0;
    
    /// 整数部
    WORDSET_RegisterNumber( camera->p_debug_wordset, 0, x, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
    WORDSET_RegisterNumber( camera->p_debug_wordset, 1, y, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
    WORDSET_RegisterNumber( camera->p_debug_wordset, 2, z, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );

    /// 小数部
    f_x -= x;
    f_y -= y;
    x = (s32)(f_x * 100.0f);
    y = (s32)(f_y * 100.0f);
    WORDSET_RegisterNumber( camera->p_debug_wordset, 3, x, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    WORDSET_RegisterNumber( camera->p_debug_wordset, 4, y, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    WORDSET_RegisterNumber( camera->p_debug_wordset, 5, z, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );

    GFL_MSG_GetString( camera->p_debug_msgdata, CAMERA_MAYA_ROTATE, camera->p_debug_strbuff_tmp );

    WORDSET_ExpandStr( camera->p_debug_wordset, camera->p_debug_strbuff, camera->p_debug_strbuff_tmp );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), 0, 160, camera->p_debug_strbuff, camera->p_debug_font );
  }

  GFL_BMPWIN_TransVramCharacter( p_win );
}


//----------------------------------------------------------------------------
/**
 *	@brief  カメラ操作方法を書き込む
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_DEBUG_DrawControlHelp( FIELD_CAMERA* camera, GFL_BMPWIN* p_win )
{
  GFL_MSG_GetString( camera->p_debug_msgdata, CAMERA_DELICATE_HELP, camera->p_debug_strbuff_tmp );

  WORDSET_ExpandStr( camera->p_debug_wordset, camera->p_debug_strbuff, camera->p_debug_strbuff_tmp );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), 0, 0, camera->p_debug_strbuff, camera->p_debug_font );

  GFL_BMPWIN_TransVramCharacter( p_win );
}

#endif  //PM_DEBUG

//---------------------------------------------------------------------------
/**
 * @brief
 */
//---------------------------------------------------------------------------
static void updateTraceData(CAMERA_TRACE * trace,
    const VecFx32 * inTarget, const VecFx32 * inCamPos, VecFx32 * outTarget, VecFx32 * outCamPos)
{
	int cam_ofs;
	int tgt_ofs;

	if (trace==NULL)
  {
		*outTarget = *inTarget;
		*outCamPos = *inCamPos;
    return;
  }
  if (!trace->Valid)
  {
    *outTarget = *inTarget;
		*outCamPos = *inCamPos;
    return;
  }

  cam_ofs = trace->CamPoint;    // 参照位置
  tgt_ofs = trace->TargetPoint; // 格納位置

  if (trace->UpdateFlg){
    //履歴データから座標取得
    *outTarget = trace->targetBuffer[cam_ofs];
    *outCamPos = trace->camPosBuffer[cam_ofs];
  }
  else
  {
		*outTarget = *inTarget;
		*outCamPos = *inCamPos;
    if (cam_ofs==trace->Delay){
      trace->UpdateFlg = TRUE;
    }
  }

  //ストップリクエストが出ている場合
  if (trace->StopReq)
  {
    //ターゲットに追いつくまで参照位置更新
    if (cam_ofs!=tgt_ofs){
      cam_ofs = (cam_ofs+1) % trace->bufsize;

      //追いついた
      if( cam_ofs == tgt_ofs )
      {
        //トレースをストップする
        trace->StopReq = FALSE;
        trace->Valid = FALSE;
        trace->UpdateFlg = FALSE;
      }
    }
  }
  else
  {
    //参照位置更新
    cam_ofs = (cam_ofs+1) % trace->bufsize;
    //履歴に積む
    trace->targetBuffer[tgt_ofs] = *inTarget;
    trace->camPosBuffer[tgt_ofs] = *inCamPos;
    //書き換え位置更新
    tgt_ofs = (tgt_ofs+1) % trace->bufsize;
  }

  trace->CamPoint = cam_ofs;    // 参照位置
  trace->TargetPoint = tgt_ofs; // 格納位置

  //トレースデータの無効座標軸は、そのまま現在座標を採用
  if (!trace->ValidX){	//ｘ無効判定
    outTarget->x = inTarget->x;
    outCamPos->x = inCamPos->x;
  }
  if (!trace->ValidY){	//ｙ無効判定
    outTarget->y = inTarget->y;
    outCamPos->y = inCamPos->y;
  }
  if (!trace->ValidZ){	//ｚ無効判定
    outTarget->z = inTarget->z;
    outCamPos->z = inCamPos->z;
  }
}


//---------------------------------------------------------------------------
/**
 * @brief	トレース構造体のアロケーション　カメラが対象物にバインド済みであること
 * 
 * @param	inHistNum		バッファサイズ
 * @param	inDelay			遅延度合い
 * @param	inTraceMask		トレースマスク	camera.hに定義
 * @param	inHeapID		ヒープID
 * @param	ioCamera		カメラポインタ
 * 
 * @return	none
 */
//---------------------------------------------------------------------------
void createTraceData(const int inHistNum, const int inDelay,
						const int inTraceMask, const int inHeapID,
						FIELD_CAMERA * ioCamera)
{
	int i;
	CAMERA_TRACE *trace;
	//バインドされていなければ、処理しない
	if (ioCamera->watch_target == NULL){
		return;
	}
	//配列サイズの整合性チェック
	GF_ASSERT_MSG(inDelay+1<=inHistNum, "配列数が足りません");
	
	trace = GFL_HEAP_AllocMemory(inHeapID,sizeof(CAMERA_TRACE));
  trace->targetBuffer = GFL_HEAP_AllocMemory(inHeapID, sizeof(VecFx32)*inHistNum);
  trace->camPosBuffer = GFL_HEAP_AllocMemory(inHeapID, sizeof(VecFx32)*inHistNum);
	
	//トレース配列クリア
	for(i=0;i<inHistNum;i++){
    VEC_Set(&trace->targetBuffer[i], 0, 0, 0);
    VEC_Set(&trace->camPosBuffer[i], 0, 0, 0);
	}
	
	trace->bufsize = inHistNum;
	//０番目にカメラ参照位置をセット
	trace->CamPoint = 0;
	//対象物参照位置セット
	trace->TargetPoint = 0+inDelay;
	
	trace->Delay = inDelay;
	trace->UpdateFlg = FALSE;
	
	trace->ValidX = FALSE;
	trace->ValidY = FALSE;
	trace->ValidZ = FALSE;
	if (inTraceMask & CAM_TRACE_MASK_X){
		trace->ValidX = TRUE;
	}
	if (inTraceMask & CAM_TRACE_MASK_Y){
		trace->ValidY = TRUE;
	}
	if (inTraceMask & CAM_TRACE_MASK_Z){
		trace->ValidZ = TRUE;
	}

  trace->Valid = TRUE;
  trace->StopReq = FALSE;

	ioCamera->Trace = trace;
}

//---------------------------------------------------------------------------
/**
 * @brief	トレースデータの解放
 * 
 * @param	camera_ptr		カメラポインタ
 * 
 * @return	none
 */
//---------------------------------------------------------------------------
void deleteTraceData(FIELD_CAMERA * camera_ptr)
{
	if (camera_ptr->Trace!=NULL){
    GFL_HEAP_FreeMemory(camera_ptr->Trace->targetBuffer);
    GFL_HEAP_FreeMemory(camera_ptr->Trace->camPosBuffer);
		GFL_HEAP_FreeMemory(camera_ptr->Trace);
		camera_ptr->Trace = NULL;
	}
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static void traceUpdate(FIELD_CAMERA * camera)
{
  VecFx32 cam, tgt;
  VecFx32 outCam, outTgt;

  if (camera->watch_target == NULL) return;


  GFL_G3D_CAMERA_GetTarget(camera->g3Dcamera, &tgt);
  GFL_G3D_CAMERA_GetPos(camera->g3Dcamera, &cam);

  updateTraceData( camera->Trace, &tgt, &cam, &outTgt, &outCam );

  GFL_G3D_CAMERA_SetTarget( camera->g3Dcamera, &outTgt );
  GFL_G3D_CAMERA_SetPos( camera->g3Dcamera, &outCam );
}

//----------------------------------------------------------------------------
/**
 *	@brief  トレース処理の再開
 *
 *	@param	camera_ptr  カメラポインタ
 *	@return none
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_RestartTrace(FIELD_CAMERA * camera_ptr)
{
  CAMERA_TRACE *trace;
  trace = camera_ptr->Trace;
  if (trace == NULL){
    GF_ASSERT(0);
    return;
  }
  //バッファの初期位置の設定
  {
    int i;
    //トレース配列クリア
	  for(i=0;i<trace->bufsize;i++){
      VEC_Set(&trace->targetBuffer[i], 0, 0, 0);
      VEC_Set(&trace->camPosBuffer[i], 0, 0, 0);
	  }
    //０番目にカメラ参照位置をセット
    trace->CamPoint = 0;
    //対象物参照位置セット
    trace->TargetPoint = 0+trace->Delay;
  }

  trace->UpdateFlg = FALSE;
  //メイン有効フラグをＯＮ
  trace->Valid = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  トレース処理の停止リクエスト
 *
 *	@param	camera_ptr  カメラポインタ
 *	@return none
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_StopTraceRequest(FIELD_CAMERA * camera_ptr)
{
  CAMERA_TRACE *trace;
  trace = camera_ptr->Trace;
  if (trace == NULL){
    GF_ASSERT(0);
    return;
  }
  trace->StopReq = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  今すぐにトレース処理を停止するリクエスト
 *
 *	@param	camera_ptr  カメラポインタ
 *	@return none
 *
 *	トレース座標が違う場合に使用すると、カメラが急に飛ぶような動作を行います。
 *	注意して使用してください。
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_StopTraceNow(FIELD_CAMERA * camera_ptr)
{
  CAMERA_TRACE *trace;
  trace = camera_ptr->Trace;
  if (trace == NULL){
    return;
  }

#ifdef PM_DEBUG
  // Y座標に差が出ないかチェック
  {
    int i;
    s32 check_idx, last_check_idx;

    last_check_idx = trace->TargetPoint-1;
    if( last_check_idx < 0 )
    {
      last_check_idx += trace->bufsize;
    }
    for( i=0; i<trace->bufsize; i++ )
    {
      // 参照ポジションから格納ポジションまでのY座標が一緒かチェック
      check_idx = (trace->CamPoint + i) % trace->bufsize;

      // 終了条件
      if( check_idx == last_check_idx )
      {
        break;
      }

      if( (trace->camPosBuffer[check_idx].y != trace->camPosBuffer[last_check_idx].y) ||
          (trace->targetBuffer[check_idx].y != trace->targetBuffer[last_check_idx].y) )
      {
        OS_TPrintf( "FIELD_CAMERA_StopTraceNow err トレース情報に中身が入っています\n" );
        GF_ASSERT( (trace->camPosBuffer[check_idx].y == trace->camPosBuffer[last_check_idx].y) &&
           (trace->targetBuffer[check_idx].y == trace->targetBuffer[last_check_idx].y) );
      }
    }
  }
#endif

  //トレースをストップする
  trace->StopReq = FALSE;
  trace->Valid = FALSE;
  trace->UpdateFlg = FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  トレース処理が動いているかのチェック
 *
 *	@param	camera_ptr  カメラポインタ
 *	@return BOOL        TRUEで稼働中
 */
//-----------------------------------------------------------------------------
BOOL FIELD_CAMERA_CheckTrace(FIELD_CAMERA * camera_ptr)
{
  CAMERA_TRACE *trace;
  trace = camera_ptr->Trace;
  if (trace == NULL){
    GF_ASSERT(0);
    return FALSE;
  }
  return trace->Valid;
}

//----------------------------------------------------------------------------
/**
 *	@brief  トレースシステムが有効であるかをチェック
 *
 *	@param	camera_ptr  カメラポインタ
 *	@return BOOL        TRUEで稼働中
 */
//-----------------------------------------------------------------------------
BOOL FIELD_CAMERA_CheckTraceSys(FIELD_CAMERA * camera_ptr)
{
  CAMERA_TRACE *trace;

  //ウォッチターゲットがない場合は稼動動いていないとみなす。
  if (camera_ptr->watch_target == NULL){
		return FALSE;
	}
  trace = camera_ptr->Trace;
  if (trace == NULL){
    return FALSE;
  }
  return TRUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  カメラ可動エリアの設定
 *
 *	@param	camera      カメラ
 *	@param	cp_area     エリア情報
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_SetCameraArea( FIELD_CAMERA * camera, const FIELD_CAMERA_AREA_SET* cp_area )
{
  GF_ASSERT( cp_area->num < FIELD_CAMERA_AREA_DATA_MAX );
  GFL_STD_MemCopy( cp_area->buff, camera->camera_area, sizeof(FIELD_CAMERA_AREA) * cp_area->num );
  camera->camera_area_num = cp_area->num;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ可動エリアのクリア
 *
 *	@param	camera  カメラ
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_ClearCameraArea( FIELD_CAMERA * camera )
{
  GF_ASSERT( camera );
  GFL_STD_MemClear( camera->camera_area, sizeof(FIELD_CAMERA_AREA) * FIELD_CAMERA_AREA_DATA_MAX );
  camera->camera_area_num = 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ可動エリアの数を取得
 *
 *	@param	camera  カメラ
 *
 *	@return カメラ可動エリアの数
 */
//-----------------------------------------------------------------------------
u32 FIELD_CAMERA_GetCameraAreaNum( const FIELD_CAMERA * camera )
{
  GF_ASSERT( camera );
  return camera->camera_area_num;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ可動エリア　範囲タイプの取得
 *
 *	@param	camera  カメラ
 *
 *	@return 範囲タイプ
 */
//-----------------------------------------------------------------------------
FIELD_CAMERA_AREA_TYPE FIELD_CAMERA_GetCameraAreaType( const FIELD_CAMERA * camera, u32 idx )
{
  GF_ASSERT( camera );
  GF_ASSERT( camera->camera_area_num > idx );
  return camera->camera_area[idx].area_type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ可動エリア  管理座標タイプの取得
 *
 *	@param	camera  カメラ
 *
 *	@return 管理座標タイプ
 */
//-----------------------------------------------------------------------------
FIELD_CAMERA_AREA_CONT FIELD_CAMERA_GetCameraAreaCont( const FIELD_CAMERA * camera, u32 idx )
{
  GF_ASSERT( camera );
  GF_ASSERT( camera->camera_area_num > idx );
  return camera->camera_area[idx].area_cont;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラエリアの読み込み
 *
 *	@param	camera    カメラワーク
 *	@param	area_id   エリアID
 *	@param	heapID    ヒープID
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_LoadCameraArea( FIELD_CAMERA * camera, u32 area_id, HEAPID heapID )
{
  FIELD_CAMERA_AREA_SET* p_rect;
  GF_ASSERT( camera );

  p_rect = GFL_ARCHDL_UTIL_Load( camera->camera_area_handle, 
      area_id, 
      FALSE, GFL_HEAP_LOWID(heapID) );

  TOMOYA_Printf( "camera area rect x_min[%d] x_max[%d] z_min[%d] z_max[%d]\n", 
      p_rect->buff[0].rect.x_min, p_rect->buff[0].rect.x_max,
      p_rect->buff[0].rect.z_min, p_rect->buff[0].rect.z_max );
  
  FIELD_CAMERA_SetCameraArea( camera, p_rect );
  GFL_HEAP_FreeMemory( p_rect );
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ可動範囲処理後のターゲットポジション
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_GetCameraAreaAfterTargetPos( const FIELD_CAMERA * camera, VecFx32* p_pos )
{
  GF_ASSERT( camera );
  GF_ASSERT( p_pos );
  *p_pos = camera->target_write;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ可動範囲処理後のカメラポジション
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_GetCameraAreaAfterCameraPos( const FIELD_CAMERA * camera, VecFx32* p_pos )
{
  GF_ASSERT( camera );
  GF_ASSERT( p_pos );
  *p_pos = camera->campos_write;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラエリア　動作フラグを設定
 *
 *	@param	camera    カメラ
 *	@param	flag      フラグ
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_SetCameraAreaActive( FIELD_CAMERA * camera, BOOL flag )
{
  camera->camera_area_active = flag;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラエリア　動作フラグを取得
 *
 *	@param	camera    カメラ
 *
 *	@retval TRUE    動作
 *	@retval FALSE   停止
 */
//-----------------------------------------------------------------------------
BOOL FIELD_CAMERA_GetCameraAreaActive( const FIELD_CAMERA * camera )
{
  return camera->camera_area_active;
}


//----------------------------------------------------------------------------
/**
 *	@brief  カメラエリア反映
 *
 *	@param	cp_area       エリア情報
 *	@param	p_target      ターゲット座標
 */
//-----------------------------------------------------------------------------
static void cameraArea_UpdateTarget( const FIELD_CAMERA_AREA * cp_area, VecFx32* p_target )
{
  VecFx32* p_pos;
  int i;

  GF_ASSERT( cp_area->area_type < FIELD_CAMERA_AREA_MAX );
  GF_ASSERT( cp_area->area_cont < FIELD_CAMERA_AREA_CONT_MAX );

  if( cp_area->area_cont == FIELD_CAMERA_AREA_CONT_TARGET )
  {
    p_pos = p_target;

    if( pIsAreaFunc[cp_area->area_type] )
    {
      pIsAreaFunc[cp_area->area_type]( cp_area, p_pos, p_pos );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラエリア反映
 *
 *	@param	cp_area       エリア情報
 *	@param	p_camera      カメラ座標
 */
//-----------------------------------------------------------------------------
static void cameraArea_UpdateCamPos( const FIELD_CAMERA_AREA * cp_area, VecFx32* p_camera )
{
  VecFx32* p_pos;

  GF_ASSERT( cp_area->area_type < FIELD_CAMERA_AREA_MAX );
  GF_ASSERT( cp_area->area_cont < FIELD_CAMERA_AREA_CONT_MAX );

  if( cp_area->area_cont == FIELD_CAMERA_AREA_CONT_CAMERA )
  {
    p_pos = p_camera;

    if( pIsAreaFunc[cp_area->area_type] )
    {
      pIsAreaFunc[cp_area->area_type]( cp_area, p_pos, p_pos );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ可動範囲計算　矩形範囲外　ストップ
 *
 *	@param	cp_area   矩形情報
 *	@param  cp_pos	  現在座標
 *	@param	p_pos     計算後座標
 *
 *	@retval TRUE  カメラ動作範囲外だった
 *	@retval FALSE カメラ動作範囲内だった
 */
//-----------------------------------------------------------------------------
static BOOL cameraArea_IsAreaRect( const FIELD_CAMERA_AREA* cp_area, const VecFx32* cp_pos, VecFx32* p_pos )
{
  VecFx32 pos_data = *cp_pos;
  BOOL ret = FALSE;

  // Y座標設定
  p_pos->x = pos_data.x;
  p_pos->z = pos_data.z;
  p_pos->y = pos_data.y;
  
  // X方向範囲内？
  if( (FX32_CONST(cp_area->rect.x_min) > pos_data.x) )
  {
    ret = TRUE;
    p_pos->x = FX32_CONST(cp_area->rect.x_min);
  }
  else if( (cp_area->rect.x_max<<FX32_SHIFT < pos_data.x) )
  {
    ret = TRUE;
    p_pos->x = FX32_CONST(cp_area->rect.x_max);
  }


  // Z方向範囲内？
  if( (cp_area->rect.z_min<<FX32_SHIFT > pos_data.z) )
  {
    ret = TRUE;
    p_pos->z = FX32_CONST(cp_area->rect.z_min);
  }
  else if( (cp_area->rect.z_max<<FX32_SHIFT < pos_data.z) )
  {
    ret = TRUE;
    p_pos->z = FX32_CONST(cp_area->rect.z_max);
  }


  return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ可動範囲計算　円範囲外　ストップ
 *
 *	@param	cp_area   円情報
 *	@param	cp_pos    現在座標
 *	@param	p_pos     計算後座標
 *
 *	@retval TRUE  カメラ動作範囲外だった
 *	@retval FALSE カメラ動作範囲内だった
 */
//-----------------------------------------------------------------------------
static BOOL cameraArea_IsAreaCircle( const FIELD_CAMERA_AREA* cp_area, const VecFx32* cp_pos, VecFx32* p_pos )
{
  VecFx32 def_vec = { 0,0,FX32_ONE };
  VecFx32 pos_data = *cp_pos;
  VecFx32 pos_save = *cp_pos;
  VecFx32 center;
  VecFx32 way;
  fx32  len;
  u16 min_rot, max_rot, rot_now;
  VecFx32 normal; // 回転方向検出用
  BOOL ret = FALSE;

  p_pos->x = cp_pos->x;
  p_pos->y = cp_pos->y;
  p_pos->z = cp_pos->z;

  // 中心座標を求める
  center.x = cp_area->circle.center_x;
  center.y = 0;
  center.z = cp_area->circle.center_z;
  

  // ターゲットを中心とした距離に変更
  pos_data.y = 0; // 平面で見る
  VEC_Subtract( &pos_data, &center, &way ); 

  // 距離を求める
  len = VEC_Mag( &way );

  VEC_Normalize( &way, &way );

  // start endとの外積を求める
  VEC_CrossProduct( &def_vec, &way, &normal );
  rot_now     = FX_AcosIdx( VEC_DotProduct( &def_vec, &way ) );

  if( normal.y < 0 )
  {
    rot_now = 0x10000 - rot_now;
  }

  // 距離の範囲外チェック
  if( len > cp_area->circle.r )
  {
    len = cp_area->circle.r;
    ret = TRUE;
  }

  // 角度の範囲外チェック
  if( cp_area->circle.min_rot != cp_area->circle.max_rot )
  {
    if( cp_area->circle.min_rot > cp_area->circle.max_rot )
    {
      // 360度を通過する方が　可動範囲
      min_rot = cp_area->circle.max_rot;
      max_rot = cp_area->circle.min_rot;

      //  チェック
      if( (min_rot < rot_now) && (max_rot >= rot_now) )
      {
        // 近いほう
        if( MATH_ABS(min_rot - rot_now) > MATH_ABS(max_rot - rot_now) )
        {
          rot_now = max_rot;
          ret = TRUE;
        }
        else
        {
          rot_now = min_rot;
          ret = TRUE;
        }
      }
    }
    else
    {
      // 通常の　可動範囲
      min_rot = cp_area->circle.min_rot;
      max_rot = cp_area->circle.max_rot;

      //  チェック
      if( min_rot > rot_now )
      {
        rot_now = min_rot;
        ret = TRUE;
      }
      else if( max_rot < rot_now )
      {
        rot_now = max_rot;
        ret = TRUE;
      }
    }
  }


//  TOMOYA_Printf( "rot_now=%d  min_rot=%d max_rot=%d\n", rot_now, min_rot, max_rot );


  // 座標の再計算
  if( ret )
  {
    p_pos->x = FX_Mul( FX_SinIdx( rot_now ), len ) + center.x;
    p_pos->z = FX_Mul( FX_CosIdx( rot_now ), len ) + center.z;
    p_pos->y = pos_save.y;
  }

  return ret;
}


//----------------------------------------------------------------------------
/**
 *	@brief  カメラ可動範囲計算　矩形　範囲内　ストップ
 *
 *	@param	cp_area   矩形情報
 *	@param	cp_pos    現在座標
 *	@param	p_pos     計算後座標
 *
 *	@retval TRUE  カメラ動作範囲外だった
 *	@retval FALSE カメラ動作範囲内だった
 */
//-----------------------------------------------------------------------------
static BOOL cameraArea_IsAreaRectIn( const FIELD_CAMERA_AREA* cp_area, const VecFx32* cp_pos, VecFx32* p_pos )
{
  VecFx32 pos_data = *cp_pos;
  BOOL ret = FALSE;
  fx32 dist_min;
  u32 area_type;
  enum
  {
    AREA_X_MIN,
    AREA_X_MAX,
    AREA_Z_MIN,
    AREA_Z_MAX,

    AREA_NUM,
  };

  // Y座標設定
  p_pos->x = pos_data.x;
  p_pos->z = pos_data.z;
  p_pos->y = pos_data.y;
  
  // X方向範囲内？
  if( (FX32_CONST(cp_area->rect.x_min) < pos_data.x) && (FX32_CONST(cp_area->rect.x_max) > pos_data.x) )
  {
    if( (FX32_CONST(cp_area->rect.z_min) < pos_data.z) && (FX32_CONST(cp_area->rect.z_max) > pos_data.z) )
    {
      ret = TRUE;

      // XorZ一番近いところに戻す
      dist_min  = p_pos->x - FX32_CONST(cp_area->rect.x_min);
      area_type = AREA_X_MIN;

      if( dist_min > FX32_CONST(cp_area->rect.x_max) - p_pos->x )
      {
        dist_min  = FX32_CONST(cp_area->rect.x_max) - p_pos->x;
        area_type = AREA_X_MAX;
      }

      if( dist_min > p_pos->z - FX32_CONST(cp_area->rect.z_min) )
      {
        dist_min  = p_pos->z - FX32_CONST(cp_area->rect.z_min);
        area_type = AREA_Z_MIN;
      }

      if( dist_min > FX32_CONST(cp_area->rect.z_max) - p_pos->z )
      {
        dist_min  = FX32_CONST(cp_area->rect.z_max) - p_pos->z;
        area_type = AREA_Z_MAX;
      }


      // 一番近いところの座標を設定
      switch( area_type )
      {
      case AREA_X_MIN:
        p_pos->x = FX32_CONST(cp_area->rect.x_min);
        break;
      case AREA_X_MAX:
        p_pos->x = FX32_CONST(cp_area->rect.x_max);
        break;
      case AREA_Z_MIN:
        p_pos->z = FX32_CONST(cp_area->rect.z_min);
        break;
      case AREA_Z_MAX:
        p_pos->z = FX32_CONST(cp_area->rect.z_max);
        break;
      default:
        GF_ASSERT(0);
        break;
      }
    }
  }

  return ret;
}


/**
 *
 * 線形補間が型カメラ移動関連処理
 *
*/

//カメラ線形移動補間開始のための初期化処理
//・いくつまでプッシュできるようにするか

//カメラデータプッシュ処理
//・保存するもの
//　モード、ターゲット座標、アングル、カメラ座標
//・モード変更
//・移動
//・保存しているモードに戻す
//・復帰モード別に、保存しているデータをセット
//・終了
//
//カメラ移動処理
//・現在のカメラ座標、アングル、視野角をｓｒｃにセット
//・目的のカメラ座標、アングル、視野角をｄｓｔにセット
//・移動処理

//----------------------------------------------------------------------------
/**
 *	@brief  カメラパラメータセット
 *
 *	@param	inCamera      カメラポインタ
 *	@param	outParam      パラメータ格納バッファ
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void SetNowCameraParam(const FIELD_CAMERA * inCamera, FLD_CAM_MV_PARAM_CORE *outParam)
{
  outParam->CamPos = inCamera->camPos;
  outParam->TrgtPos = inCamera->target;
  outParam->AnglePitch = inCamera->angle_pitch;
  outParam->AngleYaw = inCamera->angle_yaw;
  outParam->Distance = inCamera->angle_len;
  outParam->Fovy = inCamera->fovy;
  outParam->Shift = inCamera->target_offset;
}

#if 0
static void RecvModeCamera(FIELD_CAMERA * camera)
{
  FLD_MOVE_CAM_DATA *data;
  FLD_CAM_PUSH_PARAM *push_param;
  data = &camera->MoveData;
  push_param = &data->PushParam;
  //ターゲット位置
  ;
  //アングル
  ;
  //距離
  ;
  //視野角
  ;
}
static void RecvModeTarget(FIELD_CAMERA * camera)
{
  GF_ASSERT(0);
  //カメラ位置
  ;
  //アングル
  ;
  //距離
  ;
  //視野角
  ;
}
static void RecvModeDirect(FIELD_CAMERA * camera)
{
  //ターゲット位置
  ;
  //カメラ位置
  ;
}
#endif


//----------------------------------------------------------------------------
/**
 *	@brief　復帰情報の保存
 *
 *	@param	camera      カメラポインタ
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_SetRecvCamParam(FIELD_CAMERA * camera )
{
  FLD_MOVE_CAM_DATA *data;
  FLD_CAM_PUSH_PARAM *push_param;
  if (camera == NULL){
    GF_ASSERT( 0 );
    return;
  }

  data = &camera->MoveData;
  push_param = &data->PushParam;

  if (data->PushFlg){
    GF_ASSERT_MSG(0,"Already Exist PushData");
    return;
  }

  //情報をプッシュ
  push_param->BeforeMode = camera->mode;
  //補間移動はカメラ位置不定型のみのサポート
  FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );
  
  SetNowCameraParam(camera, &push_param->RecvParam);
#if 0  
#ifdef DEBUG_ONLY_FOR_saitou
  OS_Printf("--DUMP_RECV_FIELD_CAMERA_PARAM--\n");
  DumpCameraParam(camera);
#endif
#endif
  //プッシュしたフラグ
  data->PushFlg = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  復帰情報のクリア
 *
 *	@param	camera      カメラポインタ
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_ClearRecvCamParam(FIELD_CAMERA * camera)
{
  FLD_MOVE_CAM_DATA *data;
  FLD_CAM_PUSH_PARAM *push_param;

  if (camera == NULL){
    GF_ASSERT( 0 );
    return;
  }

  data = &camera->MoveData;
  if (!data->PushFlg){
    OS_Printf("No PushData\n");
    return;
  }
  
  push_param = &data->PushParam;
  //モードを復帰
  FIELD_CAMERA_ChangeMode( camera, push_param->BeforeMode );

  //プッシュフラグを落す
  data->PushFlg = FALSE;

}

//----------------------------------------------------------------------------
/**
 *	@brief  線形移動パラメータをセットして動作開始
 *
 *	@param	camera      カメラポインタ
 *	@param	param       パラメータ格納バッファ
 *	@param  inFrame     フレーム
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_SetLinerParam(FIELD_CAMERA * camera, const FLD_CAM_MV_PARAM *param, const u16 inFrame)
{
  FLD_MOVE_CAM_DATA *data;
  if (camera == NULL){
    GF_ASSERT( 0 );
    return;
  }

  //補間移動はカメラ位置不定型のみのサポート
  FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );

  data = &camera->MoveData;
  SetNowCameraParam(camera, &data->SrcParam);
  data->DstParam = param->Core;
  data->NowFrm = 0;
  data->CostFrm = inFrame;
  data->Chk = param->Chk;

  //コールバックをセット
  camera->CallBack = LinerMoveFunc;
  //線形補間移動を開始
  data->Valid = TRUE;

}

//----------------------------------------------------------------------------
/**
 *	@brief  線形移動パラメータをセットして動作開始(ダイレクト型)
 *
 *	@param	camera      カメラポインタ
 *	@param	inCamPos    カメラ位置
 *	@param  inTrgtPos   ターゲット位置
 *	@param  inFrame     フレーム
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_SetLinerParamDirect(
    FIELD_CAMERA * camera,
    const VecFx32 *inCamPos, const VecFx32 *inTrgtPos,
    const FLD_CAM_MV_PARAM_CHK *inChk,
    const u16 inFrame)
{
  FLD_MOVE_CAM_DATA *data;
  
  if (camera == NULL){
    GF_ASSERT( 0 );
    return;
  }

  //補間移動はダイレクト型のみのサポート
  FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_DIRECT_POS );

  data = &camera->MoveData;
  SetNowCameraParam(camera, &data->SrcParam);
  data->DstParam.CamPos = *inCamPos;
  data->DstParam.TrgtPos = *inTrgtPos;
  data->NowFrm = 0;
  data->CostFrm = inFrame;
  {
    FLD_CAM_MV_PARAM_CHK chk;
    //ダイレクト方式なので、位置と視野角と注視点オフセット以外は変化させない
    chk.Pos = inChk->Pos;
    chk.Fovy = inChk->Fovy;
    chk.Shift = inChk->Shift;
    chk.Pitch = FALSE;
    chk.Yaw = FALSE;
    chk.Dist = FALSE;

    data->Chk = chk;
  }

  //コールバックをセット
  camera->CallBack = DirectLinerMoveFunc;
  //線形補間移動を開始
  data->Valid = TRUE;

}

//----------------------------------------------------------------------------
/**
 *	@brief  リカバリー関数
 *
 *	@param	camera      カメラポインタ
 *	@param  chk         復帰パラメータ
 *	@param  inFrame     フレーム
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_RecvLinerParam(
    FIELD_CAMERA * camera, const FLD_CAM_MV_PARAM_CHK *chk, const u16 inFrame)
{
  FLD_MOVE_CAM_DATA *data;
  if (camera == NULL){
    GF_ASSERT( 0 );
    return;
  }

  data = &camera->MoveData;
  if (!data->PushFlg){
    GF_ASSERT_MSG(0,"NO RECOVER PARAM\n");
    return;
  }

  //フレーム指定が0のときは不正とみなし、処理しない
  if (inFrame == 0){
    GF_ASSERT(0);
    return;
  }

  //補間移動はカメラ位置不定型のみのサポート
  FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );

  SetNowCameraParam(camera, &data->SrcParam);
  data->DstParam = data->PushParam.RecvParam;

  data->NowFrm = 0;
  data->CostFrm = inFrame;

  data->Chk = *chk;

  //コールバックをセット
  camera->CallBack = LinerMoveFunc;
  //線形補間移動を開始
  data->Valid = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  デフォルトゾーンカメラ設定で復帰するリカバリー関数
 *
 *	@param	camera      カメラポインタ
 *	@param  inFrame     フレーム
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_RecvLinerParamDefault(
    FIELD_CAMERA * camera, const u16 inFrame)
{
  FLD_MOVE_CAM_DATA *data;
  if (camera == NULL){
    GF_ASSERT( 0 );
    return;
  }

  data = &camera->MoveData;
  if (!data->PushFlg){
    GF_ASSERT_MSG(0,"NO RECOVER PARAM\n");
    return;
  }

  //フレーム指定が0のときは不正とみなし、処理しない
  if (inFrame == 0){
    GF_ASSERT(0);
    return;
  }

  //補間移動はカメラ位置不定型のみのサポート
  FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );

  SetNowCameraParam(camera, &data->SrcParam);
  //ゾーンデフォルトカメラパラメータを取得する
  {
    FLD_CAMERA_PARAM param;
    loadInitialParameter( camera, &param );
    data->DstParam.AnglePitch = param.Angle.x;
    data->DstParam.AngleYaw = param.Angle.y;
    data->DstParam.Distance = param.Distance * FX32_ONE;
    data->DstParam.Fovy = param.PerspWay;
    // プレイヤーとのバインドチェック
    if( param.PlayerLink )
    {
      // バインドする場合
      data->DstParam.Shift = param.Shift;
      data->DstParam.TrgtPos = *camera->default_target;
    }
    else
    {
      // バインドしない場合
      VEC_Set( &data->DstParam.Shift, 0,0,0 );
      data->DstParam.TrgtPos = param.Shift;
    }
  }
  data->NowFrm = 0;
  data->CostFrm = inFrame;

  data->Chk.Shift = TRUE;
  data->Chk.Pitch = TRUE;
  data->Chk.Yaw = TRUE;
  data->Chk.Dist = TRUE;
  data->Chk.Fovy = TRUE;
  data->Chk.Pos = TRUE;

  {
    OS_Printf("pitch = %x\n",data->PushParam.RecvParam.AnglePitch);
    OS_Printf("yaw = %x\n",data->PushParam.RecvParam.AngleYaw);
    OS_Printf("dis = %x\n",data->PushParam.RecvParam.Distance);
    OS_Printf("pers = %x\n",data->PushParam.RecvParam.Fovy);
    OS_Printf("pos = %x, %x, %x\n",
        data->PushParam.RecvParam.TrgtPos.x,
        data->PushParam.RecvParam.TrgtPos.y,
        data->PushParam.RecvParam.TrgtPos.z);
    OS_Printf("shift = %x, %x, %x\n",
        data->PushParam.RecvParam.Shift.x,
        data->PushParam.RecvParam.Shift.y,
        data->PushParam.RecvParam.Shift.z);

    OS_Printf("_pitch = %x\n",data->DstParam.AnglePitch);
    OS_Printf("_yaw = %x\n",data->DstParam.AngleYaw);
    OS_Printf("_dis = %x\n",data->DstParam.Distance);
    OS_Printf("_pers = %x\n",data->DstParam.Fovy);
    OS_Printf("_pos = %x, %x, %x\n",
        data->DstParam.TrgtPos.x,
        data->DstParam.TrgtPos.y,
        data->DstParam.TrgtPos.z);
    OS_Printf("_shift = %x, %x, %x\n",
        data->DstParam.Shift.x,
        data->DstParam.Shift.y,
        data->DstParam.Shift.z);
  }

  //コールバックをセット
  camera->CallBack = LinerMoveFunc;
  //線形補間移動を開始
  data->Valid = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  線形訪韓移動関数
 *
 *	@param	camera      カメラポインタ
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void LinerMoveFunc(FIELD_CAMERA * camera, void *work)
{
  FLD_MOVE_CAM_DATA *data;
  FLD_CAM_MV_PARAM_CORE *src;
  FLD_CAM_MV_PARAM_CORE *dst;
  data = &camera->MoveData;
  
  //リクエストがなければ処理はスルーする
  if (!data->Valid){
    return;
  }
  //フレーム消化
  data->NowFrm++;

  src = &data->SrcParam;
  dst = &data->DstParam;
  //アングルピッチ
  if (data->Chk.Pitch)
  {
///    OS_Printf("PITCH\n");
    camera->angle_pitch = SubFuncU16(&src->AnglePitch, &dst->AnglePitch, data->CostFrm, data->NowFrm);
  }
  //アングルヨー
  if (data->Chk.Yaw)
  {
///    OS_Printf("YAW\n");
    camera->angle_yaw = SubFuncU16(&src->AngleYaw, &dst->AngleYaw, data->CostFrm, data->NowFrm);
  }
  //座標
  if (data->Chk.Pos)
  {
///    OS_Printf("POS\n");
    VecSubFunc(&src->TrgtPos, &dst->TrgtPos, data->CostFrm, data->NowFrm, &camera->target);
  }
  //視野角
  if (data->Chk.Fovy)
  {
    camera->fovy = SubFuncU16(&src->Fovy, &dst->Fovy, data->CostFrm, data->NowFrm);
    GFL_G3D_CAMERA_SetfovySin( camera->g3Dcamera, FX_SinIdx( camera->fovy ) );
    GFL_G3D_CAMERA_SetfovyCos( camera->g3Dcamera, FX_CosIdx( camera->fovy ) );
  }
  //距離
  if (data->Chk.Dist)
  {
///    OS_Printf("DIST\n");
    camera->angle_len = SubFuncFx(&src->Distance, &dst->Distance, data->CostFrm, data->NowFrm);

  }
  //オフセット（シフト）
  if (data->Chk.Shift)
  {
///    OS_Printf("SHIFT\n");
    VecSubFunc(&src->Shift, &dst->Shift, data->CostFrm, data->NowFrm, &camera->target_offset);
  }
  //フレームが消費フレームに到達したか？
  if(data->NowFrm >= data->CostFrm){
    //移動実行中フラグオフ
    data->Valid = FALSE;
    //コールバッククリア
    FIELD_CAMERA_ClearMvFuncCallBack(camera);
#ifdef DEBUG_ONLY_FOR_saitou
    OS_Printf("--DUMP_END_FIELD_CAMERA_PARAM--\n");
    DumpCameraParam(camera);
#endif
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  線形訪韓移動関数
 *
 *	@param	camera      カメラポインタ
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void DirectLinerMoveFunc(FIELD_CAMERA * camera, void *work)
{
  FLD_MOVE_CAM_DATA *data;
  FLD_CAM_MV_PARAM_CORE *src;
  FLD_CAM_MV_PARAM_CORE *dst;
  data = &camera->MoveData;
  
  //リクエストがなければ処理はスルーする
  if (!data->Valid){
    return;
  }
  //フレーム消化
  data->NowFrm++;

  src = &data->SrcParam;
  dst = &data->DstParam;
  //座標
  if (data->Chk.Pos)
  {
///    OS_Printf("POS\n");
    VecSubFunc(&src->TrgtPos, &dst->TrgtPos, data->CostFrm, data->NowFrm, &camera->target);
    VecSubFunc(&src->CamPos, &dst->CamPos, data->CostFrm, data->NowFrm, &camera->camPos);
  }
  //視野角
  if (data->Chk.Fovy)
  {
    camera->fovy = SubFuncU16(&src->Fovy, &dst->Fovy, data->CostFrm, data->NowFrm);
    GFL_G3D_CAMERA_SetfovySin( camera->g3Dcamera, FX_SinIdx( camera->fovy ) );
    GFL_G3D_CAMERA_SetfovyCos( camera->g3Dcamera, FX_CosIdx( camera->fovy ) );
  }
  //オフセット（シフト）
  if (data->Chk.Shift)
  {
///    OS_Printf("SHIFT\n");
    VecSubFunc(&src->Shift, &dst->Shift, data->CostFrm, data->NowFrm, &camera->target_offset);
  }
  //フレームが消費フレームに到達したか？
  if(data->NowFrm >= data->CostFrm){
    //移動実行中フラグオフ
    data->Valid = FALSE;
    //コールバッククリア
    FIELD_CAMERA_ClearMvFuncCallBack(camera);
#ifdef DEBUG_ONLY_FOR_saitou
    OS_Printf("--DUMP_END_FIELD_CAMERA_PARAM--\n");
    DumpCameraParam(camera);
#endif
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ベクトル差分計算関数
 *
 *	@param	inSrc         開始ベクトル
 *	@param  inDst         終了ベクトル
 *	@param  inCostFrm     消費フレーム
 *	@param  inNowFrm      現在フレーム
 *	@param  outVec        格納バッファ
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void VecSubFunc(
    const VecFx32 *inSrc, const VecFx32 *inDst, const u16 inCostFrm, const u16 inNowFrm, VecFx32 *outVec)
{
  if (!inCostFrm){
    GF_ASSERT(0);
    *outVec = *inDst;
  }
  outVec->x = SubFuncFx(&inSrc->x, &inDst->x, inCostFrm, inNowFrm);
  outVec->y = SubFuncFx(&inSrc->y, &inDst->y, inCostFrm, inNowFrm);
  outVec->z = SubFuncFx(&inSrc->z, &inDst->z, inCostFrm, inNowFrm);
}

//----------------------------------------------------------------------------
/**
 *	@brief  固定小数型差分計算関数
 *
 *	@param	inSrc         開始値
 *	@param  inDst         終了値
 *	@param  inCostFrm     消費フレーム
 *	@param  inNowFrm      現在フレーム
 *
 *	@retval fx32          計算結果
 */
//-----------------------------------------------------------------------------
static fx32 SubFuncFx(const fx32 *inSrc, const fx32 *inDst, const u16 inCostFrm, const u16 inNowFrm)
{
  long src = (long)*inSrc;
  long dst = (long)*inDst;
  long dif;
  long tmp;
  long val;
  fx32 ans;

  if ( inCostFrm == inNowFrm ) ans = dst;
  else
  {
    dif = dst - src;
    tmp = (dif*inNowFrm);
    val = tmp / inCostFrm;
    ans = src+val;
  }
#if 0
  fx32 dif, val, ans;
  fx32 tmp;

  if (!inCostFrm){
    GF_ASSERT(0);
    return *inDst;
  }
  dif = (*inDst) - (*inSrc);
  tmp = (dif*inNowFrm);
  val = tmp / inCostFrm;
  ans = (*inSrc)+val;
#endif  
  return ans;
}

//----------------------------------------------------------------------------
/**
 *	@brief  符号なし整数差分計算関数
 *
 *	@param	inSrc         開始値
 *	@param  inDst         終了値
 *	@param  inCostFrm     消費フレーム
 *	@param  inNowFrm      現在フレーム
 *
 *	@retval u16           計算結果
 */
//-----------------------------------------------------------------------------
static u16 SubFuncU16(const u16 *inSrc, const u16 *inDst, const u16 inCostFrm, const u16 inNowFrm)
{
  u16 dif, val;
  u16 ans;
  u16 samp_a, samp_b;

  if (!inCostFrm){
    GF_ASSERT(0);
    return *inDst;
  }
  samp_a = (*inDst) - (*inSrc);
  samp_b = 0x10000-samp_a;

  if (samp_a<= samp_b){
    dif = samp_a;
    val = (dif*inNowFrm)/inCostFrm;
    ans = (*inSrc)+val;
  }else{
    dif = samp_b;
    val = (dif*inNowFrm)/inCostFrm;
    ans = (*inSrc)-val;
  }
  return ans;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ移動コールバック関数登録
 *
 *	@param	camera      カメラポインタ
 *	@param	param       パラメータ格納バッファ
 *	@param  inFrame     フレーム
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_SetMvFuncCallBack(FIELD_CAMERA * camera, CAMERA_CALL_BACK func, void *work)
{
  GF_ASSERT(camera->CallBack == NULL);
  camera->CallBack = func;
  camera->MoveData.CallBackWorkPtr = work;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ移動コールバック関数クリア
 *
 *	@param	camera      カメラポインタ
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_ClearMvFuncCallBack(FIELD_CAMERA * camera)
{
  camera->CallBack = NULL;
  camera->MoveData.CallBackWorkPtr = NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ移動コールバック中かをチェックする
 *
 *	@param	camera      カメラポインタ
 *
 *	@retval BOOL      TRUEでコールバック関数起動中
 */
//-----------------------------------------------------------------------------
BOOL FIELD_CAMERA_CheckMvFunc(FIELD_CAMERA * camera)
{
  FLD_MOVE_CAM_DATA *data;
  data = &camera->MoveData;
  if (camera->CallBack != NULL)
  {
    if (data->Valid){
      return TRUE;
    }
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ移動目的パラメータポインタを取得
 *
 *	@param	camera      カメラポインタ
 *
 *	@retval FLD_CAM_MV_PARAM_CORE パラメータポインタ
 */
//-----------------------------------------------------------------------------
FLD_CAM_MV_PARAM_CORE *FIELD_CAMERA_GetMoveDstPrmPtr(FIELD_CAMERA * camera)
{
  return &camera->MoveData.DstParam;
}

#ifdef PM_DEBUG
//----------------------------------------------------------------------------
/**
 *	@brief  カメラ情報のダンプ
 *
 *	@param  camera        カメラポインタ
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void DumpCameraParam(FIELD_CAMERA * camera)
{
  u16 pitch, yaw;
  fx32 len;
  u16 fovy;
  VecFx32 shift, target;
  pitch = FIELD_CAMERA_GetAnglePitch(camera);
  yaw = FIELD_CAMERA_GetAngleYaw(camera);
  len = FIELD_CAMERA_GetAngleLen(camera);
  fovy = FIELD_CAMERA_GetFovy(camera);
  FIELD_CAMERA_GetTargetOffset( camera, &shift );
  FIELD_CAMERA_GetTargetPos( camera, &target );
      
  OS_Printf("%d,%d,%d,%d,%d,%d\n", pitch, yaw, len, target.x, target.y, target.z );
  OS_Printf("%d,%d,%d,%d\n", fovy, shift.x, shift.y, shift.z );
}
#endif  //PM_DEBUG






