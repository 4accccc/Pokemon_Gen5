//============================================================================
/**
 *
 *	@file		demo3d_restbl.h
 *	@brief  コンバートして吐き出されたリソーステーブルへのアクセサ
 *	@author	hosaka genya
 *	@data		2009.11.27
 *
 */
//============================================================================
#pragma once

#include <gflib.h>
#include "system/main.h"
#include "system/ica_anime.h"
#include "arc/arc_def.h"

enum
{ 
  DEMO_ID_NULL = 0,
  DEMO_ID_CRUISER, ///< 遊覧船

  DEMO_ID_MAX,
};

extern u8 Demo3D_RESTBL_GetUnitMax( void );

extern const GFL_G3D_UTIL_SETUP* Demo3D_RESTBL_GetG3DUtilSetup( u8 idx );

extern ICA_ANIME* Demo3D_RESTBL_CreatICACamera( HEAPID heapID, int buf_interval );

