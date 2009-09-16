//======================================================================
/**
 * @file	mus_shot_photo.c
 * @brief	ミュージカルショット　上画面写真
 * @author	ariizumi
 * @data	09/09/15
 *
 * モジュール名：MUS_SHOT_PHOTO
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "mus_shot_photo.h"
#include "musical/musical_camera_def.h"
#include "musical/stage/sta_local_def.h"
#include "musical/stage/sta_acting.h"
#include "musical/stage/sta_act_poke.h"
#include "musical/stage/sta_act_bg.h"

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
struct _MUS_SHOT_PHOTO_WORK
{
  HEAPID heapId;

  MUSICAL_SHOT_DATA *shotData;
  MUSICAL_POKE_PARAM *musPoke[MUSICAL_POKE_MAX];
  
  GFL_G3D_CAMERA    *camera;
  GFL_BBD_SYS     *bbdSys;

  ACTING_WORK     *actWork;

  MUS_POKE_DRAW_SYSTEM  *drawSys;
  MUS_ITEM_DRAW_SYSTEM  *itemDrawSys;
  STA_BG_SYS      *bgSys;
  STA_POKE_SYS    *pokeSys;
  STA_POKE_WORK   *pokeWork[MUSICAL_POKE_MAX];
  
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void MUS_SHOT_PHOTO_InitGraphic( MUS_SHOT_PHOTO_WORK *work );
static void MUS_SHOT_PHOTO_ExitGraphic( MUS_SHOT_PHOTO_WORK *work );
static void MUS_SHOT_PHOTO_SetupPokemon( MUS_SHOT_PHOTO_WORK *work );

//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
MUS_SHOT_PHOTO_WORK* MUS_SHOT_PHOTO_InitSystem( MUSICAL_SHOT_DATA *shotData , HEAPID heapId )
{
  u8 i,j;
  MUS_SHOT_PHOTO_WORK *work = GFL_HEAP_AllocClearMemory( heapId , sizeof( MUS_SHOT_PHOTO_WORK ) );
  
  work->heapId = heapId;
  work->shotData = shotData;
  
  for( i=0;i<MUSICAL_POKE_MAX;i++ )
  {
    
    work->musPoke[i] = MUSICAL_SYSTEM_InitMusPokeParam( shotData->shotPoke[i].monsno , 
                                                        shotData->shotPoke[i].sex , 
                                                        shotData->shotPoke[i].form , 
                                                        shotData->shotPoke[i].rare , 
                                                        heapId );
    for( j=0;j<MUSICAL_ITEM_EQUIP_MAX;j++ )
    {
      const MUS_POKE_EQUIP_POS ePos = shotData->shotPoke[i].equip[j].equipPos;
      if( ePos != MUS_POKE_EQU_INVALID )
      {
        work->musPoke[i]->equip[ePos].itemNo = shotData->shotPoke[i].equip[j].itemNo;
        work->musPoke[i]->equip[ePos].angle = shotData->shotPoke[i].equip[j].angle;
        work->musPoke[i]->equip[ePos].priority = j;
      }
    }
  }
  
  MUS_SHOT_PHOTO_InitGraphic( work );
  MUS_SHOT_PHOTO_SetupPokemon( work );
  return work;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
void MUS_SHOT_PHOTO_ExitSystem( MUS_SHOT_PHOTO_WORK *work )
{
  u8 i;
  STA_POKE_ExitSystem( work->pokeSys );
  MUS_POKE_DRAW_TermSystem( work->drawSys );
  MUS_ITEM_DRAW_TermSystem( work->itemDrawSys );

  MUS_SHOT_PHOTO_ExitGraphic( work );
  for( i=0;i<MUSICAL_POKE_MAX;i++ )
  {
    GFL_HEAP_FreeMemory( work->musPoke[i] );
  }
  
  GFL_HEAP_FreeMemory( work );
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
void MUS_SHOT_PHOTO_UpdateSystem( MUS_SHOT_PHOTO_WORK *work )
{
  STA_POKE_UpdateSystem( work->pokeSys );
  MUS_POKE_DRAW_UpdateSystem( work->drawSys ); 


  //3D描画  
  GFL_G3D_DRAW_Start();
  GFL_G3D_DRAW_SetLookAt();
  {
    STA_POKE_DrawSystem( work->pokeSys );
    MUS_POKE_DRAW_DrawSystem( work->drawSys ); 
    STA_POKE_UpdateSystem_Item( work->pokeSys );  //ポケの描画後に入れること
    GFL_BBD_Draw( work->bbdSys , work->camera , NULL );
  }
  GFL_G3D_DRAW_End();
  
}

//--------------------------------------------------------------
//	描画系初期化
//--------------------------------------------------------------
static void MUS_SHOT_PHOTO_InitGraphic( MUS_SHOT_PHOTO_WORK *work )
{
  { //3D系の設定
    static const VecFx32 cam_pos = MUSICAL_CAMERA_POS;
    static const VecFx32 cam_target = MUSICAL_CAMERA_TRG;
    static const VecFx32 cam_up = MUSICAL_CAMERA_UP;
    //エッジマーキングカラー
    static  const GXRgb stage_edge_color_table[8]=
      { GX_RGB( 0, 0, 0 ), GX_RGB( 0, 0, 0 ), 0, 0, 0, 0, 0, 0 };
    GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX256K, GFL_G3D_VMANLNK, GFL_G3D_PLT80K,
            0, work->heapId, NULL );
    GFL_BG_SetBGControl3D( 2 ); //NNS_g3dInitの中で表示優先順位変わる・・・
    GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_W );
#if 0 //透視射影カメラ
    work->camera =  GFL_G3D_CAMERA_Create( GFL_G3D_PRJPERS, 
                       FX32_SIN13,
                       FX32_COS13,
                       FX_F32_TO_FX32( 1.33f ),
                       NULL,
                       FX32_ONE,
                       FX32_ONE * 300,
                       NULL,
                       &cam_pos,
                       &cam_up,
                       &cam_target,
                       work->heapId );
#else
    //正射影カメラ
    work->camera =  GFL_G3D_CAMERA_Create( GFL_G3D_PRJORTH, 
                       FX32_ONE*12.0f,
                       0,
                       0,
                       FX32_ONE*16.0f,
                       MUSICAL_CAMERA_NEAR,
                       MUSICAL_CAMERA_FAR,
                       NULL,
                       &cam_pos,
                       &cam_up,
                       &cam_target,
                       work->heapId );
#endif
    
    GFL_G3D_CAMERA_Switching( work->camera );
    //エッジマーキングカラーセット
    G3X_SetEdgeColorTable( &stage_edge_color_table[0] );
    G3X_EdgeMarking( TRUE );
    G3X_AntiAlias( FALSE );
    G3X_AlphaBlend( TRUE );
    
    GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO , GX_BUFFERMODE_Z );
  }
  {
    GFL_BBD_SETUP bbdSetup = {
      128,128,
      {FX32_ONE,FX32_ONE,FX32_ONE},
      GX_RGB(0,0,0),
      GX_RGB(0,0,0),
      GX_RGB(0,0,0),
      GX_RGB(0,0,0),
      ACT_POLYID_SHADOW, //影用の
			GFL_BBD_ORIGIN_CENTER,
    };
    VecFx32 scale ={FX32_ONE*4,FX32_ONE*4,FX32_ONE};
    //ビルボードシステム構築
    work->bbdSys = GFL_BBD_CreateSys( &bbdSetup , work->heapId );
    //背景のために全体を４倍する
    GFL_BBD_SetScale( work->bbdSys , &scale );
  }
}

//--------------------------------------------------------------
//	描画系開放
//--------------------------------------------------------------
static void MUS_SHOT_PHOTO_ExitGraphic( MUS_SHOT_PHOTO_WORK *work )
{
  GFL_BBD_DeleteSys( work->bbdSys );
  GFL_G3D_CAMERA_Delete( work->camera );
  GFL_G3D_Exit();
  
}

//--------------------------------------------------------------
//  表示ポケモンの初期化
//--------------------------------------------------------------
static void MUS_SHOT_PHOTO_SetupPokemon( MUS_SHOT_PHOTO_WORK *work )
{
  u8 i;
  VecFx32 pos = {FX32_CONST(64.0f),FX32_CONST(160.0f),FX32_CONST(170.0f)};
  const fx32 XBase = FX32_CONST(256.0f/(MUSICAL_POKE_MAX+1));
  
  work->drawSys = MUS_POKE_DRAW_InitSystem( work->heapId );
  MUS_POKE_DRAW_SetTexAddres( work->drawSys , 0x40000 );
  work->itemDrawSys = MUS_ITEM_DRAW_InitSystem( work->bbdSys , MUSICAL_POKE_MAX*MUS_POKE_EQUIP_MAX, work->heapId );
  
  work->pokeSys = STA_POKE_InitSystem( work->heapId , work->actWork , work->drawSys , work->itemDrawSys , work->bbdSys );

  for( i=0;i<MUSICAL_POKE_MAX;i++ )
  {
    work->pokeWork[i] = NULL;
  }

  
  for( i=0;i<MUSICAL_POKE_MAX;i++ )
  {
    pos.x = XBase*(i+1);
    work->pokeWork[i] = STA_POKE_CreatePoke( work->pokeSys , work->musPoke[i] );
    STA_POKE_SetPosition( work->pokeSys , work->pokeWork[i] , &pos );

    pos.z -= FX32_CONST(30.0f); //ひとキャラの厚みは30と見る
    STA_POKE_StartAnime( work->pokeSys , work->pokeWork[i] );
    STA_POKE_SetDrawItem( work->pokeSys , work->pokeWork[i] , TRUE );
  }
}


