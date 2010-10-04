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

#include "arc_def.h"
#include "stage_gra.naix"
#include "message.naix"
#include "font/font.naix"
#include "msg/msg_musical_shot.h"

#include "print/printsys.h"
#include "print/wordset.h"
#include "musical/mus_shot_photo.h"
#include "musical/musical_camera_def.h"
#include "musical/stage/sta_local_def.h"
#include "musical/stage/sta_acting.h"
#include "musical/stage/sta_act_poke.h"
#include "musical/stage/sta_act_bg.h"
#include "musical/stage/sta_act_light.h"

#include "debug/debugwin_sys.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define MUS_PHOTO_USE_DEBUG ((USE_DEBUGWIN_SYSTEM)&(1))
#define MUS_PHOTO_DRAW_TICK (0)

#define MUS_PHOTO_SCROLL_OFFSET (128)

#define MUS_PHOTO_FRAME_3D ( GFL_BG_FRAME0_M )
#define MUS_PHOTO_FRAME_MSG ( GFL_BG_FRAME1_M )
#define MUS_PHOTO_FRAME_CURTAIN ( GFL_BG_FRAME2_M )
#define MUS_PHOTO_FRAME_MASK ( GFL_BG_FRAME3_M )

#define MSU_PHOTO_DATE_POS_X (24)
#define MSU_PHOTO_DATE_POS_Y (22)
#define MSU_PHOTO_DATE_WIDTH  ( 8)
#define MSU_PHOTO_DATE_HEIGHT ( 2)
#define MSU_PHOTO_DATE_COLOR (PRINTSYS_LSB_Make(3,0x0b,0))

#define MSU_PHOTO_TITLE_POS_X (1)
#define MSU_PHOTO_TITLE_POS_Y (1)
#define MSU_PHOTO_TITLE_WIDTH  (30)
#define MSU_PHOTO_TITLE_HEIGHT ( 2)
#define MSU_PHOTO_TITLE_COLOR (PRINTSYS_LSB_Make(0xf,2,0))

#define MSU_PHOTO_PAL_FONT (0xA)
//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
#if MUS_PHOTO_USE_DEBUG
typedef struct 
{
  BOOL startAnm;
  s16  scroll;
  u16  monsno;
  u16  form;
  u16  sex;
}MUS_SHOT_DEBUGWORK;
#endif


struct _MUS_SHOT_PHOTO_WORK
{
  HEAPID heapId;

  MUSICAL_SHOT_DATA *shotData;
  MUSICAL_POKE_PARAM *musPoke[MUSICAL_POKE_MAX];
  
  GFL_G3D_CAMERA    *camera;
  GFL_BBD_SYS     *bbdSys;

  MUS_POKE_DRAW_SYSTEM  *drawSys;
  MUS_ITEM_DRAW_SYSTEM  *itemDrawSys;
  STA_BG_SYS      *bgSys;
  STA_POKE_SYS    *pokeSys;
  STA_POKE_WORK   *pokeWork[MUSICAL_POKE_MAX];
  STA_LIGHT_SYS   *lightSys;
  STA_LIGHT_WORK    *lightWork[MUSICAL_POKE_MAX];

  //メッセージ用
  GFL_BMPWIN      *msgWinDate;
  GFL_BMPWIN      *msgWinTitle;
  GFL_FONT        *fontHandleSmall;
  GFL_FONT        *fontHandleLarge;
  
#if MUS_PHOTO_USE_DEBUG
  MUS_SHOT_DEBUGWORK debWork;
#endif
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void MUS_SHOT_PHOTO_InitGraphic( MUS_SHOT_PHOTO_WORK *work );
static void MUS_SHOT_PHOTO_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode );
static void MUS_SHOT_PHOTO_ExitGraphic( MUS_SHOT_PHOTO_WORK *work );
static void MUS_SHOT_PHOTO_SetupPokemon( MUS_SHOT_PHOTO_WORK *work );
static const fx32 MUS_SHOT_PHOTO_GetAdjustPosX( MUS_SHOT_PHOTO_WORK *work , const u8 idx , const u16 monsno );
static void MUS_SHOT_PHOTO_SetupMessage( MUS_SHOT_PHOTO_WORK *work );

void MUS_SHOT_PHOTO_InitDebug( MUS_SHOT_PHOTO_WORK *work );
void MUS_SHOT_PHOTO_TermDebug( MUS_SHOT_PHOTO_WORK *work );
static void MUS_SHOT_DebugPolyDraw( MUS_SHOT_PHOTO_WORK *work );

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
                                                        shotData->shotPoke[i].perRand ,
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
  MUS_SHOT_PHOTO_SetupMessage( work );

  work->bgSys = STA_BG_InitSystem( work->heapId , NULL );
  STA_BG_CreateBg( work->bgSys , work->shotData->bgNo );

  STA_POKE_System_SetScrollOffset( work->pokeSys , MUS_PHOTO_SCROLL_OFFSET ); 
  STA_BG_SetScrollOffset( work->bgSys , MUS_PHOTO_SCROLL_OFFSET );

#if MUS_PHOTO_USE_DEBUG
  MUS_SHOT_PHOTO_InitDebug( work );
#endif

  return work;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
void MUS_SHOT_PHOTO_ExitSystem( MUS_SHOT_PHOTO_WORK *work )
{
  u8 i;
#if MUS_PHOTO_USE_DEBUG
  MUS_SHOT_PHOTO_TermDebug( work );
#endif
  STA_LIGHT_ExitSystem( work->lightSys );
  STA_BG_ExitSystem( work->bgSys );
  STA_POKE_ExitSystem( work->pokeSys );
  MUS_POKE_DRAW_TermSystem( work->drawSys );
  MUS_ITEM_DRAW_TermSystem( work->itemDrawSys );

  GFL_BMPWIN_Delete( work->msgWinTitle );
  GFL_BMPWIN_Delete( work->msgWinDate );
  GFL_FONT_Delete( work->fontHandleLarge );
  GFL_FONT_Delete( work->fontHandleSmall );
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
#if MUS_PHOTO_DRAW_TICK
  OSTick tick = OS_GetTick();
#endif

  //アニメさせないために呼ばない
#if MUS_PHOTO_USE_DEBUG
  if( work->debWork.startAnm == TRUE )
#else
  if( 0 )
#endif
  {
    STA_BG_UpdateSystem( work->bgSys );
  }
  STA_POKE_UpdateSystem( work->pokeSys );
#if MUS_PHOTO_DRAW_TICK
  {
    OSTick nowTick = OS_GetTick();
    OS_TFPrintf(3,"%5d|",OS_TicksToMicroSeconds(nowTick-tick));
    tick = nowTick;
  }
#endif
  MUS_POKE_DRAW_UpdateSystem( work->drawSys ); 
  STA_LIGHT_UpdateSystem( work->lightSys );
#if MUS_PHOTO_DRAW_TICK
  {
    OSTick nowTick = OS_GetTick();
    OS_TFPrintf(3,"%5d|",OS_TicksToMicroSeconds(nowTick-tick));
    tick = nowTick;
  }
#endif


  //3D描画  
  GFL_G3D_DRAW_Start();
  GFL_G3D_DRAW_SetLookAt();
  {
#if MUS_PHOTO_DRAW_TICK
  {
    OSTick nowTick = OS_GetTick();
    OS_TFPrintf(3,"%5d|",OS_TicksToMicroSeconds(nowTick-tick));
    tick = nowTick;
  }
#endif
    STA_LIGHT_DrawSystem( work->lightSys );
#if MUS_PHOTO_DRAW_TICK
  {
    OSTick nowTick = OS_GetTick();
    OS_TFPrintf(3,"%5d|",OS_TicksToMicroSeconds(nowTick-tick));
    tick = nowTick;
  }
#endif
    STA_POKE_DrawSystem( work->pokeSys );
#if MUS_PHOTO_DRAW_TICK
  {
    OSTick nowTick = OS_GetTick();
    OS_TFPrintf(3,"%5d|",OS_TicksToMicroSeconds(nowTick-tick));
    tick = nowTick;
  }
#endif
    MUS_POKE_DRAW_DrawSystem( work->drawSys ); 
#if MUS_PHOTO_DRAW_TICK
  {
    OSTick nowTick = OS_GetTick();
    OS_TFPrintf(3,"%5d|",OS_TicksToMicroSeconds(nowTick-tick));
    tick = nowTick;
  }
#endif
    STA_POKE_UpdateSystem_Item( work->pokeSys );  //ポケの描画後に入れること
#if MUS_PHOTO_DRAW_TICK
  {
    OSTick nowTick = OS_GetTick();
    OS_TFPrintf(3,"%5d|",OS_TicksToMicroSeconds(nowTick-tick));
    tick = nowTick;
  }
#endif
    STA_BG_DrawSystem( work->bgSys );
#if MUS_PHOTO_DRAW_TICK
  {
    OSTick nowTick = OS_GetTick();
    OS_TFPrintf(3,"%5d|",OS_TicksToMicroSeconds(nowTick-tick));
    tick = nowTick;
  }
#endif
    GFL_BBD_Draw( work->bbdSys , work->camera , NULL );
#if MUS_PHOTO_DRAW_TICK
  {
    OSTick nowTick = OS_GetTick();
    OS_TFPrintf(3,"%5d|",OS_TicksToMicroSeconds(nowTick-tick));
    tick = nowTick;
  }
#endif
  }
  MUS_SHOT_DebugPolyDraw( work );

  GFL_G3D_DRAW_End();
#if MUS_PHOTO_DRAW_TICK
  {
    OSTick nowTick = OS_GetTick();
    OS_TFPrintf(3,"%5d|",OS_TicksToMicroSeconds(nowTick-tick));
    tick = nowTick;
    OS_TFPrintf(3,"\n");
  }
#endif
  
}

//--------------------------------------------------------------
//	描画系初期化
//--------------------------------------------------------------
static void MUS_SHOT_PHOTO_InitGraphic( MUS_SHOT_PHOTO_WORK *work )
{
  { //3D系の設定
    //初期スクロール分のカメラ位置
    static const VecFx32 cam_pos = { MUSICAL_POS_X( MUS_PHOTO_SCROLL_OFFSET ), 
                                     MUSICAL_CAMERA_POS_Y,
                                     MUSICAL_CAMERA_POS_Z};
    static const VecFx32 cam_target = { MUSICAL_POS_X( MUS_PHOTO_SCROLL_OFFSET ) ,
                                        MUSICAL_CAMERA_TRG_Y ,
                                        MUSICAL_CAMERA_TRG_Z };
    static const VecFx32 cam_up = MUSICAL_CAMERA_UP;
    //エッジマーキングカラー
    static  const GXRgb stage_edge_color_table[8]=
      { GX_RGB( 0, 0, 0 ), GX_RGB( 0, 0, 0 ), 0, 0, 0, 0, 0, 0 };
    GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX256K, GFL_G3D_VMANLNK, GFL_G3D_PLT64K,
            0, work->heapId, NULL );
    GFL_BG_SetBGControl3D( 3 ); //NNS_g3dInitの中で表示優先順位変わる・・・
    GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_W );


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
    
    GFL_G3D_CAMERA_Switching( work->camera );
    //エッジマーキングカラーセット
    G3X_SetEdgeColorTable( &stage_edge_color_table[0] );
    G3X_EdgeMarking( FALSE );
    G3X_AntiAlias( TRUE );
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
      MUS_ITEM_POLIGON_ID, //影用の
			GFL_BBD_ORIGIN_CENTER,
    };
    VecFx32 scale ={FX32_ONE*4,FX32_ONE*4,FX32_ONE};
    //ビルボードシステム構築
    work->bbdSys = GFL_BBD_CreateSys( &bbdSetup , work->heapId );
    //背景のために全体を４倍する
    GFL_BBD_SetScale( work->bbdSys , &scale );
  }
  
  //BG系
  {
    // BG1 MAIN (文字
    static const GFL_BG_BGCNT_HEADER header_main1 = {
      0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x4800, GX_BG_CHARBASE_0x00000,0x5000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG2 MAIN (幕
    static const GFL_BG_BGCNT_HEADER header_main2 = {
      0, 0, 0x2000, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_512x512, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x5000, GX_BG_CHARBASE_0x08000,0x8000,
      GX_BG_EXTPLTT_01, 1, 1, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG3 MAIN (MASK
    static const GFL_BG_BGCNT_HEADER header_main3 = {
      0, 0, 0x1000, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x10000,0x08000,
      GX_BG_EXTPLTT_23, 2, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    
    MUS_SHOT_PHOTO_SetupBgFunc( &header_main1, MUS_PHOTO_FRAME_MSG , GFL_BG_MODE_TEXT);
    MUS_SHOT_PHOTO_SetupBgFunc( &header_main2, MUS_PHOTO_FRAME_CURTAIN , GFL_BG_MODE_TEXT);
    MUS_SHOT_PHOTO_SetupBgFunc( &header_main3, MUS_PHOTO_FRAME_MASK , GFL_BG_MODE_TEXT);
    GFL_BG_SetScroll( MUS_PHOTO_FRAME_CURTAIN , GFL_BG_SCROLL_Y_SET , ACT_CURTAIN_SCROLL_MAX );
  }
  //BG読み込み
  {
    ARCHANDLE *arcHandle = GFL_ARC_OpenDataHandle( 0 , work->heapId );
    GFL_ARCHDL_UTIL_TransVramPaletteEx( arcHandle , NARC_stage_gra_maku_NCLR , 
                      PALTYPE_MAIN_BG , ACT_PLT_BG_MAIN_MAKU*32 , ACT_PLT_BG_MAIN_MAKU*32 , 32 , work->heapId );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_stage_gra_maku_NCGR ,
                      MUS_PHOTO_FRAME_CURTAIN , 0 , 0, FALSE , work->heapId );
    GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_stage_gra_maku_NSCR , 
                      MUS_PHOTO_FRAME_CURTAIN ,  0 , 0, FALSE , work->heapId );

    GFL_ARCHDL_UTIL_TransVramPaletteEx( arcHandle , NARC_stage_gra_dark_mask_NCLR , 
                      PALTYPE_MAIN_BG , ACT_PLT_BG_MAIN_MASK*32 , ACT_PLT_BG_MAIN_MASK*32 , 32 , work->heapId );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_stage_gra_dark_mask_NCGR ,
                      MUS_PHOTO_FRAME_MASK , 0 , 0, FALSE , work->heapId );
    GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_stage_gra_dark_mask_NSCR , 
                      MUS_PHOTO_FRAME_MASK ,  0 , 0, FALSE , work->heapId );

    GFL_BG_LoadScreenReq(MUS_PHOTO_FRAME_CURTAIN);
    GFL_BG_LoadScreenReq(MUS_PHOTO_FRAME_MASK);
    GFL_ARC_CloseDataHandle(arcHandle);
  }
  //ライト用のアルファ設定
  G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG3 , GX_BLEND_PLANEMASK_BG0 , 0 , 10 );
  GX_SetVisibleWnd( GX_WNDMASK_OW );
  G2_SetWndOBJInsidePlane( GX_WND_PLANEMASK_BG0 | 
                           GX_WND_PLANEMASK_BG1 | 
                           GX_WND_PLANEMASK_BG2 | 
                           GX_WND_PLANEMASK_OBJ , TRUE );
  G2_SetWndOutsidePlane(   GX_WND_PLANEMASK_BG0 | 
                           GX_WND_PLANEMASK_BG1 | 
                           GX_WND_PLANEMASK_BG2 | 
                           GX_WND_PLANEMASK_BG3 | 
                           GX_WND_PLANEMASK_OBJ , TRUE );

}
//--------------------------------------------------------------------------
//  Bg初期化 機能部
//--------------------------------------------------------------------------
static void MUS_SHOT_PHOTO_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode )
{
  GFL_BG_SetBGControl( bgPlane, bgCont, mode );
  GFL_BG_SetVisible( bgPlane, VISIBLE_ON );
  GFL_BG_ClearFrame( bgPlane );
  GFL_BG_LoadScreenReq( bgPlane );
}

//--------------------------------------------------------------
//	描画系開放
//--------------------------------------------------------------
static void MUS_SHOT_PHOTO_ExitGraphic( MUS_SHOT_PHOTO_WORK *work )
{
  GFL_BG_FreeBGControl( MUS_PHOTO_FRAME_MASK );
  GFL_BG_FreeBGControl( MUS_PHOTO_FRAME_MSG );
  GFL_BG_FreeBGControl( MUS_PHOTO_FRAME_CURTAIN );
  GFL_BBD_DeleteSys( work->bbdSys );
  GFL_G3D_CAMERA_Delete( work->camera );
  GFL_G3D_Exit();
  GFL_BG_FreeBGControl( MUS_PHOTO_FRAME_3D );
  
}

//--------------------------------------------------------------
//  表示ポケモンの初期化
//--------------------------------------------------------------
static void MUS_SHOT_PHOTO_SetupPokemon( MUS_SHOT_PHOTO_WORK *work )
{
  u8 i;
  u8 bit = 1;
  VecFx32 pos = {FX32_CONST(64.0f),FX32_CONST(155.0f),FX32_CONST(170.0f)};
  VecFx32 lightpos = {FX32_CONST(64.0f),FX32_CONST(128.0f),FX32_CONST(200.0f)};
  const fx32 XBase = FX32_CONST(256.0f/(MUSICAL_POKE_MAX+1));
  
  fx32  posZArr[4];
  fx32  posZBase = FX32_CONST(80.0f);
  
  work->drawSys = MUS_POKE_DRAW_InitSystem( work->heapId );
  MUS_POKE_DRAW_SetTexAddres( work->drawSys , 0x20000 );
  work->itemDrawSys = MUS_ITEM_DRAW_InitSystem( work->bbdSys , MUSICAL_POKE_MAX*MUS_POKE_EQUIP_MAX, work->heapId );
  
  work->pokeSys = STA_POKE_InitSystem( work->heapId , NULL , work->drawSys , work->itemDrawSys , work->bbdSys );

  for( i=0;i<MUSICAL_POKE_MAX;i++ )
  {
    work->pokeWork[i] = NULL;
  }
  
  for( i=0;i<MUSICAL_POKE_MAX;i++ )
  {
    work->pokeWork[i] = STA_POKE_CreatePoke( work->pokeSys , work->musPoke[i] );

    STA_POKE_StopAnime( work->pokeSys , work->pokeWork[i] );
    STA_POKE_SetDrawItem( work->pokeSys , work->pokeWork[i] , TRUE );
  }
  
  //先にZ座標の計算
  bit = 1;
  for( i=0;i<MUSICAL_POKE_MAX;i++ )
  {
    if( (work->shotData->spotBit & bit) == 0  )
    {
      posZArr[i] = posZBase;
      posZBase += FX32_CONST(30.0f);
    }
    bit = bit<<1;
  }
  bit = 1;
  for( i=0;i<MUSICAL_POKE_MAX;i++ )
  {
    if( work->shotData->spotBit & bit )
    {
      posZArr[i] = posZBase;
      posZBase += FX32_CONST(30.0f);
    }
    bit = bit<<1;
  }

  //ライトと目立ちポケの設定
  bit = 1;
  work->lightSys = STA_LIGHT_InitSystem(work->heapId , NULL );
  for( i=0;i<MUSICAL_POKE_MAX;i++ )
  {
    lightpos.x = MUS_SHOT_PHOTO_GetAdjustPosX( work , i , work->musPoke[i]->mcssParam.monsno );

    if( work->shotData->spotBit & bit )
    {
      VecFx32 ofs = {0,-FX32_CONST(35.0f),0};
      pos.x = MUS_SHOT_PHOTO_GetAdjustPosX( work , i , work->musPoke[i]->mcssParam.monsno );
      pos.y = FX32_CONST(155.0f);
      pos.z = posZArr[i];
      STA_POKE_SetPosition( work->pokeSys , work->pokeWork[i] , &pos );
      STA_POKE_SetPositionOffset( work->pokeSys , work->pokeWork[i] , &ofs );

      lightpos.y = FX32_CONST(88.0f);
      
    }
    else
    {
      pos.x = MUS_SHOT_PHOTO_GetAdjustPosX( work , i , work->musPoke[i]->mcssParam.monsno );
      pos.y = FX32_CONST(155.0f);
      pos.z = posZArr[i];
      STA_POKE_SetPosition( work->pokeSys , work->pokeWork[i] , &pos );
      lightpos.y = FX32_CONST(128.0f);
      
    }
    work->lightWork[i] = STA_LIGHT_CreateObject( work->lightSys , ALT_CIRCLE );
    STA_LIGHT_SetPosition( work->lightSys , work->lightWork[i] , &lightpos );
    bit = bit<<1;
    
  }
}

static const fx32 MUS_SHOT_PHOTO_GetAdjustPosX( MUS_SHOT_PHOTO_WORK *work , const u8 idx , const u16 monsno )
{
  const fx32 posXArr[MUSICAL_POKE_MAX] = {FX32_CONST(128+32),
                                          FX32_CONST(128+32+64),
                                          FX32_CONST(128+32+128),
                                          FX32_CONST(128+32+192)};
  return posXArr[idx];
}

//--------------------------------------------------------------
//  メッセージ(日付)の初期化
//--------------------------------------------------------------
static void MUS_SHOT_PHOTO_SetupMessage( MUS_SHOT_PHOTO_WORK *work )
{
  work->msgWinDate = GFL_BMPWIN_Create( MUS_PHOTO_FRAME_MSG , MSU_PHOTO_DATE_POS_X , MSU_PHOTO_DATE_POS_Y ,
                  MSU_PHOTO_DATE_WIDTH , MSU_PHOTO_DATE_HEIGHT , MSU_PHOTO_PAL_FONT ,
                  GFL_BMP_CHRAREA_GET_B );
  work->msgWinTitle = GFL_BMPWIN_Create( MUS_PHOTO_FRAME_MSG , MSU_PHOTO_TITLE_POS_X , MSU_PHOTO_TITLE_POS_Y ,
                  MSU_PHOTO_TITLE_WIDTH , MSU_PHOTO_TITLE_HEIGHT , MSU_PHOTO_PAL_FONT ,
                  GFL_BMP_CHRAREA_GET_B );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->msgWinDate) , 0 );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->msgWinTitle) , 0 );

  //フォント読み込み
  work->fontHandleSmall = GFL_FONT_Create( ARCID_FONT , NARC_font_num_gftr  , GFL_FONT_LOADTYPE_FILE , FALSE , work->heapId );
  work->fontHandleLarge = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , work->heapId );
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_MAIN_BG , MSU_PHOTO_PAL_FONT*0x20, 16*2, work->heapId );
 
  //メッセージ(日付
  {
    GFL_MSGDATA     *msgHandle;
    STRBUF *srcStr;
    STRBUF *workStr  = GFL_STR_CreateBuffer( 128 , work->heapId );
    WORDSET *wordset = WORDSET_Create( work->heapId );
    msgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_musical_shot_dat , work->heapId );
    srcStr = GFL_MSG_CreateString( msgHandle , MUSICAL_SHOT_PHOTO_01 ); 
    
    //年,月,日
    WORDSET_RegisterNumber( wordset , 0 , work->shotData->year , 2 , STR_NUM_DISP_ZERO , STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterNumber( wordset , 1 , work->shotData->month, 2 , STR_NUM_DISP_ZERO , STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterNumber( wordset , 2 , work->shotData->day  , 2 , STR_NUM_DISP_ZERO , STR_NUM_CODE_DEFAULT );
    WORDSET_ExpandStr( wordset , workStr , srcStr );
    
    PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp(work->msgWinDate) , 0 , 0 ,
                         workStr , work->fontHandleSmall , MSU_PHOTO_DATE_COLOR );

    GFL_STR_DeleteBuffer( workStr );
    GFL_STR_DeleteBuffer( srcStr );
    WORDSET_Delete( wordset );
    GFL_MSG_Delete( msgHandle );
  }

  //メッセージ(タイトル
  {
    STRBUF *workStr  = GFL_STR_CreateBuffer( MUSICAL_PROGRAM_NAME_MAX , work->heapId );
    GFL_STR_SetStringCode( workStr , work->shotData->title );
    PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp(work->msgWinTitle) , 0 , 0 ,
                         workStr , work->fontHandleLarge , MSU_PHOTO_TITLE_COLOR );
    GFL_STR_DeleteBuffer( workStr );
  }

  GFL_BMPWIN_TransVramCharacter( work->msgWinDate );
  GFL_BMPWIN_MakeScreen( work->msgWinDate );
  GFL_BMPWIN_TransVramCharacter( work->msgWinTitle );
  GFL_BMPWIN_MakeScreen( work->msgWinTitle );
  GFL_BG_LoadScreenReq( MUS_PHOTO_FRAME_MSG );
}


#pragma mark [>debug
#if MUS_PHOTO_USE_DEBUG

#define MUS_SHOT_PHOTO_DEBUG_GROUP_NUMBER (50)
struct _MUS_SHOT_DEBUGWORK
{
  BOOL startAnm;
};

static void MUS_SHOT_Debug_UpdateBgNo( void* userWork , DEBUGWIN_ITEM* item );
static void MUS_SHOT_Debug_DrawBgNo( void* userWork , DEBUGWIN_ITEM* item );
static void MUS_SHOT_Debug_UpdateAnime( void* userWork , DEBUGWIN_ITEM* item );
static void MUS_SHOT_Debug_DrawAnime( void* userWork , DEBUGWIN_ITEM* item );
static void MUS_SHOT_Debug_UpdateScroll( void* userWork , DEBUGWIN_ITEM* item );
static void MUS_SHOT_Debug_DrawScroll( void* userWork , DEBUGWIN_ITEM* item );
static void MSDU_Top( void* userWork , DEBUGWIN_ITEM* item );
static void MSDD_Top( void* userWork , DEBUGWIN_ITEM* item );
static void MSDU_MonsNo( void* userWork , DEBUGWIN_ITEM* item );
static void MSDD_MonsNo( void* userWork , DEBUGWIN_ITEM* item );
static void MSDU_Form( void* userWork , DEBUGWIN_ITEM* item );
static void MSDD_Form( void* userWork , DEBUGWIN_ITEM* item );
static void MSDU_Sex( void* userWork , DEBUGWIN_ITEM* item );
static void MSDD_Sex( void* userWork , DEBUGWIN_ITEM* item );

void MUS_SHOT_PHOTO_InitDebug( MUS_SHOT_PHOTO_WORK *work )
{
//  work->debWork = GFL_HEAP_AllocMemory( work->heapId , sizeof(MUS_SHOT_DEBUGWORK) );
//  work->debWork->startAnm = FALSE;
  DEBUGWIN_InitProc( MUS_PHOTO_FRAME_MSG , work->fontHandleLarge );

  DEBUGWIN_AddGroupToTop( MUS_SHOT_PHOTO_DEBUG_GROUP_NUMBER , "MusicalShot" , work->heapId );

  DEBUGWIN_AddItemToGroupEx( MUS_SHOT_Debug_UpdateBgNo   ,MUS_SHOT_Debug_DrawBgNo   , (void*)work , MUS_SHOT_PHOTO_DEBUG_GROUP_NUMBER , work->heapId );
  DEBUGWIN_AddItemToGroupEx( MUS_SHOT_Debug_UpdateAnime  ,MUS_SHOT_Debug_DrawAnime   , (void*)work , MUS_SHOT_PHOTO_DEBUG_GROUP_NUMBER , work->heapId );
  DEBUGWIN_AddItemToGroupEx( MUS_SHOT_Debug_UpdateScroll  ,MUS_SHOT_Debug_DrawScroll   , (void*)work , MUS_SHOT_PHOTO_DEBUG_GROUP_NUMBER , work->heapId );
  DEBUGWIN_AddItemToGroupEx( MSDU_Top  ,MSDD_Top   , (void*)work , MUS_SHOT_PHOTO_DEBUG_GROUP_NUMBER , work->heapId );
  DEBUGWIN_AddItemToGroupEx( MSDU_MonsNo  ,MSDD_MonsNo   , (void*)work , MUS_SHOT_PHOTO_DEBUG_GROUP_NUMBER , work->heapId );
  DEBUGWIN_AddItemToGroupEx( MSDU_Form  ,MSDD_Form   , (void*)work , MUS_SHOT_PHOTO_DEBUG_GROUP_NUMBER , work->heapId );
  DEBUGWIN_AddItemToGroupEx( MSDU_Sex   ,MSDD_Sex    , (void*)work , MUS_SHOT_PHOTO_DEBUG_GROUP_NUMBER , work->heapId );

  work->debWork.startAnm = FALSE;
  work->debWork.scroll = MUS_PHOTO_SCROLL_OFFSET;
  work->debWork.monsno = 0;
  work->debWork.form = 0;

}

void MUS_SHOT_PHOTO_TermDebug( MUS_SHOT_PHOTO_WORK *work )
{
  DEBUGWIN_RemoveGroup( MUS_SHOT_PHOTO_DEBUG_GROUP_NUMBER );
  DEBUGWIN_ExitProc();
//  GFL_HEAP_FreeMemory( work->debWork );
}

static void MUS_SHOT_Debug_UpdateBgNo( void* userWork , DEBUGWIN_ITEM* item )
{
  MUS_SHOT_PHOTO_WORK *work = (MUS_SHOT_PHOTO_WORK*)userWork;
  
  BOOL isUpdate = FALSE;
  
  if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )
  {
    if( work->shotData->bgNo == 0 )
    {
      work->shotData->bgNo = MUSICAL_BACK_NUM-1;
    }
    else
    {
      work->shotData->bgNo--;
    }
    isUpdate = TRUE;
  }
  if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )
  {
    if( work->shotData->bgNo == MUSICAL_BACK_NUM-1 )
    {
      work->shotData->bgNo = 0;
    }
    else
    {
      work->shotData->bgNo++;
    }
    isUpdate = TRUE;
  }
  
  if( isUpdate == TRUE )
  {
    DEBUGWIN_RefreshScreen();
  }
  
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    STA_BG_ExitSystem( work->bgSys );
    work->bgSys = STA_BG_InitSystem( work->heapId , NULL );
    STA_BG_CreateBg( work->bgSys , work->shotData->bgNo );
    MUS_SHOT_PHOTO_UpdateSystem( work );
  }
}

static void MUS_SHOT_Debug_DrawBgNo( void* userWork , DEBUGWIN_ITEM* item )
{
  MUS_SHOT_PHOTO_WORK *work = (MUS_SHOT_PHOTO_WORK*)userWork;
  DEBUGWIN_ITEM_SetNameV( item , "BgNo[%2d]",work->shotData->bgNo );
}

static void MUS_SHOT_Debug_UpdateAnime( void* userWork , DEBUGWIN_ITEM* item )
{
  u8 i;
  MUS_SHOT_PHOTO_WORK *work = (MUS_SHOT_PHOTO_WORK*)userWork;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    if( work->debWork.startAnm == TRUE )
    {
      for( i=0;i<MUSICAL_POKE_MAX;i++ )
      {
        STA_POKE_StopAnime( work->pokeSys , work->pokeWork[i] );
      }
      work->debWork.startAnm = FALSE;
    }
    else
    {
      for( i=0;i<MUSICAL_POKE_MAX;i++ )
      {
        STA_POKE_StartAnime( work->pokeSys , work->pokeWork[i] );
      }
      work->debWork.startAnm = TRUE;
    }
    DEBUGWIN_RefreshScreen();
  }
}

static void MUS_SHOT_Debug_DrawAnime( void* userWork , DEBUGWIN_ITEM* item )
{
  static const char strArr[2][4] = {"OFF","ON"};
  MUS_SHOT_PHOTO_WORK *work = (MUS_SHOT_PHOTO_WORK*)userWork;

  DEBUGWIN_ITEM_SetNameV( item , "Anime[%3s]",strArr[work->debWork.startAnm] );
  
}

static void MUS_SHOT_Debug_UpdateScroll( void* userWork , DEBUGWIN_ITEM* item )
{
  MUS_SHOT_PHOTO_WORK *work = (MUS_SHOT_PHOTO_WORK*)userWork;
  BOOL isUpdate = FALSE;
  u8 value = 1;

  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X )
  {
    value = 8;
  }
  
  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT )
  {
    work->debWork.scroll+=value;
    if( work->debWork.scroll > 256 )
    {
      work->debWork.scroll = 256;
    }
    isUpdate = TRUE;
  }
  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT )
  {
    work->debWork.scroll-=value;
    if( work->debWork.scroll < 0 )
    {
      work->debWork.scroll = 0;
    }
    isUpdate = TRUE;
  }
  if( isUpdate == TRUE )
  {
    //初期スクロール分のカメラ位置
    VecFx32 cam_pos = { 0 ,  MUSICAL_CAMERA_POS_Y , MUSICAL_CAMERA_POS_Z};
    VecFx32 cam_target = { 0, MUSICAL_CAMERA_TRG_Y , MUSICAL_CAMERA_TRG_Z };
    cam_pos.x = MUSICAL_POS_X( work->debWork.scroll );
    cam_target.x = MUSICAL_POS_X( work->debWork.scroll );
    GFL_G3D_CAMERA_SetPos( work->camera , &cam_pos );
    GFL_G3D_CAMERA_SetTarget( work->camera , &cam_target );
    GFL_G3D_CAMERA_Switching( work->camera );

    DEBUGWIN_RefreshScreen();

    MUS_SHOT_PHOTO_UpdateSystem( work );
  }
}

static void MUS_SHOT_Debug_DrawScroll( void* userWork , DEBUGWIN_ITEM* item )
{
  MUS_SHOT_PHOTO_WORK *work = (MUS_SHOT_PHOTO_WORK*)userWork;

  DEBUGWIN_ITEM_SetNameV( item , "Scroll[%d]",work->debWork.scroll );
  
}

static void MSDU_Top( void* userWork , DEBUGWIN_ITEM* item )
{
  MUS_SHOT_PHOTO_WORK *work = (MUS_SHOT_PHOTO_WORK*)userWork;
  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT )
  {
    if( work->shotData->spotBit < 15 )
    {
      work->shotData->spotBit++;
    }
    DEBUGWIN_RefreshScreen();
  }
  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT )
  {
    if( work->shotData->spotBit > 0 )
    {
      work->shotData->spotBit--;
    }
    DEBUGWIN_RefreshScreen();
  }
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    if( work->shotData->spotBit == 0 )
    {
      work->shotData->spotBit = 15;
    }
    else
    {
      work->shotData->spotBit = 0;
    }
    DEBUGWIN_RefreshScreen();
  }
}

static void MSDD_Top( void* userWork , DEBUGWIN_ITEM* item )
{
  MUS_SHOT_PHOTO_WORK *work = (MUS_SHOT_PHOTO_WORK*)userWork;
  const BOOL top1 = (work->shotData->spotBit&1);
  const BOOL top2 = (work->shotData->spotBit&2);
  const BOOL top3 = (work->shotData->spotBit&4);
  const BOOL top4 = (work->shotData->spotBit&8);
  DEBUGWIN_ITEM_SetNameV( item , "TopBit[%c%c%c%c]",(top1>0?'o':'x'),(top2>0?'o':'x'),(top3>0?'o':'x'),(top4>0?'o':'x') );
}
static void MSDU_MonsNo( void* userWork , DEBUGWIN_ITEM* item )
{
  MUS_SHOT_PHOTO_WORK *work = (MUS_SHOT_PHOTO_WORK*)userWork;
  u16 val = 1;
  

  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X ){val = 10;}
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R ){val = 100;}

  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT )
  {
    if( work->debWork.monsno +val <= 649 )
    {
      work->debWork.monsno += val;
    }
    else
    {
      work->debWork.monsno = 0;
    }
    DEBUGWIN_RefreshScreen();
  }
  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT )
  {
    if( work->debWork.monsno < val )
    {
      work->debWork.monsno = 649;
    }
    else
    {
      work->debWork.monsno -= val;
    }
    DEBUGWIN_RefreshScreen();
  }
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    u8 i;
    STA_POKE_ExitSystem( work->pokeSys );
    MUS_POKE_DRAW_TermSystem( work->drawSys );
    MUS_ITEM_DRAW_TermSystem( work->itemDrawSys );
    STA_LIGHT_ExitSystem( work->lightSys );
    
    for( i=0;i<MUSICAL_POKE_MAX;i++ )
    {
      work->musPoke[i]->mcssParam.monsno = work->debWork.monsno;
      work->musPoke[i]->mcssParam.form = work->debWork.form;
      work->musPoke[i]->mcssParam.sex = work->debWork.sex;
    }
    MUS_SHOT_PHOTO_SetupPokemon(work);
  }
  DEBUGWIN_UpdateFrame();
}

static void MSDD_MonsNo( void* userWork , DEBUGWIN_ITEM* item )
{
  MUS_SHOT_PHOTO_WORK *work = (MUS_SHOT_PHOTO_WORK*)userWork;
  DEBUGWIN_ITEM_SetNameV( item , "Monsno[%d](A:Apply)",work->debWork.monsno );
}

static void MSDU_Form( void* userWork , DEBUGWIN_ITEM* item )
{
  MUS_SHOT_PHOTO_WORK *work = (MUS_SHOT_PHOTO_WORK*)userWork;
  u16 val = 1;
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X ){val = 10;}
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R ){val = 100;}

  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT )
  {
    if( work->debWork.form +val <= 27 )
    {
      work->debWork.form += val;
    }
    else
    {
      work->debWork.form = 0;
    }
    DEBUGWIN_RefreshScreen();
  }
  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT )
  {
    if( work->debWork.form < val )
    {
      work->debWork.form = 27;
    }
    else
    {
      work->debWork.form -= val;
    }
    DEBUGWIN_RefreshScreen();
  }
}

static void MSDD_Form( void* userWork , DEBUGWIN_ITEM* item )
{
  MUS_SHOT_PHOTO_WORK *work = (MUS_SHOT_PHOTO_WORK*)userWork;
  DEBUGWIN_ITEM_SetNameV( item , "Form[%d]",work->debWork.form );
}
static void MSDU_Sex( void* userWork , DEBUGWIN_ITEM* item )
{
  MUS_SHOT_PHOTO_WORK *work = (MUS_SHOT_PHOTO_WORK*)userWork;

  if( (GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT)||
      (GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT) )
  {
    if( work->debWork.sex != 0 )
    {
      work->debWork.sex = 0;
    }
    else
    {
      work->debWork.sex = 1;
    }
    DEBUGWIN_RefreshScreen();
  }
}

static void MSDD_Sex( void* userWork , DEBUGWIN_ITEM* item )
{
  MUS_SHOT_PHOTO_WORK *work = (MUS_SHOT_PHOTO_WORK*)userWork;
  DEBUGWIN_ITEM_SetNameV( item , "Sex[%d]",work->debWork.sex );
}

#endif //MUS_PHOTO_USE_DEBUG


static void MUS_SHOT_DebugPolyDraw( MUS_SHOT_PHOTO_WORK *work )
{
#if defined(DEBUG_ONLY_FOR_ariizumi_nobuhiko)
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
  {
    VecFx32 pos;
    G3_PushMtx();
    //カメラ設定取得
    {
      MtxFx33       mtxBillboard;
      VecFx16       vecN;
      VecFx32   vecNtmp;
      MtxFx43   mtxCamera, mtxCameraInv;
      static const VecFx32 cam_pos = MUSICAL_CAMERA_POS;
      static const VecFx32 cam_target = MUSICAL_CAMERA_TRG;
      static const VecFx32 cam_up = MUSICAL_CAMERA_UP;
    
      G3_LookAt( &cam_pos, &cam_up, &cam_target, &mtxCamera );  //mtxCameraには行列計算結果が返る
      MTX_Inverse43( &mtxCamera, &mtxCameraInv );     //カメラ逆行列取得
      MTX_Copy43To33( &mtxCameraInv, &mtxBillboard );   //カメラ逆行列から回転行列を取り出す

      VEC_Subtract( &cam_pos, &cam_target, &vecNtmp );
      VEC_Normalize( &vecNtmp, &vecNtmp );
      VEC_Fx16Set( &vecN, vecNtmp.x, vecNtmp.y, vecNtmp.z );
    }
    {
      static u8 polyCol = 0;
      static fx32 polyOfs = FX32_CONST(100.0f);
      if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP )
      {
        polyOfs += FX32_ONE;
        OS_FPrintf( 2,"PolyOfs[%.1f]\n",FX_FX32_TO_F32(polyOfs));
      }
      if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN )
      {
        polyOfs -= FX32_ONE;
        OS_FPrintf( 2,"PolyOfs[%.1f]\n",FX_FX32_TO_F32(polyOfs));
      }
      //板ポリ
      G3_PolygonAttr(GX_LIGHTMASK_NONE,       // no lights
               GX_POLYGONMODE_MODULATE,     // modulation mode
               GX_CULL_NONE,          // cull back
               0,                // polygon ID(0 - 63)
               31,     // alpha(0 - 31)
               0                  // OR of GXPolygonAttrMisc's value
               );
      G3_TexImageParam( GX_TEXFMT_NONE ,
                        GX_TEXGEN_NONE ,
                        0 ,
                        0 ,
                        0 ,
                        0 ,
                        GX_TEXPLTTCOLOR0_USE ,
                        0 );
      pos.x = MUSICAL_POS_X(128.0f);
      pos.y = MUSICAL_POS_Y(96.0f);
      pos.z = polyOfs;

      G3_Translate( pos.x, pos.y, pos.z );

      {
        const fx32 size = FX32_ONE;
        G3_Begin(GX_BEGIN_QUADS);
        G3_Color( GX_RGB( polyCol, polyCol, polyCol ) );
        polyCol++;
        if( polyCol > 31 )
        {
          polyCol = 0;
        }

        G3_Vtx( FX32_CONST(-7.8), FX32_CONST( 5.8), 0 );
        G3_Vtx( FX32_CONST( 7.8), FX32_CONST( 5.8), 0 );
        G3_Vtx( FX32_CONST( 7.8), FX32_CONST(-5.8), 0 );
        G3_Vtx( FX32_CONST(-7.8), FX32_CONST(-5.8), 0 );

        G3_End();
      }
    }
    G3_PopMtx(1);
    
  }
  
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_SELECT )
  {
    u8 i;
    static VecFx32 sScale = {FX32_ONE*16,FX32_ONE*16,FX32_ONE*16};
    VecFx32 scale;
    if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP )
    {
      sScale.y += FX32_CONST(0.05);
      OS_FPrintf( 2,"scaleY[%.2f]\n",FX_FX32_TO_F32(sScale.y));
    }
    if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN )
    {
      sScale.y -= FX32_CONST(0.05);
      OS_FPrintf( 2,"scaleY[%.2f]\n",FX_FX32_TO_F32(sScale.y));
    }
    if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT )
    {
      sScale.x -= FX32_CONST(0.05);
      OS_FPrintf( 2,"scaleX[%.2f]\n",FX_FX32_TO_F32(sScale.x));
    }
    if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT )
    {
      sScale.x += FX32_CONST(0.05);
      OS_FPrintf( 2,"scaleX[%.2f]\n",FX_FX32_TO_F32(sScale.x));
    }
    scale.x = sScale.x/16;
    scale.y = sScale.y/16;
    scale.z = sScale.z/16;
    for( i=0;i<MUSICAL_POKE_MAX;i++ )
    {
      STA_POKE_SetScale( work->pokeSys , work->pokeWork[i] , &scale );
    }
  }


#endif //DEB_ARI
  
}
