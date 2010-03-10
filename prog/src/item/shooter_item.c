//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		shooter_item.c
 *	@brief  シューターアイテム
 *	@author	Toru=Nagihashi
 *	@data		2010.03.09
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//データ
#include "item/shooter_item_data.h" //cdatの中でMAXを使用しているのでそれより前にインクルード
#include "shooter_item_data.cdat"

//外部公開
#include "item/shooter_item.h"

SDK_COMPILER_ASSERT( SHOOTER_ITEM_MAX/8 <= SHOOTER_ITEM_BIT_TBL_MAX );

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================


//----------------------------------------------------------------------------
/**
 *	@brief  シューターが使用可能かチェック
 *
 *	@param	const SHOOTER_ITEM_BIT_WORK *cp_wk
 *	@param	shooter_item 
 *
 *	@return TRUEで使用可能  FALSEで使用不能
 */
//-----------------------------------------------------------------------------
BOOL SHOOTER_ITEM_IsUse( const SHOOTER_ITEM_BIT_WORK *cp_wk, u32 shooter_item )
{ 
  const u32 bit     = 1<<shooter_item;
  const u8 bit_idx  = bit / 8;
  const u8 bit_pos  = bit % 8;

  return (cp_wk->bit_tbl[ bit_idx ] & bit_pos ) != 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  シューターアイテムのインデックスからアイテムへ変換
 *
 *	@param	u32 shooter_item  シューターアイテムのインデックス
 *
 *	@return アイテム番号
 */
//-----------------------------------------------------------------------------
u32  SHOOTER_ITEM_ShooterIndexToItemIndex( u32 shooter_item )
{ 
  GF_ASSERT( shooter_item < SHOOTER_ITEM_MAX );
  return sc_shooter_item_to_item[ shooter_item ].name;
}

//----------------------------------------------------------------------------
/**
 *	@brief  シューターアイテムからコストを取得する
 *
 *	@param	u32 shooter_item  シューターアイテムのインデックス
 *
 *	@return コスト
 */
//-----------------------------------------------------------------------------
u32  SHOOTER_ITEM_ShooterIndexToCost( u32 shooter_item )
{ 
  GF_ASSERT( shooter_item < SHOOTER_ITEM_MAX );
  return sc_shooter_item_to_item[ shooter_item ].cost;
}
//----------------------------------------------------------------------------
/**
 *	@brief  アイテムからコストを取得する
 *
 *	@param	u32 item  アイテム
 *
 *	@return コスト
 */
//-----------------------------------------------------------------------------
u32  SHOOTER_ITEM_ItemIndexToCost( u32 item )
{ 
  int i;
  for( i = 0; i < SHOOTER_ITEM_MAX; i++ )
  { 
    if( sc_shooter_item_to_item[ i ].name == item )
    { 
      return sc_shooter_item_to_item[ i ].cost;
    }
  }

  GF_ASSERT_MSG( 0, "登録されていないシューターアイテムです %d", item );
  return 0;
}
