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
#include "field/field_sound.h"
#include "gamesystem/g_power.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_comm.h"
#include "waza_tool/wazano_def.h"
#include "arc_def.h"
#include "power.naix"
#include "poke_tool/status_rcv.h"
#include "../../../resource/fldmapdata/flagwork/flag_define.h"

#include "gamesystem/link_festival.h"
#include "gamesystem/link_festival_local.h"
#include "savedata/fmission_save.h"
#include "field/zonedata.h"

FS_EXTERN_OVERLAY( notwifi_fes_gimmick_item );
FS_EXTERN_OVERLAY( notwifi_fes_gimmick_npc );
FS_EXTERN_OVERLAY( notwifi_fes_gimmick_event );

//==============================================================================
//  構造体定義
//==============================================================================
static void lnkfes_OverlayLoadUnload( const FEST_MISSION* m_src, BOOL load_f );
static void lnkfes_BeaconSend( LINK_FESTIVAL* wk );
static void topscore_Clear( LINK_FESTIVAL* wk, LINKFEST_LOG_TOP_SCORE* tsp );
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
  
  wk->gsys = gsys;
  wk->gdata = GAMESYSTEM_GetGameData( wk->gsys );

  wk->fm_data_h = GFL_ARC_OpenDataHandle( ARCID_FMISSION_DATA, heapID );
 
  //ミッションアーカイブ常駐データ取得
  wk->p_arc_data = FMISSION_DATA_AllocArcCommonAllByHandle( wk->fm_data_h, heapID );

  FEST_MISSION_Clear( &wk->mission );
  wk->m_beacon = &wk->mission.beacon;
  wk->m_arc = &wk->mission.arc;
  
  wk->top_score = LINKFEST_LOG_TOP_SCORE_Create( heapID );

  //ログ領域クリア
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
  LINKFEST_LOG_TOP_SCORE_Delete( wk->top_score );

  GFL_HEAP_FreeMemory( wk->p_arc_data );  //ミッションデータアーカイブ開放
  GFL_ARC_CloseDataHandle( wk->fm_data_h); //ミッションデータアーカイブハンドル開放

  MI_CpuClear8(wk,sizeof(LINK_FESTIVAL));
  GFL_HEAP_FreeMemory( wk );
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル状態アップデート
 */
//---------------------------------------------------------
void LINK_FESTIVAL_Update( LINK_FESTIVAL* wk )
{
  FMISSION_ENTRY_TBL_Update( wk );

  if( FEST_MISSION_IsEntry(&wk->mission) == FALSE){
    return;
  }
  if( wk->live_status == FMISSION_LIVE_STATUS_PLAY ){
    u32 score = LINK_FESTIVAL_GetNowScore(wk);

    if( score-wk->now_score ){
      wk->finfo_bonus += FEST_MISSION_TimeBonusCalc( &wk->mission,  score-wk->now_score );
      wk->now_score = score;
    }
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
 * @brief リンクフェスティバル ミッションデータアーカイブハンドル取得
 */
//---------------------------------------------------------
ARCHANDLE* LINK_FESTIVAL_GetFMissionDataHandle( LINK_FESTIVAL* wk )
{
  return wk->fm_data_h;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル 送信用ミッションデータ生成
 */
//---------------------------------------------------------
void LINK_FESTIVAL_GetBeaconSendParam( LINK_FESTIVAL* wk, FEST_BEACON* dat, u16 act_param )
{
  *dat = *wk->m_beacon;

  //アクションParamと残り時間上書き
  dat->act_param = act_param;
  dat->time = LINK_FESTIVAL_GetMissionLife( wk );
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッション　現在の自分のスコアを返す
 *
 * @retval 自分のスコア 
 */
//---------------------------------------------------------
u16 LINK_FESTIVAL_GetMyScore( LINK_FESTIVAL* wk )
{
  return wk->mission.my_score;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッション残り時間を返す
 *
 * @retval  残り時間：秒単位
 */
//---------------------------------------------------------
u16 LINK_FESTIVAL_GetMissionLife( LINK_FESTIVAL* wk )
{
  u32 life = FEST_MISSION_GetLife( &wk->mission );

  return life;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションの現在の状態を返す
 */
//---------------------------------------------------------
void LINK_FESTIVAL_GetMissionStatus( LINK_FESTIVAL* wk, LINKFEST_MISSION_STATUS* sp  )
{
  int i,j,h,t;
  u8  idx[LINKFEST_LOG_MAX];

  MI_CpuClear8( sp, sizeof(LINKFEST_MISSION_STATUS) );

  sp->entry_num = wk->log_num+1;  //自分も含めるので+1
  sp->my_score = wk->mission.my_score;
  sp->total_score = LINK_FESTIVAL_GetNowScore( wk );
 
  if( wk->log_num == 0 ){
    sp->my_ranking = 1; //自分ひとりなので必ず一位
    return;
  }
  //自分のスコアのランキングを取得
  for( i = 0;i < wk->log_num; i++ ){
    idx[i] = i; 
  }

  //シェルソートでスコア高い順に並べ替え(9と3はシェルソートアルゴリズムのリテラル)
  {
    for( h = 1; h < (wk->log_num/9); h = h*3+1)
      ; //hを変化させたいだけなので、特にメイン処理はない

    for( ; h > 0; h/=3 ){
      for(i = h; i<wk->log_num; i++){
        j = i;
        while(j >= h && wk->log[ idx[j-h] ].score.value < wk->log[ idx[j] ].score.value ){
          t = idx[j];
          idx[j] = idx[j-h];
          idx[j-h] = t;
          j -= h;
        }
      }
    }
  }
#ifdef PM_DEBUG
  for( i = 0;i < wk->log_num;i++){
    IWASAWA_Printf("SSort%d idx:%d-> %d\n",i,idx[i],wk->log[idx[i]].score.value);
  }
#endif
  if( sp->my_score >= wk->log[ idx[0] ].score.value ){
    sp->my_ranking = 1; //1位
  }else if( sp->my_score <= wk->log[ idx[wk->log_num-1] ].score.value ){
    sp->my_ranking = sp->entry_num; //最下位
  }
  for( i = 0; i < wk->log_num;i++){
    if( sp->my_score >= wk->log[ idx[i] ].score.value ){
      sp->my_ranking = i+1; //1オリジンで描画するので+1して返す
      break;
    }
  }
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
 * @brief リンクフェスティバル ミッションエントリーデータテーブルポインタ取得
 */
//---------------------------------------------------------
FMISSION_ENTRY_TBL* LINK_FESTIVAL_GetMissionLogEntryTbl( LINK_FESTIVAL* wk )
{
  return &wk->mlog_entry_tbl;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションデータコモンアーカイブパラメータ取得
 */
//---------------------------------------------------------
void LINK_FESTIVAL_GetMissionArcCommonData( LINK_FESTIVAL* wk, u8 m_id, FMISSION_ARC_COMMON* arc )
{
  *arc = wk->p_arc_data[m_id];
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションスタート
 *
 * この関数を呼んだ時点で内部的にはミッションが開始され、ビーコン送信も開始されるが、
 * アップデートは無効のまま(時間カウントが始まらない)ので注意！
 *
 * ミッションをライブ状態にするには、MissionStart後にMissionPlayStart()を呼び出してください
 */
//---------------------------------------------------------
void LINK_FESTIVAL_MissionStart( LINK_FESTIVAL* wk, const FEST_MISSION* m_src, BOOL succeed_f )
{
  //ログ領域クリア
  log_ClearAll( wk );

  FEST_MISSION_Start( &wk->mission, m_src, succeed_f );
  //ステータスセット
  LINK_FESTIVAL_MissionLiveStatusSet( wk, FMISSION_LIVE_STATUS_START );

  //ハイリンク接続禁止フラグOn
  GameCommSys_SetPalaceNotConnectFlagForFMission(GAMESYSTEM_GetGameCommSysPtr(wk->gsys), TRUE );

  //最後のミッションを覚えておく
  log_LastMissionLogStore( wk, &wk->mission );

  //フェスギミック用オーバレイをロード
  lnkfes_OverlayLoadUnload( m_src, TRUE );

  //BGM変更用フラグを立てておく
  GAMEDATA_SetFesBGMFlag( wk->gdata, TRUE );

}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションプレイスタート
 *
 * MissionStartを呼んだ時点で内部的にはミッションが開始され、ビーコン送信も開始されていますが
 * アップデートは無効のまま(時間カウントが始まらない)なので
 * Start後に、この関数を呼び出してプレイ状態に遷移してください
 *
 * 時間カウントの基準も、この関数を呼び出したタイミングで決定します
 */
//---------------------------------------------------------
void LINK_FESTIVAL_MissionPlayStart( LINK_FESTIVAL* wk, BOOL succeed_f  )
{
  if( FEST_MISSION_IsEntry(&wk->mission) == FALSE){
    GF_ASSERT(0); //ミッション開始してないのに呼び出しちゃダメ
    return;
  }
  if( wk->mission.start_rtc == FMISSION_RTC_NULL ){
    //残り時間カウントの基準となるRTCをセット
    FEST_MISSION_PlayStart( &wk->mission );
    GF_ASSERT(!succeed_f);  //←継承ミッションなのにstart_rtcが無効値のままはおかしい
  }
  
  //プレイ開始ステータスセット
  LINK_FESTIVAL_MissionLiveStatusSet( wk, FMISSION_LIVE_STATUS_PLAY );
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションステータス 取得 
 */
//---------------------------------------------------------
void LINK_FESTIVAL_MissionLiveStatusSet( LINK_FESTIVAL* wk, FMISSION_LIVE_STATUS live_status )
{
  wk->live_status = live_status;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションステータス 取得 
 */
//---------------------------------------------------------
FMISSION_LIVE_STATUS LINK_FESTIVAL_MissionLiveStatusGet( LINK_FESTIVAL* wk )
{
  return wk->live_status;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッション終了
 */
//---------------------------------------------------------
u32 LINK_FESTIVAL_MissionEnd( LINK_FESTIVAL* wk )
{
  ZONEID zone_id;
  
  //BGM変更用フラグをOff
  GAMEDATA_SetFesBGMFlag( wk->gdata, FALSE );

  lnkfes_OverlayLoadUnload( &wk->mission, FALSE );
  
  FEST_MISSION_End( &wk->mission );
  
  //ハイリンク接続禁止フラグOff
  GameCommSys_SetPalaceNotConnectFlagForFMission(GAMESYSTEM_GetGameCommSysPtr(wk->gsys), FALSE );

  log_ClearAll( wk );

  if( FSND_GetLastBGM( wk->gdata ) != SEQ_BGM_FES ){
    return FES_BGM_RECOVER_NONE;
  }
  //フェスBGMだったらBGMを元に戻す
  zone_id = PLAYERWORK_getZoneID( GAMEDATA_GetMyPlayerWork( wk->gdata ) );
  return FSND_GetFieldBGM( wk->gdata, zone_id, GAMEDATA_GetSeasonID( wk->gdata ) );
}

//---------------------------------------------------------
/*
 *  @brief  リンクフェスティバル フィールドで現在表示しているトータルスコアを取得
 */
//---------------------------------------------------------
u16 LINK_FESTIVAL_FInfoTotalScoreGet( LINK_FESTIVAL* wk )
{
  return wk->finfo_total_score;
}
//---------------------------------------------------------
/*
 *  @brief  リンクフェスティバル フィールドで現在表示しているマイスコアを取得
 */
//---------------------------------------------------------
u16 LINK_FESTIVAL_FInfoMyScoreGet( LINK_FESTIVAL* wk )
{
  return wk->finfo_my_score;
}

//---------------------------------------------------------
/*
 *  @brief  リンクフェスティバル フィールドで現在表示しているスコアをアップデート
 */
//---------------------------------------------------------
void LINK_FESTIVAL_FInfoScoreUpdate( LINK_FESTIVAL* wk, u16 now_total_score )
{
  wk->finfo_total_score = now_total_score;
  wk->finfo_my_score = wk->mission.my_score;
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
  u16 old_root = wk->tmap_root;

  if( wk->tmap_root != new_root ){
    wk->tmap_root = new_root;
    return ( !ZONEDATA_IsPalace(old_root) ); //古い場所がパレスならログを生成しない
  }
  return FALSE;
}

//---------------------------------------------------------
/*
 *  @brief  リンクフェスティバル ミッションギミックワーク領域バッファへのポインタを返す
 */
//---------------------------------------------------------
void* LINK_FESTIVAL_GetMissionGimmickWork( LINK_FESTIVAL* wk )
{
  return (void*)wk->m_gmk_work;
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
  if( score > FMISSION_SCORE_MAX ){
    return FMISSION_SCORE_MAX;
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
BOOL LINK_FESTIVAL_IsMissionClear( LINK_FESTIVAL* wk, u32* score )
{
  *score = 0;
  *score = LINK_FESTIVAL_GetNowScore( wk );

  if( *score >= wk->m_beacon->goal){
//    OS_FPrintf( OSPRINT_LINKFEST_PAGE,"ミッションノルマ達成 %d/%d\n",*score,wk->mission.target );
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
  u32 score,life;
  BOOL ret;

  if( FEST_MISSION_IsEntry(&wk->mission) == FALSE){
    return FALSE;
  }
  life = LINK_FESTIVAL_GetMissionLife( wk );

  //スコアクリア型の終了チェック
  if( LINK_FESTIVAL_IsMissionClear( wk, &score ) && (wk->m_beacon->end_type == FMISSION_ENDTYPE_SCORE)){
    ret = TRUE;
  }else{
    //タイムオーバーチェック
    ret = ( life == 0 );
  }
  if( ret ){  //今の残り時間を覚えておく
    wk->mission.end_life = life; 
  }
  return ret;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル スコア加算
 */
//---------------------------------------------------------
BOOL LINK_FESTIVAL_MissionScoreCalc( GAMESYS_WORK* gsys, FMISSION_LOG_ACTION action, u16 param )
{
  BOOL ret;
  LINK_FESTIVAL* wk = GAMESYSTEM_GetLinkFestivalWork( gsys );

  ret = FEST_MISSION_ScoreCalc( &wk->mission, action, param, 1 );  //スコア加算は1固定

  if( ret ){
    LINK_FESTIVAL_TopScoreUpdate( wk, 0, wk->mission.my_score, NULL );
  }
  return ret;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッション参加人数取得
 */
//---------------------------------------------------------
int LINK_FESTIVAL_GetMissionJoinNum( LINK_FESTIVAL* wk )
{
  if( FEST_MISSION_IsEntry( &wk->mission ) == FALSE ){
    return 0;
  }
  return wk->log_num+1;
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
  const FEST_BEACON* beacon = &fmwk->beacon;

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
    wk->log_idx, wk->log_num, beacon->m_id, fmwk->key, beacon->base_param );

  wk->log_idx = (wk->log_idx + 1)%LINKFEST_LOG_MAX;
  if( wk->log_num < LINKFEST_LOG_MAX ){
    ++wk->log_num;
  }
  //エントリースタックにのっていたら削除する
  FMISSION_ENTRY_TBL_DeleteData( wk, fmwk );
  return;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　無視するミッションかチェック
 *
 * @param fmwk フェスビーコン情報を格納したFEST_MISSION構造体型ワークへのポインタ
 */
//---------------------------------------------------------
BOOL LINK_FESTIVAL_IsEgnoreMission( LINK_FESTIVAL* wk, const FEST_MISSION* fmwk )
{
  int i;
  LINKFEST_LOG* log;
  const FEST_BEACON* beacon = &fmwk->beacon;

  IWASAWA_Printf("MissionIsEgnore last( %d,%08x,%d) now(%d,%08x,%d) ",
    wk->last.mission.m_id,wk->last.mission.key, wk->last.mission.base_param,
    beacon->m_id, fmwk->key, beacon->base_param );

  //チュートリアルミッションは無視
  if( fmwk->beacon.m_id == 0 ){
    OS_FPrintf(OSPRINT_LINKFEST_PAGE,"FMIS Egnore0 m_id=%d, key=%08x, param=%d\n",
      beacon->m_id, fmwk->key, beacon->base_param );
    return TRUE;
  }
  //最後のミッションと同一か？
  if( wk->last.mission.m_id != FMISSION_ID_NULL &&
      log_mission_IsSame( &wk->last, fmwk ))
  {
    OS_FPrintf(OSPRINT_LINKFEST_PAGE,"FMIS Egnore1 m_id=%d, key=%08x, param=%d\n",
      beacon->m_id, fmwk->key, beacon->base_param );
    return TRUE;
  }
  //無視ログ登録されているか？
  if( log_CheckEgnoreLog( wk, fmwk )){
    OS_FPrintf(OSPRINT_LINKFEST_PAGE,"FMIS Egnore2 m_id=%d, key=%08x, param=%d\n",
      beacon->m_id, fmwk->key, beacon->base_param );
    return TRUE;
  }
  //制限時間が一定以上か？
  if( beacon->time < LINKFEST_LIFE_LIMIT ){
    OS_FPrintf(OSPRINT_LINKFEST_PAGE,"FMIS Egnore3 m_id=%d, key=%08x, param=%d time=%dsec\n",
      beacon->m_id, fmwk->key, beacon->base_param, beacon->time );
    return TRUE;
  }
  return FALSE;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル トップスコアアップデート
 */
//---------------------------------------------------------
void LINK_FESTIVAL_TopScoreUpdate( LINK_FESTIVAL* wk, u16 tr_id, u16 score, STRBUF* name )
{
  LINKFEST_LOG_TOP_SCORE* tsp = wk->top_score;

  if( name == NULL ){ //プレイヤー
    MYSTATUS* my = GAMEDATA_GetMyStatus( wk->gdata );

    if( tsp->score >= wk->mission.my_score ){
      return;
    }
    tsp->score = wk->mission.my_score;
    tsp->tr_id = MyStatus_GetID_Low( my );
    MyStatus_CopyNameString( my, tsp->name );
  }else{
    if( tsp->score >= score ){
      return; //未更新
    }
    tsp->score = score;
    tsp->tr_id = tr_id; 
    GFL_STR_CopyBuffer( tsp->name, name );
  }
  wk->top_score_update_f = TRUE;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル トップスコアアップデートチェック
 *
 * トップスコアが更新されたかどうかチェック
 */
//---------------------------------------------------------
BOOL LINK_FESTIVAL_TopScoreIsUpdate( LINK_FESTIVAL* wk )
{
  return wk->top_score_update_f;
}

//---------------------------------------------------------
/*
 *  @brief リンクフェスティバル 今のトップスコアが自分のものかチェック
 */
//---------------------------------------------------------
BOOL LINK_FESTIVAL_TopScoreIsMine( LINK_FESTIVAL* wk )
{
  MYSTATUS* my = GAMEDATA_GetMyStatus( wk->gdata );
  return ( wk->top_score->tr_id == MyStatus_GetID_Low( my ) );
}

//---------------------------------------------------------
/*
 *  @brief リンクフェスティバル トップスコアを取得
 *
 *  @param  LINKFEST_LOG_TOP_SCORE構造体型ワークポインタ(LINKFEST_LOG_TOP_SCORE_Createで生成してください)
 *
 *  この関数を呼び出すと、アップデートチェック用フラグが落ちるので注意してください
 */
//---------------------------------------------------------
void LINK_FESTIVAL_TopScoreGet( LINK_FESTIVAL* wk, LINKFEST_LOG_TOP_SCORE* tsp )
{
  tsp->tr_id = wk->top_score->tr_id;
  tsp->score = wk->top_score->score;
  GFL_STR_CopyBuffer( tsp->name, wk->top_score->name );
  wk->top_score_update_f = FALSE;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル トップスコア格納ワーク生成
 *
 * LINKFEST_LOT_TOP_SCORE_Delete()で開放してください
 */
//---------------------------------------------------------
LINKFEST_LOG_TOP_SCORE* LINKFEST_LOG_TOP_SCORE_Create( HEAPID heapID )
{
  LINKFEST_LOG_TOP_SCORE* tsp = GFL_HEAP_AllocClearMemory( heapID, sizeof(LINKFEST_LOG_TOP_SCORE) );
  tsp->name = GFL_STR_CreateBuffer( PERSON_NAME_SIZE+EOM_SIZE, heapID ); //<人名格納領域を初期化

  return tsp;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル トップスコア格納ワーク開放
 */
//---------------------------------------------------------
void LINKFEST_LOG_TOP_SCORE_Delete( LINKFEST_LOG_TOP_SCORE* tsp )
{
  GFL_STR_DeleteBuffer( tsp->name );
  MI_CpuClear8( tsp, sizeof(LINKFEST_LOG_TOP_SCORE));

  GFL_HEAP_FreeMemory( tsp );
}


//=========================================================================
//サブルーチン
//=========================================================================
//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ミッションタイプごとにオーバーレイロード&アンロード
 */
//---------------------------------------------------------
static void lnkfes_OverlayLoadUnload( const FEST_MISSION* m_src, BOOL load_f )
{
  FSOverlayID ol_nw;

  switch( m_src->arc.m_type ){
  case FMISSION_TYPE_ITEM:
    ol_nw = FS_OVERLAY_ID( notwifi_fes_gimmick_item );
    break;
  case FMISSION_TYPE_NPC:
    ol_nw = FS_OVERLAY_ID( notwifi_fes_gimmick_npc );
    break;
  case FMISSION_TYPE_EVENT:
    ol_nw = FS_OVERLAY_ID( notwifi_fes_gimmick_event );
    break;
  default:
    return;
  }
  if( load_f ){
    GFL_OVERLAY_Load( ol_nw );
  }else{
    GFL_OVERLAY_Unload( ol_nw );
  }
}


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
#ifdef PM_DEBUG
  if( wk->deb_spot_mode_f ){
    //配信スポットエミュレーション 
    GAMEBEACON_Set_DistributionFestMission();
  }else{
    GAMEBEACON_Set_FestMission();
  }
#else
  GAMEBEACON_Set_FestMission();
#endif
  wk->beacon_timer = wk->beacon_wait;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル トップスコア格納領域クリア
 */
//---------------------------------------------------------
static void topscore_Clear( LINK_FESTIVAL* wk, LINKFEST_LOG_TOP_SCORE* tsp )
{
  MYSTATUS* my = GAMEDATA_GetMyStatus( wk->gdata );
  //プレイヤー名で初期化しておく
  MyStatus_CopyNameString( my, tsp->name );

  tsp->tr_id = MyStatus_GetID_Low(my);
  tsp->score = 0;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ログクリア
 */
//---------------------------------------------------------
static void log_ClearAll( LINK_FESTIVAL* wk )
{
  int i;

  topscore_Clear( wk, wk->top_score );

  for(i = 0;i < LINKFEST_LOG_MAX;i++){
    LINKFEST_LOG_Clear( &wk->log[i]);
  }

  wk->log_num = 0;
  wk->log_idx = 0;
  wk->beacon_timer = 0; 
  wk->finfo_total_score = 0;
  wk->finfo_my_score = 0;
  wk->finfo_bonus = 0;
  wk->now_score = 0;
  wk->tmap_root = LINK_FESTIVAL_TMAP_ROOT_NONE; //無効値で初期化
  MI_CpuClear8( wk->m_gmk_work, sizeof(u8)*FMISSION_WORK_AREA_LEN);

  wk->top_score_update_f = FALSE;
  
  //ミッションログ領域クリア
  FMISSION_LOG_TBL_Clear( &wk->mlog_stack_tbl );
  FMISSION_LOG_TBL_Clear( &wk->mlog_view_tbl );
  FMISSION_GPOWER_TBL_Clear( &wk->mlog_gpower_tbl );
  LINK_FESTIVAL_MissionLiveStatusSet( wk, FMISSION_LIVE_STATUS_NULL );

#ifdef PM_DEBUG
  wk->deb_time_stop_f = FALSE;
  wk->deb_spot_mode_f = FALSE;
  wk->deb_log_stack_check_throw_f = FALSE;
#endif
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
  log->mission.key = fmwk->key;
  log->mission.id_param = fmwk->beacon.id_param;
}


//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ミッションログと同じミッションか？
 */
//---------------------------------------------------------
static BOOL log_mission_IsSame( LINKFEST_LOG* log, const FEST_MISSION* fmwk)
{
  if( log->mission.key != fmwk->key ||
      log->mission.id_param != fmwk->beacon.id_param ){
      return FALSE; //違うミッション
  }
  return TRUE;
}


//以下デバッグルーチン
#ifdef PM_DEBUG

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ログスタックチェックスキップフラグをセットする
 */
//---------------------------------------------------------
void DEB_LINKFEST_SetLogStackCheckThrowFlag( LINK_FESTIVAL* wk, BOOL flag )
{
  wk->deb_log_stack_check_throw_f = flag;
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ログスタックチェックスキップフラグをゲットする
 */
//---------------------------------------------------------
BOOL DEB_LINKFEST_GetLogStackCheckThrowFlag( LINK_FESTIVAL* wk )
{
  return wk->deb_log_stack_check_throw_f;
}

#endif  //PM_DEBUG
