/**
 *  @file fiele_encount_st.c
 *  @brief  field_encount.c 系列常駐関数
 *  @author Miyuki iwasawa
 *  @date   09.10.19
 */

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "fieldmap.h"
#include "map_attr.h"

#include "encount_data.h"
#include "field_encount.h"

#include "battle/battle.h"
#include "gamesystem/btl_setup.h"
#include "poke_tool/poketype.h"
#include "poke_tool/pokeparty.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/poke_personal.h"
#include "tr_tool/tr_tool.h"
#include "item/itemsym.h"

#include "enc_pokeset.h"
#include "event_battle.h"
#include "field_encount_local.h"
#include "effect_encount.h"

#include "sound/wb_sound_data.sadl"

//--------------------------------------------------------------
/**
 * エンカウントワーク作成
 * @retval ENCOUNT_WORK*
 */
//--------------------------------------------------------------
ENCOUNT_WORK * ENCOUNT_WORK_Create( HEAPID heapID )
{
  ENCOUNT_WORK *wp;
  
  wp = GFL_HEAP_AllocClearMemory( heapID, sizeof(ENCOUNT_WORK) );

#ifdef PM_DEBUG
   #ifdef DEBUG_ONLY_FOR_iwasawa
    wp->effect_encount.deb_interval = 10;
    wp->effect_encount.deb_prob = 100;
  #else
    wp->effect_encount.deb_interval = EFFENC_DEFAULT_INTERVAL;
    wp->effect_encount.deb_prob = EFFENC_DEFAULT_PROB;
  #endif
#endif
  return( wp );
}

//--------------------------------------------------------------
/**
 * エンカウントワーク破棄
 * @retval ENCOUNT_WORK*
 */
//--------------------------------------------------------------
void ENCOUNT_WORK_Delete( ENCOUNT_WORK* wp )
{
  GFL_HEAP_FreeMemory( wp );
  wp = NULL;
}

/**
 *  @brief  地続きゾーンまたぎ時の状態セット
 */
void ENCOUNT_WORK_MapWalkStepOverUpdate( FIELDMAP_WORK* fieldWork )
{
  FIELD_ENCOUNT* encount = FIELDMAP_GetEncount(fieldWork); 

  EFFECT_ENC_SetProb( encount, encount->eff_enc );
}

/**
 *  @brief  特殊マップジャンプ時の状態セット
 *
 *  空を飛ぶ、穴抜け、テレポート、全滅時のマップチェンジのみから呼び出す
 *　※マップジャンプ時全てに適用する状態遷移はない
 */
void ENCOUNT_WORK_SpMapJumpUpdate( GAMEDATA* gdata )
{
  ENCOUNT_WORK* ewk = GAMEDATA_GetEncountWork(gdata);

  //エフェクトエンカウト　歩数クリア
  ewk->effect_encount.walk_ct = 0;
}

