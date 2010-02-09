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

#include "fldeff_btrain.h"

#include "field_gimmick_bsubway.h"

//======================================================================
//  define
//======================================================================
enum
{
  BSWGMK_NO_C04R0102,
  BSWGMK_NO_C04R0103,
  BSWGMK_NO_C04R0104,
  BSWGMK_NO_C04R0105,
  BSWGMK_NO_C04R0106,
  BSWGMK_NO_C04R0107,
  BSWGMK_NO_C04R0108,
  BSWGMK_NO_C04R0110,
  BSWGMK_NO_C04R0111,
  BSWGMK_NO_MAX,
};

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
/// typedef 
//--------------------------------------------------------------
typedef struct _TAG_BSW_GMK BSW_GMK;

//ギミック関数
typedef void (*BSW_GMK_INIT_PROC)( BSW_GMK *bsw_gmk, FIELDMAP_WORK *fldmap );
typedef void (*BSW_GMK_MOVE_PROC)( BSW_GMK *bsw_gmk, FIELDMAP_WORK *fldmap );
typedef void (*BSW_GMK_DEL_PROC)( BSW_GMK *bsw_gmk, FIELDMAP_WORK *fldmap );

//--------------------------------------------------------------
/// BSW_GMK
//--------------------------------------------------------------
struct _TAG_BSW_GMK
{
  u16 init_flag;
  u16 gmk_no;
  u16 gmk_id;
  u32 zone_id;
  HEAPID heapID;
  void *bsw_work;
};

//--------------------------------------------------------------
//  BSW_GMK_PROC
//--------------------------------------------------------------
typedef struct
{
  BSW_GMK_INIT_PROC init_proc;
  BSW_GMK_DEL_PROC del_proc;
  BSW_GMK_MOVE_PROC move_proc;
}BSW_GMK_PROC;

//--------------------------------------------------------------
/// SUBWAY_WORK
//--------------------------------------------------------------
typedef struct
{
  int dmy;
}SUBWAY_WORK;

//--------------------------------------------------------------
/// SHAKE_WORK
//--------------------------------------------------------------
typedef struct
{
  int dmy;
}SHAKE_WORK;

//======================================================================
//  proto
//======================================================================
static void get_ZoneID_To_GmkID(
    const int zone_id, int *pRetID, int *pRetNo );

static const int data_ZoneID_To_GmkID[BSWGMK_NO_MAX][2];
static const BSW_GMK_PROC data_ProcTbl[BSWGMK_NO_MAX];
static const VecFx32 data_InitTrainPos[BSWGMK_NO_MAX];

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
  int no,id;
  BSW_GMK *bsw_gmk;
  HEAPID heap_id = FIELDMAP_GetHeapID( fieldmap );
  GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK *gmk_work = GAMEDATA_GetGimmickWork( gdata );
  u32 zone_id = FIELDMAP_GetZoneID( fieldmap );
  
  get_ZoneID_To_GmkID( zone_id, &id, &no );
  bsw_gmk = GIMMICKWORK_Get( gmk_work, id );
  bsw_gmk->gmk_no = no;
  bsw_gmk->gmk_id = id;
  bsw_gmk->zone_id = zone_id;
  bsw_gmk->heapID = heap_id;
  
  if( data_ProcTbl[bsw_gmk->gmk_no].init_proc != NULL ){
    data_ProcTbl[bsw_gmk->gmk_no].init_proc( bsw_gmk, fieldmap );
  }
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
  int no,id;
  BSW_GMK *bsw_gmk;
  HEAPID heap_id = FIELDMAP_GetHeapID( fieldmap );
  GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK *gmk_work = GAMEDATA_GetGimmickWork( gdata );
  u32 zone_id = FIELDMAP_GetZoneID( fieldmap );
  
  get_ZoneID_To_GmkID( zone_id, &id, &no );
  bsw_gmk = GIMMICKWORK_Get( gmk_work, id );
  
  if( data_ProcTbl[bsw_gmk->gmk_no].del_proc != NULL ){
    data_ProcTbl[bsw_gmk->gmk_no].del_proc( bsw_gmk, fieldmap );
  }
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
  int no,id;
  GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK *gmk_work = GAMEDATA_GetGimmickWork( gdata );
  u32 zone_id = FIELDMAP_GetZoneID( fieldmap );
  
  get_ZoneID_To_GmkID( zone_id, &id, &no );
  
  if( data_ProcTbl[no].move_proc != NULL ){
    BSW_GMK *bsw_gmk = GIMMICKWORK_Get( gmk_work, id );
    data_ProcTbl[no].move_proc( bsw_gmk, fieldmap );
  }
}

//======================================================================
//  バトルサブウェイ　駅
//======================================================================
//--------------------------------------------------------------
/**
 * 駅処理　初期化
 * @param bsw_gmk BSW_GMK*
 * @param fldmap FIELDMAP_WORK
 * @retval nothing
 */
//--------------------------------------------------------------
static void initProc_Subway( BSW_GMK *bsw_gmk, FIELDMAP_WORK *fldmap )
{
  SUBWAY_WORK *work;
  FLDEFF_CTRL *fectrl = FIELDMAP_GetFldEffCtrl( fldmap );
  
  work = GFL_HEAP_AllocClearMemory( bsw_gmk->heapID, sizeof(SUBWAY_WORK) );
  bsw_gmk->bsw_work = work;

  { //電車セットアップ
    FLDEFF_PROCID id = FLDEFF_PROCID_BTRAIN;
    FLDEFF_CTRL_RegistEffect( fectrl, &id, 1 );
  }
}

//--------------------------------------------------------------
/**
 * 駅処理　削除
 * @param bsw_gmk BSW_GMK*
 * @param fldmap FIELDMAP_WORK
 * @retval nothing
 */
//--------------------------------------------------------------
static void delProc_Subway( BSW_GMK *bsw_gmk, FIELDMAP_WORK *fldmap )
{
  GFL_HEAP_FreeMemory( bsw_gmk->bsw_work );
}

//--------------------------------------------------------------
/**
 * 駅処理　電車配置
 * @param fieldmap FILEDMAP_WORK
 * @param type FLDEFF_BTRAIN_TYPE
 * @param pos 配置座標
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_GIMMICK_SetTrain(
    FIELDMAP_WORK *fieldmap, FLDEFF_BTRAIN_TYPE type, const VecFx32 *pos )
{
}

//======================================================================
//  バトルサブウェイ　車内揺れ
//======================================================================
//--------------------------------------------------------------
/**
 * 車内揺れ処理　初期化
 * @param bsw_gmk BSW_GMK*
 * @param fldmap FIELDMAP_WORK
 * @retval nothing
 */
//--------------------------------------------------------------
static void initProc_ShakeTrain( BSW_GMK *bsw_gmk, FIELDMAP_WORK *fldmap )
{
  SHAKE_WORK *work;

  work = GFL_HEAP_AllocClearMemory( bsw_gmk->heapID, sizeof(SHAKE_WORK) );
  bsw_gmk->bsw_work = work;
}

//--------------------------------------------------------------
/**
 * 車内揺れ処理　削除
 * @param bsw_gmk BSW_GMK*
 * @param fldmap FIELDMAP_WORK
 * @retval nothing
 */
//--------------------------------------------------------------
static void delProc_ShakeTrain( BSW_GMK *bsw_gmk, FIELDMAP_WORK *fldmap )
{
  GFL_HEAP_FreeMemory( bsw_gmk->bsw_work );
}

//--------------------------------------------------------------
/**
 * 車内揺れ処理　動作
 * @param bsw_gmk BSW_GMK*
 * @param fldmap FIELDMAP_WORK
 * @retval nothing
 */
//--------------------------------------------------------------
static void moveProc_ShakeTrain( BSW_GMK *bsw_gmk, FIELDMAP_WORK *fldmap )
{
  SHAKE_WORK *work = bsw_gmk->bsw_work;
}

//======================================================================
//  parts
//======================================================================
//--------------------------------------------------------------
/**
 * zone idからギミックID,番号取得
 * @param zone_id ZONE ID
 * @param pRetID ギミックID格納先
 * @param pRetNo 番号格納先
 * @retval nothing
 */
//--------------------------------------------------------------
static void get_ZoneID_To_GmkID(
    const int zone_id, int *pRetID, int *pRetNo )
{
  int no = 0;
  
  while( no < BSWGMK_NO_MAX ){
    if( zone_id == data_ZoneID_To_GmkID[no][0] ){
      *pRetID = data_ZoneID_To_GmkID[no][1];
      *pRetNo = no;
      return;
    }
    no++;
  }
  
  GF_ASSERT( 0 );
  *pRetID = 0;
  *pRetNo = 0;
}

//======================================================================
//  data
//======================================================================
//--------------------------------------------------------------
/// ZONE IDに対応したギミックID
//--------------------------------------------------------------
static const int data_ZoneID_To_GmkID[BSWGMK_NO_MAX][2] =
{
  { ZONE_ID_C04R0102, FLD_GIMMICK_C04R0102 },
  { ZONE_ID_C04R0103, FLD_GIMMICK_C04R0103 },
  { ZONE_ID_C04R0104, FLD_GIMMICK_C04R0104 },
  { ZONE_ID_C04R0105, FLD_GIMMICK_C04R0105 },
  { ZONE_ID_C04R0106, FLD_GIMMICK_C04R0106 },
  { ZONE_ID_C04R0107, FLD_GIMMICK_C04R0107 },
  { ZONE_ID_C04R0108, FLD_GIMMICK_C04R0108 },
  { ZONE_ID_C04R0110, FLD_GIMMICK_C04R0110 },
  { ZONE_ID_C04R0111, FLD_GIMMICK_C04R0111 },
};

//--------------------------------------------------------------
//  ギミック番号別　処理テーブル
//--------------------------------------------------------------
static const BSW_GMK_PROC data_ProcTbl[BSWGMK_NO_MAX] =
{
  {initProc_Subway,delProc_Subway,NULL},//C04R0102
  {initProc_Subway,delProc_Subway,NULL},//C04R0103
  {initProc_Subway,delProc_Subway,NULL},//C04R0104
  {initProc_Subway,delProc_Subway,NULL},//C04R0105
  {initProc_Subway,delProc_Subway,NULL},//C04R0106
  {initProc_Subway,delProc_Subway,NULL},//C04R0107
  {initProc_Subway,delProc_Subway,NULL},//C04R0108
  {initProc_ShakeTrain,delProc_ShakeTrain,moveProc_ShakeTrain},//C04R0110
  {initProc_Subway,delProc_Subway,NULL},//FLD_GIMMICK_C04R0111
};

//--------------------------------------------------------------
/// ゾーンID別列車初期位置
//--------------------------------------------------------------
static const VecFx32 data_InitTrainPos[BSWGMK_NO_MAX] =
{
  {GRID_SIZE_FX32(0),GRID_SIZE_FX32(0),GRID_SIZE_FX32(0)}, //c04r0102
  {GRID_SIZE_FX32(0),GRID_SIZE_FX32(0),GRID_SIZE_FX32(0)}, //c04r0103
  {GRID_SIZE_FX32(0),GRID_SIZE_FX32(0),GRID_SIZE_FX32(0)}, //c04r0104
  {GRID_SIZE_FX32(0),GRID_SIZE_FX32(0),GRID_SIZE_FX32(0)}, //c04r0105
  {GRID_SIZE_FX32(0),GRID_SIZE_FX32(0),GRID_SIZE_FX32(0)}, //c04r0106
  {GRID_SIZE_FX32(0),GRID_SIZE_FX32(0),GRID_SIZE_FX32(0)}, //c04r0107
  {GRID_SIZE_FX32(0),GRID_SIZE_FX32(0),GRID_SIZE_FX32(0)}, //c04r0108
  {GRID_SIZE_FX32(0),GRID_SIZE_FX32(0),GRID_SIZE_FX32(0)}, //c04r0110
  {GRID_SIZE_FX32(0),GRID_SIZE_FX32(0),GRID_SIZE_FX32(0)}, //c04r0111
};

#if 0
//--------------------------------------------------------------
/// TRAIN_POS
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_BTRAIN_TYPE type;
  VecFx32 pos;
}TRAIN_POS;

//--------------------------------------------------------------
//  
//--------------------------------------------------------------
static const TRAIN_POS data_InitTrainPos[BSWGMK_NO_MAX] =
{
  { //シングルトレイン
    FLDEFF_BTRAIN_TYPE_01,
    {GRID_SIZE_FX32(0),GRID_SIZE_FX32(0),GRID_SIZE_FX32(0)},
  },
  { //スーパーシングルトレイン
    FLDEFF_BTRAIN_TYPE_02,
    {GRID_SIZE_FX32(0),GRID_SIZE_FX32(0),GRID_SIZE_FX32(0)},
  },
  { //ダブルトレイン
    FLDEFF_BTRAIN_TYPE_03,
    {GRID_SIZE_FX32(0),GRID_SIZE_FX32(0),GRID_SIZE_FX32(0)},
  },
  { //スーパーダブルトレイン
    FLDEFF_BTRAIN_TYPE_04,
    {GRID_SIZE_FX32(0),GRID_SIZE_FX32(0),GRID_SIZE_FX32(0)},
  },
  { //マルチトレイン
    FLDEFF_BTRAIN_TYPE_05,
    {GRID_SIZE_FX32(0),GRID_SIZE_FX32(0),GRID_SIZE_FX32(0)},
  },
  { //スーパーマルチトレイン
    FLDEFF_BTRAIN_TYPE_06,
    {GRID_SIZE_FX32(0),GRID_SIZE_FX32(0),GRID_SIZE_FX32(0)},
  },
  { //WiFiトレイン
    FLDEFF_BTRAIN_TYPE_07,
    {GRID_SIZE_FX32(0),GRID_SIZE_FX32(0),GRID_SIZE_FX32(0)},
  },
};
#endif
