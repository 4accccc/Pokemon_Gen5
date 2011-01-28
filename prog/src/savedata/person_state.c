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
static s32 Person_State_GetSecretDiffDay( const PERSON_STATE *cp_wk, const RTCDate *cp_date );
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
 *	@param	const RTCDate *cp_date  デートの日
 */
//-----------------------------------------------------------------------------
void PERSON_STATE_SetSecretDateEvent( PERSON_STATE *p_wk, PERSON_STATE_SECRET_DATE date, const RTCDate *cp_date )
{
  p_wk->secret_date_event  = date;
  p_wk->secret_date = GFDATE_RTCDate2GFDate( cp_date );
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
 *	@brief  今日がデートの日時かチェック
 *
 *	@param	const PERSON_STATE *cp_wk ワーク
 *	@param	RTCDate *cp_date          日時
 *
 *	@return TRUEならば今日がデートの日  FALSEならば違う
 */
//-----------------------------------------------------------------------------
BOOL PERSON_STATE_CheckSecretDataEventToday( const PERSON_STATE *cp_wk, const RTCDate *cp_date )
{
  if( cp_wk->secret_date_event != PERSON_STATE_SECRET_DATE_NONE)
  {
    if( Person_State_GetSecretDiffDay( cp_wk, cp_date ) )
    {
      return TRUE;
    }
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  デートの日時を過ぎたかチェック
 *
 *	@param	const PERSON_STATE *cp_wk ワーク
 *	@param	RTCDate *cp_date          日時
 *
 *	@return TRUEならば過ぎてしまった  FALSEならば違う
 */
//-----------------------------------------------------------------------------
BOOL PERSON_STATE_CheckSecretDataEventLater( const PERSON_STATE *cp_wk, const RTCDate *cp_date )
{
  if( cp_wk->secret_date_event !=  PERSON_STATE_SECRET_DATE_NONE)
  {
    if( Person_State_GetSecretDiffDay( cp_wk, cp_date ) > 0 )
    {
      return TRUE;
    }
  }

  return FALSE;
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
 *	@brief  隠しキャラの不機嫌フラグ設定
 *
 *	@param	PERSON_STATE *p_wk  ワーク
 *	@param	enable              TRUEで不機嫌,　FALSEで正常
 */
//-----------------------------------------------------------------------------
void PERSON_STATE_SetSecretDispleased( PERSON_STATE *p_wk, BOOL enable )
{
  p_wk->is_secret_displeased  = enable;
}
//----------------------------------------------------------------------------
/**
 *	@brief  隠しキャラの不機嫌フラグ取得
 *
 *	@param	PERSON_STATE *p_wk  ワーク
 *	@retval TRUEで不機嫌,　FALSEで正常
 */
//-----------------------------------------------------------------------------
BOOL PERSON_STATE_GetSecretDispleased( const PERSON_STATE *cp_wk )
{
  return cp_wk->is_secret_displeased;
}
//----------------------------------------------------------------------------
/**
 *	@brief  隠しキャラの１日１回電話フラグ設定
 *
 *	@param	PERSON_STATE *p_wk  ワーク
 *	@param	enable              TRUEですでに電話した,　FALSEでまだ電話していない
 */
//-----------------------------------------------------------------------------
void PERSON_STATE_SetSecretOnceDayCallFlag( PERSON_STATE *p_wk, BOOL enable )
{
  p_wk->is_secret_once_day_call = enable;
}
//----------------------------------------------------------------------------
/**
 *	@brief  隠しキャラの１日１回電話フラグ取得
 *
 *	@param	PERSON_STATE *p_wk  ワーク
 *	@retval TRUEですでに電話した,　FALSEでまだ電話していない
 */
//-----------------------------------------------------------------------------
BOOL PERSON_STATE_GetSecretOnceDayCallFlag( const PERSON_STATE *cp_wk )
{
  return cp_wk->is_secret_once_day_call;
}
//----------------------------------------------------------------------------
/**
 *	@brief  隠しキャラの１季節１回デートフラグを設定
 *
 *	@param	PERSON_STATE *p_wk  ワーク
 *	@param	enable  TRUEでデートした  FALSEでデートしていない
 */
//-----------------------------------------------------------------------------
void PERSON_STATE_SetSeasonDateFlag( PERSON_STATE *p_wk, BOOL enable )
{
  p_wk->is_season_date  = enable;
}
//----------------------------------------------------------------------------
/**
 *	@brief  隠しキャラの１季節１回デートフラグを設定
 *
 *	@param	PERSON_STATE *p_wk  ワーク
 *	@retval	TRUEでデートした  FALSEでデートしていない
 */
//-----------------------------------------------------------------------------
BOOL PERSON_STATE_GetSeasonDateFlag( const PERSON_STATE *cp_wk )
{
  return cp_wk->is_season_date;
}
//----------------------------------------------------------------------------
/**
 *	@brief  約束の期日チェック
 *
 *	@param	PERSON_STATE *p_wk  ワーク
 *	@param  cp_date             日
 *
 *	@return 0で当日 正の数で過ぎた  負の数で前
 */
//-----------------------------------------------------------------------------
static s32 Person_State_GetSecretDiffDay( const PERSON_STATE *cp_wk, const RTCDate *cp_date )
{
  //約束した日を保存しているので、
  //約束した日の次の日が今日ならばデートの日
  RTCDate date;
  int now_days;
  int promise_days;

  GFDATE_GFDate2RTCDate( cp_wk->secret_date, &date );
  now_days      = GFL_RTC_GetDaysOffset( cp_date );
  promise_days  = GFL_RTC_GetDaysOffset( &date );

  return now_days - (promise_days+1);
}
#ifdef PM_DEBUG
//----------------------------------------------------------------------------
/**
 *	@brief  デバッグ用  デート日付取得
 *
 *	@param	const PERSON_STATE *cp_wk ワーク
 *	@param	*p_date   日付
 */
//-----------------------------------------------------------------------------
void DEBUG_PERSON_STATE_GetDate( const PERSON_STATE *cp_wk, RTCDate *p_date )
{
  GFDATE_GFDate2RTCDate( cp_wk->secret_date, p_date );
}
#endif //PM_DEBUG
