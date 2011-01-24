//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		person_state.c
 *	@brief  重要人物状態
 *	@author	Toru=Nagihashi
 *	@data		2011.01.19
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "savedata/save_control.h"
#include "savedata/save_tbl.h"
#include "savedata/etc_save.h"

//外部公開
#include "savedata/person_state.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#if PM_DEBUG
#endif //PM_DEBUG

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	セーブワーク
//=====================================
/*
struct _PERSON_STATE
{
  u8  state[PERSON_STATE_MAX];
  u8  is_ltvt[PERSON_STATE_MAX];
};
*/

//使用分と保存分が正しいか
SDK_COMPILER_ASSERT( LOCAL_TVT_PERSON_MAX <= PERSON_STATE_MAX );

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  人物状態ワークサイズ取得
 *	@retval 人物状態ワークサイズ
 */
//-----------------------------------------------------------------------------
u32 PERSON_STATE_GetWorkSize( void )
{
  return sizeof( PERSON_STATE );
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物状態ワーク  初期化
 *
 *	@param	void *p_wk_adrs 初期化ワーク
 */
//-----------------------------------------------------------------------------
void PERSON_STATE_Init( void *p_wk_adrs )
{
  GFL_STD_MemClear( p_wk_adrs , sizeof(PERSON_STATE));
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物状態ワーク  ワーク取得
 *
 *	@param	SAVE_CONTROL_WORK *p_sv   セーブワーク
 *
 *	@return 人物状態ワーク
 */
//-----------------------------------------------------------------------------
PERSON_STATE* PERSON_STATE_GetWork( SAVE_CONTROL_WORK *p_sv )
{
  ETC_SAVE_WORK *p_etc  = SaveControl_DataPtrGet(p_sv, GMDATA_ID_ETC);
  return EtcSave_GetPersonStatePtr( p_etc );
}

//----------------------------------------------------------------------------
/**
 *	@brief  人物状態ワーク  ワーク取得CONST版
 *
 *	@param	SAVE_CONTROL_WORK *p_sv   セーブワーク
 *
 *	@return 人物状態ワークCONST版
 */
//-----------------------------------------------------------------------------
const PERSON_STATE* PERSON_STATE_GetWorkConst( const SAVE_CONTROL_WORK *cp_sv )
{
  return PERSON_STATE_GetWork( (SAVE_CONTROL_WORK *)cp_sv );
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物状態ワーク  人物状態設定
 *
 *	@param	PERSON_STATE* p_wk  ワーク
 *	@param	person  人物ID
 *	@param	state   状態ID
 */
//-----------------------------------------------------------------------------
void PERSON_STATE_SetState( PERSON_STATE* p_wk, u16 person, u16 state )
{
  p_wk->state[ person ] = state;
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物状態ワーク  人物状態取得
 *
 *	@param	PERSON_STATE* p_wk  ワーク
 *	@param	person  人物ID
 *
 *	@return 状態ID
 */
//-----------------------------------------------------------------------------
u16 PERSON_STATE_GetState( const PERSON_STATE* cp_wk, u16 person )
{
  return cp_wk->state[ person ];
}
//----------------------------------------------------------------------------
/**
 *	@brief  ローカルライブキャスター  電話かけフラグON
 *
 *	@param	PERSON_STATE *p_wk  ワーク
 *	@param	person              人物ID
 */
//-----------------------------------------------------------------------------
void PERSON_STATE_SetLocalTvtCall( PERSON_STATE *p_wk, u16 person )
{
  p_wk->is_ltvt[ person ] = TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ローカルライブキャスター  電話フラグ取得
 *
 *	@param	const PERSON_STATE *cp_wk ワーク
 *	@param	person                    人物ID
 *
 *	@return TRUE電話かけられる  FALSE電話かけられない
 */
//-----------------------------------------------------------------------------
BOOL PERSON_STATE_GetLocalTvtCall( const PERSON_STATE *cp_wk, u16 person )
{
  return cp_wk->is_ltvt[ person ];
}
