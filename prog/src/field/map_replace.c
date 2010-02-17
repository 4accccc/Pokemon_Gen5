//======================================================================
/**
 * @file	map_replace.c
 * @brief	条件によるマップ書き換え
 * @author	tamada GAMEFREAK inc.
 * @date	09.12.11
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "arc_def.h"

#include "gamesystem/gamesystem.h"
#include "field/map_matrix.h"
#include "field/field_comm/intrude_work.h"

#include "arc/fieldmap/map_replace.naix"
#include "arc/fieldmap/map_replace.h"
#include "map_replace_def.h"

#include "map_replace.h"

#include "eventwork.h"  //EVENTWORK_

#include "../../../resource/fldmapdata/flagwork/work_define.h"  //WK_SYS_MAPREPLACE～

//======================================================================
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
enum {
  FILE_ID = NARC_map_replace_map_replace_bin
};

enum {
  MAPREPLACE_EVENT_TYPE_NUM = 10,
};

//--------------------------------------------------------------
//--------------------------------------------------------------
enum {
  REPID_BASE_KEY = 0,
  REPID_VAR_A,
  REPID_VAR_B,
  REPID_VAR_C,
  REPID_VAR_D,

  REPID_MAX,
};

//--------------------------------------------------------------
/**
 * @brief   マップ書き換えデータ
 * @note  コンバータにより生成されるバイナリデータと対応している。
 */
//--------------------------------------------------------------
typedef struct {
  u16 matrix_id;    ///<対象となるマトリックス指定
  u8 layer_id;      ///<書き換えるレイヤー（地形ブロックか、マトリックスか？）
  u8 type;          ///<書き換え条件のタイプ
  u16 keys[ REPID_MAX ];    ///<書き換えデータ
  u16 dummy;
}REPLACE_DATA;

SDK_COMPILER_ASSERT( sizeof(REPLACE_DATA) == MAPREPLACE_DATASIZE );

//--------------------------------------------------------------
/**
 * @brief 書き換えオフセットワーク
 */
//--------------------------------------------------------------
typedef struct {
  u8 season_pos;      ///<書き換えタイプ季節の場合のオフセット値
  u8 version_pos;     ///<書き換えタイプVer.の場合のオフセット値
  u8 season_ver_pos;  ///<書き換えタイプ季節＋Ver.の場合のオフセット値
  ///フラグによる書き換えの場合のオフセット値
  u8 flag_pos[MAPREPLACE_EVENT_TYPE_NUM];
}REPLACE_OFFSET;

//--------------------------------------------------------------
/**
 * @brief マップ置き換え制御用ワーク
 */
//--------------------------------------------------------------
struct _MAPREPLACE_CTRL {
  HEAPID heapID;
  ARCHANDLE * pHandle;      ///<マップ置き換え指定データのハンドル
  u32 data_max;             ///<マップ置き換え指定データ数
  REPLACE_OFFSET offsets;   ///<置き換えオフセット指定を保持するワーク
  REPLACE_DATA rep_data;    ///<マップ置き換えデータの読み込み領域
};


//======================================================================
//======================================================================

static void MAPREPLACE_makeReplaceOffset ( REPLACE_OFFSET * offsets, GAMESYS_WORK * gamesystem );
static int getReplaceTablePos( MAPREPLACE_TYPE rep_type );

//======================================================================
//
//
//    外部公開関数
//
//
//======================================================================
//--------------------------------------------------------------
/**
 * @brief マップ置き換え制御ワークの解放処理
 * @param ctrl  制御ワークへのポインタ
 */
//--------------------------------------------------------------
MAPREPLACE_CTRL * MAPREPLACE_Create( HEAPID heapID, GAMESYS_WORK * gamesys )
{
  u16 size;
  MAPREPLACE_CTRL * ctrl = GFL_HEAP_AllocClearMemory( heapID, sizeof(MAPREPLACE_CTRL) );
  ctrl->heapID = heapID;
  ctrl->pHandle = GFL_ARC_OpenDataHandle( ARCID_FLDMAP_REPLACE, heapID );
  size = GFL_ARC_GetDataSizeByHandle( ctrl->pHandle, FILE_ID );
  ctrl->data_max = size / MAPREPLACE_DATASIZE;
  //REPLACE_OFFSET offsets;
  //REPLACE_DATA rep_data;


  GF_ASSERT( ctrl->data_max = MAP_REPLACE_DATA_COUNT );
  GF_ASSERT( size % MAPREPLACE_DATASIZE == 0 );

  //置き換えオフセット値を事前に算出しておく
  MAPREPLACE_makeReplaceOffset( &ctrl->offsets, gamesys );
  return ctrl;
}
//--------------------------------------------------------------
/**
 * @brief
 * @param ctrl  制御ワークへのポインタ
 * @return u32  保持している個別マップ置き換えデータの個数
 */
//--------------------------------------------------------------
u32 MAPREPLACE_GetDataMax( const MAPREPLACE_CTRL * ctrl )
{
  return ctrl->data_max;
}
//--------------------------------------------------------------
/**
 * @brief マップ置き換え個別データの読み込み
 * @param ctrl  制御ワークへのポインタ
 * @param idx   読み込むデータのインデックス
 * @return  u32 読み込んだデータが対応するマトリックスID
 */
//--------------------------------------------------------------
u32 MAPREPLACE_Load( MAPREPLACE_CTRL * ctrl, u32 idx )
{
  GF_ASSERT( idx < ctrl->data_max );
  GFL_ARC_LoadDataOfsByHandle( ctrl->pHandle, FILE_ID,
      MAPREPLACE_DATASIZE * idx, MAPREPLACE_DATASIZE, &ctrl->rep_data );
  GF_ASSERT( ctrl->rep_data.layer_id < MAPREPLACE_LAYER_MAX );
  GF_ASSERT( ctrl->rep_data.type <  MAPREPLACE_TYPE_MAX );

  return ctrl->rep_data.matrix_id;
}
//--------------------------------------------------------------
/**
 * @brief マップ置き換え制御ワークの解放処理
 * @param ctrl  制御ワークへのポインタ
 */
//--------------------------------------------------------------
void MAPREPLACE_Delete( MAPREPLACE_CTRL * ctrl )
{
  GFL_ARC_CloseDataHandle( ctrl->pHandle );
  GFL_HEAP_FreeMemory( ctrl );
}


//--------------------------------------------------------------
/**
 * @brief マップ置き換え処理
 * @param	pMat MAP_MATRIX
 * @param offsets   書き換えオフセット集合
 * @param rep_data  書き換えデータ
 */
//--------------------------------------------------------------
REPLACE_REQUEST MAPREPLACE_GetReplaceValue( const MAPREPLACE_CTRL * ctrl, u32 *before, u32 *after )
{
  const REPLACE_DATA * rep_data = &ctrl->rep_data;
  const REPLACE_OFFSET * offsets = &ctrl->offsets;
  u32 pos;
  u32 before_val, after_val;
  before_val = rep_data->keys[ REPID_BASE_KEY ];
  after_val = before_val;
  switch ( rep_data->type )
  {
  case MAPREPLACE_TYPE_SEASON:
    after_val = rep_data->keys[ offsets->season_pos ];
    break;

  case MAPREPLACE_TYPE_VERSION:
    after_val = rep_data->keys[ offsets->version_pos ];
    break;

  case MAPREPLACE_TYPE_SEASON_VERSION:
    after_val = rep_data->keys[ offsets->season_ver_pos ];
    break;

  default:
    pos = getReplaceTablePos( rep_data->type );
    if ( pos >= 0 ) {
      after_val = rep_data->keys[ offsets->flag_pos[ pos ] ];
      break;
    }
    GF_ASSERT(0);
    break;
  }
  *before = before_val;
  *after = after_val;

  if ( before_val == after_val ) return REPLACE_REQ_NON;

  if (rep_data->layer_id == MAPREPLACE_LAYER_MATRIX) {
    return REPLACE_REQ_MATRIX;
  } else if (rep_data->layer_id == MAPREPLACE_LAYER_MAPBLOCK) {
    return REPLACE_REQ_BLOCK;
  }
  GF_ASSERT(0);
  return REPLACE_REQ_NON;
}

//======================================================================
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
typedef enum {
  REPLACE_FLAG_VALUE01 = 0x1220,  //kagaya
  REPLACE_FLAG_VALUE02 = 0x0224,  //nozomu
  REPLACE_FLAG_VALUE03 = 0x0206,  //tomoya
  REPLACE_FLAG_VALUE04 = 0x1209,  //iwasawa
  REPLACE_FLAG_VALUE05 = 0x0316,  //obata
  REPLACE_FLAG_VALUE06 = 0x1227,  //masafumi
  REPLACE_FLAG_VALUE07 = 0x0830,  //nakatsui
  REPLACE_FLAG_VALUE08 = 0x0408,  //matsumiya
  REPLACE_FLAG_VALUE09 = 0x0120,  //fuchino
  REPLACE_FLAG_VALUE10 = 0x1028,  //watanabe
}REPLACE_FLAGS;

//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct {
  u8 rep_data_type;
  u8 rep_data_id;
  u16 wk_id;
  u16 wk_value;
}REPLACE_EVENT_DATA;

//--------------------------------------------------------------
//--------------------------------------------------------------
static const REPLACE_EVENT_DATA replaceEventTable[] = {
  { MAPREPLACE_TYPE_EVENT01, MAPREPLACE_ID_01, WK_SYS_MAPREPLACE01, REPLACE_FLAG_VALUE01 },
  { MAPREPLACE_TYPE_EVENT02, MAPREPLACE_ID_02, WK_SYS_MAPREPLACE02, REPLACE_FLAG_VALUE02 },
  { MAPREPLACE_TYPE_EVENT03, MAPREPLACE_ID_03, WK_SYS_MAPREPLACE03, REPLACE_FLAG_VALUE03 },
  { MAPREPLACE_TYPE_EVENT04, MAPREPLACE_ID_04, WK_SYS_MAPREPLACE04, REPLACE_FLAG_VALUE04 },
  { MAPREPLACE_TYPE_EVENT05, MAPREPLACE_ID_05, WK_SYS_MAPREPLACE05, REPLACE_FLAG_VALUE05 },
  { MAPREPLACE_TYPE_EVENT06, MAPREPLACE_ID_06, WK_SYS_MAPREPLACE06, REPLACE_FLAG_VALUE06 },
  { MAPREPLACE_TYPE_EVENT07, MAPREPLACE_ID_07, WK_SYS_MAPREPLACE07, REPLACE_FLAG_VALUE07 },
  { MAPREPLACE_TYPE_EVENT08, MAPREPLACE_ID_08, WK_SYS_MAPREPLACE08, REPLACE_FLAG_VALUE08 },
  { MAPREPLACE_TYPE_EVENT09, MAPREPLACE_ID_09, WK_SYS_MAPREPLACE09, REPLACE_FLAG_VALUE09 },
  { MAPREPLACE_TYPE_EVENT10, MAPREPLACE_ID_10, WK_SYS_MAPREPLACE10, REPLACE_FLAG_VALUE10 },
};

SDK_COMPILER_ASSERT( NELEMS(replaceEventTable) == MAPREPLACE_EVENT_TYPE_NUM );

//--------------------------------------------------------------
/**
 * @brief
 * @param rep_type
 */
//--------------------------------------------------------------
static int getReplaceTablePos( MAPREPLACE_TYPE rep_type )
{
  int i;
  for (i = 0; i < NELEMS(replaceEventTable); i++ )
  {
    if ( replaceEventTable[i].rep_data_type == rep_type )
    {
      return i;
    }
  }
  return -1;
}

//--------------------------------------------------------------
/**
 * @brief 書き換え条件オフセットの生成
 * @param offsets
 * @param gamesystem
 *
 * @todo  イベントフラグの対応を行う必要がある
 */
//--------------------------------------------------------------
static void MAPREPLACE_makeReplaceOffset ( REPLACE_OFFSET * offsets, GAMESYS_WORK * gamesystem )
{
  int i;
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gamesystem );
  EVENTWORK * ev = GAMEDATA_GetEventWork( gamedata );
  int season = GAMEDATA_GetSeasonID( gamedata );
  GAME_COMM_SYS_PTR commsys = GAMESYSTEM_GetGameCommSysPtr( gamesystem );
  
  offsets->season_pos = season;
  offsets->version_pos = Intrude_GetRomVersion( commsys ) == VERSION_BLACK? 0 : 1;
  if (offsets->version_pos == 0 )
  {
    offsets->season_ver_pos = 0;
  } else {
    offsets->season_ver_pos = 1 + season;
  }
  for ( i = 0; i < MAPREPLACE_EVENT_TYPE_NUM; i++)
  {
    const REPLACE_EVENT_DATA * evData = &replaceEventTable[ i ];
    u16 * work = EVENTWORK_GetEventWorkAdrs( ev, evData->wk_id );
    if ( *work == evData->wk_value )
    {
      offsets->flag_pos[i] = 1;
    } else {
      offsets->flag_pos[i] = 0; //とりあえず
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief マップ置き換え：フラグの反映処理
 * @param gamedata  ゲームデータへのポインタ
 * @param id        フラグ指定ID
 * @param flag      BOOL
 */
//--------------------------------------------------------------
void MAPREPLACE_ChangeFlag( GAMEDATA * gamedata, u32 id, BOOL flag )
{
  int i;
  for ( i = 0; i < NELEMS(replaceEventTable); i++ )
  {
    const REPLACE_EVENT_DATA * evData = &replaceEventTable[ i ];
    if ( evData->rep_data_id == id )
    {
      EVENTWORK * ev = GAMEDATA_GetEventWork( gamedata );
      u16 * work = EVENTWORK_GetEventWorkAdrs( ev, evData->wk_id );
      if (flag) {
        *work = evData->wk_value;
      } else {
        *work = 0;
      }
      OS_Printf("REPLACE EVENT(%d) SET: %04x\n", id, evData->wk_value );
      return;
    }
  }
  GF_ASSERT_MSG( 0, "無効なマップ置き換えID（%d)です", id );
}





