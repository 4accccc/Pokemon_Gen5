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
#include "system/mcss.h"
#include "system/mcss_tool.h"
#include "print/printsys.h"
#include "print/wordset.h"

#include "p_status_gra.naix"

#include "p_sta_sys.h"
#include "p_sta_sub.h"

#include "test/ariizumi/ari_debug.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define PSTATUS_MCSS_POS_X (FX32_CONST(( 200 )/16.0f))
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

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
struct _PSTATUS_SUB_WORK
{
  MCSS_SYS_WORK *mcssSys;
  MCSS_WORK     *pokeMcss;
  
  BOOL        isUpdateStr;
  
  GFL_BMPWIN  *bmpWinUpper;
  GFL_BMPWIN  *bmpWinDown;
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void PSTATUS_SUB_DrawStr( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork , const POKEMON_PASO_PARAM *ppp );

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
  subWork->isUpdateStr = FALSE;

  subWork->mcssSys = MCSS_Init( 1 , work->heapId );
  MCSS_SetTextureTransAdrs( subWork->mcssSys , 0 );
  MCSS_SetOrthoMode( subWork->mcssSys );

  return subWork;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
void PSTATUS_SUB_Term( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork )
{
  //PSTATUS_SUB_PokeDeleteMcss( work,subWork );
  MCSS_Exit( subWork->mcssSys );
  GFL_HEAP_FreeMemory( subWork );
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
void PSTATUS_SUB_Main( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork )
{
  if( subWork->isUpdateStr == TRUE )
  {
    if( PRINTSYS_QUE_IsExistTarget( work->printQue , GFL_BMPWIN_GetBmp( subWork->bmpWinUpper )) == FALSE && 
        PRINTSYS_QUE_IsExistTarget( work->printQue , GFL_BMPWIN_GetBmp( subWork->bmpWinDown )) == FALSE )
    {
      subWork->isUpdateStr = FALSE;
      GFL_BMPWIN_MakeTransWindow_VBlank( subWork->bmpWinUpper );
      GFL_BMPWIN_MakeTransWindow_VBlank( subWork->bmpWinDown );
    }
  }
  
  MCSS_Main( subWork->mcssSys );
}

//--------------------------------------------------------------
//	描画
//--------------------------------------------------------------
void PSTATUS_SUB_Draw( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork )
{
  MCSS_Draw( subWork->mcssSys );
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

#pragma mark [>Disp
//--------------------------------------------------------------
//	ページの表示
//--------------------------------------------------------------
void PSTATUS_SUB_DispPage( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork )
{
  //MCSS
  const POKEMON_PASO_PARAM *ppp = PSTATUS_UTIL_GetCurrentPPP( work );
  PSTATUS_SUB_PokeCreateMcss( work , subWork , ppp );

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
//	ページのクリア
//--------------------------------------------------------------
void PSTATUS_SUB_ClearPage( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork )
{

  //文字
  GFL_BMPWIN_Delete( subWork->bmpWinDown );
  GFL_BMPWIN_Delete( subWork->bmpWinUpper );

  //MCSS
  PSTATUS_SUB_PokeDeleteMcss( work,subWork );
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


  subWork->isUpdateStr = TRUE;
  
}

#pragma mark [>Poke
//--------------------------------------------------------------
//	ポケモンMCSS作成
//--------------------------------------------------------------
static void PSTATUS_SUB_PokeCreateMcss( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork , const POKEMON_PASO_PARAM *ppp )
{
  MCSS_ADD_WORK addWork;
  VecFx32 scale = {FX32_ONE*16,FX32_ONE*16,FX32_ONE};
  
  GF_ASSERT( subWork->pokeMcss == NULL );
  
  MCSS_TOOL_MakeMAWPPP( ppp , &addWork , MCSS_DIR_FRONT );
  subWork->pokeMcss = MCSS_Add( subWork->mcssSys , PSTATUS_MCSS_POS_X , PSTATUS_MCSS_POS_Y ,0 , &addWork );
  MCSS_SetScale( subWork->pokeMcss , &scale );
//  MCSS_ResetVanishFlag( subWork->pokeMcss );//転送後に自動でONになる
//  MCSS_SetMepachiFlag( subWork->pokeMcss );
}

//--------------------------------------------------------------
//	ポケモンMCSS削除
//--------------------------------------------------------------
static void PSTATUS_SUB_PokeDeleteMcss( PSTATUS_WORK *work , PSTATUS_SUB_WORK *subWork )
{
  GF_ASSERT( subWork->pokeMcss != NULL );
  
  MCSS_SetVanishFlag( subWork->pokeMcss );
  MCSS_Del(subWork->mcssSys,subWork->pokeMcss);
  subWork->pokeMcss = NULL;
}

