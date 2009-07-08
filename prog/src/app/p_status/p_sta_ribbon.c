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

//セル系
#define PSTATUS_RIBBON_BAR_NUM (10)

#define PSTATUS_RIBBON_BAR_X   (8)
#define PSTATUS_RIBBON_BAR_Y   (8)
#define PSTATUS_RIBBON_BAR_WIDTH   (136)
#define PSTATUS_RIBBON_BAR_HEIGHT  (24)

#define PSTATUS_RIBBON_BAR_CHARSIZE  (17*3*0x20)


//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
struct _PSTATUS_RIBBON_WORK
{
  BOOL isDisp;
  
  u8  ribbonNum;
  u32 pagePos;

  NNSG2dScreenData *scrDataDown;
  void *scrResDown;
  NNSG2dScreenData *scrDataUp;
  void *scrResUp;
  NNSG2dCharacterData *srcCellNcg;
  void *resCellNcg;
  
  u32 cellRes[2];
  u32 cellResNcg[PSTATUS_RIBBON_BAR_NUM];
  GFL_CLWK *clwkRibbonBar[PSTATUS_RIBBON_BAR_NUM];

  PSTA_OAM_SYS_PTR bmpOamSys;
  BOOL isUpdateStr[PSTATUS_RIBBON_BAR_NUM];
  GFL_BMP_DATA *bmpData[PSTATUS_RIBBON_BAR_NUM];
  PSTA_OAM_ACT_PTR bmpOam[PSTATUS_RIBBON_BAR_NUM];
  u8  dispRibbonNo[PSTATUS_RIBBON_BAR_NUM];
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void PSTATUS_RIBBON_CreateRibbonBarFunc( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork , const u8 idx );
static void PSTATUS_RIBBON_UpdatRibbon( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork );
static u8 PSTATUS_RIBBON_GetRibbonIdx( PSTATUS_RIBBON_WORK *ribbonWork , const u8 idx );
static u8 PSTATUS_RIBBON_GetRibbonType( PSTATUS_RIBBON_WORK *ribbonWork , const u8 idx );
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
  BOOL isUpdatePos = FALSE;
  u8 value = 2;
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X )
  {
    value = 24;
  } 
  
  if( GFL_UI_KEY_GetCont() & PAD_KEY_DOWN &&
      GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
  {
    if( ribbonWork->pagePos < value )
    {
      ribbonWork->pagePos = 0;
      ribbonWork->pagePos = TRUE;
    }
    else
    if( ribbonWork->pagePos > 0 )
    {
      ribbonWork->pagePos -= value;
      isUpdatePos = TRUE;
    }
  }
  else
  if( GFL_UI_KEY_GetCont() & PAD_KEY_UP &&
      GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
  {
    if( ribbonWork->pagePos < ribbonWork->ribbonNum*PSTATUS_RIBBON_BAR_HEIGHT )
    {
      ribbonWork->pagePos += value;
      isUpdatePos = TRUE;
    }
  }
  
  if( isUpdatePos == TRUE )
  {
    PSTATUS_RIBBON_UpdatRibbon( work , ribbonWork );
  }
  {
    u8 i;
    for( i=0;i<PSTATUS_RIBBON_BAR_NUM;i++ )
    {
      if( ribbonWork->isUpdateStr[i] == TRUE )
      {
        if( PRINTSYS_QUE_IsExistTarget( work->printQue , ribbonWork->bmpData[i] ) == FALSE )
        {
          ribbonWork->isUpdateStr[i] = FALSE;
          PSTA_OAM_ActorBmpTrans( ribbonWork->bmpOam[i] );
        }
      }
    }
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
  ribbonWork->scrResDown = GFL_ARCHDL_UTIL_LoadScreen( archandle , NARC_p_status_gra_p_status_ribbon_NSCR ,
                    FALSE , &ribbonWork->scrDataDown , work->heapId );
  ribbonWork->scrResUp = GFL_ARCHDL_UTIL_LoadScreen( archandle , NARC_p_status_gra_p_status_ribbon_u_NSCR ,
                    FALSE , &ribbonWork->scrDataUp , work->heapId );

  //書き換え用OBJキャラクタ読み込み
  ribbonWork->resCellNcg = GFL_ARCHDL_UTIL_LoadOBJCharacter( archandle , NARC_p_status_gra_p_status_ribbon_bar_NCGR ,
                    FALSE , &ribbonWork->srcCellNcg , work->heapId );

  //パレット
  ribbonWork->cellRes[0] = GFL_CLGRP_PLTT_Register( archandle , 
        NARC_p_status_gra_p_status_ribbon_bar_NCLR , CLSYS_DRAW_MAIN , 
        PSTATUS_OBJPLT_RIBBON_BAR*32 , work->heapId  );
/*
  //セル・アニメ
  ribbonWork->cellRes[1] = GFL_CLGRP_CELLANIM_Register( archandle , 
        NARC_p_status_gra_p_status_ribbon_bar_NCER , NARC_p_status_gra_p_status_ribbon_bar_NANR, work->heapId  );

  //キャラクタ
  for( i=0;i<PSTATUS_RIBBON_BAR_NUM;i++ )
  {
    ribbonWork->cellResNcg[i] = GFL_CLGRP_CGR_Register( archandle , 
          NARC_p_status_gra_p_status_ribbon_bar_NCGR , FALSE , CLSYS_DRAW_MAIN , work->heapId  );
  }
*/  
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

  GFL_CLGRP_PLTT_Release( ribbonWork->cellRes[0] );
/*  
  GFL_CLGRP_CELLANIM_Release( ribbonWork->cellRes[1] );
  for( i=0 ; i<PSTATUS_RIBBON_BAR_NUM ; i++ )
  {
    GFL_CLGRP_CGR_Release( ribbonWork->cellResNcg[i] );
  }
*/

  GFL_HEAP_FreeMemory( ribbonWork->resCellNcg );
  GFL_HEAP_FreeMemory( ribbonWork->scrResDown );
  GFL_HEAP_FreeMemory( ribbonWork->scrResUp );

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
    PSTA_OAM_ActorSetDrawEnable( ribbonWork->bmpOam[i] , TRUE );
  }

  ribbonWork->isDisp = TRUE;
  
  //TODO 仮
  ribbonWork->ribbonNum = 30;
  ribbonWork->pagePos = 0;

}

//--------------------------------------------------------------
//	ページのクリア
//--------------------------------------------------------------
void PSTATUS_RIBBON_ClearPage( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  u8 i;
  for( i=0 ; i<PSTATUS_RIBBON_BAR_NUM ; i++ )
  {
    PSTA_OAM_ActorSetDrawEnable( ribbonWork->bmpOam[i] , FALSE );
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
  //リボンの番号のセット
  for( i=0 ; i<PSTATUS_RIBBON_BAR_NUM ; i++ )
  {
    ribbonWork->dispRibbonNo[i] = PSTATUS_RIBBON_GetRibbonType( ribbonWork , i );
  }

  //セルの作成
  {
/*
    GFL_CLWK_DATA cellInitData;
    
    cellInitData.pos_x = PSTATUS_RIBBON_BAR_X;
    cellInitData.pos_y = PSTATUS_RIBBON_BAR_Y;
    cellInitData.softpri = 10;
    cellInitData.bgpri = 2;
    cellInitData.anmseq = 0;

    for( i=0 ; i<PSTATUS_RIBBON_BAR_NUM ; i++ )
    {
      ribbonWork->clwkRibbonBar[i] = GFL_CLACT_WK_Create( work->cellUnit ,
                ribbonWork->cellResNcg[i],
                ribbonWork->cellRes[SCR_PLT_ICON],
                ribbonWork->cellRes[SCR_ANM_ICON],
                &cellInitData ,CLSYS_DEFREND_MAIN , work->heapId );
      
      GFL_CLACT_WK_SetDrawEnable( ribbonWork->clwkRibbonBar[i] , TRUE );
      cellInitData.pos_y += PSTATUS_RIBBON_BAR_HEIGHT;
    }
*/
    
    PSTA_OAM_ACT_DATA oamData;
    oamData.x = PSTATUS_RIBBON_BAR_X;
    oamData.pltt_index = ribbonWork->cellRes[0];
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

      ribbonWork->bmpData[i] = GFL_BMP_CreateInVRAM( vramAdr , 
                                            PSTATUS_RIBBON_BAR_WIDTH/8 , 
                                            PSTATUS_RIBBON_BAR_HEIGHT/8 ,
                                            GFL_BMP_16_COLOR , work->heapId );

      oamData.bmp = ribbonWork->bmpData[i];
      ribbonWork->bmpOam[i] = PSTA_OAM_ActorAdd( ribbonWork->bmpOamSys , &oamData );

      PSTATUS_RIBBON_CreateRibbonBarFunc( work , ribbonWork , i );
      PSTA_OAM_ActorSetDrawEnable( ribbonWork->bmpOam[i] , FALSE );
    }
  }
}

void PSTATUS_RIBBON_DeleteRibbonBar( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  u8 i;
  for( i=0;i<PSTATUS_RIBBON_BAR_NUM;i++ )
  {
    //GFL_CLACT_WK_Remove( ribbonWork->clwkRibbonBar[i] );
    PSTA_OAM_ActorDel( ribbonWork->bmpOam[i] );
    GFL_BMP_Delete( ribbonWork->bmpData[i] );
  }
}

static void PSTATUS_RIBBON_CreateRibbonBarFunc( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork , const u8 idx )
{
  {
    u8 *chrAdr = GFL_BMP_GetCharacterAdrs( ribbonWork->bmpData[idx] );
    GFL_STD_MemCopy( ribbonWork->srcCellNcg->pRawData , chrAdr , PSTATUS_RIBBON_BAR_CHARSIZE );
  }
  {
    STRBUF *srcStr = GFL_MSG_CreateString( work->msgHandle , mes_status_test_1 ); 
    STRBUF *dstStr = GFL_STR_CreateBuffer( 32, work->heapId );
    WORDSET *wordSet = WORDSET_Create( work->heapId );
    WORDSET_RegisterNumber( wordSet , 0 , ribbonWork->dispRibbonNo[idx] , 2 , STR_NUM_DISP_ZERO , STR_NUM_CODE_DEFAULT );
    WORDSET_ExpandStr( wordSet , dstStr , srcStr );
    PRINTSYS_PrintQueColor( work->printQue , ribbonWork->bmpData[idx] , 
            10 , 6 , dstStr , work->fontHandle , PRINTSYS_LSB_Make(1,2,0) );
    GFL_STR_DeleteBuffer( srcStr );
    GFL_STR_DeleteBuffer( dstStr );
    WORDSET_Delete( wordSet );
  }
  ribbonWork->isUpdateStr[idx] = TRUE;
  
}

static void PSTATUS_RIBBON_UpdatRibbon( PSTATUS_WORK *work , PSTATUS_RIBBON_WORK *ribbonWork )
{
  u8 i;
  for( i=0;i<PSTATUS_RIBBON_BAR_NUM;i++ )
  {
    u8 ribbonType;
    s16 y = PSTATUS_RIBBON_CalcRibbonBarY( ribbonWork , i );
    PSTA_OAM_ActorSetPos( ribbonWork->bmpOam[i] , PSTATUS_RIBBON_BAR_X , y );
    
    ribbonType = PSTATUS_RIBBON_GetRibbonType( ribbonWork , i );
    if( ribbonType != ribbonWork->dispRibbonNo[i] )
    {
      ribbonWork->dispRibbonNo[i] = ribbonType;
      PSTATUS_RIBBON_CreateRibbonBarFunc( work , ribbonWork , i );
    }
  }
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
  
  return topIdx + subArrIdx;
}

//リボンの種類を求める
static u8 PSTATUS_RIBBON_GetRibbonType( PSTATUS_RIBBON_WORK *ribbonWork , const u8 idx )
{
  return PSTATUS_RIBBON_GetRibbonIdx( ribbonWork , idx );
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
  
  return PSTATUS_RIBBON_BAR_Y + subArrIdx*PSTATUS_RIBBON_BAR_HEIGHT - modPos;
}