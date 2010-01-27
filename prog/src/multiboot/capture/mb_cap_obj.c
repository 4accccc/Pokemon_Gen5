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
#define MB_CAP_OBJ_ANM_SPD (4)
#define MB_CAP_OBJ_ANM_FRAME (4)

#define MB_CAP_OBJ_STAR_SPD (FX32_CONST(0.33))
#define MB_CAP_OBJ_STAR_ROT (0x400)
#define MB_CAP_OBJ_STAR_HEIGHT (16)
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
  BOOL isPlayAnime;
  BOOL isEnable;
  u16 anmCnt;
  int objIdx;
  
  VecFx32 pos;
  
  MB_CAP_OBJ_TYPE objType;
  
  //星用
  s8  moveDir;
  u16 rad[3];
  fx32 height;
  fx32 baseY;
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

//--------------------------------------------------------------
//	オブジェクト作成
//--------------------------------------------------------------
MB_CAP_OBJ* MB_CAP_OBJ_CreateObject( MB_CAPTURE_WORK *capWork , MB_CAP_OBJ_INIT_WORK *initWork )
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
  
  objWork->isPlayAnime = FALSE;
  objWork->anmCnt = 0;
  objWork->objType = initWork->type;
  objWork->isEnable = TRUE;
  objWork->height = 0;
  
  if( objWork->objType == MCOT_STAR )
  {
    //-1 or +1
    objWork->moveDir = GFUser_GetPublicRand0(2)*2-1;
    if( objWork->moveDir == -1 )
    {
      objWork->pos.x = FX32_CONST(256+32);
    }
    else
    {
      objWork->pos.x = FX32_CONST(-32);
    }
    objWork->pos.y = FX32_CONST( 64+GFUser_GetPublicRand0(64) );
    objWork->pos.z = FX32_CONST( MB_CAP_OBJ_BASE_Z ) + objWork->pos.y;
    objWork->rad[0] = GFUser_GetPublicRand0(0x10000);
    objWork->rad[1] = GFUser_GetPublicRand0(0x10000);
    objWork->rad[2] = GFUser_GetPublicRand0(0x10000);
    objWork->baseY = FX32_CONST( GFUser_GetPublicRand0(40)-20 );
  }
  
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
//	オブジェクト更新
//--------------------------------------------------------------
void MB_CAP_OBJ_UpdateObject( MB_CAPTURE_WORK *capWork , MB_CAP_OBJ *objWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  if( objWork->isPlayAnime == TRUE )
  {
    u16 frame = objWork->anmCnt/MB_CAP_OBJ_ANM_SPD;
    objWork->anmCnt++;
    if( objWork->anmCnt >= MB_CAP_OBJ_ANM_SPD*MB_CAP_OBJ_ANM_FRAME )
    {
      frame = 0;
      objWork->isPlayAnime = FALSE;
    }
    GFL_BBD_SetObjectCelIdx( bbdSys , objWork->objIdx , &frame );
  }
}

//--------------------------------------------------------------
//	オブジェクト更新(星用
//--------------------------------------------------------------
void MB_CAP_OBJ_UpdateObject_Star( MB_CAPTURE_WORK *capWork , MB_CAP_OBJ *objWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  VecFx32 pos;
  u8 i;
  fx32 posY = 0;
  /*
  //ジャンプ移動(高さ未反映
  objWork->rad += MB_CAP_OBJ_STAR_ROT;
  if( objWork->rad >= 0x8000 )
  {
    objWork->rad -= 0x8000;
  }

  objWork->pos.x += MB_CAP_OBJ_STAR_SPD*objWork->moveDir;
  
  pos = objWork->pos;

  //高さ計算
  pos.y -= FX_SinIdx( objWork->rad ) * MB_CAP_OBJ_STAR_HEIGHT;
  */

  static const u16 addRad[3] = {2*0x10000/360,1*0x10000/360,1*0x10000/360};
  static const u16 lenRate[3] = {12,24,0};
  objWork->height = FX32_CONST(8.0f);
  objWork->pos.x += MB_CAP_OBJ_STAR_SPD*objWork->moveDir;
  pos = objWork->pos;
  
  for( i=0;i<2;i++ )
  {
    if( objWork->rad[i] + addRad[i] < 0x10000 )
    {
      objWork->rad[i] += addRad[i];
    }
    else
    {
      objWork->rad[i] = objWork->rad[i] + addRad[i] - 0x10000;
    }
    posY += FX_SinIdx(objWork->rad[i])*lenRate[i];
  }
  objWork->pos.y = FX32_CONST(96)+(posY)+objWork->baseY;
  pos.y = objWork->pos.y+objWork->height;
  
  
  GFL_BBD_SetObjectTrans( bbdSys , objWork->objIdx , &pos );
}


//--------------------------------------------------------------
//  当たり判定作成
//--------------------------------------------------------------
void MB_CAP_OBJ_GetHitWork( MB_CAP_OBJ *objWork , MB_CAP_HIT_WORK *hitWork )
{
  hitWork->pos = &objWork->pos;
  hitWork->height = objWork->height;
  hitWork->size.x = FX32_CONST(MB_CAP_OBJ_HITSIZE_XY);
  hitWork->size.y = FX32_CONST(MB_CAP_OBJ_HITSIZE_XY);
  hitWork->size.z = FX32_CONST(MB_CAP_OBJ_HITSIZE_Z);
}

//--------------------------------------------------------------
//  アニメーションをリクエスト
//--------------------------------------------------------------
void MB_CAP_OBJ_StartAnime( MB_CAP_OBJ *objWork )
{
  objWork->isPlayAnime = TRUE;
  objWork->anmCnt = 0;
}

//--------------------------------------------------------------
//  有効・無効設定
//--------------------------------------------------------------
void MB_CAP_OBJ_SetEnable( MB_CAPTURE_WORK *capWork , MB_CAP_OBJ *objWork , const BOOL flg )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );

  objWork->isEnable = flg;
  GFL_BBD_SetObjectDrawEnable( bbdSys , objWork->objIdx , &flg );
}
const BOOL MB_CAP_OBJ_GetEnable( MB_CAPTURE_WORK *capWork , MB_CAP_OBJ *objWork )
{
  return objWork->isEnable;
}
