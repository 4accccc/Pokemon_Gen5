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
 * @return Œ»İ‚Ì‹Gß( PMSEASON_SPRING ‚È‚Ç )
 */
//--------------------------------------------------------------------------------------------
u8 PMSEASON_GetRealTimeSeasonID()
{
  RTCDate date;
  u8 season;

  // Œ»İ‚ğæ“¾
  GFL_RTC_GetDate( &date );

  // ‹Gß‚ğŒvZ
  season = (date.month - 1) % PMSEASON_TOTAL;
  return season;
} 

//--------------------------------------------------------------------------------------------
/**
 * @brief Ÿ‚Ì‹Gß‚ğ‹‚ß‚é
 *
 * @param now Œ»İ‚Ì‹Gß( PMSEASON_SPRING ‚È‚Ç )
 *
 * @return Ÿ‚Ì‹Gß( PMSEASON_SPRING ‚È‚Ç )
 */
//--------------------------------------------------------------------------------------------
extern u8 PMSEASON_GetNextSeasonID( u8 now )
{
  u8 next;
  next = (now + 1) % PMSEASON_TOTAL;
  return next;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief ‘O‚Ì‹Gß‚ğ‹‚ß‚é
 *
 * @param now Œ»İ‚Ì‹Gß( PMSEASON_SPRING ‚È‚Ç )
 *
 * @return ‘O‚Ì‹Gß( PMSEASON_SPRING ‚È‚Ç )
 */
//--------------------------------------------------------------------------------------------
extern u8 PMSEASON_GetPrevSeasonID( u8 now )
{
  u8 prev;
  prev = (now + PMSEASON_TOTAL - 1) % PMSEASON_TOTAL;
  return prev;
}
