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
#include "gamesystem/link_festival_local.h"

//==============================================================================
//  構造体定義
//==============================================================================
static void lnkfes_BeaconSend( LINK_FESTIVAL* wk );
static void log_ClearAll( LINK_FESTIVAL* wk );
static void log_LastMissionLogStore( LINK_FESTIVAL* wk, const FEST_MISSION* fmwk );
static BOOL log_CheckEgnoreLog( LINK_FESTIVAL* wk, const FEST_MISSION* fmwk );
static void log_mission_LogStore( LINKFEST_LOG* log, const FEST_MISSION* fmwk );
static BOOL log_mission_IsSame( LINKFEST_LOG* log, const FEST_MISSION* fmwk );


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
  log_ClearAll( wk );

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
 * @brief リンクフェスティバル ミッションスタート
 */
//---------------------------------------------------------
void LINK_FESTIVAL_MissionStart( LINK_FESTIVAL* wk, const FEST_MISSION* mission )
{
  log_ClearAll( wk );

  FEST_MISSION_Start( &wk->mission, mission );

  //最後のミッションを覚えておく
  log_LastMissionLogStore( wk, mission );
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッション終了
 */
//---------------------------------------------------------
void LINK_FESTIVAL_MissionEnd( LINK_FESTIVAL* wk )
{
  FEST_MISSION_End( &wk->mission );

  log_ClearAll( wk );
}

//---------------------------------------------------------
/*
 *  @brief  リンクフェスティバル フィールドで現在表示しているスコアを取得
 */
//---------------------------------------------------------
u16 LINK_FESTIVAL_FInfoScoreGet( LINK_FESTIVAL* wk )
{
  return wk->finfo_score;
}

//---------------------------------------------------------
/*
 *  @brief  リンクフェスティバル フィールドで現在表示しているスコアをインクリメント
 */
//---------------------------------------------------------
void LINK_FESTIVAL_FInfoScoreInc( LINK_FESTIVAL* wk )
{
  if( wk->finfo_score < 0xFFFF ){
    wk->finfo_score += 1;
  }
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
    *score += wk->log[i].score.value;
  }
  *score += wk->mission.my_score;

  if( *score >= wk->mission.target){
//    OS_FPrintf( OSPRINT_LINKFEST_PAGE,"ミッションノルマ達成 %d/%d\n",*score,wk->mission.target );
    return TRUE;
  }
//  OS_FPrintf( OSPRINT_LINKFEST_PAGE,"ミッションノルマ未達成 %d/%d\n",*score,wk->mission.target );
  return FALSE;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル スコア加算
 */
//---------------------------------------------------------
BOOL LINK_FESTIVAL_MissionScoreCalc( GAMESYS_WORK* gsys, u16 param, u16 inc )
{
  LINK_FESTIVAL* wk;

  if( gsys == NULL ){
    GF_ASSERT(0);
    return FALSE;
  }
  wk = GAMESYSTEM_GetLinkFestivalWork( gsys );
  if( wk != NULL ){
    return FEST_MISSION_ScoreCalc( &wk->mission, param, inc );
  }
  return FALSE;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッション参加人数取得
 */
//---------------------------------------------------------
int LINK_FESTIVAL_GetMissionEntryNum( LINK_FESTIVAL* wk )
{
  if( FEST_MISSION_IsEntry( &wk->mission ) == FALSE ){
    return 0;
  }
  return wk->log_num+1;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ログストア
 *
 * @param log 情報を格納したLINKFEST_LOG構造体型ワークへのポインタ
 */
//---------------------------------------------------------
BOOL LINK_FESTIVAL_ScoreLogStore( LINK_FESTIVAL* wk, LINKFEST_LOG_PARAM* src )
{
  int i;
  LINKFEST_LOG* log;

  if( FEST_MISSION_IsLive( &wk->mission ) == FALSE ){
    return FALSE;
  }
  if( wk->mission.type != src->type ||
      wk->mission.key != src->key ){
    return FALSE; //違うミッションなのではじく
  }
  for(i = 0;i < wk->log_num; i++){
    log = &wk->log[i];

    if( log->score.tr_id == src->prm.score.tr_id ){ //上書き
      log->score.value = src->prm.score.value;
      OS_FPrintf(OSPRINT_LINKFEST_PAGE,"フェスミッション ログ%dを上書き tr_id=%d, score=%d\n",i, log->score.tr_id,log->score.value );
      return TRUE;
    }
  }
  if( wk->log_num >= LINKFEST_LOG_MAX ){
    OS_FPrintf(OSPRINT_LINKFEST_PAGE,"フェスミッション ログバッファがいっぱいです\n");
    return FALSE; //もう入らない
  }
  //新規
  log = &wk->log[wk->log_num];
  (*log).score = src->prm.score;
  OS_FPrintf(OSPRINT_LINKFEST_PAGE,"フェスミッション 新規ログ%d tr_id=%d, score=%d\n",wk->log_num, log->score.tr_id,log->score.value );
  ++wk->log_num;
  return TRUE;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　無視したミッションのログをストア
 *
 * @param fmwk ミッション情報を格納したFEST_MISSION構造体型ワークへのポインタ
 */
//---------------------------------------------------------
void LINK_FESTIVAL_EgnoreLogStore( LINK_FESTIVAL* wk, const FEST_MISSION* fmwk )
{
  int i;
  LINKFEST_LOG* log;

  if( FEST_MISSION_IsEntry( &wk->mission )){
    return;
  }
  //同じログがあるか？
  if( log_CheckEgnoreLog( wk, fmwk )){
    return;
  }
  //新規
  log = &wk->log[wk->log_idx];
  log_mission_LogStore( log, fmwk );
  OS_FPrintf(OSPRINT_LINKFEST_PAGE,"フェスミッション 新規無視ログ%d num=%d, id=%d, key=%08x, param=%d\n",
    wk->log_idx, wk->log_num, fmwk->type, fmwk->key, fmwk->param );

  wk->log_idx = (wk->log_idx + 1)%LINKFEST_LOG_MAX;
  if( wk->log_num < LINKFEST_LOG_MAX ){
    ++wk->log_num;
  }
  return;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　無視するミッションかチェック
 *
 * @param fmwk ミッション情報を格納したFEST_MISSION構造体型ワークへのポインタ
 */
//---------------------------------------------------------
BOOL LINK_FESTIVAL_IsEgnoreMission( LINK_FESTIVAL* wk, const FEST_MISSION* fmwk )
{
  int i;
  LINKFEST_LOG* log;

  //最後のミッションと同一か？
  if( wk->last.mission.type != FMISSION_TYPE_NULL &&
      log_mission_IsSame( &wk->last, fmwk ))
  {
    OS_FPrintf(OSPRINT_LINKFEST_PAGE,"フェスミッション 最後のミッションと同一なので無視 type=%d, key=%08x, param=%d\n",
      fmwk->type, fmwk->key, fmwk->param );
    return TRUE;
  }
  //無視ログ登録されているか？
  if( log_CheckEgnoreLog( wk, fmwk )){
    OS_FPrintf(OSPRINT_LINKFEST_PAGE,"フェスミッション 無視ログ登録されているので無視 type=%d, key=%08x, param=%d\n",
      fmwk->type, fmwk->key, fmwk->param );
    return TRUE;
  }
  //残り時間が一定以上か？
  if( fmwk->life/60 < 30 ){
    OS_FPrintf(OSPRINT_LINKFEST_PAGE,"フェスミッション 残り時間が30秒未満なので無視 type=%d, key=%08x, param=%d life=%dsec\n",
      fmwk->type, fmwk->key, fmwk->param, fmwk->life/60 );
    return TRUE;
  }
  return FALSE;
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
void LINKFEST_LOG_PARAM_Clear( LINKFEST_LOG_PARAM* log_prm )
{
  MI_CpuClear8( log_prm, sizeof(LINKFEST_LOG_PARAM) );
  LINKFEST_LOG_Clear( &log_prm->prm );
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
static void log_ClearAll( LINK_FESTIVAL* wk )
{
  int i;

  for(i = 0;i < LINKFEST_LOG_MAX;i++){
    LINKFEST_LOG_Clear( &wk->log[i]);
  }
  wk->log_num = 0;
  wk->log_idx = 0;
  wk->beacon_timer = 0; 
  wk->finfo_score = 0;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ラストミッションログストア
 */
//---------------------------------------------------------
static void log_LastMissionLogStore( LINK_FESTIVAL* wk, const FEST_MISSION* fmwk )
{
  LINKFEST_LOG_Clear( &wk->last );
  log_mission_LogStore( &wk->last, fmwk );
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　指定ミッションが無視ログ内にあるかチェック
 */
//---------------------------------------------------------
static BOOL log_CheckEgnoreLog( LINK_FESTIVAL* wk, const FEST_MISSION* fmwk )
{
  int i,idx;

  idx = ((int)wk->log_idx+LINKFEST_LOG_END)%LINKFEST_LOG_MAX;
  for(i = 0;i < wk->log_num;i++){
    if( log_mission_IsSame( &wk->log[idx], fmwk )){
      return TRUE;
    }
    idx = (idx+LINKFEST_LOG_END)%LINKFEST_LOG_MAX;
  }
  return FALSE;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ミッションログストア
 */
//---------------------------------------------------------
static void log_mission_LogStore( LINKFEST_LOG* log, const FEST_MISSION* fmwk )
{
  log->mission.type = fmwk->type;
  log->mission.key = fmwk->key;
  log->mission.param = fmwk->param;
}


//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ミッションログと同じミッションか？
 */
//---------------------------------------------------------
static BOOL log_mission_IsSame( LINKFEST_LOG* log, const FEST_MISSION* fmwk )
{
  if( log->mission.key != fmwk->key ||
      log->mission.type != fmwk->type ||
      log->mission.param != fmwk->param ){
      return FALSE; //違うミッション
  }
  return TRUE;
}

