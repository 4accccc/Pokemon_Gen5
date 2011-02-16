//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_reception.c
 *	@brief  レゾナンスリゾート迎賓館メインソース
 *	@author	Toru=Nagihashi
 *	@data		2011.02.16
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//system
#include <gflib.h>
#include "system/gfl_use.h"
#include "../fieldmap.h"
#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE
#include "../field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "../gmk_tmp_wk.h"

//script
#include "../script.h"     //for SCRIPT_CallScript

//archive

//SE
#include "sound/pm_sndsys.h"

//mine
#include "field/resonance_resort_data.h"
#include "resort_reception.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define RESORT_RECEPTION_ASSIGN_ID  (0)

//=============================================================================
/**
 *          構造体
 */
//=============================================================================
typedef struct
{
  int dummy;
} RESORT_RECEPTION_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  レゾナンスリゾートギミック初期化
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールド
 */
//-----------------------------------------------------------------------------
void RESORT_RECEPTION_Setup( FIELDMAP_WORK *p_fieldmap )
{
  const HEAPID heapID = FIELDMAP_GetHeapID(p_fieldmap);

  //汎用ワーク確保
  RESORT_RECEPTION_WORK *p_wk = GMK_TMP_WK_AllocWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID, heapID, sizeof(RESORT_RECEPTION_WORK));
}
//----------------------------------------------------------------------------
/**
 *	@brief  レゾナンスリゾートギミック破棄
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールド
 */
//-----------------------------------------------------------------------------
void RESORT_RECEPTION_End( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_RECEPTION_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID);

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID);
}
//----------------------------------------------------------------------------
/**
 *	@brief  レゾナンスリゾートギミックメイン処理
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールド
 */
//-----------------------------------------------------------------------------
void RESORT_RECEPTION_Move( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_RECEPTION_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID);
}

