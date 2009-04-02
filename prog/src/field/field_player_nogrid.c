void	MainPlayerAct_NoGrid( PC_ACTCONT* pcActCont, int key)
{
	VecFx32	vecMove = { 0, 0, 0 };
	u16		dir;
	BOOL	mvFlag = FALSE;

	dir = getCameraRotate( GetG3Dcamera(pcActCont->fieldWork) );

	if( key & PAD_KEY_UP ){
		mvFlag = TRUE;
		vecMove.x = FX_SinIdx( (u16)(dir + 0x8000) );
		vecMove.z = FX_CosIdx( (u16)(dir + 0x8000) );
		pcActCont->direction = dir;
	}
	if( key & PAD_KEY_DOWN ){
		mvFlag = TRUE;
		vecMove.x = FX_SinIdx( (u16)(dir + 0x0000) );
		vecMove.z = FX_CosIdx( (u16)(dir + 0x0000) );
		pcActCont->direction = dir + 0x8000;
	}
	if( key & PAD_KEY_LEFT ){
		mvFlag = TRUE;
		vecMove.x = FX_SinIdx( (u16)(dir + 0xc000) );
		vecMove.z = FX_CosIdx( (u16)(dir + 0xc000) );
		pcActCont->direction = dir + 0x4000;
	}
	if( key & PAD_KEY_RIGHT ){
		mvFlag = TRUE;
		vecMove.x = FX_SinIdx( (u16)(dir + 0x4000) );
		vecMove.z = FX_CosIdx( (u16)(dir + 0x4000) );
		pcActCont->direction = dir + 0xc000;
	}
#if 0
	if( key & PAD_BUTTON_R ){
		pcActCont->direction -= RT_SPEED;
	}
	if( key & PAD_BUTTON_L ){
		pcActCont->direction += RT_SPEED;
	}
#endif
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
		SetPlayerActAnm( pcActCont, ANMTYPE_WALK );
	} else {
		SetPlayerActAnm( pcActCont, ANMTYPE_STOP );
	}
}

void	MainPlayerAct_C3( PC_ACTCONT* pcActCont, int key, u16 angle)
{
	BOOL	mvFlag = FALSE;

	if( key & PAD_KEY_UP ){
		mvFlag = TRUE;
		pcActCont->direction = angle;
	}
	if( key & PAD_KEY_DOWN ){
		mvFlag = TRUE;
		pcActCont->direction = angle + 0x8000;
	}
	if( key & PAD_KEY_LEFT ){
		mvFlag = TRUE;
		pcActCont->direction = angle + 0x4000;
	}
	if( key & PAD_KEY_RIGHT ){
		mvFlag = TRUE;
		pcActCont->direction = angle + 0xc000;
	}

    
	if( mvFlag == TRUE ){
		SetPlayerActAnm( pcActCont, ANMTYPE_WALK );
	} else {
		SetPlayerActAnm( pcActCont, ANMTYPE_STOP );
	}
}
