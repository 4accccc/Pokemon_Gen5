#include "test_graphic/fldmmdl_btx.naix"

extern FIELD_SETUP * FIELDMAP_GetFieldSetup( FIELD_MAIN_WORK *fieldWork );
extern FLDMMDLSYS * FIELDMAP_GetFldMMdlSys( FIELD_MAIN_WORK *fieldWork );

//field_player_grid.c
static const GFL_BBDACT_RESDATA playerGridBBDactResTable[] = {
	{ ARCID_FLDMMDL_BTX, NARC_fldmmdl_btx_obj_kabi32_nsbtx,
	GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x32, 32, 32,
	GFL_BBDACT_RESTYPE_DATACUT },
};

static const GFL_BBDACT_ANM PCGridstopLAnm[] = {
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCGridstopRAnm[] = {
	{ 4, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCGridstopUAnm[] = {
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCGridstopDAnm[] = {
	{ 9, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};

static const GFL_BBDACT_ANM PCGridwalkLAnm[] = {
	{ 1, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 3, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCGridwalkRAnm[] = {
	{ 5, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 4, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 6, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 4, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCGridwalkUAnm[] = {
	{ 7, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 8, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCGridwalkDAnm[] = {
	{ 10, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 9, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 11, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 9, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCGridrunLAnm[] = {
	{ 15, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 14, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 16, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 14, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCGridrunRAnm[] = {
	{ 15, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 14, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 16, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 14, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCGridrunUAnm[] = {
	{ 8, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 7, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 10, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 7, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCGridrunDAnm[] = {
	{ 12, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 11, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 13, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 11, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM* playerGridBBDactAnmTable[] = { 
	PCGridstopUAnm, PCGridstopDAnm, PCGridstopLAnm, PCGridstopRAnm,
	PCGridwalkUAnm, PCGridwalkDAnm, PCGridwalkLAnm, PCGridwalkRAnm,
	PCGridwalkUAnm, PCGridwalkDAnm, PCGridwalkLAnm, PCGridwalkRAnm,
	PCjumpUAnm, PCjumpDAnm, PCjumpLAnm, PCjumpRAnm,
};

#if 0	//3head test
PC_ACTCONT * CreatePlayerActGrid( FIELD_SETUP*	gs, HEAPID heapID )
{
	PC_ACTCONT*	pcActCont =
		GFL_HEAP_AllocClearMemory( heapID, sizeof(PC_ACTCONT) );
	GFL_BBDACT_SYS* bbdActSys = GetBbdActSys( gs );
	GFL_BBDACT_ACTDATA actData;
	GFL_BBDACT_ACTUNIT_ID actUnitID;
	int		i, objIdx;
	VecFx32	trans;
	u8		alpha;
	BOOL	drawEnable;
	u16		setActNum;

	pcActCont->gs = gs;
	SetPlayerActAnm( pcActCont, ANMTYPE_STOP );
	FLDMAPPER_GRIDINFODATA_Init( &pcActCont->gridInfoData );
	
	//リソースセットアップ
	pcActCont->bbdActResUnitID =
		GFL_BBDACT_AddResourceUnit(
			bbdActSys, playerBBDactResTable,
			NELEMS(playerBBDactResTable) );

	actData.resID = 0;
	actData.sizX = FX16_ONE*8-1;
	actData.sizY = FX16_ONE*8-1;
	
	actData.trans.x = 0;
	actData.trans.y = 0;
	actData.trans.z = 0;

	actData.alpha = 31;
	actData.drawEnable = TRUE;
	actData.lightMask = GFL_BBD_LIGHTMASK_01;
	actData.func = playerBBDactFuncGrid;
	actData.work = pcActCont;

	pcActCont->bbdActActUnitID = GFL_BBDACT_AddAct
				( bbdActSys, pcActCont->bbdActResUnitID, &actData, 1 );
	//GFL_BBDACT_BindActTexRes
	//		( bbdActSys, pcActCont->bbdActActUnitID, pcActCont->bbdActResUnitID+1 );
	GFL_BBDACT_BindActTexResLoad
		( bbdActSys, pcActCont->bbdActActUnitID, ARCID_FLDMMDL_BTX, NARC_fldmmdl_btx_obj_kabi32_nsbtx );

	GFL_BBDACT_SetAnimeTable( bbdActSys, pcActCont->bbdActActUnitID, 
					playerGridBBDactAnmTable, NELEMS(playerGridBBDactAnmTable) );
	GFL_BBDACT_SetAnimeIdxOn( bbdActSys, pcActCont->bbdActActUnitID, 0 );
	
	//サイズ変更
	/*
	{
		int idx;
		fx16 sizeX,sizeY;

		sizeX = sizeY = FX16_ONE*4-1;
		idx = GFL_BBDACT_GetBBDActIdxResIdx(
			bbdActSys, pcActCont->bbdActActUnitID );

		GFL_BBD_SetObjectSiz(
			GFL_BBDACT_GetBBDSystem(GetBbdActSys(pcActCont->gs)),
			idx, &sizeX, &sizeY );
	}
	*/

	return pcActCont;
}
#else
static const FLDMMDL_HEADER DATA_JikiHeader =
{
	0xff,	///<識別ID
	0,	///<表示するOBJコード
	0,	///<動作コード
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

PC_ACTCONT * CreatePlayerActGrid(
		FIELD_MAIN_WORK *fieldWork, const VecFx32 *pos, HEAPID heapID )
{
	FLDMMDL_HEADER head;
	PC_ACTCONT *pcActCont;
	FLDMMDLSYS *pFldMMdlSys;

	pFldMMdlSys = FIELDMAP_GetFldMMdlSys( fieldWork );
	pcActCont = GFL_HEAP_AllocClearMemory( heapID, sizeof(PC_ACTCONT) );
	
	pcActCont->gs = FIELDMAP_GetFieldSetup( fieldWork );
	FLDMAPPER_GRIDINFODATA_Init( &pcActCont->gridInfoData );
	
	//FLDMMDLセットアップ
	head = DATA_JikiHeader;
	head.gx = SIZE_GRID_FX32( pos->x );
	head.gz = SIZE_GRID_FX32( pos->z );
	head.gy = pos->y;
	pcActCont->pFldMMdl = FLDMMDLSYS_AddFldMMdl( pFldMMdlSys, &DATA_JikiHeader, 0 );
	
	//BLACTセットアップ
	pcActCont->bbdActActUnitID =
		FLDMMDL_BLACTCONT_AddActor( pcActCont->pFldMMdl, 0 );
	FLDMMDL_SetBlActID( pcActCont->pFldMMdl, pcActCont->bbdActActUnitID );
	return pcActCont;
}
#endif

void DeletePlayerActGrid( PC_ACTCONT* pcActCont )
{
	FLDMMDL_BLACTCONT_DeleteActor(
		pcActCont->pFldMMdl, FLDMMDL_GetBlActID(pcActCont->pFldMMdl) );
	GFL_HEAP_FreeMemory( pcActCont );
}

void PlayerActGrid_Update(
	PC_ACTCONT *pcActCont, u16 dir, const VecFx32 *pos )
{
	VecFx32 trans;
	pcActCont->trans = *pos;
	
	SetGridPlayerActTrans( pcActCont, pos );
	FLDMMDL_SetForceDirDisp( pcActCont->pFldMMdl, dir );
}

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

#if 0
void PlayerActGrid_AnimeSet(
	PC_ACTCONT *pcActCont, int dir , PLAYER_ANIME_FLAG flag )
{
	int tbl[PLAYER_ANIME_FLAG_MAX] =
		{PCACTSTOP_UP,PCACTWALK_UP,PCACTRUN_UP};
	int anmID = tbl[flag] + dir;
	
	if( pcActCont->anmSetID != anmID ){
//		OS_Printf( "自機アニメセット dir=%d, anmID=%d\n", dir, anmID );
		pcActCont->anmSetID = anmID;
		GFL_BBDACT_SetAnimeIdx(
			GetBbdActSys(pcActCont->gs), pcActCont->bbdActActUnitID, anmID );
	}
	
#if 0
	trans.x = pcActCont->trans.x;
	trans.y = pcActCont->trans.y + FX32_ONE*7;	//補正
	trans.z = pcActCont->trans.z;
	
	GFL_BBD_SetObjectTrans(
		GFL_BBDACT_GetBBDSystem(GetBbdActSys(pcActCont->gs)),
		pcActCont->bbdActActUnitID, &trans );
#endif
}
#else
void PlayerActGrid_AnimeSet(
	PC_ACTCONT *pcActCont, int dir, PLAYER_ANIME_FLAG flag )
{
#if 0
	int tbl[PLAYER_ANIME_FLAG_MAX] =
		{PCACTSTOP_UP,PCACTWALK_UP,PCACTRUN_UP};
	int anmID = tbl[flag] + dir;
	
	if( pcActCont->anmSetID != anmID ){
		pcActCont->anmSetID = anmID;
		GFL_BBDACT_SetAnimeIdx(
			GetBbdActSys(pcActCont->gs), pcActCont->bbdActActUnitID, anmID );
	}
#else	
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
#endif
}
#endif

FLDMAPPER_GRIDINFODATA * PlayerActGrid_GridInfoGet( PC_ACTCONT *pcActCont )
{
	return( &pcActCont->gridInfoData );
}

void PlayerActGrid_ScaleSizeSet(
	PC_ACTCONT *pcActCont, fx16 sizeX, fx16 sizeY )
{
	GFL_BBDACT_SYS *bbdActSys = GetBbdActSys( pcActCont->gs );
	int idx = GFL_BBDACT_GetBBDActIdxResIdx(
			bbdActSys, pcActCont->bbdActActUnitID );
	GFL_BBD_SetObjectSiz(
		GFL_BBDACT_GetBBDSystem(bbdActSys),
		idx, &sizeX, &sizeY );
}

FLDMMDL * Player_GetFldMMdl( PC_ACTCONT *pcActCont );

FLDMMDL * Player_GetFldMMdl( PC_ACTCONT *pcActCont )
{
	return pcActCont->pFldMMdl;
}

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

