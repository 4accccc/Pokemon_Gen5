//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		local_tvt_util.c
 *	@brief  ローカルライブキャスター汎用モジュール
 *	@author	Toru=Nagihashi
 *	@data		2011.01.11
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/gfl_use.h"
#include "system/ds_system.h"
#include "system/rtc_tool.h"

//アーカイブ
#include "arc_def.h"
#include "comm_tvt.naix"

//自分のモジュール
#include "local_tvt_graphic.h"

//外部公開
#include "local_tvt_util.h"

//=============================================================================
/**
 *					タイムゾーン
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  タイムゾーンを取得
 *
 *	@param	const GAMEDATA *cp_gamedata ゲームデータ
 *
 *	@return タイムゾーン
 */
//-----------------------------------------------------------------------------
LOCAL_TVT_TIME_ZONE LOCAL_TVT_UTIL_GetTimeZone( const GAMEDATA *cp_gamedata )
{
  RTCTime time;
  const u8 season = GAMEDATA_GetSeasonID( cp_gamedata );
  const int morning = PM_RTC_GetTimeZoneChangeHour( season , TIMEZONE_MORNING );
  const int evening = PM_RTC_GetTimeZoneChangeHour( season , TIMEZONE_EVENING );
  const int night   = PM_RTC_GetTimeZoneChangeHour( season , TIMEZONE_NIGHT );

  GFL_RTC_GetTime( &time );

  if( time.hour >= morning && time.hour < evening )
  {
    return LTTZ_NOON;
  }
  else if( time.hour >= evening && time.hour < night )
  {
    return LTTZ_EVENING;
  }
  else
  {
    return LTTZ_NIGHT;
  }
}
//=============================================================================
/**
 *					背景素材
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  背景素材読込み
 *
 *	@param	const GAMEDATA *p_gamedata   ゲームデータ
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_UTIL_LoadBg( GAMEDATA *p_gamedata, HEAPID heapID )
{
  MYSTATUS *myStatus = GAMEDATA_GetMyStatus( p_gamedata );
  ARCHANDLE *ctvtArcHandle = GFL_ARC_OpenDataHandle( ARCID_COMM_TVT_GRA, heapID );

    if( MyStatus_GetMySex( myStatus ) == PM_MALE )
    {
      GFL_ARCHDL_UTIL_TransVramPalette( ctvtArcHandle , NARC_comm_tvt_tv_t_tuuwa_bg2_NCLR , 
          PALTYPE_SUB_BG , 0*32 , 32*3 , heapID );
    }
    else
    {
      GFL_ARCHDL_UTIL_TransVramPalette( ctvtArcHandle , NARC_comm_tvt_tv_t_tuuwa_bg_NCLR , 
          PALTYPE_SUB_BG , 0*32 , 32*3 , heapID );
    }
    GFL_ARCHDL_UTIL_TransVramBgCharacter( ctvtArcHandle , NARC_comm_tvt_tv_t_tuuwa_bg_NCGR ,
        LTVT_FRAME_SUB_BG , 0 , 0, FALSE , heapID );
    GFL_ARCHDL_UTIL_TransVramScreen( ctvtArcHandle , NARC_comm_tvt_tv_t_common_bg_NSCR , 
        LTVT_FRAME_SUB_BG ,  0 , 0, FALSE , heapID );
    GFL_BG_LoadScreenReq( LTVT_FRAME_SUB_BG );

    GFL_ARCHDL_UTIL_TransVramPalette( ctvtArcHandle , NARC_comm_tvt_noise_bg_NCLR , 
        PALTYPE_MAIN_BG , LTVT_PLT_MAIN_NOIZE*32 , 32*1 , heapID );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( ctvtArcHandle , NARC_comm_tvt_noise_bg_NCGR ,
        LTVT_FRAME_NOIZE_BG , 0 , 0, FALSE , heapID );
    GFL_ARCHDL_UTIL_TransVramScreen( ctvtArcHandle , NARC_comm_tvt_noise_bg_NSCR , 
        LTVT_FRAME_NOIZE_BG ,  0 , 0, FALSE , heapID );
    GFL_BG_ChangeScreenPalette( LTVT_FRAME_NOIZE_BG , 0,0,32,32,LTVT_PLT_MAIN_NOIZE );
    GFL_BG_LoadScreenReq( LTVT_FRAME_NOIZE_BG );

    GFL_ARC_CloseDataHandle( ctvtArcHandle );

}
