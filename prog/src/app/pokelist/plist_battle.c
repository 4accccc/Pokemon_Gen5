//======================================================================
/**
 * @file	plist_battle.c
 * @brief	ポケモンリスト バトル表示系処理(相手パーティー・制限時間
 * @author	ariizumi
 * @data	09/10/15
 *
 * モジュール名：PLIST_BATTLE
 */
//======================================================================
#include <gflib.h>
#include "buflen.h"
#include "app/app_menu_common.h"
#include "pokeicon/pokeicon.h"
#include "system/main.h"
#include "system/gfl_use.h"

#include "arc_def.h"
#include "pokelist_gra.naix"
#include "msg/msg_pokelist.h"

#include "plist_sys.h"
#include "plist_message.h"
#include "plist_battle.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define
#define PLIST_BATTLE_ICON_LEFT  (0)
#define PLIST_BATTLE_ICON_TOP   (4)
#define PLIST_BATTLE_ICON_WIDTH  (5)
#define PLIST_BATTLE_ICON_HEIGHT (6)  //スペース込み

#define PLIST_BATTLE_TIME_STR_LEFT   ( 7)
#define PLIST_BATTLE_TIME_STR_TOP    (21)
#define PLIST_BATTLE_TIME_STR_WIDTH  (10)
#define PLIST_BATTLE_TIME_STR_HEIGHT ( 3)
#define PLIST_BATTLE_TIME_NUM_LEFT   (20)
#define PLIST_BATTLE_TIME_NUM_TOP    (21)
#define PLIST_BATTLE_TIME_NUM_WIDTH  ( 5)
#define PLIST_BATTLE_TIME_NUM_HEIGHT ( 3)

#define PLIST_BATTLE_PAL_ANM_MAX (60)

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
  BOOL isUpdateStr;
  GFL_BMPWIN  *sexStrWin;
  GFL_BMPWIN  *lvStrWin;
  u32       pokeIconRes;
  GFL_CLWK *pokeIcon;
  GFL_CLWK *itemIcon;
}PLIST_BATTLE_POKE_ICON;

struct _PLIST_BATTLE_PARAM_WORK
{
  
  BOOL isUpdateStr;
  GFL_BMPWIN  *nameStr;
  
  PLIST_BATTLE_POKE_ICON *icon[PLIST_LIST_MAX];
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void PLIST_BATTLE_InitCell( PLIST_WORK *work );
static void PLIST_BATTLE_TermCell( PLIST_WORK *work );
static void PLIST_BATTLE_InitResource( PLIST_WORK *work );
static void PLIST_BATTLE_TermResource( PLIST_WORK *work );

static PLIST_BATTLE_PARAM_WORK* PLIST_BATTLE_CreateBattleParam( PLIST_WORK *work , 
                        PL_COMM_BATTLE_PARAM *initParam , u8 charX, u8 charY , u8 space );
static void PLIST_BATTLE_DeleteBattleParam( PLIST_WORK *work , PLIST_BATTLE_PARAM_WORK *param );
static void PLIST_BATTLE_UpdateBattleParam( PLIST_WORK *work , PLIST_BATTLE_PARAM_WORK *param );

static PLIST_BATTLE_POKE_ICON* PLIST_BATTLE_CreateBattleParamIcon( PLIST_WORK *work , POKEMON_PARAM *pp , const u8 posX , const u8 posY );
static void PLIST_BATTLE_DeleteBattleParamIcon( PLIST_WORK *work , PLIST_BATTLE_POKE_ICON *iconWork );
static void PLIST_BATTLE_UpdateBattleParamIcon( PLIST_WORK *work , PLIST_BATTLE_POKE_ICON *iconWork );

static void PLIST_BATTLE_DispWaitingMessage( PLIST_WORK *work );

static void PLIST_BATTLE_InitTimeLimit( PLIST_WORK *work );
static void PLIST_BATTLE_TermTimeLimit( PLIST_WORK *work );
static void PLIST_BATTLE_UpdateTimeLimit( PLIST_WORK *work );

//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
void PLIST_BATTLE_InitBattle( PLIST_WORK *work )
{
  PLIST_BATTLE_InitCell( work );
  PLIST_BATTLE_InitResource( work );

  if( work->plData->is_disp_party == TRUE )
  {
    switch( work->plData->comm_type )
    {
    case PL_COMM_SINGLE:  //敵Aのみ
      work->battleParty[PL_COMM_PLAYER_TYPE_ENEMY_A] = PLIST_BATTLE_CreateBattleParam( work , 
                  &work->plData->comm_battle[PL_COMM_PLAYER_TYPE_ENEMY_A] , 10 , 0 , 2 );
      break;

    case PL_COMM_MULTI:   //味方・敵A・敵B
      work->battleParty[PL_COMM_PLAYER_TYPE_ALLY] = PLIST_BATTLE_CreateBattleParam( work , 
                  &work->plData->comm_battle[PL_COMM_PLAYER_TYPE_ALLY] , 0 , 0 , 0 );
      work->battleParty[PL_COMM_PLAYER_TYPE_ENEMY_A] = PLIST_BATTLE_CreateBattleParam( work , 
                  &work->plData->comm_battle[PL_COMM_PLAYER_TYPE_ENEMY_A] , 11 , 0 , 0 );
      work->battleParty[PL_COMM_PLAYER_TYPE_ENEMY_B] = PLIST_BATTLE_CreateBattleParam( work , 
                  &work->plData->comm_battle[PL_COMM_PLAYER_TYPE_ENEMY_B] , 22 , 0 , 0 );
      break;
    }
  }
  work->befSelectedNum = work->plData->comm_selected_num;
  //初回強制更新のため0xFFFFにしておく
  work->befTimeLimit = 0xFFFF;
  
  if(  work->plData->use_tile_limit == TRUE  )
  {
    PLIST_BATTLE_InitTimeLimit( work );
  }
  
  work->barPalletAnmCnt = 0;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
void PLIST_BATTLE_TermBattle( PLIST_WORK *work )
{
  if(  work->plData->use_tile_limit == TRUE  )
  {
    PLIST_BATTLE_TermTimeLimit( work );
  }
  if( work->plData->is_disp_party == TRUE )
  {
    switch( work->plData->comm_type )
    {
    case PL_COMM_SINGLE:  //敵Aのみ
      PLIST_BATTLE_DeleteBattleParam( work , work->battleParty[PL_COMM_PLAYER_TYPE_ENEMY_A] );
      break;

    case PL_COMM_MULTI:   //味方・敵A・敵B
      PLIST_BATTLE_DeleteBattleParam( work , work->battleParty[PL_COMM_PLAYER_TYPE_ALLY] );
      PLIST_BATTLE_DeleteBattleParam( work , work->battleParty[PL_COMM_PLAYER_TYPE_ENEMY_A] );
      PLIST_BATTLE_DeleteBattleParam( work , work->battleParty[PL_COMM_PLAYER_TYPE_ENEMY_B] );
      break;
    }
  }

  PLIST_BATTLE_TermResource( work );
  PLIST_BATTLE_TermCell( work );
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
void PLIST_BATTLE_UpdateBattle( PLIST_WORK *work )
{
  if( work->plData->is_disp_party == TRUE )
  {
    switch( work->plData->comm_type )
    {
    case PL_COMM_SINGLE:  //敵Aのみ
      PLIST_BATTLE_UpdateBattleParam( work , work->battleParty[PL_COMM_PLAYER_TYPE_ENEMY_A] );
      break;

    case PL_COMM_MULTI:   //味方・敵A・敵B
      PLIST_BATTLE_UpdateBattleParam( work , work->battleParty[PL_COMM_PLAYER_TYPE_ALLY] );
      PLIST_BATTLE_UpdateBattleParam( work , work->battleParty[PL_COMM_PLAYER_TYPE_ENEMY_A] );
      PLIST_BATTLE_UpdateBattleParam( work , work->battleParty[PL_COMM_PLAYER_TYPE_ENEMY_B] );
      break;
    }
  }
  
  //待機人数メッセージの更新
  if( work->befSelectedNum != work->plData->comm_selected_num &&
      work->mainSeq == PSMS_SELECT_POKE )
  {
    PLIST_BATTLE_DispWaitingMessage( work );
    work->befSelectedNum = work->plData->comm_selected_num;
  }
  if( work->plData->use_tile_limit == TRUE )
  {
    PLIST_BATTLE_UpdateTimeLimit( work );
  }
}

#pragma mark graphic
//--------------------------------------------------------------
//	cellの設定
//--------------------------------------------------------------
static void PLIST_BATTLE_InitCell( PLIST_WORK *work )
{
  const u32 cellNum = PL_COMM_PLAYER_TYPE_MAX * PLIST_LIST_MAX * 2;
  work->cellUnitBattle = GFL_CLACT_UNIT_Create( cellNum , PLIST_CELLUNIT_PRI_MAIN, work->heapId );
  
}

//--------------------------------------------------------------
//	cell開放
//--------------------------------------------------------------
static void PLIST_BATTLE_TermCell( PLIST_WORK *work )
{
  GFL_CLACT_UNIT_Delete( work->cellUnitBattle );
}

//--------------------------------------------------------------
//	リソース読み込み
//--------------------------------------------------------------
static void PLIST_BATTLE_InitResource( PLIST_WORK *work )
{
  ARCHANDLE *arcHandle = GFL_ARC_OpenDataHandle( ARCID_POKELIST , work->heapId );

  GFL_ARCHDL_UTIL_TransVramPaletteEx( arcHandle , NARC_pokelist_gra_list_battle_sub_NCLR , 
                    PALTYPE_SUB_BG , PLIST_BG_SUB_PLT_PLATE*16*2 , PLIST_BG_SUB_PLT_PLATE*16*2 , 
                    16*2 , work->heapId );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_pokelist_gra_list_battle_sub_NCGR ,
                    PLIST_BG_SUB_BATTLE_WIN , 0 , 0, FALSE , work->heapId );

  if( work->plData->is_disp_party == TRUE )
  {
    switch( work->plData->comm_type )
    {
    case PL_COMM_SINGLE:  //敵Aのみ
      GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_pokelist_gra_list_battle_sub_s_NSCR , 
                        PLIST_BG_SUB_BATTLE_WIN ,  0 , 0, FALSE , work->heapId );
      break;
    case PL_COMM_MULTI:   //味方・敵A・敵B
      GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_pokelist_gra_list_battle_sub_m_NSCR , 
                        PLIST_BG_SUB_BATTLE_WIN ,  0 , 0, FALSE , work->heapId );
      break;
    }
  }

  GFL_ARC_CloseDataHandle(arcHandle);
  
  //共通素材
  {
    ARCHANDLE *arcHandleCommon = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId() , work->heapId );

    //アイコン用アイテム
    work->cellRes[PCR_PLT_ITEM_ICON_SUB] = GFL_CLGRP_PLTT_Register( arcHandle , 
          APP_COMMON_GetPokeItemIconPltArcIdx() , CLSYS_DRAW_SUB , 
          PLIST_OBJPLT_ITEM_ICON*32 , work->heapId  );
    work->cellRes[PCR_NCG_ITEM_ICON_SUB] = GFL_CLGRP_CGR_Register( arcHandle , 
          APP_COMMON_GetPokeItemIconCharArcIdx() , FALSE , CLSYS_DRAW_SUB , work->heapId  );
    work->cellRes[PCR_ANM_ITEM_ICON_SUB] = GFL_CLGRP_CELLANIM_Register( arcHandle , 
          APP_COMMON_GetPokeItemIconCellArcIdx(APP_COMMON_MAPPING_128K) , 
          APP_COMMON_GetPokeItemIconAnimeArcIdx(APP_COMMON_MAPPING_128K), work->heapId  );
    
    //制限時間バー
    if( work->plData->use_tile_limit == TRUE )
    {
      //上画面バー  
      GFL_ARCHDL_UTIL_TransVramPalette( arcHandle , APP_COMMON_GetBarPltArcIdx() , 
                        PALTYPE_SUB_BG , PLIST_BG_SUB_PLT_BAR*32 , 32 , work->heapId );
      GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , APP_COMMON_GetBarCharArcIdx() ,
                    PLIST_BG_SUB_BATTLE_BAR , 0 , 0, FALSE , work->heapId );
      GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , APP_COMMON_GetBarScrnArcIdx() , 
                        PLIST_BG_SUB_BATTLE_BAR ,  0 , 0, FALSE , work->heapId );
      GFL_BG_ChangeScreenPalette( PLIST_BG_SUB_BATTLE_BAR , 0,0,32,32,PLIST_BG_SUB_PLT_BAR );
      GFL_BG_LoadScreenV_Req( PLIST_BG_SUB_BATTLE_BAR );
    }
    
    GFL_ARC_CloseDataHandle(arcHandleCommon);
  }
  //ポケアイコン用リソース
  //キャラクタは各自で
  {
    ARCHANDLE *arcHandlePoke = GFL_ARC_OpenDataHandle( ARCID_POKEICON , work->heapId );
    work->cellRes[PCR_PLT_POKEICON_SUB] = GFL_CLGRP_PLTT_RegisterComp( arcHandlePoke , 
          POKEICON_GetPalArcIndex() , CLSYS_DRAW_SUB , 
          PLIST_OBJPLT_POKEICON*32 , work->heapId  );
    
    GFL_ARC_CloseDataHandle(arcHandlePoke);
  }
  //アニメ用に、プレートの選択色のパレットを退避
  GFL_STD_MemCopy16( (void*)(HW_BG_PLTT+0x400+PLIST_BG_SUB_PLT_BAR*32) , work->barPalletAnm , 16*2 );
}

//--------------------------------------------------------------
//	リソース開放
//--------------------------------------------------------------
static void PLIST_BATTLE_TermResource( PLIST_WORK *work )
{
  //Cell系リソースは外でまとめて開放
}

#pragma mark [>battle param
//--------------------------------------------------------------
//	バトル用表示物(1パーティー分)
//--------------------------------------------------------------
static PLIST_BATTLE_PARAM_WORK* PLIST_BATTLE_CreateBattleParam( PLIST_WORK *work , 
                        PL_COMM_BATTLE_PARAM *initParam , u8 charX, u8 charY , u8 space )
{
  PLIST_BATTLE_PARAM_WORK* param = GFL_HEAP_AllocMemory( work->heapId , sizeof(PLIST_BATTLE_PARAM_WORK) );

  {
    u8 i;
    const u8 partyNum = PokeParty_GetPokeCount( initParam->pp );
    for( i=0;i<PLIST_LIST_MAX;i++ )
    {
      if( i<partyNum )
      {
        const u8 iconX = charX + (i%2) * (PLIST_BATTLE_ICON_WIDTH + space ) + PLIST_BATTLE_ICON_LEFT;
        const u8 iconY = charY + (i/2) * PLIST_BATTLE_ICON_HEIGHT + PLIST_BATTLE_ICON_TOP;
        //4ピクセルつめる
        const u8 ofsX = ( (i%2) == 0 ? 4 : 0 );
        POKEMON_PARAM *pp = PokeParty_GetMemberPointer( initParam->pp , i );
        param->icon[i] = PLIST_BATTLE_CreateBattleParamIcon( work , pp , iconX*8+ofsX , iconY*8 );
      }
      else
      {
        param->icon[i] = NULL;
      }
    }
  }
  //トレーナー名
  {
    STRBUF *srcStr;
    param->nameStr = GFL_BMPWIN_Create( PLIST_BG_SUB_BATTLE_STR , charX+1+(space/2) , charY + 1 , 
          10 , 2 , PLIST_BG_SUB_PLT_FONT , GFL_BMP_CHRAREA_GET_B );
    
    srcStr = GFL_STR_CreateBuffer( BUFLEN_PERSON_NAME , work->heapId ); 
    GFL_STR_SetStringCode( srcStr , initParam->name );
    PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( param->nameStr ) , 
            0 , 0 , srcStr , work->fontHandle , PLIST_FONT_BATTLE_PARAM );
    GFL_STR_DeleteBuffer( srcStr );
    param->isUpdateStr = TRUE;
  }
  return param;
}

//--------------------------------------------------------------
//	バトル用表示物(1パーティー分)
//--------------------------------------------------------------
static void PLIST_BATTLE_DeleteBattleParam( PLIST_WORK *work , PLIST_BATTLE_PARAM_WORK *param )
{
  u8 i;
  for( i=0;i<PLIST_LIST_MAX;i++ )
  {
    if( param->icon[i] != NULL )
    {
      PLIST_BATTLE_DeleteBattleParamIcon( work , param->icon[i] );
    }
  }
  GFL_BMPWIN_Delete( param->nameStr );
  GFL_HEAP_FreeMemory( param );
}


//--------------------------------------------------------------
//	バトル用表示物(1パーティー分)
//--------------------------------------------------------------
static void PLIST_BATTLE_UpdateBattleParam( PLIST_WORK *work , PLIST_BATTLE_PARAM_WORK *param )
{
  u8 i;
  for( i=0;i<PLIST_LIST_MAX;i++ )
  {
    if( param->icon[i] != NULL )
    {
      PLIST_BATTLE_UpdateBattleParamIcon( work , param->icon[i] );
    }
  }
  
  if( param->isUpdateStr == TRUE )
  {
    if( PRINTSYS_QUE_IsExistTarget( work->printQue , GFL_BMPWIN_GetBmp( param->nameStr )) == FALSE )
    {
      param->isUpdateStr = FALSE;
      GFL_BMPWIN_MakeTransWindow_VBlank( param->nameStr );
    }
    
  }
}

//--------------------------------------------------------------
//	アイコンの作成
//--------------------------------------------------------------
static PLIST_BATTLE_POKE_ICON* PLIST_BATTLE_CreateBattleParamIcon( PLIST_WORK *work , POKEMON_PARAM *pp , const u8 posX , const u8 posY )
{
  PLIST_BATTLE_POKE_ICON *iconWork = GFL_HEAP_AllocMemory( work->heapId , sizeof(PLIST_BATTLE_POKE_ICON) );
  //ポケアイコン
  {
    ARCHANDLE *arcHandle = GFL_ARC_OpenDataHandle( ARCID_POKEICON , work->heapId );
    const POKEMON_PASO_PARAM *ppp = PP_GetPPPPointerConst( pp );
    GFL_CLWK_DATA cellInitData;
    u8 pltNum;

    iconWork->pokeIconRes = GFL_CLGRP_CGR_Register( arcHandle , 
        POKEICON_GetCgxArcIndex(ppp) , FALSE , CLSYS_DRAW_SUB , work->heapId  );
    
    GFL_ARC_CloseDataHandle(arcHandle);

    pltNum = POKEICON_GetPalNumGetByPPP( ppp );
    cellInitData.pos_x = posX+16;
    cellInitData.pos_y = posY+8;
    cellInitData.anmseq = POKEICON_ANM_HPMAX;
    cellInitData.softpri = 16;
    cellInitData.bgpri = 2;
    iconWork->pokeIcon = GFL_CLACT_WK_Create( work->cellUnitBattle ,
              iconWork->pokeIconRes,
              work->cellRes[PCR_PLT_POKEICON_SUB],
              work->cellRes[PCR_ANM_POKEICON],
              &cellInitData ,CLSYS_DEFREND_SUB , work->heapId );
    GFL_CLACT_WK_SetPlttOffs( iconWork->pokeIcon , pltNum , CLWK_PLTTOFFS_MODE_PLTT_TOP );
    GFL_CLACT_WK_SetAutoAnmFlag( iconWork->pokeIcon , TRUE );
  }
  //アイテムアイコン
  {
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_x = posX+32;
    cellInitData.pos_y = posY+16;
    cellInitData.anmseq = 0;
    cellInitData.softpri = 8;
    cellInitData.bgpri = 2;
    iconWork->itemIcon = GFL_CLACT_WK_Create( work->cellUnitBattle ,
              work->cellRes[PCR_NCG_ITEM_ICON_SUB],
              work->cellRes[PCR_PLT_ITEM_ICON_SUB],
              work->cellRes[PCR_ANM_ITEM_ICON_SUB],
              &cellInitData ,CLSYS_DEFREND_SUB , work->heapId );
    if( PP_Get( pp , ID_PARA_item , NULL ) != 0 )
    {
      GFL_CLACT_WK_SetDrawEnable( iconWork->itemIcon , TRUE );
    }
    else
    {
      GFL_CLACT_WK_SetDrawEnable( iconWork->itemIcon , FALSE );
    }
  }
  //性別マーク
  {
    const u32 sex = PP_Get( pp , ID_PARA_sex , NULL );
    const u8 charX = posX/8;
    const u8 charY = posY/8;
    const u8 modX = posX%8;

    iconWork->sexStrWin = GFL_BMPWIN_Create( PLIST_BG_SUB_BATTLE_STR , charX + 3 , charY, 
          3 , 2 , PLIST_BG_SUB_PLT_FONT , GFL_BMP_CHRAREA_GET_B );
    if( sex == PTL_SEX_MALE )
    {
      PLIST_UTIL_DrawStrFunc( work , iconWork->sexStrWin , mes_pokelist_01_28 ,
                      modX+4 , 0 , PLIST_FONT_BATTLE_BLUE );
    }
    else if( sex == PTL_SEX_FEMALE )
    {
      PLIST_UTIL_DrawStrFunc( work , iconWork->sexStrWin , mes_pokelist_01_29 ,
                      modX+4 , 0 , PLIST_FONT_BATTLE_RED );
    }
  }
  //レベル
  {
    const u32 lv = PP_CalcLevel( pp );
    const u8 charX = posX/8;
    const u8 charY = posY/8;
    const u8 modX = posX%8;
    WORDSET *wordSet = WORDSET_Create( work->heapId );

    iconWork->lvStrWin = GFL_BMPWIN_Create( PLIST_BG_SUB_BATTLE_STR , charX , charY + 3 , 
          5 , 1 , PLIST_BG_SUB_PLT_FONT , GFL_BMP_CHRAREA_GET_B );

    WORDSET_RegisterNumber( wordSet , 0 , lv , 3 , STR_NUM_DISP_LEFT , STR_NUM_CODE_DEFAULT );

    PLIST_UTIL_DrawValueStrFuncSys( work , iconWork->lvStrWin , 
                      wordSet , mes_pokelist_01_03 , 
                      modX , 0 , PLIST_FONT_BATTLE_PARAM );

    WORDSET_Delete( wordSet );
  }
  iconWork->isUpdateStr = TRUE;
  
  return iconWork;
}

//--------------------------------------------------------------
//	アイコンの開放
//--------------------------------------------------------------
static void PLIST_BATTLE_DeleteBattleParamIcon( PLIST_WORK *work , PLIST_BATTLE_POKE_ICON *iconWork )
{
  GFL_BMPWIN_Delete( iconWork->sexStrWin );
  GFL_BMPWIN_Delete( iconWork->lvStrWin );
  GFL_CLACT_WK_Remove( iconWork->itemIcon );
  GFL_CLACT_WK_Remove( iconWork->pokeIcon );
  GFL_CLGRP_CGR_Release( iconWork->pokeIconRes );
  GFL_HEAP_FreeMemory( iconWork );
  
}

//--------------------------------------------------------------
//	アイコンの更新
//--------------------------------------------------------------
static void PLIST_BATTLE_UpdateBattleParamIcon( PLIST_WORK *work , PLIST_BATTLE_POKE_ICON *iconWork )
{
  if( iconWork->isUpdateStr == TRUE )
  {
    if( PRINTSYS_QUE_IsExistTarget( work->printQue , GFL_BMPWIN_GetBmp( iconWork->sexStrWin )) == FALSE &&
        PRINTSYS_QUE_IsExistTarget( work->printQue , GFL_BMPWIN_GetBmp( iconWork->lvStrWin )) == FALSE )
    {
      iconWork->isUpdateStr = FALSE;
      GFL_BMPWIN_MakeTransWindow_VBlank( iconWork->sexStrWin );
      GFL_BMPWIN_MakeTransWindow_VBlank( iconWork->lvStrWin );
    }
  }
}

#pragma mark [>waiting message
//--------------------------------------------------------------
//	待機メッセージの表示
//--------------------------------------------------------------
void PLIST_BATTLE_OpenWaitingMessage( PLIST_WORK *work )
{
  PLIST_BATTLE_DispWaitingMessage( work );

  work->befSelectedNum = work->plData->comm_selected_num;
}

static void PLIST_BATTLE_DispWaitingMessage( PLIST_WORK *work )
{
  if( work->plData->comm_selected_num == 0 )
  {
    //誰も決めてない
    //表示は無しでOK
  }
  else
  {
    if( PLIST_MSG_IsOpenWindow( work , work->msgWork ) == FALSE )
    {
      PLIST_MSG_OpenWindow( work , work->msgWork , PMT_BAR_BATTLE );
    }
    
    if( work->plData->comm_type == PL_COMM_SINGLE )
    {
      //○○は待機中
      STRBUF *srcStr;
      srcStr = GFL_STR_CreateBuffer( BUFLEN_PERSON_NAME , work->heapId ); 
      GFL_STR_SetStringCode( srcStr , work->plData->comm_battle[PL_COMM_PLAYER_TYPE_ENEMY_A].name );
      PLIST_MSG_CreateWordSet( work , work->msgWork );
      PLIST_MSG_AddWordSet_Word( work , work->msgWork , 0 , srcStr , 
                  work->plData->comm_battle[PL_COMM_PLAYER_TYPE_ENEMY_A].sex );

      PLIST_MSG_DrawMessageNoWait( work , work->msgWork , mes_pokelist_13_01 );

      PLIST_MSG_DeleteWordSet( work , work->msgWork );
      GFL_STR_DeleteBuffer( srcStr );
    }
    else
    {
      //××にん　たいきちゅう(アラビア数字ではないのでメッセージが別
      PLIST_MSG_DrawMessageNoWait( work , work->msgWork , mes_pokelist_13_02+(work->plData->comm_selected_num-1) );
    }
  }
}

#pragma mark [>time limit
//--------------------------------------------------------------
//	制限時間初期化
//--------------------------------------------------------------
static void PLIST_BATTLE_InitTimeLimit( PLIST_WORK *work )
{
  work->timeLimitStr = GFL_BMPWIN_Create( PLIST_BG_SUB_BATTLE_STR , 
              PLIST_BATTLE_TIME_STR_LEFT , PLIST_BATTLE_TIME_STR_TOP , 
              PLIST_BATTLE_TIME_STR_WIDTH , PLIST_BATTLE_TIME_STR_HEIGHT ,
              PLIST_BG_SUB_PLT_FONT , GFL_BMP_CHRAREA_GET_B );
  work->timeLimitNumStr = GFL_BMPWIN_Create( PLIST_BG_SUB_BATTLE_STR , 
              PLIST_BATTLE_TIME_NUM_LEFT , PLIST_BATTLE_TIME_NUM_TOP , 
              PLIST_BATTLE_TIME_NUM_WIDTH , PLIST_BATTLE_TIME_NUM_HEIGHT ,
              PLIST_BG_SUB_PLT_FONT , GFL_BMP_CHRAREA_GET_B );

  {
    STRBUF *str = GFL_MSG_CreateString( work->msgHandle , mes_pokelist_13_05 ); 

    PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( work->timeLimitStr ) , 
            0 , 4 , str , work->fontHandle , PLIST_FONT_BATTLE_PARAM );

    GFL_STR_DeleteBuffer( str );
    work->isUpdateLimitStr = TRUE;
  }

}

//--------------------------------------------------------------
//	制限時間開放
//--------------------------------------------------------------
static void PLIST_BATTLE_TermTimeLimit( PLIST_WORK *work )
{
  GFL_BMPWIN_Delete( work->timeLimitStr );
  GFL_BMPWIN_Delete( work->timeLimitNumStr );
}

//--------------------------------------------------------------
//	制限時間更新
//--------------------------------------------------------------
static void PLIST_BATTLE_UpdateTimeLimit( PLIST_WORK *work )
{
  if( work->befTimeLimit != work->plData->time_limit )
  {
    const u8 min = work->plData->time_limit/60;
    const u8 sec = work->plData->time_limit%60;
    STRBUF *str = GFL_MSG_CreateString( work->msgHandle , mes_pokelist_13_06 ); 
    STRBUF *workStr = GFL_STR_CreateBuffer( 32 , work->heapId );
    WORDSET *wordSet = WORDSET_Create( work->heapId );

    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->timeLimitNumStr ) , 0 );

    WORDSET_RegisterNumber( wordSet , 0 , min , 2 , STR_NUM_DISP_SPACE , STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterNumber( wordSet , 1 , sec , 2 , STR_NUM_DISP_ZERO , STR_NUM_CODE_DEFAULT );
    WORDSET_ExpandStr( wordSet , workStr , str );

    PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( work->timeLimitNumStr ) , 
            0 , 4 , workStr , work->fontHandle , PLIST_FONT_BATTLE_PARAM );

    WORDSET_Delete( wordSet );
    GFL_STR_DeleteBuffer( workStr );
    GFL_STR_DeleteBuffer( str );
    work->isUpdateLimitNum = TRUE;

    work->befTimeLimit = work->plData->time_limit;
  }
  if( work->plData->time_limit == 0 )
  {
    PLIST_ForceExit_Timeup( work );
  }
  
  if( work->isUpdateLimitNum == TRUE )
  {
    if( PRINTSYS_QUE_IsExistTarget( work->printQue , GFL_BMPWIN_GetBmp( work->timeLimitNumStr )) == FALSE )
    {
      work->isUpdateLimitNum = FALSE;
      GFL_BMPWIN_MakeTransWindow_VBlank( work->timeLimitNumStr );
    }
  }

  if( work->isUpdateLimitStr == TRUE )
  {
    if( PRINTSYS_QUE_IsExistTarget( work->printQue , GFL_BMPWIN_GetBmp( work->timeLimitStr )) == FALSE )
    {
      work->isUpdateLimitStr = FALSE;
      GFL_BMPWIN_MakeTransWindow_VBlank( work->timeLimitStr );
    }
  }
  
  if( work->plData->time_limit <= 10 )
  {
    //パレットアニメ
    u8 i;
    const fx32 cos = FX_CosIdx( work->barPalletAnmCnt*0x10000/PLIST_BATTLE_PAL_ANM_MAX );
    const u8 addVal = (cos+FX32_ONE) * 32 / (FX32_ONE*2);
    
    for( i=0;i<16;i++ )
    {
      u8 r = (work->barPalletAnm[i]&GX_RGB_R_MASK)>>GX_RGB_R_SHIFT;
      u8 g = (work->barPalletAnm[i]&GX_RGB_G_MASK)>>GX_RGB_G_SHIFT;
      u8 b = (work->barPalletAnm[i]&GX_RGB_B_MASK)>>GX_RGB_B_SHIFT;
      r = (r+addVal > 31?31:r+addVal);
      work->barPalletTrans[i] = GX_RGB(r,g,b);
    }
    NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG_PLTT_SUB ,
                                        PLIST_BG_SUB_PLT_BAR * 32 ,
                                        work->barPalletTrans , 2*16 );
    

    work->barPalletAnmCnt++;
    if( work->barPalletAnmCnt >= PLIST_BATTLE_PAL_ANM_MAX )
    {
      work->barPalletAnmCnt = 0;
    }
  }
}

