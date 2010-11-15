//============================================================================================
/**
 * @file    itemuse.c
 * @brief   フィールドアイテム使用処理
 * @author  Hiroyuki Nakamura
 * @date    05.12.12
 */
//============================================================================================
#include "gflib.h"


#include "app/itemuse.h"
#include "itemuse_local.h"



//=============================================================================================
/**
 * @brief 指定の道具が使えるか許可・不許可情報を格納する(itemmenu.h内のenum定義ITEMCHECK_ENABLE)
 *
 * @param   param   BAG_PARAMのポインタ
 * @param   type    使う道具の種類（itemmenu.h内のenum定義ITEMCHECK_ENABLEから指定）
 * @param   check   ITEMUSE_CODE
 *
 * @retval  none
 */
//=============================================================================================
void ITEMUSE_SetItemUseCheck( ITEMCHECK_WORK * icwk, ITEMCHECK_ENABLE type, ITEMUSE_CODE check )
{
  GF_ASSERT( type<ITEMCHECK_MAX );

  icwk->itemuse[type] = check;
}


//=============================================================================================
/**
 * @brief 指定の道具が使えるか許可・不許可情報を格納する(itemmenu.h内のenum定義ITEMCHECK_ENABLE)
 *
 * @param   param   BAG_PARAMのポインタ
 * @param   type    使う道具の種類（itemmenu.h内のenum定義ITEMCHECK_ENABLEから指定）
 *
 * @retval  ITEMUSE_CODE  ITEMUSE_OK=0なら使える
 */
//=============================================================================================
ITEMUSE_CODE ITEMUSE_GetItemUseCheck( ITEMCHECK_WORK * icwk, ITEMCHECK_ENABLE type )
{
  GF_ASSERT( type<ITEMCHECK_MAX );

  return (icwk->itemuse[type]);
}

