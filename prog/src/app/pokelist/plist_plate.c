//======================================================================
/**
 * @file	plist_plate.c
 * @brief	ポケモンリスト プレート
 * @author	ariizumi
 * @data	09/06/10
 *
 * モジュール名：PLIST_PLATE
 */
//======================================================================
#include <gflib.h>
#include "buflen.h"
#include "system/main.h"
#include "system/gfl_use.h"

#include "arc_def.h"
#include "msg/msg_pokelist.h"

#include "item/item.h"
#include "poke_tool/monsno_def.h"
#include "poke_tool/poke_regulation.h"
#include "pokeicon/pokeicon.h"
#include "print/global_msg.h"

#include "plist_sys.h"
#include "plist_plate.h"

#include "test/ariizumi/ari_debug.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define PLIST_PLATE_WIDTH  (16)
#define PLIST_PLATE_HEIGHT (6)

//各表示物の位置
//プレートの左上からの相対座標
#define PLIST_PLATE_POKE_POS_X (24)
#define PLIST_PLATE_POKE_POS_Y (16)
#define PLIST_PLATE_POKE_ACTIVE_POS_X (26)
#define PLIST_PLATE_POKE_ACTIVE_POS_Y (19)
#define PLIST_PLATE_BALL_POS_X (16)
#define PLIST_PLATE_BALL_POS_Y (16)
#define PLIST_PLATE_HPBASE_POS_X (84)
#define PLIST_PLATE_HPBASE_POS_Y (28)
#define PLIST_PLATE_ITEM_POS_X (PLIST_PLATE_POKE_POS_X+8)
#define PLIST_PLATE_ITEM_POS_Y (PLIST_PLATE_POKE_POS_Y+8)
#define PLIST_PLATE_CONDITION_POS_X (32)
#define PLIST_PLATE_CONDITION_POS_Y (42)

//文字の位置
#define PLIST_PLATE_STR_NAME_X (40+PLIST_MSG_STR_OFS_X)
#define PLIST_PLATE_STR_NAME_Y ( 8+PLIST_MSG_STR_OFS_Y)
#define PLIST_PLATE_STR_SEX_X (104+PLIST_MSG_STR_OFS_X)
#define PLIST_PLATE_STR_SEX_Y ( 8+PLIST_MSG_STR_OFS_Y)

#define PLIST_PLATE_NUM_BASAE_Y (34+PLIST_MSG_STR_OFS_Y)  //LvとHpのY座標
#define PLIST_PLATE_STR_LEVEL_X (12+PLIST_MSG_STR_OFS_X)
#define PLIST_PLATE_STR_LEVEL_Y (PLIST_PLATE_NUM_BASAE_Y)
#define PLIST_PLATE_STR_HP_X (64+PLIST_MSG_STR_OFS_X)
#define PLIST_PLATE_STR_HP_Y (PLIST_PLATE_NUM_BASAE_Y)
#define PLIST_PLATE_STR_SLASH_X (PLIST_PLATE_STR_HP_X+24+PLIST_MSG_STR_OFS_X)
#define PLIST_PLATE_STR_SLASH_Y (PLIST_PLATE_NUM_BASAE_Y)
#define PLIST_PLATE_STR_HPMAX_X (PLIST_PLATE_STR_SLASH_X+8+PLIST_MSG_STR_OFS_X)
#define PLIST_PLATE_STR_HPMAX_Y (PLIST_PLATE_NUM_BASAE_Y)

#define PLIST_PLATE_STR_BTL_ORDER_X (44+PLIST_MSG_STR_OFS_X)
#define PLIST_PLATE_STR_BTL_ORDER_Y (PLIST_PLATE_NUM_BASAE_Y-4)

//HPバー系
#define PLIST_PLATE_HPBAR_LEN (48)
#define PLIST_PLATE_HPBAR_TOP (PLIST_PLATE_HPBASE_POS_Y-2)
#define PLIST_PLATE_HPBAR_LEFT (PLIST_PLATE_HPBASE_POS_X-23)

#define PLIST_PLATE_CELLNUM (6)
#define PLIST_RENDER_MAIN (0)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
//状態異常アイコンのIDX
enum
{
  POKE_CONICON_POKERUSU,
  POKE_CONICON_MAHI,
  POKE_CONICON_KOORI,
  POKE_CONICON_NEMURI,
  POKE_CONICON_DOKU,
  POKE_CONICON_YAKEDO,
  POKE_CONICON_HINSI,
};

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
struct _PLIST_PLATE_WORK
{
  POKEMON_PARAM *pp;
  u8 idx;
  u16 dispHp;  //HPだけアニメのため値を保存する
  u16 nowHp;   //実際のHp
  BOOL isEgg;
  BOOL isActive;
  BOOL isBlank;
  BOOL isUpdateStr;

  PLIST_PLATE_BATTLE_ORDER btlOrder;
  GFL_BMPWIN *bmpWin;

  //Obj系  
  GFL_CLSYS_REND *cellRender;
  GFL_CLUNIT  *cellUnit;
  u32      pokeIconNcgRes;

  GFL_CLWK *pokeIcon;
  GFL_CLWK *ballIcon;
  GFL_CLWK *hpBase;
  GFL_CLWK *itemIcon;
  GFL_CLWK *conditionIcon;
  
};
//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

//プレートの左上位置(キャラ単位)
static const u8 PLIST_PLATE_POS_ARR[6][2] =
{
  {0, 1},{16, 2},
  {0, 7},{16, 8},
  {0,13},{16,14},
};

//プレートの素材の座標(0用と1～用
static const u8 PLATE_SCR_POS_ARR[2][2] ={{0,1},{16,2}};

static void PLIST_PLATE_CreateCell( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork );
static void PLIST_PLATE_CreatePokeIcon( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork );
static void PLIST_PLATE_DrawParamMain( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork );
static void PLIST_PLATE_DrawParam( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork );
static void PLIST_PLATE_DrawHPBar( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork );
static void PLIST_PLATE_DrawParamEgg( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork );
static const u8 PLIST_PLATE_GetHPRate( PLIST_PLATE_WORK *plateWork );
static void PLIST_PLATE_CalcCellPos( PLIST_PLATE_WORK *plateWork , const s16 x , const s16 y , GFL_CLACTPOS *pos );

static void PLIST_PLATE_CheckBattleOrder( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork );

//--------------------------------------------------------------
//	プレート作成
//--------------------------------------------------------------
PLIST_PLATE_WORK* PLIST_PLATE_CreatePlate( PLIST_WORK *work , const u8 idx , POKEMON_PARAM *pp )
{
  u8 i;
  const u8 baseX = PLIST_PLATE_POS_ARR[idx][0];
  const u8 baseY = PLIST_PLATE_POS_ARR[idx][1];
  PLIST_PLATE_WORK *plateWork;
  plateWork = GFL_HEAP_AllocMemory( work->heapId , sizeof(PLIST_PLATE_WORK) );
  plateWork->idx = idx;
  plateWork->pp = pp;
  plateWork->isBlank = FALSE;
  plateWork->isActive = FALSE;
  plateWork->isUpdateStr = FALSE;
  //HPだけアニメのため値を保存する
  plateWork->dispHp = PP_Get( plateWork->pp , ID_PARA_hp , NULL );
  plateWork->nowHp = plateWork->dispHp;
  //BG系
  GFL_BG_WriteScreenExpand( PLIST_BG_PLATE , 
                      baseX+PLIST_BG_SCROLL_X_CHAR ,baseY ,
                      PLIST_PLATE_WIDTH , PLIST_PLATE_HEIGHT ,
                      &work->plateScrData->rawData , 
                      PLATE_SCR_POS_ARR[(idx==0?0:1)][0] , PLATE_SCR_POS_ARR[(idx==0?0:1)][1] ,
                      32 , 32 );  //←グラフィックデータのサイズ
  PLIST_PLATE_ChangeColor( work , plateWork , PPC_NORMAL );
  
  plateWork->isEgg =  PP_Get( plateWork->pp , ID_PARA_tamago_flag , NULL );

  //Obj系
  {
    //ユニットとレンダーをそれぞれ作る
    GFL_REND_SURFACE_INIT renderInit;
    renderInit.lefttop_x = 0;
    renderInit.lefttop_y = 0;
    renderInit.width  = 256;
    renderInit.height = 192;
    renderInit.type = CLSYS_DRAW_MAIN;
    plateWork->cellRender = GFL_CLACT_USERREND_Create( &renderInit , 1 , work->heapId );
    plateWork->cellUnit = GFL_CLACT_UNIT_Create( PLIST_PLATE_CELLNUM , PLIST_CELLUNIT_PRI_PLATE , work->heapId );
    GFL_CLACT_UNIT_SetUserRend( plateWork->cellUnit , plateWork->cellRender );
  }
  //セル作成
  PLIST_PLATE_CreateCell( work , plateWork );
  PLIST_PLATE_CreatePokeIcon( work , plateWork );
  {
    //設定が終わったらレンダラを動かす
    GFL_CLACTPOS surfacePos;
    surfacePos.x = -baseX*8;
    surfacePos.y = -baseY*8;
    GFL_CLACT_USERREND_SetSurfacePos( plateWork->cellRender , PLIST_RENDER_MAIN , &surfacePos );
  }
  
  plateWork->bmpWin = GFL_BMPWIN_Create( PLIST_BG_PARAM , baseX+PLIST_BG_SCROLL_X_CHAR , baseY , 
          PLIST_PLATE_WIDTH , PLIST_PLATE_HEIGHT , PLIST_BG_PLT_HP_BAR , GFL_BMP_CHRAREA_GET_B );
  
  //バトルチェック
  PLIST_PLATE_CheckBattleOrder( work , plateWork );
      
  //参加番号のチェック
  for( i=0;i<6;i++ )
  {
    if( work->plData->in_num[i] == plateWork->idx +1 )
    {
      plateWork->btlOrder = i;
    }
  }

  PLIST_PLATE_DrawParamMain( work , plateWork );
  return plateWork;
}

//--------------------------------------------------------------
//	プレート作成(空
//--------------------------------------------------------------
PLIST_PLATE_WORK* PLIST_PLATE_CreatePlate_Blank( PLIST_WORK *work , const u8 idx )
{
  PLIST_PLATE_WORK *plateWork;
  static const u8 PLATE_SCR_POS_ARR[2][2] ={{0,1},{16,2}};
  plateWork = GFL_HEAP_AllocMemory( work->heapId , sizeof(PLIST_PLATE_WORK) );
  
  //BG系
  GFL_BG_WriteScreenExpand( PLIST_BG_PLATE , 
                      PLIST_PLATE_POS_ARR[idx][0]+PLIST_BG_SCROLL_X_CHAR ,PLIST_PLATE_POS_ARR[idx][1] ,
                      PLIST_PLATE_WIDTH , PLIST_PLATE_HEIGHT ,
                      &work->plateScrData->rawData , 0 , 19 , 32 , 32 ); 
  
  plateWork->idx = idx;
  plateWork->pp = NULL;
  plateWork->isBlank = TRUE;
  plateWork->btlOrder = PPBO_INVALLID;
  GFL_BG_LoadScreenV_Req( PLIST_BG_PLATE );

  return plateWork;
}

//--------------------------------------------------------------
//	プレート削除
//--------------------------------------------------------------
void PLIST_PLATE_DeletePlate( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork )
{
  if( plateWork->isBlank == FALSE )
  {
    GFL_BMPWIN_Delete( plateWork->bmpWin );
    
    GFL_CLACT_WK_Remove( plateWork->pokeIcon );
    GFL_CLACT_WK_Remove( plateWork->ballIcon );
    GFL_CLACT_WK_Remove( plateWork->hpBase );
    GFL_CLACT_USERREND_Delete( plateWork->cellRender );
    GFL_CLACT_UNIT_Delete( plateWork->cellUnit );

    GFL_CLGRP_CGR_Release( plateWork->pokeIconNcgRes );
  }
  GFL_HEAP_FreeMemory( plateWork );
}

//--------------------------------------------------------------
//	プレート更新
//--------------------------------------------------------------
void PLIST_PLATE_UpdatePlate( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork )
{
  if( plateWork->isBlank == FALSE )
  {
    if( plateWork->isUpdateStr == TRUE )
    {
      if( PRINTSYS_QUE_IsExistTarget( work->printQue , GFL_BMPWIN_GetBmp( plateWork->bmpWin )) == FALSE )
      {
        plateWork->isUpdateStr = FALSE;
        GFL_BMPWIN_MakeTransWindow_VBlank( plateWork->bmpWin );
      }
    }
    
    if( plateWork->isActive == TRUE )
    {
      //選択中アイコンのアニメ
      if( PP_GetSick( plateWork->pp ) == POKESICK_NULL &&
          plateWork->dispHp != 0 )
      {
        GFL_CLACTPOS cellPos;
        PLIST_PLATE_CalcCellPos( plateWork , 
                                 PLIST_PLATE_POKE_ACTIVE_POS_X,
                                 PLIST_PLATE_POKE_ACTIVE_POS_Y,
                                 &cellPos );
        if( GFL_CLACT_WK_GetAnmFrame(plateWork->pokeIcon) == 1 )
        {
          cellPos.y -= 6;
        }
        GFL_CLACT_WK_SetPos( plateWork->pokeIcon , &cellPos , CLSYS_DRAW_MAIN );
      }
    }
  }
}


//--------------------------------------------------------------
//	セル作成
//--------------------------------------------------------------
static void PLIST_PLATE_CreateCell( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork )
{
  //ボールアイコン
  {
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_x = PLIST_PLATE_BALL_POS_X;
    cellInitData.pos_y = PLIST_PLATE_BALL_POS_Y;
    cellInitData.anmseq = 0;
    cellInitData.softpri = 1;
    cellInitData.bgpri = 2;
    plateWork->ballIcon = GFL_CLACT_WK_Create( plateWork->cellUnit ,
              work->cellRes[PCR_NCG_BALL],
              work->cellRes[PCR_PLT_OBJ_COMMON],
              work->cellRes[PCR_ANM_BALL],
              &cellInitData ,PLIST_RENDER_MAIN , work->heapId );
    GFL_CLACT_WK_SetAutoAnmFlag( plateWork->ballIcon , TRUE );
  }
  
  //HP土台
  {
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_x = PLIST_PLATE_HPBASE_POS_X;
    cellInitData.pos_y = PLIST_PLATE_HPBASE_POS_Y;
    cellInitData.anmseq = 0;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 2;
    plateWork->hpBase = GFL_CLACT_WK_Create( plateWork->cellUnit ,
              work->cellRes[PCR_NCG_HP_BASE],
              work->cellRes[PCR_PLT_HP_BASE],
              work->cellRes[PCR_ANM_HP_BASE],
              &cellInitData ,PLIST_RENDER_MAIN , work->heapId );
  }
  //アイテムアイコン
  {
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_x = PLIST_PLATE_ITEM_POS_X;
    cellInitData.pos_y = PLIST_PLATE_ITEM_POS_Y;
    cellInitData.anmseq = 0;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 2;
    plateWork->itemIcon = GFL_CLACT_WK_Create( plateWork->cellUnit ,
              work->cellRes[PCR_NCG_ITEM_ICON],
              work->cellRes[PCR_PLT_ITEM_ICON],
              work->cellRes[PCR_ANM_ITEM_ICON],
              &cellInitData ,PLIST_RENDER_MAIN , work->heapId );
    GFL_CLACT_WK_SetDrawEnable( plateWork->itemIcon , FALSE );
  }

  //状態異常アイコン
  {
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_x = PLIST_PLATE_CONDITION_POS_X;
    cellInitData.pos_y = PLIST_PLATE_CONDITION_POS_Y;
    cellInitData.anmseq = 0;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 2;
    plateWork->conditionIcon = GFL_CLACT_WK_Create( plateWork->cellUnit ,
              work->cellRes[PCR_NCG_CONDITION],
              work->cellRes[PCR_PLT_CONDITION],
              work->cellRes[PCR_ANM_CONDITION],
              &cellInitData ,PLIST_RENDER_MAIN , work->heapId );
    GFL_CLACT_WK_SetDrawEnable( plateWork->conditionIcon , FALSE );
  }
}
//--------------------------------------------------------------
//	ポケモンアイコン作成
//--------------------------------------------------------------
static void PLIST_PLATE_CreatePokeIcon( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork )
{
  //ポケアイコン
  {
    ARCHANDLE *arcHandle = GFL_ARC_OpenDataHandle( ARCID_POKEICON , work->heapId );
    const POKEMON_PARAM *pokePara = PokeParty_GetMemberPointer( work->plData->pp , plateWork->idx );
    const POKEMON_PASO_PARAM *ppp = PP_GetPPPPointerConst( pokePara );
    GFL_CLWK_DATA cellInitData;
    u8 pltNum;

    plateWork->pokeIconNcgRes = GFL_CLGRP_CGR_Register( arcHandle , 
        POKEICON_GetCgxArcIndex(ppp) , FALSE , CLSYS_DRAW_MAIN , work->heapId  );
    
    GFL_ARC_CloseDataHandle(arcHandle);

    pltNum = POKEICON_GetPalNumGetByPPP( ppp );
    cellInitData.pos_x = PLIST_PLATE_POKE_POS_X;
    cellInitData.pos_y = PLIST_PLATE_POKE_POS_Y;
    cellInitData.anmseq = POKEICON_ANM_HPMAX;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 2;
    plateWork->pokeIcon = GFL_CLACT_WK_Create( plateWork->cellUnit ,
              plateWork->pokeIconNcgRes,
              work->cellRes[PCR_PLT_POKEICON],
              work->cellRes[PCR_ANM_POKEICON],
              &cellInitData ,PLIST_RENDER_MAIN , work->heapId );
    GFL_CLACT_WK_SetPlttOffs( plateWork->pokeIcon , pltNum , CLWK_PLTTOFFS_MODE_PLTT_TOP );
    GFL_CLACT_WK_SetAutoAnmFlag( plateWork->pokeIcon , TRUE );
  }
}

//--------------------------------------------------------------
//	パラメータ描画メイン
//--------------------------------------------------------------
static void PLIST_PLATE_DrawParamMain( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork )
{
  if( plateWork->isEgg == FALSE )
  {
    PLIST_PLATE_DrawParam( work , plateWork );
    PLIST_PLATE_DrawHPBar( work , plateWork );
  }
  else
  {
    PLIST_PLATE_DrawParamEgg( work , plateWork );
  }
}

//--------------------------------------------------------------
//	文字列描画
//--------------------------------------------------------------
static void PLIST_PLATE_DrawParam( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork )
{
  const PRINTSYS_LSB fontCol = PRINTSYS_LSB_Make( PLIST_FONT_PARAM_LETTER , PLIST_FONT_PARAM_SHADOW , 0 );
  //名前
  {
    WORDSET *wordSet = WORDSET_Create( work->heapId );
    WORDSET_RegisterPokeNickName( wordSet , 0 , plateWork->pp );

    PLIST_UTIL_DrawValueStrFunc( work , plateWork->bmpWin , 
                      wordSet , mes_pokelist_01_09 ,
                      PLIST_PLATE_STR_NAME_X , PLIST_PLATE_STR_NAME_Y , fontCol );

    WORDSET_Delete( wordSet );
  }
  //性別
  {
    u32 sex = PP_Get( plateWork->pp , ID_PARA_sex , NULL );
    if( sex == PTL_SEX_MALE )
    {
      const PRINTSYS_LSB fontColBlue = PRINTSYS_LSB_Make( PLIST_FONT_PARAM_LETTER_BLUE , PLIST_FONT_PARAM_SHADOW_BLUE , 0 );
      PLIST_UTIL_DrawStrFunc( work , plateWork->bmpWin , mes_pokelist_01_28 ,
                      PLIST_PLATE_STR_SEX_X , PLIST_PLATE_STR_SEX_Y , fontColBlue );
    }
    else if( sex == PTL_SEX_FEMALE )
    {
      const PRINTSYS_LSB fontColRed = PRINTSYS_LSB_Make( PLIST_FONT_PARAM_LETTER_RED , PLIST_FONT_PARAM_SHADOW_RED , 0 );
      PLIST_UTIL_DrawStrFunc( work , plateWork->bmpWin , mes_pokelist_01_29 ,
                      PLIST_PLATE_STR_SEX_X , PLIST_PLATE_STR_SEX_Y , fontColRed );
    }
  }

  //レベル
  if( (PP_GetSick( plateWork->pp ) == POKESICK_NULL &&
      plateWork->dispHp != 0) ||
      plateWork->btlOrder != PPBO_INVALLID )  //バトルのときはレベル表示
  {
    
    WORDSET *wordSet = WORDSET_Create( work->heapId );

    u32 lv = PP_CalcLevel( plateWork->pp );
    WORDSET_RegisterNumber( wordSet , 0 , lv , 3 , STR_NUM_DISP_LEFT , STR_NUM_CODE_DEFAULT );

    PLIST_UTIL_DrawValueStrFuncSys( work , plateWork->bmpWin , 
                      wordSet , mes_pokelist_01_03 , 
                      PLIST_PLATE_STR_LEVEL_X , PLIST_PLATE_STR_LEVEL_Y , fontCol );

    WORDSET_Delete( wordSet );

    GFL_CLACT_WK_SetDrawEnable( plateWork->conditionIcon , FALSE );
  }
  else
  {
    //レベルの代わりに状態異常アイコン
    if( plateWork->dispHp == 0 )
    {
      GFL_CLACT_WK_SetAnmSeq( plateWork->conditionIcon , POKE_CONICON_HINSI );
    }
    else
    {
      switch(PP_GetSick( plateWork->pp ))
      {
      case POKESICK_MAHI:   ///< まひ
        GFL_CLACT_WK_SetAnmSeq( plateWork->conditionIcon , POKE_CONICON_MAHI );
        break;
      case POKESICK_NEMURI: ///< ねむり
        GFL_CLACT_WK_SetAnmSeq( plateWork->conditionIcon , POKE_CONICON_NEMURI );
        break;
      case POKESICK_KOORI:  ///< こおり
        GFL_CLACT_WK_SetAnmSeq( plateWork->conditionIcon , POKE_CONICON_KOORI );
        break;
      case POKESICK_YAKEDO: ///< やけど
        GFL_CLACT_WK_SetAnmSeq( plateWork->conditionIcon , POKE_CONICON_YAKEDO );
        break;
      case POKESICK_DOKU:   ///< どく
        GFL_CLACT_WK_SetAnmSeq( plateWork->conditionIcon , POKE_CONICON_DOKU );
        break;
      }
    }
    GFL_CLACT_WK_SetDrawEnable( plateWork->conditionIcon , TRUE );
  }
  
  //HPorバトル参加順or技教え
  if( plateWork->btlOrder != PPBO_INVALLID )
  {
    //バトル参加順
    u32 strId;
    STRBUF *srcStr;
    if( plateWork->btlOrder == PPBO_JOIN_OK )
    {
      strId = mes_pokelist_06_01;
    }
    else
    if( plateWork->btlOrder == PPBO_JOIN_NG )
    {
      strId = mes_pokelist_06_02;
    }
    else
    {
      strId = mes_pokelist_06_03 + plateWork->btlOrder;
    }
    
    PLIST_UTIL_DrawStrFunc( work , plateWork->bmpWin , strId ,
                    PLIST_PLATE_STR_BTL_ORDER_X , PLIST_PLATE_STR_BTL_ORDER_Y , fontCol );
    
    //HPバー非表示
    GFL_CLACT_WK_SetDrawEnable( plateWork->hpBase , FALSE );
  }
  else
  if( work->plData->mode == PL_MODE_WAZASET ||
      work->isSetWazaMode == TRUE )
  {
    u32 strId;
    const PLIST_SKILL_CAN_LEARN learnType = PLIST_UTIL_CheckLearnSkill( work , plateWork->pp , plateWork->idx );
    
    switch( learnType )
    {
    case LSCL_OK:
    case LSCL_OK_FULL:
      strId = mes_pokelist_06_07;
      break;
      
    case LSCL_NG:
      strId = mes_pokelist_06_08;
      break;
      
    case LSCL_LEARN:
      strId = mes_pokelist_06_09;
      break;
    }
    PLIST_UTIL_DrawStrFunc( work , plateWork->bmpWin , strId ,
                    PLIST_PLATE_STR_BTL_ORDER_X , PLIST_PLATE_STR_BTL_ORDER_Y , fontCol );
    //HPバー非表示
    GFL_CLACT_WK_SetDrawEnable( plateWork->hpBase , FALSE );
  }
  else
  if( work->plData->mode == PL_MODE_SHINKA )
  {
    const u16 evoMonsNo = PLIST_UTIL_CheckItemEvolution( work , plateWork->pp , work->plData->item );
    u32 strId;
    
    if( evoMonsNo == 0 )
    {
      //進化不可
      strId = mes_pokelist_06_11;
    }
    else
    {
      strId = mes_pokelist_06_10;
      //進化可
    }
    
    PLIST_UTIL_DrawStrFunc( work , plateWork->bmpWin , strId ,
                    PLIST_PLATE_STR_BTL_ORDER_X , PLIST_PLATE_STR_BTL_ORDER_Y , fontCol );
    //HPバー非表示
    GFL_CLACT_WK_SetDrawEnable( plateWork->hpBase , FALSE );
  }
  else
  {
    //HP描画
    {
      WORDSET *wordSet = WORDSET_Create( work->heapId );

      u32 hpMax = PP_Get( plateWork->pp , ID_PARA_hpmax , NULL );
      WORDSET_RegisterNumber( wordSet , 0 , hpMax , 3 , STR_NUM_DISP_LEFT , STR_NUM_CODE_DEFAULT );

      PLIST_UTIL_DrawValueStrFuncSys( work , plateWork->bmpWin , 
                        wordSet , mes_pokelist_01_15 , 
                        PLIST_PLATE_STR_HPMAX_X , PLIST_PLATE_STR_HPMAX_Y , fontCol );

      WORDSET_Delete( wordSet );
    }
    //HP現在
    {
      WORDSET *wordSet = WORDSET_Create( work->heapId );
      u32 hp = plateWork->dispHp;
      WORDSET_RegisterNumber( wordSet , 0 , hp , 3 , STR_NUM_DISP_SPACE , STR_NUM_CODE_DEFAULT );

      PLIST_UTIL_DrawValueStrFuncSys( work , plateWork->bmpWin , 
                        wordSet , mes_pokelist_01_21 , 
                        PLIST_PLATE_STR_HP_X , PLIST_PLATE_STR_HP_Y , fontCol );

      WORDSET_Delete( wordSet );
    }
    //HPのスラッシュ
    {
      PLIST_UTIL_DrawStrFuncSys( work , plateWork->bmpWin , mes_pokelist_01_27 ,
                      PLIST_PLATE_STR_SLASH_X , PLIST_PLATE_STR_SLASH_Y , fontCol );
    }
  }
  
  //アイテムアイコン
  {
    u32 itemId = PP_Get( plateWork->pp , ID_PARA_item , NULL );
    //TODO メールチェック
    if( itemId != 0 )
    {
      GFL_CLACT_WK_SetDrawEnable( plateWork->itemIcon , TRUE );
      if( ITEM_CheckMail( itemId ) == TRUE )
      {
        GFL_CLACT_WK_SetAnmSeq( plateWork->itemIcon , 1 );
      }
      else
      {
        GFL_CLACT_WK_SetAnmSeq( plateWork->itemIcon , 0 );
      }
    }
    else
    {
      GFL_CLACT_WK_SetDrawEnable( plateWork->itemIcon , FALSE );
    }
  }
 
  //ポケアイコンアニメ
  {
    const u8 rate = PLIST_PLATE_GetHPRate( plateWork );
    u16 anmSeq = POKEICON_ANM_HPMAX;
    if( rate == 0 )
    {
      anmSeq = POKEICON_ANM_DEATH;
    }
    else
    if( PP_GetSick( plateWork->pp ) != POKESICK_NULL )
    {
      anmSeq = POKEICON_ANM_STCHG;
    }
    else
    if( rate <= 25 )
    {
      anmSeq = POKEICON_ANM_HPRED;
    }
    else
    if( rate <= 50 )
    {
      anmSeq = POKEICON_ANM_HPYERROW;
    }
    else
    if( rate < 100 )
    {
      anmSeq = POKEICON_ANM_HPGREEN;
    }
    
    GFL_CLACT_WK_SetAnmSeq( plateWork->pokeIcon , anmSeq );
      
  }

  plateWork->isUpdateStr = TRUE;
  
}

//--------------------------------------------------------------
//	HPバー表示
//--------------------------------------------------------------
static void PLIST_PLATE_DrawHPBar( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork )
{
  if( PLIST_UTIL_IsBattleMenu( work ) == FALSE && 
      work->plData->mode != PL_MODE_WAZASET )
  {
    const u8 rate = PLIST_PLATE_GetHPRate( plateWork );
    u8 len = PLIST_PLATE_HPBAR_LEN*rate/100;
    u8 inCol,outCol;
    GFL_BMP_DATA *bmp = GFL_BMPWIN_GetBmp( plateWork->bmpWin );
    
    //色決定
    if( rate <= 25 )
    {
      inCol  = PLIST_HPBAR_COL_RED_IN;
      outCol = PLIST_HPBAR_COL_RED_OUT;
    }
    else
    if( rate <= 50 )
    {
      inCol  = PLIST_HPBAR_COL_YELLOW_IN;
      outCol = PLIST_HPBAR_COL_YELLOW_OUT;
    }
    else
    {
      inCol  = PLIST_HPBAR_COL_GREEN_IN;
      outCol = PLIST_HPBAR_COL_GREEN_OUT;
    }
    
    GFL_BMP_Fill( bmp , PLIST_PLATE_HPBAR_LEFT , PLIST_PLATE_HPBAR_TOP ,
                  len , 1 , outCol );
    GFL_BMP_Fill( bmp , PLIST_PLATE_HPBAR_LEFT , PLIST_PLATE_HPBAR_TOP+1 ,
                  len , 2 , inCol );
    GFL_BMP_Fill( bmp , PLIST_PLATE_HPBAR_LEFT , PLIST_PLATE_HPBAR_TOP+3 ,
                  len , 1 , outCol );

    //HPバー表示
    GFL_CLACT_WK_SetDrawEnable( plateWork->hpBase , TRUE );
  }

}

//--------------------------------------------------------------
//	タマゴ用描画
//--------------------------------------------------------------
static void PLIST_PLATE_DrawParamEgg( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork )
{
  const PRINTSYS_LSB fontCol = PRINTSYS_LSB_Make( PLIST_FONT_PARAM_LETTER , PLIST_FONT_PARAM_SHADOW , 0 );
  //名前
  {
    WORDSET *wordSet = WORDSET_Create( work->heapId );
    
    STRBUF *tmpStr = GFL_MSG_CreateString( GlobalMsg_PokeName, MONSNO_TAMAGO );
    WORDSET_RegisterWord( wordSet, 0, tmpStr, 0,TRUE,PM_LANG );

    PLIST_UTIL_DrawValueStrFunc( work , plateWork->bmpWin , 
                      wordSet , mes_pokelist_01_09 ,
                      PLIST_PLATE_STR_NAME_X , PLIST_PLATE_STR_NAME_Y , fontCol );

    GFL_STR_DeleteBuffer( tmpStr );
    WORDSET_Delete( wordSet );
  }

  //HPバー非表示
  GFL_CLACT_WK_SetDrawEnable( plateWork->hpBase , FALSE );

  //ポケアイコンアニメ
  {
    GFL_CLACT_WK_SetAnmSeq( plateWork->pokeIcon , POKEICON_ANM_HPMAX );
  }

  plateWork->isUpdateStr = TRUE;
  
}
//--------------------------------------------------------------
//	プレート選択状態のON・OFF
//--------------------------------------------------------------
void PLIST_PLATE_SetActivePlate( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork , const BOOL isActive )
{
  if( isActive == TRUE )
  {
    if( work->mainSeq == PSMS_CHANGE_POKE ||
        work->mainSeq == PSMS_USE_POKE )
    {
      PLIST_PLATE_ChangeColor( work , plateWork , PPC_CHANGE );
    }
    else
    {
      PLIST_PLATE_ChangeColor( work , plateWork , PPC_NORMAL_SELECT );
    }
    GFL_CLACT_WK_SetAnmSeq( plateWork->ballIcon , 1 );
    {
      GFL_CLACTPOS cellPos;
      PLIST_PLATE_CalcCellPos( plateWork , 
                               PLIST_PLATE_POKE_ACTIVE_POS_X,
                               PLIST_PLATE_POKE_ACTIVE_POS_Y,
                               &cellPos );
      GFL_CLACT_WK_SetPos( plateWork->pokeIcon , &cellPos , CLSYS_DRAW_MAIN );
    }
  }
  else
  {
    //入れ替え中で、入れ替えターゲットのときは色を戻さない
    if( ( !(work->mainSeq == PSMS_CHANGE_POKE && work->changeTarget == plateWork->idx)) &&
        ( !(work->mainSeq == PSMS_USE_POKE && work->useTarget == plateWork->idx) ) )
    {
      PLIST_PLATE_ChangeColor( work , plateWork , PPC_NORMAL );
    }
    GFL_CLACT_WK_SetAnmSeq( plateWork->ballIcon , 0 );
    {
      GFL_CLACTPOS cellPos;
      PLIST_PLATE_CalcCellPos( plateWork , 
                               PLIST_PLATE_POKE_POS_X,
                               PLIST_PLATE_POKE_POS_Y,
                               &cellPos );
      GFL_CLACT_WK_SetPos( plateWork->pokeIcon , &cellPos , CLSYS_DRAW_MAIN );
    }
  }
  plateWork->isActive = isActive;
}

//--------------------------------------------------------------
//	色変え
//--------------------------------------------------------------
void PLIST_PLATE_ChangeColor( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork , PLIST_PLATE_COLTYPE col )
{
  if( col == PPC_NORMAL && 
      PLIST_PLATE_GetHPRate( plateWork ) == 0 )
  {
    GFL_BG_ChangeScreenPalette( PLIST_BG_PLATE , 
                PLIST_PLATE_POS_ARR[plateWork->idx][0]+PLIST_BG_SCROLL_X_CHAR ,
                PLIST_PLATE_POS_ARR[plateWork->idx][1] ,
                PLIST_PLATE_WIDTH , PLIST_PLATE_HEIGHT , PPC_DEATH );
  }
  else
  if( col == PPC_NORMAL_SELECT && 
      PLIST_PLATE_GetHPRate( plateWork ) == 0 )
  {
    GFL_BG_ChangeScreenPalette( PLIST_BG_PLATE , 
                PLIST_PLATE_POS_ARR[plateWork->idx][0]+PLIST_BG_SCROLL_X_CHAR ,
                PLIST_PLATE_POS_ARR[plateWork->idx][1] ,
                PLIST_PLATE_WIDTH , PLIST_PLATE_HEIGHT , PPC_DEATH_SELECT );
  }
  else
  {
    GFL_BG_ChangeScreenPalette( PLIST_BG_PLATE , 
                PLIST_PLATE_POS_ARR[plateWork->idx][0]+PLIST_BG_SCROLL_X_CHAR ,
                PLIST_PLATE_POS_ARR[plateWork->idx][1] ,
                PLIST_PLATE_WIDTH , PLIST_PLATE_HEIGHT , col );
  }

  GFL_BG_LoadScreenV_Req( PLIST_BG_PLATE );
}

//--------------------------------------------------------------
//入れ替えアニメでプレートを移動させる(キャラ単位
//--------------------------------------------------------------
void PLIST_PLATE_MovePlate( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork , const u8 moveValue )
{
  const u8 baseX = PLIST_PLATE_POS_ARR[plateWork->idx][0];
  const u8 baseY = PLIST_PLATE_POS_ARR[plateWork->idx][1];
  
  const int value = (plateWork->idx%2==0 ? -moveValue : moveValue );
  
  GFL_BMPWIN_SetPosX( plateWork->bmpWin , 
            PLIST_PLATE_POS_ARR[plateWork->idx][0] + PLIST_BG_SCROLL_X_CHAR + value );
  
  {
    //レンダラを動かす
    GFL_CLACTPOS surfacePos;
    surfacePos.x = -(baseX+value)*8;
    surfacePos.y = -baseY*8;
    GFL_CLACT_USERREND_SetSurfacePos( plateWork->cellRender , PLIST_RENDER_MAIN , &surfacePos );
  }
  //BG系
  GFL_BG_WriteScreenExpand( PLIST_BG_PLATE , 
                      baseX+PLIST_BG_SCROLL_X_CHAR+value ,baseY ,
                      PLIST_PLATE_WIDTH , PLIST_PLATE_HEIGHT ,
                      &work->plateScrData->rawData , 
                      PLATE_SCR_POS_ARR[(plateWork->idx==0?0:1)][0] , PLATE_SCR_POS_ARR[(plateWork->idx==0?0:1)][1] ,
                      32 , 32 );  //←グラフィックデータのサイズ
  GFL_BG_ChangeScreenPalette( PLIST_BG_PLATE , 
              baseX+PLIST_BG_SCROLL_X_CHAR+value , baseY ,
              PLIST_PLATE_WIDTH , PLIST_PLATE_HEIGHT , PPC_CHANGE );

	GFL_BMPWIN_MakeScreen( plateWork->bmpWin );
  GFL_BG_LoadScreenV_Req( PLIST_BG_PARAM );
  GFL_BG_LoadScreenV_Req( PLIST_BG_PLATE );
}

//--------------------------------------------------------------
//プレートを移動させる(キャラ単位
//--------------------------------------------------------------
void PLIST_PLATE_MovePlateXY( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork , const s8 subX , const s8 subY )
{
  const int baseX = PLIST_PLATE_POS_ARR[plateWork->idx][0]+subX;
  const int baseY = PLIST_PLATE_POS_ARR[plateWork->idx][1]+subY;
  
  if( plateWork->isBlank == FALSE )
  {
    u8 palCol;
    GFL_BMPWIN_SetPosX( plateWork->bmpWin , 
              baseX + PLIST_BG_SCROLL_X_CHAR );
    GFL_BMPWIN_SetPosY( plateWork->bmpWin , 
              baseY );
  	GFL_BMPWIN_MakeScreen( plateWork->bmpWin );
  
    {
      //レンダラを動かす
      GFL_CLACTPOS surfacePos;
      surfacePos.x = -baseX*8;
      surfacePos.y = -baseY*8;
      GFL_CLACT_USERREND_SetSurfacePos( plateWork->cellRender , PLIST_RENDER_MAIN , &surfacePos );
    }
    //BG系
    GFL_BG_WriteScreenExpand( PLIST_BG_PLATE , 
                        baseX+PLIST_BG_SCROLL_X_CHAR ,baseY ,
                        PLIST_PLATE_WIDTH , PLIST_PLATE_HEIGHT ,
                        &work->plateScrData->rawData , 
                        PLATE_SCR_POS_ARR[(plateWork->idx==0?0:1)][0] , PLATE_SCR_POS_ARR[(plateWork->idx==0?0:1)][1] ,
                        32 , 32 );  //←グラフィックデータのサイズ
    if( plateWork->idx == work->changeTarget )
    {
      palCol = PPC_CHANGE;
    }
    else
    if( plateWork->idx == work->pokeCursor )
    {
      if( PLIST_PLATE_GetHPRate( plateWork ) == 0 )
      {
        palCol = PPC_DEATH_SELECT;
      }
      else
      {
        palCol = PPC_NORMAL_SELECT;
      }
    }
    else
    {
      if( PLIST_PLATE_GetHPRate( plateWork ) == 0 )
      {
        palCol = PPC_DEATH;
      }
      else
      {
        palCol = PPC_NORMAL;
      }
    }

    GFL_BG_ChangeScreenPalette( PLIST_BG_PLATE , 
                baseX+PLIST_BG_SCROLL_X_CHAR , baseY ,
                PLIST_PLATE_WIDTH , PLIST_PLATE_HEIGHT , palCol );
  }
  else
  {
    GFL_BG_WriteScreenExpand( PLIST_BG_PLATE , 
                        baseX+PLIST_BG_SCROLL_X_CHAR ,baseY ,
                        PLIST_PLATE_WIDTH , PLIST_PLATE_HEIGHT ,
                        &work->plateScrData->rawData , 0 , 19 , 32 , 32 ); 
  }

  GFL_BG_LoadScreenV_Req( PLIST_BG_PARAM );
  GFL_BG_LoadScreenV_Req( PLIST_BG_PLATE );
}

//--------------------------------------------------------------
//プレートをクリアする(入れ替えの移動前に行う
//--------------------------------------------------------------
void PLIST_PLATE_ClearPlate( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork , const u8 moveValue )
{
  const u8 baseX = PLIST_PLATE_POS_ARR[plateWork->idx][0];
  const u8 baseY = PLIST_PLATE_POS_ARR[plateWork->idx][1];
  
  const int value = (plateWork->idx%2==0 ? -moveValue : moveValue );

  GFL_BG_FillScreen( PLIST_BG_PLATE , 0 ,
              baseX+PLIST_BG_SCROLL_X_CHAR+value , baseY ,
              PLIST_PLATE_WIDTH , PLIST_PLATE_HEIGHT , PPC_CHANGE );
  GFL_BG_FillScreen( PLIST_BG_PARAM , 0 ,
              baseX+PLIST_BG_SCROLL_X_CHAR+value , baseY ,
              PLIST_PLATE_WIDTH , PLIST_PLATE_HEIGHT , PPC_CHANGE );
}

//--------------------------------------------------------------
//パラメータ再作成
//--------------------------------------------------------------
void PLIST_PLATE_ResetParam( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork , POKEMON_PARAM *pp , const u8 moveValue )
{
  const u8 baseX = PLIST_PLATE_POS_ARR[plateWork->idx][0];
  const u8 baseY = PLIST_PLATE_POS_ARR[plateWork->idx][1];
  
  const int value = (plateWork->idx%2==0 ? -moveValue : moveValue );

  plateWork->pp = pp;
  plateWork->isUpdateStr = FALSE;
  plateWork->isEgg =  PP_Get( plateWork->pp , ID_PARA_tamago_flag , NULL );
  plateWork->dispHp = PP_Get( plateWork->pp , ID_PARA_hp , NULL );
  plateWork->nowHp = plateWork->dispHp;

  GFL_CLACT_WK_Remove( plateWork->pokeIcon );
  GFL_CLGRP_CGR_Release( plateWork->pokeIconNcgRes );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( plateWork->bmpWin ) , 0 );
  GFL_CLACT_WK_SetDrawEnable( plateWork->itemIcon , FALSE );
  GFL_CLACT_WK_SetDrawEnable( plateWork->conditionIcon , FALSE );
  
  {
    //設定が終わったらレンダラを一回戻す
    GFL_CLACTPOS surfacePos;
    surfacePos.x = 0;
    surfacePos.y = 0;
    GFL_CLACT_USERREND_SetSurfacePos( plateWork->cellRender , PLIST_RENDER_MAIN , &surfacePos );
  }
  //セル作成
  PLIST_PLATE_CreatePokeIcon( work , plateWork );
  {
    //設定が終わったらレンダラを動かす
    GFL_CLACTPOS surfacePos;
    surfacePos.x = -(baseX+value)*8;
    surfacePos.y = -baseY*8;
    GFL_CLACT_USERREND_SetSurfacePos( plateWork->cellRender , PLIST_RENDER_MAIN , &surfacePos );
  }
  
  PLIST_PLATE_DrawParamMain( work , plateWork );
  
}
//--------------------------------------------------------------
//文字だけ再描画
//--------------------------------------------------------------
void PLIST_PLATE_ReDrawParam( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork )
{
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( plateWork->bmpWin ) , 0 );
  PLIST_PLATE_DrawParamMain( work , plateWork );
}

#pragma mark [>HPbar anime
//--------------------------------------------------------------
//	HPバーアニメ初期化
//--------------------------------------------------------------
void PLIST_PALTE_InitHpAnime( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork )
{
  plateWork->nowHp = PP_Get( plateWork->pp , ID_PARA_hp , NULL );
}

//--------------------------------------------------------------
//	HPバーアニメ更新
//--------------------------------------------------------------
const BOOL PLIST_PALTE_UpdateHpAnime( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork )
{
  if( plateWork->dispHp < plateWork->nowHp )
  {
    plateWork->dispHp++;

    if( plateWork->dispHp == 1 )
    {
      //ライフが増えるので色の変更
      PLIST_PLATE_ChangeColor( work , plateWork , PPC_NORMAL_SELECT );
    }
    PLIST_PLATE_ReDrawParam( work , plateWork );
    return FALSE;
  }
  else
  if( plateWork->dispHp > plateWork->nowHp )
  {
    plateWork->dispHp--;
    //瀕死は考慮しない
    PLIST_PLATE_ReDrawParam( work , plateWork );
    return FALSE;
  }
  else
  {
    return TRUE;
  }
}

#pragma mark [>outer func
//--------------------------------------------------------------
//選択できるか？
//--------------------------------------------------------------
const BOOL PLIST_PLATE_CanSelect( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork )
{
  if( plateWork->isBlank == TRUE )
  {
    return FALSE;
  }
  return TRUE;
}

//--------------------------------------------------------------
//プレートの位置取得(clact用
//--------------------------------------------------------------
void PLIST_PLATE_GetPlatePosition( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork , GFL_CLACTPOS *pos )
{
  pos->x = (PLIST_PLATE_POS_ARR[plateWork->idx][0] + (PLIST_PLATE_WIDTH/2))*8;
  pos->y = (PLIST_PLATE_POS_ARR[plateWork->idx][1] + (PLIST_PLATE_HEIGHT/2))*8;
}

//--------------------------------------------------------------
//プレートの位置取得(TP判定用
//--------------------------------------------------------------
void PLIST_PLATE_GetPlateRect( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork , GFL_UI_TP_HITTBL *hitTbl )
{
  hitTbl->rect.left   = PLIST_PLATE_POS_ARR[plateWork->idx][0]*8;
  hitTbl->rect.top    = PLIST_PLATE_POS_ARR[plateWork->idx][1]*8;
  hitTbl->rect.right  = (PLIST_PLATE_POS_ARR[plateWork->idx][0]+PLIST_PLATE_WIDTH )*8;
  hitTbl->rect.bottom = (PLIST_PLATE_POS_ARR[plateWork->idx][1]+PLIST_PLATE_HEIGHT)*8;
}

//--------------------------------------------------------------
//バトル参加用状態取得
//--------------------------------------------------------------
const PLIST_PLATE_BATTLE_ORDER PLIST_PLATE_GetBattleOrder( const PLIST_PLATE_WORK *plateWork )
{
  return plateWork->btlOrder;
}

//--------------------------------------------------------------
//バトル参加用状態設定
//--------------------------------------------------------------
void PLIST_PLATE_SetBattleOrder( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork , const PLIST_PLATE_BATTLE_ORDER order )
{
  if( plateWork->btlOrder != order )
  {
    if( order < PPBO_JOIN_OK )
    {
      plateWork->btlOrder = order;
    }
    else
    {
      PLIST_PLATE_CheckBattleOrder( work , plateWork );
    }
    PLIST_PLATE_ReDrawParam( work , plateWork );
  }
}

//--------------------------------------------------------------
//表示HP取得
//--------------------------------------------------------------
u16 PLIST_PLATE_GetDispHp( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork )
{
  return plateWork->dispHp;
}

//--------------------------------------------------------------
//タマゴチェック
//--------------------------------------------------------------
const BOOL PLIST_PLATE_IsEgg( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork )
{
  return plateWork->isEgg;
}

#pragma mark [>battle order
//--------------------------------------------------------------
//バトルに参加できるか？(初期参加番号が入っているか？
//--------------------------------------------------------------
static void PLIST_PLATE_CheckBattleOrder( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork )
{
  u8 i;
  if( PLIST_UTIL_IsBattleMenu(work) == FALSE )
  {
    //バトルじゃない！
    plateWork->btlOrder = PPBO_INVALLID;
    return;
  }
  
  //参加条件のチェック
  //レギュレーションもチェック
  //持ち物の被りは下でチェック
  if( PokeRegulationCheckPokePara( work->plData->reg , plateWork->pp ) == TRUE )
  {
    plateWork->btlOrder = PPBO_JOIN_OK;
  }
  else
  {
    plateWork->btlOrder = PPBO_JOIN_NG;
  }
}


const PLIST_PLATE_CAN_BATTLE PLIST_PLATE_CanJoinBattle( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork )
{

  if( plateWork->btlOrder != PPBO_JOIN_OK )
  {
    return PPCB_NG;
  }
  {
    u8 i;
    const u32 monsno = PP_Get( plateWork->pp , ID_PARA_monsno , NULL );
    const u32 item = PP_Get( plateWork->pp , ID_PARA_item , NULL );
    REGULATION *reg = (REGULATION*)work->plData->reg;
    //すでに同じポケがいるか？
    //すでに同じアイテムがあるかチェック
    for( i=0;i<6;i++ )
    {
      if( work->plData->in_num[i] >= 1 )
      {
        const POKEMON_PARAM *pp = work->plateWork[ work->plData->in_num[i]-1 ]->pp;
        const u32 monsno2 = PP_Get( pp , ID_PARA_monsno , NULL );
        const u32 item2 = PP_Get( pp , ID_PARA_item , NULL );
        
        if( monsno == monsno2 &&
            reg->BOTH_POKE == 1 )
        {
          return PPCB_NG_SAME_MONSNO;
        }
        if( item == item2 &&
            item != 0 &&
            reg->BOTH_ITEM == 1 )
        {
          return PPCB_NG_SAME_ITEM;
        }
      }
    }
  }
  return PPCB_OK;
}

#pragma mark [>util
//--------------------------------------------------------------
//HPの割合の計算0～100
//--------------------------------------------------------------
static const u8 PLIST_PLATE_GetHPRate( PLIST_PLATE_WORK *plateWork )
{
  const u32 hpmax = PP_Get( plateWork->pp , ID_PARA_hpmax , NULL );
  const u32 hp = plateWork->dispHp;
  u8 rate = 100*hp/hpmax;
  
  if( hp != 0 && rate == 0 )
  {
    rate = 1;
  }
  if( hp != hpmax && rate == 100 )
  {
    rate = 99;
  }
  return rate;
}

//--------------------------------------------------------------
//レンダラーに対応したセルの位置の計算
//--------------------------------------------------------------
static void PLIST_PLATE_CalcCellPos( PLIST_PLATE_WORK *plateWork , const s16 x , const s16 y , GFL_CLACTPOS *pos )
{
  GFL_CLACTPOS rendPos;
  GFL_CLACT_USERREND_GetSurfacePos( plateWork->cellRender , PLIST_RENDER_MAIN , &rendPos );
  
  pos->x = -rendPos.x + x;
  pos->y = -rendPos.y + y;
}

