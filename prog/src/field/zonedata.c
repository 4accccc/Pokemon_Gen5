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

#include "field/zonedata.h"
#include "zonetableformat.h"
#include "gamesystem/pm_season.h"

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
	pos->x = zdbuf.sx * FX32_ONE * 16;
	pos->y = zdbuf.sy * FX32_ONE * 16;
	pos->z = zdbuf.sz * FX32_ONE * 16;
	TAMADA_Printf("%s x,y,z=%d,%d,%d\n",__FILE__,pos->x, pos->y, pos->z);
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
BOOL ZONEDATA_DEBUG_IsRailMap(u16 zone_id)
{
  return ( (zone_id == ZONE_ID_C03) || (zone_id == ZONE_ID_H01) );
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
 * @brief  地名メッセージ用のウィンドウIDを取得
 * @param  zoneid ゾーン指定ID
 * @return 地名メッセージ用ウィンドウのID
 */
//------------------------------------------------------------------
u16 ZONEDATA_GetPlaceNameWinID(u16 zone_id)
{
	ZONEDATA zdbuf;
	getZoneData( &zdbuf, zone_id );
	return zdbuf.placename_win;
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	ゾーン名データの取得
 * @param	heapID		作業用ヒープの指定
 * @param	buffer		名前を取得するためのバッファ(ZONEDATA_NAME_LENGTHの長さが必要）
 * @param	zoneid		取得するゾーンの名前
 */
//------------------------------------------------------------------
void ZONEDATA_GetZoneName(HEAPID heapID, char * buffer, u16 zone_id)
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




