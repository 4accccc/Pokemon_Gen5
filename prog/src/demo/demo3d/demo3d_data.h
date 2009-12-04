//============================================================================
/**
 *
 *	@file		demo3d_data.h
 *	@brief  コンバートして吐き出されたリソーステーブルへのアクセサ
 *	@author	hosaka genya
 *	@data		2009.11.27
 *
 */
//============================================================================
#pragma once

#include <gflib.h>

#include "demo/demo3d.h"

extern u8 Demo3D_DATA_GetUnitMax( DEMO3D_ID id );

extern const GFL_G3D_UTIL_SETUP* Demo3D_DATA_GetG3DUtilSetup( DEMO3D_ID id, u8 setup_idx );

extern ICA_ANIME* Demo3D_DATA_CreateICACamera( DEMO3D_ID id, HEAPID heapID, int buf_interval );

