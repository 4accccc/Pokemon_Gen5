//============================================================================================
/**
 * @file  pdc.c
 * @brief ポケモンドリームキャッチ（パレスでの捕獲ゲーム）
 * @author  soga
 * @date  2010.03.12
 */
//============================================================================================
#include <gflib.h>
#include <procsys.h>

#include "system/main.h"

#include "app/pdc.h"
#include "pdc_main.h"

//============================================================================================
/**
 *  定数宣言
 */
//============================================================================================
#define POKEMON_DREAM_CATCH_HEAP_SIZE ( 0xc0000 )

//============================================================================================
/**
 *  構造体宣言
 */
//============================================================================================

struct _PDC_SETUP_PARAM
{ 
  POKEMON_PARAM*        pp;
  BTL_FIELD_SITUATION*  bfs;
  MYSTATUS*             my_status;
  MYITEM*               my_item;
  CONFIG*               config;
  ZUKAN_SAVEDATA*       zs;
  PDC_RESULT            result;
};

typedef struct
{ 
  PDC_MAIN_WORK*    pdc_main;
  PDC_SETUP_PARAM*  psp;

  HEAPID            heapID;
}PDC_WORK;

//============================================================================================
/**
 *  プロトタイプ宣言
 */
//============================================================================================
static GFL_PROC_RESULT PDC_ProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT PDC_ProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT PDC_ProcExit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

FS_EXTERN_OVERLAY(battle_view);

//--------------------------------------------------------------------------
/**
 * @brief PDC_SETUP_PARAM生成
 *
 * @param[in] pp        POKEMON_PARAM構造体
 * @param[in] bfs       BTL_FIELD_SITUATION構造体
 * @param[in] my_status プレイヤーデータ
 * @param[in] my_item   持ちアイテムデータ
 * @param[in] config    コンフィグ
 * @param[in] zs        図鑑セーブデータ
 * @param[in] heapID    ヒープID
 *
 * @retval  PDC_SETUP_PARAM
 */
//--------------------------------------------------------------------------
PDC_SETUP_PARAM*  PDC_MakeSetUpParam( POKEMON_PARAM* pp, BTL_FIELD_SITUATION* bfs, MYSTATUS* my_status, MYITEM* my_item, CONFIG* config, ZUKAN_SAVEDATA* zs, HEAPID heapID )
{ 
  PDC_SETUP_PARAM*  psp = GFL_HEAP_AllocMemory( heapID, sizeof( PDC_SETUP_PARAM ) );

  psp->pp         = pp;
  psp->bfs        = bfs;
  psp->my_status  = my_status;
  psp->my_item    = my_item;
  psp->config     = config;
  psp->zs         = zs;
  psp->result     = PDC_RESULT_NONE;

  return psp;
}

//--------------------------------------------------------------------------
/**
 * @brief pp取得
 *
 * @param[in] psp PDC_SETUP_PARAM
 *
 * @retval  POKEMON_PARAM
 */
//--------------------------------------------------------------------------
POKEMON_PARAM*  PDC_GetPP( PDC_SETUP_PARAM* psp )
{ 
  return psp->pp;
}

//--------------------------------------------------------------------------
/**
 * @brief bfs取得
 *
 * @param[in] psp PDC_SETUP_PARAM
 *
 * @retval  bfs
 */
//--------------------------------------------------------------------------
BTL_FIELD_SITUATION*  PDC_GetBFS( PDC_SETUP_PARAM* psp )
{ 
  return psp->bfs;
}

//--------------------------------------------------------------------------
/**
 * @brief my_status取得
 *
 * @param[in] psp PDC_SETUP_PARAM
 *
 * @retval  my_status
 */
//--------------------------------------------------------------------------
MYSTATUS*  PDC_GetMyStatus( PDC_SETUP_PARAM* psp )
{ 
  return psp->my_status;
}

//--------------------------------------------------------------------------
/**
 * @brief my_item取得
 *
 * @param[in] psp PDC_SETUP_PARAM
 *
 * @retval  my_item
 */
//--------------------------------------------------------------------------
MYITEM*  PDC_GetMyItem( PDC_SETUP_PARAM* psp )
{ 
  return psp->my_item;
}

//--------------------------------------------------------------------------
/**
 * @brief config取得
 *
 * @param[in] psp PDC_SETUP_PARAM
 *
 * @retval  config
 */
//--------------------------------------------------------------------------
CONFIG*  PDC_GetConfig( PDC_SETUP_PARAM* psp )
{ 
  return psp->config;
}

//--------------------------------------------------------------------------
/**
 * @brief zs取得
 *
 * @param[in] psp PDC_SETUP_PARAM
 *
 * @retval  zs
 */
//--------------------------------------------------------------------------
ZUKAN_SAVEDATA*  PDC_GetZukanWork( PDC_SETUP_PARAM* psp )
{ 
  return psp->zs;
}

//--------------------------------------------------------------------------
/**
 * @brief PDC_RESULT取得
 *
 * @param[in] psp PDC_SETUP_PARAM
 *
 * @retval  PDC_RESULT
 */
//--------------------------------------------------------------------------
PDC_RESULT  PDC_GetResult( PDC_SETUP_PARAM* psp )
{ 
  return psp->result;
}

//--------------------------------------------------------------------------
/**
 * PROC Init
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT PDC_ProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  PDC_SETUP_PARAM*  psp = ( PDC_SETUP_PARAM* )pwk;
  PDC_WORK* wk;

  GFL_OVERLAY_Load(FS_OVERLAY_ID(battle_view));

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_POKEMON_DREAM_CATCH, POKEMON_DREAM_CATCH_HEAP_SIZE );
  wk = GFL_PROC_AllocWork( proc, sizeof( PDC_WORK ), HEAPID_POKEMON_DREAM_CATCH );
  MI_CpuClearFast( wk, sizeof( PDC_WORK ) );
  wk->heapID = HEAPID_POKEMON_DREAM_CATCH;

  wk->pdc_main = PDC_MAIN_Init( psp, wk->heapID );
  wk->psp = psp;

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT PDC_ProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  PDC_WORK* wk = ( PDC_WORK* )mywk;

  if( ( wk->psp->result = PDC_MAIN_Main( wk->pdc_main ) ) != PDC_RESULT_NONE )
  { 
    return GFL_PROC_RES_FINISH;
  }
  else
  {
    return GFL_PROC_RES_CONTINUE;
  }
}
//--------------------------------------------------------------------------
/**
 * PROC Exit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT PDC_ProcExit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  PDC_WORK* wk = ( PDC_WORK* )mywk;

  PDC_MAIN_Exit( wk->pdc_main );

  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_POKEMON_DREAM_CATCH );

  GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle_view ) );

  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA   PDC_ProcData = {
  PDC_ProcInit,
  PDC_ProcMain,
  PDC_ProcExit,
};

