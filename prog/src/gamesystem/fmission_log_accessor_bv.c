//====================================================================================
/**
 *  @file   fmission_log_accessor_bv.c
 *  @brief  フェスミッションログ　データアクセス(フィールド BEACON_VIEWオーバーレイ)
 *  @author Miyuki Iwasawa
 *  @date   11.03.23
 *
 *  fmission_logのルーチンのうち、フィールド上からだけアクセスできれば良い関数を集めてある
 *  BeaconViewバージョン
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
#include "gamesystem/fmission_log.h"
#include "gamesystem/fmission_log_accessor_bv.h"

//------------------------------------------------------------------
/*
 *  ミッションエントリーデータテーブル データ取得
 *
 *  @param  tbl       FMISSION_ENTRY_TBL型構造体へのポインタ
 *  @param  fmwk      ミッションパラメータを受けるSTRBUFへのポインタ
 *  @param  name_buf  ミッション配布者の名前を受けるSTRBUFへのポインタ
 *
 *  @retval TRUE      データ取得成功
 *  @retval FALSE     データ取得失敗
 *
 *  情報取得に成功した場合、テーブル内のデータが前詰めされます
 */
//------------------------------------------------------------------
BOOL FMISSION_ENTRY_TBL_GetData( FMISSION_ENTRY_TBL* tbl, FEST_MISSION* fmwk, STRBUF* name_buf )
{
  BOOL ret = FALSE;

  if( tbl->entry_num == 0 ){
    return FALSE;
  }
  if( tbl->entry[0].mission.life > (30*60) ){
    GFL_STR_SetStringCodeOrderLength( name_buf, tbl->entry[0].name, PERSON_NAME_SIZE+EOM_SIZE );
    *fmwk = tbl->entry[0].mission;
    ret = TRUE;
  }else{
    GFL_STR_ClearBuffer( name_buf );
  }

  //テーブル前詰め
  GFL_STD_MemCopy32( &tbl->entry[1], &tbl->entry[0], sizeof(FMISSION_ENTRY)*FMISSION_ENTRY_TBL_END);
  FMISSION_ENTRY_Clear( &tbl->entry[FMISSION_ENTRY_TBL_END] );
  --tbl->entry_num;

  return ret;
}

