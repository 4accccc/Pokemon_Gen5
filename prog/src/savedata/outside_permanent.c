//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		outside_permanent.c
 *	@brief  管理外永続データ
 *	@author	Toru=Nagihashi
 *	@data		2011.06.01
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include "system/gfl_use.h"
#include "savedata/outside_permanent.h"
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
///	キーシステムセーブデータ
//=====================================
struct _OUTSIDE_PERMANENT
{
  u8  key_system_area[KEY_SYSTEM_SAVE_WORK_SIZE];
  u8  data_convert_area[DATA_CONVERT_SAVE_WORK_SIZE];
};

SDK_COMPILER_ASSERT( OUTSIDE_PERMANENT_WORK_SIZE == sizeof(OUTSIDE_PERMANENT) );

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief ワークサイズ取得
 *
 *	@return サイズ
 */
//-----------------------------------------------------------------------------
u32 OUTSIDE_PERMANENT_GetWorkSize( void )
{
  return sizeof( OUTSIDE_PERMANENT );
}
//----------------------------------------------------------------------------
/**
 *	@brief  セーブ初期化
 *
 *	@param	OUTSIDE_PERMANENT *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void OUTSIDE_PERMANENT_Init( OUTSIDE_PERMANENT *p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(OUTSIDE_PERMANENT) );

  {
    KEY_SYSTEM_SAVE *p_key = OUTSIDE_PERMANENT_GetKeySystem( p_wk );
    KEY_SYSTEM_SAVE_Init( p_key );
  }

  {
    DATA_CONVERT_SAVE *p_data = OUTSIDE_PERMANENT_GetDataConvert( p_wk );
    DATA_CONVERT_SAVE_Init( p_data );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  キーシステムセーブ取得
 *
 *	@param	OUTSIDE_PERMANENT *p_wk   ワーク
 *
 *	@return キーシステムセーブデータ
 */
//-----------------------------------------------------------------------------
KEY_SYSTEM_SAVE * OUTSIDE_PERMANENT_GetKeySystem( OUTSIDE_PERMANENT *p_wk )
{
  return (KEY_SYSTEM_SAVE *)p_wk->key_system_area;
}
//----------------------------------------------------------------------------
/**
 *	@brief  データコンバートセーブ取得
 *
 *	@param	OUTSIDE_PERMANENT *p_wk   ワーク
 *
 *	@return データコンバートセーブデータ
 */
//-----------------------------------------------------------------------------
DATA_CONVERT_SAVE * OUTSIDE_PERMANENT_GetDataConvert( OUTSIDE_PERMANENT *p_wk )
{
  return (DATA_CONVERT_SAVE *)p_wk->data_convert_area;
}
