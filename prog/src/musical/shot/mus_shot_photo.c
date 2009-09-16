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
#include "mus_shot_photo.h"
#include "musical/musical_camera_def.h"
#include "musical/stage/sta_local_def.h"
#include "musical/stage/sta_acting.h"
#include "musical/stage/sta_act_poke.h"
#include "musical/stage/sta_act_bg.h"
#include "musical/stage/sta_act_light.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define MUS_PHOTO_SCROLL_OFFSET (128)

#define MUS_PHOTO_BG_3D ( GFL_BG_FRAME0_M )
#define MUS_PHOTO_BG_MSG ( GFL_BG_FRAME1_M )
#define MUS_PHOTO_BG_CURTAIN ( GFL_BG_FRAME2_M )

#define MSU_PHOTO_DATE_POS_X (25)
#define MSU_PHOTO_DATE_POS_Y (22)
#define MSU_PHOTO_DATE_WIDTH  ( 8)
#define MSU_PHOTO_DATE_HEIGHT ( 2)
#define MSU_PHOTO_DATE_COLOR (PRINTSYS_LSB_Make(3,0x0b,0))

#define MSU_PHOTO_PAL_FONT (0xA)
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

  MUS_POKE_DRAW_SYSTEM  *drawSys;
  MUS_ITEM_DRAW_SYSTEM  *itemDrawSys;
  STA_BG_SYS      *bgSys;
  STA_POKE_SYS    *pokeSys;
  STA_POKE_WORK   *pokeWork[MUSICAL_POKE_MAX];
  STA_LIGHT_SYS   *lightSys;
  STA_LIGHT_WORK    *lightWork[MUSICAL_POKE_MAX];

  //メッセージ用
  GFL_BMPWIN      *msgWin;
  GFL_FONT        *fontHandle;
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void MUS_SHOT_PHOTO_InitGraphic( MUS_SHOT_PHOTO_WORK *work );
static void MUS_SHOT_PHOTO_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode );
static void MUS_SHOT_PHOTO_ExitGraphic( MUS_SHOT_PHOTO_WORK *work );
static void MUS_SHOT_PHOTO_SetupPokemon( MUS_SHOT_PHOTO_WORK *work );
static void MUS_SHOT_PHOTO_SetupMessage( MUS_SHOT_PHOTO_WORK *work );

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
  MUS_SHOT_PHOTO_SetupMessage( work );

  work->bgSys = STA_BG_InitSystem( work->heapId , NULL );
  STA_BG_CreateBg( work->bgSys , work->shotData->bgNo );

  STA_POKE_System_SetScrollOffset( work->pokeSys , MUS_PHOTO_SCROLL_OFFSET ); 
  STA_BG_SetScrollOffset( work->bgSys , MUS_PHOTO_SCROLL_OFFSET );
  return work;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
void MUS_SHOT_PHOTO_ExitSystem( MUS_SHOT_PHOTO_WORK *work )
{
  u8 i;
  STA_LIGHT_ExitSystem( work->lightSys );
  STA_BG_ExitSystem( work->bgSys );
  STA_POKE_ExitSystem( work->pokeSys );
  MUS_POKE_DRAW_TermSystem( work->drawSys );
  MUS_ITEM_DRAW_TermSystem( work->itemDrawSys );

  GFL_BMPWIN_Delete( work->msgWin );
  GFL_FONT_Delete( work->fontHandle );
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
  //アニメさせないために呼ばない
  //STA_BG_UpdateSystem( work->bgSys );
  STA_POKE_UpdateSystem( work->pokeSys );
  MUS_POKE_DRAW_UpdateSystem( work->drawSys ); 
  STA_LIGHT_UpdateSystem( work->lightSys );


  //3D描画  
  GFL_G3D_DRAW_Start();
  GFL_G3D_DRAW_SetLookAt();
  {
    STA_LIGHT_DrawSystem( work->lightSys );
    STA_POKE_DrawSystem( work->pokeSys );
    MUS_POKE_DRAW_DrawSystem( work->drawSys ); 
    STA_POKE_UpdateSystem_Item( work->pokeSys );  //ポケの描画後に入れること
    STA_BG_DrawSystem( work->bgSys );
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
    GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX256K, GFL_G3D_VMANLNK, GFL_G3D_PLT80K,
            0, work->heapId, NULL );
    GFL_BG_SetBGControl3D( 2 ); //NNS_g3dInitの中で表示優先順位変わる・・・
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
  
  //BG系
  {
    // BG1 MAIN (文字
    static const GFL_BG_BGCNT_HEADER header_main1 = {
      0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x4800, GX_BG_CHARBASE_0x08000,0x8000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG2 MAIN (幕
    static const GFL_BG_BGCNT_HEADER header_main2 = {
      0, 0, 0x1000, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x5000, GX_BG_CHARBASE_0x00000,0x4000,
      GX_BG_EXTPLTT_01, 1, 1, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    
    MUS_SHOT_PHOTO_SetupBgFunc( &header_main1, MUS_PHOTO_BG_MSG , GFL_BG_MODE_TEXT);
    MUS_SHOT_PHOTO_SetupBgFunc( &header_main2, MUS_PHOTO_BG_CURTAIN , GFL_BG_MODE_TEXT);
    GFL_BG_SetScroll( MUS_PHOTO_BG_CURTAIN , GFL_BG_SCROLL_Y_SET , ACT_CURTAIN_SCROLL_MAX );
  }
  //BG読み込み
  {
    ARCHANDLE *arcHandle = GFL_ARC_OpenDataHandle( ARCID_STAGE_GRA , work->heapId );
    GFL_ARCHDL_UTIL_TransVramPalette( arcHandle , NARC_stage_gra_maku_NCLR , 
                      PALTYPE_MAIN_BG , 0 , 0 , work->heapId );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_stage_gra_maku_NCGR ,
                      MUS_PHOTO_BG_CURTAIN , 0 , 0, FALSE , work->heapId );
    GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_stage_gra_maku_NSCR , 
                      MUS_PHOTO_BG_CURTAIN ,  0 , 0, FALSE , work->heapId );
    GFL_BG_LoadScreenReq(MUS_PHOTO_BG_CURTAIN);
    GFL_ARC_CloseDataHandle(arcHandle);
  }
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
  GFL_BG_FreeBGControl( MUS_PHOTO_BG_MSG );
  GFL_BG_FreeBGControl( MUS_PHOTO_BG_CURTAIN );
  GFL_BBD_DeleteSys( work->bbdSys );
  GFL_G3D_CAMERA_Delete( work->camera );
  GFL_G3D_Exit();
  GFL_BG_FreeBGControl( MUS_PHOTO_BG_3D );
  
}

//--------------------------------------------------------------
//  表示ポケモンの初期化
//--------------------------------------------------------------
static void MUS_SHOT_PHOTO_SetupPokemon( MUS_SHOT_PHOTO_WORK *work )
{
  u8 i;
  u8 bit = 1;
  const fx32 posXArr[MUSICAL_POKE_MAX] = {FX32_CONST(128+32),
                                          FX32_CONST(128+32+64),
                                          FX32_CONST(128+32+128),
                                          FX32_CONST(128+32+192)};
  VecFx32 pos = {FX32_CONST(64.0f),FX32_CONST(160.0f),FX32_CONST(170.0f)};
  VecFx32 lightpos = {FX32_CONST(64.0f),FX32_CONST(128.0f),FX32_CONST(200.0f)};
  const fx32 XBase = FX32_CONST(256.0f/(MUSICAL_POKE_MAX+1));
  
  work->drawSys = MUS_POKE_DRAW_InitSystem( work->heapId );
  MUS_POKE_DRAW_SetTexAddres( work->drawSys , 0x40000 );
  work->itemDrawSys = MUS_ITEM_DRAW_InitSystem( work->bbdSys , MUSICAL_POKE_MAX*MUS_POKE_EQUIP_MAX, work->heapId );
  
  work->pokeSys = STA_POKE_InitSystem( work->heapId , NULL , work->drawSys , work->itemDrawSys , work->bbdSys );

  for( i=0;i<MUSICAL_POKE_MAX;i++ )
  {
    work->pokeWork[i] = NULL;
  }
  
  for( i=0;i<MUSICAL_POKE_MAX;i++ )
  {
    pos.x = posXArr[i];
    work->pokeWork[i] = STA_POKE_CreatePoke( work->pokeSys , work->musPoke[i] );
    STA_POKE_SetPosition( work->pokeSys , work->pokeWork[i] , &pos );

    pos.z -= FX32_CONST(30.0f); //ひとキャラの厚みは30と見る
    STA_POKE_StopAnime( work->pokeSys , work->pokeWork[i] );
    STA_POKE_SetDrawItem( work->pokeSys , work->pokeWork[i] , TRUE );
  }
  
  //ライトと目立ちポケの設定
  work->lightSys = STA_LIGHT_InitSystem(work->heapId , NULL );
  for( i=0;i<MUSICAL_POKE_MAX;i++ )
  {
    if( work->shotData->spotBit & bit )
    {
      lightpos.x = posXArr[i];
      work->lightWork[i] = STA_LIGHT_CreateObject( work->lightSys , ALT_CIRCLE );
      STA_LIGHT_SetPosition( work->lightSys , work->lightWork[i] , &lightpos );
    }
    else
    {
      work->lightWork[i] = NULL;
    }
    bit = bit<<1;
  }

}


//--------------------------------------------------------------
//  メッセージ(日付)の初期化
//--------------------------------------------------------------
static void MUS_SHOT_PHOTO_SetupMessage( MUS_SHOT_PHOTO_WORK *work )
{
  work->msgWin = GFL_BMPWIN_Create( MUS_PHOTO_BG_MSG , MSU_PHOTO_DATE_POS_X , MSU_PHOTO_DATE_POS_Y ,
                  MSU_PHOTO_DATE_WIDTH , MSU_PHOTO_DATE_HEIGHT , MSU_PHOTO_PAL_FONT ,
                  GFL_BMP_CHRAREA_GET_B );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->msgWin) , 0 );

  //フォント読み込み
  work->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_small_nftr , GFL_FONT_LOADTYPE_FILE , FALSE , work->heapId );
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_MAIN_BG , MSU_PHOTO_PAL_FONT*0x20, 16*2, work->heapId );
 
  //メッセージ
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
    
    PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp(work->msgWin) , 0 , 0 ,
                         workStr , work->fontHandle , MSU_PHOTO_DATE_COLOR );

    GFL_STR_DeleteBuffer( workStr );
    GFL_STR_DeleteBuffer( srcStr );
    WORDSET_Delete( wordset );
    GFL_MSG_Delete( msgHandle );
  }

  GFL_BMPWIN_TransVramCharacter( work->msgWin );
  GFL_BMPWIN_MakeScreen( work->msgWin );
  GFL_BG_LoadScreenReq( MUS_PHOTO_BG_MSG );
}
