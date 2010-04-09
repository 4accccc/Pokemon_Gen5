//======================================================================
/**
 * @file  field_gimmick_d12_st.c
 * @brief  ビクティの島 ギミック 常駐
 * @author  Saito
 */
//======================================================================
#include <gflib.h>
#include "fieldmap.h" 
#include "field_gimmick_d12.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "system/gfl_use.h"

#include "field_gimmick_d12_sv.h"

//------------------------------------------------------------------------------------------
/**
 * @brief ギミックを非表示にする
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void D12_GIMMICK_Vanish( GAMESYS_WORK *gsys  )
{
  D12_SV_WORK *gmk_sv_work;
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_D12 );
  }

  gmk_sv_work->Vanish = TRUE;

}

