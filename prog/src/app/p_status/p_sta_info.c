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
#include "message.naix"
#include "p_status_gra.naix"

#include "msg/msg_trainermemo.h"
#include "msg/msg_place_name.h"
#include "msg/msg_place_name_out.h"
#include "msg/msg_place_name_per.h"
#include "msg/msg_place_name_spe.h"

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
#define PSTATUS_INFO_MEMO_WIN_HEIGHT  (20)
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

  GFL_MSGDATA *msgMemo;
  GFL_MSGDATA *msgPlace;
  GFL_MSGDATA *msgPlaceSp;
  GFL_MSGDATA *msgPlaceOut;
  GFL_MSGDATA *msgPlacePerson;

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
static STRBUF* PSTATUS_INFO_GetPlaceStr( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork , const u32 place );

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
  infoWork->localZukanNoList = POKE_PERSONAL_GetZenkokuToChihouArray( work->heapId, NULL );

//  {
//    int i;
//    for( i=0;i<=MONSNO_END;i++ )
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

  //トレーナーメモ
  infoWork->msgMemo = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_trainermemo_dat , work->heapId );
  infoWork->msgPlace   = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_place_name_dat , work->heapId );
  infoWork->msgPlaceSp = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_place_name_spe_dat , work->heapId );
  infoWork->msgPlaceOut= GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_place_name_out_dat , work->heapId );
  infoWork->msgPlacePerson = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_place_name_per_dat , work->heapId );

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

  GFL_MSG_Delete( infoWork->msgMemo );
  GFL_MSG_Delete( infoWork->msgPlace );
  GFL_MSG_Delete( infoWork->msgPlaceSp );
  GFL_MSG_Delete( infoWork->msgPlaceOut );
  GFL_MSG_Delete( infoWork->msgPlacePerson );


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
    }
    WORDSET_Delete( wordSet );
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
  u32 height = 0;
  const u32 isEgg = PPP_Get( ppp , ID_PARA_tamago_flag , NULL );

  //性格
  if( isEgg == 0 )
  {
    const u32 seikaku = PPP_Get( ppp , ID_PARA_seikaku , NULL );
    STRBUF *srcStr = GFL_MSG_CreateString( infoWork->msgMemo , trmemo_01_01 + seikaku );
    
    PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( infoWork->bmpWinUp ) , 
            PSTATUS_INFO_MEMO_STR_X , PSTATUS_INFO_MEMO_STR_Y + height, 
            srcStr , work->fontHandle , PSTATUS_STR_COL_VALUE );
    GFL_STR_DeleteBuffer( srcStr );
    
    height = 16;
  }
  
  //メモ
  {
    BOOL isParent;  //自分のポケか？
    u16 memoId;
    STRBUF *srcStr;
    STRBUF *dstStr = GFL_STR_CreateBuffer( 256 , work->heapId );
    STRBUF *placeStr1;
    STRBUF *placeStr2;
    WORDSET *wordSet  = WORDSET_CreateEx( 10 , WORDSET_DEFAULT_BUFLEN , work->heapId );
    const u32 year1   = PPP_Get( ppp , ID_PARA_birth_year , NULL );
    const u32 month1  = PPP_Get( ppp , ID_PARA_birth_month , NULL )+1;
    const u32 day1    = PPP_Get( ppp , ID_PARA_birth_day , NULL )+1;
    const u32 year2   = PPP_Get( ppp , ID_PARA_get_year , NULL );
    const u32 month2  = PPP_Get( ppp , ID_PARA_get_month , NULL )+1;
    const u32 day2    = PPP_Get( ppp , ID_PARA_get_day , NULL )+1;
    const u32 level   = PPP_Get( ppp , ID_PARA_get_level , NULL );
    const u32 isEvent = PPP_Get( ppp , ID_PARA_event_get_flag , NULL );
    //場所はポケシフター対応で書き換える可能性あり。
    u32 place1  = PPP_Get( ppp , ID_PARA_birth_place , NULL );
    u32 place2  = PPP_Get( ppp , ID_PARA_get_place , NULL );
    
    //自分のポケかチェック
    {
      MYSTATUS *myStatus = GAMEDATA_GetMyStatus( work->psData->game_data );
      isParent = PPP_IsMatchOya( ppp , myStatus );
    }

    //種類分岐

    if( isEgg == 0 )
    {
      if( place2 == 30001 )
      {
        //ポケシフター
        const u32 rom = PPP_Get( ppp , ID_PARA_get_cassette , NULL );
        u16 newRomMsg = trmemo_02_09_01;
        u16 oldRomMsg = trmemo_02_10_01;
        if( isEvent == TRUE )
        {
          newRomMsg = trmemo_02_11_01;
          oldRomMsg = trmemo_02_12_01;
        }

        switch( rom )
        {
        case VERSION_SAPPHIRE:  //1	///<	バージョン：AGBサファイア
        case VERSION_RUBY:		  //2	///<	バージョン：AGBルビー
        case VERSION_EMERALD:	  //3	///<	バージョン：AGBエメラルド
          place1 = 30006;
          memoId = oldRomMsg;
          break;
          
        case VERSION_RED:			  //4	///<	バージョン：AGBファイアーレッド
        case VERSION_GREEN:		  //5	///<	バージョン：AGBリーフグリーン
          place1 = 30004;
          memoId = oldRomMsg;
          break;

        case VERSION_GOLD:		  //7	///<	バージョン：ゴールド用予約定義
        case VERSION_SILVER:	  //8	///<	バージョン：シルバー用予約定義
          place1 = 30005;
          memoId = newRomMsg;
          break;

        case VERSION_DIAMOND:	  //10	///<	バージョン：DSダイヤモンド
        case VERSION_PEARL:		  //11	///<	バージョン：DSパール
        case VERSION_PLATINUM:  //12	///<	バージョン：DSプラチナ
          place1 = 30007;
          memoId = newRomMsg;
          break;

        case VERSION_COLOSSEUM: //15	///<	バージョン：GCコロシアム
          place1 = 30008;
          memoId = oldRomMsg;
          break;

        default:
          place1 = 30009;
          memoId = newRomMsg;
          break;
          
        }
      }
      else if( place2 >= 30010 && place2 <= 30013 )
      {
        //2010映画
        switch( place2 )
        {
        case 30010:
          memoId = trmemo_02_13_01;
          break;
        case 30011:
          memoId = trmemo_02_14_01;
          break;
        case 30012:
          memoId = trmemo_02_15_01;
          break;
        case 30013:
          memoId = trmemo_02_16_01;
          break;
        }
      }
      else
      {
        //その他
        if( isEvent == FALSE )
        {
          if( place1 == 0 )
          {
            if( place2 == 30002 )
            {
              //ゲーム内交換
              memoId = trmemo_02_01_01;
            }
            else
            {
              //通常捕獲
              memoId = trmemo_02_02_01;
            }
          }
          else
          {
            if( place1 <= 60000 )
            {
              //交換タマゴ孵化
              memoId = trmemo_02_03_01;
            }
            else
            {
              //ゲーム内タマゴ孵化
              memoId = trmemo_02_04_01;
            }
          }
        }
        else
        {
          //イベント配布
          if( place1 == 0 )
          {
            //不思議な贈り物経由
            memoId = trmemo_02_05_01;
          }
          else
          {
            //会場配布
            memoId = trmemo_02_06_01;
          }
        }
      }
      
      
      if( isParent == FALSE )
      {
        //タマゴじゃなければ、他人のポケの時のMsgは+1の位置にある。
        memoId++;
      }
    }
    else
    {
      //タマゴ
      if( isEvent == FALSE )
      {
        if( isParent == TRUE )
        {
          memoId = trmemo_01_T_01_01;
        }
        else
        {
          memoId = trmemo_01_T_01_02;
        }
      }
      else
      {
        if( isParent == TRUE )
        {
          memoId = trmemo_01_T_01_03;
        }
        else
        {
          memoId = trmemo_01_T_01_04;
        }
      }
    }

    
    //場所
    placeStr1 = PSTATUS_INFO_GetPlaceStr( work , infoWork , place1 );
    placeStr2 = PSTATUS_INFO_GetPlaceStr( work , infoWork , place2 );
    
    WORDSET_RegisterNumber( wordSet , 0 , year1 , 2 , STR_NUM_DISP_ZERO , STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterNumber( wordSet , 1 , month1 , 2 , STR_NUM_DISP_LEFT , STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterNumber( wordSet , 2 , day1 , 2 , STR_NUM_DISP_LEFT , STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterNumber( wordSet , 3 , level , 3 , STR_NUM_DISP_LEFT , STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterWord( wordSet , 4 , placeStr1 , 0,TRUE,PM_LANG );
    WORDSET_RegisterNumber( wordSet , 5 , year2 , 2 , STR_NUM_DISP_ZERO , STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterNumber( wordSet , 6 , month2 , 2 , STR_NUM_DISP_LEFT , STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterNumber( wordSet , 7 , day2 , 2 , STR_NUM_DISP_LEFT , STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterWord( wordSet , 8 , placeStr2 , 0,TRUE,PM_LANG );

    srcStr = GFL_MSG_CreateString( infoWork->msgMemo , memoId ); 
    WORDSET_ExpandStr( wordSet , dstStr , srcStr );
    PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( infoWork->bmpWinUp ) , 
            PSTATUS_INFO_MEMO_STR_X , PSTATUS_INFO_MEMO_STR_Y + height, 
            dstStr , work->fontHandle , PSTATUS_STR_COL_VALUE );

    WORDSET_Delete( wordSet );

    //TODO 許可制の関数
    height += PRINTSYS_GetLineCount( dstStr )*16;
    //TODO ずれてる・・・？
    //height = 16*4;
    
    GFL_STR_DeleteBuffer( placeStr1 );
    GFL_STR_DeleteBuffer( placeStr2 );
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
    STRBUF *srcStr;
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
    
    msgId = trmemo_03_01_00 + maxIdx*5 + maxRand%5;
    
    srcStr = GFL_MSG_CreateString( infoWork->msgMemo , msgId );
    PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( infoWork->bmpWinUp ) , 
            PSTATUS_INFO_MEMO_STR_X , PSTATUS_INFO_MEMO_STR_Y + height, 
            srcStr , work->fontHandle , PSTATUS_STR_COL_VALUE );
    GFL_STR_DeleteBuffer( srcStr );
    
  }
  else
  {
    //タマゴ
    u16 msgId;
    STRBUF *srcStr;
    const u32 natuki = PPP_Get( ppp , ID_PARA_friend , NULL );
    if( natuki <= 5 )
    {
      msgId = trmemo_01_T_02_01;
    }
    else
    if( natuki <= 10 )
    {
      msgId = trmemo_01_T_02_02;
    }
    else
    if( natuki <= 40 )
    {
      msgId = trmemo_01_T_02_03;
    }
    else
    {
      msgId = trmemo_01_T_02_04;
    }

    srcStr = GFL_MSG_CreateString( infoWork->msgMemo , msgId );
    PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( infoWork->bmpWinUp ) , 
            PSTATUS_INFO_MEMO_STR_X , PSTATUS_INFO_MEMO_STR_Y + height, 
            srcStr , work->fontHandle , PSTATUS_STR_COL_VALUE );
    GFL_STR_DeleteBuffer( srcStr );
  }
}


//地名変換
static STRBUF* PSTATUS_INFO_GetPlaceStr( PSTATUS_WORK *work , PSTATUS_INFO_WORK *infoWork , const u32 place )
{
  if( place <= 30000 )
  {
    if( place >= msg_place_name_max )
    {
      return GFL_MSG_CreateString( infoWork->msgPlace , MAPNAME_TOOIBASYO ); 
    }
    else
    {
      return GFL_MSG_CreateString( infoWork->msgPlace , place ); 
    }
  }
  else
  if( place <= 40000 )
  {
    const u32 temp = place-30001;
    if( temp >= msg_place_name_spe_max )
    {
      return GFL_MSG_CreateString( infoWork->msgPlace , MAPNAME_TOOIBASYO ); 
    }
    else
    {
      return GFL_MSG_CreateString( infoWork->msgPlaceSp , temp ); 
    }
  }
  else
  if( place <= 60000 )
  {
    const u32 temp = place-40001;
    if( temp >= msg_place_name_out_max )
    {
      return GFL_MSG_CreateString( infoWork->msgPlaceOut , MAPNAME_TOOIBASYO_OUT ); 
    }
    else
    {
      return GFL_MSG_CreateString( infoWork->msgPlaceOut , temp ); 
    }
  }
  else
  if( place <= 65535 )
  {
    const u32 temp = place-60001;
    if( temp >= msg_place_name_per_max )
    {
      return GFL_MSG_CreateString( infoWork->msgPlacePerson , MAPNAME_TOOKUNIIRUHITO ); 
    }
    else
    {
      return GFL_MSG_CreateString( infoWork->msgPlacePerson , temp ); 
    }
  }
  return GFL_MSG_CreateString( infoWork->msgPlace , MAPNAME_TOOIBASYO ); 
}
