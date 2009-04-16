void	MainPlayerAct_NoGrid( PC_ACTCONT* pcActCont, int key)
{
	VecFx32	vecMove = { 0, 0, 0 };
	u16		dir;
	BOOL	mvFlag = FALSE;

	dir = FIELD_CAMERA_GetDirectionOnXZ( FIELDMAP_GetFieldCamera(pcActCont->fieldWork) );

	if( key & PAD_KEY_UP ){
		mvFlag = TRUE;
		vecMove.x = FX_SinIdx( (u16)(dir + 0x8000) );
		vecMove.z = FX_CosIdx( (u16)(dir + 0x8000) );
		pcActCont->direction = dir;
		FLDMMDL_SetDirDisp(pcActCont->pFldMMdl, DIR_UP);
	}
	if( key & PAD_KEY_DOWN ){
		mvFlag = TRUE;
		vecMove.x = FX_SinIdx( (u16)(dir + 0x0000) );
		vecMove.z = FX_CosIdx( (u16)(dir + 0x0000) );
		pcActCont->direction = dir + 0x8000;
		FLDMMDL_SetDirDisp(pcActCont->pFldMMdl, DIR_DOWN);
	}
	if( key & PAD_KEY_LEFT ){
		mvFlag = TRUE;
		vecMove.x = FX_SinIdx( (u16)(dir + 0xc000) );
		vecMove.z = FX_CosIdx( (u16)(dir + 0xc000) );
		pcActCont->direction = dir + 0x4000;
		FLDMMDL_SetDirDisp(pcActCont->pFldMMdl, DIR_LEFT);
	}
	if( key & PAD_KEY_RIGHT ){
		mvFlag = TRUE;
		vecMove.x = FX_SinIdx( (u16)(dir + 0x4000) );
		vecMove.z = FX_CosIdx( (u16)(dir + 0x4000) );
		pcActCont->direction = dir + 0xc000;
		FLDMMDL_SetDirDisp(pcActCont->pFldMMdl, DIR_RIGHT);
	}
	if (key & PAD_BUTTON_Y) {
		vecMove.y = -2 * FX32_ONE;
	}
	if (key & PAD_BUTTON_X) {
		vecMove.y = +2 * FX32_ONE;
	}
	if (key & PAD_BUTTON_B) {
		VEC_Add(&pcActCont->trans, &vecMove, &pcActCont->trans);
	} else {
		VecFx32 newPos = pcActCont->trans;
		fx32 diff;
		CalcSetGroundMove( GetFieldG3Dmapper(pcActCont->fieldWork), &pcActCont->gridInfoData, 
	//							&newPos, &vecMove, MV_SPEED );
								&pcActCont->trans, &vecMove, MV_SPEED );
#if 0
		diff = newPos.y - pcActCont->trans.y;
		if ((diff >= 0 && diff < FX32_ONE * 32)
				|| (diff < 0 && diff > FX32_ONE * -32) ){
			pcActCont->trans = newPos;
		}
#endif
	}

    
	if( mvFlag == TRUE ){
		//SetPlayerActAnm( pcActCont, ANMTYPE_WALK );
		FLDMMDL_SetDrawStatus(pcActCont->pFldMMdl, DRAW_STA_WALK);
	} else {
		//SetPlayerActAnm( pcActCont, ANMTYPE_STOP );
		FLDMMDL_SetDrawStatus(pcActCont->pFldMMdl, DRAW_STA_STOP);
	}
}

void	MainPlayerAct_C3( PC_ACTCONT* pcActCont, int key, u16 angle)
{
	BOOL	mvFlag = FALSE;

	if( key & PAD_KEY_UP ){
		mvFlag = TRUE;
		pcActCont->direction = angle;
		FLDMMDL_SetDirDisp(pcActCont->pFldMMdl, DIR_UP);
	}
	if( key & PAD_KEY_DOWN ){
		mvFlag = TRUE;
		pcActCont->direction = angle + 0x8000;
		FLDMMDL_SetDirDisp(pcActCont->pFldMMdl, DIR_DOWN);
	}
	if( key & PAD_KEY_LEFT ){
		mvFlag = TRUE;
		pcActCont->direction = angle + 0x4000;
		FLDMMDL_SetDirDisp(pcActCont->pFldMMdl, DIR_LEFT);
	}
	if( key & PAD_KEY_RIGHT ){
		mvFlag = TRUE;
		pcActCont->direction = angle + 0xc000;
		FLDMMDL_SetDirDisp(pcActCont->pFldMMdl, DIR_RIGHT);
	}

    
	if( mvFlag == TRUE ){
		//SetPlayerActAnm( pcActCont, ANMTYPE_WALK );
		FLDMMDL_SetDrawStatus(pcActCont->pFldMMdl, DRAW_STA_WALK);
	} else {
		//SetPlayerActAnm( pcActCont, ANMTYPE_STOP );
		FLDMMDL_SetDrawStatus(pcActCont->pFldMMdl, DRAW_STA_STOP);
	}
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	移動方向の地形に沿ったベクトル取得
 */
//------------------------------------------------------------------
#define WALK_LIMIT_HEIGHT ( 16 * FX32_ONE )

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
#if 0
		if( initSw == FALSE ){
			//移動制限テスト
			if(FX_Mul((height-pos->y),(height-pos->y))
					>=FX_Mul(WALK_LIMIT_HEIGHT,WALK_LIMIT_HEIGHT)){
				return FALSE;
			}
		} 
#endif
		*gridInfoData = gridInfo.gridData[p];	//グリッドデータ更新
		VEC_Set( pos, posNext.x, gridInfoData->height, posNext.z );		//位置情報更新
	}
	return TRUE;
}
	

