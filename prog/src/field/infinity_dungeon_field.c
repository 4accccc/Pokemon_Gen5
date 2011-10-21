//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		infinity_dungeon_field.c
 *	@brief  無限ダンジョンフィールド常住処理
 *	@author	Masayuki Onoue
 *	@date		2011.10.21
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "infinity_dungeon_field.h"
#include "../../../resource/fldmapdata/eventdata/define/total_header.h"
#include "arc/fieldmap/zone_id.h"

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョン内の岩をリセットする
 *
 *	@param	*mmdlsys 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INF_DUNGEON_FIELD_ResetPushRock( MMDLSYS * mmdlsys )
{
  // 無限ダンジョン関連の岩データ位置をリセットする
  MMDLSYS_ROCKPOS_ResetFallParam( mmdlsys ,ZONE_ID_D31R0801 ,D31R0801_ROOCK_01 );
  MMDLSYS_ROCKPOS_ResetFallParam( mmdlsys ,ZONE_ID_D31R0801 ,D31R0801_ROOCK_02 );
  MMDLSYS_ROCKPOS_ResetFallParam( mmdlsys ,ZONE_ID_D31R0801 ,D31R0801_ROOCK_03 );
  MMDLSYS_ROCKPOS_ResetFallParam( mmdlsys ,ZONE_ID_D31R0801 ,D31R0801_ROOCK_04 );
}

