//======================================================================
/**
 * @file	mus_shot_data.c
 * @brief	ミュージカルショット　下画面情報
 * @author	ariizumi
 * @data	09/09/16
 *
 * モジュール名：MUS_SHOT_INFO
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/net_err.h"

#include "arc_def.h"
//#include "musical_shot.naix"
#include "dressup_gra.naix"
#include "message.naix"
#include "font/font.naix"
#include "msg/msg_musical_shot.h"

#include "app/app_taskmenu.h"
#include "app/app_menu_common.h"
#include "gamesystem/msgspeed.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "system/bmp_winframe.h"
#include "system/time_icon.h"
#include "system/wipe.h"
#include "test/ariizumi/ari_debug.h"
#include "mus_shot_info.h"
#include "mus_shot_snd_def.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define
#define MUS_INFO_FRAME_MSG ( GFL_BG_FRAME0_S )
#define MUS_INFO_FRAME_BAR ( GFL_BG_FRAME2_S )
#define MUS_INFO_FRAME_BG ( GFL_BG_FRAME3_S )

#define MUS_INFO_PAL_BAR ( 0x09 )
#define MUS_INFO_PAL_FONT ( 0x0a )
#define MUS_INFO_PAL_WIN ( 0x0b )
#define MUS_INFO_PAL_YESNO ( 0x0c ) //２本

#define MUS_INFO_MSGWIN_X ( 1 )
#define MUS_INFO_MSGWIN_Y ( 1 )
#define MUS_INFO_MSGWIN_WIDTH ( 30 )
#define MUS_INFO_MSGWIN_HEIGHT ( 4 )

#define MUS_INFO_YESNO_X ( 32-APP_TASKMENU_PLATE_WIDTH_YN_WIN )
#define MUS_INFO_YESNO_Y ( 6 )
#define MSU_INFO_YESNO_COLOR (PRINTSYS_LSB_Make(0xE,0xF,0))

#define MUS_INFO_CGX_WIN (1)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  MSIS_FADE_WAIT_PLAY_SE,
  MSIS_FADE_WAIT,
  //保存確認モード
  MSIS_MSG_WAIT,
  MSIS_YESNO_WAIT,
  MSIS_COMM_WAIT,
  //見るだけモード
  MSIS_KEY_WAIT,
  MSIS_WAIT_ANIME,
  
  MSIS_FINISH,
  
}MUSICAL_SHOT_INFO_STATE;

typedef enum
{
  SICR_PLT_APP,
  SICR_NCG_APP,
  SICR_ANM_APP,

  SICR_MAX,
  
}SHOT_INFO_CELL_RES;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
struct _MUS_SHOT_INFO_WORK
{
  HEAPID heapId;
  MUSICAL_SHOT_INFO_STATE state;
  u8 bgScrollCnt;
  
  BOOL              isChackMode;
  MUSICAL_SHOT_DATA *shotData;
  MUSICAL_SAVE      *musicalSave;
  //メッセージ用
  GFL_TCBLSYS     *tcblSys;
  GFL_BMPWIN      *msgWin;
  GFL_FONT      *fontHandle;
  PRINT_STREAM    *printHandle;
  GFL_MSGDATA     *msgHandle;
  STRBUF          *msgStr;
  BOOL            isUpdateQue;
  TIMEICON_WORK *timeIcon;
  
  PRINT_QUE *printQue;
  APP_TASKMENU_WORK *yesNoWork;
	APP_TASKMENU_RES	*takmenures;

  MUS_COMM_WORK *commWork;
  
  GFL_CLUNIT  *cellUnit;
  u32         cellResIdx[SICR_MAX];
  GFL_CLWK    *clwkReturn;
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void MUS_SHOT_INFO_InitGraphic( MUS_SHOT_INFO_WORK *infoWork );
static void MUS_SHOT_INFO_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode );
static void MUS_SHOT_INFO_InitMessage( MUS_SHOT_INFO_WORK *infoWork );
static void MUS_SHOT_INFO_ExitMessage( MUS_SHOT_INFO_WORK *infoWork );
static void MUS_SHOT_INFO_DispMessage( MUS_SHOT_INFO_WORK *infoWork , const u16 msgId );
static void MUS_SHOT_INFO_DispMessageTimer( MUS_SHOT_INFO_WORK *infoWork , const u16 msgId );
static void MUS_SHOT_INFO_HideMessage( MUS_SHOT_INFO_WORK *infoWork );
static void MUS_SHOT_INFO_DispYesNo( MUS_SHOT_INFO_WORK *infoWork );

//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
MUS_SHOT_INFO_WORK* MUS_SHOT_INFO_InitSystem( MUSICAL_SHOT_DATA *shotData , MUSICAL_SAVE *musicalSave , const BOOL isChackMode , MUS_COMM_WORK *commWork , HEAPID heapId )
{
  MUS_SHOT_INFO_WORK* infoWork = GFL_HEAP_AllocClearMemory( heapId , sizeof( MUS_SHOT_INFO_WORK ));
  
  infoWork->heapId = heapId;
  infoWork->isChackMode = isChackMode;
  infoWork->shotData = shotData;
  infoWork->musicalSave = musicalSave;
  infoWork->commWork = commWork;
  infoWork->msgStr = NULL;
  infoWork->bgScrollCnt = 0;

  if( isChackMode == TRUE )
  {
    infoWork->state = MSIS_FADE_WAIT_PLAY_SE;
  }
  else
  {
    infoWork->state = MSIS_FADE_WAIT;
  }
  
  MUS_SHOT_INFO_InitGraphic( infoWork );
  MUS_SHOT_INFO_InitMessage( infoWork );

	infoWork->takmenures	= APP_TASKMENU_RES_Create( MUS_INFO_FRAME_MSG, MUS_INFO_PAL_YESNO, infoWork->fontHandle, infoWork->printQue, infoWork->heapId );

  GFL_NET_ReloadIconTopOrBottom(FALSE , heapId );
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );

  return infoWork;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
void MUS_SHOT_INFO_ExitSystem( MUS_SHOT_INFO_WORK *infoWork )
{
	APP_TASKMENU_RES_Delete( infoWork->takmenures );

  if( infoWork->isChackMode == FALSE )
  {
    GFL_CLGRP_PLTT_Release(     infoWork->cellResIdx[SICR_PLT_APP] );
    GFL_CLGRP_CGR_Release(      infoWork->cellResIdx[SICR_NCG_APP] );
    GFL_CLGRP_CELLANIM_Release( infoWork->cellResIdx[SICR_ANM_APP] );

    GFL_CLACT_WK_Remove( infoWork->clwkReturn );
  }

  GFL_CLACT_UNIT_Delete( infoWork->cellUnit );

  MUS_SHOT_INFO_ExitMessage( infoWork );
  GFL_BG_FreeBGControl( MUS_INFO_FRAME_MSG );
  GFL_BG_FreeBGControl( MUS_INFO_FRAME_BAR );
  GFL_BG_FreeBGControl( MUS_INFO_FRAME_BG );
  GFL_HEAP_FreeMemory( infoWork );
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
void MUS_SHOT_INFO_UpdateSystem( MUS_SHOT_INFO_WORK *infoWork )
{
  switch( infoWork->state )
  {
  case MSIS_FADE_WAIT_PLAY_SE:
    infoWork->state = MSIS_FADE_WAIT;
    PMSND_PlaySE( STA_SE_SHUTTER_SE );
    //break;  break througth

  case MSIS_FADE_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      if( infoWork->isChackMode == TRUE )
      {
        MUS_SHOT_INFO_DispMessage( infoWork , MUSICAL_SHOT_INFO_01 );
        infoWork->state = MSIS_MSG_WAIT;
      }
      else
      {
        infoWork->state = MSIS_KEY_WAIT;
      }
    }
    break;

  case MSIS_MSG_WAIT:
    if( infoWork->printHandle == NULL )
    {
      MUS_SHOT_INFO_DispYesNo( infoWork );
      infoWork->state = MSIS_YESNO_WAIT;
    }
    break;

  case MSIS_YESNO_WAIT:
    APP_TASKMENU_UpdateMenu( infoWork->yesNoWork );
    if( APP_TASKMENU_IsFinish( infoWork->yesNoWork ) == TRUE )
    {
      const u8 retPos = APP_TASKMENU_GetCursorPos( infoWork->yesNoWork );
      APP_TASKMENU_CloseMenu( infoWork->yesNoWork );
      if( retPos == 0 )
      {
        //セーブする！
        MUSICAL_SHOT_DATA *shotData = MUSICAL_SAVE_GetMusicalShotData( infoWork->musicalSave );
        GFL_STD_MemCopy( infoWork->shotData , shotData , sizeof(MUSICAL_SHOT_DATA) );
      }
      
      if( infoWork->commWork != NULL )
      {
        infoWork->state = MSIS_COMM_WAIT;
        MUS_COMM_SendTimingCommand( infoWork->commWork , MUS_COMM_TIMMING_SHOT_CONFIRM );
        MUS_SHOT_INFO_DispMessageTimer( infoWork , MUSICAL_SHOT_INFO_04 );
      }
      else
      {
        infoWork->state = MSIS_FINISH;
      }
    }
    else
    if( NetErr_App_CheckError() != NET_ERR_CHECK_NONE )
    {
      APP_TASKMENU_CloseMenu( infoWork->yesNoWork );
      infoWork->state = MSIS_FINISH;
    }
    break;
  
  case MSIS_COMM_WAIT:
    if( MUS_COMM_CheckTimingCommand( infoWork->commWork , MUS_COMM_TIMMING_SHOT_CONFIRM ) == TRUE )
    {
      infoWork->state = MSIS_FINISH;
    }
    else
    if( NetErr_App_CheckError() != NET_ERR_CHECK_NONE )
    {
      infoWork->state = MSIS_FINISH;
    }
    break;
  //見るだけモード
  case MSIS_KEY_WAIT:
    {
      static const GFL_UI_TP_HITTBL hitTbl[2] = 
      {
        { 192-24 , 192 ,
          256-24 , 255 },
        { GFL_UI_TP_HIT_END ,0,0,0 },
      };
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ||
          GFL_UI_TP_HitTrg( hitTbl ) == 0 )
      {
        PMSND_PlaySE( STA_SE_CANCEL );
        infoWork->state = MSIS_WAIT_ANIME;
        GFL_CLACT_WK_SetAnmSeq( infoWork->clwkReturn , APP_COMMON_BARICON_RETURN_ON );
        GFL_CLACT_WK_SetAutoAnmFlag( infoWork->clwkReturn , TRUE );
      }
    }
    break;
  case MSIS_WAIT_ANIME:
    if( GFL_CLACT_WK_CheckAnmActive( infoWork->clwkReturn ) == FALSE )
    {
      infoWork->state = MSIS_FINISH;
    }
    break;
  
  case MSIS_FINISH:
    break;
  }

  if( infoWork->printHandle != NULL  )
  {
    if( PRINTSYS_PrintStreamGetState( infoWork->printHandle ) == PRINTSTREAM_STATE_DONE )
    {
      PRINTSYS_PrintStreamDelete( infoWork->printHandle );
      infoWork->printHandle = NULL;
    }
  }
  PRINTSYS_QUE_Main( infoWork->printQue );

  if( infoWork->isUpdateQue == TRUE )
  {
    if( PRINTSYS_QUE_IsFinished( infoWork->printQue ) == TRUE )
    {
      infoWork->isUpdateQue = FALSE;
      infoWork->timeIcon = TIMEICON_CreateTcbl( infoWork->tcblSys , infoWork->msgWin , 0x0F , TIMEICON_DEFAULT_WAIT , infoWork->heapId );
      GFL_BMPWIN_TransVramCharacter( infoWork->msgWin );
    }
  }
  GFL_TCBL_Main( infoWork->tcblSys );

  //スクロール系
  infoWork->bgScrollCnt++;
  if( infoWork->bgScrollCnt > 1 )
  {
    infoWork->bgScrollCnt = 0;
    GFL_BG_SetScrollReq( MUS_INFO_FRAME_BG , GFL_BG_SCROLL_X_INC , 1 );
    GFL_BG_SetScrollReq( MUS_INFO_FRAME_BG , GFL_BG_SCROLL_Y_DEC , 1 );
  }
}

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
const BOOL MUS_SHOT_INFO_IsFinish( MUS_SHOT_INFO_WORK *infoWork )
{
  if( infoWork->state == MSIS_FINISH )
  {
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
//	描画系初期化
//--------------------------------------------------------------
static void MUS_SHOT_INFO_InitGraphic( MUS_SHOT_INFO_WORK *infoWork )
{
  //BG系
  {
    // BG0 SUB (メッセージWin
    static const GFL_BG_BGCNT_HEADER header_sub0 = {
      0, 0, 0x1000, 0, // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x04000,0x2000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    /*
    // BG1 SUB (カーテン右
    static const GFL_BG_BGCNT_HEADER header_sub1 = {
      0, 0, 0x1000, 0, // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x08000,0x0000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    */
    // BG2 SUB (情報
    static const GFL_BG_BGCNT_HEADER header_sub2 = {
      0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x00000,0x4000,
      GX_BG_EXTPLTT_01, 2, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG3 SUB (背景
    static const GFL_BG_BGCNT_HEADER header_sub3 = {
      0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000,0x0000,
      GX_BG_EXTPLTT_01, 3, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    MUS_SHOT_INFO_SetupBgFunc( &header_sub0 , MUS_INFO_FRAME_MSG  , GFL_BG_MODE_TEXT );
    MUS_SHOT_INFO_SetupBgFunc( &header_sub2 , MUS_INFO_FRAME_BAR  , GFL_BG_MODE_TEXT );
    MUS_SHOT_INFO_SetupBgFunc( &header_sub3 , MUS_INFO_FRAME_BG , GFL_BG_MODE_TEXT );
  }

  //OBJ系の初期化
  {
    infoWork->cellUnit = GFL_CLACT_UNIT_Create( 8 , 0, infoWork->heapId );
    GFL_CLACT_UNIT_SetDefaultRend( infoWork->cellUnit );

    GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );
  }
  
  //BG読み込み
  /*
  {
    ARCHANDLE *arcHandle = GFL_ARC_OpenDataHandle( ARCID_MUSICAL_SHOT , infoWork->heapId );
    GFL_ARCHDL_UTIL_TransVramPalette( arcHandle , NARC_musical_shot_info_bg_NCLR , 
                      PALTYPE_SUB_BG , 0 , 0 , infoWork->heapId );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_musical_shot_info_bg_NCGR ,
                      MUS_INFO_FRAME_BG , 0 , 0, FALSE , infoWork->heapId );
    GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_musical_shot_info_bg_NSCR , 
                      MUS_INFO_FRAME_BG ,  0 , 0, FALSE , infoWork->heapId );
    GFL_BG_LoadScreenReq(MUS_INFO_FRAME_BG);
    GFL_ARC_CloseDataHandle(arcHandle);
  }
  */
  {
    ARCHANDLE *arcHandle = GFL_ARC_OpenDataHandle( ARCID_DRESSUP_GRA , infoWork->heapId );

    //下画面
    GFL_ARCHDL_UTIL_TransVramPalette( arcHandle , NARC_dressup_gra_test_bg_d_NCLR , 
                      PALTYPE_SUB_BG , 0 , 0 , infoWork->heapId );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_dressup_gra_bg_mirror_NCGR ,
                      MUS_INFO_FRAME_BG , 0 , 0, FALSE , infoWork->heapId );
    GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_dressup_gra_test_bg_d_NSCR , 
                      MUS_INFO_FRAME_BG ,  0 , 0, FALSE , infoWork->heapId );
    GFL_ARC_CloseDataHandle(arcHandle);
  }
  if( infoWork->isChackMode == FALSE )
  {
    //共通素材
    {
      ARCHANDLE *commonArcHandle = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId() , infoWork->heapId );
      
      //バー
      GFL_ARCHDL_UTIL_TransVramPalette( commonArcHandle , APP_COMMON_GetBarPltArcIdx() , 
                        PALTYPE_SUB_BG , MUS_INFO_PAL_BAR*32 , 32 , infoWork->heapId );
      GFL_ARCHDL_UTIL_TransVramBgCharacter( commonArcHandle , APP_COMMON_GetBarCharArcIdx() ,
                        MUS_INFO_FRAME_BAR , 0 , 0, FALSE , infoWork->heapId );
      GFL_ARCHDL_UTIL_TransVramScreen( commonArcHandle , APP_COMMON_GetBarScrnArcIdx() , 
                        MUS_INFO_FRAME_BAR , 0 , 0, FALSE , infoWork->heapId );
      GFL_BG_ChangeScreenPalette( MUS_INFO_FRAME_BAR , 0 , 21 , 32 , 3 , MUS_INFO_PAL_BAR );
      GFL_BG_LoadScreenReq( MUS_INFO_FRAME_BAR );

      //バーアイコン
      infoWork->cellResIdx[SICR_PLT_APP] = GFL_CLGRP_PLTT_RegisterEx( commonArcHandle , 
            APP_COMMON_GetBarIconPltArcIdx() , CLSYS_DRAW_SUB , 
            0 , 0 , 
            APP_COMMON_BARICON_PLT_NUM , infoWork->heapId  );
      infoWork->cellResIdx[SICR_NCG_APP] = GFL_CLGRP_CGR_Register( commonArcHandle , 
            APP_COMMON_GetBarIconCharArcIdx() , FALSE , CLSYS_DRAW_SUB , infoWork->heapId  );
      infoWork->cellResIdx[SICR_ANM_APP] = GFL_CLGRP_CELLANIM_Register( commonArcHandle , 
            APP_COMMON_GetBarIconCellArcIdx(APP_COMMON_MAPPING_32K) , 
            APP_COMMON_GetBarIconAnimeArcIdx(APP_COMMON_MAPPING_32K), 
            infoWork->heapId  );

      GFL_ARC_CloseDataHandle( commonArcHandle );
    }
    
    {
      //OBJの作成
      GFL_CLWK_DATA cellInitData;
      cellInitData.pos_x = 256-24;
      cellInitData.pos_y = 192-24;
      cellInitData.anmseq = APP_COMMON_BARICON_RETURN;
      cellInitData.bgpri = 0;
      cellInitData.softpri = 0;

      infoWork->clwkReturn = GFL_CLACT_WK_Create( infoWork->cellUnit ,
                infoWork->cellResIdx[SICR_NCG_APP],
                infoWork->cellResIdx[SICR_PLT_APP],
                infoWork->cellResIdx[SICR_ANM_APP],
                &cellInitData ,CLSYS_DRAW_SUB , infoWork->heapId );
    }
  }

}
//--------------------------------------------------------------------------
//  Bg初期化 機能部
//--------------------------------------------------------------------------
static void MUS_SHOT_INFO_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode )
{
  GFL_BG_SetBGControl( bgPlane, bgCont, mode );
  GFL_BG_SetVisible( bgPlane, VISIBLE_ON );
  GFL_BG_ClearFrame( bgPlane );
  GFL_BG_LoadScreenReq( bgPlane );
}

//--------------------------------------------------------------------------
//  メッセージ系初期化
//--------------------------------------------------------------------------
static void MUS_SHOT_INFO_InitMessage( MUS_SHOT_INFO_WORK *infoWork )
{
  //メッセージ用処理
  infoWork->msgWin = GFL_BMPWIN_Create( MUS_INFO_FRAME_MSG , MUS_INFO_MSGWIN_X , MUS_INFO_MSGWIN_Y ,
                  MUS_INFO_MSGWIN_WIDTH , MUS_INFO_MSGWIN_HEIGHT , MUS_INFO_PAL_FONT ,
                  GFL_BMP_CHRAREA_GET_B );
  GFL_BMPWIN_TransVramCharacter( infoWork->msgWin );
  GFL_BMPWIN_MakeScreen( infoWork->msgWin );
  GFL_BG_LoadScreenReq(MUS_INFO_FRAME_MSG);
  
  
  //フォント読み込み
  infoWork->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , infoWork->heapId );
  
  //メッセージ
  infoWork->msgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_musical_shot_dat , infoWork->heapId );

  BmpWinFrame_GraphicSet( MUS_INFO_FRAME_MSG , MUS_INFO_CGX_WIN , MUS_INFO_PAL_WIN , 0 , infoWork->heapId );

  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_SUB_BG , MUS_INFO_PAL_FONT*0x20, 16*2, infoWork->heapId );
  GFL_FONTSYS_SetDefaultColor();
  
  infoWork->tcblSys = GFL_TCBL_Init( infoWork->heapId , infoWork->heapId , 3 , 0x100 );
  infoWork->printHandle = NULL;
  infoWork->msgStr = NULL;
  infoWork->isUpdateQue = FALSE;
  infoWork->timeIcon = NULL;

  //YesNo用
  infoWork->printQue = PRINTSYS_QUE_Create( infoWork->heapId );
}

//--------------------------------------------------------------------------
//  メッセージ系開放
//--------------------------------------------------------------------------
static void MUS_SHOT_INFO_ExitMessage( MUS_SHOT_INFO_WORK *infoWork )
{
  PRINTSYS_QUE_Delete( infoWork->printQue );
  if( infoWork->timeIcon != NULL )
  {
    TIMEICON_Exit( infoWork->timeIcon );
    infoWork->timeIcon = NULL;
  }
  if( infoWork->printHandle != NULL )
  {
    PRINTSYS_PrintStreamDelete( infoWork->printHandle );
  }
  if( infoWork->msgStr != NULL )
  {
    GFL_STR_DeleteBuffer( infoWork->msgStr );
  }
  GFL_MSG_Delete( infoWork->msgHandle );
  GFL_BMPWIN_Delete( infoWork->msgWin );
  GFL_FONT_Delete( infoWork->fontHandle );
  GFL_TCBL_Exit( infoWork->tcblSys );
}

//--------------------------------------------------------------------------
//  メッセージ表示
//--------------------------------------------------------------------------
static void MUS_SHOT_INFO_DispMessage( MUS_SHOT_INFO_WORK *infoWork , const u16 msgId )
{
  if( infoWork->printHandle != NULL )
  {
    MUS_TPrintf( NULL , "Message is not finish!!\n" );
    PRINTSYS_PrintStreamDelete( infoWork->printHandle );
    infoWork->printHandle = NULL;
  }

  {
    if( infoWork->msgStr != NULL )
    {
      GFL_STR_DeleteBuffer( infoWork->msgStr );
      infoWork->msgStr = NULL;
    }
    
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( infoWork->msgWin ) , 0xf );
    infoWork->msgStr = GFL_MSG_CreateString( infoWork->msgHandle , msgId );
    infoWork->printHandle = PRINTSYS_PrintStream( infoWork->msgWin , 0,0, infoWork->msgStr ,infoWork->fontHandle ,
                        MSGSPEED_GetWait() , infoWork->tcblSys , 2 , infoWork->heapId , 0 );
  }
  BmpWinFrame_Write( infoWork->msgWin , WINDOW_TRANS_ON_V , MUS_INFO_CGX_WIN , MUS_INFO_PAL_WIN );
}

static void MUS_SHOT_INFO_DispMessageTimer( MUS_SHOT_INFO_WORK *infoWork , const u16 msgId )
{
  {
    if( infoWork->msgStr != NULL )
    {
      GFL_STR_DeleteBuffer( infoWork->msgStr );
      infoWork->msgStr = NULL;
    }
    
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( infoWork->msgWin ) , 0xf );
    infoWork->msgStr = GFL_MSG_CreateString( infoWork->msgHandle , msgId );
    PRINTSYS_PrintQue( infoWork->printQue , GFL_BMPWIN_GetBmp( infoWork->msgWin ) , 
            0 , 0 , infoWork->msgStr , infoWork->fontHandle );
    infoWork->isUpdateQue = TRUE;
  }
  BmpWinFrame_Write( infoWork->msgWin , WINDOW_TRANS_ON_V , MUS_INFO_CGX_WIN , MUS_INFO_PAL_WIN );
}

//--------------------------------------------------------------------------
//  メッセージ表示
//--------------------------------------------------------------------------
static void MUS_SHOT_INFO_HideMessage( MUS_SHOT_INFO_WORK *infoWork )
{
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( infoWork->msgWin ) , 0 );
  BmpWinFrame_Clear( infoWork->msgWin , WINDOW_TRANS_ON_V );
  if( infoWork->timeIcon != NULL )
  {
    TIMEICON_Exit( infoWork->timeIcon );
    infoWork->timeIcon = NULL;
  }
}

//--------------------------------------------------------------------------
//  選択肢表示
//--------------------------------------------------------------------------
static void MUS_SHOT_INFO_DispYesNo( MUS_SHOT_INFO_WORK *infoWork )
{
  APP_TASKMENU_ITEMWORK itemWork[2];
  APP_TASKMENU_INITWORK initWork;
  
  itemWork[0].str = GFL_MSG_CreateString( infoWork->msgHandle , MUSICAL_SHOT_INFO_02 );
  itemWork[1].str = GFL_MSG_CreateString( infoWork->msgHandle , MUSICAL_SHOT_INFO_03 );
  itemWork[0].msgColor = MSU_INFO_YESNO_COLOR;
  itemWork[1].msgColor = MSU_INFO_YESNO_COLOR;
  itemWork[0].type = APP_TASKMENU_WIN_TYPE_NORMAL;
  itemWork[1].type = APP_TASKMENU_WIN_TYPE_NORMAL;

  initWork.heapId = infoWork->heapId;
  initWork.itemNum = 2;
  initWork.itemWork = itemWork;
//  initWork.bgFrame = MUS_INFO_FRAME_MSG;
 // initWork.palNo = MUS_INFO_PAL_YESNO;
  initWork.posType = ATPT_LEFT_UP;
  initWork.charPosX = MUS_INFO_YESNO_X;
  initWork.charPosY = MUS_INFO_YESNO_Y;
  //initWork.msgHandle = infoWork->msgHandle;
 // initWork.fontHandle = infoWork->fontHandle;
//  initWork.printQue = infoWork->printQue;
  initWork.w = APP_TASKMENU_PLATE_WIDTH_YN_WIN;
  initWork.h = APP_TASKMENU_PLATE_HEIGHT_YN_WIN;

  infoWork->yesNoWork = APP_TASKMENU_OpenMenu( &initWork, infoWork->takmenures );
  
  GFL_STR_DeleteBuffer( itemWork[0].str );
  GFL_STR_DeleteBuffer( itemWork[1].str );
}
