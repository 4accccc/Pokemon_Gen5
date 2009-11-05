//////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  電光掲示板のゾーン依存データ
 * @file   elboard_zone_data.c
 * @author obata
 * @date   2009.10.27
 *
 */
//////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "elboard_zone_data.h"


//-----------------------------------------------------------------------------------
/**
 * @brief 電光掲示板ゾーン依存データを読み込む
 * 
 * @param buf    読み込んだデータの格納先
 * @param arc_id 読み込むデータのアーカイブID
 * @param dat_id 読み込むデータのアーカイブ内インデックス
 *
 * @return 読み込みが正しく行われた場合 TRUE
 */
//-----------------------------------------------------------------------------------
BOOL ELBOARD_ZONE_DATA_Load( ELBOARD_ZONE_DATA* buf, ARCID arc_id, ARCDATID dat_id )
{
  // 構造体へダイレクトに放り込む
  GFL_ARC_LoadDataOfs( buf, arc_id, dat_id, 0, sizeof(ELBOARD_ZONE_DATA) );

  // DEBUG:
  OBATA_Printf( "------------------------------------ ELBOARD_ZONE_DATA_Load\n" );
  OBATA_Printf( "zone_id = %d\n", buf->zone_id );
  OBATA_Printf( "x = %d\n", buf->x );
  OBATA_Printf( "y = %d\n", buf->y );
  OBATA_Printf( "z = %d\n", buf->z );
  OBATA_Printf( "dir = %d\n", buf->dir );
  OBATA_Printf( "msgID_date = %d\n", buf->msgID_date );
  OBATA_Printf( "msgID_weather = %d\n", buf->msgID_weather );
  OBATA_Printf( "msgID_infoA = %d\n", buf->msgID_infoA );
  OBATA_Printf( "msgID_infoB = %d\n", buf->msgID_infoB );
  OBATA_Printf( "msgID_infoC = %d\n", buf->msgID_infoC );
  OBATA_Printf( "msgID_infoD = %d\n", buf->msgID_infoD );
  OBATA_Printf( "msgID_infoE = %d\n", buf->msgID_infoE );
  OBATA_Printf( "msgID_infoF = %d\n", buf->msgID_infoF );
  OBATA_Printf( "msgID_infoG = %d\n", buf->msgID_infoG );
  OBATA_Printf( "msgID_infoH = %d\n", buf->msgID_infoH );
  OBATA_Printf( "msgID_infoI = %d\n", buf->msgID_infoI );
  OBATA_Printf( "msgID_cmMon = %d\n", buf->msgID_cmMon );
  OBATA_Printf( "msgID_cmTue = %d\n", buf->msgID_cmTue );
  OBATA_Printf( "msgID_cmWed = %d\n", buf->msgID_cmWed );
  OBATA_Printf( "msgID_cmThu = %d\n", buf->msgID_cmThu );
  OBATA_Printf( "msgID_cmFri = %d\n", buf->msgID_cmFri );
  OBATA_Printf( "msgID_cmSat = %d\n", buf->msgID_cmSat );
  OBATA_Printf( "msgID_cmSun = %d\n", buf->msgID_cmSun ); 
  OBATA_Printf( "zoneID_weather_1 = %d\n", buf->zoneID_weather_1 ); 
  OBATA_Printf( "zoneID_weather_2 = %d\n", buf->zoneID_weather_2 ); 
  OBATA_Printf( "zoneID_weather_3 = %d\n", buf->zoneID_weather_3 ); 
  OBATA_Printf( "zoneID_weather_4 = %d\n", buf->zoneID_weather_4 ); 
  OBATA_Printf( "-----------------------------------------------------------\n" );
  return TRUE;
}
