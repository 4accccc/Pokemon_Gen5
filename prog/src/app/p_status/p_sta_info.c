//======================================================================
/**
 * @file	p_sta_info.c
 * @brief	ポケモンステータス 情報ページ
 * @author	ariizumi
 * @data	09/07/06
 *
 * モジュール名：PSTATUS_INFO
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
#include "p_sta_info.h"

#include "test/ariizumi/ari_debug.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//文字用座標
#define PSTATUS_INFO_STR_ZUKAN_X ( 0+ PSTATUS_STR_OFS_X)
#define PSTATUS_INFO_STR_ZUKAN_Y ( 0+ PSTATUS_STR_OFS_Y)
#define PSTATUS_INFO_STR_ZUKAN_VAL_X (64+ PSTATUS_STR_OFS_X)
#define PSTATUS_INFO_STR_ZUKAN_VAL_Y ( 0+ PSTATUS_STR_OFS_Y)

#define PSTATUS_INFO_STR_NAME_X ( 0+ PSTATUS_STR_OFS_X)
#define PSTATUS_INFO_STR_NAME_Y ( 0+ PSTATUS_STR_OFS_Y)
#define PSTATUS_INFO_STR_NAME_VAL_X (64+ PSTATUS_STR_OFS_X)
#define PSTATUS_INFO_STR_NAME_VAL_Y ( 0+ PSTATUS_STR_OFS_Y)

#define PSTATUS_INFO_STR_TYPE_X ( 0+ PSTATUS_STR_OFS_X)
#define PSTATUS_INFO_STR_TYPE_Y ( 0+ PSTATUS_STR_OFS_Y)

#define PSTATUS_INFO_STR_PARENT_X ( 0+ PSTATUS_STR_OFS_X)
#define PSTATUS_INFO_STR_PARENT_Y ( 0+ PSTATUS_STR_OFS_Y)
#define PSTATUS_INFO_STR_PARENT_VAL_X (64+ PSTATUS_STR_OFS_X)
#define PSTATUS_INFO_STR_PARENT_VAL_Y ( 0+ PSTATUS_STR_OFS_Y)

#define PSTATUS_INFO_STR_ID_X ( 0+ PSTATUS_STR_OFS_X)
#define PSTATUS_INFO_STR_ID_Y ( 0+ PSTATUS_STR_OFS_Y)
#define PSTATUS_INFO_STR_ID_VAL_X (64+ PSTATUS_STR_OFS_X)
#define PSTATUS_INFO_STR_ID_VAL_Y ( 0+ PSTATUS_STR_OFS_Y)

#define PSTATUS_INFO_STR_NOWEXP_X ( 0+ PSTATUS_STR_OFS_X)
#define PSTATUS_INFO_STR_NOWEXP_Y ( 0+ PSTATUS_STR_OFS_Y)
#define PSTATUS_INFO_STR_NOWEXP_VAL_X ( 0+ PSTATUS_STR_OFS_X)
#define PSTATUS_INFO_STR_NOWEXP_VAL_Y ( 0+ PSTATUS_STR_OFS_Y)

#define PSTATUS_INFO_STR_NEXTEXP_X ( 0+ PSTATUS_STR_OFS_X)
#define PSTATUS_INFO_STR_NEXTEXP_Y ( 0+ PSTATUS_STR_OFS_Y)
#define PSTATUS_INFO_STR_NEXTEXP_ATO_X ( 0+ PSTATUS_STR_OFS_X)
#define PSTATUS_INFO_STR_NEXTEXP_ATO_Y ( 0+ PSTATUS_STR_OFS_Y)
#define PSTATUS_INFO_STR_NEXTEXP_VAL_X (32+ PSTATUS_STR_OFS_X)
#define PSTATUS_INFO_STR_NEXTEXP_VAL_Y ( 0+ PSTATUS_STR_OFS_Y)

//フォントカラー(表題と数値用
#define PSTATUS_INFO_STR_COL_TITLE (PRINTSYS_LSB_Make(0xf,2,0))
#define PSTATUS_INFO_STR_COL_VALUE (PRINTSYS_LSB_Make(1,2,0))

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
enum STATUS_INFO_BMPWIN
{
  SIB_ZUKAN,
  SIB_NAME,
  SIB_TYPE,
  SIB_PARENT,
  SIB_ID,
  SIB_NOWEXP,
  SIB_NOWEXP_NUM,
  SIB_NEXTEXP,
  SIB_NEXTEXP_NUM,
  SIB_EXP_BAR,
  
  SIB_MAX,
};


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
struct _PSTATUS_INFO_WORK
{
  BOOL isUpdateStr;
  BOOL isDisp;

  GFL_BMPWIN  *bmpWin[SIB_MAX];

  NNSG2dScreenData *scrDataDown;
  void *scrResDown;
  NNSG2dScreenData *scrDataUp;
  void *scrResUp;
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void PSTATUS_INFO_DrawStr( PSTATUS_WORK *work , PSTATUS_INFO_WORK *subWork , const POKEMON_PASO_PARAM *ppp );
static void PSTATUS_INFO_DrawStrFunc( PSTATUS_WORK *work , PSTATUS_INFO_WORK *subWork , GFL_BMPWIN *bmpWin , 
                                           const u16 strId , const u16 posX , const u16 posY , const u16 col );
static void PSTATUS_INFO_DrawValueStrFunc( PSTATUS_WORK *work , PSTATUS_INFO_WORK *subWork , GFL_BMPWIN *bmpWin , 
                                           WORDSET *wordSet, const u16 strId , const u16 posX , const u16 posY , const u16 col );

static const u8 winPos[SIB_MAX][4] =
{
  {  2,  1, 16,  2},  //図鑑
  {  2,  3, 16,  2},  //名前
  {  2,  5,  8,  2},  //タイプ
  {  2,  7, 16,  2},  //親
  {  2,  9, 16,  2},  //ID
  {  2, 11, 16,  2},  //現在の経験地
  { 10, 13,  8,  2},  //↑数値
  {  2, 15, 16,  2},  //次までの経験地
  {  6, 17, 12,  2},  //↑数値
  {  9, 19,  9,  1},  //経験地バー
};

//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
PSTATUS_INFO_WORK* PSTATUS_INFO_Init( PSTATUS_WORK *work )
{
  PSTATUS_INFO_WORK* infoWork;
  
  infoWork = GFL_HEAP_AllocMemory( work->heapId , sizeof(PSTATUS_INFO_WORK) );
  infoWork->isUpdateStr = FALSE;
  infoWork->isDisp = FALSE;


  return infoWork;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
void PSTATUS_INFO_Term( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork )
{
  GFL_HEAP_FreeMemory( infoWork );
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
void PSTATUS_INFO_Main( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork )
{
  if( infoWork->isUpdateStr == TRUE )
  {
    u8 i;
    BOOL isFinish = TRUE;

    for( i=0;i<SIB_MAX;i++ )
    {
      if( PRINTSYS_QUE_IsExistTarget( work->printQue , GFL_BMPWIN_GetBmp( infoWork->bmpWin[i] )) == TRUE )
      {
        isFinish = FALSE;
        break;
      }
    }
    if( isFinish == TRUE )
    {
      for( i=0;i<SIB_MAX;i++ )
      {
        GFL_BMPWIN_MakeTransWindow_VBlank( infoWork->bmpWin[i] );
      }
      infoWork->isUpdateStr = FALSE;
    }
  }
}

#pragma mark [>Resource
//--------------------------------------------------------------
//	リソース読み込み
//--------------------------------------------------------------
void PSTATUS_INFO_LoadResource( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork , ARCHANDLE *archandle )
{
  //書き換え用スクリーン読み込み
  infoWork->scrResDown = GFL_ARCHDL_UTIL_LoadScreen( archandle , NARC_p_status_gra_p_status_info_NSCR ,
                    FALSE , &infoWork->scrDataDown , work->heapId );
  infoWork->scrResUp = GFL_ARCHDL_UTIL_LoadScreen( archandle , NARC_p_status_gra_p_status_info_u_NSCR ,
                    FALSE , &infoWork->scrDataUp , work->heapId );
}

//--------------------------------------------------------------
//	リソース開放
//--------------------------------------------------------------
void PSTATUS_INFO_ReleaseResource( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork )
{
  //ここで描画物の開放もしておく
  if( infoWork->isDisp == TRUE )
  {
    PSTATUS_INFO_ClearPage( work , infoWork );
  }

  GFL_HEAP_FreeMemory( infoWork->scrResDown );
  GFL_HEAP_FreeMemory( infoWork->scrResUp );
}

#pragma mark [>Disp
//--------------------------------------------------------------
//	ページの表示
//--------------------------------------------------------------
void PSTATUS_INFO_DispPage( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork )
{
  u8 i;
  const POKEMON_PASO_PARAM *ppp = PSTATUS_UTIL_GetCurrentPPP( work );

  //Window下地の張替え
  GFL_BG_WriteScreenExpand( PSTATUS_BG_PLATE , 
                    0 , 0 , PSTATUS_MAIN_PAGE_WIDTH , 24 ,
                    infoWork->scrDataDown->rawData ,
                    0 , 0 , 32 , 32 );
  GFL_BG_LoadScreenV_Req( PSTATUS_BG_PLATE );

  GFL_BG_LoadScreen( PSTATUS_BG_SUB_PLATE, 
                     infoWork->scrDataUp->rawData, 
                     infoWork->scrDataUp->szByte, 
                     0 );
  GFL_BG_LoadScreenBuffer( PSTATUS_BG_SUB_PLATE, 
                     infoWork->scrDataUp->rawData, 
                     infoWork->scrDataUp->szByte );

  GFL_BG_LoadScreenV_Req( PSTATUS_BG_SUB_PLATE );

  for( i=0;i<SIB_MAX;i++ )
  {
    infoWork->bmpWin[i] = GFL_BMPWIN_Create( PSTATUS_BG_PARAM ,
                winPos[i][0] , winPos[i][1] , winPos[i][2] , winPos[i][3] ,
                PSTATUS_BG_PLT_FONT , GFL_BMP_CHRAREA_GET_B );
  }
  
  PSTATUS_INFO_DrawStr( work , infoWork , ppp );

  infoWork->isDisp = TRUE;

}

//--------------------------------------------------------------
//	ページのクリア
//--------------------------------------------------------------
void PSTATUS_INFO_ClearPage( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork )
{
  u8 i;
  for( i=0;i<SIB_MAX;i++ )
  {
    GFL_BMPWIN_Delete( infoWork->bmpWin[i] );
  }
  
  GFL_BG_FillScreen( PSTATUS_BG_PARAM , 0 , 0 , 0 , 
                     PSTATUS_MAIN_PAGE_WIDTH , 21 ,
                     GFL_BG_SCRWRT_PALNL );
  GFL_BG_LoadScreenV_Req( PSTATUS_BG_PARAM );
  
  infoWork->isDisp = FALSE;

}

//--------------------------------------------------------------
//	文字の描画
//--------------------------------------------------------------
void PSTATUS_INFO_DrawStr( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork , const POKEMON_PASO_PARAM *ppp )
{
  //図鑑
  PSTATUS_INFO_DrawStrFunc( work , infoWork , infoWork->bmpWin[SIB_ZUKAN] , mes_status_02_02 ,
                            PSTATUS_INFO_STR_ZUKAN_X , PSTATUS_INFO_STR_ZUKAN_Y , PSTATUS_INFO_STR_COL_TITLE );
  {
    WORDSET *wordSet = WORDSET_Create( work->heapId );
    u32 no = PPP_Get( ppp , ID_PARA_monsno , NULL );
    //FIXME :ローカル図鑑処理
//    if( work->psData->zukan_mode ==  )
//    {
//    }
    WORDSET_RegisterNumber( wordSet , 0 , no , 3 , STR_NUM_DISP_ZERO , STR_NUM_CODE_DEFAULT );
    PSTATUS_INFO_DrawValueStrFunc( work , infoWork , infoWork->bmpWin[SIB_ZUKAN] , wordSet , mes_status_02_03 , 
                                   PSTATUS_INFO_STR_ZUKAN_VAL_X , PSTATUS_INFO_STR_ZUKAN_VAL_Y , PSTATUS_INFO_STR_COL_VALUE );
    WORDSET_Delete( wordSet );
  }

  //名前
  PSTATUS_INFO_DrawStrFunc( work , infoWork , infoWork->bmpWin[SIB_NAME] , mes_status_02_04 ,
                            PSTATUS_INFO_STR_NAME_X , PSTATUS_INFO_STR_NAME_Y , PSTATUS_INFO_STR_COL_TITLE );
  {
    STRBUF *srcStr;
    u32 no = PPP_Get( ppp , ID_PARA_monsno , NULL );
    srcStr = GFL_MSG_CreateString( GlobalMsg_PokeName , no ); 
    PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( infoWork->bmpWin[SIB_NAME] ) , 
            PSTATUS_INFO_STR_NAME_VAL_X , PSTATUS_INFO_STR_NAME_VAL_Y , 
            srcStr , work->fontHandle , PSTATUS_INFO_STR_COL_VALUE );
    GFL_STR_DeleteBuffer( srcStr );
  }

  //タイプ
  PSTATUS_INFO_DrawStrFunc( work , infoWork , infoWork->bmpWin[SIB_TYPE] , mes_status_02_06 ,
                            PSTATUS_INFO_STR_TYPE_X , PSTATUS_INFO_STR_TYPE_Y , PSTATUS_INFO_STR_COL_TITLE );

  //親
  PSTATUS_INFO_DrawStrFunc( work , infoWork , infoWork->bmpWin[SIB_PARENT] , mes_status_02_07 ,
                            PSTATUS_INFO_STR_PARENT_X , PSTATUS_INFO_STR_PARENT_Y , PSTATUS_INFO_STR_COL_TITLE );
  {
    STRBUF *parentName  = GFL_STR_CreateBuffer( 32, work->heapId );
    WORDSET *wordSet = WORDSET_Create( work->heapId );
    PPP_Get( ppp , ID_PARA_oyaname , parentName );
    WORDSET_RegisterWord( wordSet , 0 , parentName , 0,TRUE,PM_LANG );
    PSTATUS_INFO_DrawValueStrFunc( work , infoWork , infoWork->bmpWin[SIB_PARENT] , wordSet , mes_status_02_08 , 
                                   PSTATUS_INFO_STR_PARENT_VAL_X , PSTATUS_INFO_STR_PARENT_VAL_Y , PSTATUS_INFO_STR_COL_VALUE );
    WORDSET_Delete( wordSet );
    GFL_STR_DeleteBuffer( parentName );
  }
  
  //ID
  PSTATUS_INFO_DrawStrFunc( work , infoWork , infoWork->bmpWin[SIB_ID] , mes_status_02_09 ,
                            PSTATUS_INFO_STR_ID_X , PSTATUS_INFO_STR_ID_Y , PSTATUS_INFO_STR_COL_TITLE );
  {
    WORDSET *wordSet = WORDSET_Create( work->heapId );
    u32 id = PPP_Get( ppp , ID_PARA_id_no , NULL );
    WORDSET_RegisterNumber( wordSet , 0 , id , 5 , STR_NUM_DISP_LEFT , STR_NUM_CODE_DEFAULT );
    PSTATUS_INFO_DrawValueStrFunc( work , infoWork , infoWork->bmpWin[SIB_ID] , wordSet , mes_status_02_10 , 
                                   PSTATUS_INFO_STR_ID_VAL_X , PSTATUS_INFO_STR_ID_VAL_Y , PSTATUS_INFO_STR_COL_VALUE );
    WORDSET_Delete( wordSet );
  }

  //経験値
  PSTATUS_INFO_DrawStrFunc( work , infoWork , infoWork->bmpWin[SIB_NOWEXP] , mes_status_02_11 ,
                            PSTATUS_INFO_STR_NOWEXP_X , PSTATUS_INFO_STR_NOWEXP_Y , PSTATUS_INFO_STR_COL_TITLE );
  {
    WORDSET *wordSet = WORDSET_Create( work->heapId );
    u32 exp = PPP_Get( ppp , ID_PARA_exp , NULL );
    WORDSET_RegisterNumber( wordSet , 0 , exp , 7 , STR_NUM_DISP_LEFT , STR_NUM_CODE_DEFAULT );
    PSTATUS_INFO_DrawValueStrFunc( work , infoWork , infoWork->bmpWin[SIB_NOWEXP_NUM] , wordSet , mes_status_02_12 , 
                                   PSTATUS_INFO_STR_NOWEXP_VAL_X , PSTATUS_INFO_STR_NOWEXP_VAL_Y , PSTATUS_INFO_STR_COL_VALUE );
    WORDSET_Delete( wordSet );
  }

  //次までの経験値
  PSTATUS_INFO_DrawStrFunc( work , infoWork , infoWork->bmpWin[SIB_NEXTEXP] , mes_status_02_13 ,
                            PSTATUS_INFO_STR_NEXTEXP_X , PSTATUS_INFO_STR_NEXTEXP_Y , PSTATUS_INFO_STR_COL_TITLE );
  PSTATUS_INFO_DrawStrFunc( work , infoWork , infoWork->bmpWin[SIB_NEXTEXP_NUM] , mes_status_02_14 ,
                            PSTATUS_INFO_STR_NEXTEXP_ATO_X , PSTATUS_INFO_STR_NEXTEXP_ATO_Y , PSTATUS_INFO_STR_COL_VALUE );
  {
    WORDSET *wordSet = WORDSET_Create( work->heapId );
    u32 minExp = PPP_GetMinExp( ppp );
    WORDSET_RegisterNumber( wordSet , 0 , minExp , 6 , STR_NUM_DISP_LEFT , STR_NUM_CODE_DEFAULT );
    PSTATUS_INFO_DrawValueStrFunc( work , infoWork , infoWork->bmpWin[SIB_NEXTEXP_NUM] , wordSet , mes_status_02_15 , 
                                   PSTATUS_INFO_STR_NEXTEXP_VAL_X , PSTATUS_INFO_STR_NEXTEXP_VAL_Y , PSTATUS_INFO_STR_COL_VALUE );
    WORDSET_Delete( wordSet );
  }

  infoWork->isUpdateStr = TRUE;
}

//--------------------------------------------------------------
//	文字の描画
//--------------------------------------------------------------
static void PSTATUS_INFO_DrawStrFunc( PSTATUS_WORK *work , PSTATUS_INFO_WORK *subWork , GFL_BMPWIN *bmpWin , 
                                      const u16 strId , const u16 posX , const u16 posY , const u16 col )
{
  STRBUF *srcStr;
  srcStr = GFL_MSG_CreateString( work->msgHandle , strId ); 
  PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( bmpWin ) , 
          posX , posY , srcStr , work->fontHandle , col );
  GFL_STR_DeleteBuffer( srcStr );
}

//--------------------------------------------------------------
//	文字の描画(値用
//--------------------------------------------------------------
static void PSTATUS_INFO_DrawValueStrFunc( PSTATUS_WORK *work , PSTATUS_INFO_WORK *subWork , GFL_BMPWIN *bmpWin , 
                                      WORDSET *wordSet, const u16 strId , const u16 posX , const u16 posY , const u16 col )
{
  STRBUF *srcStr;
  STRBUF *dstStr = GFL_STR_CreateBuffer( 16, work->heapId );

  srcStr = GFL_MSG_CreateString( work->msgHandle , strId ); 
  WORDSET_ExpandStr( wordSet , dstStr , srcStr );
  PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( bmpWin ) , 
          posX , posY , dstStr , work->fontHandle , col );

  GFL_STR_DeleteBuffer( srcStr );
  GFL_STR_DeleteBuffer( dstStr );
}
