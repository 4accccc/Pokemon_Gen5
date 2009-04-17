//============================================================================================
/**
 * @file	field_player.c
 * @date	2008.9.29
 * @brief	（仮）フィールドプレイヤー制御
 */
//============================================================================================
#include <gflib.h>
#include "field_g3d_mapper.h"
#include "field_player.h"
#include "arc_def.h"
#include "test_graphic/fld_act.naix"

#include "fieldmap_local.h"	//FIELDMAP_GetFieldCamera
#include "field_camera.h"	//FIELD_CAMERA_～
#include "fldmmdl.h"

//============================================================================================
/**
 * @brief	プレーヤーアクター
 */
//============================================================================================
struct _PC_ACTCONT {
	HEAPID					heapID;
	FIELD_MAIN_WORK *		fieldWork;
//	u16						cameraRotate;
	GFL_BBDACT_RESUNIT_ID	bbdActResUnitID;
//	u16						bbdActResCount;
	GFL_BBDACT_ACTUNIT_ID	bbdActActUnitID;
//	u16						bbdActActCount;
	u16						direction;
	VecFx32					trans;
//	int						anmSetID;
//	BOOL					anmSetReq;			

	FLDMAPPER_GRIDINFODATA		gridInfoData;
	
	FLDMMDL *pFldMMdl;
};


//------------------------------------------------------------------
//------------------------------------------------------------------
void	SetPlayerActTrans( PC_ACTCONT* pcActCont, const VecFx32* trans )
{
	int gx = SIZE_GRID_FX32( trans->x );
	int gy = SIZE_GRID_FX32( trans->y );
	int gz = SIZE_GRID_FX32( trans->z );
	VEC_Set( &pcActCont->trans, trans->x, trans->y, trans->z );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void	GetPlayerActTrans( PC_ACTCONT* pcActCont, VecFx32* trans )
{
	VEC_Set( trans, pcActCont->trans.x, pcActCont->trans.y, pcActCont->trans.z );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void	SetPlayerActDirection( PC_ACTCONT* pcActCont, const u16* direction )
{
	pcActCont->direction  = *direction;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void	GetPlayerActDirection( PC_ACTCONT* pcActCont, u16* direction )
{
	*direction = pcActCont->direction;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GFL_BBDACT_RESUNIT_ID GetPlayerBBdActResUnitID( PC_ACTCONT *pcActCont )
{
	return( pcActCont->bbdActResUnitID );
}

//#include "field_player_nogrid.c"
//#include "field_player_grid.c"

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static void setNogridMoveValue(PC_ACTCONT * pcActCont, u16 key, VecFx32 * vec)
{
	u16		dir;
	BOOL	mvFlag = FALSE;

	dir = FIELD_CAMERA_GetDirectionOnXZ( FIELDMAP_GetFieldCamera(pcActCont->fieldWork) );

	if( key & PAD_KEY_UP ){
		mvFlag = TRUE;
		vec->x = FX_SinIdx( (u16)(dir + 0x8000) );
		vec->z = FX_CosIdx( (u16)(dir + 0x8000) );
		pcActCont->direction = dir;
		FLDMMDL_SetDirDisp(pcActCont->pFldMMdl, DIR_UP);
	}
	if( key & PAD_KEY_DOWN ){
		mvFlag = TRUE;
		vec->x = FX_SinIdx( (u16)(dir + 0x0000) );
		vec->z = FX_CosIdx( (u16)(dir + 0x0000) );
		pcActCont->direction = dir + 0x8000;
		FLDMMDL_SetDirDisp(pcActCont->pFldMMdl, DIR_DOWN);
	}
	if( key & PAD_KEY_LEFT ){
		mvFlag = TRUE;
		vec->x = FX_SinIdx( (u16)(dir + 0xc000) );
		vec->z = FX_CosIdx( (u16)(dir + 0xc000) );
		pcActCont->direction = dir + 0x4000;
		FLDMMDL_SetDirDisp(pcActCont->pFldMMdl, DIR_LEFT);
	}
	if( key & PAD_KEY_RIGHT ){
		mvFlag = TRUE;
		vec->x = FX_SinIdx( (u16)(dir + 0x4000) );
		vec->z = FX_CosIdx( (u16)(dir + 0x4000) );
		pcActCont->direction = dir + 0xc000;
		FLDMMDL_SetDirDisp(pcActCont->pFldMMdl, DIR_RIGHT);
	}
	if (key & PAD_BUTTON_Y) {
		vec->y = -2 * FX32_ONE;
	}
	if (key & PAD_BUTTON_X) {
		vec->y = +2 * FX32_ONE;
	}

	if( mvFlag == TRUE ){
		FLDMMDL_SetDrawStatus(pcActCont->pFldMMdl, DRAW_STA_WALK);
	} else {
		FLDMMDL_SetDrawStatus(pcActCont->pFldMMdl, DRAW_STA_STOP);
	}
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void	MainPlayerAct_NoGrid( PC_ACTCONT* pcActCont, int key)
{
	VecFx32	vecMove = { 0, 0, 0 };
	setNogridMoveValue(pcActCont, key, &vecMove);

	if (key & PAD_BUTTON_B) {
		VEC_Add(&pcActCont->trans, &vecMove, &pcActCont->trans);
	} else {
		VecFx32 newPos = pcActCont->trans;
		fx32 diff;
		CalcSetGroundMove( GetFieldG3Dmapper(pcActCont->fieldWork), &pcActCont->gridInfoData, 
								&pcActCont->trans, &vecMove, MV_SPEED );
	}
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
void	MainPlayerAct_C3( PC_ACTCONT* pcActCont, int key, u16 angle)
{
	VecFx32	vecMove = { 0, 0, 0 };
	setNogridMoveValue(pcActCont, key, &vecMove);
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	移動方向の地形に沿ったベクトル取得
 */
//------------------------------------------------------------------
static void GetGroundMoveVec
		( const VecFx16* vecN, const VecFx32* pos, const VecFx32* vecMove, VecFx32* result )
{
	VecFx32	vecN32, posNext;
	fx32	by, valD;

	VEC_Add( pos, vecMove, &posNext );

	VEC_Set( &vecN32, vecN->x, vecN->y, vecN->z );
	valD = -( FX_Mul(vecN32.x,pos->x) + FX_Mul(vecN32.y,pos->y) + FX_Mul(vecN32.z,pos->z) ); 
	by = -( FX_Mul( vecN32.x, posNext.x ) + FX_Mul( vecN32.z, posNext.z ) + valD );
	posNext.y = FX_Div( by, vecN32.y );

	VEC_Subtract( &posNext, pos, result );
	VEC_Normalize( result, result );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL CalcSetGroundMove( const FLDMAPPER* g3Dmapper, FLDMAPPER_GRIDINFODATA* gridInfoData, 
								VecFx32* pos, VecFx32* vecMove, fx32 speed )
{
	FLDMAPPER_GRIDINFO gridInfo;
	VecFx32	posNext, vecGround;
	fx32	height = 0;
	BOOL	initSw = FALSE;

	//VEC_Normalize( &vecMove, &vecMove );
	//
	if( (gridInfoData->vecN.x|gridInfoData->vecN.y|gridInfoData->vecN.z) == 0 ){
		//vecN = {0,0,0}の場合は初期状態
		VecFx16	vecNinit = { 0, FX16_ONE, 0 };

		GetGroundMoveVec( &vecNinit, pos, vecMove, &vecGround );
		initSw = TRUE;
	} else {
		GetGroundMoveVec( &gridInfoData->vecN, pos, vecMove, &vecGround );
	}
	VEC_MultAdd( speed, &vecGround, pos, &posNext );
	if( posNext.y < 0 ){
		posNext.y = 0;	//ベースライン
	}
	if( FLDMAPPER_CheckOutRange( g3Dmapper, &posNext ) == TRUE ){
	//	OS_Printf("マップ範囲外で移動不可\n");
		return FALSE;
	}

	//プレーヤー用動作。この位置中心に高さデータが存在するため、すべて取得して設定
	if( FLDMAPPER_GetGridInfo( g3Dmapper, &posNext, &gridInfo ) == FALSE ){
		return FALSE;
	}

	if( gridInfo.count ){
		int		i = 0, p;
		fx32	h_tmp, diff1, diff2;

		height = gridInfo.gridData[0].height;
		p = 0;
		i++;
		for( ; i<gridInfo.count; i++ ){
			h_tmp = gridInfo.gridData[i].height;

			diff1 = height - pos->y;
			diff2 = h_tmp - pos->y;

			if( FX_Mul( diff2, diff2 ) < FX_Mul( diff1, diff1 ) ){
				height = h_tmp;
				p = i;
			}
		}
		*gridInfoData = gridInfo.gridData[p];	//グリッドデータ更新
		VEC_Set( pos, posNext.x, gridInfoData->height, posNext.z );		//位置情報更新
	}
	return TRUE;
}
	
//============================================================================================
//============================================================================================

#include "test_graphic/fldmmdl_btx.naix"

extern FLDMMDLSYS * FIELDMAP_GetFldMMdlSys( FIELD_MAIN_WORK *fieldWork );


//------------------------------------------------------------------
//------------------------------------------------------------------
static const FLDMMDL_HEADER DATA_JikiHeader =
{
	0xff,	///<識別ID
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

//------------------------------------------------------------------
//------------------------------------------------------------------
PC_ACTCONT * CreatePlayerActGrid(
		FIELD_MAIN_WORK *fieldWork, const VecFx32 *pos, HEAPID heapID )
{
	FLDMMDL_HEADER head;
	PC_ACTCONT *pcActCont;
	FLDMMDLSYS *pFldMMdlSys;

	pFldMMdlSys = FIELDMAP_GetFldMMdlSys( fieldWork );
	pcActCont = GFL_HEAP_AllocClearMemory( heapID, sizeof(PC_ACTCONT) );
	
	pcActCont->fieldWork = fieldWork;
	FLDMAPPER_GRIDINFODATA_Init( &pcActCont->gridInfoData );
	
	//FLDMMDLセットアップ
	pcActCont->pFldMMdl = FLDMMDLSYS_SearchOBJID( pFldMMdlSys, 0xff );

	if( pcActCont->pFldMMdl == NULL ){	//新規
		head = DATA_JikiHeader;
		head.gx = SIZE_GRID_FX32( pos->x );
		head.gz = SIZE_GRID_FX32( pos->z );
		head.y = pos->y;
		pcActCont->pFldMMdl = FLDMMDLSYS_AddFldMMdl( pFldMMdlSys, &DATA_JikiHeader, 0 );
	}else{	//復帰
	}
	
	FLDMMDL_SetStatusBitNotZoneDelete( pcActCont->pFldMMdl, TRUE );

	return pcActCont;
}


//------------------------------------------------------------------
//------------------------------------------------------------------
void DeletePlayerActGrid( PC_ACTCONT* pcActCont )
{
	GFL_HEAP_FreeMemory( pcActCont );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void PlayerActGrid_Update(
	PC_ACTCONT *pcActCont, u16 dir, const VecFx32 *pos )
{
	VecFx32 trans;
	pcActCont->trans = *pos;
	
	SetGridPlayerActTrans( pcActCont, pos );
	FLDMMDL_SetForceDirDisp( pcActCont->pFldMMdl, dir );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void SetGridPlayerActTrans( PC_ACTCONT* pcActCont, const VecFx32* trans )
{
	int gx = SIZE_GRID_FX32( trans->x );
	int gy = SIZE_GRID_FX32( trans->y );
	int gz = SIZE_GRID_FX32( trans->z );
	
	FLDMMDL_SetOldGridPosX( pcActCont->pFldMMdl,
		FLDMMDL_GetGridPosX(pcActCont->pFldMMdl) );
	FLDMMDL_SetOldGridPosY( pcActCont->pFldMMdl,
		FLDMMDL_GetGridPosY(pcActCont->pFldMMdl) );
	FLDMMDL_SetOldGridPosZ( pcActCont->pFldMMdl,
		FLDMMDL_GetGridPosZ(pcActCont->pFldMMdl) );
	
	FLDMMDL_SetGridPosX( pcActCont->pFldMMdl, gx );
	FLDMMDL_SetGridPosY( pcActCont->pFldMMdl, gy );
	FLDMMDL_SetGridPosZ( pcActCont->pFldMMdl, gz );
	FLDMMDL_SetVectorPos( pcActCont->pFldMMdl, trans );
	
	VEC_Set( &pcActCont->trans, trans->x, trans->y, trans->z );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void PlayerActGrid_AnimeSet(
	PC_ACTCONT *pcActCont, int dir, PLAYER_ANIME_FLAG flag )
{
	switch( flag ){
	case PLAYER_ANIME_FLAG_STOP:
		FLDMMDL_SetDrawStatus( pcActCont->pFldMMdl, DRAW_STA_STOP );
		break;
	case PLAYER_ANIME_FLAG_WALK:
		FLDMMDL_SetDrawStatus( pcActCont->pFldMMdl, DRAW_STA_WALK_8F );
		break;
	default:
		FLDMMDL_SetDrawStatus( pcActCont->pFldMMdl, DRAW_STA_WALK_4F );
		break;
	}
}

//------------------------------------------------------------------
//------------------------------------------------------------------
FLDMAPPER_GRIDINFODATA * PlayerActGrid_GridInfoGet( PC_ACTCONT *pcActCont )
{
	return( &pcActCont->gridInfoData );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void PlayerActGrid_ScaleSizeSet(
	PC_ACTCONT *pcActCont, fx16 sizeX, fx16 sizeY )
{
	GFL_BBDACT_SYS *bbdActSys = GetBbdActSys( pcActCont->fieldWork );
	int idx = GFL_BBDACT_GetBBDActIdxResIdx(
			bbdActSys, pcActCont->bbdActActUnitID );
	GFL_BBD_SetObjectSiz(
		GFL_BBDACT_GetBBDSystem(bbdActSys),
		idx, &sizeX, &sizeY );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
FLDMMDL * Player_GetFldMMdl( PC_ACTCONT *pcActCont );

FLDMMDL * Player_GetFldMMdl( PC_ACTCONT *pcActCont )
{
	return pcActCont->pFldMMdl;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void PLAYER_GRID_GetFrontGridPos(
	PC_ACTCONT *pcActCont, int *gx, int *gy, int *gz )
{
	int dir;
	FLDMMDL *fmmdl = Player_GetFldMMdl( pcActCont );
	
	*gx = FLDMMDL_GetGridPosX( fmmdl );
	*gy = FLDMMDL_GetGridPosY( fmmdl );
	*gz = FLDMMDL_GetGridPosZ( fmmdl );
	dir = FLDMMDL_GetDirDisp( fmmdl );
	
	*gx += FLDMMDL_TOOL_GetDirAddValueGridX( dir );
	*gz += FLDMMDL_TOOL_GetDirAddValueGridZ( dir );
}

