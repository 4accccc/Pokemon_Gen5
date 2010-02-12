//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_season_time.c
 *	@brief  季節による　時間帯変化
 *	@author	tomoya takahashi
 *	@date		2010.01.28
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "field_season_time.h"

#include "system/rtc_tool.h"

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
///	季節による時間帯変化管理ワーク
//=====================================
struct _FLD_SEASON_TIME 
{
  u32 dummy;  // @TODO 後は季節ごとの時間帯切り替え時間
};

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------


//----------------------------------------------------------------------------
/**
 *	@brief  季節の時間帯管理ワーク　生成
 *
 *	@param	season_id   季節ID
 *	@param	heapID      ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
FLD_SEASON_TIME* FLD_SEASON_TIME_Create( u8 season_id, HEAPID heapID )
{
  FLD_SEASON_TIME* p_sys = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLD_SEASON_TIME) );
  return p_sys;
}

//----------------------------------------------------------------------------
/**
 *	@brief  季節の時間帯管理ワーク　破棄
 *
 *	@param	p_sys   システム
 */
//-----------------------------------------------------------------------------
void FLD_SEASON_TIME_Delete( FLD_SEASON_TIME* p_sys )
{
  GFL_HEAP_FreeMemory( p_sys );
}



//----------------------------------------------------------------------------
/**
 *	@brief  季節ごとの時間帯取得
 *
 *	@param	cp_sys  システム
 *
 *	@return 時間帯
 */
//-----------------------------------------------------------------------------
TIMEZONE FLD_SEASON_TIME_GetTimeZone( const FLD_SEASON_TIME* cp_sys )
{
  GF_ASSERT( cp_sys );
  return GFL_RTC_GetTimeZone();
}

// 今の時間からタイムゾーン切り替え時間までの差
//----------------------------------------------------------------------------
/**
 *	@brief  今の時間からタイムゾーン切り替えまでの差
 *
 *	@param	cp_sys    システム
 *	@param	timezone  タイムゾーン
 *
 *	@return　秒数(-12時間～11時間)
 */
//-----------------------------------------------------------------------------
int FLD_SEASON_TIME_GetTimeZoneChangeTime( const FLD_SEASON_TIME* cp_sys, TIMEZONE timezone )
{
  static const u8 sc_TIMEZONE_CHANGE_TIME[ 5 ] = 
  {
    4,
    10,
    17,
    20,
    0
  };
  int time_second;
  int sub_second;
  int change_time;
  RTCTime time;

  
  GF_ASSERT( cp_sys );
  GF_ASSERT( timezone < TIMEZONE_MAX );
  
  GFL_RTC_GetTime( &time );

  change_time = sc_TIMEZONE_CHANGE_TIME[timezone];
  time_second = (time.hour * 60 * 60) + (time.minute * 60) + time.second;
  sub_second = (change_time*60*60) - time_second;
  if( sub_second > (12*60*60) )
  {
    sub_second = (24*60*60) - sub_second;
  }
  return sub_second;
}

