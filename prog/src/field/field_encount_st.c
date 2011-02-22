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

#include "sound/swan_sound_data.sadl"
#include "debug/debug_flg.h"

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
    wp->effect_encount.deb_interval = 1;
    wp->effect_encount.deb_prob = 100;
    wp->effect_encount.deb_ofsx = 0;
    wp->effect_encount.deb_ofsz = 0;
    wp->effect_encount.deb_prob_fix = 1;
//    DEBUG_FLG_FlgOn( DEBUG_FLG_DisableEncount );

  #else
    wp->effect_encount.deb_interval = EFFENC_NORMAL_INTERVAL;
    wp->effect_encount.deb_prob = EFFENC_NORMAL_PROB;
  #endif
#endif

  ENCOUNT_WORK_EncountProbReset( wp, MATTR_NO_ATTR );
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
void ENCOUNT_WORK_MapWalkStepOverUpdate( GAMEDATA* gdata, FIELDMAP_WORK* fieldWork )
{
  FIELD_ENCOUNT* encount = FIELDMAP_GetEncount(fieldWork); 
  FIELD_PLAYER *fplayer = FIELDMAP_GetFieldPlayer( fieldWork );

  EFFECT_ENC_SetProb( encount, encount->eff_enc );
  ENCOUNT_WORK_EncountProbReset( GAMEDATA_GetEncountWork(gdata), FIELD_PLAYER_GetMapAttr( fplayer ) );
}

/**
 *  @brief  特殊マップジャンプ時の状態セット
 *
 *  空を飛ぶ、穴抜け、テレポート、全滅時のマップチェンジのみから呼び出す
 */
void ENCOUNT_WORK_SpMapJumpUpdate( GAMEDATA* gdata )
{
  ENCOUNT_WORK* ewk = GAMEDATA_GetEncountWork(gdata);

  //エフェクトエンカウト　歩数クリア
  ewk->effect_encount.walk_ct = 0;
}

/*
 *  @brief  マップチェンジ時の状態セット(フィールドが再構築されてから呼び出す)
 */
void ENCOUNT_WORK_MapJumpUpdate( GAMEDATA* gdata, FIELDMAP_WORK* fieldWork )
{
  FIELD_PLAYER *fplayer = FIELDMAP_GetFieldPlayer( fieldWork );

  ENCOUNT_WORK_EncountProbReset( GAMEDATA_GetEncountWork(gdata), FIELD_PLAYER_GetMapAttr( fplayer ) );
}

/*
 *  @brief  エンカウント率補整値リセット
 */
void ENCOUNT_WORK_EncountProbReset( ENCOUNT_WORK* wp, MAPATTR attr )
{
  wp->player.walk_ct = 0;

  wp->player.o_attr = attr;
  wp->player.n_prob = 1;
  wp->player.step = 0;
  wp->player.move_f = 0;

  OS_FPrintf( 3, "#Enc Prob Reset\n");
}
