//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		animal_trail_data.c
 *	@brief  獣道用アクセサ
 *	@author	Toru=Nagihashi
 *	@data		2011.09.02
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include "fieldmap/zone_id.h"

#include "../../../resource/animal_trail/animal_trail_pos_data.cdat"  // 帰る先データ

#include "animal_trail_data.h"

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  けもの道帰るデータの取得
 *
 *	@param	u32 idx けもの道ID
 *	@param	param   データ参照IDX
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
s32 ANIMAL_TRAIL_DATA_GetExitData( u32 idx, ANIMAL_TRAIL_DATA_PARAM param )
{
  GF_ASSERT( idx < NELEMS(sc_exit_data) );
  GF_ASSERT( param < ANIMAL_TRAIL_DATA_PARAM_MAX );
  switch( param )
  {
  case ANIMAL_TRAIL_DATA_PARAM_ZONEID:  return sc_exit_data[idx].zone_id;
  case ANIMAL_TRAIL_DATA_PARAM_X:       return sc_exit_data[idx].gx;
  case ANIMAL_TRAIL_DATA_PARAM_Y:       return sc_exit_data[idx].gy;
  case ANIMAL_TRAIL_DATA_PARAM_Z:       return sc_exit_data[idx].gz;
  default: return 0;
  }
}
