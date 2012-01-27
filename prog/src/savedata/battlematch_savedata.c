//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		battlematch_savedata.h
 *	@brief  バトル大会用セーブデータ
 *	@author	Toru=Nagihashi
 *	@date		2010.03.08
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include "savedata/battlematch_savedata.h"
#include "battlematch_savedata_def.h"
#include "savedata/save_tbl.h"
//=============================================================================
/**
 *        構造体
 */
//=============================================================================
//-------------------------------------
///	セーブデータ不完全型
//=====================================
struct _BATTLEMATCH_DATA
{ 
  LIVEMATCH_DATA  livematch;
  RNDMATCH_DATA   rndmatch;

  u8  mac_address[6];
  u8  is_power_off;
  u8  is_rndmatch;
  s64 prog_sec;
};

static s64 BattleMatch_GetProgTime( void );

//=============================================================================
/**
 *				外部公開
*/
//=============================================================================
//----------------------------------------------------------
//セーブデータシステムに依存する関数
//----------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  ワークサイズ取得
 */
//-----------------------------------------------------------------------------
int BATTLEMATCH_GetWorkSize( void )
{ 
  return sizeof( BATTLEMATCH_DATA );
}
//----------------------------------------------------------------------------
/**
 *	@brief  初期化
 *
 *	@param	BATTLEMATCH_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BATTLEMATCH_Init( BATTLEMATCH_DATA *p_wk )
{ 
  LIVEMATCH_Init( &p_wk->livematch );
  RNDMATCH_Init( &p_wk->rndmatch );

  OS_GetMacAddress( p_wk->mac_address );
}

//----------------------------------------------------------
//セーブデータ取得のための関数
//----------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  ワーク取得  CONST版
 *
 *	@param	SAVE_CONTROL_WORK * p_sv  セーブデータ
 *
 *	@return ワーク取得
 */
//-----------------------------------------------------------------------------
const BATTLEMATCH_DATA * SaveData_GetBattleMatchConst( const SAVE_CONTROL_WORK * cp_sv )
{ 
  return SaveData_GetBattleMatch( (SAVE_CONTROL_WORK*)cp_sv );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ワーク取得
 *
 *	@param	SAVE_CONTROL_WORK * p_sv  セーブデータ
 *
 *	@return ワーク取得
 */
//-----------------------------------------------------------------------------
BATTLEMATCH_DATA * SaveData_GetBattleMatch( SAVE_CONTROL_WORK * p_sv )
{ 
  return SaveControl_DataPtrGet( p_sv , GMDATA_ID_BATTLEMATCH );
}

//----------------------------------------------------------
//大会データからの取得
//----------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  ランダムマッチのデータ取得  CONST
 *
 *	@param	const BATTLEMATCH_DATA * cp_sv  ワーク
 *
 *	@return ランダムマッチのデータを取得
 */
//-----------------------------------------------------------------------------
const RNDMATCH_DATA * BATTLEMATCH_GetRndMatchConst( const BATTLEMATCH_DATA * cp_sv )
{ 
  return &cp_sv->rndmatch;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ランダムマッチのデータ取得
 *
 *	@param	const BATTLEMATCH_DATA * cp_sv  ワーク
 *
 *	@return ランダムマッチのデータを取得
 */
//-----------------------------------------------------------------------------
RNDMATCH_DATA * BATTLEMATCH_GetRndMatch( BATTLEMATCH_DATA * p_sv )
{ 
  return &p_sv->rndmatch;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ライブ大会のデータ取得  CONST版
 *
 *	@param	const LIVEMATCH_DATA * cp_sv  ワーク
 *
 *	@return ライブ大会のデータ取得
 */
//-----------------------------------------------------------------------------
const LIVEMATCH_DATA * BATTLEMATCH_GetLiveMatchConst( const BATTLEMATCH_DATA * cp_sv )
{ 
  return &cp_sv->livematch;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ライブ大会のデータ取得
 *
 *	@param	LIVEMATCH_DATA * p_sv   ワーク
 *
 *	@return ライブ大会のデータ取得
 */
//-----------------------------------------------------------------------------
LIVEMATCH_DATA * BATTLEMATCH_GetLiveMatch( BATTLEMATCH_DATA * p_sv )
{ 
  return &p_sv->livematch;
}

//----------------------------------------------------------------------------
/**
 *	@brief  電源切断チェック開始
 *
 *	@param	BATTLEMATCH_DATA * p_sv ワーク
 */
//-----------------------------------------------------------------------------
void BATTLEMATCH_StartPowerOffCheck( BATTLEMATCH_DATA * p_sv )
{
  OS_GetMacAddress( p_sv->mac_address );
  p_sv->is_power_off = TRUE;

  BATTLEMATCH_SetProgTime( p_sv );

}
//----------------------------------------------------------------------------
/**
 *	@brief  電源切断チェック終了
 *
 *	@param	BATTLEMATCH_DATA * p_sv ワーク
 */
//-----------------------------------------------------------------------------
void BATTLEMATCH_EndPowerOffCheck( BATTLEMATCH_DATA * p_sv )
{
  p_sv->is_power_off = FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  電源切断チェック
 *
 *	@param	const BATTLEMATCH_DATA * cp_sv  ワーク
 *
 *	@return TRUE電源切断  FALSE違う
 */
//-----------------------------------------------------------------------------
BOOL BATTLEMATCH_IsPowerOff( const BATTLEMATCH_DATA * cp_sv )
{
  return cp_sv->is_power_off;
}
//----------------------------------------------------------------------------
/**
 *	@brief  同じDSかチェック
 *
 *	@param	const BATTLEMATCH_DATA * cp_sv  ワーク
 *
 *	@return TRUE同じ  FALSE違う
 */
//-----------------------------------------------------------------------------
BOOL BATTLEMATCH_IsSameDS( const BATTLEMATCH_DATA * cp_sv )
{
  u8 mac_address[6];
  OS_GetMacAddress( mac_address );
  return GFL_STD_MemComp( cp_sv->mac_address, mac_address, 6 ) == 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ペナルティタイム中かチェック
 *
 *	@param	const BATTLEMATCH_DATA * cp_sv ワーク
 *
 *	@return TRUEペナルティタイム終わった  FALSEペナルティタイム中
 */
//-----------------------------------------------------------------------------
BOOL BATTLEMATCH_IsEndPenaltyTime( const BATTLEMATCH_DATA * cp_sv )
{
  static const s64  sc_penalty_time = 60*60*1;  //1時間

  if( BATTLEMATCH_IsPowerOff(cp_sv) )
  {
    if( BattleMatch_GetProgTime() - cp_sv->prog_sec > sc_penalty_time )
    {
      return TRUE;
    }
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  経過時間を設定
 *
 *	@param	BATTLEMATCH_DATA * p_sv ワーク
 */
//-----------------------------------------------------------------------------
void BATTLEMATCH_SetProgTime( BATTLEMATCH_DATA * p_sv )
{
  p_sv->prog_sec = BattleMatch_GetProgTime();
}

//----------------------------------------------------------------------------
/**
 *	@brief  ランダムマッチ参加フラグ設定
 *
 *	@param	BATTLEMATCH_DATA * p_sv ワーク
 *	@param	is_on TRUE 参加 FALSE 参加してない
 */
//-----------------------------------------------------------------------------
void BATTLEMATCH_SetRndmatch( BATTLEMATCH_DATA * p_sv, BOOL is_on )
{
  p_sv->is_rndmatch  = is_on;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ランダムマッチ参加フラグ取得
 *
 *	@param	const BATTLEMATCH_DATA * cp_sv ワーク
 *
 *	@return TRUE参加中  FALSE参加していない
 */
//-----------------------------------------------------------------------------
BOOL BATTLEMATCH_GetRndmatch( const BATTLEMATCH_DATA * cp_sv )
{
  return cp_sv->is_rndmatch;
}
//----------------------------------------------------------------------------
/**
 *	@brief  経過時間を取得
 *
 *	@param	void 
 *
 *	@return 経過時間
 */
//-----------------------------------------------------------------------------
static s64 BattleMatch_GetProgTime( void )
{
  RTCDate date;
  RTCTime time;
  GFL_RTC_GetDateTime( &date, &time );
  return RTC_ConvertDateTimeToSecond(&date, &time) - OS_GetOwnerRtcOffset();
}
