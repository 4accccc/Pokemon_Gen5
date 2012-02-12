//==============================================================================
/**
 * @file    fest_mission.c
 * @brief   リンクフェスティバル ミッション管理
 * @author  iwasawa
 * @date    2010.12.16
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

SDK_COMPILER_ASSERT( sizeof(FEST_BEACON) <= 20); //フェスミッション構造体型サイズはmax20byteまで

typedef BOOL (*MISSION_SCORE_CALC_FUNC)( const FEST_MISSION* fmwk, u16 param );


typedef struct _MISSION_SCORE_CALC_PARAM{
  MISSION_SCORE_CALC_FUNC func;
}MISSION_SCORE_CALC_PARAM;


//====================================================
///ミッションタイプ別スコアインクリメントチェック関数
static u32 score_CalcTimeBonus( FEST_MISSION* fmwk );
static BOOL score_CalcWildBtlWin( const FEST_MISSION* fmwk, u16 param );
static BOOL score_CalcCommon( const FEST_MISSION* fmwk, u16 param );

/*
 *  ミッションスコア計算に必要なパラメータテーブル
 */
static const MISSION_SCORE_CALC_PARAM DATA_ScoreCalcParamTbl[] = {
  { NULL },
  { score_CalcWildBtlWin },
  { score_CalcWildBtlWin },
  { score_CalcCommon },
  { score_CalcCommon },
  { score_CalcCommon },
  { score_CalcCommon },
};

//---------------------------------------------------------
/*
 * @brief FEST_MISSION型ワークを生成
 */
//---------------------------------------------------------
FEST_MISSION* FEST_MISSION_AllocWork( HEAPID heapID )
{
  FEST_MISSION* fmwk = GFL_HEAP_AllocClearMemory( heapID, sizeof(FEST_MISSION) );

  FEST_MISSION_Clear( fmwk );
  return fmwk;
}

//---------------------------------------------------------
/*
 * @brief ミッション管理領域クリア
 */
//---------------------------------------------------------
void FEST_MISSION_Clear( FEST_MISSION* fmwk )
{
  FEST_BEACON* beacon = &fmwk->beacon;
  
  MI_CpuClear8( fmwk, sizeof(FEST_MISSION));

  //無効値で初期化
  beacon->goal = FMISSION_GOAL_NULL;
  beacon->m_id = FMISSION_ID_NULL;

  fmwk->start_rtc = FMISSION_RTC_NULL;
}


//---------------------------------------------------------
/*
 * @brief ミッション開始処理
 *
 * LINK_FESTIVAL_MissionStart()の内部からだけ呼び出しOK
 */
//---------------------------------------------------------
void FEST_MISSION_Start( FEST_MISSION* fmwk, const FEST_MISSION* src, BOOL succeed_f )
{
  FEST_BEACON* beacon = &fmwk->beacon;
  FEST_MISSION_Clear( fmwk );

  *fmwk = *src;
  if( !succeed_f ){ //継承ミッションでなければキー生成
    fmwk->key = GFUser_GetPublicRand(GFL_STD_RAND_MAX);  //32bitランダムキー生成
    fmwk->start_rtc = FMISSION_RTC_NULL;
  }else{
    fmwk->my_score = 0; //継承ミッションの場合、スコアクリア
  }

  OS_FPrintf(OSPRINT_LINKFEST_PAGE,"フェスミッション スタート m_id=%d, key = %08x, goal=%d, param=%d, time=%d sec\n",
    beacon->m_id, fmwk->key, beacon->goal, beacon->base_param, beacon->time);
}

//---------------------------------------------------------
/*
 * @brief ミッションプレイ開始処理
 *
 * LINK_FESTIVAL_MissionPlayStart()の内部からだけ呼び出しOK
 */
//---------------------------------------------------------
void FEST_MISSION_PlayStart( FEST_MISSION* fmwk )
{
  FEST_MISSION_SetRTC( fmwk );
}

//---------------------------------------------------------
/*
 * @brief ミッション終了処理
 */
//---------------------------------------------------------
void FEST_MISSION_End( FEST_MISSION* fmwk )
{
  FEST_MISSION_Clear( fmwk );
  OS_FPrintf(OSPRINT_LINKFEST_PAGE,"フェスミッション終了\n");
}

//---------------------------------------------------------
/*
 * @brief ミッションRTCセット
 *
 * RTCパラメータを現パラメータでセット
 */
//---------------------------------------------------------
void FEST_MISSION_SetRTC( FEST_MISSION* fmwk )
{
  fmwk->start_rtc = GFL_RTC_GetDateTimeBySecond();
}

//---------------------------------------------------------
/*
 * @brief ミッションRTC 残り時間取得
 *
 * 現在のRTCを取得し、start_rtcとの差分から残り時間を取得する
 */
//---------------------------------------------------------
u32 FEST_MISSION_GetLife( const FEST_MISSION* fmwk )
{
  s64 now_sec = GFL_RTC_GetDateTimeBySecond();
  s64 diff = 0;

  if( now_sec == FMISSION_RTC_NULL ){
    return 0; //エラー値(= -1)が返ってきたら終了
  }
  if( fmwk->start_rtc == FMISSION_RTC_NULL ){
    return  fmwk->beacon.time;  //スタート時間が未登録の場合制限時間をそのまま返す
  }

#ifdef PM_DEBUG
  if( fmwk->deb_time_stop_rtc ){
    now_sec = fmwk->deb_time_stop_rtc;
  }
#endif
  
  diff = ( now_sec-fmwk->start_rtc );
  
  if( diff < 0 ){
    return 0; //逆転してたら何かおかしいので0を返す
  }
//  IWASAWA_Printf("rtc=%08x%08x, time=%d, diff=%d, ", (u32)(fmwk->start_rtc>>32), (u32)(fmwk->start_rtc&0xFFFFFFFF), fmwk->beacon.time, diff );
  diff = (s64)fmwk->beacon.time - diff;
//  IWASAWA_Printf("life=%d\n",diff);
  if( diff < 0 ){
    return 0;
  }
  return (u32)diff;
}

//---------------------------------------------------------
/*
 * @brief ミッションRTC エントリーに十分な残り時間が残っているか？
 *
 * 現在のRTCを取得し、start_rtcとの差分から残り時間を取得
 * エントリー条件に十分な残り時間があるかを返す
 */
//---------------------------------------------------------
BOOL FEST_MISSION_IsLifeLeftForEntry( const FEST_MISSION* fmwk )
{
  u32 life = FEST_MISSION_GetLife( fmwk );

  if( life < LINKFEST_LIFE_LIMIT ){
    return FALSE;
  }
  return TRUE;
}

//---------------------------------------------------------
/*
 * @brief ミッションが生きているか取得
 *
 * @retval  TRUE   ミッション中且つライフ有
 * @retval  FALSE  ミッション中でないorミッションのライフ切れ
 */
//---------------------------------------------------------
BOOL FEST_MISSION_IsLive( const FEST_MISSION* fmwk )
{
//  if( fmwk->m_id == FMISSION_ID_NULL || fmwk->life <= 0 ){
  if( fmwk->beacon.m_id == FMISSION_ID_NULL ){
    return FALSE;
  }
  return TRUE;
}

//---------------------------------------------------------
/*
 * @brief ミッションに参加しているか？
 *
 * @retval  TRUE   ミッション中
 * @retval  FALSE  ミッション中でない
 */
//---------------------------------------------------------
BOOL FEST_MISSION_IsEntry( const FEST_MISSION* fmwk )
{
  return fmwk->beacon.m_id != FMISSION_ID_NULL;
}

//---------------------------------------------------------
/*
 * @brief ミッション　スコアインクリメントチェック
 */
//---------------------------------------------------------
BOOL FEST_MISSION_IsScoreUp( FEST_MISSION* fmwk, FMISSION_LOG_ACTION action, u16 value )
{
  const MISSION_SCORE_CALC_PARAM* prm = &DATA_ScoreCalcParamTbl[ fmwk->arc.m_type ];
  IWASAWA_Printf("FMSUp a=%d,\n", action );

  switch( fmwk->arc.m_type ){
  case FMISSION_TYPE_EFFECT_ENC:
    if( action != FMISSION_LOG_ACT_EFF_ENC_WIN && 
        action != FMISSION_LOG_ACT_EFF_ENC_CAPTURE ){
      return FALSE;
    }
    break;
  case FMISSION_TYPE_MUSHROOM_ENC:
    if( action != FMISSION_LOG_ACT_MUSHROOM_WIN && 
        action != FMISSION_LOG_ACT_MUSHROOM_CAPTURE ){
      return FALSE;
    }
    break;
  case FMISSION_TYPE_EVENT:
    if( (action-FMISSION_LOG_ACT_GRP_EVENT) != fmwk->beacon.base_param ){
      return FALSE;
    }
    break;
  case FMISSION_TYPE_ITEM:
    if( action != FMISSION_LOG_ACT_KIRA_ITEM_GET ){
      return FALSE;
    }
    break;
  case FMISSION_TYPE_NPC:
    if( (action-FMISSION_LOG_ACT_GRP_NPC) != fmwk->beacon.base_param ){
      return FALSE;
    }
    break;
  }
  return prm->func(fmwk,value );
}

//---------------------------------------------------------
/*
 * @brief ミッション　スコアインクリメント
 */
//---------------------------------------------------------
BOOL FEST_MISSION_ScoreCalc( FEST_MISSION* fmwk,  FMISSION_LOG_ACTION action, u16 param, u16 inc )
{
  FEST_BEACON* beacon = &fmwk->beacon;
  FMISSION_ARC_COMMON* arc = &fmwk->arc;

  if( FEST_MISSION_IsLive( fmwk ) == FALSE ){
    return FALSE;
  }
  if( !FEST_MISSION_IsScoreUp( fmwk, action, param )){
    return FALSE;
  }
  if( (fmwk->my_score + inc ) > FMISSION_SCORE_MAX ){
    fmwk->my_score = FMISSION_SCORE_MAX;
  }else{
    fmwk->my_score += inc;
  }
  OS_FPrintf(OSPRINT_LINKFEST_PAGE,"ミッション m_id=%d key=%08x スコア加算 %d ノルマ %d\n",
      beacon->m_id, fmwk->key, fmwk->my_score, beacon->goal );
  return TRUE;
}

//---------------------------------------------------------
/*
 * @brief ミッション　タイムボーナス計算
 *
 * @return  秒単位のボーナス(1秒未満は1を返す)
 */
//---------------------------------------------------------
u32 FEST_MISSION_TimeBonusCalc( FEST_MISSION* fmwk,  u16 score_diff )
{
  FEST_BEACON* beacon = &fmwk->beacon;
  u32 bonus = score_CalcTimeBonus( fmwk ) * score_diff;
  
  if( bonus == 0 ){
    return 0;
  }
  if( beacon->time + bonus > FMISSION_TIME_MAX ){
    bonus = FMISSION_TIME_MAX-beacon->time;
    beacon->time = FMISSION_TIME_MAX;
  }else{
    beacon->time += bonus;
  }
#if 0
  //フレームを秒に変換して返す
  if( (bonus/60) == 0 ){
    bonus = 1; //最低1
  }else{
    bonus /= 60;
  }
#endif
  OS_FPrintf(OSPRINT_LINKFEST_PAGE,"ミッション m_id=%d key=%08x スコア加算 %d ボーナス %d\n",beacon->m_id, fmwk->key, score_diff, bonus );
  return bonus;
}

//---------------------------------------------------------
/*
 * @brief フェスビーコン比較
 */
//---------------------------------------------------------
BOOL FEST_MISSION_IsSame( const FEST_MISSION* f1, const FEST_MISSION* f2 )
{
  if( f1->key != f2->key ||
      f1->beacon.id_param != f2->beacon.id_param ){
    return FALSE;
  }
  return TRUE;
}

//========================================================================
/*
  サブルーチン
*/
//========================================================================

//---------------------------------------------------------
/*
 * @brief スコアボーナス計算
 */
//---------------------------------------------------------
static u32 score_CalcTimeBonus( FEST_MISSION* fmwk )
{
  static const u8 bonus_tbl[FMISSION_TBONUS_MAX] = { 0, 30, 60, 180 };

  return bonus_tbl[fmwk->beacon.time_bonus];
}

//---------------------------------------------------------
/*
 * @brief スコア計算 野生ポケモンバトル 
 */
//---------------------------------------------------------
static BOOL score_CalcWildBtlWin( const FEST_MISSION* fmwk, u16 param )
{
  return ( fmwk->beacon.base_param == 0 || fmwk->beacon.base_param == param );
}

//---------------------------------------------------------
/*
 * @brief スコア計算 パラメータはチェックしない
 */
//---------------------------------------------------------
static BOOL score_CalcCommon( const FEST_MISSION* fmwk, u16 param )
{
  return TRUE;
}

//=================================================
//以下デバッグルーチン
//=================================================
#ifdef PM_DEBUG
//---------------------------------------------------------
/*
 * @brief ミッション寿命強制破棄
 */
//---------------------------------------------------------
void FEST_MISSION_DEBUG_KillMission( FEST_MISSION* fmwk )
{
  FEST_BEACON* beacon = &fmwk->beacon;
  if( beacon->m_id == FMISSION_ID_NULL ){
    return;
  }
  beacon->time = 0;
  OS_FPrintf(OSPRINT_LINKFEST_PAGE,"フェスミッション %d key = %08x 寿命強制破棄\n", beacon->m_id, fmwk->key );
}

#endif  //PM_DEBUG


