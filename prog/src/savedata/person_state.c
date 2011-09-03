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
void PERSON_STATE_SetLocalTvtCall( PERSON_STATE *p_wk, u16 person, BOOL is_ltvt )
{
  p_wk->is_ltvt[ person ] = is_ltvt;
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
//----------------------------------------------------------------------------
/**
 *	@brief  隠しキャラとのデートイベント設定
 *
 *	@param	PERSON_STATE *p_wk  ワーク
 *	@param  date                デートタイプ
 */
//-----------------------------------------------------------------------------
void PERSON_STATE_SetSecretDateEvent( PERSON_STATE *p_wk, PERSON_STATE_SECRET_DATE date )
{
  p_wk->secret_date_event  = date;
}
//----------------------------------------------------------------------------
/**
 *	@brief  隠しキャラとのデートイベント消去
 *
 *	@param	PERSON_STATE *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void PERSON_STATE_ClearSecretDateEvent( PERSON_STATE *p_wk )
{
  p_wk->secret_date_event  = PERSON_STATE_SECRET_DATE_NONE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  隠しキャラとのデートイベント取得
 *
 *	@param	PERSON_STATE *p_wk  ワーク
 *	@retval	デートイベント
 */
//-----------------------------------------------------------------------------
extern PERSON_STATE_SECRET_DATE PERSON_STATE_GetSecretDateEvent( const PERSON_STATE *cp_wk )
{
  return cp_wk->secret_date_event;
}
//----------------------------------------------------------------------------
/**
 *	@brief  隠しキャラの１日１回デートフラグ設定
 *
 *	@param	PERSON_STATE *p_wk  ワーク
 *	@param	enable              TRUEですでにデートした,　FALSEでまだデートしていない
 */
//-----------------------------------------------------------------------------
void PERSON_STATE_SetSecretOnceDateFlag( PERSON_STATE *p_wk, BOOL enable )
{
  p_wk->is_secret_once_date = enable;
}
//----------------------------------------------------------------------------
/**
 *	@brief  隠しキャラの１日１回デートフラグ取得
 *
 *	@param	PERSON_STATE *p_wk  ワーク
 *	@retval	enable              TRUEですでにデートした,　FALSEでまだデートしていない
 */
//-----------------------------------------------------------------------------
BOOL PERSON_STATE_GetSecretOnceDateFlag( const PERSON_STATE *cp_wk )
{
  return cp_wk->is_secret_once_date;
}
//----------------------------------------------------------------------------
/**
 *	@brief  交換したかどうかを設定
 *
 *	@param	PERSON_STATE *p_wk  ワーク
 *	@param	enable  TRUE交換した  FALSE交換してない
 */
//-----------------------------------------------------------------------------
void PERSON_STATE_SetTradeFlag( PERSON_STATE *p_wk, BOOL enable )
{
  p_wk->is_trade = enable;
}
//----------------------------------------------------------------------------
/**
 *	@brief  交換したかどうかチェック
 *
 *	@param	const PERSON_STATE *cp_wk   ワーク
 *
 *	@return TRUE交換した  FALSE交換してない
 */
//-----------------------------------------------------------------------------
BOOL PERSON_STATE_GetTradeFlag( const PERSON_STATE *cp_wk )
{
  return cp_wk->is_trade;
}

//----------------------------------------------------------------------------
/**
 *	@brief  交換したポケモン番号保存
 *
 *	@param	PERSON_STATE *p_wk  ワーク
 *	@param	monsno  交換したポケモン番号
 */
//-----------------------------------------------------------------------------
void PERSON_STATE_SetTradeMonsno( PERSON_STATE *p_wk, u16 monsno )
{
  p_wk->trade_monsno = monsno;
}
//----------------------------------------------------------------------------
/**
 *	@brief  交換したポケモン番号取得
 *
 *	@param	const PERSON_STATE *cp_wk   ワーク
 *
 *	@return 交換したポケモン番号
 */
//-----------------------------------------------------------------------------
u16 PERSON_STATE_GetTradeMonsno( const PERSON_STATE *cp_wk )
{
  return cp_wk->trade_monsno;
}
//----------------------------------------------------------------------------
/**
 *	@brief  デート終了フラグをセット
 *
 *	@param	PERSON_STATE *p_wk  ワーク
 *	@param	enable  TRUEならばデート終了した  FALSEならばデート終了してない
 */
//-----------------------------------------------------------------------------
void PERSON_STATE_SetDateEndFlag( PERSON_STATE *p_wk, BOOL enable )
{
  p_wk->is_date_end = enable;
}
//----------------------------------------------------------------------------
/**
 *	@brief  デート終了フラグを取得
 *
 *	@param	const PERSON_STATE *cp_wk ワーク
 *
 *	@return TRUEならばデート終了した  FALSEならばデート終了してない
 */
//-----------------------------------------------------------------------------
BOOL PERSON_STATE_GetDateEndFlag( const PERSON_STATE *cp_wk )
{
  return cp_wk->is_date_end;
}
//----------------------------------------------------------------------------
/**
 *	@brief  呼び出し可能場所フラグ設定
 *
 *	@param	PERSON_STATE *p_wk  ワーク
 *	@param	place 場所
 *	@param	enable  TRUE  呼び出した  FALSE呼び出していない
 */
//-----------------------------------------------------------------------------
void PERSON_STATE_SetCallPlaceFlag( PERSON_STATE *p_wk, PERSON_STATE_SECRET_CALLPLACE place, BOOL enable )
{
  GF_ASSERT( place < PERSON_STATE_SECRET_CALLPLACE_MAX );
  p_wk->call_place[ place / 8 ] |= (place % 8);
}
//----------------------------------------------------------------------------
/**
 *	@brief  呼び出し可能場所フラグ取得
 *
 *	@param	PERSON_STATE *p_wk  ワーク
 *	@param	place 場所
 *
 *	@return TRUE呼び出している  FALSE呼び出していない
 */
//-----------------------------------------------------------------------------
BOOL PERSON_STATE_GetCallPlaceFlag( const PERSON_STATE *cp_wk, PERSON_STATE_SECRET_CALLPLACE place )
{
  GF_ASSERT( place < PERSON_STATE_SECRET_CALLPLACE_MAX );
  return cp_wk->call_place[ place / 8 ] & (place % 8) != 0;
}
