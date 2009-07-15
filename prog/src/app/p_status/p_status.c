//======================================================================
/**
 * @file	p_status.c
 * @brief	ポケモンステータス
 * @author	ariizumi
 * @data	09/07/01
 *
 * モジュール名：POKE_STATUS
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "p_sta_local_def.h"
#include "p_sta_sys.h"

//デバッグポケパーティ作成用
#include  "poke_tool/pokeparty.h"
#include  "poke_tool/poke_tool.h"
#include  "poke_tool/monsno_def.h"

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
static GFL_PROC_RESULT PokeStatusProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT PokeStatusProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT PokeStatusProc_Term( GFL_PROC * proc, int * seq , void *pwk, void *mywk );

GFL_PROC_DATA PokeStatus_ProcData =
{
  PokeStatusProc_Init,
  PokeStatusProc_Main,
  PokeStatusProc_Term
};

//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
static GFL_PROC_RESULT PokeStatusProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  PSTATUS_DATA *psData = pwk;
  PSTATUS_WORK *psWork;
  
  switch( *seq )
  {
  case 0:
    GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_POKE_STATUS, 0x30000 );
    
    psWork = GFL_PROC_AllocWork( proc, sizeof(PSTATUS_WORK), HEAPID_POKE_STATUS );
    psWork->heapId = HEAPID_POKE_STATUS;

    DEBUG_PerformanceSetActive( FALSE );
    //デバグ
    if( pwk == NULL )
    {
      u8 i;
      POKEPARTY *pokeParty;
      psData = GFL_HEAP_AllocMemory( HEAPID_POKE_STATUS , sizeof(PSTATUS_DATA) );
      pokeParty = PokeParty_AllocPartyWork(HEAPID_POKE_STATUS);
      PokeParty_Init( pokeParty , 6 );
      for( i=0;i<5;i++ )
      {
        POKEMON_PARAM *pPara = PP_Create( i+3 , 10 , PTL_SETUP_POW_AUTO , HEAPID_POKE_STATUS );
#if DEB_ARI
        switch( i )
        {
        case 1:
          PP_Put( pPara , ID_PARA_hp , 20 );
          break;
        case 2:
          PP_Put( pPara , ID_PARA_hp , 10 );
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
        {
          u16 oyaName[5] = {L'ブ',L'ラ',L'ッ',L'ク',0xFFFF};
          PP_Put( pPara , ID_PARA_oyaname_raw , (u32)&oyaName[0] );
          PP_Put( pPara , ID_PARA_oyasex , PTL_SEX_MALE );
        }
        PokeParty_Add( pokeParty , pPara );
        GFL_HEAP_FreeMemory( pPara );
      }
      
      psData->ppd = pokeParty;
#if PM_DEBUG
      psData->ppt = PST_PP_TYPE_DEBUG;
      psData->max = 255;  //u8なので255が最大
#else
      psData->ppt = PST_PP_TYPE_POKEPARTY;
      psData->max = 5;
#endif
      psData->mode = PST_MODE_NORMAL;
      psData->pos = 0;
      
      GFL_UI_SetTouchOrKey( GFL_APP_KTST_TOUCH );
    }
    
    psWork->psData = psData;
    *seq = 1;
    break;
  case 1:
    psWork = mywk;
    if( PSTATUS_InitPokeStatus( psWork ) == TRUE )
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
static GFL_PROC_RESULT PokeStatusProc_Term( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  PSTATUS_WORK *psWork = mywk;

  if( PSTATUS_TermPokeStatus( psWork ) == FALSE )
  {
    return GFL_PROC_RES_CONTINUE;
  }

  //デバグ
  if( pwk == NULL )
  {
    GFL_HEAP_FreeMemory( psWork->psData->ppd );
    GFL_HEAP_FreeMemory( psWork->psData );
  }

  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_POKE_STATUS );

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
static GFL_PROC_RESULT PokeStatusProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  PSTATUS_WORK *psWork = mywk;
  const PSTATUS_RETURN_TYPE ret = PSTATUS_UpdatePokeStatus( psWork );

  if( ret == SRT_RETURN )
  {
    return GFL_PROC_RES_FINISH;
  }
  else
  if( ret == SRT_EXIT )
  {
    return GFL_PROC_RES_FINISH;
  }
  return GFL_PROC_RES_CONTINUE;
}


