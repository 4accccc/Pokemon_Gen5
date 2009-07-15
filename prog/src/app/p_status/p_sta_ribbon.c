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

#include "p_status_gra.naix"

#include "p_sta_sys.h"
#include "p_sta_ribbon.h"
#include "p_sta_oam.h"

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
  GFL_CLWK *clwkRibbonBar;
  
}PSTATUS_RIBBON_DISP_WORK;

struct _PSTATUS_RIBBON_WORK
{
  BOOL isDisp;
  BOOL isMoveRibbon;
  
  u8  ribbonNum;
  u32 pagePos;

  NNSG2dScreenData *scrDataDown;
  void *scrResDown;
  NNSG2dScreenData *scrDataUp;
  void *scrResUp;
  NNSG2dCharacterData *srcCellNcg;
  void *resCellNcg;

  //カーソル系
  GFL_CLWK *clwkCur;
  u8       selectIdx;
  u8       selectType;
  u8       befSelectIdx;
  s32      speed;
  
  BOOL     isTouchBar;
  
  
  PSTA_OAM_SYS_PTR bmpOamSys;
  PSTATUS_RIBBON_DISP_WORK ribbonDispWork[PSTATUS_RIBBON_BAR_NUM];
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

//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
PSTATUS_RIBBON_WORK* PSTATUS_RIBBON_Init( PSTATUS_WORK *work )
{
  PSTATUS_RIBBON_WORK* ribbonWork;
  
  ribbonWork = GFL_HEAP_AllocMemory( work->heapId , sizeof(PSTATUS_RIBBON_WORK) );
  ribbonWork->isDisp = FALSE;
  return ribbonWork;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
void PSTATUS_RIBBON_Term( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  GFL_HEAP_FreeMemory( ribbonWork );
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
void PSTATUS_RIBBON_Main( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  PSTATUS_RIBBON_UpdateUI( work , ribbonWork );
  PSTATUS_RIBBON_UpdatRibbon( work , ribbonWork );
}

#pragma mark [>Resource
//--------------------------------------------------------------
//	リソース読み込み
//--------------------------------------------------------------
void PSTATUS_RIBBON_LoadResource( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork , ARCHANDLE *archandle )
{
  u8 i;
  //書き換え用スクリーン読み込み
  ribbonWork->scrResDown = GFL_ARCHDL_UTIL_LoadScreen( archandle , NARC_p_status_gra_p_st_info_d_NSCR ,
                    FALSE , &ribbonWork->scrDataDown , work->heapId );
  ribbonWork->scrResUp = GFL_ARCHDL_UTIL_LoadScreen( archandle , NARC_p_status_gra_p_st_info_u_NSCR ,
                    FALSE , &ribbonWork->scrDataUp , work->heapId );

  //書き換え用OBJキャラクタ読み込み
  ribbonWork->resCellNcg = GFL_ARCHDL_UTIL_LoadOBJCharacter( archandle , NARC_p_status_gra_p_status_ribbon_bar_NCGR ,
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
    cellInitData.bgpri = 1;
    cellInitData.anmseq = 0;
    
    ribbonWork->clwkCur = GFL_CLACT_WK_Create( work->cellUnit ,
              work->cellRes[SCR_NCG_RIBBON_CUR],
              work->cellRes[SCR_PLT_CURSOR_COMMON],
              work->cellRes[SCR_ANM_RIBBON_CUR],
              &cellInitData ,CLSYS_DEFREND_MAIN , work->heapId );

    GFL_CLACT_WK_SetDrawEnable( ribbonWork->clwkCur , FALSE );
  }  
}

void PSTATUS_RIBBON_TermCell( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  GFL_CLACT_WK_Remove( ribbonWork->clwkCur );
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
        GFL_CLACT_WK_SetDrawEnable( ribbonWork->clwkCur , TRUE );
        work->ktst = GFL_APP_END_KEY;
      }
      else
      {
        ribbonWork->isTouchBar = TRUE;
        ribbonWork->selectIdx = touchBar;
        ribbonWork->selectType = PSTATUS_RIBBON_GetRibbonType( ribbonWork , touchBar );
        PSTATUS_SetActiveBarButton( work , FALSE );
        work->ktst = GFL_APP_END_TOUCH;
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
    GFL_CLACT_WK_SetDrawEnable( ribbonWork->clwkCur , FALSE );
    ribbonWork->selectIdx = 0xFF;
    ribbonWork->selectType = PSTATUS_RIBBON_INVALID_TYPE;
    work->ktst = GFL_APP_END_KEY;
    return TRUE;
  }
  else
  if( work->ktst == GFL_APP_END_TOUCH )
  {
    if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_A )
    {
      ribbonWork->speed = 0;
      PSTATUS_RIBBON_SetCursorTopBar( work , ribbonWork );
      GFL_CLACT_WK_SetDrawEnable( ribbonWork->clwkCur , TRUE );
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
        if( PSTATUS_RIBBON_CalcRibbonBarY( ribbonWork , ribbonWork->selectIdx ) > bottomPos )
        {
          const u8 sub = PSTATUS_RIBBON_CalcRibbonBarY( ribbonWork , ribbonWork->selectIdx ) - bottomPos;
          PSTATUS_RIBBON_MoveBar( work , ribbonWork , sub );
        }
        PSTATUS_RIBBON_SetCursorPosBar( work , ribbonWork , ribbonWork->selectIdx );
      }
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
        if( PSTATUS_RIBBON_CalcRibbonBarY( ribbonWork , ribbonWork->selectIdx ) < PSTATUS_RIBBON_BAR_Y )
        {
          const s16 sub = PSTATUS_RIBBON_CalcRibbonBarY( ribbonWork , ribbonWork->selectIdx ) - PSTATUS_RIBBON_BAR_Y;
          PSTATUS_RIBBON_MoveBar( work , ribbonWork , sub );
        }
        PSTATUS_RIBBON_SetCursorPosBar( work , ribbonWork , ribbonWork->selectIdx );
      }
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
    GFL_CLACT_WK_SetDrawEnable( ribbonWork->clwkCur , FALSE );
    ribbonWork->selectIdx = 0xFF;
    ribbonWork->selectType = PSTATUS_RIBBON_INVALID_TYPE;
    work->ktst = GFL_APP_END_TOUCH;
  }
  else
  if( GFL_UI_TP_GetTrg() == TRUE )
  {
    const int touchBar = PSTATUS_RIBBON_CheckTouchBar( work , ribbonWork );
    if( touchBar != GFL_UI_TP_HIT_NONE )
    {
      ribbonWork->selectIdx = touchBar;
      ribbonWork->selectType = PSTATUS_RIBBON_GetRibbonType( ribbonWork , touchBar );
      ribbonWork->isTouchBar = TRUE;
      GFL_CLACT_WK_SetDrawEnable( ribbonWork->clwkCur , FALSE );
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
    PSTATUS_RIBBON_MoveBar( work , ribbonWork , tpSub );
    ribbonWork->speed = tpSub*PSTATUS_RIBBON_BAR_SPEED_RATE;
  }
  else
  {
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
  //Window下地の張替え
  GFL_BG_WriteScreenExpand( PSTATUS_BG_PLATE , 
                    0 , 0 , PSTATUS_MAIN_PAGE_WIDTH , 24 ,
                    ribbonWork->scrDataDown->rawData ,
                    0 , 0 , 32 , 32 );
  GFL_BG_LoadScreenV_Req( PSTATUS_BG_PLATE );

  GFL_BG_LoadScreen( PSTATUS_BG_SUB_PLATE, 
                     ribbonWork->scrDataUp->rawData, 
                     ribbonWork->scrDataUp->szByte, 
                     0 );
  GFL_BG_LoadScreenBuffer( PSTATUS_BG_SUB_PLATE, 
                     ribbonWork->scrDataUp->rawData, 
                     ribbonWork->scrDataUp->szByte );
  GFL_BG_LoadScreenV_Req( PSTATUS_BG_SUB_PLATE );
  

  for( i=0 ; i<PSTATUS_RIBBON_BAR_NUM ; i++ )
  {
    PSTA_OAM_ActorSetDrawEnable( ribbonWork->ribbonDispWork[i].bmpOam , TRUE );
  }

  ribbonWork->pagePos = 0;
  ribbonWork->selectIdx = 0xFF;
  ribbonWork->selectType = PSTATUS_RIBBON_INVALID_TYPE;

  ribbonWork->befSelectIdx = 0xFF;
  ribbonWork->isDisp = TRUE;
  ribbonWork->isMoveRibbon = TRUE;
  
}

//--------------------------------------------------------------
//	ページのクリア
//--------------------------------------------------------------
void PSTATUS_RIBBON_ClearPage( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  u8 i;
  for( i=0 ; i<PSTATUS_RIBBON_BAR_NUM ; i++ )
  {
    PSTA_OAM_ActorSetDrawEnable( ribbonWork->ribbonDispWork[i].bmpOam , FALSE );
  }
  
  GFL_BG_FillScreen( PSTATUS_BG_PARAM , 0 , 0 , 0 , 
                     PSTATUS_MAIN_PAGE_WIDTH , 21 ,
                     GFL_BG_SCRWRT_PALNL );
  GFL_BG_LoadScreenV_Req( PSTATUS_BG_PARAM );
  
  ribbonWork->isDisp = FALSE;


}

void PSTATUS_RIBBON_CreateRibbonBar( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  u8 i;

  //TODO 仮
  ribbonWork->ribbonNum = 80;

  //リボンの番号のセット
  for( i=0 ; i<PSTATUS_RIBBON_BAR_NUM ; i++ )
  {
    ribbonWork->ribbonDispWork[i].dispRibbonNo = PSTATUS_RIBBON_GetRibbonType( ribbonWork , i );
  }

  //セルの作成
  {
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

      PSTATUS_RIBBON_CreateRibbonBarFunc( work , ribbonWork , &ribbonWork->ribbonDispWork[i] );
      PSTA_OAM_ActorSetDrawEnable( ribbonWork->ribbonDispWork[i].bmpOam , FALSE );
    }
  }
}

void PSTATUS_RIBBON_DeleteRibbonBar( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  u8 i;
  for( i=0;i<PSTATUS_RIBBON_BAR_NUM;i++ )
  {
    //GFL_CLACT_WK_Remove( ribbonWork->ribbonDispWork[i].clwkRibbonBar );
    GFL_BMP_Clear( ribbonWork->ribbonDispWork[i].bmpData , 0 );
    PSTA_OAM_ActorBmpTrans( ribbonWork->ribbonDispWork[i].bmpOam );

    PSTA_OAM_ActorDel( ribbonWork->ribbonDispWork[i].bmpOam );
    GFL_BMP_Delete( ribbonWork->ribbonDispWork[i].bmpData );
  }
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
    STRBUF *srcStr = GFL_MSG_CreateString( work->msgHandle , mes_status_test_1 ); 
    STRBUF *dstStr = GFL_STR_CreateBuffer( 32, work->heapId );
    WORDSET *wordSet = WORDSET_Create( work->heapId );
    WORDSET_RegisterNumber( wordSet , 0 , ribbonDispWork->dispRibbonNo , 2 , STR_NUM_DISP_ZERO , STR_NUM_CODE_DEFAULT );
    WORDSET_ExpandStr( wordSet , dstStr , srcStr );
    //ここのフォントはOBJのパレットを使っているので注意！！！ 
    PRINTSYS_PrintQueColor( work->printQue , ribbonDispWork->bmpData , 
            10 , 6 , dstStr , work->fontHandle , PRINTSYS_LSB_Make(1,2,0) );
    GFL_STR_DeleteBuffer( srcStr );
    GFL_STR_DeleteBuffer( dstStr );
    WORDSET_Delete( wordSet );
  }
  ribbonDispWork->isUpdateStr = TRUE;
  
}

static void PSTATUS_RIBBON_UpdatRibbon( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  u8 i;
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
  
  if( ribbonWork->selectIdx != ribbonWork->befSelectIdx )
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
    return ribbonIdx+1;
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