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
#include "musical/stage/sta_acting.h"
#include "musical/stage/sta_act_poke.h"

#include "test/ariizumi/musical_view.h"
#include "test/ariizumi/mus_mcss_debug.h"
#include "test/ariizumi/ari_debug.h"

#include "debug/gf_mcs.h"


#define HEAPID_MUSICAL_VIEW (HEAPID_MUSICAL_STAGE)
#if USE_MUSICAL_EDIT

//デバッグ用マクロ
#if 1
#define MUS_VIEW_TPrintf(...) (void)((OS_TFPrintf(3,__VA_ARGS__)))
#else
#define MUS_VIEW_TPrintf(...) ((void)0)
#endif //DEB_ARI

//======================================================================
//  define
//======================================================================
#pragma mark [>define
#define MUS_VIEW_PAL_FONT (0x0E)

#define MUS_VIEW_BG_MAIN (GFL_BG_FRAME1_M)
#define MUS_VIEW_BG_SUB  (GFL_BG_FRAME1_S)
#define MUS_VIEW_BG_STR  (GFL_BG_FRAME2_S)

#define MUS_VIEW_TEST_KEY_CONT (PAD_BUTTON_L|PAD_BUTTON_R)
#define MUS_VIEW_TEST_KEY_TRG (PAD_BUTTON_X)
#define MUS_VIEW_TEST_KEY_TRG_Q (PAD_BUTTON_Y)
#define MUS_VIEW_TEST_TIME (5*60)
#define MUS_VIEW_TEST_TIME_QUICK (5)
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
  GFL_BMPWIN    *strWin;

  MUS_POKE_DRAW_SYSTEM *pokeDrawSys;
  
  STA_POKE_SYS  *pokeSys;
  STA_POKE_WORK *pokeWork;
  STA_POKE_WORK *pokeWorkBack;


  MUS_ITEM_DRAW_SYSTEM  *itemDrawSys;

  BOOL  updateWin;
  BOOL  isUseMcs;
  BOOL  isRefresh;
  BOOL  isRefreshName;
  BOOL  isRefreshItem;
  u16   isDispBit;
  
  u16 monsno;
  u16 sex;
  u16 form;
  u16 rare;
  
  u8 bgColIdx;
  BOOL  isAnime;

  MUSICAL_POKE_PARAM *musPoke;
  u16 rot;
  
  //AutoTest関係
  BOOL isTest;
  BOOL isQuickTest;
  u16  startNo;
  u16  startForm;
  u16  cnt;
  u8   posState[MUS_POKE_EQUIP_MAX];
  u8   posStateRot[2];
  u8   posStateShadow[2];
  
  fx32 polyOfs;
  u8   polyCol;
  
}MUS_VIEW_LOCAL_WORK;

//======================================================================
//  proto
//======================================================================
#pragma mark [>prot
static void MUSICAL_VIEW_InitGraphic( MUS_VIEW_LOCAL_WORK *work );
static void MUSICAL_VIEW_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode );
static void MUSICAL_VIEW_LoadPoke( MUS_VIEW_LOCAL_WORK *work );
static void MUSICAL_VIEW_UnLoadPoke( MUS_VIEW_LOCAL_WORK *work );
static void MUSICAL_VIEW_LoadItem( MUS_VIEW_LOCAL_WORK *work );
static void MUSICAL_VIEW_UnLoadItem( MUS_VIEW_LOCAL_WORK *work );
static void MUSICAL_VIEW_UpdateUI( MUS_VIEW_LOCAL_WORK *work );
static void MUSICAL_VIEW_UpdateAutoTest( MUS_VIEW_LOCAL_WORK *work );
static void MUSICAL_VIEW_UpdatePoke( MUS_VIEW_LOCAL_WORK *work );
static void MUSICAL_VIEW_ChangeEquip( MUS_VIEW_LOCAL_WORK *work , const u8 no );
static void MUSICAL_VIEW_UpdateNumberFunc( const s32 num , const u8 digit , const u8 x , const u8 y );
static void MUSICAL_VIEW_DrawEquipPos( MUS_VIEW_LOCAL_WORK *work );

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

    static const GFL_BG_BGCNT_HEADER bgHeader = {
      0, 0, 0x800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x08000,0x08000,
      GX_BG_EXTPLTT_23, 3, 1, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    static const GFL_BG_BGCNT_HEADER bgHeader2 = {
      0, 0, 0x800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x10000,0x08000,
      GX_BG_EXTPLTT_23, 2, 1, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    
    GFL_BG_SetBGMode( &sys_data );
    GFL_BG_SetBGControl3D( 0 );

    MUSICAL_VIEW_SetupBgFunc( &bgHeader , MUS_VIEW_BG_MAIN , GFL_BG_MODE_TEXT );
    MUSICAL_VIEW_SetupBgFunc( &bgHeader , MUS_VIEW_BG_SUB  , GFL_BG_MODE_TEXT );
    MUSICAL_VIEW_SetupBgFunc( &bgHeader2, MUS_VIEW_BG_STR  , GFL_BG_MODE_TEXT );
    
    //BG
    GFL_ARC_UTIL_TransVramPalette( 0 , NARC_mus_debug_musical_editor_NCLR , 
                      PALTYPE_SUB_BG , 0 , sizeof(u16)*16*2 , work->heapId );
    GFL_ARC_UTIL_TransVramBgCharacter( 0 , NARC_mus_debug_musical_editor_NCGR ,
                      MUS_VIEW_BG_SUB , 0 , 0, FALSE , work->heapId );
    GFL_ARC_UTIL_TransVramScreen( 0 , NARC_mus_debug_musical_view_NSCR , 
                      MUS_VIEW_BG_SUB ,  0 , 0, FALSE , work->heapId );
    GFL_ARC_UTIL_TransVramPalette( 0 , NARC_mus_debug_musical_editor_NCLR , 
                      PALTYPE_MAIN_BG , 0 , sizeof(u16)*16*2 , work->heapId );
    GFL_ARC_UTIL_TransVramBgCharacter( 0 , NARC_mus_debug_musical_editor2_NCGR ,
                      MUS_VIEW_BG_MAIN , 0 , 0, FALSE , work->heapId );
    GFL_ARC_UTIL_TransVramScreen( 0 , NARC_mus_debug_musical_view2_NSCR , 
                      MUS_VIEW_BG_MAIN ,  0 , 0, FALSE , work->heapId );

  }
  
  { //3D系の設定
    static const VecFx32 cam_pos = MUSICAL_CAMERA_POS;
    static const VecFx32 cam_target = MUSICAL_CAMERA_TRG;
    static const VecFx32 cam_up = MUSICAL_CAMERA_UP;
    //エッジマーキングカラー
    static  const GXRgb stage_edge_color_table[8]=
      { GX_RGB( 0, 0, 0 ), GX_RGB( 0, 0, 0 ), 0, 0, 0, 0, 0, 0 };
    GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX256K, GFL_G3D_VMANLNK, GFL_G3D_PLT32K,
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
    G3X_EdgeMarking( FALSE );
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
  
  //フォント読み込み(デバッグでSmallフォント
  work->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_small_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , work->heapId );
  GFL_FONTSYS_SetColor( 1 , 2 , 0 );
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_SUB_BG , MUS_VIEW_PAL_FONT*0x20, 16*2, work->heapId );
  
  work->strWin = GFL_BMPWIN_Create( MUS_VIEW_BG_STR , 1 , 3 , 9 , 3 , MUS_VIEW_PAL_FONT , GFL_BMP_CHRAREA_GET_B );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->strWin ) , 0 );
  GFL_BMPWIN_TransVramCharacter( work->strWin );
  GFL_BMPWIN_MakeScreen( work->strWin );
  GFL_BG_LoadScreenV_Req( MUS_VIEW_BG_STR );  
  work->isRefreshName = TRUE;

  //システム
  work->pokeDrawSys = MUS_POKE_DRAW_InitSystem( work->heapId );
  work->itemDrawSys = MUS_ITEM_DRAW_InitSystem( work->bbdSys , MUS_POKE_EQUIP_MAX*2 , work->heapId );
  work->pokeSys = STA_POKE_InitSystem( work->heapId , NULL , work->pokeDrawSys , work->itemDrawSys , work->bbdSys );
}

//--------------------------------------------------------------------------
//  Bg初期化 機能部
//--------------------------------------------------------------------------
static void MUSICAL_VIEW_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode )
{
  GFL_BG_SetBGControl( bgPlane, bgCont, mode );
  GFL_BG_SetVisible( bgPlane, VISIBLE_ON );
  GFL_BG_ClearFrame( bgPlane );
  GFL_BG_LoadScreenReq( bgPlane );
}

static void MUSICAL_VIEW_LoadPoke( MUS_VIEW_LOCAL_WORK *work )
{

  MUSICAL_POKE_PARAM *musPoke = work->musPoke;
  VecFx32 pos1 = {FX32_CONST( 64.0f ),FX32_CONST( 128.0f ),FX32_CONST(40.0f)};
  VecFx32 pos2 = {FX32_CONST(192.0f ),FX32_CONST( 128.0f ),FX32_CONST(40.0f)};
  work->rot = 0;
  if( work->pokeWork == NULL )
  {
    work->pokeWork = STA_POKE_CreatePoke( work->pokeSys , work->musPoke );
    STA_POKE_SetPosition( work->pokeSys , work->pokeWork , &pos1 );
    STA_POKE_SetDrawItem( work->pokeSys , work->pokeWork , TRUE );
  }
  if( work->pokeWorkBack == NULL )
  {
    work->pokeWorkBack = STA_POKE_CreatePoke( work->pokeSys , work->musPoke );
    STA_POKE_SetPosition( work->pokeSys , work->pokeWorkBack , &pos2 );

    STA_POKE_SetFrontBack( work->pokeSys , work->pokeWorkBack , FALSE );
    STA_POKE_SetDrawItem( work->pokeSys , work->pokeWorkBack , TRUE );
  }


  if( work->isAnime == TRUE )
  {
    STA_POKE_StartAnime( work->pokeSys , work->pokeWork );
    STA_POKE_StartAnime( work->pokeSys , work->pokeWorkBack );
  }
  else
  {
    STA_POKE_StopAnime( work->pokeSys , work->pokeWork );
    STA_POKE_StopAnime( work->pokeSys , work->pokeWorkBack );
  }
  
}

static void MUSICAL_VIEW_UnLoadPoke( MUS_VIEW_LOCAL_WORK *work )
{
  if( work->pokeWork != NULL )
  {
    STA_POKE_DeletePoke( work->pokeSys , work->pokeWork );
    work->pokeWork = NULL;
  }
  if( work->pokeWorkBack != NULL )
  {
    STA_POKE_DeletePoke( work->pokeSys , work->pokeWorkBack );
    work->pokeWorkBack = NULL;
  }
}

static void MUSICAL_VIEW_LoadItem( MUS_VIEW_LOCAL_WORK *work )
{
}

static void MUSICAL_VIEW_UnLoadItem( MUS_VIEW_LOCAL_WORK *work )
{
}

static void MUSICAL_VIEW_UpdateUI( MUS_VIEW_LOCAL_WORK *work )
{
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
  {
    if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP )
    {
      work->polyOfs += FX32_CONST(0.1f);
      ARI_Printf( "PolyOfs[%.1f]\n",FX_FX32_TO_F32(work->polyOfs));
    }
    if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN )
    {
      work->polyOfs -= FX32_CONST(0.1f);
      ARI_Printf( "PolyOfs[%.1f]\n",FX_FX32_TO_F32(work->polyOfs));
    }
  }
  else
  {
    if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )
    {
      work->monsno--;
      work->isRefresh = TRUE;
      work->isRefreshName = TRUE;
    }
    if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
    {
      work->monsno++;
      work->isRefresh = TRUE;
      work->isRefreshName = TRUE;
    }
  }
  if( GFL_UI_KEY_GetCont() & PAD_KEY_LEFT )
  {
    work->rot -= 0x100;
    STA_POKE_SetRotate( work->pokeSys , work->pokeWork , work->rot );
    STA_POKE_SetRotate( work->pokeSys , work->pokeWorkBack , work->rot );
  }
  if( GFL_UI_KEY_GetCont() & PAD_KEY_RIGHT )
  {
    work->rot += 0x100;
    STA_POKE_SetRotate( work->pokeSys , work->pokeWork , work->rot );
    STA_POKE_SetRotate( work->pokeSys , work->pokeWorkBack , work->rot );
  }
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    work->isRefresh = TRUE;
  }
  
  {
    static const GFL_UI_TP_HITTBL hitTbl[] =
    {
      //上矢印
      {  8 , 24 ,  8 , 24 },
      {  8 , 24 , 24 , 40 },
      {  8 , 24 , 40 , 56 },
      //下矢印
      { 48 , 64 ,  8 , 24 },
      { 48 , 64 , 24 , 40 },
      { 48 , 64 , 40 , 56 },
      //フォルム変更
      {  8 , 24 , 64 , 80 },
      { 48 , 64 , 64 , 80 },
      //特殊ボタン
      { 72 , 88 ,  8 , 24 },
      { 72 , 88 , 32 , 48 },
      { 72 , 88 , 56 , 72 },
      { 72 , 88 , 80 , 96 },
      //装備変更
      {  8 , 24 ,168 ,186 },
      {  8 , 24 ,186 ,202 },
      {  8 , 24 ,202 ,218 },
      {  8 , 24 ,218 ,234 },
      {  8 , 24 ,234 ,250 },
      //特殊ボタン(追加分
      { 72 , 88 , 104 , 120 },
      { 72 , 88 , 128 , 144 },
      { 72 , 88 , 152 , 168 },
      { 72 , 88 , 176 , 192 },

      { GFL_UI_TP_HIT_END ,0,0,0},
    };
    const int ret = GFL_UI_TP_HitTrg( hitTbl );
    
    switch( ret )
    {
    case 0:case 1:case 2:case 3:case 4:case 5:
      {
        //monsno操作
        const int arr[6] = {-100,-10,-1,100,10,1};
        const int val = arr[ret];
        if( work->monsno + val <= 0 )
        {
          work->monsno = MONSNO_MAX;
        }
        else
        if( work->monsno + val >= MONSNO_MAX )
        {
          work->monsno = 1;
        }
        else
        {
          work->monsno += val;
        }
      }
      work->isRefreshName = TRUE;
      break;
    case 6:
      if( work->form == 0 )
      {
        work->form = 28;
      }
      else
      {
        work->form--;
      }
      work->isRefreshName = TRUE;
      break;
    case 7:
      if( work->form == 28 )
      {
        work->form = 0;
      }
      else
      {
        work->form++;
      }
      work->isRefreshName = TRUE;
      break;

    case 8:
      work->sex = (work->sex==0?1:0);
      work->isRefreshName = TRUE;
      break;
    case 9:
      work->rare = (work->rare==0?1:0);
      work->isRefreshName = TRUE;
      break;
    case 10:
      work->isRefresh = TRUE;
      break;
    case 11:
      if( work->isDispBit != 0 )
      {
        work->isDispBit = 0;
      }
      else
      {
        work->isDispBit = 0xFFFF;
      }
      break;

    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
      MUSICAL_VIEW_ChangeEquip( work , ret-12);
      work->isRefreshItem = TRUE;
      break;

    case 17:
      {
        STA_POKE_DIR dir = STA_POKE_GetPokeDir( work->pokeSys , work->pokeWork );
        STA_POKE_SetPokeDir( work->pokeSys , work->pokeWork , !dir );
        STA_POKE_SetPokeDir( work->pokeSys , work->pokeWorkBack , !dir );
      }
      break;

    case 18:
      {
        const u16 bgColArr[] = {0x0000,0x7fff,0x7c1f,0x03e0};
        work->bgColIdx++;
        if( work->bgColIdx >= 4 )
        {
          work->bgColIdx = 0;
        }
        *((u16 *)HW_BG_PLTT) = bgColArr[work->bgColIdx];
        
      }
      
      break;

    case 19:
      {
        if( work->isAnime == FALSE )
        {
          work->isAnime = TRUE;
          STA_POKE_StartAnime( work->pokeSys , work->pokeWork );
          STA_POKE_StartAnime( work->pokeSys , work->pokeWorkBack );
        }
        else
        {
          work->isAnime = FALSE;
          STA_POKE_StopAnime( work->pokeSys , work->pokeWork );
          STA_POKE_StopAnime( work->pokeSys , work->pokeWorkBack );
        }
      }
      break;
    case 20:
      {
        MUS_POKE_DRAW_SYSTEM *musPokeSys = STA_POKE_GetMusPokeSys( work->pokeSys );
        MUS_POKE_DRAW_WORK *musPokeWork = STA_POKE_GetMusPokeWork( work->pokeSys ,work->pokeWork );
        MUS_POKE_DRAW_WORK *musPokeWorkBack = STA_POKE_GetMusPokeWork( work->pokeSys ,work->pokeWorkBack );
        MUS_POKE_StepAnime( musPokeSys , musPokeWork , FX32_ONE );
        MUS_POKE_StepAnime( musPokeSys , musPokeWorkBack , FX32_ONE );
      }
      break;
    }
  }
  if( (GFL_UI_KEY_GetTrg() & MUS_VIEW_TEST_KEY_TRG) &&
      (GFL_UI_KEY_GetCont() & MUS_VIEW_TEST_KEY_CONT) )
  {
    u8 i;
    work->isRefresh = TRUE;
    work->isRefreshName = TRUE;
    work->isTest = TRUE;
    work->isQuickTest = FALSE;
    work->startNo = work->monsno;
    work->startForm = work->form;
    work->cnt = 0;
    for( i=0;i<MUS_POKE_EQUIP_MAX;i++ )
    {
      work->posState[i] = 0;
    }
    work->posStateRot[0] = 0;
    work->posStateRot[1] = 0;
    work->posStateShadow[0] = 0;
    work->posStateShadow[1] = 0;
    MUS_VIEW_TPrintf( "---Start Musical view auto test!---\n" );
  }
  if( (GFL_UI_KEY_GetTrg() & MUS_VIEW_TEST_KEY_TRG_Q) &&
      (GFL_UI_KEY_GetCont() & MUS_VIEW_TEST_KEY_CONT) )
  {
    u8 i;
    work->isRefresh = TRUE;
    work->isRefreshName = TRUE;
    work->isTest = TRUE;
    work->isQuickTest = TRUE;
    work->startNo = work->monsno;
    work->startForm = work->form;
    work->cnt = 0;
    for( i=0;i<MUS_POKE_EQUIP_MAX;i++ )
    {
      work->posState[i] = 0;
    }
    work->posStateRot[0] = 0;
    work->posStateRot[1] = 0;
    work->posStateShadow[0] = 0;
    work->posStateShadow[1] = 0;
    MUS_VIEW_TPrintf( "---Start Musical view auto quick test!---\n" );
  }

}

static void MUSICAL_VIEW_UpdateAutoTest( MUS_VIEW_LOCAL_WORK *work )
{
  MUS_POKE_DRAW_WORK *musPokeWork = STA_POKE_GetMusPokeWork( work->pokeSys , work->pokeWork );
  MUS_POKE_DRAW_WORK *musPokeWorkBack = STA_POKE_GetMusPokeWork( work->pokeSys , work->pokeWorkBack );
  work->cnt++;

  //座標チェック
  {
    u8 i;
    {
      BOOL *flg = MUS_POKE_DRAW_GetEnableRotateOfs(musPokeWork);
      if( *flg == TRUE )
      {
        if( work->posStateRot[0] == 0 )
        {
          work->posStateRot[0] = 1;
        }
      }
      else
      {
        if( work->posStateRot[0] != 0 )
        {
          work->posStateRot[0] = 2;
        }
      }
      *flg = FALSE;
    }
    {
      BOOL *flg = MUS_POKE_DRAW_GetEnableShadowOfs(musPokeWork);
      if( *flg == TRUE )
      {
        if( work->posStateShadow[0] == 0 )
        {
          work->posStateShadow[0] = 1;
        }
      }
      else
      {
        if( work->posStateShadow[0] != 0 )
        {
          work->posStateShadow[0] = 2;
        }
      }
      *flg = FALSE;
    }
    {
      BOOL *flg = MUS_POKE_DRAW_GetEnableRotateOfs(musPokeWorkBack);
      if( *flg == TRUE )
      {
        if( work->posStateRot[1] == 0 )
        {
          work->posStateRot[1] = 1;
        }
      }
      else
      {
        if( work->posStateRot[1] != 0 )
        {
          work->posStateRot[1] = 2;
        }
      }
      *flg = FALSE;
    }
    {
      BOOL *flg = MUS_POKE_DRAW_GetEnableShadowOfs(musPokeWorkBack);
      if( *flg == TRUE )
      {
        if( work->posStateShadow[1] == 0 )
        {
          work->posStateShadow[1] = 1;
        }
      }
      else
      {
        if( work->posStateShadow[1] != 0 )
        {
          work->posStateShadow[1] = 2;
        }
      }
      *flg = FALSE;
    }

    for( i=0;i<MUS_POKE_EQUIP_MAX;i++ )
    {
      MUS_POKE_EQUIP_DATA *equipData = MUS_POKE_DRAW_GetEquipData( musPokeWork , i );
      if( equipData->isEnable == TRUE )
      {
        if( work->posState[i] == 0 )
        {
          work->posState[i] = 1;
        }
      }
      else
      {
        //座標が一瞬あった
        if( work->posState[i] != 0 )
        {
          work->posState[i] = 2;
        }
      }
    }
  }

  //リロード
  if( work->cnt >= MUS_VIEW_TEST_TIME ||
      (work->cnt >= MUS_VIEW_TEST_TIME_QUICK && work->isQuickTest == TRUE ))
  {
    u8 i;
    char strArr[3][3] ={"×","○","▲"};
    POKEMON_PERSONAL_DATA *perData = POKE_PERSONAL_OpenHandle( work->monsno, 0, work->heapId );
    const u8 formNum = POKE_PERSONAL_GetParam( perData , POKEPER_ID_form_max );
    POKE_PERSONAL_CloseHandle( perData );
    
    //座標表示
    MUS_VIEW_TPrintf( "[%3d][%2d]",work->monsno,work->form );
    MUS_VIEW_TPrintf( "[%2s]",strArr[work->posStateRot[0]] );
    MUS_VIEW_TPrintf( "[%2s]|",strArr[work->posStateShadow[0]] );
    for( i=0;i<MUS_POKE_EQUIP_MAX;i++ )
    {
      MUS_VIEW_TPrintf( "[%2s]",strArr[work->posState[i]] );
    }
    MUS_VIEW_TPrintf( "|[%2s]",strArr[work->posStateRot[1]] );
    MUS_VIEW_TPrintf( "[%2s]",strArr[work->posStateShadow[1]] );
    MUS_VIEW_TPrintf( "\n" );

    if( work->form == formNum-1 )
    {
      work->form = 0;
      if( work->monsno == MONSNO_MAX-2 )  //たまごもあるから-2
      {
        work->monsno = 1;
      }
      else
      {
        work->monsno++;
      }
    }
    else
    {
      work->form++;
    }
    work->isRefresh = TRUE;
    work->isRefreshName = TRUE;
    work->cnt = 0;

    for( i=0;i<MUS_POKE_EQUIP_MAX;i++ )
    {
      work->posState[i] = 0;
    }
    work->posStateRot[0] = 0;
    work->posStateRot[1] = 0;
    work->posStateShadow[0] = 0;
    work->posStateShadow[1] = 0;
    if( work->startNo == work->monsno &&
        work->startForm == work->form )
    {
      work->isTest = FALSE;
      MUS_VIEW_TPrintf( "---Finish Musical view auto test!---\n" );
    }
  }

  if( (GFL_UI_KEY_GetTrg() & MUS_VIEW_TEST_KEY_TRG) &&
      (GFL_UI_KEY_GetCont() & MUS_VIEW_TEST_KEY_CONT) )
  {
    MUS_VIEW_TPrintf( "---Cancel Musical view auto test!---\n" );
    work->isTest = FALSE;
  }
}

static void MUSICAL_VIEW_UpdatePoke( MUS_VIEW_LOCAL_WORK *work )
{
  int ePos;
  VecFx32 pos;
  MUS_POKE_DRAW_WORK *musPokeWork = STA_POKE_GetMusPokeWork( work->pokeSys , work->pokeWork );
  MUS_POKE_DRAW_WORK *musPokeWorkBack = STA_POKE_GetMusPokeWork( work->pokeSys , work->pokeWorkBack );

  for( ePos=0;ePos<MUS_POKE_EQUIP_MAX;ePos++ )
  {
    MUS_POKE_EQUIP_DATA *equipData = MUS_POKE_DRAW_GetEquipData( musPokeWork , ePos );
    u8 scrPosX = (ePos%2==0?10:26);
    u8 scrPosY = 12 + (ePos/2)*2;
    if( equipData->isEnable == TRUE )
    {
      const u16 itemRot = equipData->itemRot;
      const u16 rotZ = 0x10000-equipData->rot;

      MtxFx33 rotWork;
      VecFx32 rotOfs;
      VecFx32 ofs;
      MTX_RotZ33( &rotWork , -FX_SinIdx( rotZ ) , FX_CosIdx( rotZ ) );
      MTX_MultVec33( &equipData->ofs , &rotWork , &ofs );
      
      {
        MTX_MultVec33( &equipData->rotOfs , &rotWork , &rotOfs );
        VEC_Subtract( &equipData->rotOfs , &rotOfs , &rotOfs );
      }

      MUSICAL_VIEW_UpdateNumberFunc( F32_CONST(equipData->pos.x+ofs.x+rotOfs.x) , 3 ,scrPosX   , scrPosY );
      MUSICAL_VIEW_UpdateNumberFunc( F32_CONST(equipData->pos.y+ofs.y+rotOfs.y) , 3 ,scrPosX+4 , scrPosY );
    }
    else
    {
      u16 chrNo[3] = {0x0e,0x0f,0x10};
      GFL_BG_WriteScreen( MUS_VIEW_BG_SUB , &chrNo , scrPosX-2,scrPosY,3,1);
      GFL_BG_WriteScreen( MUS_VIEW_BG_SUB , &chrNo , scrPosX+2,scrPosY,3,1);
    }
    {
      VecFx32 *shadowOfs = MUS_POKE_DRAW_GetShadowOfs( musPokeWork );
      VecFx32 *rotOfs = MUS_POKE_DRAW_GetRotateOfs( musPokeWork );
      MUSICAL_VIEW_UpdateNumberFunc( F32_CONST(shadowOfs->x) , 3 ,10 , 22 );
      MUSICAL_VIEW_UpdateNumberFunc( F32_CONST(shadowOfs->y) , 3 ,14 , 22 );
      MUSICAL_VIEW_UpdateNumberFunc( F32_CONST(rotOfs->x) , 3 ,26 , 22 );
      MUSICAL_VIEW_UpdateNumberFunc( F32_CONST(rotOfs->y) , 3 ,30 , 22 );
    }
    
    //デバッグ用に一回見たらFALSEに
    equipData->isEnable = FALSE;
  }

  GFL_BG_LoadScreenV_Req( MUS_VIEW_BG_SUB );

  {
    VecFx32 pos = {MUSICAL_POS_X( 64.0f ),MUSICAL_POS_Y( 128.0f ),FX32_CONST(40.0f)};

    if( !(GFL_UI_KEY_GetCont() & PAD_BUTTON_R) )
    {
      VecFx32 *shadowOfs; //影差分
      shadowOfs = MUS_POKE_DRAW_GetShadowOfs( musPokeWork );
      //影の座標を基準に
      pos.x = MUSICAL_POS_X_FX( FX32_CONST(64.0f)-shadowOfs->x);
      pos.y = MUSICAL_POS_Y_FX( FX32_CONST(128.0f)-shadowOfs->y);
    }
    
    MUS_POKE_DRAW_SetPosition( musPokeWork , &pos);
  }
  {
    VecFx32 pos = {MUSICAL_POS_X(192.0f ),MUSICAL_POS_Y( 128.0f ),FX32_CONST(40.0f)};
    if( !(GFL_UI_KEY_GetCont() & PAD_BUTTON_R) )
    {
      VecFx32 *shadowOfs; //影差分
      shadowOfs = MUS_POKE_DRAW_GetShadowOfs( musPokeWorkBack );
      //影の座標を基準に
      pos.x = MUSICAL_POS_X_FX( FX32_CONST(192.0f)-shadowOfs->x);
      pos.y = MUSICAL_POS_Y_FX( FX32_CONST(128.0f)-shadowOfs->y);
    }
    MUS_POKE_DRAW_SetPosition( musPokeWorkBack , &pos);
  }

  if( work->isRefresh == TRUE )
  {
    work->musPoke->mcssParam.monsno = work->monsno;
    work->musPoke->mcssParam.sex    = work->sex;
    work->musPoke->mcssParam.form = work->form;
    work->musPoke->mcssParam.rare   = work->rare;
    
    MUSICAL_VIEW_UnLoadPoke( work );
    MUSICAL_VIEW_LoadPoke( work );
    work->isRefresh = FALSE;
    work->isRefreshItem = TRUE;
  }
  if( work->isRefreshItem == TRUE )
  {
    MUSICAL_VIEW_UnLoadItem( work );
    MUSICAL_VIEW_LoadItem( work );
    work->isRefreshItem = FALSE;
  }
  if( work->isRefreshName == TRUE )
  {
    STRCODE code[4];
    STRBUF *str = GFL_STR_CreateBuffer( 32 , work->heapId );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->strWin ) , 0 );
    
    //名前
    GFL_MSG_GetString( GlobalMsg_PokeName , work->monsno , str );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp(work->strWin) , 0 , 4 , str , work->fontHandle );
    
    //番号
    code[0] = work->monsno/100 + L'0';
    code[1] = (work->monsno%100)/10  + L'0';
    code[2] = (work->monsno%10)   + L'0';
    code[3] = GFL_STR_GetEOMCode();
    GFL_STR_SetStringCode( str , code );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp(work->strWin) , 0 , 12 , str , work->fontHandle );

    //性別
    if( work->sex == 0 )
    {
      code[0] = L'♂';
    }
    else
    {
      code[0] = L'♀';
    }
    code[1] = GFL_STR_GetEOMCode();
    GFL_STR_SetStringCode( str , code );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp(work->strWin) , 24 , 12 , str , work->fontHandle );
    
    //レア
    if( work->rare == 1 )
    {
      code[0] = L'R';
      code[1] = GFL_STR_GetEOMCode();
      GFL_STR_SetStringCode( str , code );
      PRINTSYS_Print( GFL_BMPWIN_GetBmp(work->strWin) , 32 , 12 , str , work->fontHandle );
    }

    //フォルム
    code[0] = work->form/10 + L'0';
    code[1] = (work->form%10)  + L'0';
    code[2] = GFL_STR_GetEOMCode();
    GFL_STR_SetStringCode( str , code );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp(work->strWin) , 56 , 8 , str , work->fontHandle );
    
    GFL_BMPWIN_TransVramCharacter( work->strWin );
    GFL_STR_DeleteBuffer( str );

    work->isRefreshName = FALSE;
  }


}

static void MUSICAL_VIEW_ChangeEquip( MUS_VIEW_LOCAL_WORK *work , const u8 no )
{
  static const u16 itemArr[5][9] =
  {
    {MUSICAL_ITEM_INVALID,MUSICAL_ITEM_INVALID,MUSICAL_ITEM_INVALID,MUSICAL_ITEM_INVALID,MUSICAL_ITEM_INVALID,MUSICAL_ITEM_INVALID,MUSICAL_ITEM_INVALID,MUSICAL_ITEM_INVALID,MUSICAL_ITEM_INVALID},
    {10,16,59,8,MUSICAL_ITEM_INVALID,66,15,63,68},
    {37,0,38,8,MUSICAL_ITEM_INVALID,13,36,26,28},
    {10,97,71,54,MUSICAL_ITEM_INVALID,4,82,32,72},
    {MUSICAL_ITEM_INVALID,MUSICAL_ITEM_INVALID,11,MUSICAL_ITEM_INVALID,29,47,89,MUSICAL_ITEM_INVALID,84},
  };
  u8 ePos;
  
  for( ePos=0;ePos<MUS_POKE_EQUIP_MAX;ePos++ )
  {
    work->musPoke->equip[ePos].itemNo = itemArr[no][ePos];
    work->musPoke->equip[ePos].priority = ePos;
  }

}

static void MUSICAL_VIEW_UpdateNumberFunc( const s32 num , const u8 digit , const u8 x , const u8 y )
{
  const BOOL isMinus = (num<0?TRUE:FALSE);
  u32 tempNum = MATH_ABS(num);
  u16 nowDigit = 0;
  
  while( tempNum != 0 || nowDigit == 0 )
  {
    u16 chrNo = tempNum%10 + 0x02;
    GFL_BG_WriteScreen( MUS_VIEW_BG_SUB , &chrNo , x-nowDigit,y,1,1);
    nowDigit++;
    tempNum/=10;
  }
  if( isMinus == TRUE )
  {
    u16 chrNo = 0x0C;
    GFL_BG_WriteScreen( MUS_VIEW_BG_SUB , &chrNo , x-nowDigit,y,1,1);
    nowDigit++;
  }
  while( nowDigit < digit )
  {
    u16 chrNo = 0x01;
    GFL_BG_WriteScreen( MUS_VIEW_BG_SUB , &chrNo , x-nowDigit,y,1,1);
    nowDigit++;
  }
}

static void MUSICAL_VIEW_DrawEquipPos( MUS_VIEW_LOCAL_WORK *work )
{
  int ePos;
  VecFx32 pos;
  MtxFx33       mtxBillboard;
  VecFx16       vecN;
  MUS_POKE_DRAW_WORK *musPokeWork = STA_POKE_GetMusPokeWork( work->pokeSys , work->pokeWork );
  G3_PushMtx();
  G3_MtxMode( GX_MTXMODE_POSITION_VECTOR );
  //カメラ設定取得
  {
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

  if( work->isDispBit != 0 )
  {
    G3_PolygonAttr(GX_LIGHTMASK_NONE,       // no lights
             GX_POLYGONMODE_MODULATE,     // modulation mode
             GX_CULL_NONE,          // cull back
             63,                // polygon ID(0 - 63)
             0,                 // alpha(0 - 31)
             0                  // OR of GXPolygonAttrMisc's value
             );
    for( ePos=0;ePos<MUS_POKE_EQUIP_MAX;ePos++ )
    {
      if( work->isDispBit & (1<<ePos) )
      {
        MUS_POKE_EQUIP_DATA *equipData = MUS_POKE_DRAW_GetEquipData( musPokeWork , ePos );
        if( equipData->isEnable == TRUE )
        {
          const u16 itemRot = equipData->itemRot;
          const u16 rotZ = 0x10000-equipData->rot;

          MtxFx33 rotWork;
          VecFx32 rotOfs;
          VecFx32 ofs;

          G3_PushMtx();


          MTX_RotZ33( &rotWork , -FX_SinIdx( rotZ ) , FX_CosIdx( rotZ ) );
          MTX_MultVec33( &equipData->ofs , &rotWork , &ofs );

          //pos.x = MUSICAL_POS_X_FX(equipData->pos.x+ofs.x+FX32_CONST(128.0f) + rotOfs.x);
          //pos.y = MUSICAL_POS_Y_FX(equipData->pos.y+ofs.y+FX32_CONST(96.0f) + rotOfs.y);
          pos.x = MUSICAL_POS_X_FX(equipData->pos.x + ofs.x + FX32_CONST(128.0f));
          pos.y = MUSICAL_POS_Y_FX(equipData->pos.y + ofs.y + FX32_CONST(96.0f));
          pos.z = FX32_CONST(120.0f); //ポケの前に出す

          G3_Translate( pos.x, pos.y, pos.z );
          G3_RotZ( -FX_SinIdx( itemRot-rotZ ), FX_CosIdx( itemRot-rotZ ) );
          G3_Scale( FX32_CONST(0.25f), FX32_CONST(0.25f), pos.z );
          {
            const fx32 size = FX32_ONE;
            G3_Begin(GX_BEGIN_QUADS);
            G3_Color( GX_RGB( 0, 0, 0 ) );
            //十字
            G3_Vtx( -size, 0, 0 );
            G3_Vtx(  size, 0, 0 );
            G3_Vtx(  size, 0, 0 );
            G3_Vtx( -size, 0, 0 );
            G3_Vtx( 0, size, 0 );
            G3_Vtx( 0, size, 0 );
            G3_Vtx( 0,-size, 0 );
            G3_Vtx( 0,-size, 0 );
            G3_End();
          }
          G3_PopMtx(1);
        }
      }
    } 
  }
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
  {
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
    pos.z = FX32_CONST(40.0f) + work->polyOfs;

    G3_Translate( pos.x, pos.y, pos.z );

    {
      const fx32 size = FX32_ONE;
      G3_Begin(GX_BEGIN_QUADS);
      G3_Color( GX_RGB( work->polyCol, work->polyCol, work->polyCol ) );
      work->polyCol++;
      if( work->polyCol > 31 )
      {
        work->polyCol = 0;
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
  work->isRefreshItem = FALSE;
  work->isRefreshName = TRUE;
  work->monsno = 0;
#if defined(DEBUG_ONLY_FOR_ariizumi_nobuhiko)
  work->monsno = 117;
#endif
  work->sex = 0;
  work->form = 0;
  work->rare = 0;
  work->isDispBit = 0;
  work->bgColIdx = 0;
  work->isAnime = TRUE;
  work->polyOfs = 0;
  work->polyCol = 0;
  
  work->musPoke = MUSICAL_SYSTEM_InitMusPokeParam( work->monsno , work->sex , work->form , work->rare , 0 , work->heapId );
  work->pokeWork = NULL;
  work->pokeWorkBack = NULL;
  
  work->isUseMcs = FALSE;
  
  work->isTest = FALSE;
  work->isQuickTest = FALSE;

  MUSICAL_VIEW_InitGraphic( work );
  if( OS_GetConsoleType() & (OS_CONSOLE_ISDEBUGGER|OS_CONSOLE_TWLDEBUGGER) )
  {
    if( NNS_McsIsServerConnect() == TRUE )
    {
      ARI_TPrintf("Musical View MCS is active!!\n");
      work->isUseMcs = TRUE;
      MUS_MCSS_SetDebugLoadFlg( MUS_POKE_DRAW_GetMcssSys(work->pokeDrawSys) , TRUE );
    }
  }

  MUSICAL_VIEW_LoadPoke( work );
  MUSICAL_VIEW_LoadItem( work );
  
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
  
  if( work->isTest == FALSE )
  {
    MUSICAL_VIEW_UpdateUI( work );
  }
  else
  {
    MUSICAL_VIEW_UpdateAutoTest( work );
  }
  MUSICAL_VIEW_UpdatePoke( work );
  
  STA_POKE_UpdateSystem( work->pokeSys );
  MUS_POKE_DRAW_UpdateSystem( work->pokeDrawSys );

  if( work->isTest == FALSE )
  {
    MUS_POKE_DRAW_UpdateSystem( work->pokeDrawSys );
    MUS_POKE_DRAW_UpdateSystem( work->pokeDrawSys );
  }
  MUS_ITEM_DRAW_UpdateSystem( work->itemDrawSys ); 
  
  {
    //3D描画  
    GFL_G3D_DRAW_Start();
    GFL_G3D_DRAW_SetLookAt();
    {
      STA_POKE_DrawSystem( work->pokeSys );
      MUS_POKE_DRAW_DrawSystem( work->pokeDrawSys ); 
      STA_POKE_UpdateSystem_Item( work->pokeSys );  //ポケの描画後に入れること
      GFL_BBD_Draw( work->bbdSys , work->camera , NULL );

      MUSICAL_VIEW_DrawEquipPos( work );
    }
    GFL_G3D_DRAW_End();
  }
  return GFL_PROC_RES_CONTINUE;
}

#endif //USE_MUSICAL_EDIT
