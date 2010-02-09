//======================================================================
/**
 * @file	fieldmap_ctrl_grid.c
 * @brief	フィールドマップ　コントロール　グリッド処理
 * @author	kagaya
 * @date	09.04.22
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "field/zonedata.h"
#include "arc/fieldmap/zone_id.h"

#include "fldmmdl.h"
#include "fieldmap.h"

#include "fieldmap_ctrl_grid.h"

#include "arc/arc_def.h"
#include "arc/arc_def.h"
#include "arc/fieldmap/camera_scroll.naix"

#ifdef PM_DEBUG
//@todo 12月ＲＯＭ用対処
#include "../../../resource/fldmapdata/flagwork/flag_define.h"  //for SYS_FLAG_
#endif

//======================================================================
//	範囲情報
//======================================================================
 



//======================================================================
//	define
//======================================================================
//--------------------------------------------------------------
///	グリッド処理　シーケンス
//--------------------------------------------------------------
typedef enum
{
	GRIDPROC_SEQNO_INIT = 0,
	GRIDPROC_SEQNO_MAIN,
	GRIDPROC_SEQNO_MAX,
}GRIDPROC_SEQNO;

//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
//	FIELDMAP_CTRL_GRID*
//--------------------------------------------------------------
struct _TAG_FIELDMAP_CTRL_GRID
{
	HEAPID heapID;
	FIELDMAP_WORK *fieldWork;
	
	u16 seq_proc;	
	u8 padding0;
	u8 padding1;

	BOOL jikiMovePause;
};

//======================================================================
//	proto
//======================================================================
static void mapCtrlGrid_Create(
	FIELDMAP_WORK *fieldWork, VecFx32 *pos, u16 dir );
static void mapCtrlGrid_Delete( FIELDMAP_WORK *fieldWork );
static void mapCtrlGrid_Main( FIELDMAP_WORK *fieldWork, VecFx32 *pos );
static const VecFx32 * mapCtrlGrid_GetCameraTarget( FIELDMAP_WORK *fieldWork );

static u16 grid_ChangeFourDir( u16 dir );


static BOOL gym_check( u16 zone_id );

//======================================================================
//	フィールドマップ　グリッド処理
//======================================================================
//--------------------------------------------------------------
///	マップコントロール　グリッド移動
//--------------------------------------------------------------
const DEPEND_FUNCTIONS FieldMapCtrl_GridFunctions =
{
	FLDMAP_CTRLTYPE_GRID,
	mapCtrlGrid_Create,
	mapCtrlGrid_Main,
	mapCtrlGrid_Delete,
  mapCtrlGrid_GetCameraTarget,
};

//--------------------------------------------------------------
/**
 * フィールドマップ　グリッド処理　初期化 
 * @param	fieldWork	FIELDMAP_WORK
 * @param	pos	自機初期位置
 * @param	dir 自機初期方向
 * @retval	nothing
 */
//--------------------------------------------------------------
static void mapCtrlGrid_Create(
	FIELDMAP_WORK *fieldWork, VecFx32 *pos, u16 dir )
{
	HEAPID heapID;
	FIELDMAP_CTRL_GRID *gridWork;
	
	heapID = FIELDMAP_GetHeapID( fieldWork );
	gridWork = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELDMAP_CTRL_GRID) );
	
	gridWork->heapID = heapID;
	gridWork->fieldWork = fieldWork;
	
	{	//ビルボード設定
#if 0 //標準を使用する
		VecFx32 scale = {
			FX32_ONE+(FX32_ONE/2)+(FX32_ONE/4),
			FX32_ONE+(FX32_ONE/2)+(FX32_ONE/4),
			FX32_ONE+(FX32_ONE/2)+(FX32_ONE/4),
		};
		GFL_BBDACT_SYS *bbdActSys = FIELDMAP_GetBbdActSys( fieldWork );
		GFL_BBD_SetScale( GFL_BBDACT_GetBBDSystem(bbdActSys), &scale );
#endif
	}

#if 0
	{	//マップ描画オフセット
		VecFx32 offs = { -FX32_ONE*8, 0, FX32_ONE*8 };
		FLDMAPPER *mapper = FIELDMAP_GetFieldG3Dmapper( fieldWork );
		FLDMAPPER_SetDrawOffset( mapper, &offs );
	}
#endif

	{ //自機作成
		FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
    MMDL *fmmdl = FIELD_PLAYER_GetMMdl( fld_player );
#if 0
    u16 dir4 = grid_ChangeFourDir( dir );
#else
   u16 dir4 = dir;
#endif
    MMDL_SetDirDisp( fmmdl, dir4 );
//    KAGAYA_Printf( "わたされた方向 %xH, %d\n", dir, dir4 );
		FIELD_PLAYER_SetUpGrid( fld_player, heapID );
    
    { //カメラ座標セット
      const VecFx32 *pos = MMDL_GetVectorPosAddress( fmmdl );
      FIELDMAP_SetNowPosTarget( fieldWork, pos );
    }
	}
  
	FIELDMAP_SetMapCtrlWork( fieldWork, gridWork );
}

//--------------------------------------------------------------
/**
 * フィールドマップ　グリッド処理　終了
 * @param	fieldWork	FIELDMAP_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
static void mapCtrlGrid_Delete( FIELDMAP_WORK *fieldWork )
{
	FIELDMAP_CTRL_GRID *gridWork;
	gridWork = FIELDMAP_GetMapCtrlWork( fieldWork );
	GFL_HEAP_FreeMemory( gridWork );
}

//--------------------------------------------------------------
/**
 * フィールドマップ　グリッド処理　メイン
 * @param	fieldWork FIELDMAP_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
static void mapCtrlGrid_Main( FIELDMAP_WORK *fieldWork, VecFx32 *pos )
{
	FIELDMAP_CTRL_GRID *gridWork = FIELDMAP_GetMapCtrlWork( fieldWork );
	FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
	
  if( gridWork->jikiMovePause == FALSE )
	{	//自機移動
		int key_trg = GFL_UI_KEY_GetTrg( );
		int key_cont = GFL_UI_KEY_GetCont( );


#ifdef PM_DEBUG
    //@todo 12月ＲＯＭ用対処
    {
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldWork );
      GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
      EVENTWORK *evwork = GAMEDATA_GetEventWork( gdata );
      if ( !EVENTWORK_CheckEventFlag( evwork, SYS_FLAG_RUNNINGSHOES) ){
        //Bボタンをマスク
        if (key_trg & PAD_BUTTON_B) key_trg ^= PAD_BUTTON_B;
        if (key_cont & PAD_BUTTON_B) key_cont ^= PAD_BUTTON_B;
      }
    }
#endif  //PM_DEBUG

		FIELD_PLAYER_MoveGrid( fld_player, key_trg, key_cont );
	}
}

//--------------------------------------------------------------
/**
 * フィールドマップ　グリッド処理　カメラターゲット取得
 * @param fieldWork FIELDMAP_WORK
 * @retval VecFx32* カメラターゲット
 */
//--------------------------------------------------------------
static const VecFx32 * mapCtrlGrid_GetCameraTarget( FIELDMAP_WORK *fieldWork )
{
  FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
  MMDL *mmdl = FIELD_PLAYER_GetMMdl( fld_player );
  const VecFx32 *pos = MMDL_GetVectorPosAddress( mmdl ); 
  return( pos );
}

//======================================================================
//	グリッド処理　パーツ
//======================================================================
//--------------------------------------------------------------
/**
 * 360度方向->４方向に
 * @param	dir	方向　0x10000単位
 * @retval	u16 DIR_UP等
 */
//--------------------------------------------------------------
static u16 grid_ChangeFourDir( u16 dir )
{
	if( (dir>0x2000) && (dir<0x6000) ){
		dir = DIR_LEFT;
	}else if( (dir >= 0x6000) && (dir <= 0xa000) ){
		dir = DIR_DOWN;
	}else if( (dir > 0xa000)&&(dir < 0xe000) ){
		dir = DIR_RIGHT;
	}else{
		dir = DIR_UP;
	}
	return( dir );
}

//--------------------------------------------------------------
/**
 * フィールドマップ　グリッド処理　自機動作停止
 * @param fieldMap FIELDMAP_WORK
 * @param flag TRUE=停止 FALSE=動作
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELDMAP_CTRL_GRID_SetPlayerPause( FIELDMAP_WORK *fieldMap, BOOL flag )
{
  FIELDMAP_CTRL_GRID *gridWork = FIELDMAP_GetMapCtrlWork( fieldMap );
#ifdef PM_DEBUG
  {
    FLDMAP_CTRLTYPE type = FIELDMAP_GetMapControlType( fieldMap );
    GF_ASSERT( type == FLDMAP_CTRLTYPE_GRID );
  }
#endif
  gridWork->jikiMovePause = flag;
}




