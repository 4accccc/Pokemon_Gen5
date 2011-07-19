//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		data_convert_save.c
 *	@brief  データコンバートセーブ
 *	@author	Toru=Nagihashi
 *	@data		2011.07.15
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include "system/gfl_use.h"
#include "savedata/data_convert_save.h"
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
//-------------------------------------
///	データコンバートセーブデータ
//=====================================
struct _DATA_CONVERT_SAVE
{
  WB_SAVE_CONV_OWNER  owner;
  WB_SAVE_CONV_DATA   data;
};

SDK_COMPILER_ASSERT( DATA_CONVERT_SAVE_WORK_SIZE == sizeof(DATA_CONVERT_SAVE) );

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ワークサイズ取得
 *
 *	@return ワークサイズ
 */
//-----------------------------------------------------------------------------
u32  DATA_CONVERT_SAVE_GetWorkSize( void )
{
  return sizeof(DATA_CONVERT_SAVE);
}
//----------------------------------------------------------------------------
/**
 *	@brief  ワーク初期化
 *
 *	@param	DATA_CONVERT_SAVE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void DATA_CONVERT_SAVE_Init( DATA_CONVERT_SAVE *p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(DATA_CONVERT_SAVE) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  コンバート
 *
 *	@param	DATA_CONVERT_SAVE *p_wk ワーク
 *	@param	void *cp_data_adrs      コンバート元データ
 */
//-----------------------------------------------------------------------------
void DATA_CONVERT_SAVE_Convert( DATA_CONVERT_SAVE *p_wk, const WB_SAVE_CONV_ALL_DATA *cp_all_data )
{
  p_wk->owner   = cp_all_data->owner;
  p_wk->data    = cp_all_data->data;
}

//----------------------------------------------------------------------------
/**
 *	@brief  コンバート元データの人と、既にコンバート済みデータの持ち主が同じか
 *	        もしコンバートを1回もしていなかったらTRUEが帰る
 *
 *	@param	const DATA_CONVERT_SAVE *cp_wk  ワーク
 *	@param	void *cp_data_adrs  コンバート元データ
 *
 *	@return TRUE同じ人  FALSE違う人
 */
//-----------------------------------------------------------------------------
BOOL DATA_CONVERT_SAVE_IsSameOwner( const DATA_CONVERT_SAVE *cp_wk, const WB_SAVE_CONV_OWNER *cp_owner )
{
  return GFL_STD_CODED_CheckSum(&cp_wk->owner, sizeof(WB_SAVE_CONV_OWNER)) == 0
      || GFL_STD_MemComp( &cp_wk->owner, cp_owner, sizeof(WB_SAVE_CONV_OWNER) ) == 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  データを取得
 *
 *	@param	const DATA_CONVERT_SAVE *cp_wk  ワーク
 *	@param	id            受け取りID
 *	@param	*p_data_adrs  データ受取りアドレス
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
u32 DATA_CONVERT_SAVE_GetData( const DATA_CONVERT_SAVE *cp_wk, DATA_CONVERT_GET_ID id, void *p_data_adrs )
{
  switch( id )
  {
  case DATA_CONVERT_GET_ID_MUSICAL_POINT:
    return cp_wk->data.musical_sum_point;
  case DATA_CONVERT_GET_ID_MUSICAL_ENTRY:
    return cp_wk->data.musical_entry_num;
  case DATA_CONVERT_GET_ID_DENDOU_POKE_00: //p_data_adrsからWB_SAVE_CONV_DENDOU_POKEMON_DATA構造体で受け取る
    /* fall though */
  case DATA_CONVERT_GET_ID_DENDOU_POKE_01: //ゞ
    /* fall though */
  case DATA_CONVERT_GET_ID_DENDOU_POKE_02: //ゞ
    /* fall though */
  case DATA_CONVERT_GET_ID_DENDOU_POKE_03: //ゞ
    /* fall though */
  case DATA_CONVERT_GET_ID_DENDOU_POKE_04: //ゞ
    /* fall though */
  case DATA_CONVERT_GET_ID_DENDOU_POKE_05: //ゞ
    {
      u16 idx = id - DATA_CONVERT_GET_ID_DENDOU_POKE_00;
      GFL_STD_MemCopy( &cp_wk->data.pokemon[idx], p_data_adrs, sizeof(WB_SAVE_CONV_DENDOU_POKEMON_DATA) );
    }
    return 0;

  case DATA_CONVERT_GET_ID_OWNER:  //p_data_adrsからWB_SAVE_CONV_OWNER構造体で受け取る
    GFL_STD_MemCopy( &cp_wk->owner, p_data_adrs, sizeof(WB_SAVE_CONV_OWNER) );
    return 0;
  case DATA_CONVERT_GET_ID_MUSICAL_ITEMBIT:  //p_data_adrsからu8のWB_SAVE_CONV_MUSICAL_ITEM_BIT配列で受け取る
    GFL_STD_MemCopy( cp_wk->data.musical_item, p_data_adrs, sizeof(u8)*WB_SAVE_CONV_MUSICAL_ITEM_BIT);
    return 0;
  default:
    GF_ASSERT(0);
  }

  return 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  フラグを取得
 *
 *	@param	const DATA_CONVERT_SAVE *cp_wk  ワーク
 *	@param	flag  チェックするフラグ
 *
 *	@return TRUEならばフラグON  FALSEならばフラグOFF
 */
//-----------------------------------------------------------------------------
BOOL DATA_CONVERT_SAVE_GetFlag( const DATA_CONVERT_SAVE *cp_wk, WB_SAVE_CONV_FLAG flag )
{
  GF_ASSERT( flag < WB_SAVE_CONV_FLAG_MAX );
  return (cp_wk->data.flag[ flag/8 ] & (1<<(flag%8)) ) != 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ワークの値をチェック
 *
 *	@param	const DATA_CONVERT_SAVE *cp_wk  ワーク
 *	@param	wk  調べるワーク
 *
 *	@return ワークの値
 */
//-----------------------------------------------------------------------------
u16 DATA_CONVERT_SAVE_GetWork( const DATA_CONVERT_SAVE *cp_wk, WB_SAVE_CONV_WORK wk )
{
  GF_ASSERT( wk < WB_SAVE_CONV_WORK_MAX );
  return cp_wk->data.work[ wk ];
}

#ifdef PM_DEBUG
void DATA_CONVERT_SAVE_SetData( DATA_CONVERT_SAVE *p_wk, DATA_CONVERT_GET_ID id, u32 data )
{
  switch( id )
  {
  case DATA_CONVERT_GET_ID_MUSICAL_POINT:
    p_wk->data.musical_sum_point  = data;
    break;
  case DATA_CONVERT_GET_ID_MUSICAL_ENTRY:
    p_wk->data.musical_entry_num  = data;
    break;
  case DATA_CONVERT_GET_ID_DENDOU_POKE_00: //p_data_adrsからWB_SAVE_CONV_DENDOU_POKEMON_DATA構造体で受け取る
    /* fall though */
  case DATA_CONVERT_GET_ID_DENDOU_POKE_01: //ゞ
    /* fall though */
  case DATA_CONVERT_GET_ID_DENDOU_POKE_02: //ゞ
    /* fall though */
  case DATA_CONVERT_GET_ID_DENDOU_POKE_03: //ゞ
    /* fall though */
  case DATA_CONVERT_GET_ID_DENDOU_POKE_04: //ゞ
    /* fall though */
  case DATA_CONVERT_GET_ID_DENDOU_POKE_05: //ゞ
    {
      u16 idx = id - DATA_CONVERT_GET_ID_DENDOU_POKE_00;
      GFL_STD_MemCopy( (void*)data, &p_wk->data.pokemon[idx], sizeof(WB_SAVE_CONV_DENDOU_POKEMON_DATA) );
    }
    break;
  case DATA_CONVERT_GET_ID_OWNER:  //p_data_adrsからWB_SAVE_CONV_OWNER構造体で受け取る
    GFL_STD_MemCopy( (void*)data, &p_wk->owner, sizeof(WB_SAVE_CONV_OWNER) );
    break;
  case DATA_CONVERT_GET_ID_MUSICAL_ITEMBIT:  //p_data_adrsからu8のWB_SAVE_CONV_MUSICAL_ITEM_BIT配列で受け取る
    GFL_STD_MemCopy( (void*)data, p_wk->data.musical_item, sizeof(u8)*WB_SAVE_CONV_MUSICAL_ITEM_BIT);
    break;
  default:
    GF_ASSERT(0);
  }
}
void DATA_CONVERT_SAVE_SetFlag( DATA_CONVERT_SAVE *p_wk, WB_SAVE_CONV_FLAG flag, BOOL on_off )
{
  GF_ASSERT( flag < WB_SAVE_CONV_FLAG_MAX );

  if( on_off )
  {
    p_wk->data.flag[ flag/8 ] |= (1<<(flag%8));
  }
  else
  {
    p_wk->data.flag[ flag/8 ] &= ~(1<<(flag%8));
  }
}
void DATA_CONVERT_SAVE_SetWork( DATA_CONVERT_SAVE *p_wk, WB_SAVE_CONV_WORK wk, u16 value )
{
  GF_ASSERT( wk < WB_SAVE_CONV_WORK_MAX );
  p_wk->data.work[ wk ]  = value;
}
#endif
