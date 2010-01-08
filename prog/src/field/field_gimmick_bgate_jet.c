/*
 *  @file   field_gimmick_bgate_jet.c
 *  @brief  ジェットバッジゲートギミック
 *  @author Miyuki Iwasawa
 *  @date   10.01.08
 */

#include "fieldmap.h"
#include "field_gimmick_league_front01.h"
#include "gamesystem/game_event.h"
#include "gamesystem/gamesystem.h"
#include "savedata/gimmickwork.h"  // for GIMMICKWORK
#include "system/ica_anime.h"  // for ICA_ANIME
#include "field_gimmick_def.h"  // for FLD_GIMMICK_C09P01
#include "field/field_const.h"  // for FIELD_CONST_GRID_SIZE
#include "fld_exp_obj.h"  // for FLD_EXP_OBJ
#include "../../../resource/fldmapdata/zonetable/zone_id.h"  // for ZONE_ID_C09P02
#include "event_mapchange.h"  // for EVENT_ChangeMapPosNoFade
#include "event_fieldmap_control.h"  // for EVENT_FieldBrightOut
#include "field_gimmick_bgate_jet.h"  //
#include "eventwork.h"  // for EVENTWORK_
#include "../../../resource/fldmapdata/flagwork/flag_define.h"  // for SYS_FLAG_BIGFOUR_xxxx


typedef struct JET_BGATE_SV_WORK{
  fx32  py;
  u32   rot;
}JET_BGATE_SV_WORK;

//==========================================================================================
// ■ギミック登録関数
//==========================================================================================

//------------------------------------------------------------------------------------------
/**
 * @brief 初期化関数
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void FLD_GIMMICK_JetBadgeGate_Setup( FIELDMAP_WORK* fieldmap )
{
  JET_BGATE_SV_WORK *sv_work;
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldmap ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_BGATE_JET );
  }


}

//------------------------------------------------------------------------------------------
/**
 * @brief 終了関数
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void FLD_GIMMICK_JetBadgeGate_End( FIELDMAP_WORK* fieldmap )
{

}

//------------------------------------------------------------------------------------------
/**
 * @brief メイン関数
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void FLD_GIMMICK_JetBadgeGate_Move( FIELDMAP_WORK* fieldmap )
{

}


//--------------------------------------------------------------
/**
 * @brief   ジェットバッジゲートギミック初期化
 *
 * @param   gsys            ゲームシステムポインタ
 * @return  none
 */
//--------------------------------------------------------------
void FLD_GIMMICK_JetBadgeGate_Init( GAMESYS_WORK* gsys )
{
  JET_BGATE_SV_WORK *sv_work;
  {
    GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gdata);
    sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_BGATE_JET );
  }

  MI_CpuClear8( sv_work, sizeof( JET_BGATE_SV_WORK ));
}

