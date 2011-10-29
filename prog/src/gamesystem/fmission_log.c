//====================================================================================
/**
 *  @file   fmission_log.c
 *  @brief  フェスミッションログ　管理(常駐)
 *  @author Miyuki Iwasawa
 *  @date   11.03.09
 */
//====================================================================================

#include <gflib.h>
#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_data.h"
#include "gamesystem/game_beacon.h"
#include "gamesystem/game_beacon_types.h"
#include "gamesystem/link_festival.h"
#include "gamesystem/fest_mission.h"
#include "gamesystem/g_power.h"
#include "msg/msg_beacon_status.h"
#include "savedata/encount_sv.h"
#include "savedata/misc.h"
#include "net/net_whpipe.h"
#include "savedata/my_pms_data.h"
#include "app/townmap_util.h"
#include "net_app\union\union_beacon_tool.h"
#include "savedata/playtime.h"
#include "field/research_team_def.h"
#include "system/gfl_use.h"
#include "field/zonedata.h"
#include "savedata/resonance_resort_sv.h"
#include "savedata/mystatus.h"

#include "game_beacon_local.h"
#include "debug/debug_flg.h"

#include "link_festival_local.h"

typedef enum{
  ENTRY_UPDATE0,
  ENTRY_UPDATE1,
  ENTRY_UPDATE2,
  ENTRY_NEW,    //新規
  ENTRY_NONE,   //積まない
}ENTRY_CHECK;

static FMISSION_LOG_ACTION log_GetLogActionFromBeacon( const GAMEBEACON_INFO *info, FMISSION_LOG_SET_MODE mode );
static BOOL log_MakeLogFromBeacon( FMISSION_LOG* p_log, const GAMEBEACON_INFO *info, int score, FMISSION_LOG_SET_MODE mode, BOOL mine_f );
static ENTRY_CHECK entry_CheckStock( LINK_FESTIVAL* fest_sys, const GAMEBEACON_INFO *info );

//------------------------------------------------------------------
/*
 *  ログクリア
 */
//------------------------------------------------------------------
void FMISSION_LOG_Clear( FMISSION_LOG* log )
{
  GFL_STD_MemClear( log, sizeof(FMISSION_LOG));
  {
    STRCODE eom = GFL_STR_GetEOMCode();
    GFL_STD_MemFill16( log->name, eom, (PERSON_NAME_SIZE+EOM_SIZE)*sizeof(STRCODE));
    GFL_STD_MemFill16( log->str, eom, (GAMEBEACON_FREEWORD_MESSAGE_LEN)*sizeof(STRCODE) );
  }
}

//------------------------------------------------------------------
/**
 * ログパラメータ取得　名前
 *
 * @param   p_log	    	ログ情報
 * @param   s_buf       PERSON_NAME_SIZE+EOM_SIZEを格納可能なSTRBUFのポインタ
 */
//------------------------------------------------------------------
void FMISSION_LOG_GetName( FMISSION_LOG* p_log, STRBUF* s_buf )
{
  GFL_STR_SetStringCodeOrderLength( s_buf, p_log->name, PERSON_NAME_SIZE+EOM_SIZE );
}

//------------------------------------------------------------------
/*
 *  ログテーブルクリア
 */
//------------------------------------------------------------------
void FMISSION_LOG_TBL_Clear( FMISSION_LOG_TBL* tbl )
{
  int i;

  for(i = 0;i < FMISSION_LOG_MAX;i++){
    FMISSION_LOG_Clear( &tbl->log[i] );
  }
  tbl->ring_top = 0;
  tbl->entry_num = 0;
  tbl->view_top = 0;
}

//------------------------------------------------------------------
/*
 *  ログスタック データセット
 *
 *  @param  tbl   FMISSION_LOG_TBL型構造体へのポインタ
 *  @param  info  セットするビーコン情報
 *  @param  new_f 新規エントリーならTRUE
 *  @return データをセットした配列の参照index
 *
 *  データがいっぱいの場合、先頭のデータをところてん式に押し出して末尾にセット
 */
//------------------------------------------------------------------
int FMISSION_LOG_TBL_SetLog( FMISSION_LOG_TBL* tbl, const FMISSION_LOG* p_log )
{
  if( tbl->entry_num == FMISSION_LOG_MAX ){
    //スタックテーブル前詰め
    GFL_STD_MemCopy16( &tbl->log[1], &tbl->log[0], sizeof(FMISSION_LOG)*FMISSION_LOG_END);
    tbl->log[FMISSION_LOG_END] = *p_log;
  }else{
    tbl->log[tbl->entry_num++] = *p_log;
  }
  return (tbl->entry_num-1);
}

//------------------------------------------------------------------
/*
 *  ビーコンからミッションログパラメータを取得してスタックにセット
 *
 *  @param  tbl     FMISSION_LOG_TBL型構造体へのポインタ
 *  @param  info    セットするビーコン情報
 *  @param  score   ログに保存するスコア
 *  @param  mode    ログのセットモード enum FMISSION_LOG_SET_MODE
 *  @param  mine_f  自分の行動ログならTRUE
 *
 *  データがいっぱいの場合、先頭のデータをところてん式に押し出して末尾にセット
 */
//------------------------------------------------------------------
void FMISSION_LOG_TBL_SetLogFromBeacon( FMISSION_LOG_TBL* tbl, const GAMEBEACON_INFO *info, int score, FMISSION_LOG_SET_MODE mode, BOOL mine_f )
{
  FMISSION_LOG  log;

  if( log_MakeLogFromBeacon( &log, info, score,mode, mine_f )){
    FMISSION_LOG_TBL_SetLog( tbl, &log );
  }
}

//------------------------------------------------------------------
/*
 *  ミッション開催ログをスタックにセット
 *
 *  @param  tbl     FMISSION_LOG_TBL型構造体へのポインタ
 *  @param  GAMEDATA*
 *
 *  データがいっぱいの場合、先頭のデータをところてん式に押し出して末尾にセット
 */
//------------------------------------------------------------------
void FMISSION_LOG_TBL_SetLogMissionOpen( FMISSION_LOG_TBL* tbl, GAMEDATA* gdata )
{
  FMISSION_LOG  log;
  MYSTATUS* my =  GAMEDATA_GetMyStatus( gdata );
 
  FMISSION_LOG_Clear(&log); 
  
  log.action = FMISSION_LOG_ACT_OPEN;
  log.mine_f = TRUE;
  log.tr_id =  MyStatus_GetID_Low( my );
  log.sex = MyStatus_GetMySex( my );

  log.tr_view = MyStatus_GetTrainerView( my );  //0-15で返ってくる
  if( log.sex == PM_FEMALE ){
    log.tr_view -= UNION_VIEW_INDEX_WOMAN_START;  //男女0-7に丸める
  }
  MyStatus_CopyNameStrCode( my, log.name, PERSON_NAME_SIZE+EOM_SIZE);

  FMISSION_LOG_TBL_SetLog( tbl, &log );
}



//------------------------------------------------------------------
/*
 *  デルパワーパラメータクリア
 */
//------------------------------------------------------------------
void FMISSION_GPOWER_Clear( FMISSION_GPOWER* power )
{
  GFL_STD_MemClear( power, sizeof(FMISSION_GPOWER));
  power->gpower_id = GPOWER_ID_NULL;
  {
    STRCODE eom = GFL_STR_GetEOMCode();
    GFL_STD_MemFill16( power->name, eom, (PERSON_NAME_SIZE+EOM_SIZE)*sizeof(STRCODE));
  }
}

//------------------------------------------------------------------
/*
 *  デルパワーテーブルクリア
 */
//------------------------------------------------------------------
void FMISSION_GPOWER_TBL_Clear( FMISSION_GPOWER_TBL* tbl )
{
  int i;

  for(i = 0;i < FMISSION_GPOWER_TBL_MAX;i++){
    FMISSION_GPOWER_Clear( &tbl->power[i] );
  }
  tbl->entry_num = 0;
}

//------------------------------------------------------------------
/*
 *  デルパワースタック データセット
 *
 *  @param  tbl   FMISSION_GPOWER_TBL型構造体へのポインタ
 *  @param  info  セットするビーコン情報
 *  @return データをセットしたらTRUE
 *
 *  データがいっぱいの場合、積むのを諦める
 */
//------------------------------------------------------------------
BOOL FMISSION_GPOWER_TBL_Set( FMISSION_GPOWER_TBL* tbl, const GAMEBEACON_INFO *info )
{
  FMISSION_GPOWER power;
  GPOWER_ID id;

  if( tbl->entry_num == FMISSION_GPOWER_TBL_MAX ){
    return FALSE; //いっぱいなら積まない
  }
  FMISSION_GPOWER_Clear( &power );

  //配布パワーを受け取る(ミッションパワーはFMISSION_LOGからログ画面で起動)
  switch( info->action.action_no ){
  case GAMEBEACON_ACTION_DISTRIBUTION_GPOWER:
    id = info->action.distribution.gpower_id;
    power.spot_f = TRUE;
    break;
  default:
    return FALSE;
  }
#if 0
  {
    POWER_CONV_DATA* gpower_data = GPOWER_PowerData_LoadAlloc( GFL_HEAP_LOWID(GFL_HEAPID_APP ) );
    GPOWER_ID ret_id = GPOWER_Check_OccurID( id, gpower_data );

    GPOWER_PowerData_Unload( gpower_data );

    //現在進行形で発動中のパワーは受け取らない
    if( ret_id == id ){
      return FALSE; //同じパワー発動中は捨てる
    }
  }
#endif
  //名前(info->nameはEOM抜き)
  GFL_STD_MemCopy16( info->name, power.name, PERSON_NAME_SIZE*sizeof(STRCODE));
  power.gpower_id = id;

  tbl->power[tbl->entry_num++] = power;

  return TRUE;
}

//------------------------------------------------------------------
/*
 *  ミッションエントリーデータ パラメータクリア
 */
//------------------------------------------------------------------
void FMISSION_ENTRY_Clear( FMISSION_ENTRY* entry )
{
  GFL_STD_MemClear( entry, sizeof(FMISSION_ENTRY));
  FEST_MISSION_Clear( &entry->mission );

  {
    STRCODE eom = GFL_STR_GetEOMCode();
    GFL_STD_MemFill16( entry->name, eom, (PERSON_NAME_SIZE+EOM_SIZE)*sizeof(STRCODE));
  }
}

//------------------------------------------------------------------
/*
 *  ミッションエントリーデータ テーブルクリア
 */
//------------------------------------------------------------------
void FMISSION_ENTRY_TBL_Clear( FMISSION_ENTRY_TBL* tbl )
{
  int i;

  for(i = 0;i < FMISSION_ENTRY_TBL_MAX;i++){
    FMISSION_ENTRY_Clear( &tbl->entry[i] );
  }
  tbl->entry_num = 0;
}

//------------------------------------------------------------------
/*
 *  ミッションエントリーデータスタック データセット
 *
 *  @param  tbl   FMISSION_ENTRY_TBL型構造体へのポインタ
 *  @param  info  セットするビーコン情報
 *  @return データをセットしたらTRUE
 *
 *  データがいっぱいの場合、一番古いデータを押し出して積む
 */
//------------------------------------------------------------------
void FMISSION_ENTRY_TBL_Set( LINK_FESTIVAL* fest_sys, const GAMEBEACON_INFO *info )
{
  FMISSION_ENTRY entry;
  ENTRY_CHECK ret;
  FMISSION_ENTRY_TBL* tbl = &fest_sys->mlog_entry_tbl;

  //アクションチェック
  if(GAMEBEACON_Check_FestMissionWithParam(info) == FALSE){
    return;
  }
  ret = entry_CheckStock( fest_sys, info );

  if(ret == ENTRY_NONE){
    return;
  }

  FMISSION_ENTRY_Clear(&entry);

  //ビーコン継承パラメータセット
  entry.mission.beacon = info->action.fmission;

  //コモンアーカイブデータセット
  LINK_FESTIVAL_GetMissionArcCommonData( fest_sys, info->action.fmission.m_id, &entry.mission.arc );
 
  //名前(info->nameはEOM抜き)
  GFL_STD_MemCopy16( info->name, entry.name, PERSON_NAME_SIZE*sizeof(STRCODE));

  //積んだ時点のRTCを覚えておく
  FEST_MISSION_SetRTC( &entry.mission );

  if( ret == ENTRY_NEW ){
    if( tbl->entry_num == FMISSION_ENTRY_TBL_MAX ){
      //古いデータを押し出して積む
      GFL_STD_MemCopy32( &tbl->entry[1], &tbl->entry[0], sizeof(FMISSION_ENTRY)*FMISSION_ENTRY_TBL_END);
      tbl->entry[FMISSION_ENTRY_TBL_END] = entry;
    }else{
      tbl->entry[tbl->entry_num++] = entry;
    }
  }else{
    //上書き
    tbl->entry[ret] = entry;
  }
  GAMEBEACON_SetCGearInfoFlag( GAMEBEACON_CGEAR_INFO_FMISSION );
}

//------------------------------------------------------------------
/*
 *  ミッションエントリーデータスタック アップデート 
 *
 *  @param  tbl   FMISSION_ENTRY_TBL型構造体へのポインタ
 *
 *  格納したミッションのライフのエントリー寿命をチェックし、寿命が尽きたらテーブルから削除 
 */
//------------------------------------------------------------------
void FMISSION_ENTRY_TBL_Update( LINK_FESTIVAL* fest_sys )
{
  int i,end;
  FMISSION_ENTRY_TBL* tbl = &fest_sys->mlog_entry_tbl;

  for(i = 0;i < tbl->entry_num; i++ ){
    if( FEST_MISSION_IsLifeLeftForEntry( &tbl->entry[i].mission ) == FALSE ){
      //エントリー寿命が尽きたので削除
      end = tbl->entry_num-1;
      if( i < end ){
        GFL_STD_MemCopy32( &tbl->entry[i+1], &tbl->entry[i], sizeof(FMISSION_ENTRY)*(end-i));
        if( end > 0){
          --i;  //前詰めしたのでサーチ位置もずらしておく
        }
      }
      FMISSION_ENTRY_Clear( &tbl->entry[tbl->entry_num-1] );
      --tbl->entry_num;

      if( tbl->entry_num == 0 ){
        GAMEBEACON_ResetCGearInfoFlag( GAMEBEACON_CGEAR_INFO_FMISSION );
      }
    }
  }
}


//------------------------------------------------------------------
/*
 *  ミッションエントリーデータスタック 指定データと同じミッションをテーブルから削除する
 *
 *  @param  tbl   FMISSION_ENTRY_TBL型構造体へのポインタ
 *
 *  格納したミッションを指定ミッションを比較し、同じミッションだったらテーブルから削除 
 */
//------------------------------------------------------------------
void FMISSION_ENTRY_TBL_DeleteData( LINK_FESTIVAL* fest_sys, const FEST_MISSION* fmwk )
{
  int i,end;
  FMISSION_ENTRY_TBL* tbl = &fest_sys->mlog_entry_tbl;

  for(i = 0;i < tbl->entry_num; i++ ){
    if( FEST_BEACON_IsSame( &tbl->entry[i].mission.beacon, &fmwk->beacon ) ){
      //同じミッションなので削除
      end = tbl->entry_num-1;
      if( i < end ){
        GFL_STD_MemCopy32( &tbl->entry[i+1], &tbl->entry[i], sizeof(FMISSION_ENTRY)*(end-i));
        if( end > 0){
          --i;  //前詰めしたのでサーチ位置もずらしておく
        }
      }
      FMISSION_ENTRY_Clear( &tbl->entry[tbl->entry_num-1] );
      --tbl->entry_num;

      if( tbl->entry_num == 0 ){
        GAMEBEACON_ResetCGearInfoFlag( GAMEBEACON_CGEAR_INFO_FMISSION );
      }
    }
  }
}

//=================================================================================
///サブルーチン
//=================================================================================

//------------------------------------------------------------------
/*
 *  ビーコンからミッションログアクションを取得
 *
 *  @param  info  セットするビーコン情報
 *  @param  new_f 新規エントリーならTRUE
 */
//------------------------------------------------------------------
static FMISSION_LOG_ACTION log_GetLogActionFromBeacon( const GAMEBEACON_INFO *info, FMISSION_LOG_SET_MODE mode )
{
  switch( mode ){
  case FMISSION_LOG_SET_NEW:
    return FMISSION_LOG_ACT_NEW_ENTRY;  
  case FMISSION_LOG_SET_MAP:
    return FMISSION_LOG_ACT_MAP_CHANGE;
  }
  if( info->action.action_no == GAMEBEACON_ACTION_FMISSION ||
      info->action.action_no == GAMEBEACON_ACTION_DISTRIBUTION_FMISSION ){
    return FMISSION_LOG_ACT_NULL; //積まない
  }
  return FMISSION_LOG_ACT_GRP_BEACON + (info->action.action_no-GAMEBEACON_ACTION_FEST_BEACON_LOG_ACT_START );
}

//------------------------------------------------------------------
/*
 *  ビーコンからミッションログパラメータを取得
 *
 *  @param  info  セットするビーコン情報
 *  @param  new_f 新規エントリーならTRUE
 */
//------------------------------------------------------------------
static BOOL log_MakeLogFromBeacon( FMISSION_LOG* p_log, const GAMEBEACON_INFO *info, int score, FMISSION_LOG_SET_MODE mode, BOOL mine_f )
{
  FMISSION_LOG_ACTION action = log_GetLogActionFromBeacon( info, mode );

  if( action == FMISSION_LOG_ACT_NULL ){
    return FALSE;
  }
  FMISSION_LOG_Clear(p_log); 

  p_log->action = action;
  p_log->mine_f = mine_f;
  p_log->tr_id = info->trainer_id;
  p_log->sex = info->sex;
  p_log->tr_view = info->trainer_view;
  p_log->score = score;

  //名前(info->nameはEOM抜き)
  GFL_STD_MemCopy16( info->name, p_log->name, PERSON_NAME_SIZE*sizeof(STRCODE));

  switch( action ){
  case FMISSION_LOG_ACT_FREEWORD:
    //EOM込み
    GFL_STD_MemCopy16( info->action.freeword.freeword_message, p_log->str, GAMEBEACON_FREEWORD_MESSAGE_LEN*sizeof(STRCODE));
    p_log->param = info->action.freeword.shortcut_id;
    break;
  case FMISSION_LOG_ACT_POKE_DOWN:
  case FMISSION_LOG_ACT_POKE_HP_LITTLE:
  case FMISSION_LOG_ACT_POKE_PP_LITTLE:
    //EOM込み
    GFL_STD_MemCopy16( info->action.normal.nickname, p_log->str, MONS_NAME_SIZE*sizeof(STRCODE));
    p_log->param = 0;
    break;
  case FMISSION_LOG_ACT_MAP_CHANGE:
    p_log->param = info->townmap_root_zone_id;
    break;
  default:
    p_log->param = info->action.fmission.act_param;
    break;
  }
  return TRUE;
}

//------------------------------------------------------------------
/*
 *  ミッションエントリーデータをストックするかチェック 
 *
 *  @param  info  セットするビーコン情報
 *  @param  new_f 新規エントリーならTRUE
 */
//------------------------------------------------------------------
static ENTRY_CHECK entry_CheckStock( LINK_FESTIVAL* fest_sys, const GAMEBEACON_INFO *info )
{
  int i;
  const FEST_BEACON* beacon;
  FMISSION_ENTRY_TBL* tbl = &fest_sys->mlog_entry_tbl;

  //アクションチェック
  if(GAMEBEACON_Check_FestMissionWithParam(info) == FALSE){
    return ENTRY_NONE;
  }
  beacon = &info->action.fmission;

  //スタックにすでに積まれているミッションと同一なら無視か上書き
  for( i = 0;i < tbl->entry_num;i++){
    if( FEST_BEACON_IsSame( &tbl->entry[i].mission.beacon, beacon )){
      if( tbl->entry[i].mission.beacon.time < beacon->time ){  //新しい方がライフが長ければ上書き
        return ENTRY_UPDATE0+i;
      }
      return ENTRY_NONE;
    }
  }
  //現在のミッションと同一なら積まない
  if( LINK_FESTIVAL_IsMissionEntry( fest_sys )){
    if( FEST_BEACON_IsSame( beacon, fest_sys->m_beacon )){
      return ENTRY_NONE;
    }
  }
  //無視対象のミッションなら積まない
  if( LINK_FESTIVAL_IsEgnoreMission( fest_sys, beacon )){
    return ENTRY_NONE;
  }
  return ENTRY_NEW;
}
