//============================================================================================
/**
 * @file	pm_season.h
 * @brief	季節の定義
 * @author	tamada GAME FREAK Inc.
 * @date	09.03.17
 *
 */
//============================================================================================

#include <gflib.h>
#include "gamesystem/pm_season.h"
#include "gamesystem/game_data.h"
#include "gamesystem/pm_season_local.h"
#include "field/field_comm/intrude_work.h"



//----------------------------------------------------------------------------
/**
 *	@brief  通信（進入）に対応した季節を取得
 *
 *	@param	gamedata  ゲームデータ
 *
 *	@return 通信（進入）に対応した季節を取得
 *
 *	＊使用箇所
 *	  エリアデータ－－－－－－－地形にかかわる情報
 *	  フィールドエフェクト－－－地面にかかわる演出
 */
//-----------------------------------------------------------------------------
u8 PMSEASON_GetConsiderCommSeason( GAMESYS_WORK* gamesys )
{
  GAME_COMM_SYS_PTR gamecomm = GAMESYSTEM_GetGameCommSysPtr( gamesys );
  return Intrude_GetSeasonID( gamecomm );
}


//----------------------------------------------------------------------------
/**
 *	@brief  季節、次の季節の取得
 *
 *	@param	gamedata     ゲームデータ
 *	@param  prevSeason  今の季節　格納先
 *	@param  nextSeason  次の季節　格納先
 *
 *	@retval TRUE    変更あり
 *	@retval FALSE   変更なし
 */
//-----------------------------------------------------------------------------
BOOL PMSEASON_GetNextSeason( const GAMEDATA* gamedata, u16* prevSeason, u16* nextSeason )
{
  // 進入中は、季節の更新を行わない。
  if( GAMEDATA_GetIntrudeReverseArea( gamedata ) ){
    *prevSeason = GAMEDATA_GetSeasonID( gamedata );
    *nextSeason = *prevSeason;
    return FALSE;
  }
  
  // その他の場合は、季節変更
  *prevSeason = GAMEDATA_GetSeasonID( gamedata );
  *nextSeason = PMSEASON_GetRealTimeSeasonID();


  if( (*prevSeason) == (*nextSeason) ){
    return FALSE;
  }
  return TRUE;
}



//--------------------------------------------------------------------------------------------
/**
 * @brief 現在時刻の季節を計算する
 *
 * @return 現在時刻の季節( PMSEASON_SPRING など )
 */
//--------------------------------------------------------------------------------------------
u8 PMSEASON_GetRealTimeSeasonID()
{
  RTCDate date;
  u8 season;

  // 現在時刻を取得
  GFL_RTC_GetDate( &date );

  // 季節を計算
  season = (date.month - 1) % PMSEASON_TOTAL;
  return season;
} 

//--------------------------------------------------------------------------------------------
/**
 * @brief 次の季節を求める
 *
 * @param now 現在の季節( PMSEASON_SPRING など )
 *
 * @return 次の季節( PMSEASON_SPRING など )
 */
//--------------------------------------------------------------------------------------------
u8 PMSEASON_GetNextSeasonID( u8 now )
{
  u8 next;
  next = (now + 1) % PMSEASON_TOTAL;
  return next;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief 前の季節を求める
 *
 * @param now 現在の季節( PMSEASON_SPRING など )
 *
 * @return 前の季節( PMSEASON_SPRING など )
 */
//--------------------------------------------------------------------------------------------
u8 PMSEASON_GetPrevSeasonID( u8 now )
{
  u8 prev;
  prev = (now + PMSEASON_TOTAL - 1) % PMSEASON_TOTAL;
  return prev;
}


//----------------------------------------------------------------------------
/**
 *	@brief  季節を設定
 *
 *	@param	gamedata    ゲームデータ
 *	@param  season      次の季節
 */
//-----------------------------------------------------------------------------
void PMSEASON_SetSeasonID( GAMEDATA* gamedata, u16 season )
{
  GAMEDATA_SetSeasonID( gamedata, season );
}

