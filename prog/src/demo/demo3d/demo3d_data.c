//============================================================================
/**
 *
 *	@file		demo3d_data.c
 *	@brief
 *	@author		hosaka genya
 *	@data		2009.11.27
 *
 */
//============================================================================
#pragma once

//@TODO 後でresourceフォルダから自動生成させる予定。
//@TODO リソースID

#include <gflib.h>

#include "system/main.h"
#include "system/ica_anime.h"
#include "arc/arc_def.h"

#include "demo/demo3d.h"

#include "arc/demo3d.naix"

#include "demo3d_data.h"

// コンバートデータ
#include "demo3d_restbl.cdat"

ICA_ANIME* Demo3D_DATA_CreateICACamera( DEMO3D_ID id, HEAPID heapID, int buf_interval )
{
  GF_ASSERT( id < DEMO3D_ID_MAX );
  return ICA_ANIME_CreateStreamingAlloc( heapID, ARCID_DEMO3D_GRA, c_ica_camera_idx[id], buf_interval );
}

u8 Demo3D_DATA_GetUnitMax( DEMO3D_ID id )
{
  return SETUP_INDEX_MAX;
}

const GFL_G3D_UTIL_SETUP* Demo3D_DATA_GetG3DUtilSetup( DEMO3D_ID id, u8 setup_idx )
{
  GF_ASSERT( setup_idx < SETUP_INDEX_MAX );
  return &setup[ setup_idx ];
}



