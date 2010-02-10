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
#include "app/app_menu_common.h"

#include "arc_def.h"
#include "zukan_data.naix"
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

//タイプアイコン
#define PSTATUS_INFO_TYPEICON_1_X  (96)
#define PSTATUS_INFO_TYPEICON_1_Y  (48)
#define PSTATUS_INFO_TYPEICON_2_X  (96+32)
#define PSTATUS_INFO_TYPEICON_2_Y  (48)

//経験値バー
#define PSTATUS_INFO_EXPBAR_TOP  (3)
#define PSTATUS_INFO_EXPBAR_LEFT (0)
#define PSTATUS_INFO_EXPBAR_HEIGHT  (3)
#define PSTATUS_INFO_EXPBAR_WIDTH (64)

#define PSTATUS_INFO_EXPBAR_COL (5) //とりあえずフォントの青

//上画面
#define PSTATUS_INFO_MEMO_WIN_TOP  (5)
#define PSTATUS_INFO_MEMO_WIN_LEFT (4)
#define PSTATUS_INFO_MEMO_WIN_HEIGHT  (16)
#define PSTATUS_INFO_MEMO_WIN_WIDTH (24)

#define PSTATUS_INFO_MEMO_STR_X ( 0+ PSTATUS_STR_OFS_X)
#define PSTATUS_INFO_MEMO_STR_Y ( 0+ PSTATUS_STR_OFS_Y)


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

  u16 *localZukanNoList;

  GFL_BMPWIN  *bmpWin[SIB_MAX];
  GFL_BMPWIN  *bmpWinUp;

  NNSG2dScreenData *scrDataDown;
  void *scrResDown;
  NNSG2dScreenData *scrDataUp;
  void *scrResUp;
  NNSG2dScreenData *scrDataUpTitle;
  void *scrResUpTitle;
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void PSTATUS_INFO_DrawState( PSTATUS_WORK *work , PSTATUS_INFO_WORK *subWork , const POKEMON_PASO_PARAM *ppp );
static void PSTATUS_INFO_DrawStateUp( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork , const POKEMON_PASO_PARAM *ppp );

static const u8 winPos[SIB_MAX][4] =
{
  {  2,  1, 16,  2},  //図鑑
  {  2,  3, 16,  2},  //名前
  {  2,  5,  8,  2},  //タイプ
  {  2,  7, 16,  2},  //親
  {  2,  9, 16,  2},  //ID
  {  2, 11, 17,  2},  //現在の経験地
  { 10, 13,  8,  2},  //↑数値
  {  2, 15, 16,  2},  //次までの経験地
  {  6, 17, 12,  2},  //↑数値
  { 10, 19,  8,  1},  //経験地バー
};

//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
PSTATUS_INFO_WORK* PSTATUS_INFO_Init( PSTATUS_WORK *work )
{
  u32 size;
  PSTATUS_INFO_WORK* infoWork;
  
  infoWork = GFL_HEAP_AllocMemory( work->heapId , sizeof(PSTATUS_INFO_WORK) );
  infoWork->isUpdateStr = FALSE;
  infoWork->isDisp = FALSE;
  infoWork->localZukanNoList = GFL_ARC_UTIL_LoadEx( ARCID_ZUKAN_DATA, NARC_zukan_data_zkn_chihou_no_dat, FALSE, work->heapId, &size );

//  {
//    int i;
//    for( i=0;i<size/2;i++ )
//    {
//      OS_TPrintf("[%3d->%3d]\n",i,infoWork->localZukanNoList[i]);
//    }
//  }

  return infoWork;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
void PSTATUS_INFO_Term( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork )
{
  GFL_HEAP_FreeMemory( infoWork->localZukanNoList );
  GFL_HEAP_FreeMemory( infoWork );
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
void PSTATUS_INFO_Main( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork )
{

}

#pragma mark [>Resource
//--------------------------------------------------------------
//	リソース読み込み
//--------------------------------------------------------------
void PSTATUS_INFO_LoadResource( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork , ARCHANDLE *archandle )
{
  //書き換え用スクリーン読み込み
  infoWork->scrResDown = GFL_ARCHDL_UTIL_LoadScreen( archandle , NARC_p_status_gra_p_st_info_d_NSCR ,
                    FALSE , &infoWork->scrDataDown , work->heapId );
  infoWork->scrResUp = GFL_ARCHDL_UTIL_LoadScreen( archandle , NARC_p_status_gra_p_st_info_u_NSCR ,
                    FALSE , &infoWork->scrDataUp , work->heapId );
  infoWork->scrResUpTitle = GFL_ARCHDL_UTIL_LoadScreen( archandle , NARC_p_status_gra_p_st_infotitle_u_NSCR ,
                    FALSE , &infoWork->scrDataUpTitle , work->heapId );
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
  GFL_HEAP_FreeMemory( infoWork->scrResUpTitle );
}

#pragma mark [>Disp
//--------------------------------------------------------------
//	ページの表示
//--------------------------------------------------------------
void PSTATUS_INFO_DispPage( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork )
{
  u8 i;
  const POKEMON_PASO_PARAM *ppp = PSTATUS_UTIL_GetCurrentPPP( work );

  for( i=0;i<SIB_MAX;i++ )
  {
    infoWork->bmpWin[i] = GFL_BMPWIN_Create( PSTATUS_BG_PARAM ,
                winPos[i][0] , winPos[i][1] , winPos[i][2] , winPos[i][3] ,
                PSTATUS_BG_PLT_FONT , GFL_BMP_CHRAREA_GET_B );
  }
  infoWork->bmpWinUp = GFL_BMPWIN_Create( PSTATUS_BG_SUB_STR ,
              PSTATUS_INFO_MEMO_WIN_LEFT , PSTATUS_INFO_MEMO_WIN_TOP ,
              PSTATUS_INFO_MEMO_WIN_WIDTH , PSTATUS_INFO_MEMO_WIN_HEIGHT ,
              PSTATUS_BG_SUB_PLT_FONT , GFL_BMP_CHRAREA_GET_B );

  PSTATUS_INFO_DrawState( work , infoWork , ppp );
  PSTATUS_INFO_DrawStateUp( work , infoWork , ppp );

  infoWork->isDisp = TRUE;
}


//--------------------------------------------------------------
//	ページの表示(転送タイミング
//--------------------------------------------------------------
void PSTATUS_INFO_DispPage_Trans( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork )
{
  u8 i;
  const POKEMON_PASO_PARAM *ppp = PSTATUS_UTIL_GetCurrentPPP( work );

  //Window下地の張替え
  GFL_BG_WriteScreenExpand( PSTATUS_BG_PLATE , 
//                    0 , 0 , PSTATUS_MAIN_PAGE_WIDTH , 24 ,
                    0 , 0 , 32 , 24 ,
                    infoWork->scrDataDown->rawData ,
                    0 , 0 , 32 , 32 );
  GFL_BG_LoadScreenV_Req( PSTATUS_BG_PLATE );

  GFL_BG_LoadScreenBuffer( PSTATUS_BG_SUB_PLATE, 
                     infoWork->scrDataUp->rawData, 
                     infoWork->scrDataUp->szByte );

  GFL_BG_LoadScreenV_Req( PSTATUS_BG_SUB_PLATE );

  //上画面タイトル
  GFL_BG_WriteScreenExpand( PSTATUS_BG_SUB_TITLE , 
                    0 , 0 , 32 , PSTATUS_SUB_TITLE_HEIGHT ,
                    infoWork->scrDataUpTitle->rawData ,
                    0 , 0 , 32 , 32 );
  GFL_BG_LoadScreenV_Req( PSTATUS_BG_SUB_TITLE );

  for( i=0;i<SIB_MAX;i++ )
  {
    GFL_BMPWIN_MakeTransWindow_VBlank( infoWork->bmpWin[i] );
  }
  GFL_BMPWIN_MakeTransWindow_VBlank( infoWork->bmpWinUp );

  //タイプアイコン
  {
    const u32 type1 = PPP_Get( ppp, ID_PARA_type1, NULL );
    const u32 type2 = PPP_Get( ppp, ID_PARA_type2, NULL );
    GFL_CLACTPOS cellPos;
    {
      NNSG2dImageProxy imageProxy;
      GFL_CLGRP_CGR_GetProxy( work->cellResTypeNcg[type1] , &imageProxy );
      GFL_CLACT_WK_SetImgProxy( work->clwkTypeIcon[0] , &imageProxy );
      GFL_CLACT_WK_SetPlttOffs( work->clwkTypeIcon[0] , 
                                APP_COMMON_GetPokeTypePltOffset(type1) , 
                                CLWK_PLTTOFFS_MODE_PLTT_TOP );
      cellPos.x = PSTATUS_INFO_TYPEICON_1_X;
      cellPos.y = PSTATUS_INFO_TYPEICON_1_Y;
      GFL_CLACT_WK_SetPos( work->clwkTypeIcon[0] , &cellPos , CLSYS_DEFREND_MAIN );
      GFL_CLACT_WK_SetDrawEnable( work->clwkTypeIcon[0] , TRUE );
      GFL_CLACT_WK_SetBgPri( work->clwkTypeIcon[0] , 1 );
    }
    
    if( type1 != type2 )
    {
      NNSG2dImageProxy imageProxy;
      GFL_CLGRP_CGR_GetProxy( work->cellResTypeNcg[type2] , &imageProxy );
      GFL_CLACT_WK_SetImgProxy( work->clwkTypeIcon[1] , &imageProxy );
      GFL_CLACT_WK_SetPlttOffs( work->clwkTypeIcon[1] , 
                                APP_COMMON_GetPokeTypePltOffset(type2) , 
                                CLWK_PLTTOFFS_MODE_PLTT_TOP );
      cellPos.x = PSTATUS_INFO_TYPEICON_2_X;
      cellPos.y = PSTATUS_INFO_TYPEICON_2_Y;
      GFL_CLACT_WK_SetPos( work->clwkTypeIcon[1] , &cellPos , CLSYS_DEFREND_MAIN );
      GFL_CLACT_WK_SetDrawEnable( work->clwkTypeIcon[1] , TRUE );
      GFL_CLACT_WK_SetBgPri( work->clwkTypeIcon[1] , 1 );
    }
    else
    {
      GFL_CLACT_WK_SetDrawEnable( work->clwkTypeIcon[1] , FALSE );
    }
  }
  
  //バーアイコン処理
  GFL_CLACT_WK_SetAnmSeq( work->clwkBarIcon[SBT_PAGE1] , SBA_PAGE1_SELECT );
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
  GFL_BMPWIN_Delete( infoWork->bmpWinUp );
  
  infoWork->isDisp = FALSE;
}
//--------------------------------------------------------------
//	ページのクリア(転送タイミング
//--------------------------------------------------------------
void PSTATUS_INFO_ClearPage_Trans( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork )
{
  u8 i;
  GFL_BG_FillScreen( PSTATUS_BG_PARAM , 0 , 0 , 0 , 
                     PSTATUS_MAIN_PAGE_WIDTH , 21 ,
                     GFL_BG_SCRWRT_PALNL );
  GFL_BG_LoadScreenReq( PSTATUS_BG_PARAM );

  GFL_BG_ClearScreenCodeVReq( PSTATUS_BG_SUB_STR , 0 );
  GFL_BG_LoadScreenReq( PSTATUS_BG_SUB_STR );

  GFL_CLACT_WK_SetDrawEnable( work->clwkTypeIcon[0] , FALSE );
  GFL_CLACT_WK_SetDrawEnable( work->clwkTypeIcon[1] , FALSE );

  //バーアイコン処理
  GFL_CLACT_WK_SetAnmSeq( work->clwkBarIcon[SBT_PAGE1] , SBA_PAGE1_NORMAL );
}

//--------------------------------------------------------------
//	文字の描画
//--------------------------------------------------------------
static void PSTATUS_INFO_DrawState( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork , const POKEMON_PASO_PARAM *ppp )
{
  //図鑑
  PSTATUS_UTIL_DrawStrFunc( work , infoWork->bmpWin[SIB_ZUKAN] , mes_status_02_02 ,
                            PSTATUS_INFO_STR_ZUKAN_X , PSTATUS_INFO_STR_ZUKAN_Y , PSTATUS_STR_COL_TITLE );
  {
    WORDSET *wordSet = WORDSET_Create( work->heapId );
    u32 no = PPP_Get( ppp , ID_PARA_monsno , NULL );
    if( work->psData->zukan_mode == FALSE )
    {
      no = infoWork->localZukanNoList[no];
    }
    
    if( no == 0 )
    {
      PSTATUS_UTIL_DrawValueStrFunc( work , infoWork->bmpWin[SIB_ZUKAN] , wordSet , mes_status_02_16 , 
                                     PSTATUS_INFO_STR_ZUKAN_VAL_X , PSTATUS_INFO_STR_ZUKAN_VAL_Y , PSTATUS_STR_COL_VALUE );
    }
    else
    {
      WORDSET_RegisterNumber( wordSet , 0 , no , 3 , STR_NUM_DISP_ZERO , STR_NUM_CODE_DEFAULT );
      PSTATUS_UTIL_DrawValueStrFunc( work , infoWork->bmpWin[SIB_ZUKAN] , wordSet , mes_status_02_03 , 
                                     PSTATUS_INFO_STR_ZUKAN_VAL_X , PSTATUS_INFO_STR_ZUKAN_VAL_Y , PSTATUS_STR_COL_VALUE );
      WORDSET_Delete( wordSet );
    }
  }

  //名前
  PSTATUS_UTIL_DrawStrFunc( work , infoWork->bmpWin[SIB_NAME] , mes_status_02_04 ,
                            PSTATUS_INFO_STR_NAME_X , PSTATUS_INFO_STR_NAME_Y , PSTATUS_STR_COL_TITLE );
  {
    STRBUF *srcStr;
    u32 no = PPP_Get( ppp , ID_PARA_monsno , NULL );
    srcStr = GFL_MSG_CreateString( GlobalMsg_PokeName , no ); 
    PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( infoWork->bmpWin[SIB_NAME] ) , 
            PSTATUS_INFO_STR_NAME_VAL_X , PSTATUS_INFO_STR_NAME_VAL_Y , 
            srcStr , work->fontHandle , PSTATUS_STR_COL_VALUE );
    GFL_STR_DeleteBuffer( srcStr );
  }

  //タイプ
  PSTATUS_UTIL_DrawStrFunc( work , infoWork->bmpWin[SIB_TYPE] , mes_status_02_06 ,
                            PSTATUS_INFO_STR_TYPE_X , PSTATUS_INFO_STR_TYPE_Y , PSTATUS_STR_COL_TITLE );

  //親
  PSTATUS_UTIL_DrawStrFunc( work , infoWork->bmpWin[SIB_PARENT] , mes_status_02_07 ,
                            PSTATUS_INFO_STR_PARENT_X , PSTATUS_INFO_STR_PARENT_Y , PSTATUS_STR_COL_TITLE );
  {
    STRBUF *parentName  = GFL_STR_CreateBuffer( 32, work->heapId );
    WORDSET *wordSet = WORDSET_Create( work->heapId );
    const u32 sex = PPP_Get( ppp , ID_PARA_oyasex , parentName );
    PRINTSYS_LSB col = PSTATUS_STR_COL_VALUE;
    if( sex == PTL_SEX_MALE )
    {
      col = PSTATUS_STR_COL_BLUE;
    }
    else if( sex == PTL_SEX_FEMALE )
    {
      col = PSTATUS_STR_COL_RED;
    }

    PPP_Get( ppp , ID_PARA_oyaname , parentName );
    WORDSET_RegisterWord( wordSet , 0 , parentName , 0,TRUE,PM_LANG );
    PSTATUS_UTIL_DrawValueStrFunc( work , infoWork->bmpWin[SIB_PARENT] , wordSet , mes_status_02_08 , 
                                   PSTATUS_INFO_STR_PARENT_VAL_X , PSTATUS_INFO_STR_PARENT_VAL_Y , col );
    WORDSET_Delete( wordSet );
    GFL_STR_DeleteBuffer( parentName );
  }
  
  //ID
  PSTATUS_UTIL_DrawStrFunc( work , infoWork->bmpWin[SIB_ID] , mes_status_02_09 ,
                            PSTATUS_INFO_STR_ID_X , PSTATUS_INFO_STR_ID_Y , PSTATUS_STR_COL_TITLE );
  {
    WORDSET *wordSet = WORDSET_Create( work->heapId );
    u32 id = PPP_Get( ppp , ID_PARA_id_no , NULL )%100000;
    WORDSET_RegisterNumber( wordSet , 0 , id , 5 , STR_NUM_DISP_LEFT , STR_NUM_CODE_DEFAULT );
    PSTATUS_UTIL_DrawValueStrFunc( work , infoWork->bmpWin[SIB_ID] , wordSet , mes_status_02_10 , 
                                   PSTATUS_INFO_STR_ID_VAL_X , PSTATUS_INFO_STR_ID_VAL_Y , PSTATUS_STR_COL_VALUE );
    WORDSET_Delete( wordSet );
  }

  //経験値
  {
    const u32 lv = PPP_Get( ppp, ID_PARA_level, NULL );
    const u32 exp = PPP_Get( ppp , ID_PARA_exp , NULL );
    const u32 nextLvExp = POKETOOL_GetMinExp(PPP_Get( ppp, ID_PARA_monsno, NULL ),
                                    PPP_Get( ppp, ID_PARA_form_no, NULL ),
                                    lv+1) ;
    const u32 nowLvExp = POKETOOL_GetMinExp(PPP_Get( ppp, ID_PARA_monsno, NULL ),
                                    PPP_Get( ppp, ID_PARA_form_no, NULL ),
                                    lv) ;
    u32 minExp = nextLvExp - exp;
    if( lv == 100 )
    {
      minExp = 0;
    }
    //現在の経験値
    PSTATUS_UTIL_DrawStrFunc( work , infoWork->bmpWin[SIB_NOWEXP] , mes_status_02_11 ,
                              PSTATUS_INFO_STR_NOWEXP_X , PSTATUS_INFO_STR_NOWEXP_Y , PSTATUS_STR_COL_TITLE );
    {
      WORDSET *wordSet = WORDSET_Create( work->heapId );
      WORDSET_RegisterNumber( wordSet , 0 , exp , 7 , STR_NUM_DISP_LEFT , STR_NUM_CODE_DEFAULT );
      PSTATUS_UTIL_DrawValueStrFunc( work , infoWork->bmpWin[SIB_NOWEXP_NUM] , wordSet , mes_status_02_12 , 
                                     PSTATUS_INFO_STR_NOWEXP_VAL_X , PSTATUS_INFO_STR_NOWEXP_VAL_Y , PSTATUS_STR_COL_VALUE );
      WORDSET_Delete( wordSet );
    }

    //次までの経験値
    PSTATUS_UTIL_DrawStrFunc( work , infoWork->bmpWin[SIB_NEXTEXP] , mes_status_02_13 ,
                              PSTATUS_INFO_STR_NEXTEXP_X , PSTATUS_INFO_STR_NEXTEXP_Y , PSTATUS_STR_COL_TITLE );
    PSTATUS_UTIL_DrawStrFunc( work , infoWork->bmpWin[SIB_NEXTEXP_NUM] , mes_status_02_14 ,
                              PSTATUS_INFO_STR_NEXTEXP_ATO_X , PSTATUS_INFO_STR_NEXTEXP_ATO_Y , PSTATUS_STR_COL_VALUE );
    {
      WORDSET *wordSet = WORDSET_Create( work->heapId );
      WORDSET_RegisterNumber( wordSet , 0 , minExp , 6 , STR_NUM_DISP_LEFT , STR_NUM_CODE_DEFAULT );
      PSTATUS_UTIL_DrawValueStrFunc( work , infoWork->bmpWin[SIB_NEXTEXP_NUM] , wordSet , mes_status_02_15 , 
                                     PSTATUS_INFO_STR_NEXTEXP_VAL_X , PSTATUS_INFO_STR_NEXTEXP_VAL_Y , PSTATUS_STR_COL_VALUE );
      WORDSET_Delete( wordSet );
    }
    
    //経験値バー
    if( lv != 100 )
    {
      const u32 modLvExp = (nextLvExp-nowLvExp);
      u32 len = PSTATUS_INFO_EXPBAR_WIDTH * (modLvExp-minExp) / modLvExp;
      if( (modLvExp-minExp) != 0 && len == 0 )
      {
        len = 1;
      }
      if( len == PSTATUS_INFO_EXPBAR_WIDTH )
      {
        len--;
      }
      GFL_BMP_Fill( GFL_BMPWIN_GetBmp(infoWork->bmpWin[SIB_EXP_BAR]) , 
                    PSTATUS_INFO_EXPBAR_LEFT , PSTATUS_INFO_EXPBAR_TOP ,
                    len , PSTATUS_INFO_EXPBAR_HEIGHT , PSTATUS_INFO_EXPBAR_COL );

    }
  }


  infoWork->isUpdateStr = TRUE;
}

//--------------------------------------------------------------
//	文字の描画
//--------------------------------------------------------------
static void PSTATUS_INFO_DrawStateUp( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork , const POKEMON_PASO_PARAM *ppp )
{
  u32 height;
  const u32 isEgg = PPP_Get( ppp , ID_PARA_tamago_flag , NULL );
  //メモ
  {
    //FIXME 場所対応
    u16 placeName[7] = {L'な',L'ぞ',L'の',L'ば',L'し',L'ょ',0xFFFF};

    STRBUF *srcStr;
    STRBUF *dstStr = GFL_STR_CreateBuffer( 256 , work->heapId );
    STRBUF *placeStr = GFL_STR_CreateBuffer( 8 , work->heapId );
    WORDSET *wordSet = WORDSET_CreateEx( 10 , WORDSET_DEFAULT_BUFLEN , work->heapId );
    const u32 year  = PPP_Get( ppp , ID_PARA_get_year , NULL );
    const u32 month = PPP_Get( ppp , ID_PARA_get_month , NULL )+1;
    const u32 day   = PPP_Get( ppp , ID_PARA_get_day , NULL )+1;
    const u32 place = PPP_Get( ppp , ID_PARA_get_place , NULL );
    const u32 level = PPP_Get( ppp , ID_PARA_get_level , NULL );

    //場所仮
    GFL_STR_SetStringCode( placeStr , placeName );
    
    WORDSET_RegisterNumber( wordSet , 0 , year , 2 , STR_NUM_DISP_ZERO , STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterNumber( wordSet , 1 , month , 2 , STR_NUM_DISP_LEFT , STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterNumber( wordSet , 2 , day , 2 , STR_NUM_DISP_LEFT , STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterNumber( wordSet , 3 , level , 3 , STR_NUM_DISP_LEFT , STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterWord( wordSet , 4 , placeStr , 0,TRUE,PM_LANG );
    WORDSET_RegisterNumber( wordSet , 5 , year , 2 , STR_NUM_DISP_ZERO , STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterNumber( wordSet , 6 , month , 2 , STR_NUM_DISP_LEFT , STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterNumber( wordSet , 7 , day , 2 , STR_NUM_DISP_LEFT , STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterWord( wordSet , 8 , placeStr , 0,TRUE,PM_LANG );

    if( isEgg == 0 )
    {
      srcStr = GFL_MSG_CreateString( work->msgHandle , mes_status_03_03_01_01 ); 
    }
    else
    {
      //タマゴ
      srcStr = GFL_MSG_CreateString( work->msgHandle , mes_status_03_T_01_02_01 ); 
    }
    WORDSET_ExpandStr( wordSet , dstStr , srcStr );
    PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( infoWork->bmpWinUp ) , 
            PSTATUS_INFO_MEMO_STR_X , PSTATUS_INFO_MEMO_STR_Y , 
            dstStr , work->fontHandle , PSTATUS_STR_COL_VALUE );

    WORDSET_Delete( wordSet );

    height = PRINTSYS_GetStrHeight( dstStr , work->fontHandle );
    //TODO ずれてる・・・？
    height = 16*4;
    
    GFL_STR_DeleteBuffer( placeStr );
    GFL_STR_DeleteBuffer( srcStr );
    GFL_STR_DeleteBuffer( dstStr );
  }

  //メモ２
  if( isEgg == 0 )
  {
    u8 i;
    u8 maxIdx = 0;
    u8 maxRand = 0;
    u32 msgId;
    const int paraId[6] = 
    {
      ID_PARA_hp_rnd,               //HP乱数
      ID_PARA_pow_rnd,              //攻撃力乱数
      ID_PARA_def_rnd,              //防御力乱数
      ID_PARA_agi_rnd,              //素早さ乱数
      ID_PARA_spepow_rnd,             //特攻乱数
      ID_PARA_spedef_rnd,             //特防乱数
    };
    
    for( i=0;i<6;i++ )
    {
      const u32 rand = PPP_Get( ppp , paraId[i] , NULL );
      if( maxRand < rand )
      {
        maxRand = rand;
        maxIdx = i;
      }
    }
    
    msgId = mes_status_03_05_01_00 + maxIdx*5 + maxRand%5;
    PSTATUS_UTIL_DrawStrFunc( work , infoWork->bmpWinUp , msgId ,
                          PSTATUS_INFO_MEMO_STR_X , PSTATUS_INFO_MEMO_STR_Y + height , 
                          PSTATUS_STR_COL_VALUE );

  }
  else
  {
    //タマゴ
    u16 msgId;
    const u32 natuki = PPP_Get( ppp , ID_PARA_friend , NULL );
    if( natuki <= 5 )
    {
      msgId = mes_status_03_T_02_01;
    }
    else
    if( natuki <= 10 )
    {
      msgId = mes_status_03_T_02_02;
    }
    else
    if( natuki <= 40 )
    {
      msgId = mes_status_03_T_02_03;
    }
    else
    {
      msgId = mes_status_03_T_02_04;
    }
    PSTATUS_UTIL_DrawStrFunc( work , infoWork->bmpWinUp , msgId ,
                          PSTATUS_INFO_MEMO_STR_X , PSTATUS_INFO_MEMO_STR_Y + height , 
                          PSTATUS_STR_COL_VALUE );
  }
}
