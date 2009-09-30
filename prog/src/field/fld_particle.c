//======================================================================
/**
 * @file	fld_particle.c
 * @brief	フィールドパーティクル
 * @author	Saito
 *
 */
//======================================================================

#include <gflib.h>	//for particle
#include "fld_particle.h"

#include "arc_def.h"


///戦闘パーティクルのカメラニア設定
#define BP_NEAR			(FX32_ONE)
///戦闘パーティクルのカメラファー設定
#define BP_FAR			(FX32_ONE * 900)


typedef struct FLD_PRTCL_SYS_tag
{
  GFL_G3D_CAMERA *CameraPtr;
  HEAPID HeapID;
  void *PrtclBuff;
	GFL_PTC_PTR PrtclPtr;
}FLD_PRTCL_SYS;


static void ParticleCallBack(GFL_EMIT_PTR emit);

//--------------------------------------------------------------------------------------------
/**
 * 初期化
 *
 * @param   inHeapID			    ヒープＩＤ
 *
 * @return	FLD_PRTCL_SYS_PTR   モジュールポインタ
 */
//--------------------------------------------------------------------------------------------
FLD_PRTCL_SYS_PTR FLD_PRTCL_Init(const HEAPID inHeapID)
{
  FLD_PRTCL_SYS_PTR sys;

  sys = GFL_HEAP_AllocClearMemory(inHeapID, sizeof(FLD_PRTCL_SYS));
  sys->HeapID = inHeapID;
	
  GFL_PTC_Init(inHeapID);

	//パーティクルメモリ確保
	sys->PrtclBuff = GFL_HEAP_AllocClearMemory(inHeapID, PARTICLE_LIB_HEAP_SIZE);
#if 0
	sys->PrtclPtr = GFL_PTC_Create(
											  sys->PrtclBuff, 
											  PARTICLE_LIB_HEAP_SIZE, TRUE, inHeapID);
  camera_ptr = GFL_PTC_GetCameraPtr(sys->PrtclPtr);

  {
    fx32 near = BP_NEAR;
    fx32 far = BP_FAR;
    GFL_G3D_CAMERA_SetNear( camera_ptr, &near );
    GFL_G3D_CAMERA_SetFar( camera_ptr, &far );
  }

  //@todo
  {
    void *resource;
    resource = FLD_PRTCL_LoadResource(sys,
			ARCID_FLD_SPA, /*NARC_particledata_title_part_spa*/0);

    FLD_PRTCL_SetResource(sys, resource, TRUE, NULL);
  }
#endif
  return sys;
}

//--------------------------------------------------------------------------------------------
/**
 * パーティクルポインタ生成
 *
 * @param   FLD_PRTCL_SYS_PTR sys
 *
 * @return	GFL_PTC_PTR   パーティクルポインタ
 *
 * @note    戻り値として取得したポインタは外で解放しないように。
 */
//--------------------------------------------------------------------------------------------
GFL_PTC_PTR FLD_PRTCL_Create(FLD_PRTCL_SYS_PTR sys)
{
  GFL_G3D_CAMERA *camera_ptr;

  sys->PrtclPtr = GFL_PTC_Create(
											  sys->PrtclBuff, 
											  PARTICLE_LIB_HEAP_SIZE, TRUE, sys->HeapID);
  camera_ptr = GFL_PTC_GetCameraPtr(sys->PrtclPtr);

  {
    fx32 near = BP_NEAR;
    fx32 far = BP_FAR;
    GFL_G3D_CAMERA_SetNear( camera_ptr, &near );
    GFL_G3D_CAMERA_SetFar( camera_ptr, &far );
  }

  return sys->PrtclPtr;
}

//--------------------------------------------------------------------------------------------
/**
 * パティクルポインタ破棄
 *
 * @param   FLD_PRTCL_SYS_PTR sys
 *
 * @return	FLD_PRTCL_SYS_PTR   モジュールポインタ
 */
//--------------------------------------------------------------------------------------------
void FLD_PRTCL_Delete(FLD_PRTCL_SYS_PTR sys)
{
  if (sys->PrtclPtr!=NULL){
    GFL_PTC_Delete(sys->PrtclPtr);
    sys->PrtclPtr = NULL;
  }
}

//--------------------------------------------------------------------------------------------
/**
 * リソースのロード
 *
 * @param   
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void *FLD_PRTCL_LoadResource(FLD_PRTCL_SYS_PTR sys, int inArcID, int inArcIdx)
{
  void *resource;
  resource = GFL_PTC_LoadArcResource(inArcID, inArcIdx, sys->HeapID);
  return resource;
}

//--------------------------------------------------------------------------------------------
/**
 * リソースのセット
 *
 * @param   
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void FLD_PRTCL_SetResource(FLD_PRTCL_SYS_PTR sys, void *resource, int inTexAtOnce, GFL_TCBSYS *tcbsys)
{
  GFL_PTC_SetResource(sys->PrtclPtr, resource, inTexAtOnce, tcbsys);
}

//--------------------------------------------------------------------------------------------
/**
 * エミッターの終了チェック
 *
 * @param   
 *
 * @return	BOOL      TRUE:終了   FALSE:継続
 */
//--------------------------------------------------------------------------------------------
BOOL FLD_PRTCL_CheckEmitEnd(FLD_PRTCL_SYS_PTR sys)
{
  if (sys->PrtclPtr == NULL){
    return TRUE;
  }

  if ( GFL_PTC_GetEmitterNum(sys->PrtclPtr) == 0 ){
    return TRUE;
  }

  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * 解放
 *
 * @param   sys     モジュールポインタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void FLD_PRTCL_End(FLD_PRTCL_SYS_PTR sys)
{
  //パーティクル解放
  FLD_PRTCL_Delete(sys);
	GFL_PTC_Exit();
  GFL_HEAP_FreeMemory( sys->PrtclBuff );
	GFL_HEAP_FreeMemory( sys );
}

//--------------------------------------------------------------------------------------------
/**
 * メイン関数
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void FLD_PRTCL_Main(void)
{
///  GF_G3X_Reset();
  GFL_PTC_Main();
}
