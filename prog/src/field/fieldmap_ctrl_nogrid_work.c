//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		fieldmap_ctrl_nogrid_work.h
 *	@brief  フィールドマップコントロール　ノーグリッド　共通ワーク
 *	@author	tomoya takahashi
 *	@date		2009.08.28
 *
 *	モジュール名：FIELDMAP_CTRL_NOGRID_WORK
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>


#include "fieldmap_ctrl_nogrid_work.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	フィールドマップコントロール　ノーグリッドマップ　共通ワーク
//=====================================
struct _FIELDMAP_CTRL_NOGRID_WORK 
{
  FIELD_PLAYER_NOGRID_WORK* p_player_work;

  void* p_localwork;
};

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------


//----------------------------------------------------------------------------
/**
 *	@brief  ワーク生成
 *
 *	@param	heapID  ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
FIELDMAP_CTRL_NOGRID_WORK* FIELDMAP_CTRL_NOGRID_WORK_Create( u32 heapID )
{
  FIELDMAP_CTRL_NOGRID_WORK* p_wk;

  p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELDMAP_CTRL_NOGRID_WORK) );

  return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ワーク破棄
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void FIELDMAP_CTRL_NOGRID_WORK_Delete( FIELDMAP_CTRL_NOGRID_WORK* p_wk )
{
  // ローカルワークのフリーが行われているかチェック
  GF_ASSERT( p_wk->p_localwork == NULL );

  GFL_HEAP_FreeMemory( p_wk );
}


//----------------------------------------------------------------------------
/**
 *	@brief  ノーグリッドワーク　ローカルワークの確保
 *
 *	@param	p_wk        ワーク
 *	@param	heapID      ヒープID
 *	@param	size        サイズ
 *
 *	@return ローカルワーク確保
 */
//-----------------------------------------------------------------------------
void* FIELDMAP_CTRL_NOGRID_WORK_AllocLocalWork( FIELDMAP_CTRL_NOGRID_WORK* p_wk, u32 heapID, u32 size )
{
  GF_ASSERT( p_wk );
  GF_ASSERT( p_wk->p_localwork == NULL );

  p_wk->p_localwork = GFL_HEAP_AllocClearMemory( heapID, size );
  return p_wk->p_localwork;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ノーグリッドワーク  ローカルワークの破棄
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void FIELDMAP_CTRL_NOGRID_WORK_FreeLocalWork( FIELDMAP_CTRL_NOGRID_WORK* p_wk )
{
  GF_ASSERT( p_wk );

  if( p_wk->p_localwork )
  {
    GFL_HEAP_FreeMemory( p_wk->p_localwork );
    p_wk->p_localwork = NULL;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ノーグリッドワーク  ローカルワークの取得
 *
 *	@param	cp_wk   ワーク
 *  
 *	@return ローカルワーク
 */
//-----------------------------------------------------------------------------
void* FIELDMAP_CTRL_NOGRID_WORK_GetLocalWork( const FIELDMAP_CTRL_NOGRID_WORK* cp_wk )
{
  GF_ASSERT( cp_wk );

  return cp_wk->p_localwork;
}



//----------------------------------------------------------------------------
/**
 *	@brief  ノーグリッドワーク　プレイヤーワークの取得
 *
 *	@param	cp_wk   ワーク
 *  
 *	@return ノーグリッドプレイヤーワーク
 */
//-----------------------------------------------------------------------------
FIELD_PLAYER_NOGRID_WORK* FIELDMAP_CTRL_NOGRID_WORK_GetNogridPlayerWork( const FIELDMAP_CTRL_NOGRID_WORK* cp_wk )
{
  GF_ASSERT( cp_wk );

  return cp_wk->p_player_work;
}

