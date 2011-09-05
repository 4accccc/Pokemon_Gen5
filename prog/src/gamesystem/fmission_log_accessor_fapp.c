//====================================================================================
/**
 *  @file   fmission_log_accessor_fapp.c
 *  @brief  フェスミッション詳細画面　データアクセス(フィールド FMISSION_APPオーバーレイ)
 *  @author Miyuki Iwasawa
 *  @date   11.09.05
 *
 *  fmission_logのルーチンのうち、フェスミッション詳細画面アプリ上からだけ
 *  アクセスできれば良い関数を集めてある
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
#include "savedata/playtime.h"
#include "field/research_team_def.h"
#include "system/gfl_use.h"
#include "field/zonedata.h"
#include "savedata/resonance_resort_sv.h"

#include "game_beacon_local.h"
#include "debug/debug_flg.h"

#include "link_festival_local.h"
#include "gamesystem/fmission_log.h"
#include "gamesystem/fmission_log_accessor_fapp.h"

static void log_Clear( FMISSION_LOG_LATEST_PARAM* prm );

//---------------------------------------------------------
/*
 * @brief フェス最新ログ格納ワーク生成
 *
 * FMISSION_LOG_LATEST_Delete()で開放してください
 */
//---------------------------------------------------------
FMISSION_LOG_LATEST* FMISSION_LOG_LATEST_Create( HEAPID heapID )
{
  FMISSION_LOG_LATEST* lp;
  int i;

  lp = GFL_HEAP_AllocClearMemory( heapID, sizeof(FMISSION_LOG_LATEST) );

  for( i = 0; i < FMISSION_LOG_LATEST_NUM_MAX; i++ ){
    lp->log[i].name = GFL_STR_CreateBuffer( PERSON_NAME_SIZE+EOM_SIZE, heapID ); //<人名格納領域を初期化
  }
  lp->num = 0;

  return lp;
}

//---------------------------------------------------------
/*
 * @brief フェス最新ログ格納ワーク開放
 */
//---------------------------------------------------------
void FMISSION_LOG_LATEST_Delete( FMISSION_LOG_LATEST* lp )
{
  int i;

  for( i = 0; i < FMISSION_LOG_LATEST_NUM_MAX; i++){
    GFL_STR_DeleteBuffer( lp->log[i].name );
    lp->log[i].name = NULL;
  }
  lp->num = 0;
  GFL_HEAP_FreeMemory( lp );
  lp = NULL;
}


//------------------------------------------------------------------
/*
 *  ログスタック 最新3件取得
 *
 *  @param  tbl     FMISSION_LOG_TBL型構造体へのポインタ
 *  @param  lp   取得するログ情報を格納するFMISSION_LOG_LATEST型ワークへのポインタ
 *
 *  @retval TRUE    データ取得成功
 *  @retval FALSE   スタックテーブルにデータがない
 */
//------------------------------------------------------------------
BOOL FMISSION_LOG_StackTbl_GetLogLatest( FMISSION_LOG_TBL* tbl, FMISSION_LOG_LATEST* lp )
{
  int i,j;
  FMISSION_LOG_LATEST_PARAM* prm;

  lp->num = 0;
  for( i = 0;i < FMISSION_LOG_LATEST_NUM_MAX;i++){
    log_Clear( &lp->log[i] );
  }
  if( tbl->entry_num == 0 ){
    return FALSE;
  }

  for( i = tbl->entry_num-1; i >= 0; i-- ){ //後ろが最新なので後ろからループ
    //tr_idチェック
    for( j = 0; j < lp->num; j++ ){
      if( lp->log[j].tr_id == tbl->log[i].tr_id ){
        continue; //すでに格納済み
      }
    }
    prm = &lp->log[lp->num];
    prm->tr_id = tbl->log[i].tr_id;
    prm->score = tbl->log[i].score;
    FMISSION_LOG_GetName( &tbl->log[i], prm->name );

    if( ++lp->num >= FMISSION_LOG_LATEST_NUM_MAX ){
      break;  //3件取得したので終わり
    }
  }
  return TRUE;
}


//------------------------------------------------------------------
/*
 *  @brief  最新ログクリア
 */
//------------------------------------------------------------------
static void log_Clear( FMISSION_LOG_LATEST_PARAM* prm )
{
  GFL_STR_ClearBuffer( prm->name );
  prm->tr_id = 0;
  prm->score = 0;
}
