//======================================================================
/**
 * @file	pokelist.c
 * @brief	ポケモンリスト
 * @author	ariizumi
 * @data	09/06/10
 *
 * モジュール名：POKE_LIST
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "plist_local_def.h"
#include "plist_sys.h"

//デバッグポケパーティ作成用
#include  "poke_tool/pokeparty.h"
#include  "poke_tool/poke_tool.h"
#include  "poke_tool/poke_regulation.h"
#include  "waza_tool/wazano_def.h"  //デバッグポケ生成用

//デバッグメータ消し
#include "test/performance.h"
#include "test/ariizumi/ari_debug.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static GFL_PROC_RESULT PokeListProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT PokeListProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT PokeListProc_Term( GFL_PROC * proc, int * seq , void *pwk, void *mywk );

FS_EXTERN_OVERLAY(pokelist);

GFL_PROC_DATA PokeList_ProcData =
{
  PokeListProc_Init,
  PokeListProc_Main,
  PokeListProc_Term
};

//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
static GFL_PROC_RESULT PokeListProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  PLIST_DATA *plData = pwk;
  PLIST_WORK *plWork = mywk;;

  switch( *seq )
  {
  case 0:
    
    GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_POKELIST, 0x30000 );
    
    plWork = GFL_PROC_AllocWork( proc, sizeof(PLIST_WORK), HEAPID_POKELIST );
    plWork->heapId = HEAPID_POKELIST;

    DEBUG_PerformanceSetActive( FALSE );
    //デバグ
    if( pwk == NULL )
    {
      u8 i;
      plData = GFL_HEAP_AllocClearMemory( HEAPID_POKELIST , sizeof(PLIST_DATA) );
      plData->pp = PokeParty_AllocPartyWork(HEAPID_POKELIST);
      plData->ret_sel = PL_SEL_POS_POKE1;
      PokeParty_Init( plData->pp , 6 );
      for( i=0;i<5;i++ )
      {
        //*
        static const u8 no[5] = {33,135,32,2,3};
        static const u8 lv[5] = {34,10,15,10,10};
        POKEMON_PARAM *pPara = PP_Create( no[i] , lv[i] , PTL_SETUP_POW_AUTO , HEAPID_POKELIST );
  #if DEB_ARI
        switch( i )
        {
        case 1:
          //PP_Put( pPara , ID_PARA_hp , 20 );
          PP_Put( pPara , ID_PARA_pp1 , 3 );
          PP_Put( pPara , ID_PARA_pp2 , 3 );
          PP_Put( pPara , ID_PARA_waza1 , WAZANO_TAMAGOUMI );
          break;
        case 2:
          PP_Put( pPara , ID_PARA_hp , 30 );
          PP_SetSick( pPara , POKESICK_DOKU );
          break;
        case 3:
          PP_Put( pPara , ID_PARA_hp , 0 );
          PP_Put( pPara , ID_PARA_item , 1 );
          break;
        case 4:
          PP_Put( pPara , ID_PARA_hp , 1 );
          PP_Put( pPara , ID_PARA_item , 1 );
          break;
        }
  #endif
        /*/
        //バトル用テスト
        u8 noArr[6] = {1,1,2,3,4,5};
        POKEMON_PARAM *pPara = PP_Create( noArr[i] , 10 , PTL_SETUP_POW_AUTO , HEAPID_POKELIST );
        switch( i )
        {
        case 0:
        case 2:
          PP_Put( pPara , ID_PARA_item , 1 );
          break;
        }
        
        //*/
        {
          u16 oyaName[5] = {L'ブ',L'ラ',L'ッ',L'ク',0xFFFF};
          PP_Put( pPara , ID_PARA_oyaname_raw , (u32)&oyaName[0] );
          PP_Put( pPara , ID_PARA_oyasex , PTL_SEX_MALE );
        }
        PokeParty_Add( plData->pp , pPara );
        GFL_HEAP_FreeMemory( pPara );
      }
      
      plData->mode = PL_MODE_FIELD;

      if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
      {
        u8 i,p;
        //レギュレーション作成
        plData->reg = (void*)PokeRegulation_LoadDataAlloc( REG_LV50_SINGLE , HEAPID_POKELIST );
        for( i=0;i<6;i++ )
        {
          plData->in_num[i] = 0;
        }
        plData->mode = PL_MODE_BATTLE;
        for( p=0;p<3;p++ )
        {
          plData->comm_battle[p].pp = PokeParty_AllocPartyWork(HEAPID_POKELIST);
          for( i=0;i<6;i++ )
          {
            POKEMON_PARAM *pPara = PP_Create( GFUser_GetPublicRand0(250) , 100 , PTL_SETUP_POW_AUTO , HEAPID_POKELIST );
            u16 oyaName[5] = {L'ブ',L'ラ',L'ッ',L'ク',0xFFFF};
            PP_Put( pPara , ID_PARA_oyaname_raw , (u32)&oyaName[0] );
            PP_Put( pPara , ID_PARA_oyasex , PTL_SEX_MALE );
            PP_Put( pPara , ID_PARA_item , GFUser_GetPublicRand0(2) );
            PokeParty_Add( plData->comm_battle[p].pp , pPara );
            GFL_HEAP_FreeMemory( pPara );
          }
          {
            u16 *name = GFL_HEAP_AllocMemory( HEAPID_POKELIST , 12 );
            name[0] = L'ぷ';
            name[1] = L'れ';
            name[2] = L'い';
            name[3] = L'や';
            name[4] = L'１'+p;
            name[5] = 0xFFFF;
            plData->comm_battle[p].name = name;
          }
          plData->comm_battle[p].sex = GFUser_GetPublicRand0(2);
        }
        //plData->comm_type = PL_COMM_MULTI;
        plData->comm_type = PL_COMM_SINGLE;
        plData->is_disp_party = TRUE;
        plData->use_tile_limit = TRUE;
        plData->time_limit = 30;
        plData->comm_selected_num = 1;
      }
      else
      if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X )
      {
        //plData->mode = PL_MODE_SHINKA;
        plData->mode = PL_MODE_ITEMUSE;
        //plData->item = 17; //傷薬
        //plData->item = 26; //E傷薬
        //plData->item = 28; //元気のかけら
        //plData->item = 18; //毒消し
        //plData->item = 47; //ブロムヘキシン
        plData->item = 38; //PPエイド
        //plData->item = 41; //PPマックス
        //plData->item = 51; //Pアップ
        
        //plData->item = 44; //聖なる灰
        //plData->item = 174; //マトマの実
        //plData->item = 81; //月の石
        //plData->item = 50; //あめ
      }
      else
      if( GFL_UI_KEY_GetCont() & PAD_BUTTON_Y )
      {
        //plData->mode = PL_MODE_SODATEYA;
        
        plData->mode = PL_MODE_WAZASET;
        plData->item = 328; //技マシン気合パンチ
        plData->waza = 0;
        plData->wazaLearnBit = 0x3E;
        
      }
      GFL_UI_SetTouchOrKey( GFL_APP_KTST_TOUCH );
    }
    
    plWork->plData = plData;
    (*seq) = 1;
    
    //break through
  case 1:
    if( PLIST_InitPokeList( plWork ) == TRUE )
    {
      return GFL_PROC_RES_FINISH;
    }
    break;
  }
  return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
static GFL_PROC_RESULT PokeListProc_Term( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  PLIST_WORK *plWork = mywk;

  if( PLIST_TermPokeList( plWork ) == FALSE )
  {
    return GFL_PROC_RES_CONTINUE;
  }

  //デバグ
  if( pwk == NULL )
  {
    u8 i;
    for( i=0;i<6;i++ )
    {
      ARI_TPrintf("%d[%d]\n",i,plWork->plData->in_num[i]);
    }
    for( i=0;i<3;i++ )
    {
      if( plWork->plData->comm_battle[i].pp )
      {
        GFL_HEAP_FreeMemory( plWork->plData->comm_battle[i].pp );
      }
      if( plWork->plData->comm_battle[i].name )
      {
        GFL_HEAP_FreeMemory( (void*)plWork->plData->comm_battle[i].name );
      }
    }
    if( plWork->plData->reg )
    {
      GFL_HEAP_FreeMemory( plWork->plData->reg );
    }
    GFL_HEAP_FreeMemory( plWork->plData->pp );
    GFL_HEAP_FreeMemory( plWork->plData );
  }

  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_POKELIST );
  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
static GFL_PROC_RESULT PokeListProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  PLIST_WORK *plWork = mywk;
  
  if( pwk == NULL &&
      plWork->plData->use_tile_limit == TRUE)
  {
    if( plWork->plData->time_limit > 0 )
    {
      static u8 cnt = 0;
      cnt++;
      if( cnt > 60 )
      {
        cnt = 0;
        OS_TPrintf("[%d]\n",plWork->plData->time_limit);
        plWork->plData->time_limit--;
      }
    }
  }
  if( PLIST_UpdatePokeList( plWork ) == TRUE )
  {
    return GFL_PROC_RES_FINISH;
  }

  return GFL_PROC_RES_CONTINUE;
}


