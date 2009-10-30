//============================================================================================
/**
 * @file	zonedata.c
 * @brief	ゾーン別データ
 * @author	tamada GAME FREAK inc.
 * @date	08.11.12
 */
//============================================================================================

#include <gflib.h>

#include "arc/arc_def.h"
#include "arc/fieldmap/zonedata.naix"
#include "arc/fieldmap/zone_id.h"
#include "arc/fieldmap/map_matrix.naix"

#include "field/field_const.h"

#include "field/zonedata.h"
#include "zonetableformat.h"
#include "gamesystem/pm_season.h"

#include "arc/fieldmap/field_rail_setup.naix"  //NARC_field_rail_data_～
#include "../../resource/fldmapdata/zonetable/header/maptype.h"


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	範囲チェック
 */
//------------------------------------------------------------------
static inline u16 check_range(u16 zone_id)
{
	GF_ASSERT(zone_id < ZONE_ID_MAX);
	if ( zone_id >= ZONE_ID_MAX ) {
		return 0;
	} else {
		return zone_id;
	}
}
#define	CHECK_RANGE(value)	{value = check_range(value);}

//------------------------------------------------------------------
/**
 * @brief	ゾーンの最大数の取得
 * @return	u16	最大数
 */
//------------------------------------------------------------------
u16 ZONEDATA_GetZoneIDMax(void)
{
	return ZONE_ID_MAX;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static ZONEDATA * loadZoneData(HEAPID heapID)
{
	ZONEDATA * buffer;
	buffer = GFL_ARC_LoadDataAlloc(ARCID_ZONEDATA, NARC_zonedata_zonetable_bin, heapID);
	return buffer;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static ZONEDATA * getZoneData(ZONEDATA * zdbuf, u16 zone_id)
{
	CHECK_RANGE(zone_id);	//範囲外チェック
	GFL_ARC_LoadDataOfs(zdbuf,
			ARCID_ZONEDATA, NARC_zonedata_zonetable_bin,
			sizeof(ZONEDATA) * zone_id, sizeof(ZONEDATA));
	return zdbuf;
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	エリア指定の取得
 * @param	zone_id	ゾーン指定ID
 * @return	u16	エリア指定値
 */
//------------------------------------------------------------------
u16 ZONEDATA_GetAreaID(u16 zone_id)
{
	ZONEDATA zdbuf;
	getZoneData(&zdbuf, zone_id);
	return zdbuf.area_id;
}
//------------------------------------------------------------------
/**
 * @brief	マップアーカイブ指定の取得
 * @param	zone_id	ゾーン指定ID
 * @return	u8		map archive指定
 *
 */
//------------------------------------------------------------------
u8 ZONEDATA_GetMapArc(u16 zone_id)
{
	ZONEDATA zdbuf;
	getZoneData(&zdbuf, zone_id);
	return zdbuf.matrix_arc;
}
//------------------------------------------------------------------
/**
 * @brief	マトリックス指定の取得
 * @param	zone_id	ゾーン指定ID
 * @return	u16	マトリックス指定値
 */
//------------------------------------------------------------------
u16 ZONEDATA_GetMatrixID(u16 zone_id)
{
	ZONEDATA zdbuf;
	getZoneData(&zdbuf, zone_id);
	return zdbuf.matrix_id;
}
//------------------------------------------------------------------
/**
 * @brief	マップ構造指定の取得
 * @param	zone_id	ゾーン指定ID
 * @return	u16	マップ構造指定値
 */
//------------------------------------------------------------------
u16 ZONEDATA_GetMapRscID(u16 zone_id)
{
	ZONEDATA zdbuf;
	getZoneData(&zdbuf, zone_id);
	return zdbuf.maprsc_id;
}

//------------------------------------------------------------------
/**
 * @brief スクリプト指定IDの取得
 * @param	zone_id	ゾーン指定ID
 * @return	u16	スクリプトアーカイブでの指定ID
 */
//------------------------------------------------------------------
u16 ZONEDATA_GetScriptArcID(u16 zone_id)
{ 
  ZONEDATA zdbuf;
  getZoneData(&zdbuf, zone_id);
  return zdbuf.script_id;
}

//------------------------------------------------------------------
/**
 * @brief 特殊スクリプト指定IDの取得
 * @param	zone_id	ゾーン指定ID
 * @return	u16	スクリプトアーカイブでの指定ID
 */
//------------------------------------------------------------------
u16 ZONEDATA_GetSpScriptArcID(u16 zone_id)
{ 
  ZONEDATA zdbuf;
  getZoneData(&zdbuf, zone_id);
  return zdbuf.sp_script_id;
}

//------------------------------------------------------------------
/**
 * @brief フィールドメッセージ指定IDの取得
 * @param	zone_id	ゾーン指定ID
 * @return	u16	メッセージアーカイブでの指定ID
 */
//------------------------------------------------------------------
u16 ZONEDATA_GetMessageArcID(u16 zone_id)
{ 
  ZONEDATA zdbuf;
  getZoneData(&zdbuf, zone_id);
  return zdbuf.msg_id;
}

//------------------------------------------------------------------
/**
 * @brief	マップ開始位置の取得（デバッグ用）
 * @param	zone_id	ゾーン指定ID
 * @param	pos		開始位置を受け取るためのfx32型ポインタ
 */
//------------------------------------------------------------------
void ZONEDATA_DEBUG_GetStartPos(u16 zone_id, VecFx32 * pos)
{
	ZONEDATA zdbuf;
	getZoneData(&zdbuf, zone_id);
	pos->x = zdbuf.sx * FX32_ONE * FIELD_CONST_GRID_SIZE;
	pos->y = zdbuf.sy * FX32_ONE * FIELD_CONST_GRID_SIZE;
	pos->z = zdbuf.sz * FX32_ONE * FIELD_CONST_GRID_SIZE;
	TAMADA_Printf("%s x,y,z=%d,%d,%d\n",__FILE__,pos->x, pos->y, pos->z);
}

//----------------------------------------------------------------------------
/**
 * @brief	マップ開始レール位置の取得（デバッグ用）
 *
 *	@param	zone_id
 *	@param	pos
 */
//-----------------------------------------------------------------------------
void ZONEDATA_DEBUG_GetStartRailPos(u16 zone_id, VecFx32 * pos)
{
	ZONEDATA zdbuf;
	getZoneData(&zdbuf, zone_id);
	pos->x = zdbuf.sx;
	pos->y = zdbuf.sy;
	pos->z = zdbuf.sz;
}

//------------------------------------------------------------------
/**
 * @brief	カメラ指定IDの取得
 * @param	zone_id	ゾーン指定ID
 * @return	u8		カメラ指定IDの値
 */
//------------------------------------------------------------------
u8 ZONEDATA_GetCameraID(u16 zone_id)
{
	ZONEDATA zdbuf;
	getZoneData(&zdbuf, zone_id);
	return zdbuf.camera_id;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラエリア指定IDの取得
 *	@param	zone_id ゾーン指定ID
 *	@return カメラエリア指定ID
 */
//-----------------------------------------------------------------------------
u16 ZONEDATA_GetCameraAreaID(u16 zone_id)
{
	ZONEDATA zdbuf;
	getZoneData(&zdbuf, zone_id);
	return zdbuf.camera_area;
}

//------------------------------------------------------------------
/**
 * @brief	BGM指定IDの取得
 * @param	zone_id		ゾーン指定ID
 * @param	season_id	季節指定ID
 * @return	u8		BGM指定IDの値
 */
//------------------------------------------------------------------
u16 ZONEDATA_GetBGMID(u16 zone_id, u8 season_id)
{
	ZONEDATA zdbuf;
	getZoneData(&zdbuf, zone_id);
	switch (season_id) {
	case PMSEASON_SPRING:
		return zdbuf.bgm_spring_id;
	case PMSEASON_SUMMER:
		return zdbuf.bgm_summer_id;
	case PMSEASON_AUTUMN:
		return zdbuf.bgm_autumn_id;
	case PMSEASON_WINTER:
		return zdbuf.bgm_winter_id;
	}
	GF_ASSERT(season_id < PMSEASON_TOTAL);
	return 0;
}

//------------------------------------------------------------------
/**
 * @brief 天候指定IDの取得
 * @param	zone_id	ゾーン指定ID
 * @return  u16 天候指定ID（include/field/weather_no.hを参照）
 */
//------------------------------------------------------------------
u16 ZONEDATA_GetWeatherID(u16 zone_id)
{
	ZONEDATA zdbuf;
	getZoneData(&zdbuf, zone_id);
  return zdbuf.weather_id;
}

//------------------------------------------------------------------
/**
 * @brief	特殊なサンプルOBJを使用するかどうかの設定取得
 * @param	zone_id	ゾーン指定ID
 * @param	BOOL	TRUEのとき、サンプルOBJを使用する
 */
//------------------------------------------------------------------
BOOL ZONEDATA_DEBUG_IsSampleObjUse(u16 zone_id)
{
	ZONEDATA zdbuf;
	getZoneData(&zdbuf, zone_id);
	return zdbuf.movemodel_id != 0;
}

//------------------------------------------------------------------
/**
 * @brief レール使用マップかどうかのチェック
 *
 *
 */
//------------------------------------------------------------------
BOOL ZONEDATA_IsRailMap(u16 zone_id)
{
  return (ZONEDATA_GetRailDataID(zone_id) != ZONEDATA_NO_RAILDATA_ID);
}

//------------------------------------------------------------------
/**
 *  @brief  レールデータ指定IDを取得
 *  @param  zone_id ゾーン指定ID
 *  @retval ZONEDATA_NO_RAILDATA_ID( == -1)  レールデータが存在しない
 *  @retval other レールデータアーカイブ指定ID
 */
//------------------------------------------------------------------
int ZONEDATA_GetRailDataID(u16 zone_id)
{
  switch (zone_id)
  {
  case ZONE_ID_C03:
    return NARC_field_rail_setup_C03_dat;
  case ZONE_ID_H01:
    return NARC_field_rail_setup_H01_dat;
  case ZONE_ID_C03P02:
    return NARC_field_rail_setup_C03P02_dat;
  case ZONE_ID_D09:
    return NARC_field_rail_setup_D09_dat;

#ifdef DEBUG_ONLY_FOR_tomoya_takahashi
  case ZONE_ID_T01:
    return NARC_field_rail_setup_T01_dat;
#endif
  }
  return ZONEDATA_NO_RAILDATA_ID;
}
//------------------------------------------------------------------
/**
 * @brief  ユニオンルームマップかどうかのチェック
 * @param  zoneid ゾーン指定ID
 * @return BOOL ユニオンルームだったらTRUE
 */
//------------------------------------------------------------------
BOOL ZONEDATA_IsUnionRoom(u16 zone_id)
{
  return (zone_id == ZONE_ID_UNION);
}
//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL ZONEDATA_IsColosseum(u16 zone_id)
{
  if ( (zone_id == ZONE_ID_CLOSSEUM) || (zone_id == ZONE_ID_CLOSSEUM02) )
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}
//==================================================================
/**
 * パレスマップかどうかのチェック
 * @param   zone_id		ゾーン指定ID
 * @retval  BOOL		ビンゴマップだったらTRUE
 */
//==================================================================
BOOL ZONEDATA_IsPalace(u16 zone_id)
{
  return (zone_id == ZONE_ID_PALACE01);
}
//==================================================================
/**
 * 裏フィールドかどうかのチェック
 * @param   zone_id		ゾーン指定ID
 * @retval  BOOL		ビンゴマップだったらTRUE
 */
//==================================================================
BOOL ZONEDATA_IsPalaceField(u16 zone_id)
{
  if((zone_id >= ZONE_ID_PLC04 && zone_id <= ZONE_ID_PLC10) //裏フィールド
      || (zone_id == ZONE_ID_PALACE02)){  //ビンゴマップ
    return TRUE;
  }
  return FALSE;
}
//==================================================================
/**
 * ビンゴマップかどうかのチェック
 * @param   zone_id		ゾーン指定ID
 * @retval  BOOL		ビンゴマップだったらTRUE
 */
//==================================================================
BOOL ZONEDATA_IsBingo(u16 zone_id)
{
  return (zone_id == ZONE_ID_PALACE02);
}

//------------------------------------------------------------------
/**
 * @brief  ミュージカル待合室かどうかのチェック
 * @param  zoneid ゾーン指定ID
 * @return BOOL ミュージカル待合室だったらTRUE
 */
//------------------------------------------------------------------
BOOL ZONEDATA_IsMusicalWaitingRoom(u16 zone_id)
{
  return (zone_id == ZONE_ID_C04R0202);
}
//------------------------------------------------------------------
/**
 * @brief  地名メッセージのIDを取得
 * @param  zoneid ゾーン指定ID
 * @return 地名メッセージID
 */
//------------------------------------------------------------------
u16 ZONEDATA_GetPlaceNameID(u16 zone_id)
{
	ZONEDATA zdbuf;
	getZoneData( &zdbuf, zone_id );
	return zdbuf.placename_id;
}

//------------------------------------------------------------------
/**
 * @brief  地名表示制御用フラグ取得
 * @param  zoneid ゾーン指定ID
 * @return  BOOL
 */
//------------------------------------------------------------------
BOOL ZONEDATA_GetPlaceNameFlag(u16 zone_id)
{
	ZONEDATA zdbuf;
	getZoneData( &zdbuf, zone_id );
	return zdbuf.placename_flag;
}

//------------------------------------------------------------------
/**
 * @brief 指定ゾーンのマップタイプ取得
 */
//------------------------------------------------------------------
static u16 getMapType(u16 zone_id)
{
  ZONEDATA zdbuf;
  getZoneData( &zdbuf, zone_id );
  return zdbuf.maptype;
}

//------------------------------------------------------------------
/**
 * @brief  指定ゾーンがダンジョンかどうかを判定
 * @param  zone_id 判定するゾーンのID
 * @return ダンジョンならTRUE
 */
//------------------------------------------------------------------
BOOL ZONEDATA_IsDungeon(u16 zone_id)
{
  u16 maptype = getMapType( zone_id );
  if( maptype == MAPTYPE_DUNGEON )
  {
    return TRUE;
  } 
  return FALSE;
}

//------------------------------------------------------------------
/** 
 * @brief 指定ゾーンがフィールドマップのマトリックス指定かどうかを判定
 * @param  zone_id 判定するゾーンのID
 * @return  BOOL TRUEのとき、フィールドマップのマトリックス指定
 */
//------------------------------------------------------------------
BOOL ZONEDATA_IsFieldMatrixID(u16 zone_id)
{
	ZONEDATA zdbuf;
	getZoneData(&zdbuf, zone_id);
	return (zdbuf.matrix_id == NARC_map_matrix_wb_mat_bin);
}

//------------------------------------------------------------------
/** 
 * @brief 指定ゾーンのエンカウントデータIDを取得
 * @param  zone_id 判定するゾーンのID
 * @return  BOOL TRUEのとき、フィールドマップのマトリックス指定
 */
//------------------------------------------------------------------
u16 ZONEDATA_GetEncountDataID(u16 zone_id)
{
	ZONEDATA zdbuf;
	getZoneData(&zdbuf, zone_id);
	return (zdbuf.enc_data_id);
}

//------------------------------------------------------------------
/** 
 * @brief 指定ゾーンのイベントデータIDの取得
 * @param  zone_id 判定するゾーンのID
 * @return  イベントデータのアーカイブID
 */
//------------------------------------------------------------------
u16 ZONEDATA_GetEventDataArcID(u16 zone_id)
{
	ZONEDATA zdbuf;
	getZoneData(&zdbuf, zone_id);
	return (zdbuf.event_data_id);
}

//------------------------------------------------------------------
/** 
 * @brief 指定ゾーンの戦闘背景IDの取得
 * @param  zone_id 判定するゾーンのID
 * @return  イベントデータのアーカイブID
 */
//------------------------------------------------------------------
u8 ZONEDATA_GetBattleBGType(u16 zone_id)
{
	ZONEDATA zdbuf;
	getZoneData(&zdbuf, zone_id);
	return (zdbuf.battle_bg_type);
}


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	ゾーン名データの取得
 * @param	buffer		名前を取得するためのバッファ(ZONEDATA_NAME_LENGTHの長さが必要）
 * @param	zoneid		取得するゾーンの名前
 */
//------------------------------------------------------------------
void ZONEDATA_DEBUG_GetZoneName(char * buffer, u16 zone_id)
{
	CHECK_RANGE(zone_id);	//範囲外チェック
	GFL_ARC_LoadDataOfs(buffer,
			ARCID_ZONEDATA, NARC_zonedata_zonename_bin,
			ZONEDATA_NAME_LENGTH * zone_id, ZONEDATA_NAME_LENGTH);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const char * ZONEDATA_GetAllZoneName(HEAPID heapID)
{
	char * namedata;
	namedata = GFL_ARC_LoadDataAlloc(
			ARCID_ZONEDATA, NARC_zonedata_zonename_bin, heapID);
	return namedata;
}




