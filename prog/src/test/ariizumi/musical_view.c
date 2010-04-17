//======================================================================
/**
 * @file  dressup_system.h
 * @brief ミュージカルのエディタ
 * @author  ariizumi
 * @data  09/03/02
 */
//======================================================================
#include <gflib.h>
#include <nnsys/mcs.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "gamesystem/game_data.h"
#include "arc_def.h"
#include "font/font.naix"
#include "mus_debug.naix"
#include "message.naix"
#include "msg/msg_mus_item_name.h"

#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

#include "print/printsys.h"
#include "print/str_tool.h"
#include "print/global_msg.h"

#include "musical/musical_system.h"
#include "musical/mus_item_draw.h"
#include "musical/mus_poke_draw.h"
#include "musical/musical_camera_def.h"

#include "test/ariizumi/musical_view.h"
#include "test/ariizumi/mus_mcss_debug.h"
#include "test/ariizumi/ari_debug.h"

#include "debug/gf_mcs.h"


#define HEAPID_MUSICAL_VIEW (HEAPID_MUSICAL_STAGE)
#if USE_MUSICAL_EDIT

//======================================================================
//  define
//======================================================================
#pragma mark [>define
#define MUS_VIEW_PAL_FONT (0x0E)

//======================================================================
//  enum
//======================================================================
#pragma mark [>enum

//======================================================================
//  typedef struct
//======================================================================
#pragma mark [>struct
typedef struct
{
  HEAPID        heapId;

  GFL_G3D_CAMERA    *camera;
  GFL_BBD_SYS     *bbdSys;

  GFL_FONT      *fontHandle;
  PRINT_QUE     *printQue;
  GFL_MSGDATA   *msgHandle;

  MUS_POKE_DRAW_SYSTEM *pokeSys;
  MUS_POKE_DRAW_WORK   *pokeWork;
  MUS_POKE_DRAW_WORK   *pokeWorkBack;
  MUS_ITEM_DRAW_SYSTEM  *itemDrawSys;
  MUS_ITEM_DRAW_WORK  *itemWork[MUS_POKE_EQUIP_MAX];
  GFL_G3D_RES     *itemRes[MUS_POKE_EQUIP_MAX];

  BOOL  isUseMcs;
  BOOL  isRefresh;
  
  u16 monsno;
  u16 sex;
  u16 form;
  u16 rare;

  MUSICAL_POKE_PARAM *musPoke;
  
}MUS_VIEW_LOCAL_WORK;

//======================================================================
//  proto
//======================================================================
#pragma mark [>prot
static void MUSICAL_VIEW_InitGraphic( MUS_VIEW_LOCAL_WORK *work );
static void MUSICAL_VIEW_LoadPoke( MUS_VIEW_LOCAL_WORK *work );
static void MUSICAL_VIEW_UnLoadPoke( MUS_VIEW_LOCAL_WORK *work );
static void MUSICAL_VIEW_UpdateUI( MUS_VIEW_LOCAL_WORK *work );

static GFL_PROC_RESULT MusicalViewProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT MusicalViewProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT MusicalViewProc_Term( GFL_PROC * proc, int * seq , void *pwk, void *mywk );

FS_EXTERN_OVERLAY(musical);
FS_EXTERN_OVERLAY(musical_shot);

GFL_PROC_DATA MusicalView_ProcData =
{
  MusicalViewProc_Init,
  MusicalViewProc_Main,
  MusicalViewProc_Term
};

static const GFL_DISP_VRAM vramBank = {
  GX_VRAM_BG_128_D,       // メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
//  GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
  GX_VRAM_SUB_BG_128_C,     // サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット
  GX_VRAM_OBJ_NONE,       // メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット
  GX_VRAM_SUB_OBJ_NONE,     // サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット
  GX_VRAM_TEX_01_AB,        // テクスチャイメージスロット
  GX_VRAM_TEXPLTT_0123_E,     // テクスチャパレットスロット
  GX_OBJVRAMMODE_CHAR_1D_32K,
  GX_OBJVRAMMODE_CHAR_1D_32K
};

//--------------------------------------------------------------
//  設定周り
//--------------------------------------------------------------
static void MUSICAL_VIEW_InitGraphic( MUS_VIEW_LOCAL_WORK *work )
{
  GX_SetDispSelect(GX_DISP_SELECT_MAIN_SUB);
  GFL_DISP_SetBank( &vramBank );
  GFL_BG_Init( work->heapId );
  GFL_BMPWIN_Init( work->heapId );

  {
    static const GFL_BG_SYS_HEADER sys_data = {
        GX_DISPMODE_GRAPHICS, GX_BGMODE_3, GX_BGMODE_0, GX_BG0_AS_3D,
    };

    GFL_BG_SetBGMode( &sys_data );
    GFL_BG_SetBGControl3D( 1 );
  }
  
  { //3D系の設定
    static const VecFx32 cam_pos = MUSICAL_CAMERA_POS;
    static const VecFx32 cam_target = MUSICAL_CAMERA_TRG;
    static const VecFx32 cam_up = MUSICAL_CAMERA_UP;
    //エッジマーキングカラー
    static  const GXRgb stage_edge_color_table[8]=
      { GX_RGB( 0, 0, 0 ), GX_RGB( 0, 0, 0 ), 0, 0, 0, 0, 0, 0 };
    GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX256K, GFL_G3D_VMANLNK, GFL_G3D_PLT64K,
            0, work->heapId, NULL );
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
      0, //影用の
			GFL_BBD_ORIGIN_CENTER,
    };
    VecFx32 scale ={FX32_ONE*4,FX32_ONE*4,FX32_ONE};
    //ビルボードシステム構築
    work->bbdSys = GFL_BBD_CreateSys( &bbdSetup , work->heapId );
    //背景のために全体を４倍する
    GFL_BBD_SetScale( work->bbdSys , &scale );
  }
  
  //フォント読み込み
  work->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_small_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , work->heapId );
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_SUB_BG , MUS_VIEW_PAL_FONT*0x20, 16*2, work->heapId );

  //システム
  work->pokeSys = MUS_POKE_DRAW_InitSystem( work->heapId );
  work->itemDrawSys = MUS_ITEM_DRAW_InitSystem( work->bbdSys , MUS_POKE_EQUIP_MAX , work->heapId );
}

static void MUSICAL_VIEW_LoadPoke( MUS_VIEW_LOCAL_WORK *work )
{

  u8 ePos;  //equipPos
  MUSICAL_POKE_PARAM *musPoke = work->musPoke;
  VecFx32 pos1 = {MUSICAL_POS_X( 64.0f ),MUSICAL_POS_Y( 128.0f ),FX32_CONST(40.0f)};
  VecFx32 pos2 = {MUSICAL_POS_X(192.0f ),MUSICAL_POS_Y( 128.0f ),FX32_CONST(40.0f)};
  if( work->pokeWork == NULL )
  {
    work->pokeWork = MUS_POKE_DRAW_Add( work->pokeSys , musPoke , TRUE );
    MUS_POKE_DRAW_SetPosition( work->pokeWork , &pos1 );
    MUS_POKE_DRAW_SetShowFlg( work->pokeWork , TRUE );
  }
  if( work->pokeWorkBack == NULL )
  {
    work->pokeWorkBack = MUS_POKE_DRAW_Add( work->pokeSys , musPoke , TRUE );
    MUS_POKE_DRAW_SetPosition( work->pokeWorkBack , &pos2 );
    MUS_POKE_DRAW_SetShowFlg( work->pokeWorkBack , TRUE );
    MUS_POKE_DRAW_SetFrontBack( work->pokeWorkBack , FALSE );
  }

  for( ePos=0;ePos<MUS_POKE_EQUIP_MAX;ePos++ )
  {
    const u16 itemNo = musPoke->equip[ePos].itemNo;
    if( itemNo != MUSICAL_ITEM_INVALID )
    {
      work->itemRes[ePos] = MUS_ITEM_DRAW_LoadResource( itemNo );
      work->itemWork[ePos] = MUS_ITEM_DRAW_AddResource( work->itemDrawSys , itemNo , work->itemRes[ePos] , &pos1 );
      MUS_ITEM_DRAW_SetSize( work->itemDrawSys , work->itemWork[ePos] , FX16_CONST(0.25f) , FX16_CONST(0.25f) );
    }
    else
    {
      work->itemRes[ePos] = NULL;
      work->itemWork[ePos] = NULL;
    }
  }
  MUS_POKE_DRAW_StartAnime( work->pokeWork );
  MUS_POKE_DRAW_StartAnime( work->pokeWorkBack );

}

static void MUSICAL_VIEW_UnLoadPoke( MUS_VIEW_LOCAL_WORK *work )
{
  u8 i;
  if( work->pokeWork != NULL )
  {
    MUS_POKE_DRAW_Del( work->pokeSys , work->pokeWork );
    work->pokeWork = NULL;
  }
  if( work->pokeWorkBack != NULL )
  {
    MUS_POKE_DRAW_Del( work->pokeSys , work->pokeWorkBack );
    work->pokeWorkBack = NULL;
  }
  for( i=0;i<MUS_POKE_EQUIP_MAX;i++ )
  {
    if( work->itemRes[i] != NULL )
    {
      MUS_ITEM_DRAW_DelItem( work->itemDrawSys , work->itemWork[i] );
      MUS_ITEM_DRAW_DeleteResource( work->itemRes[i] );
    }
  }
}

static void MUSICAL_VIEW_UpdateUI( MUS_VIEW_LOCAL_WORK *work )
{
  if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
  {
    work->monsno++;
    work->isRefresh = TRUE;
  }
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    work->isRefresh = TRUE;
  }
  
  if( work->isUseMcs == TRUE )
  {
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    {
//      void* buf = MUS_MCSS_DebugLoadFile( 0 , 0 , work->heapId );
//      GFL_HEAP_FreeMemory( buf );
    }
  }
}

//--------------------------------------------------------------
//  proc 周り
//--------------------------------------------------------------
static GFL_PROC_RESULT MusicalViewProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  MUS_VIEW_LOCAL_WORK *work;
  
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MUSICAL_VIEW, 0x80000 );
  GFL_OVERLAY_Load(FS_OVERLAY_ID(musical_shot));
  GFL_OVERLAY_Load(FS_OVERLAY_ID(musical));

  work = GFL_PROC_AllocWork( proc, sizeof(MUS_VIEW_LOCAL_WORK), HEAPID_MUSICAL_VIEW );

  work->heapId = HEAPID_MUSICAL_VIEW;
  work->isRefresh = FALSE;
  work->monsno = 1;
  work->sex = 0;
  work->form = 0;
  work->rare = 0;
  
  work->musPoke = MUSICAL_SYSTEM_InitMusPokeParam( work->monsno , work->sex , work->form , work->rare , work->heapId );
  work->pokeWork = NULL;
  work->pokeWorkBack = NULL;
  
  work->isUseMcs = FALSE;

  MUSICAL_VIEW_InitGraphic( work );
  if( OS_GetConsoleType() & (OS_CONSOLE_ISDEBUGGER|OS_CONSOLE_TWLDEBUGGER) )
  {
    if( NNS_McsIsServerConnect() == TRUE )
    {
      ARI_TPrintf("Musical View MCS is active!!\n");
      work->isUseMcs = TRUE;
      MUS_MCSS_SetDebugLoadFlg( MUS_POKE_DRAW_GetMcssSys(work->pokeSys) , TRUE );
    }
  }

  MUSICAL_VIEW_LoadPoke( work );
  
  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT MusicalViewProc_Term( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  MUS_VIEW_LOCAL_WORK *work = mywk;


  //終了は考慮しない
  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT MusicalViewProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  MUS_VIEW_LOCAL_WORK *work = mywk;

  MUSICAL_VIEW_UpdateUI( work );
  
  if( work->isRefresh == TRUE )
  {
    GFL_HEAP_FreeMemory(work->musPoke);
    work->musPoke = MUSICAL_SYSTEM_InitMusPokeParam( work->monsno , work->sex , work->form , work->rare , work->heapId );
    
    MUSICAL_VIEW_UnLoadPoke( work );
    MUSICAL_VIEW_LoadPoke( work );
    work->isRefresh = FALSE;
  }


  MUS_POKE_DRAW_UpdateSystem( work->pokeSys );
  MUS_ITEM_DRAW_UpdateSystem( work->itemDrawSys ); 
  
  {
    //3D描画  
    GFL_G3D_DRAW_Start();
    GFL_G3D_DRAW_SetLookAt();
    {
      MUS_POKE_DRAW_DrawSystem( work->pokeSys ); 
      GFL_BBD_Draw( work->bbdSys , work->camera , NULL );
      /*
      if( work->isDrawEPos == TRUE )
      {
        MusicalSetting_DrawEquipPos( work );
      }
      */
    }
    GFL_G3D_DRAW_End();
  }
  return GFL_PROC_RES_CONTINUE;
}

#endif //USE_MUSICAL_EDIT
