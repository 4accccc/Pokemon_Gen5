//======================================================================
/**
 * @file	p_sta_sub.c
 * @brief	ポケモンステータス 右側情報画面
 * @author	ariizumi
 * @data	09/06/10
 *
 * モジュール名：PSTATUS_SUB
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/wipe.h"
#include "print/printsys.h"
#include "print/wordset.h"

#include "arc_def.h"

#include "p_sta_sys.h"
#include "p_sta_sub.h"

#include "test/ariizumi/ari_debug.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define PSTATUS_MCSS_POS_X (FX32_CONST(( 200+16 )/16.0f))
#define PSTATUS_MCSS_POS_Y (FX32_CONST((192.0f-( 120 ))/16.0f))

//座標系
//上用Win
#define PSTATUS_SUB_UPWIN_LEFT (19)
#define PSTATUS_SUB_UPWIN_TOP  ( 0)
#define PSTATUS_SUB_UPWIN_WIDTH  (13)
#define PSTATUS_SUB_UPWIN_HEIGHT ( 4)

//下用Win
#define PSTATUS_SUB_DOWNWIN_LEFT (19)
#define PSTATUS_SUB_DOWNWIN_TOP  (17)
#define PSTATUS_SUB_DOWNWIN_WIDTH  (13)
#define PSTATUS_SUB_DOWNWIN_HEIGHT ( 4)

//文字列共通オフセット
#define PSTATUS_STR_OFS_X (2)
#define PSTATUS_STR_OFS_Y (2)

//上Win用文字座標
#define PSTATUS_SUB_STR_NAME_X (24 + PSTATUS_STR_OFS_X)
#define PSTATUS_SUB_STR_NAME_Y ( 0 + PSTATUS_STR_OFS_Y)
#define PSTATUS_SUB_STR_SEX_X (88 + PSTATUS_STR_OFS_X)
#define PSTATUS_SUB_STR_SEX_Y ( 0 + PSTATUS_STR_OFS_Y)
#define PSTATUS_SUB_STR_LV_X ( 8 + PSTATUS_STR_OFS_X)
#define PSTATUS_SUB_STR_LV_Y (16 + PSTATUS_STR_OFS_Y)

//下Win用文字座標
#define PSTATUS_SUB_STR_ITEM_X ( 8 + PSTATUS_STR_OFS_X)
#define PSTATUS_SUB_STR_ITEM_Y ( 0 + PSTATUS_STR_OFS_Y)
#define PSTATUS_SUB_STR_ITEMNAME_X ( 8 + PSTATUS_STR_OFS_X)
#define PSTATUS_SUB_STR_ITEMNAME_Y (16 + PSTATUS_STR_OFS_Y)

//セル系座標
#define PSTATUS_SUB_CELL_BALL_X ( 168 )
#define PSTATUS_SUB_CELL_BALL_Y (   8 )

//ポケのタッチ範囲
#define PSTATUS_SUB_TOUCH_TOP ( 32)
#define PSTATUS_SUB_TOUCH_BOTTOM ( 120)
#define PSTATUS_SUB_TOUCH_LEFT ( 152)
#define PSTATUS_SUB_TOUCH_RIGHT ( 255)

//カメラ定義
#define PSTATUS_SUB_CAMERA_FRONT_X (FX32_CONST(-41.0f))
#define PSTATUS_SUB_CAMERA_BACK_X (FX32_CONST(-66.0f))


//くすぐり系定義
#define PSTATUS_SUB_TICKLE_WIDHT (8)  //くすぐり幅
#define PSTATUS_SUB_TICKLE_TIME (20)  //くすぐり有効時間
#define PSTATUS_SUB_TICKLE_NUM (4)  //くすぐり回数

//ジャンプ系定義
#define PSTATUS_SUB_JUMP_TIME (10)
#define PSTATUS_SUB_JUMP_HEIGHT (2)  //FX16にかけるので整数でOK

//前→後ろアニメ定義
#define PSTATUS_SUB_FLIP_TIME (3)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

typedef enum
{
  PSS_DISP_FORNT,
  PSS_FRONT_TO_BACK,
  PSS_DISP_BACK,
  PSS_BACK_TO_FRONT,
}PSTATUS_SUB_STATE;

enum
{
  SSMT_BOX_DISC,
  SSMT_BOX_TRIANGLE,
  SSMT_BOX_SQUARE,
  SSMT_BOX_HEART,
  SSMT_BOX_STAR,
  SSMT_BOX_DIAMOND,
  SSMT_MARK_RARE,
  SSMT_MARK_POKERUS,
  
  SSMT_MAX,
}PSTAUTS_SUB_MARK_TYPE;

//くすぐりの方向
typedef enum
{
  SSTD_NONE,  //初回
  SSTD_LEFT,  //左に行った
  SSTD_RIGHT, //右に行った
  
}PSTATUS_SUB_TICKLE_DIR;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
struct _PSTATUS_SUB_WORK
{
  MCSS_WORK     *pokeMcss;
  MCSS_WORK     *pokeMcssBack;
  
  PSTATUS_SUB_STATE state;
  BOOL  isDispFront;
  
  GFL_BMPWIN  *bmpWinUpper;
  GFL_BMPWIN  *bmpWinDown;
  
  GFL_CLWK    *clwkBall;
  GFL_CLWK    *clwkMark[SSMT_MAX];
  
  //TPくすぐり操作用
  BOOL        IsHoldTp;
  PSTATUS_SUB_TICKLE_DIR tickleDir;
  u32         befTpx;
  u32         befTpy;
  u8          holdCnt;
  u8          tickleNum;
  
  //ジャンプ演出管理
  u8          flipAnimeCnt;

};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void PSTATUS_SUB_DrawStr( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork , const POKEMON_PASO_PARAM *ppp );

static void PSTATUS_SUB_UpdateTP( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork );

static void PSTATUS_SUB_PokeCreateMcss( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork , const POKEMON_PASO_PARAM *ppp );
static void PSTATUS_SUB_PokeDeleteMcss( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork );

//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
PSTATUS_SUB_WORK* PSTATUS_SUB_Init( PSTATUS_WORK *work )
{
  PSTATUS_SUB_WORK* subWork;
  
  subWork = GFL_HEAP_AllocMemory( work->heapId , sizeof(PSTATUS_SUB_WORK) );
  subWork->pokeMcss = NULL;
  subWork->pokeMcssBack = NULL;
  subWork->IsHoldTp = FALSE;
  subWork->isDispFront = TRUE;
  subWork->state = PSS_DISP_FORNT;
  return subWork;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
void PSTATUS_SUB_Term( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork )
{
  //PSTATUS_SUB_PokeDeleteMcss( work,subWork );
  GFL_HEAP_FreeMemory( subWork );
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
void PSTATUS_SUB_Main( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork )
{
  if( subWork->state == PSS_DISP_FORNT ||
      subWork->state == PSS_DISP_BACK )
  {
    PSTATUS_SUB_UpdateTP( work , subWork );
  }
  else
  if( subWork->state == PSS_BACK_TO_FRONT )
  {
    //ポケモンジャンプ処理
    VecFx32 cam_pos = {PSTATUS_SUB_CAMERA_FRONT_X,FX32_CONST(0.0f),FX32_CONST(101.0f)};
    VecFx32 ofs={0,0,0};
    
    subWork->flipAnimeCnt++;
    
    if( subWork->flipAnimeCnt > PSTATUS_SUB_JUMP_TIME/2 &&
        subWork->isDispFront == FALSE )
    {
      MCSS_ResetVanishFlag( subWork->pokeMcss );
      MCSS_SetVanishFlag( subWork->pokeMcssBack );
      subWork->isDispFront = TRUE;
    }

    if( subWork->flipAnimeCnt >= PSTATUS_SUB_JUMP_TIME )
    {
      subWork->state = PSS_DISP_FORNT;
    }
    else
    {
      u16 rad;
      fx32 sin;
      const fx32 camSubX = (PSTATUS_SUB_CAMERA_FRONT_X-PSTATUS_SUB_CAMERA_BACK_X);
      const fx32 camOfsX = camSubX*subWork->flipAnimeCnt/PSTATUS_SUB_JUMP_TIME;
      cam_pos.x = PSTATUS_SUB_CAMERA_BACK_X+camOfsX;
      
      rad = subWork->flipAnimeCnt*0x8000/PSTATUS_SUB_JUMP_TIME;
      sin = FX_SinIdx( rad );
      ofs.y = (sin*PSTATUS_SUB_JUMP_HEIGHT);
      
    }
    MCSS_SetOfsPosition( subWork->pokeMcss , &ofs );
    MCSS_SetOfsPosition( subWork->pokeMcssBack , &ofs );
    GFL_G3D_CAMERA_SetPos( work->camera , &cam_pos );
    GFL_G3D_CAMERA_Switching( work->camera );
  }
  else
  if( subWork->state == PSS_FRONT_TO_BACK )
  {
    VecFx32 cam_pos = {PSTATUS_SUB_CAMERA_BACK_X,FX32_CONST(0.0f),FX32_CONST(101.0f)};
    VecFx32 shadowScale = {PSTATUS_SUB_SHADOW_SCALE_BACK_X , PSTATUS_SUB_SHADOW_SCALE_BACK_Y , PSTATUS_SUB_SHADOW_SCALE_BACK_Z};
    
    subWork->flipAnimeCnt++;

    if( subWork->flipAnimeCnt < PSTATUS_SUB_FLIP_TIME )
    {
      /*
      const fx32 scaleX = (PSTATUS_SUB_SHADOW_SCALE_BACK_X - PSTATUS_SUB_SHADOW_SCALE_X)*subWork->flipAnimeCnt/PSTATUS_SUB_FLIP_TIME;
      const fx32 scaleY = (PSTATUS_SUB_SHADOW_SCALE_BACK_Y - PSTATUS_SUB_SHADOW_SCALE_Y)*subWork->flipAnimeCnt/PSTATUS_SUB_FLIP_TIME;
      const fx32 camX = (PSTATUS_SUB_CAMERA_BACK_X - PSTATUS_SUB_CAMERA_FRONT_X)*subWork->flipAnimeCnt/PSTATUS_SUB_FLIP_TIME;
      
      shadowScale.x = PSTATUS_SUB_SHADOW_SCALE_X + scaleX;
      shadowScale.y = PSTATUS_SUB_SHADOW_SCALE_Y + scaleY;
      cam_pos.x = PSTATUS_SUB_CAMERA_FRONT_X + camX;
      */
      //数値指定
      const fx32 scareXArr[2] = { FX32_CONST(1.2f),FX32_CONST(1.4f) };
      const fx32 scareYArr[2] = { FX32_CONST(2.2f),FX32_CONST(2.3f) };
      const fx32 camXArr[2] = { FX32_CONST(-57.0f),FX32_CONST(-60.0f) };
      const u8 arrIdx = subWork->flipAnimeCnt-1;
      
      shadowScale.x = scareXArr[arrIdx];
      shadowScale.y = scareYArr[arrIdx];
      cam_pos.x = camXArr[arrIdx];
      
      GFL_G3D_CAMERA_SetPos( work->camera , &cam_pos );
      GFL_G3D_CAMERA_Switching( work->camera );
      MCSS_SetShadowScale( subWork->pokeMcss , &shadowScale );
    }
    else
    {
      //ひっくり返る！
      
      MCSS_SetVanishFlag( subWork->pokeMcss );
      MCSS_ResetVanishFlag( subWork->pokeMcssBack );
      MCSS_SetShadowScale( subWork->pokeMcssBack , &shadowScale );
      GFL_G3D_CAMERA_SetPos( work->camera , &cam_pos );
      GFL_G3D_CAMERA_Switching( work->camera );
      subWork->isDispFront = FALSE;
      subWork->state = PSS_DISP_BACK;
      subWork->IsHoldTp = FALSE;
    }
  }
}

//--------------------------------------------------------------
//	描画
//--------------------------------------------------------------
void PSTATUS_SUB_Draw( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork )
{
  //未使用
}

#pragma mark [>Resource
//--------------------------------------------------------------
//	リソース読み込み
//--------------------------------------------------------------
void PSTATUS_SUB_LoadResource( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork , ARCHANDLE *archandle )
{
  GFL_ARCHDL_UTIL_TransVramScreen( archandle , NARC_p_status_gra_p_st_bg_d_NSCR , 
                    PSTATUS_BG_PLATE ,  0 , 0, FALSE , work->heapId );
  
}

//--------------------------------------------------------------
//	リソース開放
//--------------------------------------------------------------
void PSTATUS_SUB_ReleaseResource( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork )
{
  //ここで描画物の開放もしておく
  PSTATUS_SUB_ClearPage( work , subWork );

}

#pragma mark [>Cell
//--------------------------------------------------------------
//	セル初期化
//--------------------------------------------------------------
void PSTATUS_SUB_InitCell( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork )
{
  //選択カーソル
  {
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_x = PSTATUS_SUB_CELL_BALL_X;
    cellInitData.pos_y = PSTATUS_SUB_CELL_BALL_Y;
    cellInitData.softpri = 10;
    cellInitData.bgpri = 1;
    cellInitData.anmseq = 0;
    
    subWork->clwkBall = GFL_CLACT_WK_Create( work->cellUnit ,
              work->cellRes[SCR_NCG_BALL],
              work->cellRes[SCR_PLT_BALL],
              work->cellRes[SCR_ANM_BALL],
              &cellInitData ,CLSYS_DEFREND_MAIN , work->heapId );

    GFL_CLACT_WK_SetDrawEnable( subWork->clwkBall , FALSE );
  }
  {
    static const u8 markPosX[SSMT_MAX] = 
    { 208 , 216 , 224 , 232 , 240 , 248 , 184 , 176 };
    static const u8 markPosY = 128;
    static const u8 markAnmSeq[SSMT_MAX] = 
    { 0 , 2 , 4 , 6 , 8 , 10 , 12 , 13 };
    u8 i;
    //マーク
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_y = markPosY;
    cellInitData.softpri = 10;
    cellInitData.bgpri = 1;
    
    for( i=0;i<SSMT_MAX;i++ )
    {
      cellInitData.pos_x = markPosX[i];
      cellInitData.anmseq = markAnmSeq[i];
      subWork->clwkMark[i] = GFL_CLACT_WK_Create( work->cellUnit ,
                work->cellRes[SCR_NCG_MARK],
                work->cellRes[SCR_PLT_MARK],
                work->cellRes[SCR_ANM_MARK],
                &cellInitData ,CLSYS_DEFREND_MAIN , work->heapId );
      GFL_CLACT_WK_SetDrawEnable( subWork->clwkMark[i] , TRUE );
    }
    
  }
}

//--------------------------------------------------------------
//	セル開放
//--------------------------------------------------------------
void PSTATUS_SUB_TermCell( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork )
{
  u8 i;
  for( i=0;i<SSMT_MAX;i++ )
  {
    GFL_CLACT_WK_Remove( subWork->clwkMark[i] );
  }
  GFL_CLACT_WK_Remove( subWork->clwkBall );
}

#pragma mark [>Disp
//--------------------------------------------------------------
//	ページの表示
//--------------------------------------------------------------
void PSTATUS_SUB_DispPage( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork )
{
  //MCSS
  const POKEMON_PASO_PARAM *ppp = PSTATUS_UTIL_GetCurrentPPP( work );
//  PSTATUS_SUB_PokeCreateMcss( work , subWork , ppp );

  //BmpWin
  subWork->bmpWinUpper = GFL_BMPWIN_Create( PSTATUS_BG_PARAM ,
              PSTATUS_SUB_UPWIN_LEFT , PSTATUS_SUB_UPWIN_TOP ,
              PSTATUS_SUB_UPWIN_WIDTH , PSTATUS_SUB_UPWIN_HEIGHT ,
              PSTATUS_BG_PLT_FONT , GFL_BMP_CHRAREA_GET_B );

  subWork->bmpWinDown  = GFL_BMPWIN_Create( PSTATUS_BG_PARAM ,
              PSTATUS_SUB_DOWNWIN_LEFT , PSTATUS_SUB_DOWNWIN_TOP ,
              PSTATUS_SUB_DOWNWIN_WIDTH , PSTATUS_SUB_DOWNWIN_HEIGHT ,
              PSTATUS_BG_PLT_FONT , GFL_BMP_CHRAREA_GET_B );

  PSTATUS_SUB_DrawStr( work , subWork , ppp );
}
//--------------------------------------------------------------
//	ページの表示
//--------------------------------------------------------------
void PSTATUS_SUB_DispPage_Trans( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork )
{
  //MCSS
  const POKEMON_PASO_PARAM *ppp = PSTATUS_UTIL_GetCurrentPPP( work );
  PSTATUS_SUB_PokeCreateMcss( work , subWork , ppp );
  GFL_BMPWIN_MakeTransWindow_VBlank( subWork->bmpWinUpper );
  GFL_BMPWIN_MakeTransWindow_VBlank( subWork->bmpWinDown );
  
  //ボールの更新
  {
    u32 ballIdx = PPP_Get( ppp , ID_PARA_get_ball , NULL );
    if( ballIdx != 0 )
    {
      ballIdx--;
    }
    {
      NNSG2dPaletteData *pltData;
      void *pltRes = GFL_ARC_UTIL_LoadPalette( ARCID_P_STATUS , 
                          NARC_p_status_gra_ball00_NCLR + ballIdx ,
                          &pltData , work->heapId );
      GFL_CLGRP_PLTT_Replace( work->cellRes[SCR_PLT_BALL] , pltData , 1 );
      GFL_HEAP_FreeMemory( pltRes );
    }
    {
      NNSG2dCharacterData *ncgData;
      void *ncgRes = GFL_ARC_UTIL_LoadBGCharacter( ARCID_P_STATUS , 
                          NARC_p_status_gra_ball00_NCGR + ballIdx ,
                          FALSE , &ncgData , work->heapId );
      GFL_CLGRP_CGR_Replace( work->cellRes[SCR_NCG_BALL] , ncgData );
      GFL_HEAP_FreeMemory( ncgRes );
    }
    
    GFL_CLACT_WK_SetDrawEnable( subWork->clwkBall , TRUE );
  }
  //マークの更新
  {
    GFL_CLACT_WK_SetDrawEnable( subWork->clwkMark[SSMT_MARK_RARE] , FALSE );
    GFL_CLACT_WK_SetDrawEnable( subWork->clwkMark[SSMT_MARK_POKERUS] , FALSE );
    
  }
  /*
  //切り替え時前向き固定
  subWork->isDispFront = TRUE;
  subWork->isJumpPoke = FALSE;
  {
    VecFx32 ofs={0,0,0};
    static const VecFx32 cam_pos = {PSTATUS_SUB_CAMERA_FRONT_X,FX32_CONST(0.0f),FX32_CONST(101.0f)};
    MCSS_SetOfsPosition( subWork->pokeMcss , &ofs );
    MCSS_SetOfsPosition( subWork->pokeMcssBack , &ofs );
    GFL_G3D_CAMERA_SetPos( work->camera , &cam_pos );
    GFL_G3D_CAMERA_Switching( work->camera );
  }
  */

}

//--------------------------------------------------------------
//	ページのクリア
//--------------------------------------------------------------
void PSTATUS_SUB_ClearPage( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork )
{

  //文字
  GFL_BMPWIN_Delete( subWork->bmpWinDown );
  GFL_BMPWIN_Delete( subWork->bmpWinUpper );

  //MCSS
//  PSTATUS_SUB_PokeDeleteMcss( work,subWork );
}
//--------------------------------------------------------------
//	ページのクリア
//--------------------------------------------------------------
void PSTATUS_SUB_ClearPage_Trans( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork )
{
  //MCSS
  PSTATUS_SUB_PokeDeleteMcss( work,subWork );
  GFL_CLACT_WK_SetDrawEnable( subWork->clwkBall , FALSE );
}

//--------------------------------------------------------------
//	文字の描画
//--------------------------------------------------------------
void PSTATUS_SUB_DrawStr( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork , const POKEMON_PASO_PARAM *ppp )
{
  //名前
  {
    WORDSET *wordSet = WORDSET_Create( work->heapId );
    STRBUF *srcStr;
    STRBUF *dstStr = GFL_STR_CreateBuffer( 32, work->heapId );
    STRBUF *nameStr = GFL_STR_CreateBuffer( 32, work->heapId );
    PPP_Get( ppp, ID_PARA_nickname, nameStr );

    WORDSET_RegisterWord( wordSet , 0 , nameStr , 0,TRUE,PM_LANG );
    srcStr = GFL_MSG_CreateString( work->msgHandle , mes_status_01_01 ); 
    WORDSET_ExpandStr( wordSet , dstStr , srcStr );

    PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( subWork->bmpWinUpper ) , 
            PSTATUS_SUB_STR_NAME_X , PSTATUS_SUB_STR_NAME_Y , 
            dstStr , work->fontHandle , PSTATUS_STR_COL_BLACK);

    GFL_STR_DeleteBuffer( nameStr );
    GFL_STR_DeleteBuffer( srcStr );
    GFL_STR_DeleteBuffer( dstStr );
    WORDSET_Delete( wordSet );
  }
  
  //性別
  {
    STRBUF *srcStr;
    u32 sex = PPP_Get( ppp , ID_PARA_sex , NULL );
    if( sex == PTL_SEX_MALE )
    {
      srcStr = GFL_MSG_CreateString( work->msgHandle , mes_status_01_02 ); 
      PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( subWork->bmpWinUpper ) , 
              PSTATUS_SUB_STR_SEX_X , PSTATUS_SUB_STR_SEX_Y ,
              srcStr , work->fontHandle , PSTATUS_STR_COL_BLUE );
      GFL_STR_DeleteBuffer( srcStr );
    }
    else if( sex == PTL_SEX_FEMALE )
    {
      srcStr = GFL_MSG_CreateString( work->msgHandle , mes_status_01_03 ); 
      PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( subWork->bmpWinUpper ) , 
              PSTATUS_SUB_STR_SEX_X , PSTATUS_SUB_STR_SEX_Y ,
              srcStr , work->fontHandle , PSTATUS_STR_COL_RED );
      GFL_STR_DeleteBuffer( srcStr );
    }
  }

  //レベル
  {
    WORDSET *wordSet = WORDSET_Create( work->heapId );
    STRBUF *srcStr;
    STRBUF *dstStr = GFL_STR_CreateBuffer( 16, work->heapId );

    u32 lv = PPP_CalcLevel( ppp );
    WORDSET_RegisterNumber( wordSet , 0 , lv , 3 , STR_NUM_DISP_LEFT , STR_NUM_CODE_DEFAULT );
    srcStr = GFL_MSG_CreateString( work->msgHandle , mes_status_01_04 ); 
    WORDSET_ExpandStr( wordSet , dstStr , srcStr );
    PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( subWork->bmpWinUpper ) , 
            PSTATUS_SUB_STR_LV_X , PSTATUS_SUB_STR_LV_Y , 
            dstStr , work->fontHandle , PSTATUS_STR_COL_BLACK );

    GFL_STR_DeleteBuffer( srcStr );
    GFL_STR_DeleteBuffer( dstStr );
    WORDSET_Delete( wordSet );
  }

  //"もちもの"
  {
    STRBUF *srcStr;
    srcStr = GFL_MSG_CreateString( work->msgHandle , mes_status_01_05 ); 
    PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( subWork->bmpWinDown ) , 
            PSTATUS_SUB_STR_ITEM_X , PSTATUS_SUB_STR_ITEM_Y ,
            srcStr , work->fontHandle , PSTATUS_STR_COL_BLACK );
    GFL_STR_DeleteBuffer( srcStr );
  }

  //所持アイテム名
  {
    WORDSET *wordSet = WORDSET_Create( work->heapId );
    STRBUF *srcStr;
    STRBUF *dstStr = GFL_STR_CreateBuffer( 16, work->heapId );

    u32 itemId = PPP_Get( ppp , ID_PARA_item , NULL );
    WORDSET_RegisterItemName( wordSet , 0 , itemId );
    srcStr = GFL_MSG_CreateString( work->msgHandle , mes_status_01_06 ); 
    WORDSET_ExpandStr( wordSet , dstStr , srcStr );
    PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( subWork->bmpWinDown ) , 
            PSTATUS_SUB_STR_ITEMNAME_X , PSTATUS_SUB_STR_ITEMNAME_Y , 
            dstStr , work->fontHandle , PSTATUS_STR_COL_BLACK );

    GFL_STR_DeleteBuffer( srcStr );
    GFL_STR_DeleteBuffer( dstStr );
    WORDSET_Delete( wordSet );
  }
  
}

#pragma mark [>UI
static void PSTATUS_SUB_UpdateTP( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork )
{
  u32 tpx,tpy;
  //当たり判定作成
  GFL_UI_TP_HITTBL hitTbl[2] =
  {
    { PSTATUS_SUB_TOUCH_TOP , PSTATUS_SUB_TOUCH_BOTTOM , PSTATUS_SUB_TOUCH_LEFT , PSTATUS_SUB_TOUCH_RIGHT },
    { GFL_UI_TP_HIT_END ,0,0,0 },
  };
  
  GFL_UI_TP_GetPointCont( &tpx,&tpy );
  if( subWork->state == PSS_DISP_FORNT )
  {
    /*
    //くすぐり判定
    const int retCont = GFL_UI_TP_HitCont( hitTbl );
    if( retCont == 0 )
    {
      if( subWork->IsHoldTp == FALSE )
      {
        subWork->IsHoldTp = TRUE;
        subWork->tickleDir = SSTD_NONE;
        subWork->befTpx = tpx;
        subWork->befTpy = tpy;
        subWork->holdCnt = 0;
        subWork->tickleNum = 0;
      }
      else
      {
        const int subX = subWork->befTpx - tpx;
        if( ( subWork->tickleDir == SSTD_NONE || subWork->tickleDir == SSTD_LEFT ) &&
            subX > PSTATUS_SUB_TICKLE_WIDHT )
        {
          subWork->tickleNum++;
          subWork->tickleDir = SSTD_RIGHT;
          subWork->holdCnt = 0;
          subWork->befTpx = tpx;
          subWork->befTpy = tpy;
        }
        else
        if( ( subWork->tickleDir == SSTD_NONE || subWork->tickleDir == SSTD_RIGHT ) &&
            subX < -PSTATUS_SUB_TICKLE_WIDHT )
        {
          subWork->tickleNum++;
          subWork->tickleDir = SSTD_LEFT;
          subWork->holdCnt = 0;
          subWork->befTpx = tpx;
          subWork->befTpy = tpy;
        }
        else
        {
          subWork->holdCnt++;
          if( subWork->holdCnt > PSTATUS_SUB_TICKLE_TIME )
          {
            //時間かかりすぎでリセット
            subWork->tickleDir = SSTD_NONE;
            subWork->holdCnt = 0;
            subWork->tickleNum = 0;
            subWork->befTpx = tpx;
            subWork->befTpy = tpy;
          }
        }
        if( subWork->tickleNum >= PSTATUS_SUB_TICKLE_NUM )
        {
          //ひっくり返る！
          const VecFx32 cam_pos = {PSTATUS_SUB_CAMERA_BACK_X,FX32_CONST(0.0f),FX32_CONST(101.0f)};
          MCSS_SetVanishFlag( subWork->pokeMcss );
          MCSS_ResetVanishFlag( subWork->pokeMcssBack );
          GFL_G3D_CAMERA_SetPos( work->camera , &cam_pos );
          GFL_G3D_CAMERA_Switching( work->camera );
          subWork->isDispFront = FALSE;
          subWork->IsHoldTp = FALSE;
        }
      }
    }
    else
    {
      subWork->IsHoldTp = FALSE;
    }
    */

    //タッチで向き反転処理
    const int retCont = GFL_UI_TP_HitTrg( hitTbl );
    if( retCont == 0 )
    {
      subWork->state = PSS_FRONT_TO_BACK;
      subWork->flipAnimeCnt = 0;
    }

  }
  else
  if( subWork->state == PSS_DISP_BACK )
  {
    //タッチでジャンプ
    const int retCont = GFL_UI_TP_HitTrg( hitTbl );
    if( retCont == 0 )
    {
      subWork->state = PSS_BACK_TO_FRONT;
      subWork->flipAnimeCnt = 0;
    }
  }
  
}

#pragma mark [>Poke
//--------------------------------------------------------------
//	ポケモンMCSS作成
//--------------------------------------------------------------
static void PSTATUS_SUB_PokeCreateMcss( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork , const POKEMON_PASO_PARAM *ppp )
{
  MCSS_ADD_WORK addWork;
  VecFx32 scale = {FX32_ONE*16,FX32_ONE*16,FX32_ONE};
  VecFx32 shadowScale = {PSTATUS_SUB_SHADOW_SCALE_X , PSTATUS_SUB_SHADOW_SCALE_Y , PSTATUS_SUB_SHADOW_SCALE_Z};
  VecFx32 shadowScaleBack = {PSTATUS_SUB_SHADOW_SCALE_BACK_X , PSTATUS_SUB_SHADOW_SCALE_BACK_Y , PSTATUS_SUB_SHADOW_SCALE_BACK_Z};
  VecFx32 shadowOffset= {PSTATUS_SUB_SHADOW_OFFSET_X , PSTATUS_SUB_SHADOW_OFFSET_Y , PSTATUS_SUB_SHADOW_OFFSET_Z};
  
  work->shadowRotate = 302*65536/360;
  
  GF_ASSERT( subWork->pokeMcss == NULL );
  {
    MCSS_TOOL_MakeMAWPPP( ppp , &addWork , MCSS_DIR_FRONT );
    subWork->pokeMcss = MCSS_Add( work->mcssSys , PSTATUS_MCSS_POS_X , PSTATUS_MCSS_POS_Y ,0 , &addWork );
    MCSS_SetScale( subWork->pokeMcss , &scale );
    MCSS_SetShadowAlpha( subWork->pokeMcss , 2 );
    MCSS_SetShadowRotate( subWork->pokeMcss , PSTATUS_SUB_SHADOW_ROTATE );
    MCSS_SetShadowOffset( subWork->pokeMcss , &shadowOffset );
    MCSS_SetShadowScale( subWork->pokeMcss , &shadowScale );
  }
  {
    MCSS_TOOL_MakeMAWPPP( ppp , &addWork , MCSS_DIR_BACK );
    subWork->pokeMcssBack = MCSS_Add( work->mcssSys , PSTATUS_MCSS_POS_X , PSTATUS_MCSS_POS_Y ,0 , &addWork );
    MCSS_SetScale( subWork->pokeMcssBack , &scale );
    MCSS_SetShadowAlpha( subWork->pokeMcssBack , 2 );
    MCSS_SetShadowRotate( subWork->pokeMcssBack , PSTATUS_SUB_SHADOW_ROTATE );
    MCSS_SetShadowOffset( subWork->pokeMcssBack , &shadowOffset );
    MCSS_SetShadowScale( subWork->pokeMcssBack , &shadowScaleBack );
  }
  if( subWork->isDispFront == TRUE )
  {
    MCSS_ResetVanishFlag( subWork->pokeMcss );
    MCSS_SetVanishFlag( subWork->pokeMcssBack );
  }
  else
  {
    MCSS_ResetVanishFlag( subWork->pokeMcssBack );
    MCSS_SetVanishFlag( subWork->pokeMcss );
  }
}

//--------------------------------------------------------------
//	ポケモンMCSS削除
//--------------------------------------------------------------
static void PSTATUS_SUB_PokeDeleteMcss( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork )
{
  GF_ASSERT( subWork->pokeMcss != NULL );
  
  MCSS_SetVanishFlag( subWork->pokeMcss );
  MCSS_SetVanishFlag( subWork->pokeMcssBack );
  MCSS_Del(work->mcssSys,subWork->pokeMcss);
  MCSS_Del(work->mcssSys,subWork->pokeMcssBack);
  subWork->pokeMcss = NULL;
  subWork->pokeMcssBack = NULL;
}

//デバグ用
void PSTATUS_SUB_SetShadowScale( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork )
{
  MCSS_SetShadowScale( subWork->pokeMcss , &work->shadowScale );
  MCSS_SetShadowRotate( subWork->pokeMcss , work->shadowRotate );
  MCSS_SetShadowOffset( subWork->pokeMcss , &work->shadowOfs );
  MCSS_SetShadowScale( subWork->pokeMcssBack , &work->shadowScale );
  MCSS_SetShadowRotate( subWork->pokeMcssBack , work->shadowRotate );
  MCSS_SetShadowOffset( subWork->pokeMcssBack , &work->shadowOfs );
}
