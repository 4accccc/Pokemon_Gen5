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
///スタックポケモン数
#define SYMBOL_STACK_MAX          (10)
///シンボルポケモン最大数
#define SYMBOL_POKE_MAX           (520 + SYMBOL_STACK_MAX)


//==============================================================================
//  構造体定義
//==============================================================================
///シンボルエンカウント1匹分のセーブ
struct _SYMBOL_POKEMON{
  u32 monsno:11;        ///<ポケモン番号
  u32 wazano:10;        ///<技番号
  u32 sex:2;            ///<性別(PTL_SEX_MALE, PTL_SEX_FEMALE, PTL_SEX_UNKNOWN)
  u32 form_no:6;        ///<フォルム番号
  u32        :3;        //余り
};

///シンボルエンカウントセーブデータ
struct _SYMBOL_SAVE_WORK{
  struct _SYMBOL_POKEMON symbol_poke[SYMBOL_POKE_MAX]; ///<シンボルポケモン
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
  OS_TPrintf("SYMBOL_SAVE SIZE = %d\n", sizeof(SYMBOL_SAVE_WORK));
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

//==================================================================
/**
 * シンボルポケモン登録
 *
 * @param   symbol_save		シンボルセーブ領域へのポインタ
 * @param   monsno        ポケモン番号
 * @param   wazano		    技番号
 * @param   sex		        性別(PTL_SEX_MALE, PTL_SEX_FEMALE, PTL_SEX_UNKNOWN)
 * @param   form_no		    フォルム番号
 */
//==================================================================
void SymbolSave_Set(SYMBOL_SAVE_WORK *symbol_save, u16 monsno, u16 wazano, u8 sex, u8 form_no)
{
  int i;
  SYMBOL_POKEMON *spoke = symbol_save->symbol_poke;
  
  for(i = 0; i < SYMBOL_POKE_MAX; i++){
    if(spoke->monsno == 0){
      spoke->monsno = monsno;
      spoke->wazano = wazano;
      spoke->sex = sex;
      spoke->form_no = form_no;
      return;
    }
    spoke++;
  }
}

