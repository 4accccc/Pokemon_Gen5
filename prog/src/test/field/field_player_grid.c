//field_player_grid.c
static void playerBBDactFuncGrid(
	GFL_BBDACT_SYS* bbdActSys, int actIdx, void* work )
{
	GFL_BBD_SYS *bbdSys = GFL_BBDACT_GetBBDSystem( bbdActSys );
	PC_ACTCONT *pcActCont = (PC_ACTCONT*)work;
	VecFx32	trans = pcActCont->trans;
	
	#if 0
	dir = pcActCont->direction -
		getCameraRotate(GetG3Dcamera(pcActCont->gs) );
	anmID = getPlayerBBDanm(
		pcActCont->anmSetID, dir, playerBBDanmOffsTblMine );
	OS_Printf( "アニメID = %d\n", anmID );

	//カメラ補正(アニメ向きの変更をするのに参照)
	GFL_BBDACT_SetAnimeIdxContinue(
		GetBbdActSys( pcActCont->gs ), actIdx, anmID );
	#endif

	//位置補正
	trans.x = pcActCont->trans.x + (FX32_ONE*8) + (FX32_ONE*8);	//補正
	trans.y = pcActCont->trans.y + FX32_ONE*10;	//補正
	trans.z = pcActCont->trans.z;
	GFL_BBD_SetObjectTrans( bbdSys, actIdx, &trans );
}

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
	InitGetFieldG3DmapperGridInfoData( &pcActCont->gridInfoData );
	
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
		( bbdActSys, pcActCont->bbdActActUnitID, ARCID_FLDMAP_ACTOR, NARC_fld_act_hero_nsbtx );

	GFL_BBDACT_SetAnimeTable( bbdActSys, pcActCont->bbdActActUnitID, 
					playerBBDactAnmTable, NELEMS(playerBBDactAnmTable) );
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

void PlayerActGrid_Update(
	PC_ACTCONT *pcActCont, u16 dir, const VecFx32 *pos )
{
	VecFx32 trans;
	pcActCont->trans = *pos;
}

void PlayerActGrid_AnimeSet(
	PC_ACTCONT *pcActCont, int dir , int move )
{
	int anmBase,anmID;
//	VecFx32	trans = pcActCont->trans;
	
	if( move == FALSE ){
		anmBase = PCACTSTOP_UP;
	}else{
		anmBase = PCACTWALK_UP;
	}
	
	anmID = anmBase + dir;
	
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

FLD_G3D_MAPPER_INFODATA * PlayerActGrid_GridInfoGet( PC_ACTCONT *pcActCont )
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

