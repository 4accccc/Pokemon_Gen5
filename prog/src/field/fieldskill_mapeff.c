//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		fieldskill_mapeff.c
 *	@brief  フィールド技　ゾーンエフェクト
 *	@author	tomoya takahashi
 *	@date		2009.11.19
 *
 *	モジュール名：FIELDSKILL_MAPEFF
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "fieldskill_mapeff.h"

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
///	フィールドスキル　ゾーンごとの効果
//=====================================
struct _FIELDSKILL_MAPEFF 
{
	FIELD_FLASH * field_flash;
};

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

// フラッシュワーク管理
static void SKILL_MAPEFF_CreateFlash( FIELDSKILL_MAPEFF* p_wk, FIELDSKILL_MAPEFF_MSK msk, HEAPID heapID );
static void SKILL_MAPEFF_DeleteFlash( FIELDSKILL_MAPEFF* p_wk );
static void SKILL_MAPEFF_MainFlash( FIELDSKILL_MAPEFF* p_wk );
static void SKILL_MAPEFF_DrawFlash( FIELDSKILL_MAPEFF* p_wk );




//----------------------------------------------------------------------------
/**
 *	@brief  フィールド技　効果管理システム　生成
 *
 *  @param  msk       設定マスク
 *	@param	heapID    ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
FIELDSKILL_MAPEFF* FIELDSKILL_MAPEFF_Create( FIELDSKILL_MAPEFF_MSK msk, HEAPID heapID )
{
  FIELDSKILL_MAPEFF* p_wk;

  p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELDSKILL_MAPEFF) );

  // フラッシュ生成
  SKILL_MAPEFF_CreateFlash( p_wk, msk, heapID );

  return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  破棄処理
 *
 *	@param	p_wk ワーク
 */
//-----------------------------------------------------------------------------
void FIELDSKILL_MAPEFF_Delete( FIELDSKILL_MAPEFF* p_wk )
{
  // フラッシュ破棄
  SKILL_MAPEFF_DeleteFlash( p_wk );

  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  メイン動作
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void FIELDSKILL_MAPEFF_Main( FIELDSKILL_MAPEFF* p_wk )
{
  // フラッシュメイン
  SKILL_MAPEFF_MainFlash( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  描画動作
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void FIELDSKILL_MAPEFF_Draw( FIELDSKILL_MAPEFF* p_wk )
{
  // フラッシュ描画
  SKILL_MAPEFF_DrawFlash( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  フラッシュが動作しているかチェック
 *
 *	@param	cp_wk   ワーク
 *
 *	@retval TRUE    している
 *	@retval FALSE   していない
 */
//-----------------------------------------------------------------------------
BOOL FIELDSKILL_MAPEFF_IsFlash( const FIELDSKILL_MAPEFF* cp_wk )
{
  if( cp_wk->field_flash )
  {
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フラッシュワーク取得
 *
 *	@param	cp_wk   ワーク
 *
 *	@return フラッシュワーク
 */
//-----------------------------------------------------------------------------
FIELD_FLASH* FIELDSKILL_MAPEFF_GetFlash( const FIELDSKILL_MAPEFF* cp_wk )
{
  GF_ASSERT( cp_wk->field_flash );
  return cp_wk->field_flash;
}




//-----------------------------------------------------------------------------
/**
 *      private関数
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  フラッシュシステムの生成
 *
 *	@param	p_wk    ワーク
 *	@param	msk     マスク
 *	@param  heapID  ヒープID
 *
 *	フラッシュは、発生箇所が少ないので、必要なときにだけ生成する
 */
//-----------------------------------------------------------------------------
static void SKILL_MAPEFF_CreateFlash( FIELDSKILL_MAPEFF* p_wk, FIELDSKILL_MAPEFF_MSK msk, HEAPID heapID )
{
  if( (msk & (FIELDSKILL_MAPEFF_MSK_FLASH_NEAR|FIELDSKILL_MAPEFF_MSK_FLASH_FAR)) )
  {
    p_wk->field_flash = FIELD_FLASH_Create( heapID );

    // 両方設定されている
    GF_ASSERT( (msk & (FIELDSKILL_MAPEFF_MSK_FLASH_NEAR|FIELDSKILL_MAPEFF_MSK_FLASH_FAR)) != (FIELDSKILL_MAPEFF_MSK_FLASH_NEAR|FIELDSKILL_MAPEFF_MSK_FLASH_FAR) );
    
    // FARを優先
    if( msk & FIELDSKILL_MAPEFF_MSK_FLASH_FAR )
    {
      FIELD_FLASH_Control( p_wk->field_flash, FIELD_FLASH_REQ_ON_FAR );
    }
    else
    {
      FIELD_FLASH_Control( p_wk->field_flash, FIELD_FLASH_REQ_ON_NEAR );
    }
  }
  else
  {
    p_wk->field_flash = NULL;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  フラッシュワークを破棄
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void SKILL_MAPEFF_DeleteFlash( FIELDSKILL_MAPEFF* p_wk )
{
  FIELD_FLASH_Delete( p_wk->field_flash );
  p_wk->field_flash = NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フラッシュメイン処理
 *
 *	@param	p_wk 
 */
//-----------------------------------------------------------------------------
static void SKILL_MAPEFF_MainFlash( FIELDSKILL_MAPEFF* p_wk )
{
  FIELD_FLASH_Update( p_wk->field_flash );
}

//----------------------------------------------------------------------------
/**
 *	@brief  フラッシュ描画処理
 *
 *	@param	p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void SKILL_MAPEFF_DrawFlash( FIELDSKILL_MAPEFF* p_wk )
{
  FIELD_FLASH_Draw( p_wk->field_flash );
}



