//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_location.c
 *	@brief
 *  @author  tamada GAME FREAK inc.
 *	@date		2009.08.07
 *
 *	ロケーション用の関数をrailから移植  tomoya takahashi
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


#include "gflib.h"
#include "field/rail_location.h"
#include "field/field_dir.h"

#include "field_rail.h"

//============================================================================================
//============================================================================================

//============================================================================================
//
//
//      DIRとRAIL_KEYの対応データ
//
//
//============================================================================================
static const u8 sc_DIR_TO_RAILKEY[DIR_MAX4] = 
{
  RAIL_KEY_UP,
  RAIL_KEY_DOWN,
  RAIL_KEY_LEFT,
  RAIL_KEY_RIGHT,
};
//------------------------------------------------------------------
//------------------------------------------------------------------

//------------------------------------------------------------------
/**
 * @brief RAIL_LOCATION構造体の初期化処理
 */
//------------------------------------------------------------------
void RAIL_LOCATION_Init(RAIL_LOCATION * railLoc)
{
  railLoc->type = FIELD_RAIL_TYPE_POINT;
  railLoc->rail_index = 0;
  railLoc->line_grid = 0;
  railLoc->width_grid = 0;
  railLoc->key = RAIL_KEY_NULL;
}


//------------------------------------------------------------------
// RAIL_KEYとDIRの変換
//  
//  DIR_～をRAIL_KEY_～に変換
//------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  RAIL_KEYをDIRに変換
 *
 *	@param	key キー
 *
 *	@return　DIR
 */
//-----------------------------------------------------------------------------
u32 FIELD_RAIL_TOOL_ConvertRailKeyToDir( u32 key )
{
  int i;

  GF_ASSERT( key < RAIL_KEY_MAX );

  for( i=0; i<DIR_MAX4; i++ )
  {
    if( sc_DIR_TO_RAILKEY[i] == key )
    {
      return i;
    }
  }

  GF_ASSERT( 0 );
  return 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  DIRをRAIL_KEYに変換
 *
 *	@param	dir   DIR
 *
 *	@return RAIL_KEY
 */
//-----------------------------------------------------------------------------
u32 FIELD_RAIL_TOOL_ConvertDirToRailKey( u32 dir )
{
  // 対応しているのは
  // DIR_UP DIR_DOWN DIR_LEFT DIR_RIGHT
  GF_ASSERT( dir < DIR_MAX4 );

  return sc_DIR_TO_RAILKEY[dir];
}
