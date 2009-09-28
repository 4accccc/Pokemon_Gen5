//======================================================================
/**
 * @file	mus_comm_lobby.c
 * @brief	ミュージカル通信ロビー
 * @author	ariizumi
 * @data	09/05/29
 *
 * モジュール名：MUS_LOBBY
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"
#include "print/gf_font.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "msg/msg_musical_comm.h"
#include "infowin/infowin.h"

#include "system/bmp_winframe.h"
#include "system/wipe.h"
#include "sound/pm_sndsys.h"

#include "mus_comm_func.h"
#include "mus_comm_lobby.h"
#include "mus_comm_define.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//PLT・FRAME_CHR上下共通
#define MUS_LOBBY_INFO_PLT (0xF)
#define MUS_LOBBY_FONT_PLT (0xE)
#define MUS_LOBBY_FRAME_PLT (0xD)

#define MUS_LOBBY_FRANE_CHR (1)

#define MUS_LOBBY_BG_BMP  ( GFL_BG_FRAME0_S )
#define MUS_LOBBY_BG_INFO ( GFL_BG_FRAME3_S )
#define MUS_LOBBY_BG_STR  ( GFL_BG_FRAME1_M )


//BMPWIN 位置(左位置・幅・高さは共通
#define MUS_LOBBY_BMPWIN_LEFT (8)
#define MUS_LOBBY_BMPWIN_WIDTH (16)
#define MUS_LOBBY_BMPWIN_HEIGHT (2)

//TopMenu
#define MUS_LOBBY_TOPMENU_CHILD  (6)
#define MUS_LOBBY_TOPMENU_PARENT (11)
#define MUS_LOBBY_TOPMENU_CANCEL (20)

//ParentMenu
#define MUS_LOBBY_PARENT_MEMBER_TOP (4)
#define MUS_LOBBY_PARENT_MEMBER_HEIGHT (8)

#define MUS_LOBBY_PARENT_START_TOP (20)

//ChildMenu

#define MUS_LOBBY_SELECT_NONE (0xFF)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

typedef enum
{
  MLS_INIT_TOPMENU,
  MLS_INIT_TOPMENU_WAIT,
  MLS_LOOP_TOPMENU,

  MLS_INIT_PARENT,
  MLS_INIT_PARENT_WAIT,
  MLS_LOOP_PARENT,
  MLS_PARENT_WAIT_START,

  MLS_INIT_CHILD,
  MLS_INIT_CHILD_WAIT,
  MLS_LOOP_CHILD,

  MLS_INIT_FADE_OUT,
  MLS_WAIT_FADE_OUT,
}MUSICAL_LOBBY_STATE;

//このdefineは以下のenumの最大値にすること
#define MUS_LOBBY_ITEM_MAX (3)
//開始時メニュー用
typedef enum
{
  MLBT_START_CHILD,
  MLBT_START_PARENT,
  MLBT_CANCEL,
  MLBT_MAX,
}MUSICAL_LOBBY_BMPWIN_TOP;

typedef enum
{
  MLBP_MEMBER,
  MLBP_START,
  MLBP_MAX,
}MUSICAL_LOBBY_BMPWIN_PARENT;

typedef enum
{
  MLBC_MEMBER,
  MLBC_MAX,
}MUSICAL_LOBBY_BMPWIN_CHILD;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef struct
{
  HEAPID heapId;
  MUS_COMM_WORK *commWork;
  
  BOOL    isRefreshMember;
  u8     selectItem;
  MUSICAL_LOBBY_STATE state;
  
  PRINT_QUE *printQue;
  GFL_FONT *fontHandle;
  GFL_MSGDATA *msgHandle;
  GFL_BMPWIN *winArr[MUS_LOBBY_ITEM_MAX];
  GFL_BMPWIN *strWin; //画面上用
  
}MUS_LOBBY_WORK;


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static GFL_PROC_RESULT MUS_LOBBY_InitProc( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT MUS_LOBBY_ExitProc( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT MUS_LOBBY_MainProc( GFL_PROC * proc, int * seq , void *pwk, void *mywk );

static void MUS_LOBBY_InitGraphic( MUS_LOBBY_WORK *work );
static void MUS_LOBBY_InitBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane );

static void MUS_LOBBY_DrawStringFunc( MUS_LOBBY_WORK *work , GFL_BMPWIN *win , u8 strId , const BOOL isCenter );

static void MUS_LOBBY_InitUpperStr( MUS_LOBBY_WORK *work );
static void MUS_LOBBY_ExitUpperStr( MUS_LOBBY_WORK *work );
static void MUS_LOBBY_SetUpperStr( MUS_LOBBY_WORK *work , u16 strId);

static void MUS_LOBBY_InitTopMenu( MUS_LOBBY_WORK *work );
static void MUS_LOBBY_ExitTopMenu( MUS_LOBBY_WORK *work );
static void MUS_LOBBY_DispTopMenu( MUS_LOBBY_WORK *work );
static void MUS_LOBBY_LoopTopMenu( MUS_LOBBY_WORK *work );

static void MUS_LOBBY_InitParentMenu( MUS_LOBBY_WORK *work );
static void MUS_LOBBY_ExitParentMenu( MUS_LOBBY_WORK *work );
static void MUS_LOBBY_DispParentMenu( MUS_LOBBY_WORK *work );
static void MUS_LOBBY_LoopParentMenu( MUS_LOBBY_WORK *work );
static void MUS_LOBBY_DrawMember( MUS_LOBBY_WORK *work );

static void MUS_LOBBY_InitChildMenu( MUS_LOBBY_WORK *work );
static void MUS_LOBBY_ExitChildMenu( MUS_LOBBY_WORK *work );
static void MUS_LOBBY_DispChildMenu( MUS_LOBBY_WORK *work );
static void MUS_LOBBY_LoopChildMenu( MUS_LOBBY_WORK *work );

GFL_PROC_DATA MUS_LOBBY_ProcData =
{
  MUS_LOBBY_InitProc,
  MUS_LOBBY_MainProc,
  MUS_LOBBY_ExitProc
};

static const GFL_DISP_VRAM vramBank = {
  GX_VRAM_BG_128_A,       // メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
  GX_VRAM_SUB_BG_128_C,     // サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット
  GX_VRAM_OBJ_128_B,        // メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット
  GX_VRAM_SUB_OBJ_128_D,      // サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット
  GX_VRAM_TEX_NONE,       // テクスチャイメージスロット
  GX_VRAM_TEXPLTT_NONE,     // テクスチャパレットスロット
  GX_OBJVRAMMODE_CHAR_1D_128K,   // メインOBJマッピングモード
  GX_OBJVRAMMODE_CHAR_1D_128K,   // サブOBJマッピングモード
};

#pragma mark [>proc 
//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
static GFL_PROC_RESULT MUS_LOBBY_InitProc( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  MUS_LOBBY_WORK *work;
  
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MUSICAL_LOBBY, 0x10000 );

  work = GFL_PROC_AllocWork( proc, sizeof(MUS_LOBBY_WORK), HEAPID_MUSICAL_LOBBY );
  work->heapId = HEAPID_MUSICAL_LOBBY;
  work->state = MLS_INIT_TOPMENU;
  work->commWork = pwk;
  
  MUS_LOBBY_InitGraphic( work );

  INFOWIN_Init( MUS_LOBBY_BG_INFO , MUS_LOBBY_INFO_PLT , MUS_COMM_GetGameComm(work->commWork) , work->heapId );

  work->printQue = PRINTSYS_QUE_Create( work->heapId );

  MUS_LOBBY_InitUpperStr( work );
  
  WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
                  WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , work->heapId );
  
  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
static GFL_PROC_RESULT MUS_LOBBY_ExitProc( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  MUS_LOBBY_WORK *work = mywk;

  GX_SetMasterBrightness(-16);  
  GXS_SetMasterBrightness(-16);
  
  MUS_LOBBY_ExitUpperStr( work );

  PRINTSYS_QUE_Clear( work->printQue );
  PRINTSYS_QUE_Delete( work->printQue );

  INFOWIN_Exit();
  GFL_MSG_Delete( work->msgHandle );
  GFL_FONT_Delete( work->fontHandle );
  GFL_BMPWIN_Exit();
  GFL_BG_FreeBGControl(MUS_LOBBY_BG_BMP);
  GFL_BG_FreeBGControl(MUS_LOBBY_BG_INFO);
  GFL_BG_FreeBGControl(MUS_LOBBY_BG_STR);
  GFL_BG_Exit();

  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_MUSICAL_LOBBY );

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
static GFL_PROC_RESULT MUS_LOBBY_MainProc( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  MUS_LOBBY_WORK *work = mywk;
  
  switch( work->state )
  {
    //Topメニュー
  case MLS_INIT_TOPMENU:
    MUS_LOBBY_InitTopMenu( work );
    work->state = MLS_INIT_TOPMENU_WAIT;
    break;

  case MLS_INIT_TOPMENU_WAIT:
    if( PRINTSYS_QUE_IsFinished( work->printQue ) == TRUE &&
        WIPE_SYS_EndCheck() == TRUE )
    {
      MUS_LOBBY_DispTopMenu( work );
      work->state = MLS_LOOP_TOPMENU;
    }
    break;

  case MLS_LOOP_TOPMENU:
    MUS_LOBBY_LoopTopMenu( work );
    if( work->selectItem != MUS_LOBBY_SELECT_NONE )
    {
      MUS_LOBBY_ExitTopMenu( work );
      switch( work->selectItem )
      {
      case MLBT_START_PARENT:
        work->state = MLS_INIT_PARENT;
        break;

      case MLBT_START_CHILD:
        work->state = MLS_INIT_CHILD;
        break;

      case MLBT_CANCEL:
        work->state = MLS_INIT_FADE_OUT;
        break;
      }
    }
    break;
    
    //親機開始
  case MLS_INIT_PARENT:
    work->state = MLS_INIT_PARENT_WAIT;
    MUS_LOBBY_InitParentMenu( work );
    MUS_COMM_InitComm( work->commWork );
    break;
    
  case MLS_INIT_PARENT_WAIT:
    if( PRINTSYS_QUE_IsFinished( work->printQue ) == TRUE &&
        MUS_COMM_IsInitComm(work->commWork) == TRUE )
    {
      MUS_LOBBY_DispParentMenu( work );
      MUS_COMM_StartParent( work->commWork );
      work->state = MLS_LOOP_PARENT;
    }
    break;
  
  case MLS_LOOP_PARENT:
    MUS_LOBBY_LoopParentMenu( work );
    if( work->selectItem != MUS_LOBBY_SELECT_NONE )
    {
      if( MUS_COMM_StartGame( work->commWork ) == TRUE )
      {
        work->state = MLS_PARENT_WAIT_START;
      }
    }
    break;
    
  case MLS_PARENT_WAIT_START:
    if( MUS_COMM_IsStartGame( work->commWork ) == TRUE )
    {
      if( MUS_COMM_SetCommGameState( work->commWork , MCGS_DRESSUP ) == TRUE )
      {
        MUS_LOBBY_ExitParentMenu( work );
        work->state = MLS_INIT_FADE_OUT;
      }
    }
    break;

    //子機開始
  case MLS_INIT_CHILD:
    work->state = MLS_INIT_CHILD_WAIT;
    MUS_LOBBY_InitChildMenu( work );
    MUS_COMM_InitComm( work->commWork );
    break;
    
  case MLS_INIT_CHILD_WAIT:
    if( PRINTSYS_QUE_IsFinished( work->printQue ) == TRUE &&
        MUS_COMM_IsInitComm(work->commWork) == TRUE )
    {
      MUS_LOBBY_DispChildMenu( work );
      MUS_COMM_StartChild( work->commWork );
      work->state = MLS_LOOP_CHILD;
    }
    break;

  case MLS_LOOP_CHILD:
    MUS_LOBBY_LoopChildMenu( work );
    if( MUS_COMM_IsStartGame( work->commWork ) == TRUE )
    {
      if( MUS_COMM_SetCommGameState( work->commWork , MCGS_DRESSUP ) == TRUE )
      {
        MUS_LOBBY_ExitChildMenu( work );
        work->state = MLS_INIT_FADE_OUT;
      }
    }
    break;
    
  case MLS_INIT_FADE_OUT:
    WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , work->heapId );
    work->state = MLS_WAIT_FADE_OUT;
    break;
  
  case MLS_WAIT_FADE_OUT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      return GFL_PROC_RES_FINISH;
    }
    break;
  }

  PRINTSYS_QUE_Main( work->printQue );
  INFOWIN_Update();
  return GFL_PROC_RES_CONTINUE;
}



//--------------------------------------------------------------
//	グラフィック系初期化
//--------------------------------------------------------------
static void MUS_LOBBY_InitGraphic( MUS_LOBBY_WORK *work )
{
  GFL_DISP_GX_InitVisibleControl();
  GFL_DISP_GXS_InitVisibleControl();
  GX_SetVisiblePlane( 0 );
  GXS_SetVisiblePlane( 0 );
  WIPE_SetBrightness(WIPE_DISP_MAIN,WIPE_FADE_BLACK);
  WIPE_SetBrightness(WIPE_DISP_SUB,WIPE_FADE_BLACK);
  WIPE_ResetWndMask(WIPE_DISP_MAIN);
  WIPE_ResetWndMask(WIPE_DISP_SUB);
  
  GX_SetDispSelect(GX_DISP_SELECT_MAIN_SUB);
  GFL_DISP_SetBank( &vramBank );
  
  GFL_BG_Init( work->heapId );
  GFL_BMPWIN_Init( work->heapId );
  
  {
    static const GFL_BG_SYS_HEADER sys_data = 
      { GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_3D };

    //BG0S
    static const GFL_BG_BGCNT_HEADER bgCont_Bmp = {
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, 0x8000,
      GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
    };

    //BG3S
    static const GFL_BG_BGCNT_HEADER bgCont_Info = {
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x0c000, 0x4000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
    };

    //BG1M
    static const GFL_BG_BGCNT_HEADER bgCont_Str = {
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, 0x4000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
    };
    
    GFL_BG_SetBGMode( &sys_data );

    MUS_LOBBY_InitBgFunc( &bgCont_Bmp  , MUS_LOBBY_BG_BMP  );
    MUS_LOBBY_InitBgFunc( &bgCont_Info , MUS_LOBBY_BG_INFO );
    MUS_LOBBY_InitBgFunc( &bgCont_Str  , MUS_LOBBY_BG_STR  );
  }
  
  //フォント用パレット
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_MAIN_BG , MUS_LOBBY_FONT_PLT * 32, 16*2, work->heapId );
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_SUB_BG  , MUS_LOBBY_FONT_PLT * 32, 16*2, work->heapId );
  work->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , work->heapId );
  work->msgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_musical_comm_dat , work->heapId );

  GFL_FONTSYS_SetDefaultColor();
    
  //WinFrame用グラフィック設定
  BmpWinFrame_GraphicSet( MUS_LOBBY_BG_BMP , MUS_LOBBY_FRANE_CHR , MUS_LOBBY_FRAME_PLT  , 1, work->heapId );
  BmpWinFrame_GraphicSet( MUS_LOBBY_BG_STR , MUS_LOBBY_FRANE_CHR , MUS_LOBBY_FRAME_PLT  , 1, work->heapId );

  //背景色
  *((u16 *)HW_BG_PLTT) = 0x7d8c;//RGB(12, 12, 31);
  *((u16 *)(HW_BG_PLTT+0x400)) = 0x7d8c;//RGB(12, 12, 31);
  
}


//--------------------------------------------------------------------------
//  Bg初期化 機能部
//--------------------------------------------------------------------------
static void MUS_LOBBY_InitBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane )
{
  GFL_BG_SetBGControl( bgPlane, bgCont, GFL_BG_MODE_TEXT );
  GFL_BG_SetVisible( bgPlane, VISIBLE_ON );
  GFL_BG_ClearFrame( bgPlane );
  GFL_BG_LoadScreenReq( bgPlane );
}

//--------------------------------------------------------------------------
//  BmpWinに文字描画(更新はしない
//--------------------------------------------------------------------------
static void MUS_LOBBY_DrawStringFunc( MUS_LOBBY_WORK *work , GFL_BMPWIN *win ,  u8 strId , const BOOL isCenter )
{
  STRBUF *str = GFL_MSG_CreateString( work->msgHandle , strId );
  u8 left = 0;
  
  if( isCenter == TRUE )
  {
    const u8 width = GFL_BMPWIN_GetSizeX( win );
    left = (width*8 - PRINTSYS_GetStrWidth( str , work->fontHandle , 0 ))/2 ;
  }

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( win ) , 0xF );

  PRINTSYS_PrintQue( work->printQue , GFL_BMPWIN_GetBmp( win ) ,
                     left , 0 , str , work->fontHandle );
  GFL_STR_DeleteBuffer( str );
}

#pragma mark [> upper func
//--------------------------------------------------------------------------
//  上画面文字列
//--------------------------------------------------------------------------
static void MUS_LOBBY_InitUpperStr( MUS_LOBBY_WORK *work )
{
  work->strWin = GFL_BMPWIN_Create( MUS_LOBBY_BG_STR ,1 , 19 , 30 , 4 ,
                          MUS_LOBBY_FONT_PLT , GFL_BMP_CHRAREA_GET_B );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->strWin ) , 0xF );
  TalkWinFrame_Write( work->strWin , WINDOW_TRANS_ON ,
              MUS_LOBBY_FRANE_CHR ,MUS_LOBBY_FRAME_PLT );
  GFL_BMPWIN_MakeTransWindow_VBlank( work->strWin );
}

static void MUS_LOBBY_ExitUpperStr( MUS_LOBBY_WORK *work )
{
  GFL_BMPWIN_Delete( work->strWin );
}

static void MUS_LOBBY_SetUpperStr( MUS_LOBBY_WORK *work , u16 strId)
{
  MUS_LOBBY_DrawStringFunc( work , work->strWin , strId , FALSE );
  GFL_BMPWIN_TransVramCharacter( work->strWin );
}

#pragma mark [>TopMenu
//--------------------------------------------------------------------------
//  TopMenu 初期化
//--------------------------------------------------------------------------
static void MUS_LOBBY_InitTopMenu(  MUS_LOBBY_WORK *work )
{
  u8 i;
  
  work->winArr[MLBT_START_PARENT] = GFL_BMPWIN_Create( MUS_LOBBY_BG_BMP ,
                                MUS_LOBBY_BMPWIN_LEFT , MUS_LOBBY_TOPMENU_PARENT , 
                                MUS_LOBBY_BMPWIN_WIDTH , MUS_LOBBY_BMPWIN_HEIGHT ,
                                MUS_LOBBY_FONT_PLT , GFL_BMP_CHRAREA_GET_B );
  work->winArr[MLBT_START_CHILD] = GFL_BMPWIN_Create( MUS_LOBBY_BG_BMP ,
                                MUS_LOBBY_BMPWIN_LEFT , MUS_LOBBY_TOPMENU_CHILD ,
                                MUS_LOBBY_BMPWIN_WIDTH , MUS_LOBBY_BMPWIN_HEIGHT ,
                                MUS_LOBBY_FONT_PLT , GFL_BMP_CHRAREA_GET_B );
  work->winArr[MLBT_CANCEL] = GFL_BMPWIN_Create( MUS_LOBBY_BG_BMP ,
                                MUS_LOBBY_BMPWIN_LEFT , MUS_LOBBY_TOPMENU_CANCEL ,
                                MUS_LOBBY_BMPWIN_WIDTH , MUS_LOBBY_BMPWIN_HEIGHT ,
                                MUS_LOBBY_FONT_PLT , GFL_BMP_CHRAREA_GET_B );

  for( i=0;i<MLBT_MAX;i++ )
  {
    MUS_LOBBY_DrawStringFunc( work , work->winArr[i] , LOBBY_TOPMENU_1+i , TRUE );
  }
  work->selectItem = MUS_LOBBY_SELECT_NONE;
  MUS_LOBBY_SetUpperStr( work , LOBBY_STR_1 );
}


//--------------------------------------------------------------------------
//  TopMenu 開放
//--------------------------------------------------------------------------
static void MUS_LOBBY_ExitTopMenu(  MUS_LOBBY_WORK *work )
{
  u8 i;
  
  for( i=0;i<MLBT_MAX;i++ )
  {
    GFL_BMPWIN_Delete( work->winArr[i] );
  }
}

//--------------------------------------------------------------------------
//  TopMenu 表示
//--------------------------------------------------------------------------
static void MUS_LOBBY_DispTopMenu(  MUS_LOBBY_WORK *work )
{
  u8 i;
  GFL_BG_ClearScreen( MUS_LOBBY_BG_BMP );
  for( i=0;i<MLBT_MAX;i++ )
  {
    TalkWinFrame_Write( work->winArr[i] , WINDOW_TRANS_ON ,
                MUS_LOBBY_FRANE_CHR ,MUS_LOBBY_FRAME_PLT );
    GFL_BMPWIN_MakeTransWindow_VBlank( work->winArr[i] );
  }
}

//--------------------------------------------------------------------------
//  TopMenu 更新
//--------------------------------------------------------------------------
static void MUS_LOBBY_LoopTopMenu(  MUS_LOBBY_WORK *work )
{
  const GFL_UI_TP_HITTBL hitTbl[MLBT_MAX+1] =
  {
    { 
      MUS_LOBBY_TOPMENU_CHILD*8 , 
      (MUS_LOBBY_TOPMENU_CHILD+MUS_LOBBY_BMPWIN_HEIGHT)*8 ,
      MUS_LOBBY_BMPWIN_LEFT*8 ,
      (MUS_LOBBY_BMPWIN_LEFT+MUS_LOBBY_BMPWIN_WIDTH)*8 ,
    },
    { 
      MUS_LOBBY_TOPMENU_PARENT*8 , 
      (MUS_LOBBY_TOPMENU_PARENT+MUS_LOBBY_BMPWIN_HEIGHT)*8 ,
      MUS_LOBBY_BMPWIN_LEFT*8 ,
      (MUS_LOBBY_BMPWIN_LEFT+MUS_LOBBY_BMPWIN_WIDTH)*8 ,
    },
    { 
      MUS_LOBBY_TOPMENU_CANCEL*8 , 
      (MUS_LOBBY_TOPMENU_CANCEL+MUS_LOBBY_BMPWIN_HEIGHT)*8 ,
      MUS_LOBBY_BMPWIN_LEFT*8 ,
      (MUS_LOBBY_BMPWIN_LEFT+MUS_LOBBY_BMPWIN_WIDTH)*8 ,
    },
    { GFL_UI_TP_HIT_END , 0 , 0 , 0 }
  };
  
  const int ret = GFL_UI_TP_HitTrg( hitTbl );
  
  if( ret != GFL_UI_TP_HIT_NONE )
  {
    work->selectItem = ret;
  }
}

#pragma mark [>parent menu
//--------------------------------------------------------------------------
//  親機メニュー
//--------------------------------------------------------------------------
static void MUS_LOBBY_InitParentMenu( MUS_LOBBY_WORK *work )
{
  u8 i;
  
  work->winArr[MLBP_MEMBER] = GFL_BMPWIN_Create( MUS_LOBBY_BG_BMP ,
                                MUS_LOBBY_BMPWIN_LEFT , MUS_LOBBY_PARENT_MEMBER_TOP , 
                                MUS_LOBBY_BMPWIN_WIDTH , MUS_LOBBY_PARENT_MEMBER_HEIGHT ,
                                MUS_LOBBY_FONT_PLT , GFL_BMP_CHRAREA_GET_B );
  work->winArr[MLBP_START] = GFL_BMPWIN_Create( MUS_LOBBY_BG_BMP ,
                                MUS_LOBBY_BMPWIN_LEFT , MUS_LOBBY_PARENT_START_TOP ,
                                MUS_LOBBY_BMPWIN_WIDTH , MUS_LOBBY_BMPWIN_HEIGHT ,
                                MUS_LOBBY_FONT_PLT , GFL_BMP_CHRAREA_GET_B );
  
  for( i=0;i<MLBP_MAX;i++ )
  {
    if( i == MLBP_MEMBER )
    {
      MUS_LOBBY_DrawMember( work );
    }
    else
    {
      MUS_LOBBY_DrawStringFunc( work , work->winArr[i] , LOBBY_PARENT_1+(i-1) , TRUE );
    }
  }
  work->selectItem = MUS_LOBBY_SELECT_NONE;
}

static void MUS_LOBBY_ExitParentMenu( MUS_LOBBY_WORK *work )
{
  u8 i;
  
  for( i=0;i<MLBP_MAX;i++ )
  {
    GFL_BMPWIN_Delete( work->winArr[i] );
  }
}

static void MUS_LOBBY_DispParentMenu( MUS_LOBBY_WORK *work )
{
  u8 i;
  GFL_BG_ClearScreen( MUS_LOBBY_BG_BMP );
  for( i=0;i<MLBP_MAX;i++ )
  {
    TalkWinFrame_Write( work->winArr[i] , WINDOW_TRANS_ON ,
                MUS_LOBBY_FRANE_CHR ,MUS_LOBBY_FRAME_PLT );
    GFL_BMPWIN_MakeTransWindow_VBlank( work->winArr[i] );
  }
}

static void MUS_LOBBY_LoopParentMenu( MUS_LOBBY_WORK *work )
{
  //メンバーの更新判定
  if( MUS_COMM_IsRefreshUserData( work->commWork ) == TRUE )
  {
    MUS_COMM_ResetRefreshUserData( work->commWork );
    MUS_LOBBY_DrawMember( work );
  }
  if( work->isRefreshMember == TRUE &&
      PRINTSYS_QUE_IsFinished( work->printQue ) == TRUE )
  {
    work->isRefreshMember = FALSE;
    GFL_BMPWIN_MakeTransWindow_VBlank( work->winArr[MLBP_MEMBER] );
  }

  //TPチェック
  {
    const GFL_UI_TP_HITTBL hitTbl[MLBT_MAX+1] =
    {
      { 
        MUS_LOBBY_PARENT_START_TOP*8 , 
        (MUS_LOBBY_PARENT_START_TOP+MUS_LOBBY_BMPWIN_HEIGHT)*8 ,
        MUS_LOBBY_BMPWIN_LEFT*8 ,
        (MUS_LOBBY_BMPWIN_LEFT+MUS_LOBBY_BMPWIN_WIDTH)*8 ,
      },
      { GFL_UI_TP_HIT_END , 0 , 0 , 0 }
    };
    
    const int ret = GFL_UI_TP_HitTrg( hitTbl );
    
    if( ret != GFL_UI_TP_HIT_NONE )
    {
      work->selectItem = ret;
    }
  }
}

static void MUS_LOBBY_DrawMember( MUS_LOBBY_WORK *work )
{
  u8 i;
  
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->winArr[MLBP_MEMBER] ) , 0xF );

  for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
  {
    MYSTATUS *myStatus = MUS_COMM_GetPlayerMyStatus( work->commWork , i );
    STRBUF *str = GFL_MSG_CreateString( work->msgHandle , LOBBY_PARENT_MEMBER1+i );
    if( myStatus != NULL )
    {
      STRBUF *newStr = GFL_STR_CreateBuffer( 32 , work->heapId );
      WORDSET *wordSet = WORDSET_Create( work->heapId );
      WORDSET_RegisterPlayerName( wordSet , 0 , myStatus );
      WORDSET_ExpandStr( wordSet , newStr , str );
      
      GFL_STR_DeleteBuffer( str );
      str = newStr;
      WORDSET_Delete( wordSet );
    }
    
    PRINTSYS_PrintQue( work->printQue , GFL_BMPWIN_GetBmp( work->winArr[MLBP_MEMBER] ) ,
                       0 , i*16 , str , work->fontHandle );
    GFL_STR_DeleteBuffer( str );
    
  }
  work->isRefreshMember = TRUE;
}

#pragma mark [>child menu
//--------------------------------------------------------------------------
//  子機メニュー
//--------------------------------------------------------------------------
static void MUS_LOBBY_InitChildMenu( MUS_LOBBY_WORK *work )
{
  u8 i;
  
  work->winArr[MLBC_MEMBER] = GFL_BMPWIN_Create( MUS_LOBBY_BG_BMP ,
                                MUS_LOBBY_BMPWIN_LEFT , MUS_LOBBY_PARENT_MEMBER_TOP , 
                                MUS_LOBBY_BMPWIN_WIDTH , MUS_LOBBY_PARENT_MEMBER_HEIGHT ,
                                MUS_LOBBY_FONT_PLT , GFL_BMP_CHRAREA_GET_B );
  
  MUS_LOBBY_DrawMember( work );

  work->selectItem = MUS_LOBBY_SELECT_NONE;
}

static void MUS_LOBBY_ExitChildMenu( MUS_LOBBY_WORK *work )
{
  u8 i;
  
  for( i=0;i<MLBC_MAX;i++ )
  {
    GFL_BMPWIN_Delete( work->winArr[i] );
  }
}

static void MUS_LOBBY_DispChildMenu( MUS_LOBBY_WORK *work )
{
  u8 i;
  GFL_BG_ClearScreen( MUS_LOBBY_BG_BMP );
  for( i=0;i<MLBC_MAX;i++ )
  {
    TalkWinFrame_Write( work->winArr[i] , WINDOW_TRANS_ON ,
                MUS_LOBBY_FRANE_CHR ,MUS_LOBBY_FRAME_PLT );
    GFL_BMPWIN_MakeTransWindow_VBlank( work->winArr[i] );
  }
}

static void MUS_LOBBY_LoopChildMenu( MUS_LOBBY_WORK *work )
{
  if( MUS_COMM_IsRefreshUserData( work->commWork ) == TRUE )
  {
    MUS_COMM_ResetRefreshUserData( work->commWork );
    MUS_LOBBY_DrawMember( work );
  }
  if( work->isRefreshMember == TRUE &&
      PRINTSYS_QUE_IsFinished( work->printQue ) == TRUE )
  {
    work->isRefreshMember = FALSE;
    GFL_BMPWIN_MakeTransWindow_VBlank( work->winArr[MLBC_MEMBER] );
  }

}

