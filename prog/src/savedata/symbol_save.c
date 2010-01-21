//==============================================================================
/**
 * @file    symbol_save.c
 * @brief   シンボルエンカウント用セーブデータ
 * @author  matsuda
 * @date    2010.01.06(水)
 */
//==============================================================================
#include <gflib.h>
#include "savedata/save_control.h"
#include "savedata/save_tbl.h"
#include "savedata/symbol_save.h"


//==============================================================================
//  定数定義
//==============================================================================
///シンボルポケモン最大数
#define SYMBOL_POKE_MAX           (200)


//==============================================================================
//  構造体定義
//==============================================================================
///シンボルエンカウントセーブデータ
struct _SYMBOL_SAVE_WORK{
  u16 monsno[SYMBOL_POKE_MAX];    ///<シンボルポケモン
};




//------------------------------------------------------------------
/**
 * セーブデータサイズを返す
 *
 * @retval  int		
 */
//------------------------------------------------------------------
u32 SymbolSave_GetWorkSize( void )
{
	return sizeof(SYMBOL_SAVE_WORK);
}

//--------------------------------------------------------------
/**
 * @brief   ワーク初期化
 *
 * @param   rec		
 */
//--------------------------------------------------------------
void SymbolSave_WorkInit(void *work)
{
  SYMBOL_SAVE_WORK *symbol_save = work;
  
  GFL_STD_MemClear(symbol_save, sizeof(SYMBOL_SAVE_WORK));
}


//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * ポケモン登録
 *
 * @param   symbol_save		シンボルセーブ領域へのポインタ
 * @param   monsno        ポケモン番号
 */
//==================================================================
void SymbolSave_Set(SYMBOL_SAVE_WORK *symbol_save, u16 monsno)
{
  int i;
  
  for(i = 0; i < SYMBOL_POKE_MAX; i++){
    if(symbol_save->monsno[i] == 0){
      symbol_save->monsno[i] = monsno;
      return;
    }
  }
}

//==================================================================
/**
 * @brief   SYMBOL_SAVE_WORKデータ取得
 * @param   pSave		セーブデータポインタ
 * @return  SYMBOL_SAVE_WORKデータ
 */
//==================================================================

SYMBOL_SAVE_WORK* SymbolSave_GetSymbolData(SAVE_CONTROL_WORK* pSave)
{
	SYMBOL_SAVE_WORK* pData;
	pData = SaveControl_DataPtrGet(pSave, GMDATA_ID_SYMBOL);
	return pData;
}
