//======================================================================
/**
 * @file  sta_act_audience.h
 * @brief ステージ用　観客
 * @author  ariizumi
 * @data  09/05/26
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "arc_def.h"
#include "stage_audience.naix"

#include "sta_local_def.h"
#include "sta_act_audience.h"
#include "script/sta_act_script_def.h"

#include "test/ariizumi/ari_debug.h"



//======================================================================
//  define
//======================================================================
#pragma mark [> define

#define STA_AUDI_NUM_X (8)
#define STA_AUDI_NUM_Y (5)
#define STA_AUDI_NUM   (STA_AUDI_NUM_X*STA_AUDI_NUM_Y)

#define STA_AUDI_TOP (2)

#define STA_AUDI_SURFACE (CLSYS_DEFREND_SUB)

#define STA_AUDI_LOOK_DELAY (GFL_STD_MtRand0( 60 )+1)
#define STA_AUDI_BIG_ANGLE_OFFSET (64)  //向きがBIGになるための位置差分

#define STA_AUDI_NO_LOOK (0xFFFF)
//======================================================================
//  enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  SAP_LEFT_BIG,
  SAP_LEFT_SMALL,
  SAP_RIGHT_SMALL,
  SAP_RIGHT_BIG,
}STAGE_AUDIENCE_POS;

//======================================================================
//  typedef struct
//======================================================================
#pragma mark [> struct
//観客ワーク
typedef struct
{
//  GFL_CLWK* cell;
  u8  trgPoke;
  u8  delay;
  u16 lookPos;
  u16 selfPos;
}STA_AUDI_WORK;

//メインワーク
struct _STA_AUDI_SYS
{
  HEAPID heapId;
  
  u32 pltIdx;
  u32 ncgIdx[STA_AUDI_NUM];
  u32 anmIdx[STA_AUDI_NUM];

  ACTING_WORK *actWork;
  BOOL attentionPokeFlg[MUSICAL_POKE_MAX];
  BOOL isUpdateAttention;

//  GFL_CLUNIT* cellUnit;
  STA_AUDI_WORK audience[STA_AUDI_NUM];
};

//======================================================================
//  proto
//======================================================================
#pragma mark [> proto

static void STA_AUDI_InitGraphic( STA_AUDI_SYS *work );
static void STA_AUDI_TermGraphic( STA_AUDI_SYS *work );

static void STA_AUDI_InitCell( STA_AUDI_SYS *work );
static void STA_AUDI_TermCell( STA_AUDI_SYS *work );

static void STA_AUDI_UpdateAudience( STA_AUDI_SYS *work );
static void STA_AUDI_UpdateAudienceFunc( STA_AUDI_SYS *work , STA_AUDI_WORK *audiWork );

static void STA_AUDI_CheckAudienceState( STA_AUDI_SYS *work );
static void STA_AUDI_SetAudienceState_Default( STA_AUDI_SYS *work );

//--------------------------------------------------------------
//  初期化
//--------------------------------------------------------------
STA_AUDI_SYS* STA_AUDI_InitSystem( HEAPID heapId , ACTING_WORK *actWork )
{
  u8 i;
  STA_AUDI_SYS *work = GFL_HEAP_AllocMemory( heapId , sizeof(STA_AUDI_SYS) );
  
  work->heapId = heapId;
  STA_AUDI_InitGraphic( work );
  STA_AUDI_InitCell( work );
  
  work->actWork = actWork;
  work->isUpdateAttention = FALSE;
  for( i=0;i<MUSICAL_POKE_MAX;i++ )
  {
    work->attentionPokeFlg[i] = FALSE;
  }
  
  return work;
}

//--------------------------------------------------------------
//  開放
//--------------------------------------------------------------
void	STA_AUDI_ExitSystem( STA_AUDI_SYS *work )
{
  STA_AUDI_TermCell( work );
  STA_AUDI_TermGraphic( work );
  GFL_HEAP_FreeMemory( work );
}

//--------------------------------------------------------------
//  更新
//--------------------------------------------------------------
void	STA_AUDI_UpdateSystem( STA_AUDI_SYS *work )
{
  if( STA_ACT_GetCurtainHeight( work->actWork ) == 0 )
  {
    u8 i;
    if( GFL_STD_MtRand0( 10 ) == 0 )
    {
      const u8 changeNum = 3;
      for( i=0;i<changeNum ;i++ )
      {
        const u8 anm = GFL_STD_MtRand0( 4 );
        const u8 num = GFL_STD_MtRand0( STA_AUDI_NUM );
//        GFL_CLACT_WK_SetAnmSeq( work->audience[num].cell, anm );
      }
    }
  }
  else
  {
    if( work->isUpdateAttention == TRUE )
    {
      STA_AUDI_CheckAudienceState( work );
      work->isUpdateAttention = FALSE;
    }
    STA_AUDI_UpdateAudience( work );
  }
}

//--------------------------------------------------------------
//  絵の初期化
//--------------------------------------------------------------
static void STA_AUDI_InitGraphic( STA_AUDI_SYS *work )
{
  u8 i;
  ARCHANDLE *arcHandle = GFL_ARC_OpenDataHandle( ARCID_STAGE_AUDIENCE , work->heapId );
/*
  work->pltIdx = GFL_CLGRP_PLTT_Register( arcHandle , 
                            NARC_stage_audience_audience_obj_NCLR , 
                            CLSYS_DRAW_SUB , 
                            0 , 
                            work->heapId  );

  for( i=0;i<STA_AUDI_NUM;i++ )
  {
    work->anmIdx[i] = GFL_CLGRP_CELLANIM_Register( arcHandle , 
                                NARC_stage_audience_audience_01_NCER , 
                                NARC_stage_audience_audience_01_NANR, 
                                work->heapId  );
    work->ncgIdx[i] = GFL_CLGRP_CGR_Register_VramTransfer( arcHandle , 
                                NARC_stage_audience_audience_01_NCGR , 
                                FALSE , 
                                CLSYS_DRAW_SUB , 
                                work->anmIdx[i] ,
                                work->heapId  );
  }
*/
  GFL_ARC_CloseDataHandle(arcHandle);
}

static void STA_AUDI_TermGraphic( STA_AUDI_SYS *work )
{
  u8 i;
  /*
  for( i=0;i<STA_AUDI_NUM;i++ )
  {
    GFL_CLGRP_CGR_Release(work->ncgIdx[i]);
    GFL_CLGRP_CELLANIM_Release(work->anmIdx[i]);
  }
  GFL_CLGRP_PLTT_Release(work->pltIdx);
  */
}

//--------------------------------------------------------------
//  セルの初期化
//--------------------------------------------------------------
static void STA_AUDI_InitCell( STA_AUDI_SYS *work )
{
  u8 x,y;
  /*
  GFL_CLWK_DATA initWork;
  work->cellUnit = GFL_CLACT_UNIT_Create( STA_AUDI_NUM , 0 , work->heapId );
  
  for( y=0;y<STA_AUDI_NUM_Y;y++ )
  {
    const s8 ofs = (y%2==0 ? 4 : -4 ) + GFL_STD_MtRand0(3)-1;
    for( x=0;x<STA_AUDI_NUM_X;x++ )
    {
      const u8 i = x + y*STA_AUDI_NUM_X;
      initWork.pos_x = 16 + x*32 +ofs;
      initWork.pos_y = 80 + y*28;
      initWork.anmseq = ( initWork.pos_x < 128 ? SAP_RIGHT_SMALL : SAP_LEFT_SMALL );
      initWork.softpri = STA_AUDI_NUM-i;
      initWork.bgpri = 0;
      work->audience[i].cell = GFL_CLACT_WK_CreateVT( work->cellUnit ,
                             work->ncgIdx[i] ,
                             work->pltIdx ,
                             work->anmIdx[i] ,
                             &initWork ,
                             STA_AUDI_SURFACE , 
                             work->heapId );
      GFL_CLACT_WK_SetDrawEnable( work->audience[i].cell, TRUE );

      work->audience[i].selfPos = initWork.pos_x;
      work->audience[i].delay = STA_AUDI_LOOK_DELAY;
      work->audience[i].trgPoke = STA_AUDI_NO_TARGET;
    }
  }
  */
  for( y=0;y<STA_AUDI_NUM_Y;y++ )
  {
    const s8 ofs = (y%2==0 ? 4 : -4 ) + GFL_STD_MtRand0(3)-1;
    for( x=0;x<STA_AUDI_NUM_X;x++ )
    {
      u8 ix,iy;
      u16 topCharNo;
      BOOL isFlip = FALSE;
      const u8 i = x + y*STA_AUDI_NUM_X;
      
      if( x<2 )
      {
        isFlip = TRUE;
        topCharNo = 0x08+1024;  //1024は水平反転
      }
      else if( x<4 )
      {
        isFlip = TRUE;
        topCharNo = 0x0c+1024;  //1024は水平反転
      }
      else if( x<6 )
      {
        topCharNo = 0x0c;
      }
      else
      {
        topCharNo = 0x08;
      }
      
      for( iy=0;iy<4;iy++ )
      {
        for( ix=0;ix<4;ix++ )
        {
          u8 scrX;
          if( isFlip == TRUE )
          {
            scrX = x*4+(3-ix);
          }
          else
          {
            scrX = x*4+ix;
          }
          GFL_BG_FillScreen( ACT_FRAME_SUB_AUDI_FACE , 
                             topCharNo+ix+(0x20*iy) , 
                             scrX , y*4+STA_AUDI_TOP+iy ,
                             1 , 1 , 0 );
          
        }
      }

      work->audience[i].selfPos = x*32 + 16;
      work->audience[i].delay = STA_AUDI_LOOK_DELAY;
      work->audience[i].trgPoke = STA_AUDI_NO_TARGET;
    }
  }
  GFL_BG_LoadScreenReq( ACT_FRAME_SUB_AUDI_FACE );
  
}

static void STA_AUDI_TermCell( STA_AUDI_SYS *work )
{
  u8 i;
  /*
  for( i=0;i<STA_AUDI_NUM;i++ )
  {
    GFL_CLACT_WK_Remove( work->audience[i].cell );
  }
  GFL_CLACT_UNIT_Delete( work->cellUnit );
  */
}

static void STA_AUDI_UpdateAudience( STA_AUDI_SYS *work )
{
  u8 i;
  for( i=0;i<STA_AUDI_NUM;i++ )
  {
    STA_AUDI_UpdateAudienceFunc( work , &work->audience[i] );
  }
}

static void STA_AUDI_UpdateAudienceFunc( STA_AUDI_SYS *work , STA_AUDI_WORK *audiWork )
{
  if( audiWork->delay > 0 )
  {
    audiWork->delay--;
  }
  if( audiWork->delay == 0 )
  {
    if( audiWork->trgPoke == STA_AUDI_NO_TARGET )
    {
      audiWork->lookPos = STA_AUDI_NO_LOOK;
      if( audiWork->selfPos < 128 )
      {
        //GFL_CLACT_WK_SetAnmSeq( audiWork->cell , SAP_RIGHT_SMALL );
      }
      else
      {
        //GFL_CLACT_WK_SetAnmSeq( audiWork->cell , SAP_LEFT_SMALL );
      }
    }
    else
    {
      STA_POKE_SYS *pokeSys = STA_ACT_GetPokeSys( work->actWork );
      STA_POKE_WORK *pokeWork = STA_ACT_GetPokeWork( work->actWork , audiWork->trgPoke );
      const u16 stgOfs = STA_ACT_GetStageScroll( work->actWork );
      VecFx32 pokePos;
      s16 posOffset;
      STA_POKE_GetPosition( pokeSys , pokeWork , &pokePos );
      posOffset = (FX_FX32_TO_F32( pokePos.x )-stgOfs) - audiWork->selfPos;
      
      if( posOffset < -STA_AUDI_BIG_ANGLE_OFFSET )
      {
        //GFL_CLACT_WK_SetAnmSeq( audiWork->cell , SAP_LEFT_BIG );
      }
      else
      if( posOffset > STA_AUDI_BIG_ANGLE_OFFSET )
      {
        //GFL_CLACT_WK_SetAnmSeq( audiWork->cell , SAP_RIGHT_BIG );
      }
      else
      if( posOffset < 0 )
      {
        //GFL_CLACT_WK_SetAnmSeq( audiWork->cell , SAP_LEFT_SMALL );
      }
      else
      {
        //GFL_CLACT_WK_SetAnmSeq( audiWork->cell , SAP_RIGHT_SMALL );
      }
    }
    audiWork->delay = STA_AUDI_LOOK_DELAY;
  }
}

//観客状態をチェック
static void STA_AUDI_CheckAudienceState( STA_AUDI_SYS *work )
{
  u8 i;
  u8 trgNum = 0;
  u8 trgArr[MUSICAL_POKE_MAX] = {0,0,0,0};

  for( i=0;i<MUSICAL_POKE_MAX;i++ )
  {
    if( work->attentionPokeFlg[i] == TRUE )
    {
      trgArr[trgNum] = i;
      trgNum++;
    }
  }
  
  if( trgNum != 1 )
  {
    STA_AUDI_SetAudienceState_Default( work );
  }
  else
  {
    for( i=0;i<STA_AUDI_NUM;i++ )
    {
      work->audience[i].delay = STA_AUDI_LOOK_DELAY;
      work->audience[i].trgPoke = trgArr[GFL_STD_MtRand0(trgNum)];
    }
  }
}

//観客状態を(注目ポケなし)へ
static void STA_AUDI_SetAudienceState_Default( STA_AUDI_SYS *work )
{
  u8 i;
  for( i=0;i<STA_AUDI_NUM;i++ )
  {
    work->audience[i].delay = STA_AUDI_LOOK_DELAY;
    work->audience[i].trgPoke = STA_AUDI_NO_TARGET;
  }
}

void	STA_AUDI_SetAttentionPoke( STA_AUDI_SYS *work , const u8 trgPoke , const BOOL flg )
{
  work->attentionPokeFlg[trgPoke] = flg;
  
  work->isUpdateAttention = TRUE;
}

