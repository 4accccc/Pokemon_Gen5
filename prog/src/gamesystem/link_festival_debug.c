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

#endif  //PM_DEBUG

