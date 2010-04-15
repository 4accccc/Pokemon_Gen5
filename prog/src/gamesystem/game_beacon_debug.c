/*
 *  @file   game_beacon_debug.c
 *  @brief  ビーコン関連デバッグ関数置き場(フィールド常駐)
 *  @author Miyuki Iwasawa
 *  @date   10.04.15
 */

#ifdef PM_DEBUG

#include <gflib.h>
#include "system\gfl_use.h"
#include "gamesystem/game_data.h"
#include "gamesystem/game_beacon.h"
#include "gamesystem/game_beacon_types.h"
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

#include "gamesystem/game_beacon_debug.h"

//------------------------------------------------------------------
/**
 * デバッグ用：数パターンのビーコンを、トレーナーIDランダムでリクエストする
 */
//------------------------------------------------------------------
void DEBUG_GAMEBEACON_Set_Random(void)
{
  GAMEBEACON_INFO* info = DEBUG_SendBeaconInfo_GetPtr();

  u16 org_id = info->trainer_id;

  info->trainer_id = GFUser_GetPublicRand0(0xFFFF);
  GAMEBEACON_Set_BattlePokeStart( GFUser_GetPublicRand0( MONSNO_END-1 )+1 );
//  info->trainer_id = org_id;
}


#endif  //PM_DEBUG

