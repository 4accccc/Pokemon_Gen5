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
  int resIdx;
  int objIdx;
};

typedef struct
{
  const u32 datId;
  const u32 texSize;
}MB_CAP_OBJ_DATA;

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

static const MB_CAP_OBJ_DATA MB_CAP_ObjTypeArr[MCOT_MAX] =
{
  { NARC_mb_capture_gra_cap_obj_kusamura1_nsbtx , GFL_BBD_TEXSIZDEF_128x32 },//  MCOT_GRASS,
  { NARC_mb_capture_gra_cap_obj_kusamura2_nsbtx , GFL_BBD_TEXSIZDEF_128x32 },//  MCOT_GRASS_SIZE,
  { NARC_mb_capture_gra_cap_obj_wood_nsbtx , GFL_BBD_TEXSIZDEF_128x32 },//  MCOT_WATER,
  { NARC_mb_capture_gra_cap_obj_water_nsbtx , GFL_BBD_TEXSIZDEF_32x32 },//  MCOT_WOOD,
};

//--------------------------------------------------------------
//	オブジェクト作成
//--------------------------------------------------------------
MB_CAP_OBJ *MB_CAP_OBJ_CreateObject( MB_CAPTURE_WORK *capWork , MB_CAP_OBJ_INIT_WORK *initWork )
{
  const HEAPID heapId = MB_CAPTURE_GetHeapId( capWork );
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  ARCHANDLE *arcHandle = MB_CAPTURE_GetArcHandle( capWork );
  MB_CAP_OBJ *objWork = GFL_HEAP_AllocClearMemory( heapId , sizeof( MB_CAP_OBJ ) );
  GFL_G3D_RES* res = GFL_G3D_CreateResourceHandle( arcHandle , MB_CAP_ObjTypeArr[initWork->type].datId );
  const BOOL flg = TRUE;
  
  GFL_G3D_TransVramTexture( res );
  objWork->resIdx = GFL_BBD_AddResource( bbdSys , 
                                       res , 
                                       GFL_BBD_TEXFMT_PAL16 ,
                                       MB_CAP_ObjTypeArr[initWork->type].texSize ,
                                       32 , 32 );
  GFL_BBD_CutResourceData( bbdSys , objWork->resIdx );
  
  objWork->objIdx = GFL_BBD_AddObject( bbdSys , 
                                     objWork->resIdx ,
                                     FX32_ONE , 
                                     FX32_ONE , 
                                     &initWork->pos ,
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
  GFL_BBD_RemoveResource( bbdSys , objWork->resIdx );
  
  GFL_HEAP_FreeMemory( objWork );
}



