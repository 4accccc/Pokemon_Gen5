//======================================================================
/**
 * @file	  mb_cap_obj.c
 * @brief	  捕獲ゲーム・障害物
 * @author	ariizumi
 * @data	  09/11/24
 *
 * モジュール名：MB_CAP_OBJ
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "mb_capture_gra.naix"

#include "./mb_cap_obj.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
struct _MB_CAP_OBJ
{
  int objIdx;
  
  VecFx32 pos;
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

//--------------------------------------------------------------
//	オブジェクト作成
//--------------------------------------------------------------
MB_CAP_OBJ *MB_CAP_OBJ_CreateObject( MB_CAPTURE_WORK *capWork , MB_CAP_OBJ_INIT_WORK *initWork )
{
  const HEAPID heapId = MB_CAPTURE_GetHeapId( capWork );
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  ARCHANDLE *arcHandle = MB_CAPTURE_GetArcHandle( capWork );
  MB_CAP_OBJ *objWork = GFL_HEAP_AllocClearMemory( heapId , sizeof( MB_CAP_OBJ ) );
  const int resIdx = MB_CAPTURE_GetBbdResIdx( capWork , MCBR_OBJ_GRASS + initWork->type );
  const BOOL flg = TRUE;
  
  objWork->pos = initWork->pos;
  objWork->objIdx = GFL_BBD_AddObject( bbdSys , 
                                     resIdx ,
                                     FX32_ONE , 
                                     FX32_ONE , 
                                     &objWork->pos ,
                                     31 ,
                                     GFL_BBD_LIGHT_NONE );
  GFL_BBD_SetObjectDrawEnable( bbdSys , objWork->objIdx , &flg );
  //3D設定の関係で反転させる・・・
  GFL_BBD_SetObjectFlipT( bbdSys , objWork->objIdx , &flg );
  return objWork;
}

//--------------------------------------------------------------
//	オブジェクト開放
//--------------------------------------------------------------
void MB_CAP_OBJ_DeleteObject( MB_CAPTURE_WORK *capWork , MB_CAP_OBJ *objWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );

  GFL_BBD_RemoveObject( bbdSys , objWork->objIdx );
  
  GFL_HEAP_FreeMemory( objWork );
}

//--------------------------------------------------------------
//  当たり判定作成
//--------------------------------------------------------------
void MB_CAP_OBJ_GetHitWork( MB_CAP_OBJ *objWork , MB_CAP_HIT_WORK *hitWork )
{
  hitWork->pos = &objWork->pos;
  hitWork->height = 0;
  hitWork->size.x = FX32_CONST(MB_CAP_OBJ_HITSIZE_XY);
  hitWork->size.y = FX32_CONST(MB_CAP_OBJ_HITSIZE_XY);
  hitWork->size.z = FX32_CONST(MB_CAP_OBJ_HITSIZE_Z);
}


