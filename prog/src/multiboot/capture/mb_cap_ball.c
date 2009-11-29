//======================================================================
/**
 * @file	  mb_cap_obj.c
 * @brief	  捕獲ゲーム・障害物
 * @author	ariizumi
 * @data	  09/11/24
 *
 * モジュール名：MB_CAP_BALL
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "mb_capture_gra.naix"

#include "./mb_cap_ball.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define
//X192:Y120の距離は226 //66フレームで飛んで行くには3.5くらい
#define MB_CAP_BALL_SHOT_SPEED (FX32_CONST(3.5f))

#define MB_CAP_BALL_ANIM_SHOT_SPEED (6)
#define MB_CAP_BALL_ANIM_SHOT_FRAME (6)

#define MB_CAP_BALL_HEIGHT_MAX (32)
#define MB_CAP_BALL_HEIGHT_SCALE_MAX FX32_CONST(1.5)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
struct _MB_CAP_BALL
{
  BOOL isFinish;
  
  u16 frame;
  u16 shotFrame;  //飛んでる時間

  u16 anmFrame;
  u16 anmIdx;
  
  int objIdx;
  int objShadowIdx;
  
  VecFx32 targetPos;
  VecFx32 pos;
  fx32    height;
  
  VecFx32 spd;
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

//--------------------------------------------------------------
//	オブジェクト作成
//--------------------------------------------------------------
MB_CAP_BALL* MB_CAP_BALL_CreateObject( MB_CAPTURE_WORK *capWork , MB_CAP_BALL_INIT_WORK *initWork )
{
  const HEAPID heapId = MB_CAPTURE_GetHeapId( capWork );
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  ARCHANDLE *arcHandle = MB_CAPTURE_GetArcHandle( capWork );
  MB_CAP_BALL *ballWork = GFL_HEAP_AllocClearMemory( heapId , sizeof( MB_CAP_BALL ) );
  const int resIdx = MB_CAPTURE_GetBbdResIdx( capWork , MCBR_BALL );
  const int resShadowIdx = MB_CAPTURE_GetBbdResIdx( capWork , MCBR_SHADOW );
  const BOOL flg = TRUE;
  ballWork->pos.x = FX32_CONST(128);
  ballWork->pos.y = FX32_CONST(MB_CAP_UPPER_BALL_POS_BASE_Y);
  ballWork->pos.z = FX32_CONST(MB_CAP_UPPER_BALL_BASE_Z);
  
  ballWork->targetPos = initWork->targetPos;
  
  ballWork->objIdx = GFL_BBD_AddObject( bbdSys , 
                                     resIdx ,
                                     FX32_ONE , 
                                     FX32_ONE , 
                                     &ballWork->pos ,
                                     31 ,
                                     GFL_BBD_LIGHT_NONE );
  ballWork->objShadowIdx = GFL_BBD_AddObject( bbdSys , 
                                     resShadowIdx ,
                                     FX32_ONE , 
                                     FX32_ONE , 
                                     &ballWork->pos ,
                                     31 ,
                                     GFL_BBD_LIGHT_NONE );
  GFL_BBD_SetObjectDrawEnable( bbdSys , ballWork->objIdx , &flg );
  GFL_BBD_SetObjectDrawEnable( bbdSys , ballWork->objShadowIdx , &flg );
  //3D設定の関係で反転させる・・・
  GFL_BBD_SetObjectFlipT( bbdSys , ballWork->objIdx , &flg );

  //移動速度の計算
  {
    fx32 subX = ballWork->targetPos.x - ballWork->pos.x;
    fx32 subY = ballWork->targetPos.y - ballWork->pos.y;
    fx32 len = FX_Mul(subX,subX) + FX_Mul(subY,subY);
    fx32 len_sqrt = FX_Sqrt( len );
    
    ballWork->spd.x = FX_Div( FX_Mul(subX,MB_CAP_BALL_SHOT_SPEED) , len_sqrt );
    ballWork->spd.y = FX_Div( FX_Mul(subY,MB_CAP_BALL_SHOT_SPEED) , len_sqrt );
    ballWork->spd.z = 0;
    
    ballWork->shotFrame = F32_CONST( FX_Div( len_sqrt , MB_CAP_BALL_SHOT_SPEED ) );
  }
  
  ballWork->frame = 0;
  ballWork->anmFrame = 0;
  ballWork->anmIdx = 0;
  ballWork->isFinish = FALSE;
  
  return ballWork;
}

//--------------------------------------------------------------
//	オブジェクト開放
//--------------------------------------------------------------
void MB_CAP_BALL_DeleteObject( MB_CAPTURE_WORK *capWork , MB_CAP_BALL *ballWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );

  GFL_BBD_RemoveObject( bbdSys , ballWork->objIdx );
  GFL_BBD_RemoveObject( bbdSys , ballWork->objShadowIdx );
  
  GFL_HEAP_FreeMemory( ballWork );
}

//--------------------------------------------------------------
//	オブジェクト更新
//--------------------------------------------------------------
void MB_CAP_BALL_UpdateObject( MB_CAPTURE_WORK *capWork , MB_CAP_BALL *ballWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  VecFx32 pos;

  VEC_Add( &ballWork->pos , &ballWork->spd , &ballWork->pos );
  
  GFL_BBD_SetObjectTrans( bbdSys , ballWork->objShadowIdx , &ballWork->pos );

  //高さ計算
  {
    u16 angle = 0x8000 * ballWork->frame / ballWork->shotFrame;
    fx16 sin = FX_SinIdx( angle );
    fx16 size = FX_Mul( MB_CAP_BALL_HEIGHT_SCALE_MAX , sin ) + FX16_ONE;

    ballWork->height = sin*MB_CAP_BALL_HEIGHT_MAX;
    GFL_BBD_SetObjectSiz( bbdSys , ballWork->objIdx , &size , &size );
  }

  pos.x = ballWork->pos.x;
  pos.y = ballWork->pos.y - ballWork->height;
  pos.z = ballWork->pos.z + FX32_ONE;
  GFL_BBD_SetObjectTrans( bbdSys , ballWork->objIdx , &pos );

  
  //アニメの更新
  ballWork->anmFrame++;
  if( ballWork->anmFrame >= MB_CAP_BALL_ANIM_SHOT_SPEED )
  {
    ballWork->anmFrame = 0;
    ballWork->anmIdx++;
    if( ballWork->anmIdx >= MB_CAP_BALL_ANIM_SHOT_FRAME )
    {
      ballWork->anmIdx = 0;
    }
    GFL_BBD_SetObjectCelIdx( bbdSys , ballWork->objIdx , &ballWork->anmIdx );
  }
  
  //終了判定
  ballWork->frame++;
  if( ballWork->frame > ballWork->shotFrame )
  {
    ballWork->isFinish = TRUE;
  }
}


#pragma mark [>outer func
//--------------------------------------------------------------
//  外部提供関数群
//--------------------------------------------------------------
const BOOL MB_CAP_BALL_IsFinish( const MB_CAP_BALL *ballWork )
{
  return ballWork->isFinish;
}
