//============================================================================================
/**
 * @file	field_sub_grid.c
 * @brief	フィールドメイン処理サブ（グリッド移動）
 *
 * このソースはfield_main.cにインクルードされています。
 * 最終的にはちゃんと分割管理されますが、実験環境のために
 * しばらくはこの形式です。
 */
//============================================================================================

//------------------------------------------------------------------
/**
 * @brief	初期化処理（デフォルト）
 */
//------------------------------------------------------------------
static void GridMoveCreate( FIELD_WORK * fieldWork, VecFx32 * pos, u16 dir)
{
	fieldWork->camera_control = FLD_CreateCamera( fieldWork->gs, fieldWork->heapID );
	fieldWork->fldActCont = FLD_CreateFieldActSys( fieldWork->gs, fieldWork->heapID );
	fieldWork->pcActCont = CreatePlayerAct( fieldWork->gs, fieldWork->heapID );
	SetPlayerActTrans( fieldWork->pcActCont, pos );
	SetPlayerActDirection( fieldWork->pcActCont, &dir );
}

//------------------------------------------------------------------
/**
 * @brief	メイン処理（デフォルト）
 */
//------------------------------------------------------------------
static void GridMoveMain( FIELD_WORK* fieldWork, VecFx32 * pos )
{
	MainPlayerAct( fieldWork->pcActCont, fieldWork->key_cont );
	FLD_MainFieldActSys( fieldWork->fldActCont );
	
	GetPlayerActTrans( fieldWork->pcActCont, pos );

	FLD_SetCameraTrans( fieldWork->camera_control, pos );
	//FLD_SetCameraDirection( fieldWork->camera_control, &dir );

	FLD_MainCamera( fieldWork->camera_control, fieldWork->key_cont );
}

//------------------------------------------------------------------
/**
 * @brief	終了処理（デフォルト）
 */
//------------------------------------------------------------------
static void GridMoveDelete( FIELD_WORK* fieldWork )
{
	DeletePlayerAct( fieldWork->pcActCont );
	FLD_DeleteCamera( fieldWork->camera_control );
	FLD_DeleteFieldActSys( fieldWork->fldActCont );
}

