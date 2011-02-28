//====================================================================
/*
 *  @file fest_mission_data.h
 *  @brief  フェスミッションアーカイブデータ操作
 *  @author Miyuki Iwasawa
 *  @date   11.02.22
 */
//====================================================================

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "arc_def.h"

#include "field/zonedata.h"
#include "sound/pm_sndsys.h"

#include "debug/debug_str_conv.h" // for DEB_STR_CONV_SjisToStrcode

#include "arc/fieldmap/zone_id.h"
#include "poke_tool/monsno_def.h"
#include "tr_tool/trno_def.h"
#include "item/itemsym.h"
#include "waza_tool/wazano_def.h"

#include "savedata/intrude_save.h"
#include "savedata/playtime.h"

#include "gamesystem/link_festival.h"
#include "gamesystem/game_beacon.h"
#include "gamesystem/game_beacon_types.h"
#include "gamesystem/game_beacon_local.h" //許可なく他でinclude禁止

#include "gamesystem/fest_mission_data.h"

//@todo コンバータ作成時にアーカイブとして外部化する
static const FMISSION_ARCDATA DATA_MissionArcData[] = {
  { //タブンネを探せミッション
    FMISSION_TYPE_POKE_SEARCH, 5*60, MONSNO_TABUNNE,
    3, ITEM_DERUDAMA, 100, 0, FMISSION_ENDTYPE_CLEAR, 6
  },
  { //タブンネを探せミッション
    FMISSION_TYPE_POKE_SEARCH, 5*60, MONSNO_TABUNNE,
    10, ITEM_DERUDAMA, 100, 0, FMISSION_ENDTYPE_CLEAR, 6
  },
};

//------------------------------------------------------------
/*
 *  @brief  ミッションデータマネージャー初期化
 */
//------------------------------------------------------------
FMISSION_DATA_MAN*  FMISSION_DATA_Init( HEAPID heapID )
{
  FMISSION_DATA_MAN* man = GFL_HEAP_AllocClearMemory(heapID, sizeof(FMISSION_DATA_MAN));
  return man;
}

//------------------------------------------------------------
/*
 *  @brief  ミッションデータマネージャー開放
 */
//------------------------------------------------------------
void FMISSION_DATA_Release( FMISSION_DATA_MAN* man )
{
  GFL_HEAP_FreeMemory(man);
}

//------------------------------------------------------------
/*
 *  @brief  指定ミッションのパラメータをFEST_MISSION型ワークに取得する
 */
//------------------------------------------------------------
void FMISSION_DATA_GetMissionParam( FMISSION_DATA_MAN* man, FEST_MISSION_ID id, FEST_MISSION* fmwk )
{
  const FMISSION_ARCDATA* dat;

  MI_CpuClear8(fmwk,sizeof(FEST_MISSION));

  dat = &DATA_MissionArcData[id];

  fmwk->type = dat->type; 
  fmwk->time = dat->time;
  fmwk->param = dat->param;
  fmwk->target = dat->target;
  fmwk->item = dat->item;
  fmwk->item_num = dat->item_num;
  fmwk->calc_type = dat->calc_type;
  fmwk->end_type = dat->end_type;
  fmwk->bonus = dat->bonus;

  fmwk->life = fmwk->time*60; //sec2frame
}
