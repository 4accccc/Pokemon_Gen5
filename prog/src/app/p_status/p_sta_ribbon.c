//======================================================================
/**
 * @file	p_sta_ribbon.c
 * @brief	ポケモンステータス リボンページ
 * @author	ariizumi
 * @data	09/07/06
 *
 * モジュール名：PSTATUS_RIBBON
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/wipe.h"
#include "system/mcss.h"
#include "system/mcss_tool.h"
#include "print/global_msg.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "app/app_menu_common.h"

#include "arc_def.h"
#include "p_status_gra.naix"
#include "msg/msg_ribbon.h"
#include "message.naix"

#include "p_sta_sys.h"
#include "p_sta_ribbon.h"
#include "p_sta_oam.h"
#include "p_sta_snd_def.h"
#include "ribbon.h"

#include "test/ariizumi/ari_debug.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//フォントカラー(表題と数値用
//#define PSTATUS_SUB_STR_COL_TITLE (PRINTSYS_LSB_Make(0xf,2,0))
//#define PSTATUS_SUB_STR_COL_VALUE (PRINTSYS_LSB_Make(1,2,0))

//バー系
//用意するバーの数
#define PSTATUS_RIBBON_BAR_NUM (10)
//Window内(?)に表示する個数
#define PSTATUS_RIBBON_BAR_DIPS_NUM (6)
#define PSTATUS_RIBBON_INVALID_TYPE (0xFFFF)

#define PSTATUS_RIBBON_BAR_X   (8)
#define PSTATUS_RIBBON_BAR_Y   (8)
#define PSTATUS_RIBBON_BAR_WIDTH   (136)
#define PSTATUS_RIBBON_BAR_HEIGHT  (24)

#define PSTATUS_RIBBON_BAR_CHARSIZE  (17*3*0x20)

#define PSTATUS_RIBBON_BAR_SPEED_RATE (2)

//リボン詳細用
#define PSTATUS_RIBBON_NAME_WIN_LEFT (5)
#define PSTATUS_RIBBON_NAME_WIN_TOP (7)
#define PSTATUS_RIBBON_NAME_WIN_WIDTH (22)
#define PSTATUS_RIBBON_NAME_WIN_HEIGHT (2)

#define PSTATUS_RIBBON_INFO_WIN_LEFT (1)
#define PSTATUS_RIBBON_INFO_WIN_TOP (16)
#define PSTATUS_RIBBON_INFO_WIN_WIDTH (30)
#define PSTATUS_RIBBON_INFO_WIN_HEIGHT (4)

#define PSTATUS_RIBBON_NAME_STR_X ( 0+ PSTATUS_STR_OFS_X)
#define PSTATUS_RIBBON_NAME_STR_Y ( 0+ PSTATUS_STR_OFS_Y)
#define PSTATUS_RIBBON_INFO_STR_X ( 0+ PSTATUS_STR_OFS_X)
#define PSTATUS_RIBBON_INFO_STR_Y ( 0+ PSTATUS_STR_OFS_Y)

#define PSTATUS_RIBBON_ICON_X ( 128 )
#define PSTATUS_RIBBON_ICON_Y ( 88 )

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef struct
{
  u16  dispRibbonNo;
  u32 cellResNcg;
  BOOL isUpdateStr;

  GFL_BMP_DATA *bmpData;
  PSTA_OAM_ACT_PTR bmpOam;
  
}PSTATUS_RIBBON_DISP_WORK;

typedef struct
{
  BOOL isEnable;
  u8 ribbonNo;
  u8 category;
  u8 catNo;
}PSTATUS_RIBBON_DATA_WORK;

struct _PSTATUS_RIBBON_WORK
{
  BOOL isDisp;
  BOOL isDispInfo;
  BOOL isUpdateStrInfo;
  BOOL isMoveRibbon;
  
  u8  ribbonNum;
  u32 pagePos;

  u32 ribbonIconNcg;
  GFL_CLWK *clwkRibbonIcon;
  GFL_BMPWIN *bmpWinName;
  GFL_BMPWIN *bmpWinInfo;
  
  NNSG2dScreenData *scrDataDown;
  void *scrResDown;
  NNSG2dScreenData *scrDataUp;
  void *scrResUp;
  NNSG2dScreenData *scrDataUpDetail;
  void *scrResUpDetail;
  NNSG2dScreenData *scrDataUpTitle;
  void *scrResUpTitle;
  NNSG2dCharacterData *srcCellNcg;
  void *resCellNcg;

  GFL_MSGDATA *ribbonMsg;

  //カーソル系
  GFL_CLWK *clwkCur;
  u8       selectIdx;
  u8       selectType;
  u8       befSelectIdx;
  s32      speed;
  u8       sndCnt;
  
  BOOL     isTouchBar;
  BOOL     isUpdateIdx;
  
  
  PSTA_OAM_SYS_PTR bmpOamSys;
  PSTATUS_RIBBON_DISP_WORK ribbonDispWork[PSTATUS_RIBBON_BAR_NUM];
  PSTATUS_RIBBON_DATA_WORK ribbonDataWork[RIBBON_MAX];
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void PSTATUS_RIBBON_CreateRibbonBarFunc( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork , PSTATUS_RIBBON_DISP_WORK *ribbonDispWork );
static void PSTATUS_RIBBON_UpdateUI( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork );
static void PSTATUS_RIBBON_UpdatRibbon( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork );
static u8 PSTATUS_RIBBON_GetRibbonIdx( PSTATUS_RIBBON_WORK *ribbonWork , const u8 idx );
static u16 PSTATUS_RIBBON_GetRibbonType( PSTATUS_RIBBON_WORK *ribbonWork , const u8 idx );
static s16 PSTATUS_RIBBON_CalcRibbonBarY( PSTATUS_RIBBON_WORK *ribbonWork , const u8 idx );

static void PSTATUS_RIBBON_DispInfo( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork );
static void PSTATUS_RIBBON_DispInfo_Trans( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork );
static void PSTATUS_RIBBON_ClearInfo( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork );
static void PSTATUS_RIBBON_ClearInfo_Trans( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork );

static const u8 RibbonDispArr [RIBBON_MAX] =
{
  0,32,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,
  19,20,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,
  51,52,21,22,53,54,55,56,57,58,23,24,59,70,60,61,72,62,63,64,
  65,66,67,68,69,28,29,30,31,78,79,71,74,75,76,77,25,26,27,73,
};

//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
PSTATUS_RIBBON_WORK* PSTATUS_RIBBON_Init( PSTATUS_WORK *work )
{
  PSTATUS_RIBBON_WORK* ribbonWork;
  
  ribbonWork = GFL_HEAP_AllocMemory( work->heapId , sizeof(PSTATUS_RIBBON_WORK) );
  ribbonWork->isDisp = FALSE;
  ribbonWork->sndCnt = 0;
  ribbonWork->ribbonMsg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , 
                                          NARC_message_ribbon_dat , work->heapId );

  return ribbonWork;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
void PSTATUS_RIBBON_Term( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  GFL_MSG_Delete( ribbonWork->ribbonMsg );
  GFL_HEAP_FreeMemory( ribbonWork );
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
void PSTATUS_RIBBON_Main( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  PSTATUS_RIBBON_UpdateUI( work , ribbonWork );
  if( ribbonWork->isUpdateStrInfo == TRUE )
  {
    if( PRINTSYS_QUE_IsFinished( work->printQue ) == TRUE )
    {
      PSTATUS_RIBBON_ClearInfo_Trans( work , ribbonWork );
      PSTATUS_RIBBON_DispInfo_Trans( work , ribbonWork );
      ribbonWork->isUpdateStrInfo = FALSE;
    }
  }
  else
  {
    PSTATUS_RIBBON_UpdatRibbon( work , ribbonWork );
  }
  if( ribbonWork->sndCnt > 0 )
  {
    ribbonWork->sndCnt--;
  }
}

#pragma mark [>Resource
//--------------------------------------------------------------
//	リソース読み込み
//--------------------------------------------------------------
void PSTATUS_RIBBON_LoadResource( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork , ARCHANDLE *archandle )
{
  u8 i;
  //書き換え用スクリーン読み込み
  ribbonWork->scrResDown = GFL_ARCHDL_UTIL_LoadScreen( archandle , NARC_p_status_gra_p_st_ribbon_d_NSCR ,
                    FALSE , &ribbonWork->scrDataDown , work->heapId );
  ribbonWork->scrResUp = GFL_ARCHDL_UTIL_LoadScreen( archandle , NARC_p_status_gra_p_st_ribbon_u1_NSCR ,
                    FALSE , &ribbonWork->scrDataUp , work->heapId );
  ribbonWork->scrResUpDetail = GFL_ARCHDL_UTIL_LoadScreen( archandle , NARC_p_status_gra_p_st_ribbon_u2_NSCR ,
                    FALSE , &ribbonWork->scrDataUpDetail , work->heapId );
  ribbonWork->scrResUpTitle = GFL_ARCHDL_UTIL_LoadScreen( archandle , NARC_p_status_gra_p_st_ribbontitle_u_NSCR ,
                    FALSE , &ribbonWork->scrDataUpTitle , work->heapId );

  //書き換え用OBJキャラクタ読み込み
  ribbonWork->resCellNcg = GFL_ARCHDL_UTIL_LoadOBJCharacter( archandle , NARC_p_status_gra_p_st_ribbon_bar_NCGR ,
                    FALSE , &ribbonWork->srcCellNcg , work->heapId );

  ribbonWork->bmpOamSys = PSTA_OAM_Init( work->heapId , work->cellUnit );
}

//--------------------------------------------------------------
//	リソース開放
//--------------------------------------------------------------
void PSTATUS_RIBBON_ReleaseResource( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  u8 i;
  //ここで描画物の開放もしておく
  if( ribbonWork->isDisp == TRUE )
  {
    PSTATUS_RIBBON_ClearPage( work , ribbonWork );
  }

  PSTA_OAM_Exit( ribbonWork->bmpOamSys );

  GFL_HEAP_FreeMemory( ribbonWork->resCellNcg );
  GFL_HEAP_FreeMemory( ribbonWork->scrResDown );
  GFL_HEAP_FreeMemory( ribbonWork->scrResUp );
  GFL_HEAP_FreeMemory( ribbonWork->scrResUpDetail );
  GFL_HEAP_FreeMemory( ribbonWork->scrResUpTitle );

}

#pragma mark [>Cell
void PSTATUS_RIBBON_InitCell( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  //選択カーソル
  {
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_x = 0;
    cellInitData.pos_y = 0;
    cellInitData.softpri = 10;
    cellInitData.bgpri = 0;
    cellInitData.anmseq = 0;
    
    ribbonWork->clwkCur = GFL_CLACT_WK_Create( work->cellUnit ,
              work->cellRes[SCR_NCG_RIBBON_CUR],
              work->cellRes[SCR_PLT_CURSOR_COMMON],
              work->cellRes[SCR_ANM_RIBBON_CUR],
              &cellInitData ,CLSYS_DEFREND_MAIN , work->heapId );

    GFL_CLACT_WK_SetDrawEnable( ribbonWork->clwkCur , FALSE );
  }  
  
    //セルの作成
  {
    u8 i;
    PSTA_OAM_ACT_DATA oamData;
    oamData.x = PSTATUS_RIBBON_BAR_X;
    oamData.pltt_index = work->cellRes[SCR_PLT_RIBBON_BAR];
    oamData.pal_offset = 0;
    oamData.soft_pri = 0;
    oamData.bg_pri = 2;
    oamData.setSerface = CLSYS_DEFREND_MAIN;
    oamData.draw_type = CLSYS_DRAW_MAIN;
    for( i=0 ; i<PSTATUS_RIBBON_BAR_NUM ; i++ )
    {
      u8 *vramAdr = (u8*)( (u32)G2_GetOBJCharPtr() + 0x20000 - (PSTATUS_RIBBON_BAR_CHARSIZE*(i+1)) );

      //oamData.y = PSTATUS_RIBBON_BAR_Y+PSTATUS_RIBBON_BAR_HEIGHT*i;
      oamData.y = PSTATUS_RIBBON_CalcRibbonBarY( ribbonWork , i );

      ribbonWork->ribbonDispWork[i].bmpData = GFL_BMP_CreateInVRAM( vramAdr , 
                                            PSTATUS_RIBBON_BAR_WIDTH/8 , 
                                            PSTATUS_RIBBON_BAR_HEIGHT/8 ,
                                            GFL_BMP_16_COLOR , work->heapId );

      oamData.bmp = ribbonWork->ribbonDispWork[i].bmpData;
      ribbonWork->ribbonDispWork[i].bmpOam = PSTA_OAM_ActorAdd( ribbonWork->bmpOamSys , &oamData );

      //PSTATUS_RIBBON_CreateRibbonBarFunc( work , ribbonWork , &ribbonWork->ribbonDispWork[i] );
      PSTA_OAM_ActorSetDrawEnable( ribbonWork->ribbonDispWork[i].bmpOam , FALSE );
    }
  }
}

void PSTATUS_RIBBON_TermCell( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  u8 i;
  GFL_CLACT_WK_Remove( ribbonWork->clwkCur );
  for( i=0;i<PSTATUS_RIBBON_BAR_NUM;i++ )
  {
    PSTA_OAM_ActorDel( ribbonWork->ribbonDispWork[i].bmpOam );
    GFL_BMP_Delete( ribbonWork->ribbonDispWork[i].bmpData );
  }
}

#pragma mark [>UI
static const BOOL PSTATUS_RIBBON_UpdateKey( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork );
static void PSTATUS_RIBBON_UpdateTP( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork );
static const int PSTATUS_RIBBON_CheckTouchBar( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork );

static void PSTATUS_RIBBON_MoveBar( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork , s16 value);
static void PSTATUS_RIBBON_SetCursorTopBar( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork );
static void PSTATUS_RIBBON_SetCursorPosBar( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork , const u8 idx);
//--------------------------------------------------------------
//	キー・タッチ更新
//--------------------------------------------------------------
static void PSTATUS_RIBBON_UpdateUI( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  if( work->isActiveBarButton == TRUE )
  {
    if( ribbonWork->ribbonNum > 0 ) 
    {
      const int touchBar = PSTATUS_RIBBON_CheckTouchBar( work , ribbonWork );
      //アクティブ判定はここで行う
      if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_A || 
          touchBar != GFL_UI_TP_HIT_NONE)
      {
        ribbonWork->speed = 0;

        PSTATUS_SetActiveBarButton( work , FALSE );
        if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_A )
        {
          PSTATUS_RIBBON_SetCursorTopBar( work , ribbonWork );
          //GFL_CLACT_WK_SetDrawEnable( ribbonWork->clwkCur , TRUE );
          work->ktst = GFL_APP_END_KEY;
          PMSND_PlaySystemSE(PSTATUS_SND_DECIDE);
        }
        else
        {
          ribbonWork->isTouchBar = TRUE;
          ribbonWork->selectIdx = touchBar;
          ribbonWork->isUpdateIdx = TRUE;
          ribbonWork->selectType = PSTATUS_RIBBON_GetRibbonType( ribbonWork , touchBar );
          PSTATUS_SetActiveBarButton( work , FALSE );
          work->ktst = GFL_APP_END_TOUCH;
          PMSND_PlaySystemSE(PSTATUS_SND_DECIDE);
        }
        PSTATUS_RIBBON_ClearInfo( work , ribbonWork );
        PSTATUS_RIBBON_DispInfo( work , ribbonWork );
      }
    }
  }
  else
  {
    //アクティブ時の更新
    if( PSTATUS_RIBBON_UpdateKey( work,ribbonWork ) == FALSE )
    {
      PSTATUS_RIBBON_UpdateTP( work,ribbonWork );
    }
  }
}

static const BOOL PSTATUS_RIBBON_UpdateKey( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_B )
  {
    //戻る
    PSTATUS_SetActiveBarButton( work , TRUE );
    GFL_CLACT_WK_SetAnmSeq( work->clwkBarIcon[SBT_RETURN] , APP_COMMON_BARICON_RETURN_ON );
    GFL_CLACT_WK_SetDrawEnable( ribbonWork->clwkCur , FALSE );
    ribbonWork->selectIdx = 0xFF;
    ribbonWork->selectType = PSTATUS_RIBBON_INVALID_TYPE;
    ribbonWork->isUpdateIdx = TRUE;

    PSTATUS_RIBBON_ClearInfo( work , ribbonWork );
    PSTATUS_RIBBON_ClearInfo_Trans( work , ribbonWork );
    work->ktst = GFL_APP_END_KEY;
    PMSND_PlaySystemSE(PSTATUS_SND_CANCEL);
    return TRUE;
  }
  else
  if( work->ktst == GFL_APP_END_TOUCH )
  {
    if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_A ||
        GFL_UI_KEY_GetTrg() == PAD_KEY_DOWN ||
        GFL_UI_KEY_GetTrg() == PAD_KEY_UP )
    {
      ribbonWork->speed = 0;
      ribbonWork->isTouchBar = FALSE;
      PSTATUS_RIBBON_SetCursorTopBar( work , ribbonWork );
      //GFL_CLACT_WK_SetDrawEnable( ribbonWork->clwkCur , TRUE );
      PSTATUS_RIBBON_ClearInfo( work , ribbonWork );
      PSTATUS_RIBBON_DispInfo( work , ribbonWork );
      work->ktst = GFL_APP_END_KEY;
      return TRUE;
    }
  }
  else
  {
    if( GFL_UI_KEY_GetRepeat() == PAD_KEY_DOWN )
    {
      const u8 bottomPos = PSTATUS_RIBBON_BAR_X+(PSTATUS_RIBBON_BAR_DIPS_NUM-1)*PSTATUS_RIBBON_BAR_HEIGHT;
      u8 newIdx = ribbonWork->selectIdx + 1;
      if( newIdx >= PSTATUS_RIBBON_BAR_NUM )
      {
        newIdx -= PSTATUS_RIBBON_BAR_NUM;
      }
      
      if( ribbonWork->ribbonDispWork[newIdx].dispRibbonNo != PSTATUS_RIBBON_INVALID_TYPE )
      {
        ribbonWork->selectIdx = newIdx;
        ribbonWork->isUpdateIdx = TRUE;
        if( PSTATUS_RIBBON_CalcRibbonBarY( ribbonWork , ribbonWork->selectIdx ) > bottomPos )
        {
          const u8 sub = PSTATUS_RIBBON_CalcRibbonBarY( ribbonWork , ribbonWork->selectIdx ) - bottomPos;
          PSTATUS_RIBBON_MoveBar( work , ribbonWork , sub );
        }
        PSTATUS_RIBBON_SetCursorPosBar( work , ribbonWork , ribbonWork->selectIdx );

        PSTATUS_RIBBON_ClearInfo( work , ribbonWork );
        PSTATUS_RIBBON_DispInfo( work , ribbonWork );
      }
      PMSND_PlaySystemSE(PSTATUS_SND_CURSOR);
      return TRUE;
    }
    else
    if( GFL_UI_KEY_GetRepeat() == PAD_KEY_UP )
    {
      u8 newIdx;
      if( ribbonWork->selectIdx == 0 )
      {
        newIdx = PSTATUS_RIBBON_BAR_NUM-1;
      }
      else
      {
        newIdx = ribbonWork->selectIdx-1;
      }
      if( ribbonWork->ribbonDispWork[newIdx].dispRibbonNo != PSTATUS_RIBBON_INVALID_TYPE )
      {
        ribbonWork->selectIdx = newIdx;
        ribbonWork->isUpdateIdx = TRUE;

        if( PSTATUS_RIBBON_CalcRibbonBarY( ribbonWork , ribbonWork->selectIdx ) < PSTATUS_RIBBON_BAR_Y )
        {
          const s16 sub = PSTATUS_RIBBON_CalcRibbonBarY( ribbonWork , ribbonWork->selectIdx ) - PSTATUS_RIBBON_BAR_Y;
          PSTATUS_RIBBON_MoveBar( work , ribbonWork , sub );
        }
        PSTATUS_RIBBON_SetCursorPosBar( work , ribbonWork , ribbonWork->selectIdx );

        PSTATUS_RIBBON_ClearInfo( work , ribbonWork );
        PSTATUS_RIBBON_DispInfo( work , ribbonWork );
      }
      PMSND_PlaySystemSE(PSTATUS_SND_CURSOR);
      return TRUE;
    }
  }

  return FALSE;
}

static void PSTATUS_RIBBON_UpdateTP( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  if( work->barButtonHit == SBT_RETURN )
  {
    //戻るが押された
    PSTATUS_SetActiveBarButton( work , TRUE );
    GFL_CLACT_WK_SetAnmSeq( work->clwkBarIcon[SBT_RETURN] , APP_COMMON_BARICON_RETURN_ON );
    GFL_CLACT_WK_SetDrawEnable( ribbonWork->clwkCur , FALSE );
    ribbonWork->selectIdx = 0xFF;
    ribbonWork->selectType = PSTATUS_RIBBON_INVALID_TYPE;
    ribbonWork->isUpdateIdx = TRUE;
    work->ktst = GFL_APP_END_TOUCH;

    PSTATUS_RIBBON_ClearInfo( work , ribbonWork );
    PSTATUS_RIBBON_ClearInfo_Trans( work , ribbonWork );
    PMSND_PlaySystemSE(PSTATUS_SND_CANCEL);
  }
  else
  if( GFL_UI_TP_GetTrg() == TRUE &&
      work->tpx > PSTATUS_RIBBON_BAR_X &&
      work->tpx < PSTATUS_RIBBON_BAR_X + PSTATUS_RIBBON_BAR_WIDTH )
  {
    const int touchBar = PSTATUS_RIBBON_CheckTouchBar( work , ribbonWork );
    if( touchBar != GFL_UI_TP_HIT_NONE )
    {
      ribbonWork->selectIdx = touchBar;
      ribbonWork->isUpdateIdx = TRUE;
      ribbonWork->selectType = PSTATUS_RIBBON_GetRibbonType( ribbonWork , touchBar );
      ribbonWork->isTouchBar = TRUE;
      ribbonWork->speed = 0;
      GFL_CLACT_WK_SetDrawEnable( ribbonWork->clwkCur , FALSE );

      PSTATUS_RIBBON_ClearInfo( work , ribbonWork );
      PSTATUS_RIBBON_DispInfo( work , ribbonWork );

    }
    work->ktst = GFL_APP_END_TOUCH;
  }
  else
  if( GFL_UI_TP_GetCont() == TRUE &&
      ribbonWork->isTouchBar == TRUE &&
      work->tpx > PSTATUS_RIBBON_BAR_X &&
      work->tpx < PSTATUS_RIBBON_BAR_X + PSTATUS_RIBBON_BAR_WIDTH )
  {
    const s16 tpSub = work->befTpy - work->tpy;
    const s32 tempSpeed = tpSub*PSTATUS_RIBBON_BAR_SPEED_RATE;
    PSTATUS_RIBBON_MoveBar( work , ribbonWork , tpSub );
    if( tempSpeed >= 0 && ribbonWork->speed > 0 &&
        tempSpeed < ribbonWork->speed )
    {
      ribbonWork->speed = (ribbonWork->speed+tempSpeed)/2;
    }
    else
    if( tempSpeed <= 0 && ribbonWork->speed < 0 &&
        tempSpeed > ribbonWork->speed )
    {
      ribbonWork->speed = (ribbonWork->speed+tempSpeed)/2;
    }
    else
    {
      ribbonWork->speed = tempSpeed;
    }
  }
  else
  {
    const u32 befPos = ribbonWork->pagePos;
    ribbonWork->isTouchBar = FALSE;
    if( ribbonWork->speed < 0 )
    {
      ribbonWork->speed++;
      PSTATUS_RIBBON_MoveBar( work , ribbonWork , ribbonWork->speed/PSTATUS_RIBBON_BAR_SPEED_RATE );
    }
    if( ribbonWork->speed > 0 )
    {
      ribbonWork->speed--;
      PSTATUS_RIBBON_MoveBar( work , ribbonWork , ribbonWork->speed/PSTATUS_RIBBON_BAR_SPEED_RATE );
    }
  }
}

//--------------------------------------------------------------
//	バーがタッチされたか？
//--------------------------------------------------------------
static const int PSTATUS_RIBBON_CheckTouchBar( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  
  u8 i;
  GFL_UI_TP_HITTBL hitTbl[PSTATUS_RIBBON_BAR_NUM+1];

  //当たり判定の作成
  for( i=0;i<PSTATUS_RIBBON_BAR_NUM;i++ )
  {
    const s16 top = PSTATUS_RIBBON_CalcRibbonBarY( ribbonWork , i );
    
    hitTbl[i].rect.left = PSTATUS_RIBBON_BAR_X;
    hitTbl[i].rect.right= PSTATUS_RIBBON_BAR_X+PSTATUS_RIBBON_BAR_WIDTH;
    if( top < 0 )
    {
      hitTbl[i].rect.top = 0;
    }
    else
    if( top > 168 )
    {
      hitTbl[i].rect.top = 168;
    }
    else
    {
      hitTbl[i].rect.top = top;
    }
    
    if( top+PSTATUS_RIBBON_BAR_HEIGHT < 0 )
    {
      hitTbl[i].rect.bottom = 0;
    }
    else
    if( top+PSTATUS_RIBBON_BAR_HEIGHT > 168 )
    {
      hitTbl[i].rect.bottom = 168;
    }
    else
    {
      hitTbl[i].rect.bottom = top+PSTATUS_RIBBON_BAR_HEIGHT;
    }
    if( hitTbl[i].rect.top == 0 && hitTbl[i].rect.bottom == 0 )
    {
      hitTbl[i].circle.code = GFL_UI_TP_SKIP;
    }
    if( PSTATUS_RIBBON_GetRibbonType( ribbonWork , i ) == PSTATUS_RIBBON_INVALID_TYPE  )
    {
      hitTbl[i].circle.code = GFL_UI_TP_SKIP;
    }

  }
  hitTbl[PSTATUS_RIBBON_BAR_NUM].circle.code = GFL_UI_TP_HIT_END;
  
  return GFL_UI_TP_HitTrg( hitTbl );
}

//--------------------------------------------------------------
//	カーソルをトップのバーに合わせる
//--------------------------------------------------------------
static void PSTATUS_RIBBON_SetCursorTopBar( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  GFL_CLACTPOS cellPos;
  const u8 topIdx = ((ribbonWork->pagePos+PSTATUS_RIBBON_BAR_HEIGHT-1)/PSTATUS_RIBBON_BAR_HEIGHT)%PSTATUS_RIBBON_BAR_NUM;
  
  PSTATUS_RIBBON_SetCursorPosBar( work , ribbonWork , topIdx );
}
//--------------------------------------------------------------
//	カーソルを指定バーに合わせる
//--------------------------------------------------------------
static void PSTATUS_RIBBON_SetCursorPosBar( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork , const u8 idx)
{
  GFL_CLACTPOS cellPos;
  
  cellPos.x = PSTATUS_RIBBON_BAR_X;
  cellPos.y = PSTATUS_RIBBON_CalcRibbonBarY( ribbonWork , idx);
  
  GFL_CLACT_WK_SetPos( ribbonWork->clwkCur , &cellPos , CLSYS_DEFREND_MAIN );
  
  ribbonWork->selectIdx = idx;
  ribbonWork->isUpdateIdx = TRUE;
  ribbonWork->selectType = PSTATUS_RIBBON_GetRibbonType( ribbonWork , idx );
}

//--------------------------------------------------------------
//	バーの移動
//--------------------------------------------------------------
static void PSTATUS_RIBBON_MoveBar( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork , s16 value)
{
  //最大移動位置
  const u32 maxValue = (ribbonWork->ribbonNum-PSTATUS_RIBBON_BAR_DIPS_NUM)*PSTATUS_RIBBON_BAR_HEIGHT;
  //最大速度制限
  if( value > PSTATUS_RIBBON_BAR_HEIGHT )
  {
    value = PSTATUS_RIBBON_BAR_HEIGHT;
  }
  if( value < -PSTATUS_RIBBON_BAR_HEIGHT )
  {
    value = -PSTATUS_RIBBON_BAR_HEIGHT;
  }

  if( (s16)ribbonWork->pagePos+value < 0 )
  {
    ribbonWork->pagePos = 0;
  }
  else
  if( ribbonWork->pagePos+value > maxValue )
  {
    ribbonWork->pagePos = maxValue;
  }
  else
  {
    ribbonWork->pagePos += value;
  }
  
  if( value != 0 )
  {
    ribbonWork->isMoveRibbon = TRUE;
  }
}

#pragma mark [>Disp
//--------------------------------------------------------------
//	ページの表示
//--------------------------------------------------------------
void PSTATUS_RIBBON_DispPage( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  u8 i;
  ribbonWork->pagePos = 0;
  ribbonWork->selectIdx = 0xFF;
  ribbonWork->selectType = PSTATUS_RIBBON_INVALID_TYPE;

  ribbonWork->befSelectIdx = 0xFF;
  ribbonWork->isDisp = TRUE;
  ribbonWork->isDispInfo = FALSE;
  ribbonWork->isUpdateStrInfo = FALSE;
  ribbonWork->isMoveRibbon = TRUE;

  for( i=0 ; i<PSTATUS_RIBBON_BAR_NUM ; i++ )
  {
    ribbonWork->ribbonDispWork[i].dispRibbonNo = PSTATUS_RIBBON_GetRibbonType( ribbonWork , i );
    PSTATUS_RIBBON_CreateRibbonBarFunc( work , ribbonWork , 
                            &ribbonWork->ribbonDispWork[i] );
    //PSTA_OAM_ActorSetDrawEnable( ribbonWork->ribbonDispWork[i].bmpOam , FALSE );
  }

}
//--------------------------------------------------------------
//	ページの表示
//--------------------------------------------------------------
void PSTATUS_RIBBON_DispPage_Trans( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  u8 i;
  //Window下地の張替え
  GFL_BG_WriteScreenExpand( PSTATUS_BG_PLATE , 
//                    0 , 0 , PSTATUS_MAIN_PAGE_WIDTH , 24 ,
                    0 , 0 , 32 , 24 ,
                    ribbonWork->scrDataDown->rawData ,
                    0 , 0 , 32 , 32 );
  GFL_BG_LoadScreenV_Req( PSTATUS_BG_PLATE );

  GFL_BG_LoadScreenBuffer( PSTATUS_BG_SUB_PLATE, 
                     ribbonWork->scrDataUp->rawData, 
                     ribbonWork->scrDataUp->szByte );
  GFL_BG_LoadScreenV_Req( PSTATUS_BG_SUB_PLATE );
  
  //上画面タイトル
  GFL_BG_WriteScreenExpand( PSTATUS_BG_SUB_TITLE , 
                    0 , 0 , 32 , PSTATUS_SUB_TITLE_HEIGHT ,
                    ribbonWork->scrDataUpTitle->rawData ,
                    0 , 0 , 32 , 32 );
  GFL_BG_LoadScreenV_Req( PSTATUS_BG_SUB_TITLE );

  for( i=0 ; i<PSTATUS_RIBBON_BAR_NUM ; i++ )
  {
    const u16 dataNo = ribbonWork->ribbonDispWork[i].dispRibbonNo;
    s16 y = PSTATUS_RIBBON_CalcRibbonBarY( ribbonWork , i );
    PSTA_OAM_ActorSetPos( ribbonWork->ribbonDispWork[i].bmpOam , PSTATUS_RIBBON_BAR_X , y );
    if( dataNo != PSTATUS_RIBBON_INVALID_TYPE )
    {
      PSTA_OAM_ActorSetDrawEnable( ribbonWork->ribbonDispWork[i].bmpOam , TRUE );
      PSTA_OAM_ActorBmpTrans( ribbonWork->ribbonDispWork[i].bmpOam );
    }
    else
    {
      PSTA_OAM_ActorSetDrawEnable( ribbonWork->ribbonDispWork[i].bmpOam , FALSE );
    }
  }

  ribbonWork->pagePos = 0;
  ribbonWork->selectIdx = 0xFF;
  ribbonWork->selectType = PSTATUS_RIBBON_INVALID_TYPE;

  ribbonWork->befSelectIdx = 0xFF;
  ribbonWork->isDisp = TRUE;
  ribbonWork->isMoveRibbon = TRUE;
  
  //バーアイコン処理
  GFL_CLACT_WK_SetAnmSeq( work->clwkBarIcon[SBT_PAGE3] , SBA_PAGE3_SELECT );
}

//--------------------------------------------------------------
//	ページのクリア
//--------------------------------------------------------------
void PSTATUS_RIBBON_ClearPage( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  u8 i;
  ribbonWork->isDisp = FALSE;
  for( i=0 ; i<PSTATUS_RIBBON_BAR_NUM ; i++ )
  {
    //GFL_BMP_Clear( ribbonWork->ribbonDispWork[i].bmpData , 2 );
  }
}
//--------------------------------------------------------------
//	ページのクリア
//--------------------------------------------------------------
void PSTATUS_RIBBON_ClearPage_Trans( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  u8 i;
  for( i=0 ; i<PSTATUS_RIBBON_BAR_NUM ; i++ )
  {
    PSTA_OAM_ActorSetDrawEnable( ribbonWork->ribbonDispWork[i].bmpOam , FALSE );
  }
  
  GFL_BG_FillScreen( PSTATUS_BG_PARAM , 0 , 0 , 0 , 
                     PSTATUS_MAIN_PAGE_WIDTH , 21 ,
                     GFL_BG_SCRWRT_PALNL );
  GFL_BG_LoadScreenReq( PSTATUS_BG_PARAM );
  
  ribbonWork->isDisp = FALSE;

  //バーアイコン処理
  GFL_CLACT_WK_SetAnmSeq( work->clwkBarIcon[SBT_PAGE3] , SBA_PAGE3_NORMAL );
}

void PSTATUS_RIBBON_CreateRibbonBar( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  u8 i;
  u8 idx;
  u8 categoryCnt[RIBBON_CATEGORY_MAX] = {1,1,1,1,1};
  const POKEMON_PASO_PARAM *ppp = PSTATUS_UTIL_GetCurrentPPP( work );

  for( i=0;i<RIBBON_MAX;i++ )
  {
    ribbonWork->ribbonDataWork[i].isEnable = FALSE;
  }

  idx = 0;
  for( i=0;i<RIBBON_MAX;i++ )
  {
    const u8 ribbonNo = RibbonDispArr[i];
    const u32 checkId = RIBBON_DataGet( ribbonNo , RIBBON_PARA_POKEPARA );
    const u32 isHave = PPP_Get( ppp , checkId , NULL );

#if USE_STATUS_DEBUG
    if( isHave == 1 || work->isDevRibbon == TRUE )
#else
    if( isHave == 1 )
#endif
    {
      const u8 category = RIBBON_DataGet( ribbonNo , RIBBON_PARA_CATEGORY );
      ribbonWork->ribbonDataWork[idx].isEnable = TRUE;
      ribbonWork->ribbonDataWork[idx].ribbonNo = ribbonNo;
      ribbonWork->ribbonDataWork[idx].category = category;
      ribbonWork->ribbonDataWork[idx].catNo = categoryCnt[category];
      categoryCnt[category]++;
      idx++;
    }
  }

  ribbonWork->ribbonNum = idx;
  //リボンの番号のセット
  for( i=0 ; i<PSTATUS_RIBBON_BAR_NUM ; i++ )
  {
    ribbonWork->ribbonDispWork[i].dispRibbonNo = PSTATUS_RIBBON_GetRibbonType( ribbonWork , i );
  }
}


void PSTATUS_RIBBON_DeleteRibbonBar( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  /*
  u8 i;
  for( i=0;i<PSTATUS_RIBBON_BAR_NUM;i++ )
  {
  }
  */
}

static void PSTATUS_RIBBON_CreateRibbonBarFunc( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork , PSTATUS_RIBBON_DISP_WORK *ribbonDispWork )
{
  {
    u8 *srcData = ribbonWork->srcCellNcg->pRawData;
    u8 *chrAdr = GFL_BMP_GetCharacterAdrs( ribbonDispWork->bmpData );
    //選択されたリボンなので土台のキャラずらす
    if( ribbonDispWork->dispRibbonNo == ribbonWork->selectType )
    {
      srcData = (u8*)((u32)srcData + PSTATUS_RIBBON_BAR_CHARSIZE);
    }
    GFL_STD_MemCopy( srcData , chrAdr , PSTATUS_RIBBON_BAR_CHARSIZE );
  }
  {
    PSTATUS_RIBBON_DATA_WORK *ribbonData = &ribbonWork->ribbonDataWork[ribbonDispWork->dispRibbonNo];
    
    //FIXME ribbonDispWork->dispRibbonNo をバー表示用のMsg番号Idxに直す
    STRBUF *srcStr = GFL_MSG_CreateString( ribbonWork->ribbonMsg , mes_ribbon_category_01+ribbonData->category ); 
    STRBUF *dstStr = GFL_STR_CreateBuffer( 32, work->heapId );
    WORDSET *wordSet = WORDSET_Create( work->heapId );
    WORDSET_RegisterNumber( wordSet , 0 , ribbonData->catNo , 2 , STR_NUM_DISP_ZERO , STR_NUM_CODE_DEFAULT );
    WORDSET_ExpandStr( wordSet , dstStr , srcStr );
    //ここのフォントはOBJのパレットを使っているので注意！！！ 
    //個々では例外的にbmp直書きを使う
    GFL_FONTSYS_SetColor( 1,2,0 );
    PRINTSYS_Print( ribbonDispWork->bmpData , 10 , 6 , dstStr , work->fontHandle );
    GFL_FONTSYS_SetDefaultColor();
//    PRINTSYS_PrintQueColor( work->printQue , ribbonDispWork->bmpData , 
//            10 , 6 , dstStr , work->fontHandle , PRINTSYS_LSB_Make(1,2,0) );
    GFL_STR_DeleteBuffer( srcStr );
    GFL_STR_DeleteBuffer( dstStr );
    WORDSET_Delete( wordSet );
  }
  PSTA_OAM_ActorBmpTrans( ribbonDispWork->bmpOam );
//  ribbonDispWork->isUpdateStr = TRUE;
  
}

static void PSTATUS_RIBBON_UpdatRibbon( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  u8 i;
  BOOL isChange = FALSE;
  
  for( i=0;i<PSTATUS_RIBBON_BAR_NUM;i++ )
  {
    if( ribbonWork->isMoveRibbon == TRUE )
    {
      u16 ribbonType;
      s16 y = PSTATUS_RIBBON_CalcRibbonBarY( ribbonWork , i );
      PSTA_OAM_ActorSetPos( ribbonWork->ribbonDispWork[i].bmpOam , PSTATUS_RIBBON_BAR_X , y );
      
      ribbonType = PSTATUS_RIBBON_GetRibbonType( ribbonWork , i );
      if( ribbonType == PSTATUS_RIBBON_INVALID_TYPE )
      {
        ribbonWork->ribbonDispWork[i].dispRibbonNo = ribbonType;
        PSTA_OAM_ActorSetDrawEnable( ribbonWork->ribbonDispWork[i].bmpOam , FALSE );
      }
      else
      if( ribbonType != ribbonWork->ribbonDispWork[i].dispRibbonNo )
      {
        isChange = TRUE;
        ribbonWork->ribbonDispWork[i].dispRibbonNo = ribbonType;
        PSTATUS_RIBBON_CreateRibbonBarFunc( work , ribbonWork , 
                                &ribbonWork->ribbonDispWork[i] );
        PSTA_OAM_ActorSetDrawEnable( ribbonWork->ribbonDispWork[i].bmpOam , TRUE );
      }
    }

    if( ribbonWork->ribbonDispWork[i].isUpdateStr == TRUE )
    {
      if( PRINTSYS_QUE_IsExistTarget( work->printQue , ribbonWork->ribbonDispWork[i].bmpData ) == FALSE )
      {
        ribbonWork->ribbonDispWork[i].isUpdateStr = FALSE;
        PSTA_OAM_ActorBmpTrans( ribbonWork->ribbonDispWork[i].bmpOam );
      }
    }
  }
  
  if( isChange == TRUE &&
  ribbonWork->sndCnt == 0 )
  {
    PMSND_PlaySE_byPlayerID(PSTATUS_SND_SLIDE,SEPLAYER_SE1);
    ribbonWork->sndCnt = 3;
  }
      
  
  //前回idxの差分で見るとTP操作で10個飛ばしで動いたとき、変更が取れない！
//  if( ribbonWork->selectIdx != ribbonWork->befSelectIdx )
  if( ribbonWork->isUpdateIdx == TRUE )
  {
    if( ribbonWork->selectIdx < PSTATUS_RIBBON_BAR_NUM )
    {
      PSTATUS_RIBBON_CreateRibbonBarFunc( work , ribbonWork , 
                    &ribbonWork->ribbonDispWork[ribbonWork->selectIdx] );
    }
    if( ribbonWork->befSelectIdx < PSTATUS_RIBBON_BAR_NUM )
    {
      PSTATUS_RIBBON_CreateRibbonBarFunc( work , ribbonWork , 
                    &ribbonWork->ribbonDispWork[ribbonWork->befSelectIdx] );
    }
    ribbonWork->befSelectIdx = ribbonWork->selectIdx;
    ribbonWork->isUpdateIdx = FALSE;
  }
  
  ribbonWork->isMoveRibbon = FALSE;
}

//リボンが上から何番目か？
static u8 PSTATUS_RIBBON_GetRibbonIdx( PSTATUS_RIBBON_WORK *ribbonWork , const u8 idx )
{
  const u8 topIdx = ribbonWork->pagePos / PSTATUS_RIBBON_BAR_HEIGHT;
  const u8 topArrIdx = topIdx%PSTATUS_RIBBON_BAR_NUM;
  
  s8 subArrIdx = idx - topArrIdx;
  if( subArrIdx < -2 )
  {
    subArrIdx += PSTATUS_RIBBON_BAR_NUM;
  }
  if( subArrIdx > PSTATUS_RIBBON_BAR_NUM-2 )
  {
    subArrIdx -= PSTATUS_RIBBON_BAR_NUM;
  }
  
  return topIdx + subArrIdx;
}

//リボンの種類を求める
static u16 PSTATUS_RIBBON_GetRibbonType( PSTATUS_RIBBON_WORK *ribbonWork , const u8 idx )
{
  const u8 ribbonIdx = PSTATUS_RIBBON_GetRibbonIdx( ribbonWork , idx );
  if( ribbonIdx >= ribbonWork->ribbonNum )
  {
    return PSTATUS_RIBBON_INVALID_TYPE;
  }
  else
  {
    return ribbonIdx;
  }
}

//リボンバーの座標を求める
static s16 PSTATUS_RIBBON_CalcRibbonBarY( PSTATUS_RIBBON_WORK *ribbonWork , const u8 idx )
{
  const u8 topIdx = ribbonWork->pagePos / PSTATUS_RIBBON_BAR_HEIGHT;
  const u8 modPos = ribbonWork->pagePos % PSTATUS_RIBBON_BAR_HEIGHT;
  const u8 topArrIdx = topIdx%PSTATUS_RIBBON_BAR_NUM;
  
  s8 subArrIdx = idx - topArrIdx;
  if( subArrIdx < -2 )
  {
    subArrIdx += PSTATUS_RIBBON_BAR_NUM;
  }
  if( subArrIdx > PSTATUS_RIBBON_BAR_DIPS_NUM+2 && topIdx > PSTATUS_RIBBON_BAR_DIPS_NUM)
  {
    subArrIdx -= PSTATUS_RIBBON_BAR_NUM;
  }
  
  return PSTATUS_RIBBON_BAR_Y + subArrIdx*PSTATUS_RIBBON_BAR_HEIGHT - modPos;
}

#pragma mark [>DispInfo
//--------------------------------------------------------------
//	リボン詳細の表示
//--------------------------------------------------------------
static void PSTATUS_RIBBON_DispInfo( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  
  ribbonWork->bmpWinName = GFL_BMPWIN_Create( PSTATUS_BG_SUB_STR ,
              PSTATUS_RIBBON_NAME_WIN_LEFT , PSTATUS_RIBBON_NAME_WIN_TOP ,
              PSTATUS_RIBBON_NAME_WIN_WIDTH , PSTATUS_RIBBON_NAME_WIN_HEIGHT ,
              PSTATUS_BG_SUB_PLT_FONT , GFL_BMP_CHRAREA_GET_B );
  
  ribbonWork->bmpWinInfo = GFL_BMPWIN_Create( PSTATUS_BG_SUB_STR ,
              PSTATUS_RIBBON_INFO_WIN_LEFT , PSTATUS_RIBBON_INFO_WIN_TOP ,
              PSTATUS_RIBBON_INFO_WIN_WIDTH , PSTATUS_RIBBON_INFO_WIN_HEIGHT ,
              PSTATUS_BG_SUB_PLT_FONT , GFL_BMP_CHRAREA_GET_B );
  //文字列
  {
    const u16 ribbonNo = ribbonWork->ribbonDataWork[ribbonWork->selectType].ribbonNo;
    //名前
    {
      const u32 msgId = RIBBON_DataGet( ribbonNo , RIBBON_PARA_NAME );
      STRBUF *srcStr;
      srcStr = GFL_MSG_CreateString( ribbonWork->ribbonMsg , msgId ); 
      PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( ribbonWork->bmpWinName ) , 
              PSTATUS_RIBBON_NAME_STR_X , PSTATUS_RIBBON_NAME_STR_Y , srcStr , 
              work->fontHandle , PSTATUS_STR_COL_BLACK );
      GFL_STR_DeleteBuffer( srcStr );
    }
    //説明
    {
      const u32 msgId = RIBBON_InfoGet( ribbonNo , NULL );
      STRBUF *srcStr;
      srcStr = GFL_MSG_CreateString( ribbonWork->ribbonMsg , msgId ); 
      PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( ribbonWork->bmpWinInfo ) , 
              PSTATUS_RIBBON_INFO_STR_X , PSTATUS_RIBBON_INFO_STR_Y , srcStr , 
              work->fontHandle , PSTATUS_STR_COL_BLACK );
      GFL_STR_DeleteBuffer( srcStr );
    }
  }
  
  ribbonWork->isUpdateStrInfo = TRUE;
}

//--------------------------------------------------------------
//	リボン詳細の表示
//--------------------------------------------------------------
static void PSTATUS_RIBBON_DispInfo_Trans( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  const u16 ribbonNo = ribbonWork->ribbonDataWork[ribbonWork->selectType].ribbonNo;
  GFL_BMPWIN_MakeTransWindow_VBlank( ribbonWork->bmpWinName );
  GFL_BMPWIN_MakeTransWindow_VBlank( ribbonWork->bmpWinInfo );
  //アイコン
  {
    ARCHANDLE *archandle = GFL_ARC_OpenDataHandle( ARCID_P_STATUS , work->heapId );
    GFL_CLWK_DATA cellInitData;
    
    ribbonWork->ribbonIconNcg = GFL_CLGRP_CGR_Register( archandle , 
        RIBBON_DataGet( ribbonNo , RIBBON_PARA_GRAPHIC ) , 
        FALSE , CLSYS_DRAW_SUB , work->heapId  );

    GFL_ARC_CloseDataHandle(archandle);

    cellInitData.pos_x = PSTATUS_RIBBON_ICON_X;
    cellInitData.pos_y = PSTATUS_RIBBON_ICON_Y;
    cellInitData.softpri = 10;
    cellInitData.bgpri = 0;
    cellInitData.anmseq = 0;
    
    ribbonWork->clwkRibbonIcon = GFL_CLACT_WK_Create( work->cellUnit ,
              ribbonWork->ribbonIconNcg,
              work->cellRes[SCR_PLT_RIBBON_ICON],
              work->cellRes[SCR_ANM_RIBBON_ICON],
              &cellInitData ,CLSYS_DEFREND_SUB , work->heapId );
    GFL_CLACT_WK_SetPlttOffs( ribbonWork->clwkRibbonIcon , 
                            RIBBON_DataGet( ribbonNo , RIBBON_PARA_PALNUM ) , 
                            CLWK_PLTTOFFS_MODE_PLTT_TOP );
    GFL_CLACT_WK_SetDrawEnable( ribbonWork->clwkRibbonIcon , TRUE );
  }

  GFL_BG_LoadScreenBuffer( PSTATUS_BG_SUB_PLATE, 
                     ribbonWork->scrDataUpDetail->rawData, 
                     ribbonWork->scrDataUpDetail->szByte );
  GFL_BG_LoadScreenV_Req( PSTATUS_BG_SUB_PLATE );

  ribbonWork->isDispInfo = TRUE;
}

//--------------------------------------------------------------
//	リボン詳細の消去
//--------------------------------------------------------------
static void PSTATUS_RIBBON_ClearInfo( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  if( ribbonWork->isDispInfo == TRUE )
  {
    GFL_BMPWIN_Delete( ribbonWork->bmpWinName );
    GFL_BMPWIN_Delete( ribbonWork->bmpWinInfo );
  }
}

//--------------------------------------------------------------
//	リボン詳細の消去
//--------------------------------------------------------------
static void PSTATUS_RIBBON_ClearInfo_Trans( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  if( ribbonWork->isDispInfo == TRUE )
  {
    GFL_CLGRP_CGR_Release( ribbonWork->ribbonIconNcg );
    GFL_CLACT_WK_Remove( ribbonWork->clwkRibbonIcon );
    GFL_BG_ClearScreenCodeVReq( PSTATUS_BG_SUB_STR , 0 );
    GFL_BG_LoadScreenV_Req( PSTATUS_BG_SUB_STR );
    ribbonWork->isDispInfo = FALSE;

    GFL_BG_LoadScreenBuffer( PSTATUS_BG_SUB_PLATE, 
                       ribbonWork->scrDataUp->rawData, 
                       ribbonWork->scrDataUp->szByte );
    GFL_BG_LoadScreenV_Req( PSTATUS_BG_SUB_PLATE );
  }
}
