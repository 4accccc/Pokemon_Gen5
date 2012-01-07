//==============================================================================
/**
 * @file    link_festival_debug.h
 * @brief   リンクフェスティバル デバッグルーチン SRCS_OVERLAY_D_LINKFEST
 * @author  iwasawa
 * @date    2010.12.24
 */
//==============================================================================

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/rtc_tool.h"
#include "field/gpower_id.h"
#include "gamesystem/g_power.h"
#include "gamesystem/gamesystem.h"
#include "waza_tool/wazano_def.h"
#include "arc_def.h"
#include "power.naix"
#include "poke_tool/status_rcv.h"

#include "gamesystem/link_festival.h"
#include "gamesystem/link_festival_local.h"

#ifdef PM_DEBUG

/*
 *  @brief  ビーコン送信インターバル取得
 */
int DEB_LINKFEST_GetBeaconWait( LINK_FESTIVAL* wk )
{
  return wk->beacon_wait;
}

/*
 *  @brief  ビーコン送信インターバル設定
 */
void DEB_LINKFEST_SetBeaconWait( LINK_FESTIVAL* wk, int wait )
{
  wk->beacon_wait = wait;
  wk->beacon_timer = wait;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ライフのデクリメントをとめるフラグをセットする
 */
//---------------------------------------------------------
void DEB_LINKFEST_SetTimeStopFlag( LINK_FESTIVAL* wk, BOOL flag )
{
  s64 now_rtc = GFL_RTC_GetDateTimeBySecond();
  wk->deb_time_stop_f = flag;

  if( wk->deb_time_stop_f ){
    wk->mission.deb_time_stop_rtc = now_rtc;
  }else{
    s64 diff = wk->mission.deb_time_stop_rtc-wk->mission.start_rtc;
    wk->mission.start_rtc = now_rtc-diff;
    wk->mission.deb_time_stop_rtc = 0;
  }
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ライフのデクリメントをとめるフラグをゲット
 */
//---------------------------------------------------------
BOOL DEB_LINKFEST_GetTimeStopFlag( LINK_FESTIVAL* wk )
{
  return wk->deb_time_stop_f;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル 配信スポットエミュレーションフラグをセットする
 */
//---------------------------------------------------------
void DEB_LINKFEST_SetSpotModeFlag( LINK_FESTIVAL* wk, BOOL flag )
{
  wk->deb_spot_mode_f = flag;

  if( wk->deb_spot_mode_f ){
    wk->mission.beacon.distribution_f = TRUE;
  }else{
    if( wk->mission.arc.host != FMISSION_HOST_COMM ){
      wk->mission.beacon.distribution_f = FALSE;
    }
  }
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル 配信モードエミュレーションフラグをゲット
 */
//---------------------------------------------------------
BOOL DEB_LINKFEST_GetSpotModeFlag( LINK_FESTIVAL* wk )
{
  return wk->deb_spot_mode_f;
}


#endif  //PM_DEBUG

