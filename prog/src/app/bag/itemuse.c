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
 * @param   check   0:使えない  1:使える
 *
 * @retval  none
 */
//=============================================================================================
void ITEMUSE_SetItemUseCheck( ITEMCHECK_WORK * icwk, ITEMCHECK_ENABLE type, BOOL check )
{
  GF_ASSERT( type<ITEMCHECK_MAX );

  icwk->item_enable |= (check<<type);
}


//=============================================================================================
/**
 * @brief 指定の道具が使えるか許可・不許可情報を格納する(itemmenu.h内のenum定義ITEMCHECK_ENABLE)
 *
 * @param   param   BAG_PARAMのポインタ
 * @param   type    使う道具の種類（itemmenu.h内のenum定義ITEMCHECK_ENABLEから指定）
 * @param   check   0:使えない  1:使える
 *
 * @retval  none
 */
//=============================================================================================
BOOL ITEMUSE_GetItemUseCheck( ITEMCHECK_WORK * icwk, ITEMCHECK_ENABLE type )
{
  GF_ASSERT( type<ITEMCHECK_MAX );
//  return 1;

  return (icwk->item_enable>>type)&1;
}

