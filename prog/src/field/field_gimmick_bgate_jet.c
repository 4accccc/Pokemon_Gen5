/*
 *  @file   field_gimmick_bgate_jet.c
 *  @brief  ジェットバッジゲートギミック
 *  @author Miyuki Iwasawa
 *  @date   10.01.08
 */
#include <gflib.h>
#include <calctool.h>
#include "system/main.h"

#include "fieldmap.h"
#include "gamesystem/game_event.h"
#include "gamesystem/gamesystem.h"
#include "savedata/gimmickwork.h"  // for GIMMICKWORK
#include "system/ica_anime.h"  // for ICA_ANIME
#include "field_gimmick.h"
#include "field_gimmick_def.h"  // for FLD_GIMMICK_C09P01
#include "gmk_tmp_wk.h"  // for GIMMICKWORK
#include "field/field_const.h"  // for FIELD_CONST_GRID_SIZE
#include "fld_exp_obj.h"  // for FLD_EXP_OBJ
#include "../../../resource/fldmapdata/zonetable/zone_id.h"  // for ZONE_ID_C09P02
#include "event_mapchange.h"  // for EVENT_ChangeMapPosNoFade
#include "event_fieldmap_control.h"  // for EVENT_FieldBrightOut
#include "field_gimmick_bgate_jet.h"  //
#include "eventwork.h"  // for EVENTWORK_
#include "../../../resource/fldmapdata/flagwork/flag_define.h"  // for SYS_FLAG_BIGFOUR_xxxx

#define JET_BGATE_TMP_ASSIGN_ID (1)

typedef struct JET_BGATE_SV_WORK{
  s16   rot;
  s16   sy;
}JET_BGATE_SV_WORK;

typedef struct _JET_BGATE_WORK{
  FIELD_PLAYER *fplayer;
  MMDL* mmdl;

  VecFx32 vec;
}JET_BGATE_WORK;

//==========================================================================================
// ■プロトタイプ
//==========================================================================================
static BOOL sub_MoveCheck( JET_BGATE_WORK* wk );

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
  JET_BGATE_WORK *wk;
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldmap ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_BGATE_JET );
  }

  //汎用ワーク確保
  GMK_TMP_WK_AllocWork
      (fieldmap, JET_BGATE_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldmap), sizeof(JET_BGATE_WORK));
  wk = GMK_TMP_WK_GetWork(fieldmap, JET_BGATE_TMP_ASSIGN_ID);

  wk->fplayer = FIELDMAP_GetFieldPlayer( fieldmap );
  wk->mmdl = FIELD_PLAYER_GetMMdl( wk->fplayer );

  VEC_Set(&wk->vec, 0,0,0 );
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
  JET_BGATE_SV_WORK *sv_work;
  JET_BGATE_WORK *wk;
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldmap ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_BGATE_JET );
  }
  wk = GMK_TMP_WK_GetWork(fieldmap, JET_BGATE_TMP_ASSIGN_ID);

  MMDL_SetVectorOuterDrawOffsetPos( wk->mmdl, &wk->vec );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldmap, JET_BGATE_TMP_ASSIGN_ID);
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
  JET_BGATE_SV_WORK *sv_work;
  JET_BGATE_WORK *wk;
  static const u8 tbl[] = { 8, 8, 12, 14 };
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldmap ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_BGATE_JET );
  }
  wk = GMK_TMP_WK_GetWork(fieldmap, JET_BGATE_TMP_ASSIGN_ID);

  if( sub_MoveCheck( wk ) == FALSE){
    wk->vec.y = 0;
    sv_work->rot = 0;
    MMDL_SetVectorOuterDrawOffsetPos( wk->mmdl, &wk->vec );
    return;
  }
  if( sv_work->rot == 0){
    sv_work->sy = tbl[GFUser_GetPublicRand0( 4 )];
  }
  wk->vec.y = FX_Mul( GFL_CALC_Sin360R( sv_work->rot ), FX32_CONST(sv_work->sy));
  MMDL_SetVectorOuterDrawOffsetPos( wk->mmdl, &wk->vec );
  sv_work->rot = (sv_work->rot+8)%360;
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


/*
 *  @brief  座標チェック
 */
static BOOL sub_MoveCheck( JET_BGATE_WORK* wk )
{
  s16 gz = MMDL_GetGridPosZ( wk->mmdl );

  if(gz > 10){
    return TRUE;
  }
  return FALSE;
}

