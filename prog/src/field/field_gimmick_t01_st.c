//======================================================================
/**
 * @file  field_gimmick_t01_st.c
 * @brief  Ｔ01 ギミック 常駐
 * @author  Saito
 */
//======================================================================
#include <gflib.h>
#include "fieldmap.h" 
#include "field_gimmick_t01.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "system/gfl_use.h"

#include "field_gimmick_t01_sv.h"

//------------------------------------------------------------------------------------------
/**
 * @brief ギミックを発動状態にする
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void T01_GIMMICK_Start( GAMESYS_WORK *gsys )
{
  T01_SV_WORK *gmk_sv_work;
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_T01 );
  }

  gmk_sv_work->Move = TRUE;

}

