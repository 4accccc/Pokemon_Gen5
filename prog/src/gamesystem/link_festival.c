//==============================================================================
/**
 * @file    link_festival.c
 * @brief   リンクフェスティバル
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


//==============================================================================
//  構造体定義
//==============================================================================
///リンクフェスティバル　管理領域
struct _LINK_FESTIVAL{
  FEST_MISSION  mission;  //現ミッションデータ

  u32 beacon_wait;  ///frame
  u32 beacon_timer; //frame

  int log_num;
  LINKFEST_LOG  log[LINKFEST_LOG_MAX];
};

static void lnkfes_BeaconSend( LINK_FESTIVAL* wk );
static void lnkfes_LogClearAll( LINK_FESTIVAL* wk );

static void festlog_Clear( LINKFEST_LOG* log );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル管理領域生成
 */
//---------------------------------------------------------
LINK_FESTIVAL* LINK_FESTIVAL_Create( HEAPID heapID, GAMESYS_WORK* gsys )
{
  LINK_FESTIVAL* wk;
  wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(LINK_FESTIVAL) );

  FEST_MISSION_Clear( &wk->mission );
  //ログクリア
  lnkfes_LogClearAll( wk );

  wk->beacon_wait = 10*60;
  return wk;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル管理領域解放
 */
//---------------------------------------------------------
void LINK_FESTIVAL_Delete( LINK_FESTIVAL* wk )
{
  MI_CpuClear32(wk,sizeof(LINK_FESTIVAL));
  GFL_HEAP_FreeMemory( wk );
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル状態アップデート
 */
//---------------------------------------------------------
void LINK_FESTIVAL_Update( LINK_FESTIVAL* wk )
{
  FEST_MISSION_Update( &wk->mission );

  lnkfes_BeaconSend( wk );
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションワークポインタ取得
 */
//---------------------------------------------------------
FEST_MISSION* LINK_FESTIVAL_GetMissionWork( LINK_FESTIVAL* wk )
{
  return &wk->mission;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ログ解析してスコアとミッション成功・失敗を判定
 *
 * @param u32* スコアを取得するu32のワークポインタ
 * @retval  TRUE    ミッション成功
 * @retval  FALSE   ミッション失敗
 */
//---------------------------------------------------------
BOOL LINK_FESTIVAL_GetMissionResult( LINK_FESTIVAL* wk, u32* score )
{
  int i;

  *score = 0;
  if( FEST_MISSION_IsEntry(&wk->mission) == FALSE){
    return FALSE;
  }
  for(i = 0;i < wk->log_num;i++){
    *score += wk->log[i].score;
  }
  *score += wk->mission.my_score;

  if( *score >= wk->mission.target){
    OS_FPrintf( OSPRINT_LINKFEST_PAGE,"ミッションノルマ達成 %d/%d\n",*score,wk->mission.target );
    return TRUE;
  }
  OS_FPrintf( OSPRINT_LINKFEST_PAGE,"ミッションノルマ未達成 %d/%d\n",*score,wk->mission.target );
  return FALSE;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル スコア加算
 */
//---------------------------------------------------------
void LINK_FESTIVAL_MissionScoreCalc( GAMESYS_WORK* gsys, FMISSION_TYPE_ID type, u32 value )
{
  LINK_FESTIVAL* wk;

  if( gsys == NULL ){
    GF_ASSERT(0);
    return;
  }
  wk = GAMESYSTEM_GetLinkFestivalWork( gsys );
  FEST_MISSION_ScoreCalc( &wk->mission, type, value );
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ログストア
 *
 * @param log 情報を格納したLINKFEST_LOG構造体型ワークへのポインタ
 */
//---------------------------------------------------------
BOOL LINK_FESTIVAL_LogStore( LINK_FESTIVAL* wk, LINKFEST_LOG_PARAM* src )
{
  int i;
  LINKFEST_LOG* log;

  if( FEST_MISSION_IsLive( &wk->mission ) == FALSE ){
    return FALSE;
  }
  if( wk->mission.id != src->id ||
      wk->mission.key != src->key ){
    return FALSE; //違うミッションなのではじく
  }
  for(i = 0;i < wk->log_num; i++){
    log = &wk->log[i];

    if( log->tr_id == src->log.tr_id ){ //上書き
      log->score = src->log.score;
      OS_FPrintf(OSPRINT_LINKFEST_PAGE,"フェスミッション ログ%dを上書き tr_id=%d, score=%d\n",i, log->tr_id,log->score );
      return TRUE;
    }
  }
  if( wk->log_num >= LINKFEST_LOG_MAX ){
    OS_FPrintf(OSPRINT_LINKFEST_PAGE,"フェスミッション ログバッファがいっぱいです\n");
    return FALSE; //もう入らない
  }
  //新規
  log = &wk->log[wk->log_num];
  *log = src->log;
  OS_FPrintf(OSPRINT_LINKFEST_PAGE,"フェスミッション 新規ログ%d tr_id=%d, score=%d\n",wk->log_num, log->tr_id,log->score );
  ++wk->log_num;
  return TRUE;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ログ構造体クリア
 */
//---------------------------------------------------------
void LINKFEST_LOG_Clear( LINKFEST_LOG* log )
{
  MI_CpuClear8( log, sizeof(LINKFEST_LOG) );
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ログパラメータ構造体クリア
 */
//---------------------------------------------------------
void LINKFEST_LOG_PARAM_Clear( LINKFEST_LOG_PARAM* prm )
{
  MI_CpuClear8( prm, sizeof(LINKFEST_LOG_PARAM) );
  LINKFEST_LOG_Clear( &prm->log );
}

//=========================================================================
//サブルーチン
//=========================================================================
//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ビーコン送信
 */
//---------------------------------------------------------
static void lnkfes_BeaconSend( LINK_FESTIVAL* wk )
{
  if( FEST_MISSION_IsLive(&wk->mission) == FALSE){
    wk->beacon_timer = 0;
    return;
  }
  if( wk->beacon_timer > 0 ){
    --wk->beacon_timer;
    return;
  }
  GAMEBEACON_Set_FestMission(&wk->mission);
  wk->beacon_timer = wk->beacon_wait;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ログクリア
 */
//---------------------------------------------------------
static void lnkfes_LogClearAll( LINK_FESTIVAL* wk )
{
  int i;

  for(i = 0;i < LINKFEST_LOG_MAX;i++){
    LINKFEST_LOG_Clear( &wk->log[i]);
  }
  wk->log_num = 0;
  wk->beacon_timer = 0; 
}


