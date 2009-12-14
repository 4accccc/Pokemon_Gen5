//=============================================================================
/**
 *
 *	@file		intro_cmd_data.c
 *	@brief  シーンデータへのアクセサ
 *	@author	hosaka genya
 *	@data		2009.12.11
 *
 */
//=============================================================================

#include <gflib.h>

#include "sound/pm_sndsys.h" // for SEQ_SE_XXX

#include "intro_cmd_data.h"

// シーンデータ
#include "intro_cmd_data.cdat"

const INTRO_CMD_DATA* Intro_DATA_GetCmdData( INTRO_SCENE_ID scene_id )
{
  GF_ASSERT( scene_id < NELEMS(c_intro_scene_access_tbl) );
  return c_intro_scene_access_tbl[ scene_id ];
}

u16 Intro_DATA_GetSceneMax( void )
{
  return NELEMS(c_intro_scene_access_tbl);
}


