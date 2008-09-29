//------------------------------------------------------------------
//------------------------------------------------------------------
static void SpecialMain( FIELD_WORK* fieldWork, VecFx32 * pos )
{
	MainPlayerAct( fieldWork->pcActCont );
	FLD_MainFieldActSys( fieldWork->fldActCont );
	
	GetPlayerActTrans( fieldWork->pcActCont, pos );

	FLD_SetCameraTrans( fieldWork->camera_control, pos );
	//FLD_SetCameraDirection( fieldWork->camera_control, &dir );

	FLD_MainCamera( fieldWork->camera_control );
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
}

