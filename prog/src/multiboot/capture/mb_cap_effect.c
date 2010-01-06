//======================================================================
/**
 * @file	  mb_cap_effect.c
 * @brief	  捕獲ゲーム・エフェクト
 * @author	ariizumi
 * @data	  09/11/30
 *
 * モジュール名：MB_CAP_EFFECT
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "mb_capture_gra.naix"

#include "./mb_cap_effect.h"

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
struct _MB_CAP_EFFECT
{
  u16 frame;
  u16 anmIdx;
  int objIdx;
  BOOL isFinish;
  BOOL isLoop;
  
  VecFx32 pos;
  
  MB_CAP_EFFECT_TYPE type;
};


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

static const u8 MB_CAP_EFFECT_ANIME_FRAME[MCET_MAX] = { 2,MB_CAP_EFFECT_SMOKE_FRAME,MB_CAP_EFFECT_TRANS_FRAME,4,MB_CAP_EFFECT_SMOKE_FRAME,MB_CAP_EFFECT_ZZZ_FRAME };
static const u8 MB_CAP_EFFECT_ANIME_SPEED[MCET_MAX] = { 5,MB_CAP_EFFECT_SMOKE_SPEED,MB_CAP_EFFECT_TRANS_SPEED,6,MB_CAP_EFFECT_SMOKE_SPEED,MB_CAP_EFFECT_ZZZ_SPEED };

//--------------------------------------------------------------
//	エフェクト作成
//--------------------------------------------------------------
MB_CAP_EFFECT* MB_CAP_EFFECT_CreateObject( MB_CAPTURE_WORK *capWork , MB_CAP_EFFECT_INIT_WORK *initWork )
{
  const HEAPID heapId = MB_CAPTURE_GetHeapId( capWork );
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  ARCHANDLE *arcHandle = MB_CAPTURE_GetArcHandle( capWork );
  MB_CAP_EFFECT *effWork = GFL_HEAP_AllocClearMemory( heapId , sizeof( MB_CAP_EFFECT ) );
  const int resIdx = MB_CAPTURE_GetBbdResIdx( capWork , MCBR_EFF_HIT + initWork->type );
  const BOOL flg = TRUE;
  
  effWork->pos = initWork->pos;
  effWork->objIdx = GFL_BBD_AddObject( bbdSys , 
                                     resIdx ,
                                     FX32_ONE , 
                                     FX32_ONE , 
                                     &effWork->pos ,
                                     31 ,
                                     GFL_BBD_LIGHT_NONE );
  GFL_BBD_SetObjectDrawEnable( bbdSys , effWork->objIdx , &flg );
  //3D設定の関係で反転させる・・・
  GFL_BBD_SetObjectFlipT( bbdSys , effWork->objIdx , &flg );

  effWork->frame = 0;
  effWork->anmIdx = 0;
  effWork->type = initWork->type;
  effWork->isFinish = FALSE;
  effWork->isLoop = FALSE;

  return effWork;
}

//--------------------------------------------------------------
//	エフェクト開放
//--------------------------------------------------------------
void MB_CAP_EFFECT_DeleteObject( MB_CAPTURE_WORK *capWork , MB_CAP_EFFECT *effWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );

  GFL_BBD_RemoveObject( bbdSys , effWork->objIdx );
  
  GFL_HEAP_FreeMemory( effWork );
}

//--------------------------------------------------------------
//	エフェクト更新
//--------------------------------------------------------------
void MB_CAP_EFFECT_UpdateObject( MB_CAPTURE_WORK *capWork , MB_CAP_EFFECT *effWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );

  effWork->frame++;
  if( effWork->frame > MB_CAP_EFFECT_ANIME_SPEED[effWork->type] )
  {
    effWork->frame = 0;
    effWork->anmIdx++;
    if( effWork->anmIdx >= MB_CAP_EFFECT_ANIME_FRAME[effWork->type] )
    {
      if( effWork->isLoop == FALSE )
      {
        const BOOL isDisp = FALSE;
        effWork->isFinish = TRUE;
        GFL_BBD_SetObjectDrawEnable( bbdSys , effWork->objIdx , &isDisp );
      }
      else
      {
        effWork->anmIdx = 0;
        GFL_BBD_SetObjectCelIdx( bbdSys , effWork->objIdx , &effWork->anmIdx );
      }
    }
    else
    {
      GFL_BBD_SetObjectCelIdx( bbdSys , effWork->objIdx , &effWork->anmIdx );
    }
  }
}

#pragma mark [>outer func
//--------------------------------------------------------------
//  外部提供関数群
//--------------------------------------------------------------
const BOOL MB_CAP_EFFECT_IsFinish( const MB_CAP_EFFECT *effWork )
{
  return effWork->isFinish;
}
void MB_CAP_EFFECT_SetIsFinish( MB_CAP_EFFECT *effWork , const BOOL isFinish )
{
  effWork->isFinish = isFinish;
}

void MB_CAP_EFFECT_SetIsLoop( MB_CAP_EFFECT *effWork , const BOOL isLoop )
{
  effWork->isLoop = isLoop;
}

