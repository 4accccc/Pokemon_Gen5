//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_debug.c
 *	@brief  ギミック内デバッグ処理
 *	@author	Toru=Nagihashi
 *	@data		2011.09.21
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifdef PM_DEBUG
#include <gflib.h>
#include "resort_debug.h"
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

#define RESORT_DEBUG_BIT_ARRAY_LEN  ((RESORT_DEBUG_FLAG_MAX+7)/8)

static u8  s_debug_flag[ RESORT_DEBUG_BIT_ARRAY_LEN ] = {0};
static u32 s_debug_work[ RESORT_DEBUG_WORK_MAX ]  = {0};


//----------------------------------------------------------------------------
/**
 *	@brief  デバッグフラグ初期化
 */
//-----------------------------------------------------------------------------
void RESORT_DEBUG_FLAGWORK_Init( void )
{
  GFL_STD_MemClear( s_debug_flag, sizeof(u8)*RESORT_DEBUG_BIT_ARRAY_LEN );
  GFL_STD_MemFill32( s_debug_work, RESORT_DEBUG_WORK_DEFAULT, sizeof(u32)*RESORT_DEBUG_WORK_MAX );
}

//----------------------------------------------------------------------------
/**
 *	@brief  デバッグフラグ設定
 *
 *	@param	RESORT_DEBUG_FLAG flag  フラグ
 *	@param	is_on   TRUEオン    FALSEオフ
 */
//-----------------------------------------------------------------------------
void RESORT_DEBUG_FLAG_Set( RESORT_DEBUG_FLAG flag, BOOL is_on )
{
  const u8 bit = flag%8;
  const u8 arr = flag/8;

  if( is_on )
  {
    s_debug_flag[arr] |= 1<<bit;
  }
  else
  {
    s_debug_flag[arr] &= ~(1<<bit);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  デバッグフラグ取得
 *
 *	@param	RESORT_DEBUG_FLAG flag  フラグ
 *
 *	@return TRUE  オン  FALSE　オフ
 */
//-----------------------------------------------------------------------------
BOOL RESORT_DEBUG_FLAG_Get( RESORT_DEBUG_FLAG flag )
{
  const u8 bit = flag%8;
  const u8 arr = flag/8;
  
  return ( (s_debug_flag[arr] & (1<<bit)) != 0 );
}

//----------------------------------------------------------------------------
/**
 *	@brief  デバッグワーク設定
 *
 *	@param	RESORT_DEBUG_WORK wk  ワーク
 *	@param	value 値
 */
//-----------------------------------------------------------------------------
void RESORT_DEBUG_WORK_Set( RESORT_DEBUG_WORK wk, u32 value )
{
  s_debug_work[ wk ] = value;
}
//----------------------------------------------------------------------------
/**
 *	@brief  デバッグワーク取得
 *
 *	@param	RESORT_DEBUG_WORK wk ワーク
 *
 *	@return 値
 */
//-----------------------------------------------------------------------------
u32 RESORT_DEBUG_WORK_Get( RESORT_DEBUG_WORK wk )
{
  return s_debug_work[ wk ];
}

#endif  //PM_DEBUG
