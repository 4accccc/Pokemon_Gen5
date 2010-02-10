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

#include "./mb_cap_obj.h"
#include "./mb_cap_poke.h"
#include "./mb_cap_ball.h"

#include "./mb_cap_snd_def.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define MB_CAP_BALL_ANIM_SHOT_SPEED (6)
#define MB_CAP_BALL_ANIM_SHOT_FRAME (6)

#define MB_CAP_BALL_HEIGHT_MAX (48)
#define MB_CAP_BALL_HEIGHT_SCALE_MAX FX32_CONST(1.5)

#define MB_CAP_BALL_FALL_SPEED FX32_CONST(1.5)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef void (*MB_CAP_BALL_STATE_FUNC)(MB_CAPTURE_WORK *capWork , MB_CAP_BALL *ballWork );

struct _MB_CAP_BALL
{
  BOOL isFinish;
  
  u16 cnt;
  u16 shotCnt;  //飛んでる時間

  u16 anmFrame;
  u16 anmIdx;
  
  int objIdx;
  int objShadowIdx;
  
  VecFx32 targetPos;
  VecFx32 pos;
  fx32    height;
  
  VecFx32 spd;
  
  MB_CAP_BALL_STATE state;
  MB_CAP_BALL_STATE_FUNC stateFunc;
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static const BOOL MB_CAP_BALL_CheckHitObj_ShotFinish( MB_CAPTURE_WORK *capWork , MB_CAP_BALL *ballWork );
static void MB_CAP_BALL_CheckHitObj_CheckSide( MB_CAPTURE_WORK *capWork , MB_CAP_BALL *ballWork , 
                const s8 idxX , const s8 idxY , const s8 ofsX , const s8 ofsY );
static void MB_CAP_BALL_CheckHitPoke_Shooting( MB_CAPTURE_WORK *capWork , MB_CAP_BALL *ballWork );
static void MB_CAP_BALL_CheckHitBonus_Shooting( MB_CAPTURE_WORK *capWork , MB_CAP_BALL *ballWork );

static void MB_CAP_BALL_StateShot(MB_CAPTURE_WORK *capWork , MB_CAP_BALL *ballWork );
static void MB_CAP_BALL_StateCaptureAnime(MB_CAPTURE_WORK *capWork , MB_CAP_BALL *ballWork );
static void MB_CAP_BALL_StateFlying(MB_CAPTURE_WORK *capWork , MB_CAP_BALL *ballWork );

//--------------------------------------------------------------
//	オブジェクト作成
//--------------------------------------------------------------
MB_CAP_BALL* MB_CAP_BALL_CreateObject( MB_CAPTURE_WORK *capWork , MB_CAP_BALL_INIT_WORK *initWork )
{
  const HEAPID heapId = MB_CAPTURE_GetHeapId( capWork );
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  ARCHANDLE *arcHandle = MB_CAPTURE_GetArcHandle( capWork );
  MB_CAP_BALL *ballWork = GFL_HEAP_AllocClearMemory( heapId , sizeof( MB_CAP_BALL ) );
  int resIdx;
  const int resShadowIdx = MB_CAPTURE_GetBbdResIdx( capWork , MCBR_SHADOW );
  const BOOL flg = TRUE;
  const u16 rot = 0x10000-initWork->rotAngle;

  if( initWork->isBonus == TRUE )
  {
    resIdx = MB_CAPTURE_GetBbdResIdx( capWork , MCBR_BALL_BONUS );
  }
  else
  {
    resIdx = MB_CAPTURE_GetBbdResIdx( capWork , MCBR_BALL );
  }

  ballWork->pos.x = FX32_CONST(128);
  ballWork->pos.y = FX32_CONST(MB_CAP_UPPER_BALL_POS_BASE_Y);
  ballWork->pos.z = FX32_CONST(MB_CAP_UPPER_BALL_BASE_Z);
  
  ballWork->targetPos = initWork->targetPos;
  
  //文字演出のためサイズを2倍にしてるのでデフォは半分
  ballWork->objIdx = GFL_BBD_AddObject( bbdSys , 
                                     resIdx ,
                                     FX32_HALF , 
                                     FX32_HALF , 
                                     &ballWork->pos ,
                                     31 ,
                                     GFL_BBD_LIGHT_NONE );
  ballWork->objShadowIdx = GFL_BBD_AddObject( bbdSys , 
                                     resShadowIdx ,
                                     FX32_HALF , 
                                     FX32_HALF , 
                                     &ballWork->pos ,
                                     31 ,
                                     GFL_BBD_LIGHT_NONE );
  GFL_BBD_SetObjectDrawEnable( bbdSys , ballWork->objIdx , &flg );
  GFL_BBD_SetObjectDrawEnable( bbdSys , ballWork->objShadowIdx , &flg );
  GFL_BBD_SetObjectRotate( bbdSys , ballWork->objIdx , &rot );
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
    
    ballWork->shotCnt = F32_CONST( FX_Div( len_sqrt , MB_CAP_BALL_SHOT_SPEED ) );
  }
  
  ballWork->cnt = 0;
  ballWork->anmFrame = 0;
  ballWork->anmIdx = 0;
  ballWork->isFinish = FALSE;
  ballWork->state = MCBS_FLYING;
  ballWork->stateFunc = MB_CAP_BALL_StateShot;
  
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
  ballWork->stateFunc( capWork , ballWork);
}

//--------------------------------------------------------------
//	ボールとOBJの当たり判定(ショット終了時処理
//--------------------------------------------------------------
static const BOOL MB_CAP_BALL_CheckHitObj_ShotFinish( MB_CAPTURE_WORK *capWork , MB_CAP_BALL *ballWork )
{
  int i;
  BOOL isHitAny = FALSE;
  BOOL isHitAnyPoke = FALSE;
  BOOL isBlockAnyPoke = FALSE;
  MB_CAP_HIT_WORK ballHit;
  
  //オブジェとの当たり判定
  MB_CAP_BALL_GetHitWork( ballWork , &ballHit );
  for( i=0;i<MB_CAP_OBJ_NUM;i++ )
  {
    MB_CAP_OBJ *obj = MB_CAPTURE_GetObjWork( capWork , i );
    MB_CAP_HIT_WORK objHit;
    BOOL isHit;
    MB_CAP_OBJ_GetHitWork( obj , &objHit );
    isHit = MB_CAPTURE_CheckHit( &ballHit , &objHit );
    if( isHit == TRUE )
    {
      MB_CAP_OBJ_StartAnime( obj );
      PMSND_PlaySE( MB_SND_GRASS_SHAKE );

      isHitAny = TRUE;
      if( i < MB_CAP_OBJ_MAIN_NUM )
      {
        int j;
        for( j=0;j<MB_CAP_POKE_NUM;j++ )
        {
          MB_CAP_POKE *pokeWork = MB_CAPTURE_GetPokeWork( capWork , j );
          const MB_CAP_POKE_STATE pokeState = MB_CAP_POKE_GetState( pokeWork );
          const BOOL isHitPoke = MB_CAP_POKE_CheckPos( pokeWork , i%MB_CAP_OBJ_X_NUM , i/MB_CAP_OBJ_X_NUM );
          if( pokeState == MCPS_LOOK && isHitPoke == TRUE )
          {
            MB_CAP_POKE_SetRun( capWork , pokeWork );
            isHitAnyPoke = TRUE;
          }
          else
          if( pokeState == MCPS_RUN_HIDE && isHitPoke == TRUE )
          {
            MB_CAP_POKE_SetRun( capWork , pokeWork );
            isHitAnyPoke = TRUE;
          }
          else
          if( pokeState == MCPS_HIDE && isHitPoke == TRUE )
          {
            PMSND_PlaySE( MB_SND_BALL_HIT_WOOD );
            isHitAnyPoke = TRUE;
            isBlockAnyPoke = TRUE;
          }
        }
        if( isHitAnyPoke == FALSE )
        {
          VecFx32 effPos = ballWork->pos;
          
          effPos.y -= ballWork->height;
          effPos.z = FX32_CONST(MB_CAP_EFFECT_Z);
          MB_CAPTURE_CreateEffect( capWork , &effPos , MCET_HIT );
        }
        //周囲の方向チェック
        {
          const s8 idxX = i%MB_CAP_OBJ_X_NUM;
          const s8 idxY = i/MB_CAP_OBJ_X_NUM;
          
          MB_CAP_BALL_CheckHitObj_CheckSide( capWork , ballWork , idxX , idxY , -1 ,  0 );
          MB_CAP_BALL_CheckHitObj_CheckSide( capWork , ballWork , idxX , idxY ,  1 ,  0 );
          MB_CAP_BALL_CheckHitObj_CheckSide( capWork , ballWork , idxX , idxY ,  0 , -1 );
          MB_CAP_BALL_CheckHitObj_CheckSide( capWork , ballWork , idxX , idxY ,  0 ,  1 );
        }
      }
      else
      if( i < MB_CAP_OBJ_SUB_D_START )
      {
        //上の木
        const s8 idxX = (i-MB_CAP_OBJ_SUB_U_START);
        const s8 idxY = -1;
        VecFx32 effPos = ballWork->pos;
        effPos.y -= ballWork->height;
        effPos.z = FX32_CONST(MB_CAP_EFFECT_Z);
        MB_CAP_BALL_CheckHitObj_CheckSide( capWork , ballWork , idxX , idxY , 0 , 1 );
        MB_CAPTURE_CreateEffect( capWork , &effPos , MCET_HIT );
        PMSND_PlaySE( MB_SND_BALL_HIT_WOOD );
      }
      else
      if( i < MB_CAP_OBJ_SUB_R_START )
      {
        //下の草(下の水)
        const s8 idxX = (i-MB_CAP_OBJ_SUB_D_START);
        const s8 idxY = MB_CAP_OBJ_Y_NUM;
        VecFx32 effPos = ballWork->pos;
        effPos.y -= ballWork->height;
        effPos.z = FX32_CONST(MB_CAP_EFFECT_Z);
        MB_CAP_BALL_CheckHitObj_CheckSide( capWork , ballWork , idxX , idxY , 0 , -1 );
        MB_CAPTURE_CreateEffect( capWork , &effPos , MCET_HIT );
        PMSND_PlaySE( MB_SND_GRASS_SHAKE );
      }
      else
      if( i < MB_CAP_OBJ_SUB_L_START )
      {
        //右の草
        const s8 idxX = MB_CAP_OBJ_X_NUM;
        const s8 idxY = (i-MB_CAP_OBJ_SUB_R_START);
        VecFx32 effPos = ballWork->pos;
        effPos.y -= ballWork->height;
        effPos.z = FX32_CONST(MB_CAP_EFFECT_Z);
        MB_CAP_BALL_CheckHitObj_CheckSide( capWork , ballWork , idxX , idxY , -1 , 0 );
        MB_CAPTURE_CreateEffect( capWork , &effPos , MCET_HIT );
        PMSND_PlaySE( MB_SND_GRASS_SHAKE );
      }
      else
      if( i < MB_CAP_OBJ_NUM )
      {
        //左の草
        const s8 idxX = -1;
        const s8 idxY = (i-MB_CAP_OBJ_SUB_L_START);
        VecFx32 effPos = ballWork->pos;
        effPos.y -= ballWork->height;
        effPos.z = FX32_CONST(MB_CAP_EFFECT_Z);
        MB_CAP_BALL_CheckHitObj_CheckSide( capWork , ballWork , idxX , idxY , 1 , 0 );
        MB_CAPTURE_CreateEffect( capWork , &effPos , MCET_HIT );
        PMSND_PlaySE( MB_SND_GRASS_SHAKE );
      }
      
      break;
    }
  }
  if( isHitAny == FALSE )
  {
    VecFx32 effPos = ballWork->pos;
    
    effPos.y -= ballWork->height;
    effPos.z = FX32_CONST(MB_CAP_EFFECT_Z);
    MB_CAPTURE_CreateEffect( capWork , &effPos , MCET_HIT );
    PMSND_PlaySE( MB_SND_BALL_NO_HIT );
  }
  return isBlockAnyPoke;
}

//--------------------------------------------------------------
//	ボールとOBJの当たり判定(ショット終了時処理の後の4方向チェック
//--------------------------------------------------------------
static void MB_CAP_BALL_CheckHitObj_CheckSide( MB_CAPTURE_WORK *capWork , MB_CAP_BALL *ballWork , const s8 idxX , const s8 idxY , const s8 ofsX , const s8 ofsY )
{
  if( idxX + ofsX >= 0 &&
      idxX + ofsX < MB_CAP_OBJ_X_NUM &&
      idxY + ofsY >= 0 &&
      idxY + ofsY < MB_CAP_OBJ_Y_NUM )
  {
    u8 i;
    for( i=0;i<MB_CAP_POKE_NUM;i++ )
    {
      MB_CAP_POKE *pokeWork = MB_CAPTURE_GetPokeWork( capWork , i );
      const MB_CAP_POKE_STATE pokeState = MB_CAP_POKE_GetState( pokeWork );
      const BOOL isHitPoke = MB_CAP_POKE_CheckPos( pokeWork , idxX+ofsX , idxY+ofsY );
      
      if( isHitPoke == TRUE &&
          pokeState == MCPS_RUN_HIDE )
      {
        MB_CAP_POKE_DIR dir;
        if( ofsX == -1 )
        {
          dir = MCPD_LEFT;
        }
        else
        if( ofsX == 1 )
        {
          dir = MCPD_RIGHT;
        }
        else
        if( ofsY == -1 )
        {
          dir = MCPD_UP;
        }
        else
        if( ofsY == 1 )
        {
          dir = MCPD_DOWN;
        }
        
        MB_CAP_POKE_SetRunChangeDir( capWork , pokeWork , dir );
      }
    }
    
  }
}

//--------------------------------------------------------------
//	ボールとポケモンの当たり判定(飛んでる時
//--------------------------------------------------------------
static void MB_CAP_BALL_CheckHitPoke_Shooting( MB_CAPTURE_WORK *capWork , MB_CAP_BALL *ballWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  int i;
  MB_CAP_HIT_WORK ballHit;
  
  //ポケとの当たり判定
  MB_CAP_BALL_GetHitWork( ballWork , &ballHit );
  for( i=0;i<MB_CAP_POKE_NUM;i++ )
  {
    MB_CAP_HIT_WORK pokeHit;
    MB_CAP_POKE *pokeWork = MB_CAPTURE_GetPokeWork( capWork , i );
    const MB_CAP_POKE_STATE pokeState = MB_CAP_POKE_GetState( pokeWork );
    BOOL isHit;

    MB_CAP_POKE_GetHitWork( pokeWork , &pokeHit );
    isHit = MB_CAPTURE_CheckHit( &ballHit , &pokeHit );
    if( isHit == TRUE && 
        MB_CAP_POKE_CheckCanCapture( pokeWork ) == TRUE )
    {
      const u16 cellIdx = 0;
      const u16 rot = 0;
      //文字演出のためサイズを2倍にしてるのでデフォは半分
      const fx16 size = FX16_HALF;
      VecFx32 effPos = ballWork->pos;
      MB_CAPTURE_GetPokeFunc( capWork , ballWork , i );
      
      ballWork->stateFunc = MB_CAP_BALL_StateCaptureAnime;
      
      effPos.y -= ballWork->height;
      effPos.z = FX32_CONST(MB_CAP_EFFECT_Z);
      ballWork->cnt = 0;
      ballWork->state = MCBS_CAPTURE_SMOKE;
      GFL_BBD_SetObjectSiz( bbdSys , ballWork->objIdx , &size , &size );
      GFL_BBD_SetObjectCelIdx( bbdSys , ballWork->objIdx , &cellIdx );
      GFL_BBD_SetObjectRotate( bbdSys , ballWork->objIdx , &rot );
      MB_TPrintf("Hit!!\n");
      PMSND_PlaySE( MB_SND_POKE_CAPTURE );
      {
        //スコアの計算
        const s16 subX = FX_FX32_TO_F32(ballHit.pos->x - pokeHit.pos->x);
        const s16 subY = FX_FX32_TO_F32(ballHit.pos->y - pokeHit.pos->y);
        const s16 subZ = FX_FX32_TO_F32(ballHit.height - pokeHit.height);
        const u8 absX = MATH_ABS( subX );
        MB_TPrintf("[%3d:%3d:%3d]",subX,subY,subZ);
        if( absX < 5 )
        {
          MB_CAPTURE_AddScore( capWork , 100 , TRUE );
          MB_CAPTURE_CreateEffect( capWork , &effPos , MCET_CAPTURE_SMOKE3 );
          MB_TPrintf("[100]\n");
        }
        else
        if( absX < 10 )
        {
          MB_CAPTURE_AddScore( capWork , 70 , TRUE );
          MB_CAPTURE_CreateEffect( capWork , &effPos , MCET_CAPTURE_SMOKE2 );
          MB_TPrintf("[ 70]\n");
        }
        else
        {
          MB_CAPTURE_AddScore( capWork , 50 , TRUE );
          MB_CAPTURE_CreateEffect( capWork , &effPos , MCET_CAPTURE_SMOKE );
          MB_TPrintf("[ 50]\n");
        }
      }
      break;
    }
  }
}

//--------------------------------------------------------------
//	スターとポケモンの当たり判定(飛んでる時
//--------------------------------------------------------------
static void MB_CAP_BALL_CheckHitBonus_Shooting( MB_CAPTURE_WORK *capWork , MB_CAP_BALL *ballWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  int i;
  MB_CAP_OBJ *starWork = MB_CAPTURE_GetStarWork( capWork );
  MB_CAP_HIT_WORK ballHit,starHit;
  
  if( MB_CAP_OBJ_GetEnable( capWork , starWork ) == TRUE )
  {
    BOOL isHit;
    //スターとの当たり判定
    MB_CAP_BALL_GetHitWork( ballWork , &ballHit );
    MB_CAP_OBJ_GetHitWork( starWork , &starHit );
    isHit = MB_CAPTURE_CheckHit( &ballHit , &starHit );
    if( isHit == TRUE )
    {
      VecFx32 effPos = ballWork->pos;
      effPos.z -= FX32_ONE;
      MB_CAPTURE_CreateEffect( capWork , &effPos , MCET_BONUS );
      MB_CAPTURE_HitStarFunc( capWork , starWork );
      ballWork->isFinish = TRUE;
    }
  }
}

#pragma mark [>state func
//--------------------------------------------------------------
//	ステート:飛んでる
//--------------------------------------------------------------
static void MB_CAP_BALL_StateShot(MB_CAPTURE_WORK *capWork , MB_CAP_BALL *ballWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  VecFx32 pos;

  VEC_Add( &ballWork->pos , &ballWork->spd , &ballWork->pos );
  
  GFL_BBD_SetObjectTrans( bbdSys , ballWork->objShadowIdx , &ballWork->pos );

  //高さ計算
  {
    u16 angle = 0x8000 * ballWork->cnt / ballWork->shotCnt;
    fx16 sin = FX_SinIdx( angle );
    fx16 size = FX_Mul( MB_CAP_BALL_HEIGHT_SCALE_MAX , sin ) + FX16_ONE;
    //文字演出のためサイズを2倍にしてるのでデフォは半分
    size = size>>1;
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
  ballWork->cnt++;
  if( ballWork->cnt > ballWork->shotCnt )
  {
    const BOOL blockPoke = MB_CAP_BALL_CheckHitObj_ShotFinish( capWork , ballWork );
    
    if( blockPoke == TRUE )
    {
      //ぶっ飛ぶ処理へ
      if( GFUser_GetPublicRand0(2) == 0 )
      {
        ballWork->spd.x = FX32_CONST(3) + GFUser_GetPublicRand0( FX32_CONST(3) );
      }
      else
      {
        ballWork->spd.x = FX32_CONST(-3) - GFUser_GetPublicRand0( FX32_CONST(3) );
      }
      
      ballWork->spd.y = GFUser_GetPublicRand0( FX32_CONST(1) ) - FX32_HALF;
      ballWork->spd.z = FX32_CONST(4) + GFUser_GetPublicRand0( FX32_CONST(2) );
      
      ballWork->anmIdx = 0;
      GFL_BBD_SetObjectCelIdx( bbdSys , ballWork->objIdx , &ballWork->anmIdx );
      ballWork->stateFunc = MB_CAP_BALL_StateFlying;
    }
    else
    {
      ballWork->isFinish = TRUE;
    }
  }
  else
  {
    //通常の当たり判定
    MB_CAP_BALL_CheckHitPoke_Shooting( capWork , ballWork );
    MB_CAP_BALL_CheckHitBonus_Shooting( capWork , ballWork );
  }
}

//--------------------------------------------------------------
//	ステート:捕獲アニメ
//--------------------------------------------------------------
static void MB_CAP_BALL_StateCaptureAnime(MB_CAPTURE_WORK *capWork , MB_CAP_BALL *ballWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  ballWork->cnt++;
  switch( ballWork->state )
  {
  case MCBS_CAPTURE_SMOKE: //煙終了待ち
    if( ballWork->cnt > MB_CAP_EFFECT_SMOKE_FRAME_MAX )
    {
      ballWork->cnt = 0;
      ballWork->state = MCBS_CAPTURE_FALL;
    }
    break;
  case MCBS_CAPTURE_FALL: //落ちる
    {
      VecFx32 pos;
      if( ballWork->height <= MB_CAP_BALL_FALL_SPEED )
      {
        const BOOL isDisp = FALSE;
        VecFx32 effPos = ballWork->pos;
        effPos.z -= FX32_ONE;
        MB_CAPTURE_CreateEffect( capWork , &effPos , MCET_CAPTURE_TRANS );

        ballWork->height = 0;
        ballWork->cnt = 0;
        ballWork->state = MCBS_CAPTURE_TRANS;
        GFL_BBD_SetObjectDrawEnable( bbdSys , ballWork->objShadowIdx , &isDisp );
        PMSND_PlaySE( MB_SND_POKE_TRANS );
      }
      else
      {
        ballWork->height -= MB_CAP_BALL_FALL_SPEED;
      }
      pos.x = ballWork->pos.x;
      pos.y = ballWork->pos.y - ballWork->height;
      pos.z = ballWork->pos.z + FX32_ONE;
      GFL_BBD_SetObjectTrans( bbdSys , ballWork->objIdx , &pos );
    }
    break;
  case MCBS_CAPTURE_TRANS: //飛んでいく
    {
      const fx16 rate = FX32_CONST(ballWork->cnt)/MB_CAP_EFFECT_TRANS_FRAME_MAX;
      //文字演出のためサイズを2倍にしてるのでデフォは半分
      const fx16 scaleX = (FX16_ONE - rate)/2;
      const fx16 scaleY = (FX16_ONE + rate)/2;
      VecFx32 pos;
      
      pos.x = ballWork->pos.x;
      pos.y = ballWork->pos.y - rate*16;
      pos.z = ballWork->pos.z;
      GFL_BBD_SetObjectTrans( bbdSys , ballWork->objIdx , &pos );
      GFL_BBD_SetObjectSiz( bbdSys , ballWork->objIdx , &scaleX , &scaleY );
      if( ballWork->cnt > MB_CAP_EFFECT_SMOKE_FRAME_MAX )
      {
        const BOOL isDisp = FALSE;
        ballWork->isFinish = TRUE;
        GFL_BBD_SetObjectDrawEnable( bbdSys , ballWork->objIdx , &isDisp );
      }
    }
    break;
  }
}

static void MB_CAP_BALL_StateFlying(MB_CAPTURE_WORK *capWork , MB_CAP_BALL *ballWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  VecFx32 pos;
  
  //位置
  ballWork->pos.x += ballWork->spd.x;
  ballWork->pos.y += ballWork->spd.y;
  GFL_BBD_SetObjectTrans( bbdSys , ballWork->objShadowIdx , &ballWork->pos );

  //高さ
  ballWork->spd.z -= FX32_CONST(0.25);
  if( ballWork->height + ballWork->spd.z < 0 )
  {
    ballWork->spd.z *= -1;
    ballWork->spd.z = ballWork->spd.z*3/4;
    ballWork->height = ballWork->spd.z;
  }
  else
  {
    ballWork->height += ballWork->spd.z;
  }
  
  {
    //高さ拡縮処理
    //文字演出のためサイズを2倍にしてるのでデフォは半分
    fx16 size = (ballWork->height/128) + FX16_HALF;
    GFL_BBD_SetObjectSiz( bbdSys , ballWork->objIdx , &size , &size );
  }
  
  pos.x = ballWork->pos.x;
  pos.y = ballWork->pos.y - ballWork->height;
  pos.z = ballWork->pos.z + FX32_ONE;
  GFL_BBD_SetObjectTrans( bbdSys , ballWork->objIdx , &pos );

  //回る
  {
    u16 rot;
    const u16 rotVal = 0x800;
    GFL_BBD_GetObjectRotate( bbdSys , ballWork->objIdx , &rot );
    if( pos.x > 0 )
    {
      if( rot + rotVal > 0xFFFF )
      {
        rot = rot + rotVal - 0x10000;
      }
      else
      {
        rot += rotVal;
      }
    }
    else
    {
      if( rot - rotVal < 0 )
      {
        rot = rot - rotVal + 0x10000;
      }
      else
      {
        rot -= rotVal;
      }
    }
    GFL_BBD_SetObjectRotate( bbdSys , ballWork->objIdx , &rot );
  }

  if( ballWork->pos.x <= FX32_CONST(-32)    ||
      ballWork->pos.x >= FX32_CONST(256+32) )
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

//--------------------------------------------------------------
//  当たり判定作成
//--------------------------------------------------------------
void MB_CAP_BALL_GetHitWork( MB_CAP_BALL *ballWork , MB_CAP_HIT_WORK *hitWork )
{
  hitWork->pos = &ballWork->pos;
  hitWork->height = ballWork->height;
  hitWork->size.x = FX32_CONST(MB_CAP_BALL_HITSIZE);
  hitWork->size.y = FX32_CONST(MB_CAP_BALL_HITSIZE);
  hitWork->size.z = FX32_CONST(MB_CAP_BALL_HITSIZE);
}

//--------------------------------------------------------------
//  当たり判定作成
//--------------------------------------------------------------
const MB_CAP_BALL_STATE MB_CAP_BALL_GetState( const MB_CAP_BALL *ballWork )
{
  return ballWork->state;
}
