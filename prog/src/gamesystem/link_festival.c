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
#include "gamesystem/game_comm.h"
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
  //スコアログ領域クリア
  log_ClearAll( wk );
  //ミッションログ領域クリア
  FMISSION_LOG_TBL_Clear( &wk->mlog_stack_tbl );
  FMISSION_LOG_TBL_Clear( &wk->mlog_view_tbl );

  wk->beacon_wait = 10*60;
  wk->gsys = gsys;
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
  if( FEST_MISSION_IsEntry(&wk->mission) == FALSE){
    return;
  }
  if( wk->live_f && GAMEDATA_GetIntrudeReverseArea( GAMESYSTEM_GetGameData(wk->gsys)) == FALSE ){  //表フィールドにいる間だけ更新
    u32 score;
    FEST_MISSION_Update( &wk->mission );

    score = LINK_FESTIVAL_GetNowScore(wk);
    if( score-wk->now_score ){
      wk->finfo_bonus += FEST_MISSION_TimeBonusCalc( &wk->mission,  score-wk->now_score );
      wk->now_score = score;
    }
  }
  if( wk->ready_timer ){
    --wk->ready_timer;
  }
  lnkfes_BeaconSend( wk );
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッション参加中か？
 */
//---------------------------------------------------------
BOOL LINK_FESTIVAL_IsMissionEntry( LINK_FESTIVAL* wk )
{
  return FEST_MISSION_IsEntry( &wk->mission);
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
 * @brief リンクフェスティバル ミッションログスタックテーブルポインタ取得
 */
//---------------------------------------------------------
FMISSION_LOG_TBL* LINK_FESTIVAL_GetMissionLogStackTbl( LINK_FESTIVAL* wk )
{
  return &wk->mlog_stack_tbl;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションログビューテーブルポインタ取得
 */
//---------------------------------------------------------
FMISSION_LOG_TBL* LINK_FESTIVAL_GetMissionLogViewTbl( LINK_FESTIVAL* wk )
{
  return &wk->mlog_view_tbl;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションログGパワーテーブルポインタ取得
 */
//---------------------------------------------------------
FMISSION_GPOWER_TBL* LINK_FESTIVAL_GetMissionLogGPowerTbl( LINK_FESTIVAL* wk )
{
  return &wk->mlog_gpower_tbl;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションスタート
 *
 * この関数を呼んだ時点でビーコン送信は開始されるが、
 * アップデートは無効のまま(時間カウントが始まらない)ので注意！
 */
//---------------------------------------------------------
void LINK_FESTIVAL_MissionStart( LINK_FESTIVAL* wk, const FEST_MISSION* mission, BOOL succeed_f )
{
  //ログ領域クリア
  log_ClearAll( wk );
  FMISSION_LOG_TBL_Clear( &wk->mlog_stack_tbl );
  FMISSION_LOG_TBL_Clear( &wk->mlog_view_tbl );

  FEST_MISSION_Start( &wk->mission, mission, succeed_f );
  wk->live_f = FALSE;

  //ハイリンク接続禁止フラグOn
  GameCommSys_SetPalaceNotConnectFlagForFMission(GAMESYSTEM_GetGameCommSysPtr(wk->gsys), TRUE );

  //最後のミッションを覚えておく
  log_LastMissionLogStore( wk, mission );
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションアップデート　有効無効設定
 */
//---------------------------------------------------------
void LINK_FESTIVAL_MissionLiveStatusSet( LINK_FESTIVAL* wk, BOOL live_f )
{
  wk->live_f = live_f;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッション終了
 */
//---------------------------------------------------------
void LINK_FESTIVAL_MissionEnd( LINK_FESTIVAL* wk )
{
  FEST_MISSION_End( &wk->mission );
  
  //ハイリンク接続禁止フラグOff
  GameCommSys_SetPalaceNotConnectFlagForFMission(GAMESYSTEM_GetGameCommSysPtr(wk->gsys), FALSE );

  log_ClearAll( wk );
  FMISSION_LOG_TBL_Clear( &wk->mlog_stack_tbl );
  FMISSION_LOG_TBL_Clear( &wk->mlog_view_tbl );
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションReadyタイマーセット
 *
 * @param カウントダウンタイム(秒)
 */
//---------------------------------------------------------
void LINK_FESTIVAL_MissionReadyTimerSet( LINK_FESTIVAL* wk, u32 timer )
{
  wk->ready_timer = timer*60;  //sec2frame
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションReadyタイマーゲット
 *
 * @return  カウンター値(秒)
 */
//---------------------------------------------------------
int LINK_FESTIVAL_MissionReadyTimerGet( LINK_FESTIVAL* wk )
{
  if( wk->ready_timer ){
    return LINKFEST_FRAME2SEC(wk->ready_timer);
  }
  return wk->ready_timer;
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
 *  @brief  リンクフェスティバル フィールドで表示するタイムボーナス値を取得
 */
//---------------------------------------------------------
u16 LINK_FESTIVAL_FInfoTimeBonusGet( LINK_FESTIVAL* wk )
{
  return wk->finfo_bonus;
}

//---------------------------------------------------------
/*
 *  @brief  リンクフェスティバル フィールドで現在表示するタイムボーナス値をリセット
 */
//---------------------------------------------------------
void LINK_FESTIVAL_FInfoTimeBonusReset( LINK_FESTIVAL* wk )
{
  wk->finfo_bonus = 0;
}

//---------------------------------------------------------
/*
 *  @brief  リンクフェスティバル　保持しているタウンマップルートIDを更新
 *  @retval TRUE  更新された
 *  @retval FALSE 値は変わらなかった
 *
 *  tmap_root = TOWNMAP_UTIL_GetRootZoneID(cp_gamedata, zone_id);
 */
//---------------------------------------------------------
BOOL LINK_FESTIVAL_TMapRootUpdate( LINK_FESTIVAL* wk, u16 new_root )
{
  if( wk->tmap_root != new_root ){
    wk->tmap_root = new_root;
    return TRUE;
  }
  return FALSE;
}

//---------------------------------------------------------
/*
 *  @brief  リンクフェスティバル ミッションフラグセット
 */
//---------------------------------------------------------
void LINK_FESTIVAL_MissionFlagSet( LINK_FESTIVAL* wk, u16 zone_idx, u8 flag_idx )
{
  int no = zone_idx*FMISSION_GIMMICK_MAX+flag_idx;

  wk->m_flag[(no/8)] |= (0x01)<<(no%8);
}

//---------------------------------------------------------
/*
 *  @brief  リンクフェスティバル ミッションフラグチェック
 */
//---------------------------------------------------------
BOOL LINK_FESTIVAL_MissionFlagCheck( LINK_FESTIVAL* wk, u16 zone_idx, u8 flag_idx )
{
  int no = zone_idx*FMISSION_GIMMICK_MAX+flag_idx;

  return ( wk->m_flag[(no/8)] & ((0x01)<<(no%8)));
}


//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　現在のスコア
 *
 * @retval  現在のスコア
 */
//---------------------------------------------------------
u32 LINK_FESTIVAL_GetNowScore( LINK_FESTIVAL* wk )
{
  int i;
  u32 score = wk->mission.my_score;

  if( FEST_MISSION_IsEntry(&wk->mission) == FALSE){
    return 0;
  }
  for(i = 0;i < wk->log_num;i++){
    score += wk->log[i].score.value;
  }
  if( score > 0xFFFF ){
    return 0xFFFF;
  }
  return score;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ログ解析してスコアとミッション成功・失敗を判定
 *
 * @param u32* スコアを取得するu32のワークポインタ
 * @param u32* 取得するアイテム数を返すu32のワークポインタ
 * @retval  TRUE    ミッション成功
 * @retval  FALSE   ミッション失敗
 *
 * エントリーしてない時に呼び出すと不定の値が返るので注意
 * LINK_FESTIVAL_IsMissionEntry(), FEST_MISSION_IsEntry(&wk->mission)でチェックしてから呼ぶこと
 *
 */
//---------------------------------------------------------
BOOL LINK_FESTIVAL_GetMissionResult( LINK_FESTIVAL* wk, u32* score, u32* item_num )
{
  *score = 0;
  *item_num = 0;
  *score = LINK_FESTIVAL_GetNowScore( wk );

  if( *score >= wk->mission.target){
//    OS_FPrintf( OSPRINT_LINKFEST_PAGE,"ミッションノルマ達成 %d/%d\n",*score,wk->mission.target );
    *item_num = wk->mission.item_num;
    return TRUE;
  }
//  OS_FPrintf( OSPRINT_LINKFEST_PAGE,"ミッションノルマ未達成 %d/%d\n",*score,wk->mission.target );
  return FALSE;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ミッションの終了条件を満たしているか？
 *
 * @retval  TRUE    ミッション終了待ち状態
 * @retval  FALSE   ミッション中
 */
//---------------------------------------------------------
BOOL LINK_FESTIVAL_IsMissionEnd( LINK_FESTIVAL* wk )
{
  u32 score,item_num;
  BOOL ret;

  if( FEST_MISSION_IsEntry(&wk->mission) == FALSE){
    return FALSE;
  }
  ret = LINK_FESTIVAL_GetMissionResult( wk, &score, &item_num );

  //スコアクリア型の終了チェック
  if(ret && (wk->mission.end_type == FMISSION_ENDTYPE_CLEAR)){
    return TRUE;
  }
  //タイムオーバーチェック
  return FEST_MISSION_IsEnd(&wk->mission);
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル スコア加算
 */
//---------------------------------------------------------
BOOL LINK_FESTIVAL_MissionScoreCalc( GAMESYS_WORK* gsys, FMISSION_LOG_ACTION action, u16 param )
{
  LINK_FESTIVAL* wk;
  int inc = 1;  //@todo 現状スコア1固定

  if( gsys == NULL ){
    return FALSE;
  }
  wk = GAMESYSTEM_GetLinkFestivalWork( gsys );
  if( wk == NULL ){
    return FALSE;
  }
  return FEST_MISSION_ScoreCalc( &wk->mission, action, param, inc );
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
LINKFEST_SCORELOG_SET_CODE LINK_FESTIVAL_ScoreLogStore( LINK_FESTIVAL* wk, LINKFEST_LOG_PARAM* src )
{
  int i;
  LINKFEST_LOG* log;

  if( (src->type != FMISSION_TYPE_EX_WITHOUT_PARAM) && 
      (wk->mission.type != src->type || wk->mission.key != src->key ) ){
    return LINKFEST_SCORELOG_SET_FAILED;  //違うミッションなのではじく
  }
  for(i = 0;i < wk->log_num; i++){
    log = &wk->log[i];

    if( log->score.tr_id == src->prm.score.tr_id ){ //上書き
      u16 root = log->score.tmap_root;
      u32 value = log->score.value;

      log->score = src->prm.score;
      if( src->type == FMISSION_TYPE_EX_WITHOUT_PARAM ){
        log->score.value = value; //パラメータ無しタイプはスコアを書き換えない
      }
      OS_FPrintf(OSPRINT_LINKFEST_PAGE,"FMIS log%d up tr_id=%d, score=%d\n",i, log->score.tr_id,log->score.value );
      if( root != src->prm.score.tmap_root ){
        return LINKFEST_SCORELOG_SET_MAP_CHANGE;
      }
      return LINKFEST_SCORELOG_SET_UPDATE;
    }
  }
  if( src->type == FMISSION_TYPE_EX_WITHOUT_PARAM ){
    return LINKFEST_SCORELOG_SET_FAILED;  //ミッションパラメータを持たないタイプは、新規では取得しない
  }
  if( wk->log_num >= LINKFEST_LOG_MAX ){
    OS_FPrintf(OSPRINT_LINKFEST_PAGE,"FMIS BufFull\n");
    return LINKFEST_SCORELOG_SET_FAILED;  //もう入らない
  }
  //新規
  log = &wk->log[wk->log_num];
  log->score = src->prm.score;
  OS_FPrintf(OSPRINT_LINKFEST_PAGE,"FMIS nLog%d tr_id=%d, score=%d\n",wk->log_num, log->score.tr_id,log->score.value );
  ++wk->log_num;
  return LINKFEST_SCORELOG_SET_NEW_ENTRY;
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
  OS_FPrintf(OSPRINT_LINKFEST_PAGE,"FMIS nLogEgnore%d num=%d, id=%d, key=%08x, param=%d\n",
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
    OS_FPrintf(OSPRINT_LINKFEST_PAGE,"FMIS Egnore1 type=%d, key=%08x, param=%d\n",
      fmwk->type, fmwk->key, fmwk->param );
    return TRUE;
  }
  //無視ログ登録されているか？
  if( log_CheckEgnoreLog( wk, fmwk )){
    OS_FPrintf(OSPRINT_LINKFEST_PAGE,"FMIS Egnore2 type=%d, key=%08x, param=%d\n",
      fmwk->type, fmwk->key, fmwk->param );
    return TRUE;
  }
  //残り時間が一定以上か？
  if( fmwk->life/60 < 30 ){
    OS_FPrintf(OSPRINT_LINKFEST_PAGE,"FMIS Egnore3 type=%d, key=%08x, param=%d life=%dsec\n",
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
  wk->finfo_bonus = 0;
  wk->now_score = 0;
  wk->tmap_root = 0xFFFF; //無効値で初期化
  MI_CpuClear8( wk->m_flag, sizeof(u8)*FMISSION_FLAG_AREA_LEN);

  wk->ready_timer = 0;
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

