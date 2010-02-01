//============================================================================================
/**
 * @file  zonedata.c
 * @brief ゾーン別データ
 * @author  tamada GAME FREAK inc.
 * @date  08.11.12
 */
//============================================================================================

#include <gflib.h>

#include "arc/arc_def.h"
#include "arc/fieldmap/zonedata.naix"
#include "arc/fieldmap/zone_id.h"
#include "arc/fieldmap/map_matrix.naix"
#include "arc/fieldmap/grid_camera_scene.naix"

#include "field/field_const.h"

#include "field/zonedata.h"
#include "zonetableformat.h"
#include "gamesystem/pm_season.h"

#include "arc/fieldmap/field_rail_setup.naix"  //NARC_field_rail_data_～
#include "../../resource/fldmapdata/zonetable/header/maptype.h"

#include "fieldmap/field_fog_table.naix"
#include "fieldmap/field_zone_light.naix"
#include "fieldmap/fieldskill_mapeff.naix"

#include "field/field_zonefog.h"
#include "field/fieldskill_mapeff.h"


#include "sound/wb_sound_data.sadl" //SEQ_BGM_C_08_～

//-------------------------------------
// 内部ワーク
//-------------------------------------
typedef struct
{
  // アーカイブハンドル
  ARCHANDLE* handle;

  // ゾーンデータ
  ZONEDATA zoneData;    // 読み込んだゾーンデータ
  u16      zoneDataID;  // どのゾーンのデータを読み込んだか

  // ゾーンフォグ　ライト　リスト
  ZONE_FOG_DATA* zonefoglist;
  ZONE_LIGHT_DATA* zonelightlist;
  u16 zonefoglist_max;
  u8 zonelightlist_max;

  // フィールド技 効果リスト
  u8 fieldskill_mapeff_list_max;
  FIELDSKILL_MAPEFF_DATA* fieldskill_mapeff_list;

  // ゾーンID変更ID
  u8 zone_change_flag[ ZONEDATA_CHANGE_MAX ];

} ZONE_DATA_HANDLE;

static ZONE_DATA_HANDLE* data_handle = NULL;


//------------------------------------------------------------------
/**
 * @brief ゾーンID変更テーブル
 */
//------------------------------------------------------------------
#define ZONEDATA_CHANGE_ID_BEFORE (0)
#define ZONEDATA_CHANGE_ID_AFTER (1)
#define ZONEDATA_CHANGE_ID_TBL_NUM (2)
static u16 ZONEDATA_CHANGE_ID_TBL[ ZONEDATA_CHANGE_MAX ][ ZONEDATA_CHANGE_ID_TBL_NUM ] =
{
  { ZONE_ID_BC10, ZONE_ID_WC10 },   // ZONEDATA_CHANGE_BC_WF_ID BCをWFに変更する。
  { ZONE_ID_PLC10, ZONE_ID_PLCW10 },   // ZONEDATA_CHANGE_BC_WF_ID BCをWFに変更する。
};



//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief アーカイブハンドル・オープン
 */
//------------------------------------------------------------------
void ZONEDATA_Open( HEAPID heap_id )
{
  if( data_handle == NULL )
  {
    data_handle = GFL_HEAP_AllocClearMemory( heap_id, sizeof(ZONE_DATA_HANDLE) );

    // アーカイブハンドル
    data_handle->handle = GFL_ARC_OpenDataHandle( ARCID_ZONEDATA, heap_id );

    // フォグ　ライト
    {
      u32 size;

      data_handle->zonefoglist = GFL_ARC_UTIL_LoadEx( ARCID_ZONEFOG_TABLE, NARC_field_fog_table_zonefog_table_bin, FALSE, heap_id, &size );
      data_handle->zonefoglist_max = size / sizeof(ZONE_FOG_DATA);

      data_handle->zonelightlist = GFL_ARC_UTIL_LoadEx( ARCID_ZONELIGHT_TABLE, NARC_field_zone_light_light_list_bin, FALSE, heap_id, &size );
      data_handle->zonelightlist_max = size / sizeof(ZONE_LIGHT_DATA);

    }

    // フィールド技　効果
    {
      u32 size;

      data_handle->fieldskill_mapeff_list = GFL_ARC_UTIL_LoadEx( ARCID_FLDSKILL_MAPEFF, NARC_fieldskill_mapeff_fieldskill_mapeffect_bin, FALSE, heap_id, &size );
      data_handle->fieldskill_mapeff_list_max = size / sizeof(FIELDSKILL_MAPEFF_DATA);
    }

    // ゾーンデータ
    data_handle->zoneDataID = ZONE_ID_MAX;
  }
}

//------------------------------------------------------------------
/**
 * @brief アーカイブハンドル・クローズ
 */
//------------------------------------------------------------------
void ZONEDATA_Close()
{
  if( data_handle != NULL )
  {
    // フィールド技　効果リスト
    {
      GFL_HEAP_FreeMemory( data_handle->fieldskill_mapeff_list );
    }

    // フォグ　ライト
    {
      GFL_HEAP_FreeMemory( data_handle->zonelightlist );
      GFL_HEAP_FreeMemory( data_handle->zonefoglist );
    }

    // アーカイブハンドル
    GFL_ARC_CloseDataHandle( data_handle->handle );

    GFL_HEAP_FreeMemory( data_handle );
    data_handle = NULL;
  }
}


//----------------------------------------------------------------------------
/**
 *  @brief  ZONEIDの変更設定
 *
 *  @param  id      ゾーンID変更ID
 *  @param  flag    TRUE:変更ON　　FALSE：変更OFF
 */
//-----------------------------------------------------------------------------
void ZONEDATA_SetChangeZoneID( ZONEDATA_CHANGE_ID id, BOOL flag )
{
  GF_ASSERT( data_handle );
  if( data_handle != NULL )
  {
    GF_ASSERT( id < ZONEDATA_CHANGE_MAX );
    data_handle->zone_change_flag[ id ] = flag;
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  ZONEIDの変更設定を確認
 *
 *  @param  id    ゾーンID変更ID
 *
 *  @retval TRUE      変更ON
 *  @retval FALSE     変更OFF
 */
//-----------------------------------------------------------------------------
BOOL ZONEDATA_GetChangeZoneID( ZONEDATA_CHANGE_ID id )
{
  GF_ASSERT( data_handle );
  GF_ASSERT( id < ZONEDATA_CHANGE_MAX );
  return data_handle->zone_change_flag[ id ];
}


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief 範囲チェック
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
#define CHECK_RANGE(value)  {value = check_range(value);}


//------------------------------------------------------------------
/**
 * @brief ZONEIDの変更処理
 */
//------------------------------------------------------------------
static u16 ControlZoneID( u16 zone_id )
{
  int i;

  for( i=0; i<ZONEDATA_CHANGE_MAX; i++ )
  {
    if( data_handle->zone_change_flag[i] )
    {
      if( ZONEDATA_CHANGE_ID_TBL[ i ][ ZONEDATA_CHANGE_ID_BEFORE ] == zone_id )
      {
        return ZONEDATA_CHANGE_ID_TBL[ i ][ ZONEDATA_CHANGE_ID_AFTER ];
      }
    }
  }
  return zone_id;
}

//------------------------------------------------------------------
/**
 * @brief ゾーンの最大数の取得
 * @return  u16 最大数
 */
//------------------------------------------------------------------
u16 ZONEDATA_GetZoneIDMax(void)
{
  return ZONE_ID_MAX;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static ZONEDATA * loadZoneData( u16 zoneID )
{
  // ハンドルがオープンされていない
  if( data_handle == NULL )
  {
    GF_ASSERT( 0 && "アーカイブハンドルがオープンさせていません。" );
    return NULL;
  }

  // 読み込み済み
  if( zoneID == data_handle->zoneDataID ){ return &( data_handle->zoneData ); }  

  GFL_ARC_LoadDataOfsByHandle( data_handle->handle,
      NARC_result_zonedata_zonetable_bin,
      sizeof(ZONEDATA) * zoneID, sizeof(ZONEDATA), &( data_handle->zoneData ) );

  data_handle->zoneDataID = zoneID;

  return &( data_handle->zoneData );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static ZONEDATA * getZoneData(ZONEDATA * zdbuf, u16 zone_id)
{
  if( data_handle == NULL )
  {
    OS_Printf( "アーカイブハンドルがオープンさせていません。\n" );
    return zdbuf;
  }

  // ZONEIDのすり替え
  zone_id = ControlZoneID(zone_id);

  CHECK_RANGE(zone_id); //範囲外チェック

  if( data_handle != NULL )
  {
    GFL_ARC_LoadDataOfsByHandle(data_handle->handle,
        NARC_result_zonedata_zonetable_bin,
        sizeof(ZONEDATA) * zone_id, sizeof(ZONEDATA), zdbuf);
  }
  else
  {
    GFL_ARC_LoadDataOfs(zdbuf,
        ARCID_ZONEDATA, NARC_result_zonedata_zonetable_bin,
        sizeof(ZONEDATA) * zone_id, sizeof(ZONEDATA));
  }
  return zdbuf;
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief エリア指定の取得
 * @param zone_id ゾーン指定ID
 * @return  u16 エリア指定値
 */
//------------------------------------------------------------------
u16 ZONEDATA_GetAreaID(u16 zone_id)
{
  ZONEDATA* zoneData;
  zoneData = loadZoneData( zone_id );
  return zoneData->area_id;
}
//------------------------------------------------------------------
/**
 * @brief マトリックス指定の取得
 * @param zone_id ゾーン指定ID
 * @return  u16 マトリックス指定値
 */
//------------------------------------------------------------------
u16 ZONEDATA_GetMatrixID(u16 zone_id)
{
  ZONEDATA* zoneData;
  zoneData = loadZoneData( zone_id );
  return zoneData->matrix_id;
}
//------------------------------------------------------------------
/**
 * @brief マップ構造指定の取得
 * @param zone_id ゾーン指定ID
 * @return  u16 マップ構造指定値
 */
//------------------------------------------------------------------
u16 ZONEDATA_GetMapRscID(u16 zone_id)
{
  ZONEDATA* zoneData;
  zoneData = loadZoneData( zone_id );
  return zoneData->maprsc_id;
}

//------------------------------------------------------------------
/**
 * @brief スクリプト指定IDの取得
 * @param zone_id ゾーン指定ID
 * @return  u16 スクリプトアーカイブでの指定ID
 */
//------------------------------------------------------------------
u16 ZONEDATA_GetScriptArcID(u16 zone_id)
{
  ZONEDATA* zoneData;
  zoneData = loadZoneData( zone_id );
  return zoneData->script_id;
}

//------------------------------------------------------------------
/**
 * @brief 特殊スクリプト指定IDの取得
 * @param zone_id ゾーン指定ID
 * @return  u16 スクリプトアーカイブでの指定ID
 */
//------------------------------------------------------------------
u16 ZONEDATA_GetSpScriptArcID(u16 zone_id)
{
  ZONEDATA* zoneData;
  zoneData = loadZoneData( zone_id );
  return zoneData->sp_script_id;
}

//------------------------------------------------------------------
/**
 * @brief フィールドメッセージ指定IDの取得
 * @param zone_id ゾーン指定ID
 * @return  u16 メッセージアーカイブでの指定ID
 */
//------------------------------------------------------------------
u16 ZONEDATA_GetMessageArcID(u16 zone_id)
{
  ZONEDATA* zoneData;
  zoneData = loadZoneData( zone_id );
  return zoneData->msg_id;
}

//------------------------------------------------------------------
/**
 * @brief マップ開始位置の取得
 * @param zone_id ゾーン指定ID
 * @param pos   開始位置を受け取るためのfx32型ポインタ
 */
//------------------------------------------------------------------
void ZONEDATA_GetStartPos(u16 zone_id, VecFx32 * pos)
{
  ZONEDATA* zoneData;
  zoneData = loadZoneData( zone_id );
  pos->x = zoneData->sx * FX32_ONE * FIELD_CONST_GRID_SIZE;
  pos->y = zoneData->sy * FX32_ONE * FIELD_CONST_GRID_SIZE;
  pos->z = zoneData->sz * FX32_ONE * FIELD_CONST_GRID_SIZE;
  TAMADA_Printf("%s x,y,z=%d,%d,%d\n",__FILE__,pos->x, pos->y, pos->z);
}

//----------------------------------------------------------------------------
/**
 * @brief マップ開始レール位置の取得
 *
 *  @param  zone_id
 *  @param  pos
 */
//-----------------------------------------------------------------------------
void ZONEDATA_GetStartRailPos(u16 zone_id, VecFx32 * pos)
{
  ZONEDATA* zoneData;
  zoneData = loadZoneData( zone_id );
  pos->x = zoneData->sx;
  pos->y = zoneData->sy;
  pos->z = zoneData->sz;
}

//------------------------------------------------------------------
/**
 * @brief カメラ指定IDの取得
 * @param zone_id ゾーン指定ID
 * @return  u8    カメラ指定IDの値
 */
//------------------------------------------------------------------
u8 ZONEDATA_GetCameraID(u16 zone_id)
{
  ZONEDATA* zoneData;
  zoneData = loadZoneData( zone_id );
  return zoneData->camera_id;
}

//----------------------------------------------------------------------------
/**
 *  @brief  カメラエリア指定IDの取得
 *  @param  zone_id ゾーン指定ID
 *  @return カメラエリア指定ID
 */
//-----------------------------------------------------------------------------
u16 ZONEDATA_GetCameraAreaID(u16 zone_id)
{
  ZONEDATA* zoneData;
  zoneData = loadZoneData( zone_id );
  return zoneData->camera_area;
}

//------------------------------------------------------------------
/**
 * @brief BGM指定IDの取得
 * @param zone_id   ゾーン指定ID
 * @param season_id 季節指定ID
 * @return  u8    BGM指定IDの値
 */
//------------------------------------------------------------------
u16 ZONEDATA_GetBGMID(u16 zone_id, u8 season_id)
{
  u16 bgm_id = 0;
  ZONEDATA* zoneData;
  zoneData = loadZoneData( zone_id );

  switch (season_id) {
  case PMSEASON_SPRING:
    bgm_id = zoneData->bgm_spring_id;
    break;
  case PMSEASON_SUMMER:
    bgm_id = zoneData->bgm_summer_id;
    break;
  case PMSEASON_AUTUMN:
    bgm_id = zoneData->bgm_autumn_id;
    break;
  case PMSEASON_WINTER:
    bgm_id = zoneData->bgm_winter_id;
    break;
  default:
    GF_ASSERT(season_id < PMSEASON_TOTAL);
  }
  if (bgm_id == SEQ_BGM_C_08_B && GetVersion() == VERSION_WHITE )
  {
    bgm_id = SEQ_BGM_C_08_W;
  }
  return bgm_id;
}

//------------------------------------------------------------------
/**
 * @brief 天候指定IDの取得
 * @param zone_id ゾーン指定ID
 * @return  u16 天候指定ID（include/field/weather_no.hを参照）
 */
//------------------------------------------------------------------
u16 ZONEDATA_GetWeatherID(u16 zone_id)
{
  ZONEDATA* zoneData;
  zoneData = loadZoneData( zone_id );
  return zoneData->weather_id;
}

//------------------------------------------------------------------
/// 自転車に乗れるマップかどうか
//------------------------------------------------------------------
BOOL ZONEDATA_BicycleEnable( u16 zone_id )
{
  ZONEDATA* zoneData;
  zoneData = loadZoneData( zone_id );
  return zoneData->bicycle_flag;
}

//------------------------------------------------------------------
/// 自転車に乗った場合, BGMを変更するかどうか
//------------------------------------------------------------------
BOOL ZONEDATA_BicycleBGMEnable( u16 zone_id )
{
  // @todo ゾーンテーブルを参照する
  return TRUE;
}

//------------------------------------------------------------------
/// ダッシュできるマップかどうか
//------------------------------------------------------------------
BOOL ZONEDATA_DashEnable( u16 zone_id )
{
  ZONEDATA* zoneData;
  zoneData = loadZoneData( zone_id );
  return zoneData->dash_flag;
}
//------------------------------------------------------------------
/// 「そらをとぶ」等が使えるマップかどうか
//------------------------------------------------------------------
BOOL ZONEDATA_FlyEnable( u16 zone_id )
{
  ZONEDATA* zoneData;
  zoneData = loadZoneData( zone_id );
  return zoneData->fly_flag;
}
//------------------------------------------------------------------
/// 「あなぬけ」による脱出を使えるマップかどうか
//------------------------------------------------------------------
BOOL ZONEDATA_EscapeEnable( u16 zone_id )
{
  ZONEDATA* zoneData;
  zoneData = loadZoneData( zone_id );
  return zoneData->escape_flag;
}

//------------------------------------------------------------------
/**
 * @brief 特殊なサンプルOBJを使用するかどうかの設定取得
 * @param zone_id ゾーン指定ID
 * @param BOOL  TRUEのとき、サンプルOBJを使用する
 */
//------------------------------------------------------------------
BOOL ZONEDATA_DEBUG_IsSampleObjUse(u16 zone_id)
{
  ZONEDATA* zoneData;
  zoneData = loadZoneData( zone_id );
  return zoneData->movemodel_id != 0;
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
  zone_id = ControlZoneID(zone_id);

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
  zone_id = ControlZoneID(zone_id);

  switch (zone_id)
  {
  case ZONE_ID_C03:
    return NARC_field_rail_setup_C03_bin;
  case ZONE_ID_H01:
    return NARC_field_rail_setup_H01_bin;
  case ZONE_ID_C03P02:
    return NARC_field_rail_setup_C03P02_bin;
  case ZONE_ID_D09:
    return NARC_field_rail_setup_D09_bin;
  case ZONE_ID_R07R0101:
    return NARC_field_rail_setup_R07R0101_bin;
  case ZONE_ID_R07R0102:
    return NARC_field_rail_setup_R07R0102_bin;
  case ZONE_ID_R07R0103:
    return NARC_field_rail_setup_R07R0103_bin;
  case ZONE_ID_R07R0104:
    return NARC_field_rail_setup_R07R0104_bin;
  case ZONE_ID_C07GYM0101:
    return NARC_field_rail_setup_C07GYM0101_bin;
  case ZONE_ID_D08R0501:
    return NARC_field_rail_setup_D08R0501_bin;
  case ZONE_ID_D08R0701:
    return NARC_field_rail_setup_D08R0701_bin;
  case ZONE_ID_C04R0101:
    return NARC_field_rail_setup_C04R0101_bin;
  case ZONE_ID_C08GYM0101:
    return NARC_field_rail_setup_C08GYM0101_bin;
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
  zone_id = ControlZoneID(zone_id);

  return (zone_id == ZONE_ID_UNION);
}
//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL ZONEDATA_IsColosseum(u16 zone_id)
{
  zone_id = ControlZoneID(zone_id);

  if ( (zone_id == ZONE_ID_CLOSSEUM) || (zone_id == ZONE_ID_CLOSSEUM02) )
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

//------------------------------------------------------------------
/**
 * @brief  遊覧船内マップかどうかのチェック
 * @param  zoneid ゾーン指定ID
 * @return BOOL 遊覧船内だったらTRUE
 */
//------------------------------------------------------------------
BOOL ZONEDATA_IsPlBoat(u16 zone_id)
{
  zone_id = ControlZoneID(zone_id);

  return (zone_id == ZONE_ID_C03R0801);
}

//==================================================================
/**
 * パレスマップかどうかのチェック
 * @param   zone_id   ゾーン指定ID
 * @retval  BOOL    ビンゴマップだったらTRUE
 */
//==================================================================
BOOL ZONEDATA_IsPalace(u16 zone_id)
{
  zone_id = ControlZoneID(zone_id);

  return (zone_id == ZONE_ID_PALACE01);
}
//==================================================================
/**
 * 裏フィールドかどうかのチェック
 * @param   zone_id   ゾーン指定ID
 * @retval  BOOL    ビンゴマップだったらTRUE
 */
//==================================================================
BOOL ZONEDATA_IsPalaceField(u16 zone_id)
{
  zone_id = ControlZoneID(zone_id);

  if((zone_id >= ZONE_ID_PLC04 && zone_id <= ZONE_ID_PLT04) //裏フィールド
      || (zone_id == ZONE_ID_PALACE02)){      //ビンゴマップ
    return TRUE;
  }
  return FALSE;
}
//==================================================================
/**
 * ビンゴマップかどうかのチェック
 * @param   zone_id   ゾーン指定ID
 * @retval  BOOL    ビンゴマップだったらTRUE
 */
//==================================================================
BOOL ZONEDATA_IsBingo(u16 zone_id)
{
  zone_id = ControlZoneID(zone_id);
  return (zone_id == ZONE_ID_PALACE02);
}

//==================================================================
/**
 * WFBCかどうか
 * @param   zone_id   ゾーン指定ID
 * @retval  BOOL    ビンゴマップだったらTRUE
 */
//==================================================================
BOOL ZONEDATA_IsWfbc(u16 zone_id)
{
  zone_id = ControlZoneID(zone_id);
  return ((zone_id == ZONE_ID_BCWFTEST) || (zone_id == ZONE_ID_WC10) || (zone_id == ZONE_ID_BC10) || (zone_id == ZONE_ID_PLC10) || (zone_id == ZONE_ID_PLCW10));
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
  zone_id = ControlZoneID(zone_id);
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
  ZONEDATA* zoneData;
  zoneData = loadZoneData( zone_id );
  return zoneData->placename_id;
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
  ZONEDATA* zoneData;
  zoneData = loadZoneData( zone_id );
  return zoneData->placename_flag;
}

//------------------------------------------------------------------
/**
 * @brief 指定ゾーンのマップタイプ取得
 */
//------------------------------------------------------------------
static u16 getMapType(u16 zone_id)
{
  ZONEDATA* zoneData;
  zoneData = loadZoneData( zone_id );
  return zoneData->maptype;
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
  ZONEDATA* zoneData;
  zoneData = loadZoneData( zone_id );
  return (zoneData->matrix_id == NARC_map_matrix_wb_mat_bin);
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
  ZONEDATA* zoneData;
  zoneData = loadZoneData( zone_id );
  return (zoneData->enc_data_id);
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
  ZONEDATA* zoneData;
  zoneData = loadZoneData( zone_id );
  return (zoneData->event_data_id);
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
  ZONEDATA* zoneData;
  zoneData = loadZoneData( zone_id );
  return (zoneData->battle_bg_type);
}


//----------------------------------------------------------------------------
/**
 *  @brief  ぞーんごとのFOGデータインデックス取得 field_zonefog.h参照
 *
 *  @param  zone_id ゾーンID
 *
 *  @retval データインデックス
 *  @retval FIELD_ZONEFOGLIGHT_DATA_NONE  なし
 */
//-----------------------------------------------------------------------------
u32 ZONEDATA_GetFog(u16 zone_id)
{
  int i;
  u32 ret = FIELD_ZONEFOGLIGHT_DATA_NONE;

  GF_ASSERT( data_handle );

  zone_id = ControlZoneID(zone_id);

  for( i=0; i<data_handle->zonefoglist_max; i++ )
  {
    if( data_handle->zonefoglist[i].zone_id == zone_id )
    {
      ret = data_handle->zonefoglist[i].data_id;
      break;
    }
  }

  return ret;
}

//----------------------------------------------------------------------------
/**
 *  @brief  ぞーんごとのLIGHTデータインデックス取得 field_zonefog.h参照
 *
 *  @param  zone_id ゾーンID
 *
 *  @retval データインデックス
 *  @retval FIELD_ZONEFOGLIGHT_DATA_NONE  なし
 */
//-----------------------------------------------------------------------------
u32 ZONEDATA_GetLight(u16 zone_id)
{
  int i;
  u32 ret = FIELD_ZONEFOGLIGHT_DATA_NONE;

  GF_ASSERT( data_handle );

  zone_id = ControlZoneID(zone_id);

  for( i=0; i<data_handle->zonelightlist_max; i++ )
  {
    if( data_handle->zonelightlist[i].zone_id == zone_id )
    {
      ret = data_handle->zonelightlist[i].data_id;
      break;
    }
  }

  return ret;
}


//----------------------------------------------------------------------------
/**
 *  @brief  フィールド技　マップエフェクトマスク取得
 *
 *  @param  zone_id ゾーンID
 *
 *  @return FIELDSKILL_MAPEFF_MSK
 */
//-----------------------------------------------------------------------------
u32 ZONEDATA_GetFieldSkillMapEffMsk(u16 zone_id)
{
  int i;
  u32 ret = 0;

  GF_ASSERT( data_handle );

  zone_id = ControlZoneID(zone_id);

  for( i=0; i<data_handle->fieldskill_mapeff_list_max; i++ )
  {
    if( data_handle->fieldskill_mapeff_list[i].zone_id == zone_id )
    {
      ret = data_handle->fieldskill_mapeff_list[i].msk;
      break;
    }
  }

  return ret;
}

//----------------------------------------------------------------------------
/**
 *  @brief  シーンエリアIDの取得
 *
 *  @param  zone_id ゾーンID
 *
 *  @return シーンエリアデータID
 */
//-----------------------------------------------------------------------------
u32 ZONEDATA_GetSceneAreaID(u16 zone_id)
{
  zone_id = ControlZoneID(zone_id);

  switch( zone_id )
  {
  case ZONE_ID_C04:
    return NARC_grid_camera_scene_camera_scene_C04_dat;
  case ZONE_ID_T05:
    return NARC_grid_camera_scene_camera_scene_T05_dat;
  }
  return ZONEDATA_NO_SCENEAREA_ID;
}


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief ゾーン名データの取得
 * @param buffer    名前を取得するためのバッファ(ZONEDATA_NAME_LENGTHの長さが必要）
 * @param zoneid    取得するゾーンの名前
 */
//------------------------------------------------------------------
void ZONEDATA_DEBUG_GetZoneName(char * buffer, u16 zone_id)
{
  zone_id = ControlZoneID(zone_id);

  if( data_handle != NULL )
  {
    GFL_ARC_LoadDataOfsByHandle(data_handle->handle,
        NARC_result_zonedata_zonename_bin,
        ZONEDATA_NAME_LENGTH * zone_id, ZONEDATA_NAME_LENGTH, buffer);
  }
  else
  {
    GFL_ARC_LoadDataOfs(buffer,
        ARCID_ZONEDATA, NARC_result_zonedata_zonename_bin,
        ZONEDATA_NAME_LENGTH * zone_id, ZONEDATA_NAME_LENGTH);
  }
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const char * ZONEDATA_GetAllZoneName(HEAPID heapID)
{
  char * namedata;
  if( data_handle != NULL )
  {
    namedata = GFL_ARC_LoadDataAllocByHandle(
        data_handle->handle, NARC_result_zonedata_zonename_bin, heapID);
  }
  else
  {
    namedata = GFL_ARC_LoadDataAlloc(
        ARCID_ZONEDATA, NARC_result_zonedata_zonename_bin, heapID);
  }
  return namedata;
}




