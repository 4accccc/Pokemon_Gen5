//============================================================================================
/**
 * @file	field_sub_nogrid.c
 * @brief	フィールドメイン処理サブ（グリッド無し用）
 *
 * このソースはfield_main.cにインクルードされています。
 * 最終的にはちゃんと分割管理されますが、実験環境のために
 * しばらくはこの形式です。
 */
//============================================================================================

//------------------------------------------------------------------
/**
 * @brief	初期化処理（グリッド無し）
 */
//------------------------------------------------------------------
static void NoGridCreate( FIELD_WORK * fieldWork, VecFx32 * pos, u16 dir)
{
	fieldWork->camera_control = FLD_CreateCamera( fieldWork->gs, fieldWork->heapID );
	fieldWork->fldActCont = FLD_CreateFieldActSys( fieldWork->gs, fieldWork->heapID );
	//FLDACT_TestSetup( fieldWork->fldActCont );
	fieldWork->pcActCont = CreatePlayerAct( fieldWork->gs, fieldWork->heapID );
	SetPlayerActTrans( fieldWork->pcActCont, pos );
	SetPlayerActDirection( fieldWork->pcActCont, &dir );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void NoGridMain( FIELD_WORK* fieldWork, VecFx32 * pos )
{
	MainPlayerAct( fieldWork->pcActCont, fieldWork->key_cont );
	FLD_MainFieldActSys( fieldWork->fldActCont );
	
	GetPlayerActTrans( fieldWork->pcActCont, pos );

	FLD_SetCameraTrans( fieldWork->camera_control, pos );
	//FLD_SetCameraDirection( fieldWork->camera_control, &dir );

	FLD_MainCamera( fieldWork->camera_control, fieldWork->key_cont );
#if 0
	{
		GFL_G3D_CAMERA * g3Dcamera = GetG3Dcamera(fieldWork->gs);
		VecFx32 target, c_pos;
		c_pos = *pos;

		VEC_Set(	&target,
					c_pos.x,
					c_pos.y + CAMERA_TARGET_HEIGHT*FX32_ONE,
					c_pos.z);
		c_pos.x = pos->x + 16 * FX_SinIdx(0);
		c_pos.y = pos->y;
		c_pos.z = pos->z + 16 * FX_CosIdx(0);

		GFL_G3D_CAMERA_SetTarget( g3Dcamera, &target );
		GFL_G3D_CAMERA_SetPos( g3Dcamera, &c_pos );
	}
#endif
}

//------------------------------------------------------------------
/**
 * @brief	終了処理（グリッド無し）
 */
//------------------------------------------------------------------
static void NoGridDelete( FIELD_WORK* fieldWork )
{
	DeletePlayerAct( fieldWork->pcActCont );
	FLD_DeleteCamera( fieldWork->camera_control );
	//FLDACT_TestRelease( fieldWork->fldActCont );
	FLD_DeleteFieldActSys( fieldWork->fldActCont );
}

