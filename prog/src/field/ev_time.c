//============================================================================================
/**
 * @file  ev_time.c
 * @brief ゲーム内時間制御関連
 * @author  tamada
 * @date  2006.02.03
 */
//============================================================================================
#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/playerwork.h"
#include "gamesystem/game_event.h"
#include "system/main.h"    //HEAPID_FIELDMAP
#include "system/rtc_tool.h"    //HEAPID_FIELDMAP

#include "field/zonedata.h"
#include "fieldmap/zone_id.h"

#include "fieldmap.h"
#include "event_debug_menu.h"     //DEBUG_EVENT_DebugMenu
#include "script.h"     //SCRIPT_SetEventScript
#include "isdbglib.h"
#include "map_attr.h"

#include "../../../resource/fldmapdata/flagwork/flag_define.h"

#include "savedata/encount_sv.h"
#include "field_encount.h"      //FIELD_ENCOUNT_CheckEncount
#include "effect_encount.h"

#include "debug/debug_flg.h" //DEBUG_FLG_～
#include "ev_time.h"

#include "savedata/gametime.h"
#include "savedata/encount_sv.h"
#include "savedata/wifihistory.h"

#include "poke_tool/pokerus.h"
#include "field/eventwork.h"

#include "sheimi_normalform.h"

#include "savedata/irc_compatible_savedata.h"

//============================================================================================
//============================================================================================
//============================================================================================
//============================================================================================
static void UpdateDayEvent(GAMEDATA * gdata, s32 diff_days);
static void UpdateMinuteEvent(GAMEDATA * gdata, s32 diff_minute, const RTCTime * now);

static BOOL changeTimeCalc( GMTIME * tm,
    const RTCDate * now_date, const RTCTime * now_time, s32 * diff_day, s32 * diff_minute );
#if 0
static void UpdateDateCheck(GAMEDATA * gdata, GMTIME * tm, const RTCDate * now_date);
static void UpdateMinuteCheck(GAMEDATA * gdata, GMTIME * tm,
    const RTCDate * now_date, const RTCTime * now_time);
#endif

//============================================================================================
//============================================================================================
#ifdef  DEBUG_ONLY_FOR_tamada
static void DEBUG_DATE_Print( const RTCDate * date, const RTCTime * time )
{
  TAMADA_Printf("20%02d %02d/%02d(%d) %02d:%02d:%02d\n",
      date->year, date->month, date->day, date->week,
      time->hour, time->minute, time->second );
}

static void DEBUG_All_Printf( GMTIME * tm, const RTCDate * now_date, const RTCTime * now_time )
{
  TAMADA_Printf("SAVE (%04d) ", RTC_ConvertDateToDay(&tm->sv_date) );
  DEBUG_DATE_Print( &tm->sv_date, &tm->sv_time );
  TAMADA_Printf("NOW  (%04d) ", RTC_ConvertDateToDay(now_date) );
  DEBUG_DATE_Print( now_date, now_time );
}
#else
#define DEBUG_All_Printf(...)   ((void)0)
#endif  //DEBUG_ONLY_FOR_tamada
//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
void EVTIME_Update(GAMEDATA * gdata)
{
  RTCDate now_date;
  RTCTime now_time;
  GMTIME * tm = SaveData_GetGameTime( GAMEDATA_GetSaveControlWork( gdata ) );
  s32 diff_day, diff_minute;

  if (tm->use_flag == FALSE) {
    return;
  }

  GFL_RTC_GetDateTime(&now_date, &now_time);

  if ( changeTimeCalc( tm, &now_date, &now_time, &diff_day, &diff_minute ) == FALSE )
  { //時間経過に異常があった場合はスルー
    return;
  }
  if ( diff_minute == 0 && diff_day != 0 )
  { //分が進んでいないのに日が進んだらアサート
    GF_ASSERT( 0 );
    return;
  }

  if ( diff_minute )
  {
    GMTIME_CountDownPenaltyTime( tm, diff_minute );
    UpdateMinuteEvent( gdata, diff_minute, &now_time );
  }
  if ( diff_day )
  {
    UpdateDayEvent( gdata, diff_day ); 
  }
}

//------------------------------------------------------------------
/**
 * @brief 経過時間の算出処理
 * @param tm            時間経過管理ワーク
 * @param now_date      現在の日付
 * @param now_time      現在の時刻
 * @param diff_day      過去から現在への経過時間（日単位）
 * @param diff_minute   過去から現在への経過時間（分単位）
 * @return  BOOL  正常取得されたかどうか（TRUE=正常、FALSE=は異常なので更新処理を実行しない）
 *
 * @todo  now_timeの秒を０に切り捨てて演算することでRTCと時間更新の誤差を
 * 初回以外１秒未満にすることが可能（だけど検証時間が足りないのでやらない）
 */
//------------------------------------------------------------------
static BOOL changeTimeCalc( GMTIME * tm,
    const RTCDate * now_date, const RTCTime * now_time, s32 * diff_day, s32 * diff_minute )
{
  s64  sv_sec = RTC_ConvertDateTimeToSecond( &tm->sv_date, &tm->sv_time );
  s64 now_sec = RTC_ConvertDateTimeToSecond( now_date, now_time );
  s32 now_day = RTC_ConvertDateToDay( now_date );
  s32 diff;

  *diff_day = 0;
  *diff_minute = 0;

  if ( now_sec < sv_sec ) {
    //現在　＜　過去　…ありえないはず
    //現在時間をセットするだけで戻る
    DEBUG_All_Printf( tm, now_date, now_time );
    TAMADA_Printf(" Reverse Time!!\n" );
    tm->sv_date = *now_date;
    tm->sv_time = *now_time;
    tm->sv_day  = now_day;
    return FALSE;
  }

  diff = ( now_sec - sv_sec ) / 60;
  if (diff > 0) {
    //分の更新があった場合
    *diff_minute = diff;
    DEBUG_All_Printf( tm, now_date, now_time );
    TAMADA_Printf(" Update Minute %d\n", *diff_minute );
    tm->sv_date = *now_date;
    tm->sv_time = *now_time;

    //分の更新があった時だけ、日付の更新チェックを行う
    if ( now_day - tm->sv_day > 0 )
    {
      *diff_day = now_day - tm->sv_day;
      tm->sv_day = now_day;
      TAMADA_Printf(" Update Day %d\n", *diff_day );
    }
  }
  return TRUE;
}

#if 0
//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
void EVTIME_Update(GAMEDATA * gdata)
{
  RTCDate now_date;
  RTCTime now_time;
  GMTIME * tm = SaveData_GetGameTime( GAMEDATA_GetSaveControlWork( gdata ) );
  s32 diff_day, diff_minute;

  if (tm->use_flag == FALSE) {
    return;
  }

  GFL_RTC_GetDateTime(&now_date, &now_time);

  UpdateDateCheck(gdata, tm, &now_date);
  UpdateMinuteCheck(gdata, tm, &now_date, &now_time);
}

//------------------------------------------------------------------
///日数経過を見る
//------------------------------------------------------------------
static void UpdateDateCheck(GAMEDATA * gdata, GMTIME * tm, const RTCDate * now_date)
{
  s32 now_day;
  now_day = RTC_ConvertDateToDay(now_date);

  if (now_day < tm->sv_day) {
    //現在　＜　過去　…ありえないはず
    //現在時間をセットするだけで戻る
    tm->sv_day = now_day;
  }
  else if (now_day > tm->sv_day) {
    UpdateDayEvent(gdata, now_day - tm->sv_day);
    tm->sv_day = now_day;
  }
}

//------------------------------------------------------------------
///秒数経過を見る
//------------------------------------------------------------------
static void UpdateMinuteCheck(GAMEDATA * gdata, GMTIME * tm,
    const RTCDate * now_date, const RTCTime * now_time)
{
  s64 now, sv;
  s32 diff_sec,diff_minute;
  now = RTC_ConvertDateTimeToSecond(now_date, now_time);
  sv = RTC_ConvertDateTimeToSecond(&tm->sv_date, &tm->sv_time);
  if (now < sv) {
    //現在　＜　過去　…ありえないはず
    //現在時間をセットするだけで戻る
    tm->sv_date = *now_date;
    tm->sv_time = *now_time;
  }
  else{
    //秒
    diff_sec = (now-sv);
    //分
    diff_minute = diff_sec / 60;
    if (diff_minute > 0) {
      GMTIME_CountDownPenaltyTime(tm, diff_minute);
      UpdateMinuteEvent(gdata, diff_minute, now_time);

      tm->sv_date = *now_date;
      tm->sv_time = *now_time;
    }
  }
}
#endif

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief 時間によるデータ更新：日数単位
 * @param gdata     フィールド制御ワークへのポインタ
 * @param diff_days   経過した時間（日数）
 *
 * 引数にGAMEDATAをとっているが、基本的にはSAVEDATAのみで
 * 更新できるような仕組みの関数を呼ぶべき。
 */
//------------------------------------------------------------------
static void UpdateDayEvent(GAMEDATA * gdata, s32 diff_days)
{
  BOOL is_penalty = EVTIME_IsPenaltyMode(gdata);
  SAVE_CONTROL_WORK* save = GAMEDATA_GetSaveControlWork( (GAMEDATA*)gdata );

  TAMADA_Printf("Update Day Event\n");
  //下記のような感じで時間関連イベント更新処理を追加する
  //
  //TVData_Update(gdata, diff_days);
  //ZukanHyouka_Update
  //…

  //1日毎にクリアするフラグ
  EVENTWORK_ClearTimeFlags( GAMEDATA_GetEventWork( gdata ) );
//  TimeEventFlagClear(gdata);

  // 乱数の種グループ更新
//  RandomGroup_Update(SaveData_GetRandomGroup(gdata->savedata), diff_days);

  //エンカウント関連ランダムの種更新
  EncDataSave_UpdateGenerate( save );

  //ポケルス感染の経過を見る
  {
    POKEPARTY *ppt;

    ppt = GAMEDATA_GetMyPokemon( gdata );
    POKERUS_DecCounter( ppt, diff_days );
  }

  // レコードデータ1日1回更新処理
  RECORD_1day_Update( GAMEDATA_GetRecordPtr(gdata) );

  // ジオネット1日1回更新処理
  WIFIHISTORY_Update(SaveData_GetWifiHistory(save));

  //ポケモンクジ
  {
//    SysWorkUpdatePokeLot(gdata->savedata, diff_days);
  }

  // WFBC人物、アイテム
  {
    // HEAPID_PROC　テンポラリとして使用
    FIELD_WFBC_CORE_CalcOneDataStart( gdata, diff_days, HEAPID_PROC );
  }

  //WIFI通信履歴データの更新処理
//  WIFIHISTORY_Update(SaveData_GetWifiHistory(gdata->savedata));

  //調査レーダー：本日のすれ違い人数を合計に移動
  QuestionnaireWork_DateChangeUpdate( SaveData_GetQuestionnaire(save) );

  //フィーリングチェック：１日１回チェックを消す
  IRC_COMPATIBLE_SV_ClearDayFlag( IRC_COMPATIBLE_SV_GetSavedata(save) );
  
  if(!is_penalty){  //ペナルティタイムでないときだけ処理する
  }
}

//------------------------------------------------------------------
/**
 * @brief 時間によるデータ更新：分単位
 * @param gdata     フィールド制御ワークへのポインタ
 * @param diff_minute   経過した時間（分単位）
 *
 * 引数にGAMEDATAをとっているが、基本的にはSAVEDATAのみで
 * 更新できるような仕組みの関数を呼ぶべき。
 */
//------------------------------------------------------------------
static void UpdateMinuteEvent(GAMEDATA * gdata, s32 diff_minute, const RTCTime * now)
{  
  //夜になった(経過した)ならば変わった手持ちシェイミのフォルムを戻す
  {
    POKEPARTY *ppt;
    int season;
    SAVE_CONTROL_WORK* sv = GAMEDATA_GetSaveControlWork(gdata);
    ppt = SaveData_GetTemotiPokemon(sv);
    season = GAMEDATA_GetSeasonID(gdata);
    SHEIMI_NFORM_ChangeNormal_TimeUpdate(gdata, ppt, diff_minute, now, season);
  }  

  // 電光掲示板のチャンピオンニュース表示残り時間をデクリメント
  {
    SAVE_CONTROL_WORK* sv = GAMEDATA_GetSaveControlWork(gdata);
    MISC* misc = SaveData_GetMisc( sv );
    MISC_DecChampNewsMinutes( misc, diff_minute );
  }
}




//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief   イベント時間：RTCDateの取得
 * @param gdata フィールド制御ワークへのポインタ
 * @return  RTCDateへのポインタ
 */
//------------------------------------------------------------------
const RTCDate * EVTIME_GetRTCDate( const GAMEDATA * gdata )
{
  const GMTIME * tm = SaveData_GetGameTime( GAMEDATA_GetSaveControlWork( (GAMEDATA*)gdata ) );
  return &tm->sv_date;
}

//------------------------------------------------------------------
/**
 * @brief 時間帯の取得
 * @param gdata フィールド制御ワークへのポインタ
 * @return  int   時間帯（field/timezone.hを参照）
 *
 *デフォルト季節の時間帯を取得します。
 * 
 * EVTIME_GetTimeZoneを使用するか、
 * EVTIME_GetSeasonTimeZoneを使用するかは、企画者と相談してください。
 */
//------------------------------------------------------------------
int EVTIME_GetTimeZone(const GAMEDATA * gdata)
{
  GMTIME * tm = SaveData_GetGameTime( GAMEDATA_GetSaveControlWork( (GAMEDATA*)gdata ) );
  return PM_RTC_ConvertHourToTimeZone( PM_RTC_TIMEZONE_DEFAULT_SEASON, tm->sv_time.hour);
}

//------------------------------------------------------------------
/**
 * @brief 時間帯の取得
 * @param gdata フィールド制御ワークへのポインタ
 * @return  int   時間帯（field/timezone.hを参照）
 *
 * 季節の時間帯になります。
 *
 * EVTIME_GetTimeZoneを使用するか、
 * EVTIME_GetSeasonTimeZoneを使用するかは、企画者と相談してください。
 */
//------------------------------------------------------------------
int EVTIME_GetSeasonTimeZone(const GAMEDATA * gdata)
{
  GMTIME * tm = SaveData_GetGameTime( GAMEDATA_GetSaveControlWork( (GAMEDATA*)gdata ) );
  u16 season = GAMEDATA_GetSeasonID( gdata );
  return PM_RTC_ConvertHourToTimeZone( season, tm->sv_time.hour);

}

//------------------------------------------------------------------
/**
 * @brief イベント時間（月）の取得
 * @param gdata フィールド制御ワークへのポインタ
 * @return  int   月
 */
//------------------------------------------------------------------
int EVTIME_GetMonth(const GAMEDATA * gdata)
{
  GMTIME * tm = SaveData_GetGameTime( GAMEDATA_GetSaveControlWork( (GAMEDATA*)gdata ) );
  return tm->sv_date.month;
}
//------------------------------------------------------------------
/**
 * @brief イベント時間（日）の取得
 * @param gdata フィールド制御ワークへのポインタ
 * @return  int   日
 */
//------------------------------------------------------------------
int EVTIME_GetDay(const GAMEDATA * gdata)
{
  GMTIME * tm = SaveData_GetGameTime( GAMEDATA_GetSaveControlWork( (GAMEDATA*)gdata ) );
  return tm->sv_date.day;
}
//------------------------------------------------------------------
/**
 * @brief イベント時間（曜日）の取得
 * @param gdata フィールド制御ワークへのポインタ
 * @return  int   曜日
 */
//------------------------------------------------------------------
int EVTIME_GetWeek(const GAMEDATA * gdata)
{
  GMTIME * tm = SaveData_GetGameTime( GAMEDATA_GetSaveControlWork( (GAMEDATA*)gdata ) );
  return tm->sv_date.week;
}
//------------------------------------------------------------------
/**
 * @brief イベント時間（時）の取得
 * @param gdata フィールド制御ワークへのポインタ
 * @return  int   時間
 */
//------------------------------------------------------------------
int EVTIME_GetHour(const GAMEDATA * gdata)
{
  GMTIME * tm = SaveData_GetGameTime( GAMEDATA_GetSaveControlWork( (GAMEDATA*)gdata ) );
  return tm->sv_time.hour;
}
//------------------------------------------------------------------
/**
 * @brief イベント時間（分）の取得
 * @param gdata フィールド制御ワークへのポインタ
 * @return  int   分
 */
//------------------------------------------------------------------
int EVTIME_GetMinute(const GAMEDATA * gdata)
{
  GMTIME * tm = SaveData_GetGameTime( GAMEDATA_GetSaveControlWork( (GAMEDATA*)gdata ) );
  return tm->sv_time.minute;
}


//------------------------------------------------------------------
/**
 * @brief ゲーム開始の日付＆時間取得
 * @param gdata フィールド制御ワークへのポインタ
 * @param date  ゲーム開始の日付を受け取るためのRTCDate型へのポインタ
 * @param time  ゲーム開始の時間を受け取るためのRTCTime型へのポインタ
 */
//------------------------------------------------------------------
void EVTIME_GetGameStartDateTime(const GAMEDATA * gdata, RTCDate * date, RTCTime * time)
{
  GMTIME * tm = SaveData_GetGameTime( GAMEDATA_GetSaveControlWork( (GAMEDATA*)gdata ) );
  RTC_ConvertSecondToDateTime(date, time, tm->start_sec);
}

//------------------------------------------------------------------
/**
 * @brief ゲームクリアの日付＆時間取得
 * @param gdata フィールド制御ワークへのポインタ
 * @param date  ゲーム開始の日付を受け取るためのRTCDate型へのポインタ
 * @param time  ゲーム開始の時間を受け取るためのRTCTime型へのポインタ
 */
//------------------------------------------------------------------
void EVTIME_GetGameClearDateTime(const GAMEDATA * gdata, RTCDate * date, RTCTime * time)
{
  GMTIME * tm = SaveData_GetGameTime( GAMEDATA_GetSaveControlWork( (GAMEDATA*)gdata ) );
  RTC_ConvertSecondToDateTime(date, time, tm->clear_sec);
}

//------------------------------------------------------------------
/**
 * @brief ゲームクリアの日付＆時間セット
 * @param gdata フィールド制御ワークへのポインタ
 */
//------------------------------------------------------------------
void EVTIME_SetGameClearDateTime(const GAMEDATA * gdata)
{
  GMTIME * tm = SaveData_GetGameTime( GAMEDATA_GetSaveControlWork( (GAMEDATA*)gdata ) );
  tm->clear_sec = GFL_RTC_GetDateTimeBySecond();

}

//------------------------------------------------------------------
/**
 * @brief DS設定変更のペナルティモードかどうかの判定
 * @param gdata フィールド制御ワークへのポインタ
 * @return  BOOL  TRUEのとき、ペナルティ中
 */
//------------------------------------------------------------------
BOOL EVTIME_IsPenaltyMode(GAMEDATA * gdata)
{
  GMTIME * tm = SaveData_GetGameTime( GAMEDATA_GetSaveControlWork( gdata ) );
  return GMTIME_IsPenaltyMode(tm);
}


