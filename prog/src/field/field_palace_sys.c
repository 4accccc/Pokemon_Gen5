//======================================================================
/**
 * @file	field_palace_sys.c
 * @brief	侵入時管理
 * @author	ariizumi
 * @data	10/04/29
 *
 * モジュール名：FIELD_PALACE
 */
//======================================================================
#include <gflib.h>

#include "field/fieldmap.h"
#include "field/zonedata.h"
#include "field/field_comm/intrude_work.h"
#include "fieldmap/zone_id.h"

#include "arc_def.h"
#include "palace_palette.naix"

#include "field_palace_sys.h"
#include "debug/debug_flg.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define
#define FIELD_PALACE_TABLE_SIZE (32)
#define FIELD_PALACE_TABLE_NUM  ((ZONE_ID_PLT04-ZONE_ID_PALACE02)+1)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
struct _FIELD_PALACE_SYS
{
  GAMESYS_WORK *gameSys;
  FIELDMAP_WORK *fieldWork;
  void *shadeTable;
};

typedef struct
{
  u8 area;
  u8 season;
  u16 pad;
  u8 table[32];
}FIELD_PALECE_SHADE_DATA;
//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void FIELD_PALACE_LoadShadeTable( FIELD_PALACE_SYS* work , u16 areaId , const HEAPID heapId );
static void FIELD_PALACE_LoadShadeTableCore( FIELD_PALACE_SYS* work , u32 datId , u16 zoneId , u8 season , const HEAPID heapId );
//--------------------------------------------------------------
//	ワーク作成
//--------------------------------------------------------------
FIELD_PALACE_SYS* FIELD_PALACE_Create( HEAPID heapId , GAMESYS_WORK *gsys , FIELDMAP_WORK *fieldWork , u16 zoneId )
{
  FIELD_PALACE_SYS* work;
  work = GFL_HEAP_AllocClearMemory( heapId , sizeof(FIELD_PALACE_SYS) );
  
  work->gameSys = gsys;
  work->fieldWork = fieldWork;
  work->shadeTable = NULL;
  
  FIELD_PALACE_LoadShadeTable( work , zoneId , heapId );
  return work;
}

//--------------------------------------------------------------
//	ワーク破棄
//--------------------------------------------------------------
void FIELD_PALACE_Delete( FIELD_PALACE_SYS *work )
{
  if( work->shadeTable != NULL )
  {
    GFL_HEAP_FreeMemory( work->shadeTable );
  }
  GFL_HEAP_FreeMemory( work );
}

//--------------------------------------------------------------
//	濃淡テーブル取得
//--------------------------------------------------------------
u8* FIELD_PALACE_GetShadeTable( FIELD_PALACE_SYS *work )
{
  return work->shadeTable;
}

//--------------------------------------------------------------
//	グレースケール用テーブル読み込み
//--------------------------------------------------------------
static void FIELD_PALACE_LoadShadeTable( FIELD_PALACE_SYS* work , u16 zoneId , const HEAPID heapId )
{
  GAME_COMM_SYS_PTR gameComm = GAMESYSTEM_GetGameCommSysPtr( work->gameSys );
#if PM_DEBUG
  if( DEBUG_FLG_GetFlg(DEBUG_FLG_PalaceTexture) == TRUE )
  {
    GAMEDATA *gameData = GAMESYSTEM_GetGameData( work->gameSys );
    u16 datId = 0xFFFF;
    if( GET_VERSION() == VERSION_BLACK )
    {
      if( DEBUG_FLG_GetFlg(DEBUG_FLG_PalaceArrive) == TRUE )
      {
        datId = NARC_palace_palette_shade_black_normal_bin;
      }
      else
      {
        datId = NARC_palace_palette_shade_black_fog_bin;
      }
    }
    else
    if( GET_VERSION() == VERSION_WHITE )
    {
      if( DEBUG_FLG_GetFlg(DEBUG_FLG_PalaceArrive) == TRUE )
      {
        datId = NARC_palace_palette_shade_white_normal_bin;
      }
      else
      {
        datId = NARC_palace_palette_shade_white_fog_bin;
      }
    }
    if( datId != 0xFFFF )
    {
      const u8 season = GAMEDATA_GetSeasonID( gameData );
      FIELD_PALACE_LoadShadeTableCore( work , datId , zoneId , season , heapId );
    }
  }
  else
#endif
  if( ZONEDATA_IsPalaceField( zoneId ) == TRUE )
  {
    if( gameComm != NULL )
    {
      GAMEDATA *gameData = GAMESYSTEM_GetGameData( work->gameSys );
      const BOOL isArrive = FIELD_PALACE_CheckArriveZone( gameData , zoneId );
      const GRAYSCALE_TYPE type = Intrude_CheckGrayScaleMap( gameComm , gameData );
      u16 datId = 0xFFFF;
      if( type == GRAYSCALE_TYPE_BLACK )
      {
        if( isArrive == TRUE )
        {
          datId = NARC_palace_palette_shade_black_normal_bin;
        }
        else
        {
          datId = NARC_palace_palette_shade_black_fog_bin;
        }
      }
      else
      if( type == GRAYSCALE_TYPE_WHITE )
      {
        if( isArrive == TRUE )
        {
          datId = NARC_palace_palette_shade_white_normal_bin;
        }
        else
        {
          datId = NARC_palace_palette_shade_white_fog_bin;
        }
      }
      if( datId != 0xFFFF )
      {
        const u8 season = Intrude_GetSeasonID( gameComm );
        FIELD_PALACE_LoadShadeTableCore( work , datId , zoneId , season , heapId );
      }
    }
    
  }
}

static void FIELD_PALACE_LoadShadeTableCore( FIELD_PALACE_SYS* work , u32 datId , u16 zoneId , u8 season , const HEAPID heapId )
{
  BOOL isMatch = FALSE;
  int i;
  u32 size;
  FIELD_PALECE_SHADE_DATA *buf;
  ARCHANDLE *arcHandle = GFL_ARC_OpenDataHandle( ARCID_PALACE_PALETTE , GFL_HEAP_LOWID(heapId) );

  const u16 areaId = ZONEDATA_GetAreaID( zoneId );
  const u16 checkAreaId = (areaId-2)/4 + 1;

  buf = GFL_ARCHDL_UTIL_LoadEx( arcHandle , datId , FALSE , GFL_HEAP_LOWID(heapId) , &size );
  
  // 0 はデフォルトデータ
  for( i=1;i<size/sizeof(FIELD_PALECE_SHADE_DATA);i++ )
  {
    if( buf[i].area == checkAreaId ||
        buf[i].area == 255 )
    {
      if( buf[i].season == season ||
          buf[i].season == 255 )
      {
        isMatch = TRUE;
        break;
      }
    }
  }
  if( isMatch == FALSE )
  {
    i = 0;
  }

  work->shadeTable = GFL_HEAP_AllocMemory(heapId , FIELD_PALACE_TABLE_SIZE);
  GFL_STD_MemCopy( buf[i].table  ,work->shadeTable,FIELD_PALACE_TABLE_SIZE );
  
  GFL_HEAP_FreeMemory( buf );
  GFL_ARC_CloseDataHandle( arcHandle );
  
#if defined(DEBUG_ONLY_FOR_ariizumi_nobuhiko)|defined(DEBUG_ONLY_FOR_ohmori)
  OS_TPrintf("-----palace palette load-----\n");
  OS_TPrintf("LoadDatId[%d] area[%d] season[%d]\n",datId,areaId,season);
  OS_TPrintf("LoadIdx[%d] area[%d] season[%d]\n",i,buf[i].area,buf[i].season);
  {
    u8 i;
    u8 *tempArr = (u8*)work->shadeTable;
    for( i=0;i<32;i++ )
    {
      OS_TPrintf("[%d]",tempArr[i]);
    }
    OS_TPrintf("\n");
  }
  OS_TPrintf("-----palace palette load-----\n");
#endif
  
}


//--------------------------------------------------------------
//	そのゾーンに到達しているか？
//  パレス対象外なら常にTRUE
//--------------------------------------------------------------
#include "field/palace_weather.cdat"
const BOOL FIELD_PALACE_CheckArriveZone( GAMEDATA *gmData , u16 zoneId )
{
  int i;
  EVENTWORK *evWork = GAMEDATA_GetEventWork( gmData );
  // パレスなら、パレス用
  if( ZONEDATA_IsPalaceField( zoneId ) == TRUE )
  {
    for( i=0; i<NELEMS(sc_PALACE_WEATHER_DATA); i++ )
    {
      if( sc_PALACE_WEATHER_DATA[i].zone_id == zoneId )
      {
        if( EVENTWORK_CheckEventFlag( evWork, sc_PALACE_WEATHER_DATA[i].sys_flag ) == TRUE )
        {
          return TRUE;
        }
        else
        {
          return FALSE;
        }
      }
    }
  }
  return TRUE;
}
