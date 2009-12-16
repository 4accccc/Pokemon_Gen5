//============================================================================================
/**
 * @file	pm_season.h
 * @brief	‹Gß‚Ì’è‹`
 * @author	tamada GAME FREAK Inc.
 * @date	09.03.17
 *
 */
//============================================================================================

#include <gflib.h>
#include "gamesystem/pm_season.h"

//--------------------------------------------------------------------------------------------
/**
 * @brief Œ»İ‚Ì‹Gß‚ğŒvZ‚·‚é
 *
 * @param start_date ƒQ[ƒ€ŠJn“ú[Œ/“ú]
 * 
 * @return Œ»İ‚Ì‹Gß( PMSEASON_SPRING ‚È‚Ç )
 */
//--------------------------------------------------------------------------------------------
u8 PMSEASON_CalcSeasonID_byDate( const RTCDate* start_date )
{
  RTCDate now_date;
  u8 season; 

  GFL_RTC_GetDate( &now_date );
  season = (now_date.month + 12 - start_date->month) % PMSEASON_TOTAL;
  return season;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief Œ»İ‚Ì‹Gß‚ğŒvZ‚·‚é
 *
 * @param start_sec ƒQ[ƒ€ŠJn[sec]
 * 
 * @return Œ»İ‚Ì‹Gß( PMSEASON_SPRING ‚È‚Ç )
 */
//--------------------------------------------------------------------------------------------
u8 PMSEASON_CalcSeasonID_bySec( s64 start_sec )
{
  RTCDate start_date;
  RTCTime start_time;

  // ŠJn[sec]‚ğ “ú, ŠÔ‚É•ÏŠ·
  RTC_ConvertSecondToDateTime( &start_date, &start_time, start_sec );

  // ‹Gß‚ğŒvZ
  return PMSEASON_CalcSeasonID_byDate( &start_date );
} 
