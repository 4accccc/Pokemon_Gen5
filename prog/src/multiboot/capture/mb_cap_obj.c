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
  GFL_BBD_SYS *bbdSys;
  
  int resIdx;
  int objIdx;
};


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

//--------------------------------------------------------------
//	オブジェクト作成
//--------------------------------------------------------------
MB_CAP_OBJ *MB_CAP_OBJ_CreateObject( MB_CAP_OBJ_INIT_WORK *initWork )
{
  MB_CAP_OBJ *mbObj = GFL_HEAP_AllocClearMemory( initWork->heapId , sizeof( MB_CAP_OBJ ) );
  GFL_G3D_RES* res = GFL_G3D_CreateResourceHandle( initWork->arcHandle , NARC_mb_capture_gra_cap_obj_kusamura1_nsbtx );
  const BOOL flg = TRUE;
  mbObj->bbdSys = initWork->bbdSys;
  
  GFL_G3D_TransVramTexture( res );
  mbObj->resIdx = GFL_BBD_AddResource( mbObj->bbdSys , 
                                       res , 
                                       GFL_BBD_TEXFMT_PAL16 ,
                                       GFL_BBD_TEXSIZDEF_32x32 ,
                                       32 , 32 );
//  GFL_BBD_CutResourceData( mbObj->bbdSys , mbObj->resIdx );
  
  mbObj->objIdx = GFL_BBD_AddObject( mbObj->bbdSys , 
                                     mbObj->resIdx ,
                                     FX32_ONE , 
                                     FX32_ONE , 
                                     &initWork->pos ,
                                     31 ,
                                     GFL_BBD_LIGHT_NONE );
  GFL_BBD_SetObjectDrawEnable( mbObj->bbdSys , mbObj->objIdx , &flg );
//  GFL_G3D_DeleteResource( res );
  return mbObj;
}

//--------------------------------------------------------------
//	オブジェクト開放
//--------------------------------------------------------------
void MB_CAP_OBJ_DeleteObject( MB_CAP_OBJ *mbObj )
{
  GFL_BBD_RemoveObject( mbObj->bbdSys , mbObj->objIdx );
  GFL_BBD_RemoveResource( mbObj->bbdSys , mbObj->resIdx );
  
  GFL_HEAP_FreeMemory( mbObj );
}



