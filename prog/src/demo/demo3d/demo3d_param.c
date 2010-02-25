/*
 *  @file   demo3d_param.c(どこから呼ばれるか不定なので常駐)
 *  @brief  3Dデモ呼び出しパラメータセット
 *
 *  @author Miyuki Iwasawa
 *  @date   10.02.25
 */
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "gamesystem/game_data.h"
#include "field/ev_time.h"
#include "gamesystem/pm_season.h"

#include "demo/demo3d.h"

/*
 *  @brief  demo3d呼び出しパラメータセット
 *
 *  @param  pp      DEMO3D_PARAM型構造体へのポインタ
 *  @param  demo_id 呼び出すデモID DEMO3D_ID_NULL～ include/demo/demo3d_demoid.h 
 *  @param  s_frame デモのスタートフレーム指定
 *  @param  t_hour  デモのライト設定に使う時間指定(時)
 *  @param  t_min   デモのライト設定に使う時間指定(分)
 *  @param  season  デモのライト設定に使う季節 PMSEASON_SPRING～ include/gamesystem/pm_season.h
 */
void DEMO3D_PARAM_Set( DEMO3D_PARAM* pp, u8 demo_id, u32 s_frame, u8 t_hour, u8 t_min, u8 season )
{
  MI_CpuClear8( pp, sizeof(DEMO3D_PARAM));

  pp->demo_id = demo_id;
  pp->start_frame = s_frame;
  pp->hour = t_hour;
  pp->min = t_min;
  pp->season = season;
}

/*
 *  @brief  demo3d呼び出しパラメータセット(RTC依存)
 *
 *  @param  pp      DEMO3D_PARAM型構造体へのポインタ
 *  @param  demo_id 呼び出すデモID DEMO3D_ID_NULL～ include/demo/demo3d_demoid.h 
 *  @param  s_frame デモのスタートフレーム指定
 *
 *  デモのライト設定に使う時間・季節をリアルタイムクロックから算出します
 */
void DEMO3D_PARAM_SetFromRTC( DEMO3D_PARAM* pp, u8 demo_id, u32 s_frame )
{
  RTCTime time;
  GFL_RTC_GetTime(&time);

  DEMO3D_PARAM_Set( pp, demo_id, s_frame, time.hour, time.minute, PMSEASON_GetRealTimeSeasonID() );
}

/*
 *  @brief  demo3d呼び出しパラメータセット(EV-TIME依存)
 *
 *  @param  pp      DEMO3D_PARAM型構造体へのポインタ
 *  @param  demo_id 呼び出すデモID DEMO3D_ID_NULL～ include/demo/demo3d_demoid.h 
 *  @param  s_frame デモのスタートフレーム指定
 *  @param  gdata   GAMEDATAの参照ポインタ
 *
 *  デモのライト設定に使う時間・季節をEV-TIMEから算出します
 */
void DEMO3D_PARAM_SetFromEvTime( DEMO3D_PARAM* pp, u8 demo_id, u32 s_frame, const GAMEDATA* gdata )
{
  DEMO3D_PARAM_Set( pp, demo_id, s_frame,
      EVTIME_GetHour(gdata), EVTIME_GetMinute( gdata ), GAMEDATA_GetSeasonID(gdata) );
}

