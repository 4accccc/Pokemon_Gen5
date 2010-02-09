//======================================================================
/**
 * @file	field_gimmick_bsubway.c
 * @brief	フィールドギミック　バトルサブウェイ
 * @authaor	kagaya
 * @date	2008.12.11
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "sound/pm_sndsys.h"
#include "savedata/gimmickwork.h"

#include "arc/fieldmap/zone_id.h"

#include "fieldmap.h"
#include "field_gimmick_def.h"

#include "field_gimmick_bsubway.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
/// typedef 
//--------------------------------------------------------------
typedef struct _TAG_BSUBWAYGMK_WORK BSUBWAYGMK_WORK;
typedef struct _TAG_BSUBWAY_WORK BSUBWAY_WORK;

//--------------------------------------------------------------
/// BSUBWAYGMK_WORK
//--------------------------------------------------------------
struct _TAG_BSUBWAYGMK_WORK
{
  u8 init_flag;
  BSUBWAY_WORK *bsubway_work;
};

//--------------------------------------------------------------
/// BSUBWAY_WORK
//--------------------------------------------------------------
struct _TAG_BSUBWAY_WORK
{
  u32 zone_id;
  BSUBWAYGMK_WORK *bsubwaygmk_work;
  FIELDMAP_WORK *fieldmap;
};

//======================================================================
//  proto
//======================================================================

//======================================================================
//  バトルサブウェイ
//======================================================================
//--------------------------------------------------------------
/**
 * ギミック　バトルサブウェイ　初期化
 * @param fieldmap  FIELDMAP_WORK
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_GIMMICK_Setup( FIELDMAP_WORK *fieldmap )
{
  HEAPID heap_id = FIELDMAP_GetHeapID( fieldmap );
  GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK *gmk_work = GAMEDATA_GetGimmickWork( gdata );
  FLDEFF_CTRL *fectrl = FIELDMAP_GetFldEffCtrl( fieldmap );
  u32 zone_id = FIELDMAP_GetZoneID( fieldmap );
#if 0
  int gmk_id = bsubway_ZoneID_to_GimmickID( zone_id );
  BSUBWAYGMK_WORK *bsubwaygmk = GIMMICKWORK_Get( gmk_work, gmk_id );
  BSUBWAY_WORK *bsubway = GFL_HEAP_AllocClearMemory( heap_id, sizeof(BSUBWAY_WORK) );
#endif
}

//--------------------------------------------------------------
/**
 * ギミック　バトルサブウェイ　終了
 * @param fieldmap  FIELDMAP_WORK
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_GIMMICK_End( FIELDMAP_WORK *fieldmap )
{
  HEAPID heap_id = FIELDMAP_GetHeapID( fieldmap );
  GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK *gmk_work = GAMEDATA_GetGimmickWork( gdata );
  FLDEFF_CTRL *fectrl = FIELDMAP_GetFldEffCtrl( fieldmap );
  u32 zone_id = FIELDMAP_GetZoneID( fieldmap );
#if 0
  int gmk_id = bsubway_ZoneID_to_GimmickID( zone_id );
  BSUBWAYGMK_WORK *bsubwaygmk = GIMMICKWORK_Get( gmk_work, gmk_id );
  
  GFL_HEAP_FreeMemory( bsubwaygmk->bsubway_work );
#endif
}

//--------------------------------------------------------------
/**
 * ギミック　バトルサブウェイ　動作
 * @param fieldmap  FIELDMAP_WORK
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_GIMMICK_Move( FIELDMAP_WORK *fieldmap )
{
}
