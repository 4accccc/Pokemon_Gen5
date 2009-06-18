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
  PLIST_WORK *plWork;
  
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_POKELIST, 0x30000 );
  
  plWork = GFL_PROC_AllocWork( proc, sizeof(PLIST_WORK), HEAPID_POKELIST );
  plWork->heapId = HEAPID_POKELIST;
  
  //デバグ
  if( pwk == NULL )
  {
    u8 i;
    plData = GFL_HEAP_AllocMemory( HEAPID_POKELIST , sizeof(PLIST_DATA) );
    plData->pp = PokeParty_AllocPartyWork(HEAPID_POKELIST);
    plData->ret_sel = PL_SEL_POS_POKE1;
    PokeParty_Init( plData->pp , 6 );
    for( i=0;i<5;i++ )
    {
      POKEMON_PARAM *pPara = PP_Create( i+1 , 10 , PTL_SETUP_POW_AUTO , HEAPID_POKELIST );
      PokeParty_Add( plData->pp , pPara );
      GFL_HEAP_FreeMemory( pPara );
    }
    
    plData->mode = PL_MODE_FIELD;
    
    GFL_UI_SetTouchOrKey( GFL_APP_KTST_TOUCH );
  }
  
  
  plWork->plData = plData;

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
static GFL_PROC_RESULT PokeListProc_Term( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  PLIST_WORK *plWork = mywk;
  //デバグ
  if( pwk == NULL )
  {
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
  
  switch( *seq )
  {
  case 0:
    if( PLIST_InitPokeList( plWork ) == TRUE )
    {
      (*seq) = 1;
    }
    break;

  case 1:
    if( PLIST_UpdatePokeList( plWork ) == TRUE )
    {
      (*seq) = 2;
    }
    break;

  case 2:
    if( PLIST_TermPokeList( plWork ) == TRUE )
    {
      return GFL_PROC_RES_FINISH;
    }
    break;
  }
  
  return GFL_PROC_RES_CONTINUE;
}


