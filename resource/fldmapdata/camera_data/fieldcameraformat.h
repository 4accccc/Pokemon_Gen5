//このファイルはpokemon_wb/resource/fldmapdata/camera_data/にオリジナルがあります
//------------------------------------------------------------------
//------------------------------------------------------------------ 
#pragma once

typedef	struct	{
	fx32			Distance;
	VecFx32 Angle;
	u8				View;
  u8        depthType;
	u16				PerspWay;
	fx32			Near;
	fx32			Far;
  u32       PlayerLink;
	VecFx32			Shift;
}FLD_CAMERA_PARAM;



enum {
 GF_CAMERA_PERSPECTIV = 0,
 GF_CAMERA_ORTHO
};

enum {
  DEPTH_TYPE_ZBUF = 0,
  DEPTH_TYPE_WBUF
};

