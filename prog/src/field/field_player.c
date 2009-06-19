//======================================================================
/**
 * @file	field_player.c
 * @date	2008.9.29
 * @brief	フィールドプレイヤー制御
 */
//======================================================================
#include <gflib.h>

#include "fieldmap.h"
#include "field_g3d_mapper.h"
#include "fldmmdl.h"
#include "field_player.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
///	FIELD_PLAYER
//--------------------------------------------------------------
struct _FIELD_PLAYER
{
	HEAPID heapID;
	FIELDMAP_WORK *fieldWork;
  
  PLAYER_MOVE_FORM move_form;
  PLAYER_MOVE_STATE move_state;
  PLAYER_MOVE_VALUE move_value;
	
	u16 dir;
	u16 padding0;
	VecFx32 pos;
  
	MMDL *fldmmdl;
	FLDMAPPER_GRIDINFODATA gridInfoData;
  
	//以下消します
	GFL_BBDACT_RESUNIT_ID	bbdActResUnitID;
	GFL_BBDACT_ACTUNIT_ID	bbdActActUnitID;
};

//======================================================================
//	proto
//======================================================================
static const MMDL_HEADER playerdata_MMdlHeader;

//======================================================================
//	フィールドプレイヤー
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドプレイヤー　作成
 * @param FIELDMAP_WORK *fieldWork
 * @param pos 初期座標
 * @param heapID HEAPID
 * @retval FIELD_PLAYER*
 */
//--------------------------------------------------------------
FIELD_PLAYER * FIELD_PLAYER_Create(
		FIELDMAP_WORK *fieldWork, const VecFx32 *pos, HEAPID heapID )
{
	MMDLSYS *fmmdlsys;
	FIELD_PLAYER *fld_player;
	
	fld_player = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_PLAYER) );
	fld_player->fieldWork = fieldWork;
	fld_player->pos = *pos;

	FLDMAPPER_GRIDINFODATA_Init( &fld_player->gridInfoData );
	
	//MMDLセットアップ
	fmmdlsys = FIELDMAP_GetMMdlSys( fieldWork );
	
	fld_player->fldmmdl =
		MMDLSYS_SearchOBJID( fmmdlsys, MMDL_ID_PLAYER );

	if( fld_player->fldmmdl == NULL )	//新規
	{
		MMDL_HEADER head;
		head = playerdata_MMdlHeader;
		head.gx = SIZE_GRID_FX32( pos->x );
		head.gz = SIZE_GRID_FX32( pos->z );
		head.y = pos->y;
		fld_player->fldmmdl = MMDLSYS_AddMMdl( fmmdlsys, &head, 0 );
	}
	else //復帰
	{
		int gx = SIZE_GRID_FX32( pos->x );
		int gy = SIZE_GRID_FX32( pos->y );
		int gz = SIZE_GRID_FX32( pos->z );
		MMDL *fmmdl = fld_player->fldmmdl;
		
		MMDL_SetGridPosX( fmmdl, gx );
		MMDL_SetGridPosY( fmmdl, gy );
		MMDL_SetGridPosZ( fmmdl, gz );
		MMDL_SetVectorPos( fmmdl, pos );
	}
	
  { //OBJコードから動作フォームを設定
    PLAYER_MOVE_FORM form = PLAYER_MOVE_FORM_NORMAL;
    u16 code = MMDL_GetOBJCode( fld_player->fldmmdl );
    switch( code )
    {
    case HERO:
      form = PLAYER_MOVE_FORM_NORMAL;
      break;
    case CYCLEHERO:
      form = PLAYER_MOVE_FORM_CYCLE;
      break;
    case SWIMHERO:
      form = PLAYER_MOVE_FORM_SWIM;
      break;
    default:
      GF_ASSERT( 0 );
    }
    
    fld_player->move_form = form;
  }

	MMDL_SetStatusBitNotZoneDelete( fld_player->fldmmdl, TRUE );
	return( fld_player );
}

//--------------------------------------------------------------
/**
 * フィールドプレイヤー　削除
 * @param fld_player FIELD_PLAYER
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_Delete( FIELD_PLAYER *fld_player )
{
	//動作モデルの削除はフィールドメイン側で
	GFL_HEAP_FreeMemory( fld_player );
}

//--------------------------------------------------------------
/**
 * フィールドプレイヤー　更新
 * @param fld_player FIELD_PLAYER
 * @param dir 方向
 * @param pos 座標
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_Update( FIELD_PLAYER *fld_player )
{
}

//======================================================================
//  FIELD_PLAYER 動作ステータス
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドプレイヤー　動作ステータス更新
 * @param fld_player
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_UpdateMoveStatus( FIELD_PLAYER *fld_player )
{
  MMDL *fmmdl = fld_player->fldmmdl;
  PLAYER_MOVE_VALUE value = fld_player->move_value;
  PLAYER_MOVE_STATE state = fld_player->move_state;
  
  fld_player->move_state = PLAYER_MOVE_STATE_OFF;
  
  if( MMDL_CheckPossibleAcmd(fmmdl) == FALSE ){ //動作中
    switch( value ){
    case PLAYER_MOVE_VALUE_STOP:
      break;
    case PLAYER_MOVE_VALUE_WALK:
      if( state == PLAYER_MOVE_STATE_OFF ||
          state == PLAYER_MOVE_STATE_END ){
        fld_player->move_state = PLAYER_MOVE_STATE_START;
      }else{
        fld_player->move_state = PLAYER_MOVE_STATE_ON;
      }
      break;
    case PLAYER_MOVE_VALUE_TURN:
      fld_player->move_state = PLAYER_MOVE_STATE_ON;
      break;
    }
    
    return;
  }
  
  if( MMDL_CheckEndAcmd(fmmdl) == TRUE ){ //動作終了
    switch( value ){
    case PLAYER_MOVE_VALUE_STOP:
      break;
    case PLAYER_MOVE_VALUE_WALK:
      switch( state ){
      case PLAYER_MOVE_STATE_OFF:
        break;
      case PLAYER_MOVE_STATE_END:
        fld_player->move_state = PLAYER_MOVE_STATE_OFF;
        break;
      default:
        fld_player->move_state = PLAYER_MOVE_STATE_END;
      }
      break;
    case PLAYER_MOVE_VALUE_TURN:
      switch( state ){
      case PLAYER_MOVE_STATE_OFF:
        break;
      case PLAYER_MOVE_STATE_END:
        fld_player->move_state = PLAYER_MOVE_STATE_OFF;
        break;
      default:
        fld_player->move_state = PLAYER_MOVE_STATE_END;
      }
      break;
    }

    return;
  }
}

//======================================================================
//	FILED_PLAYER　参照、設定
//======================================================================
//--------------------------------------------------------------
/**
 * FILED_PLAYER　座標取得
 * @param fld_player FIELD_PLAYER
 * @param pos 座標格納先
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_GetPos( const FIELD_PLAYER *fld_player, VecFx32 *pos )
{
#if 0
	*pos = fld_player->pos;
#else //表示座標となるアクターから直に取得
  MMDL_GetVectorPos( fld_player->fldmmdl, pos );
#endif
}

//--------------------------------------------------------------
/**
 * FILED_PLAYER　座標セット
 * @param fld_player FIELD_PLAYER
 * @param pos セットする座標
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_SetPos( FIELD_PLAYER *fld_player, const VecFx32 *pos )
{
	int gx = SIZE_GRID_FX32( pos->x );
	int gy = SIZE_GRID_FX32( pos->y );
	int gz = SIZE_GRID_FX32( pos->z );
	MMDL *fmmdl = fld_player->fldmmdl;
	
	MMDL_SetOldGridPosX( fmmdl, MMDL_GetGridPosX(fmmdl) );
	MMDL_SetOldGridPosY( fmmdl, MMDL_GetGridPosY(fmmdl) );
	MMDL_SetOldGridPosZ( fmmdl, MMDL_GetGridPosZ(fmmdl) );
	MMDL_SetGridPosX( fmmdl, gx );
	MMDL_SetGridPosY( fmmdl, gy );
	MMDL_SetGridPosZ( fmmdl, gz );
	MMDL_SetVectorPos( fmmdl, pos );
	
	fld_player->pos = *pos;
}

//--------------------------------------------------------------
/**
 * FILED_PLAYER　方向取得
 * @param fld_player FIELD_PLAYER
 * @retval u16 方向
 */
//--------------------------------------------------------------
u16 FIELD_PLAYER_GetDir( const FIELD_PLAYER *fld_player )
{
	return( fld_player->dir );
}

//--------------------------------------------------------------
/**
 * FILED_PLAYER　方向セット
 * @param fld_player FIELD_PLAYER
 * @param dir 方向
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_SetDir( FIELD_PLAYER *fld_player, u16 dir )
{
	fld_player->dir = dir;
}

//--------------------------------------------------------------
/**
 * FILED_PLAYER　FIELDMAP_WORK取得
 * @param fld_player FIELD_PLAYER
 * @retval FIELDMAP_WORK*
 */
//--------------------------------------------------------------
FIELDMAP_WORK * FIELD_PLAYER_GetFieldMapWork( FIELD_PLAYER *fld_player )
{
	return( fld_player->fieldWork );
}

//--------------------------------------------------------------
/**
 * FILED_PLAYER　MMDL取得
 * @param fld_player FIELD_PLAYER
 * @retval MMDL*
 */
//--------------------------------------------------------------
MMDL * FIELD_PLAYER_GetMMdl( FIELD_PLAYER *fld_player )
{
	return( fld_player->fldmmdl );
}

//--------------------------------------------------------------
/**
 * FIELD_PLAYER FLDMAPPER_GRIDINFODATA取得
 * @param fld_player* FIELD_PLAYER
 * @retval FLDMAPPER_GRIDINFODATA*
 */
//--------------------------------------------------------------
FLDMAPPER_GRIDINFODATA * FIELD_PLAYER_GetGridInfoData(
		FIELD_PLAYER *fld_player )
{
	return( &fld_player->gridInfoData );
}

//--------------------------------------------------------------
/**
 * FIELD_PLAYER PLAYER_MOVE_VALUEセット
 * @param fld_player FIELD_PLAYER
 * @retval PLAYER_MOVE_STATE
 */
//--------------------------------------------------------------
void FIELD_PLAYER_SetMoveValue(
    FIELD_PLAYER *fld_player, PLAYER_MOVE_VALUE val )
{
  fld_player->move_value = val;
}

//--------------------------------------------------------------
/**
 * FIELD_PLAYER PLAYER_MOVE_VALUE取得
 * @param fld_player FIELD_PLAYER
 * @retval PLAYER_MOVE_STATE
 */
//--------------------------------------------------------------
PLAYER_MOVE_VALUE FIELD_PLAYER_GetMoveValue(
    const FIELD_PLAYER *fld_player )
{
  return( fld_player->move_value );
}

//--------------------------------------------------------------
/**
 * FIELD_PLAYER PLAYER_MOVE_STATE取得
 * @param fld_player FIELD_PLAYER
 * @retval PLAYER_MOVE_STATE
 */
//--------------------------------------------------------------
PLAYER_MOVE_STATE FIELD_PLAYER_GetMoveState(
    const FIELD_PLAYER *fld_player )
{
  return( fld_player->move_state );
}

//--------------------------------------------------------------
/**
 * FIELD_PLAYER PLAYER_MOVE_FORM取得
 * @param fld_player FIELD_PLAYER
 * @retval PLAYER_MOVE_FORM
 */
//--------------------------------------------------------------
PLAYER_MOVE_FORM FIELD_PLAYER_GetMoveForm(
    const FIELD_PLAYER *fld_player )
{
  return( fld_player->move_form );
}

//--------------------------------------------------------------
/**
 * FIELD_PLAYER PLAYER_MOVE_FORMセット
 * @param fld_player FIELD_PLAYER
 * @retval PLAYER_MOVE_FORM
 */
//--------------------------------------------------------------
void FIELD_PLAYER_SetMoveForm(
    FIELD_PLAYER *fld_player, PLAYER_MOVE_FORM form )
{
  fld_player->move_form = form;
}

//======================================================================
//	FILED_PLAYER　ツール
//======================================================================
//--------------------------------------------------------------
/**
 * 自機の指定方向先の位置をグリッド単位で取得
 * @param fld_player FIELD_PLAYER
 * @param gx X座標格納先
 * @param gy Y座標格納先
 * @param gz Z座標格納先
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_GetDirGridPos(
		FIELD_PLAYER *fld_player, u16 dir, s16 *gx, s16 *gy, s16 *gz )
{
	MMDL *fmmdl = FIELD_PLAYER_GetMMdl( fld_player );
	
	*gx = MMDL_GetGridPosX( fmmdl );
	*gy = MMDL_GetGridPosY( fmmdl );
	*gz = MMDL_GetGridPosZ( fmmdl );
	*gx += MMDL_TOOL_GetDirAddValueGridX( dir );
	*gz += MMDL_TOOL_GetDirAddValueGridZ( dir );
}

//--------------------------------------------------------------
/**
 * 自機の指定方向先の座標を取得
 * @param fld_player FIELD_PLAYER
 * @param pos 座標格納先
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_GetDirPos(
		FIELD_PLAYER *fld_player, u16 dir, VecFx32 *pos )
{
  s16 gx,gy,gz;
  FIELD_PLAYER_GetDirGridPos( fld_player, dir, &gx, &gy, &gz );
  MMDL_TOOL_GetCenterGridPos( gx, gz, pos );
  pos->y = GRID_SIZE_FX32( gy );
}

//--------------------------------------------------------------
/**
 * 自機の前方位置をグリッド単位で取得
 * @param fld_player FIELD_PLAYER
 * @param gx X座標格納先
 * @param gy Y座標格納先
 * @param gz Z座標格納先
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_GetFrontGridPos(
		FIELD_PLAYER *fld_player, s16 *gx, s16 *gy, s16 *gz )
{
	MMDL *fmmdl = FIELD_PLAYER_GetMMdl( fld_player );
	u16 dir = MMDL_GetDirDisp( fmmdl );
  FIELD_PLAYER_GetDirGridPos( fld_player, dir, gx, gy, gz );
}

//--------------------------------------------------------------
/**
 * 動作モデルが生きているかチェック
 * @param fld_player FIELD_PLAYER
 * @retval BOOL TRUE=生存
 */
//--------------------------------------------------------------
BOOL FIELD_PLAYER_CheckLiveMMdl( FIELD_PLAYER *fld_player )
{
  MMDL *fmmdl = FIELD_PLAYER_GetMMdl( fld_player );
  
  if( fmmdl == NULL ){
    return( FALSE );
  }
   
  if( MMDL_CheckStatusBitUse(fmmdl) == FALSE ){
    return( FALSE );
  }
  
  if( MMDL_GetOBJID(fmmdl) != MMDL_ID_PLAYER ){
    return( FALSE );
  }
  
  return( TRUE );
}

//======================================================================
//	data
//======================================================================
//--------------------------------------------------------------
/// 自機動作モデルヘッダー
//--------------------------------------------------------------
static const MMDL_HEADER playerdata_MMdlHeader =
{
	MMDL_ID_PLAYER,	///<識別ID
	HERO,	///<表示するOBJコード
	MV_DMY,	///<動作コード
	0,	///<イベントタイプ
	0,	///<イベントフラグ
	0,	///<イベントID
	0,	///<指定方向
	0,	///<指定パラメタ 0
	0,	///<指定パラメタ 1
	0,	///<指定パラメタ 2
	MOVE_LIMIT_NOT,	///<X方向移動制限
	MOVE_LIMIT_NOT,	///<Z方向移動制限
	0,	///<グリッドX
	0,	///<グリッドZ
	0,	///<Y値 fx32型
};

//======================================================================
//	以下消します
//======================================================================
GFL_BBDACT_RESUNIT_ID GetPlayerBBdActResUnitID( FIELD_PLAYER *fld_player )
{
	return( fld_player->bbdActResUnitID );
}

#if 0
void PlayerActGrid_Update(
	FIELD_PLAYER *fld_player, u16 dir, const VecFx32 *pos )
{
	VecFx32 trans;
	fld_player->pos = *pos;
	
	SetGridPlayerActTrans( fld_player, pos );
	MMDL_SetForceDirDisp( fld_player->fldmmdl, dir );
}
#endif

#if 0
void SetGridPlayerActTrans( FIELD_PLAYER* fld_player, const VecFx32* trans )
{
	int gx = SIZE_GRID_FX32( trans->x );
	int gy = SIZE_GRID_FX32( trans->y );
	int gz = SIZE_GRID_FX32( trans->z );
	
	MMDL_SetOldGridPosX( fld_player->fldmmdl,
		MMDL_GetGridPosX(fld_player->fldmmdl) );
	MMDL_SetOldGridPosY( fld_player->fldmmdl,
		MMDL_GetGridPosY(fld_player->fldmmdl) );
	MMDL_SetOldGridPosZ( fld_player->fldmmdl,
		MMDL_GetGridPosZ(fld_player->fldmmdl) );
	
	MMDL_SetGridPosX( fld_player->fldmmdl, gx );
	MMDL_SetGridPosY( fld_player->fldmmdl, gy );
	MMDL_SetGridPosZ( fld_player->fldmmdl, gz );
	MMDL_SetVectorPos( fld_player->fldmmdl, trans );
	
	VEC_Set( &fld_player->pos, trans->x, trans->y, trans->z );
}
#endif

#if 0
void PlayerActGrid_AnimeSet(
	FIELD_PLAYER *fld_player, int dir, PLAYER_ANIME_FLAG flag )
{
	switch( flag ){
	case PLAYER_ANIME_FLAG_STOP:
		MMDL_SetDrawStatus( fld_player->fldmmdl, DRAW_STA_STOP );
		break;
	case PLAYER_ANIME_FLAG_WALK:
		MMDL_SetDrawStatus( fld_player->fldmmdl, DRAW_STA_WALK_8F );
		break;
	default:
		MMDL_SetDrawStatus( fld_player->fldmmdl, DRAW_STA_WALK_4F );
		break;
	}
}
#endif

#if 0
FLDMAPPER_GRIDINFODATA * PlayerActGrid_GridInfoGet( FIELD_PLAYER *fld_player )
{
	return( &fld_player->gridInfoData );
}
#endif

#if 0
void PlayerActGrid_ScaleSizeSet(
	FIELD_PLAYER *fld_player, fx16 sizeX, fx16 sizeY )
{
	GFL_BBDACT_SYS *bbdActSys = FIELDMAP_GetBbdActSys( fld_player->fieldWork );
	int idx = GFL_BBDACT_GetBBDActIdxResIdx(
			bbdActSys, fld_player->bbdActActUnitID );
	GFL_BBD_SetObjectSiz(
		GFL_BBDACT_GetBBDSystem(bbdActSys),
		idx, &sizeX, &sizeY );
}
#endif

#if 0
void PLAYER_GRID_GetFrontGridPos(
	FIELD_PLAYER *fld_player, int *gx, int *gy, int *gz )
{
	int dir;
	MMDL *fmmdl = FIELD_PLAYER_GetMMdl( fld_player );
	
	*gx = MMDL_GetGridPosX( fmmdl );
	*gy = MMDL_GetGridPosY( fmmdl );
	*gz = MMDL_GetGridPosZ( fmmdl );
	dir = MMDL_GetDirDisp( fmmdl );
	
	*gx += MMDL_TOOL_GetDirAddValueGridX( dir );
	*gz += MMDL_TOOL_GetDirAddValueGridZ( dir );
}
#endif
