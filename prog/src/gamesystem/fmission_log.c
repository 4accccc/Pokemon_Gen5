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
#include "msg/msg_beacon_status.h"
#include "savedata/encount_sv.h"
#include "savedata/misc.h"
#include "net/net_whpipe.h"
#include "savedata/my_pms_data.h"
#include "app/townmap_util.h"
#include "net_app\union\union_beacon_tool.h"
#include "app/research_radar/research_data.h"   //QUESTION_NUM_PER_TOPIC
#include "app/research_radar/question_id.h"
#include "app/research_radar/questionnaire_index.h"
#include "savedata/playtime.h"
#include "field/research_team_def.h"
#include "system/gfl_use.h"
#include "field/zonedata.h"
#include "savedata/resonance_resort_sv.h"

#include "game_beacon_local.h"
#include "debug/debug_flg.h"

#include "link_festival_local.h"

static FMISSION_LOG_ACTION log_GetLogActionFromBeacon( const GAMEBEACON_INFO *info, FMISSION_LOG_SET_MODE mode );
static BOOL log_MakeLogFromBeacon( FMISSION_LOG* p_log, const GAMEBEACON_INFO *info, FMISSION_LOG_SET_MODE mode, BOOL mine_f );

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
    GFL_STD_MemCopy32( &tbl->log[1], &tbl->log[0], sizeof(FMISSION_LOG)*FMISSION_LOG_END);
    tbl->log[FMISSION_LOG_END] = *p_log;
  }else{
    tbl->log[tbl->entry_num++] = *p_log;
  }
  return tbl->entry_num;
}

//------------------------------------------------------------------
/*
 *  ビーコンからミッションログパラメータを取得してスタックにセット
 *
 *  @param  tbl     FMISSION_LOG_TBL型構造体へのポインタ
 *  @param  info    セットするビーコン情報
 *  @param  mode    ログのセットモード enum FMISSION_LOG_SET_MODE
 *  @param  mine_f  自分の行動ログならTRUE
 *
 *  データがいっぱいの場合、先頭のデータをところてん式に押し出して末尾にセット
 */
//------------------------------------------------------------------
void FMISSION_LOG_TBL_SetLogFromBeacon( FMISSION_LOG_TBL* tbl, const GAMEBEACON_INFO *info, FMISSION_LOG_SET_MODE mode, BOOL mine_f )
{
  FMISSION_LOG  log;

  if( log_MakeLogFromBeacon( &log, info, mode, mine_f )){
    FMISSION_LOG_TBL_SetLog( tbl, &log );
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
  if( info->action.action_no == GAMEBEACON_ACTION_FMISSION){
    return FMISSION_LOG_ACT_NULL; //積まない
  }
  return FMISSION_LOG_ACT_WP_START + (info->action.action_no-(GAMEBEACON_ACTION_FEST_BEACON_START+1));
}

//------------------------------------------------------------------
/*
 *  ビーコンからミッションログパラメータを取得
 *
 *  @param  info  セットするビーコン情報
 *  @param  new_f 新規エントリーならTRUE
 */
//------------------------------------------------------------------
static BOOL log_MakeLogFromBeacon( FMISSION_LOG* p_log, const GAMEBEACON_INFO *info, FMISSION_LOG_SET_MODE mode, BOOL mine_f )
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

  //名前(info->nameはEOM抜き)
  GFL_STD_MemCopy16( info->name, p_log->name, PERSON_NAME_SIZE);

  if( info->action.action_no == GAMEBEACON_ACTION_FMISSION_FREEWORD ){
    //EOM込み
    GFL_STD_MemCopy16( info->action.freeword_message, p_log->str, GAMEBEACON_FREEWORD_MESSAGE_LEN);
    p_log->param = 0;
  }else if( info->action.action_no == GAMEBEACON_ACTION_FMISSION_POKE_DOWN ){
    //EOM込み
    GFL_STD_MemCopy16( info->action.normal.nickname, p_log->str, MONS_NAME_SIZE);
    p_log->param = 0;
  }else if( action == FMISSION_LOG_ACT_MAP_CHANGE){
    p_log->param = info->townmap_root_zone_id;
  }else{
    p_log->param = info->action.fmission.param;
  }
  return TRUE;
}

