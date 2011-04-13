//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		gym_poison.c
 *	@brief  毒ジム
 *	@author		Katsuhiko=Ichiraku
 *	@date		2011.04.12
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

// system
#include "fieldmap.h"
#include "savedata/gimmickwork.h"
#include "system/ica_anime.h"
#include "system/main.h"    //for HEAPID_FIELDMAP

// field
#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE
#include "field_player_core.h"  // for FIELD_PLAYER_CORE_GetMMdl

// gimmick
#include "field_gimmick_def.h"
#include "gmk_tmp_wk.h"

// arc
#include "arc/fieldmap/gym_poison.naix"

// sound
#include "sound/pm_sndsys.h"

// mine
#include "gym_poison.h"
#include "gym_poison_se_def.h"


//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define GYM_POISON_UNIT_IDX (0)
#define GYM_POISON_TMP_ASSIGN_ID  (1)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ジム内部中の一時ワーク
//=====================================
typedef struct {
  u32 dummy;
} GYM_POISON_TMP;

//=============================================================================
/**
 *					static関数宣言
*/
//=============================================================================


//=============================================================================
/**
 *          外部公開関数
 */
//=============================================================================
//--------------------------------------------------------------
/**
 * @brief     セットアップ関数
 * @param	    p_fieldmap     フィールドワークポインタ
 * @return    none
 */
//--------------------------------------------------------------
void GYM_POISON_Setup(FIELDMAP_WORK *p_fieldmap)
{
  GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );

  //汎用ワーク確保
  GYM_POISON_TMP *p_wk = GMK_TMP_WK_AllocWork(p_fieldmap, GYM_POISON_TMP_ASSIGN_ID,
      FIELDMAP_GetHeapID(p_fieldmap), sizeof(GYM_POISON_TMP));

}

//--------------------------------------------------------------
/**
 * @brief   解放関数
 * @param   p_fieldmap     フィールドワークポインタ
 * @return  none
 */
//--------------------------------------------------------------
void GYM_POISON_End(FIELDMAP_WORK *p_fieldmap)
{
  GYM_POISON_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GYM_POISON_TMP_ASSIGN_ID );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(p_fieldmap, GYM_POISON_TMP_ASSIGN_ID);
}

//--------------------------------------------------------------
/**
 * @brief       動作関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_POISON_Move(FIELDMAP_WORK *p_fieldmap)
{
  GYM_POISON_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GYM_POISON_TMP_ASSIGN_ID );
  u8 i;

}

