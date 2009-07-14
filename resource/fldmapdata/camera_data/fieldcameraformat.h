//このファイルはpokemon_wb/resource/fldmapdata/camera_data/にオリジナルがあります
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef	struct	{
	fx32			Distance;
	VecFx32 Angle;
	u16				View;
	u16				PerspWay;
	fx32			Near;
	fx32			Far;
	VecFx32			Shift;
}FLD_CAMERA_PARAM;



enum {
 GF_CAMERA_PERSPECTIV = 0,
 GF_CAMERA_ORTHO
};

