//////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  電光掲示板 臨時ニュース
 * @file   elboard_spnews_data.c
 * @author obata
 * @date   2009.12.14
 */
//////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "elboard_spnews_data.h"
#include "../../../resource/fldmapdata/zonetable/zone_id.h"


//--------------------------------------------------------------------------------------
/**
 * @brief 臨時ニュースデータを読み込む
 * 
 * @param buf    読み込んだデータの格納先
 * @param arc_id 読み込むデータのアーカイブID
 * @param dat_id 読み込むデータのアーカイブ内インデックス
 *
 * @return 読み込みが正しく行われた場合 TRUE
 */
//--------------------------------------------------------------------------------------
BOOL ELBOARD_SPNEWS_DATA_Load( ELBOARD_SPNEWS_DATA* buf, ARCID arc_id, ARCDATID dat_id )
{
  // 構造体へダイレクトに放り込む
  GFL_ARC_LoadDataOfs( buf, arc_id, dat_id, 0, sizeof(ELBOARD_SPNEWS_DATA) );

  // DEBUG:
  OBATA_Printf( "---------------------------------- ELBOARD_SPNEWS_DATA_Load\n" );
  OBATA_Printf( "flag        = %d\n", buf->flag );
  OBATA_Printf( "flagControl = %d\n", buf->flagControl );
  OBATA_Printf( "msgID       = %d\n", buf->msgID );
  OBATA_Printf( "newsType    = %d\n", buf->newsType );
  OBATA_Printf( "zoneID_1    = %d\n", buf->zoneID_1 );
  OBATA_Printf( "zoneID_2    = %d\n", buf->zoneID_2 );
  OBATA_Printf( "zoneID_3    = %d\n", buf->zoneID_3 );
  OBATA_Printf( "zoneID_4    = %d\n", buf->zoneID_4 );
  OBATA_Printf( "-----------------------------------------------------------\n" );
  return TRUE;
}

//--------------------------------------------------------------------------------------
/**
 * @brief 指定したゾーンに一致するかどうかを判定する
 *
 * @param news    臨時ニュース
 * @param zone_id 判定するゾーンID
 *
 * @return 指定ゾーンに一致する場合 TRUE, そうでなければ FALSE
 */
//--------------------------------------------------------------------------------------
BOOL ELBOARD_SPNEWS_DATA_CheckZoneHit( const ELBOARD_SPNEWS_DATA* news, u32 zone_id )
{
  // if(全部無効値)
  if( (news->zoneID_1 == ZONE_ID_MAX) &&  
      (news->zoneID_2 == ZONE_ID_MAX) &&
      (news->zoneID_3 == ZONE_ID_MAX) && 
      (news->zoneID_4 == ZONE_ID_MAX) ){ return TRUE; }
  // else if(どれかに一致)
  else if( (news->zoneID_1 == zone_id) ||  
           (news->zoneID_2 == zone_id) ||
           (news->zoneID_3 == zone_id) || 
           (news->zoneID_4 == zone_id) ){ return TRUE; }
  return FALSE;
}
