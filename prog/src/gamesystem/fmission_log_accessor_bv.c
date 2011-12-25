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
 *  @param  fest_sys  LINK_FESTIVAL型構造体へのポインタ
 *  @param  fmwk      ミッションパラメータを受けるSTRBUFへのポインタ
 *  @param  name_buf  ミッション配布者の名前を受けるSTRBUFへのポインタ
 *
 *  @retval TRUE      データ取得成功
 *  @retval FALSE     データ取得失敗
 *
 *  情報取得に成功しても、テーブル内のデータは消えません。
 *  適切な処理後は FMISSION_ENTRY_TBL_DeleteData()を呼び出して不要になったデータを消してください
 */
//------------------------------------------------------------------
BOOL FMISSION_ENTRY_TBL_GetData( LINK_FESTIVAL* fest_sys, FEST_MISSION* fmwk, STRBUF* name_buf )
{
  int i;
  BOOL ret = FALSE;
  FMISSION_ENTRY_TBL* tbl = &fest_sys->mlog_entry_tbl;

  if( tbl->entry_num == 0 ){
    return FALSE;
  }
  //配信ミッションを優先チェック
  for( i = 0;i < tbl->entry_num ;i++ ){
    if( tbl->entry[i].mission.beacon.distribution_f &&
        FEST_MISSION_IsLifeLeftForEntry( &tbl->entry[i].mission ) ){  //残り時間が30秒以上じゃないと返さない
    
      GFL_STR_SetStringCodeOrderLength( name_buf, tbl->entry[i].name, PERSON_NAME_SIZE+EOM_SIZE );
      *fmwk = tbl->entry[i].mission;
      return TRUE;
    }
  }
  //配信ミッションがなければトップから返す
  for( i = 0;i < tbl->entry_num ;i++ ){
    if( FEST_MISSION_IsLifeLeftForEntry( &tbl->entry[i].mission ) ){  //残り時間が30秒以上じゃないと返さない
    
      GFL_STR_SetStringCodeOrderLength( name_buf, tbl->entry[i].name, PERSON_NAME_SIZE+EOM_SIZE );
      *fmwk = tbl->entry[i].mission;
      return TRUE;
    }
  }
  GFL_STR_ClearBuffer( name_buf );
  return FALSE;
#if 0
  if( FEST_MISSION_IsLifeLeftForEntry( &tbl->entry[0].mission ) ){  //残り時間が30秒以上じゃないと返さない
    GFL_STR_SetStringCodeOrderLength( name_buf, tbl->entry[0].name, PERSON_NAME_SIZE+EOM_SIZE );
    *fmwk = tbl->entry[0].mission;
    ret = TRUE;
  }else{
    GFL_STR_ClearBuffer( name_buf );
  }
  return ret;
#endif
}

