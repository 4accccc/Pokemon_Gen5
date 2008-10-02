//======================================================================
/**
 * @file	field_sub_grid.c
 * @brief	フィールドメイン処理サブ（グリッド移動）
 *
 * このソースはfield_main.cにインクルードされています。
 * 最終的にはちゃんと分割管理されますが、実験環境のために
 * しばらくはこの形式です。
 */
//======================================================================

//======================================================================
//	define
//======================================================================
enum
{
	DIR_UP = 0,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT,
	DIR_NOT,
	DIR_MAX4 = DIR_NOT,
};

#define PAD_KEY_BIT (PAD_KEY_UP|PAD_KEY_DOWN|PAD_KEY_LEFT|PAD_KEY_RIGHT)

#define NUM_FX32(a) (FX32_ONE*(a))
#define FX32_NUM(a) ((a)/FX32_ONE)

#define GRID_VALUE	(16)
#define GRID_VALUE_FX32 (GRID_VALUE*FX32_ONE)

#define NUM_GRID(a)	((a)/GRID_VALUE)
#define GRID_NUM(a) ((a)*GRID_VALUE)
#define NUM_GRID_FX32(a) (NUM_FX32(NUM_GRID(a)))
#define GRID_NUM_FX32(a) (FX32_NUM(GRID_NUM(a)))

//======================================================================
//	typedef struct
//======================================================================
typedef struct _FGRID_CONT		FGRID_CONT;
typedef struct _FGRID_PLAYER	FGRID_PLAYER;

//--------------------------------------------------------------
///	FGRID_CONT
//--------------------------------------------------------------
struct _FGRID_CONT
{
	HEAPID heapID;
	FIELD_WORK *pFieldWork;
	
	FGRID_PLAYER *pGridPlayer;
};

//--------------------------------------------------------------
///	FGRID_PLAYER
//--------------------------------------------------------------
struct _FGRID_PLAYER
{
	s16 dir;
	s16 gx;
	s16 gy;
	s16 gz;
	VecFx32 vec_pos;
	
	u16 move_flag;
	s16 move_dir;
	VecFx32 move_val;
	VecFx32 move_count;
	
	const FGRID_CONT *pGridCont;
	PC_ACTCONT *pActCont;
};

//======================================================================
//	proto
//======================================================================
static FGRID_CONT * FGridCont_Init( FIELD_WORK *fieldWork );
static void FGridCont_Delete( FIELD_WORK *fieldWork );

static FGRID_PLAYER * FGridPlayer_Init( FGRID_CONT *pGridCont );
static void FGridPlayer_Delete( FGRID_CONT *pGridCont );
static void FGridPlayer_Move(
	FGRID_PLAYER *pJiki, u32 key_trg, u32 key_cont );

static BOOL GridVecPosMove(
	const FLD_G3D_MAPPER *g3Dmapper,
	FLD_G3D_MAPPER_INFODATA *gridInfoData,
	VecFx32 *pos, VecFx32 *vecMove );

//======================================================================
//	フィールドグリッドタイプ　メイン
//======================================================================
//------------------------------------------------------------------
/**
 * @brief	初期化処理（デフォルト）
 */
//------------------------------------------------------------------
static void GridMoveCreate( FIELD_WORK * fieldWork, VecFx32 * pos, u16 dir)
{
	fieldWork->camera_control =
		FLD_CreateCamera( fieldWork->gs, fieldWork->heapID );
	fieldWork->fldActCont =
		FLD_CreateFieldActSys( fieldWork->gs, fieldWork->heapID );
	
	FLDACT_TestSetup( fieldWork->fldActCont );
	
	fieldWork->pcActCont = CreatePlayerActGrid(
			fieldWork->gs, fieldWork->heapID );
	
	SetPlayerActTrans( fieldWork->pcActCont, pos );
	SetPlayerActDirection( fieldWork->pcActCont, &dir );
	
	fieldWork->pGridCont = FGridCont_Init( fieldWork );
	
	//カメラ設定
	FLD_SetCameraLength( fieldWork->camera_control, 0x58 );
	FLD_SetCameraHeight( fieldWork->camera_control, 0x48000 );
}

//------------------------------------------------------------------
/**
 * @brief	メイン処理（デフォルト）
 */
//------------------------------------------------------------------
static void GridMoveMain( FIELD_WORK* fieldWork, VecFx32 * pos )
{
//	MainPlayerAct( fieldWork->pcActCont, fieldWork->key_cont );
	{
		FGRID_CONT *pGridCont = fieldWork->pGridCont;
		FGRID_PLAYER *pGridPlayer = pGridCont->pGridPlayer;
		FGridPlayer_Move( pGridPlayer, 0, fieldWork->key_cont );
	}
	
	if( (fieldWork->key_cont & PAD_BUTTON_L) ){
		FLD_SetCameraLength( fieldWork->camera_control, 0x58 );
		FLD_SetCameraHeight( fieldWork->camera_control, 0x48000 );
	}else if( (fieldWork->key_cont & PAD_BUTTON_R) ){
		FLD_SetCameraLength( fieldWork->camera_control, 0x68 );
		FLD_SetCameraHeight( fieldWork->camera_control, 0x64000 );
	}
	
	FLD_MainFieldActSys( fieldWork->fldActCont );
	GetPlayerActTrans( fieldWork->pcActCont, pos );
	FLD_SetCameraTrans( fieldWork->camera_control, pos );
//	FLD_SetCameraDirection( fieldWork->camera_control, &dir );
	FLD_MainCamera( fieldWork->camera_control, 0 );
}

//------------------------------------------------------------------
/**
 * @brief	終了処理（デフォルト）
 */
//------------------------------------------------------------------
static void GridMoveDelete( FIELD_WORK* fieldWork )
{
	FGridCont_Delete( fieldWork );
	
	DeletePlayerAct( fieldWork->pcActCont );
	FLD_DeleteCamera( fieldWork->camera_control );
	FLDACT_TestRelease( fieldWork->fldActCont );
	FLD_DeleteFieldActSys( fieldWork->fldActCont );
}

//======================================================================
//	グリッド処理
//======================================================================
//--------------------------------------------------------------
/**
 * グリッド処理　初期化
 * @param	fieldWork	FIELD_WORK
 * @retval	FGRID_CONT*
 */
//--------------------------------------------------------------
static FGRID_CONT * FGridCont_Init( FIELD_WORK *fieldWork )
{
	FGRID_CONT *pGridCont;
	
	pGridCont = GFL_HEAP_AllocClearMemory(
			fieldWork->heapID, sizeof(FGRID_CONT) );
	
	pGridCont->heapID = fieldWork->heapID;
	pGridCont->pFieldWork = fieldWork;
	pGridCont->pGridPlayer = FGridPlayer_Init( pGridCont );
	
	return( pGridCont );
}

//--------------------------------------------------------------
/**
 * グリッド処理　削除
 * @param	fieldWork	FIELD_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FGridCont_Delete( FIELD_WORK *fieldWork )
{
	FGRID_CONT *pGridCont;
	pGridCont = fieldWork->pGridCont;
	
	FGridPlayer_Delete( pGridCont );
	
	GFL_HEAP_FreeMemory( pGridCont );
	fieldWork->pGridCont = NULL;
}

//======================================================================
//	グリッド　自機
//======================================================================
//--------------------------------------------------------------
/**
 * グリッド自機　初期化
 * @param	pGridCont	FGRID_CONT
 * @retval	FGRID_PLAYER*
 */
//--------------------------------------------------------------
static FGRID_PLAYER * FGridPlayer_Init( FGRID_CONT *pGridCont )
{
	VecFx32 pos;
	FGRID_PLAYER *pJiki;

	pJiki = GFL_HEAP_AllocClearMemory(
			pGridCont->heapID, sizeof(FGRID_PLAYER) );
	pGridCont->pGridPlayer = pJiki;
	
	pJiki->pGridCont = pGridCont;
	pJiki->pActCont = pGridCont->pFieldWork->pcActCont;
	
	GetPlayerActTrans( pJiki->pActCont, &pos );
	pJiki->vec_pos = pos;

	return( pJiki );
}

//--------------------------------------------------------------
/**
 * グリッド自機　削除
 * @param	pGridCont	FGRID_CONT
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FGridPlayer_Delete( FGRID_CONT *pGridCont )
{
	FGRID_PLAYER *pJiki = pGridCont->pGridPlayer;
	GFL_HEAP_FreeMemory( pJiki );
	pGridCont->pGridPlayer = NULL;
}

//--------------------------------------------------------------
/**
 * グリッド自機　移動処理
 * @param	pJiki	FGRID_PLAYER
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FGridPlayer_Move(
	FGRID_PLAYER *pJiki, u32 key_trg, u32 key_cont )
{
	PC_ACTCONT *pcActCont = pJiki->pGridCont->pFieldWork->pcActCont;

	if( pJiki->move_flag == FALSE && (key_cont&PAD_KEY_BIT) ){
		int dir = DIR_NOT;

		if( (key_cont&PAD_KEY_UP) ){ dir = DIR_UP; }
		else if( (key_cont&PAD_KEY_DOWN) ){ dir = DIR_DOWN; }
		else if( (key_cont&PAD_KEY_LEFT) ){ dir = DIR_LEFT; }
		else if( (key_cont&PAD_KEY_RIGHT) ){ dir = DIR_RIGHT; }
		
		if( dir != DIR_NOT ){
			pJiki->move_flag = TRUE;
			pJiki->dir = dir;
			pJiki->move_dir = dir;
			
			switch( dir ){
			case DIR_UP:	pJiki->move_val.z = NUM_FX32( -1 ); break;
			case DIR_DOWN:	pJiki->move_val.z = NUM_FX32( 1 ); break;
			case DIR_LEFT:	pJiki->move_val.x = NUM_FX32( -1 ); break;
			case DIR_RIGHT: pJiki->move_val.x = NUM_FX32( 1 ); break;
			}
			
			if( (key_cont & PAD_BUTTON_B) ){
				pJiki->move_val.x <<= 1;
				pJiki->move_val.y <<= 1;
				pJiki->move_val.z <<= 1;
			}
		}
	}
	
	if( pJiki->move_flag == TRUE ){
		fx32 count = 0;
#if 0
		pJiki->vec_pos.x += pJiki->move_val.x;
		pJiki->vec_pos.y += pJiki->move_val.y;
		pJiki->vec_pos.z += pJiki->move_val.z;
#else
		GridVecPosMove(
			GetFieldG3Dmapper(pJiki->pGridCont->pFieldWork->gs),
			PlayerActGrid_GridInfoGet(pcActCont),
			&pJiki->vec_pos, &pJiki->move_val );
		
		OS_Printf( "x=%d,z=%d\n",
			FX32_NUM(pJiki->vec_pos.x), FX32_NUM(pJiki->vec_pos.y) );
		OS_Printf( "val_x=%x,val_z=%x\n",
			pJiki->move_val.x, pJiki->move_val.y );
#endif
		pJiki->move_count.x += pJiki->move_val.x;
		pJiki->move_count.y += pJiki->move_val.y;
		pJiki->move_count.z += pJiki->move_val.z;
		
		PlayerActGrid_Update( pcActCont, pJiki->dir, &pJiki->vec_pos );
		
		switch( pJiki->move_dir ){
		case DIR_UP:
		case DIR_DOWN: count = pJiki->move_count.z; break;
		case DIR_LEFT:
		case DIR_RIGHT: count = pJiki->move_count.x; break;
		}
		
		if( count < 0 ){
			count = -count;
		}

//		OS_Printf( "count=%x, MAX %x\n", count, GRID_VALUE_FX32 );

		if( count >= GRID_VALUE_FX32 ){
			pJiki->move_flag = FALSE;
			pJiki->move_dir = DIR_NOT;
			pJiki->move_val.x = 0;
			pJiki->move_val.y = 0;
			pJiki->move_val.z = 0;
			pJiki->move_count.x = 0;
			pJiki->move_count.y = 0;
			pJiki->move_count.z = 0;
		}
	}
	
	{
		int dir = 0;
		
		switch( pJiki->dir ){
		case DIR_UP:	dir = 3; break;
		case DIR_DOWN:	dir = 1; break;
		case DIR_LEFT:	dir = 0; break;
		case DIR_RIGHT:	dir = 2; break;
		}
		PlayerActGrid_AnimeSet( pcActCont, dir, pJiki->move_flag );
	}
}

//======================================================================
//	parts
//======================================================================
//--------------------------------------------------------------
/**
 * 指定座標を動かす
 * @param
 * @retval
 */
//--------------------------------------------------------------
static BOOL GridVecPosMove(
	const FLD_G3D_MAPPER *g3Dmapper,
	FLD_G3D_MAPPER_INFODATA *gridInfoData,
	VecFx32 *pos, VecFx32 *vecMove )
{
	fx32	height = 0;
	BOOL	initSw = FALSE;
	FLD_G3D_MAPPER_GRIDINFO gridInfo;
	VecFx32	posNext, vecGround;
	
	VEC_Add( pos, vecMove, &posNext );
	
	if( posNext.y < 0 ){
		posNext.y = 0;	//ベースライン
	}
	
	if( CheckFieldG3DmapperOutRange(g3Dmapper,&posNext) == TRUE ){
		return FALSE;
	}
	
	if( GetFieldG3DmapperGridInfo(g3Dmapper,&posNext,&gridInfo) == FALSE ){
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
		VEC_Set( pos, posNext.x, gridInfoData->height, posNext.z );
	}
	
	return( TRUE );
}

//======================================================================
//======================================================================
//======================================================================
//======================================================================
#if 0
//--------------------------------------------------------------
//	アクター　コントロールセットアップ
//--------------------------------------------------------------
static FGRID_ACTCONT * GridAct_SetupCont(
	FLD_ACTCONT *fldActCont, u32 heapID, int max )
{
	FGRID_ACTCONT *pGridActCont;
	
	pGridActCont = GFL_HEAP_AllocClearMemory(
			heapID, sizeof(FGRID_ACTCONT) );
	
	pGridActCont->max = max;
	pGridActCont->heapID = heapID;
	
	pGridActCont->pGridActTbl = GFL_HEAP_AllocClearMemory(
			heapID, sizeof(FGRID_ACTOR)*max );
	
	pGridActCont->pFldActCont = fldActCont;
	
	GridAct_SetupBBD( fldActCont, max );
	return( pActCont );
}
#endif

#if 0
//--------------------------------------------------------------
// アクター	ビルボードシステムセットアップ
//--------------------------------------------------------------
static void GridAct_SetupBBD( FLD_ACTCONT *fldActCont, int max )
{
	GFL_BBDACT_SYS* bbdActSys = GetBbdActSys( fldActCont->gs );	
	GFL_BBDACT_ACTDATA* actData;
	GFL_BBDACT_ACTUNIT_ID actUnitID;
	int		i, objIdx;
	VecFx32	trans;
	u8		alpha;
	BOOL	drawEnable;
	u16		setActNum;
	
	//リソースセットアップ
	fldActCont->bbdActResCount = NELEMS( testResTable );
	fldActCont->bbdActResUnitID = GFL_BBDACT_AddResourceUnit(
				bbdActSys, 
				testResTable, 
				fldActCont->bbdActResCount );
	
	fldActCont->bbdActActCount = FLD_BBDACT_ACTMAX;
}
#endif

#if 0
//--------------------------------------------------------------
///	アクター　アクター領域作成
//--------------------------------------------------------------
static void GridAct_ActWorkCreate( 

	//ＮＰＣアクターセットアップ
	{
		u16	setActNum = TEST_NPC_SETNUM;
		
		GFL_BBDACT_ACTDATA *actData = GFL_HEAP_AllocClearMemory(
				fldActCont->heapID,
				setActNum *s izeof(GFL_BBDACT_ACTDATA) );
		fx32 mapSizex, mapSizez;

		GetFieldG3DmapperSize(
			GetFieldG3Dmapper( fldActCont->gs ), &mapSizex, &mapSizez );
		
		for( i=0; i<setActNum; i++ ){
			actData[i].resID = GFUser_GetPublicRand( 10 )+1;
			actData[i].sizX = FX16_ONE*8-1;
			actData[i].sizY = FX16_ONE*8-1;
	
			actData[i].trans.x = (GFUser_GetPublicRand(mapSizex) );
			actData[i].trans.y = 0;
			actData[i].trans.z = (GFUser_GetPublicRand(mapSizez) );
			
			actData[i].alpha = 31;
			actData[i].drawEnable = TRUE;
			actData[i].lightMask = GFL_BBD_LIGHTMASK_01;
			actData[i].func = testFunc;
			actData[i].work = fldActCont;
		}
		fldActCont->bbdActActUnitID = GFL_BBDACT_AddAct( bbdActSys, fldActCont->bbdActResUnitID,
														actData, setActNum );
		for( i=0; i<setActNum; i++ ){
			GFL_BBDACT_SetAnimeTable( bbdActSys, fldActCont->bbdActActUnitID+i, 
										testAnmTable, NELEMS(testAnmTable) );
			GFL_BBDACT_SetAnimeIdxOn( bbdActSys, fldActCont->bbdActActUnitID+i, 0 );
		}
		GFL_HEAP_FreeMemory( actData );
	}
}
#endif
