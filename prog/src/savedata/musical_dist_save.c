//============================================================================================
/**
 * @file	musical_dist_save.c
 * @brief	ミュージカル配信用セーブデータ
 * @author	Nobuhiko Ariizumi
 * @date	2009/04/08
 */
//============================================================================================
#include <gflib.h>

#include "savedata/save_tbl.h"
#include "savedata/musical_dist_save.h"
#include "savedata/musical_save.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

typedef struct
{
  u32 size;
  u32 pad[3];
}DIST_DATA_HEADER;

struct _MUSICAL_DIST_SAVE
{
  BOOL isEnableData;
  int  saveSeq;
  DIST_DATA_HEADER dataHeader;
  void *saveData;
  SAVE_CONTROL_WORK *sv;
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
int MUSICAL_DIST_SAVE_GetWorkSize(void)
{
  //ヘッダ分減らしておく
  //殿堂入りでさらに減るかも
	return (127*1024);
}

void MUSICAL_DIST_SAVE_InitWork(MUSICAL_DIST_SAVE *musDistSave)
{
  //処理なし
}

MUSICAL_DIST_SAVE* MUSICAL_DIST_SAVE_LoadData( SAVE_CONTROL_WORK *sv , HEAPID heapId )
{
  MUSICAL_DIST_SAVE *distSave = GFL_HEAP_AllocClearMemory( heapId , sizeof(distSave) );
  LOAD_RESULT ret;
  distSave->saveData = GFL_HEAP_AllocClearMemory( heapId , MUSICAL_DIST_SAVE_GetWorkSize() );
  ret = SaveControl_Extra_LoadWork( sv , EXGMDATA_ID_MUSICAL_DIST , heapId , distSave->saveData , MUSICAL_DIST_SAVE_GetWorkSize() );
  
  distSave->sv = sv;
  distSave->saveSeq = 0;
  if( ret == LOAD_RESULT_OK )
  {
    //データがある
    distSave->isEnableData = TRUE;
    distSave->saveData = SaveControl_Extra_DataPtrGet( sv , EXGMDATA_ID_MUSICAL_DIST , 0 );
  }
  else
  if( ret == LOAD_RESULT_NULL || 
      ret == LOAD_RESULT_NG )
  {
    //データが無い
    distSave->isEnableData = FALSE;
    distSave->saveData = SaveControl_Extra_DataPtrGet( sv , EXGMDATA_ID_MUSICAL_DIST , 0 );
  }
  else
  {
    //破壊
    distSave->isEnableData = FALSE;
    GFL_HEAP_FreeMemory( distSave->saveData );
    distSave->saveData = NULL;
  }
  return distSave;
}

void MUSICAL_DIST_SAVE_UnloadData( MUSICAL_DIST_SAVE *distSave )
{
  SaveControl_Extra_Unload( distSave->sv , EXGMDATA_ID_MUSICAL_DIST );
  GFL_HEAP_FreeMemory( distSave );
}

//InitとMainでセーブの確保・開放、データ有効フラグのセットなどを行います。
MUSICAL_DIST_SAVE* MUSICAL_DIST_SAVE_SaveMusicalArchive_Init( SAVE_CONTROL_WORK *sv , void *arcData , const u32 size , const HEAPID heapId )
{
  MUSICAL_DIST_SAVE *distSave = MUSICAL_DIST_SAVE_LoadData( sv , heapId );
  if( distSave->saveData != NULL )
  {
    distSave->dataHeader.size = size;
    GFL_STD_MemCopy( &distSave->dataHeader , distSave->saveData , sizeof( DIST_DATA_HEADER ) );
    GFL_STD_MemCopy( arcData , (void*)((u32)distSave->saveData + sizeof( DIST_DATA_HEADER ) ) , size );
  }
  return distSave;
}

//処理が終わるとMUSICAL_DIST_SAVEは無効になっています。
const BOOL MUSICAL_DIST_SAVE_SaveMusicalArchive_Main( MUSICAL_DIST_SAVE *distSave )
{
  switch( distSave->saveSeq )
  {
  case 0:
    if( distSave->saveData == NULL )
    {
      //データが壊れていてセーブできない。
      MUSICAL_DIST_SAVE_UnloadData( distSave );
      return TRUE;
    }
    else
    {
      SaveControl_Extra_SaveAsyncInit( distSave->sv , EXGMDATA_ID_MUSICAL_DIST );
      distSave->saveSeq++;
    }
    break;
  case 1:
    {
      const SAVE_RESULT ret = SaveControl_Extra_SaveAsyncMain( distSave->sv , EXGMDATA_ID_MUSICAL_DIST );
      if( ret == SAVE_RESULT_OK )
      {
        MUSICAL_SAVE* musSave = MUSICAL_SAVE_GetMusicalSave( distSave->sv );
        MUSICAL_SAVE_SetEnableDistributData( musSave , TRUE );
        MUSICAL_DIST_SAVE_UnloadData( distSave );
        return TRUE;
      }
    }
    break;
  }
  return FALSE;
}



#pragma mark [> proto
#include "arc_def.h"
void* MUSICAL_DIST_SAVE_GetProgramData( MUSICAL_DIST_SAVE *distSave , const HEAPID heapId , u32 *dataSize )
{
  //仮
  return GFL_ARC_UTIL_LoadEx( ARCID_MUSICAL_PROGRAM_01 , 0 , FALSE , heapId , dataSize);
}
void* MUSICAL_DIST_SAVE_GetMessageData( MUSICAL_DIST_SAVE *distSave , const HEAPID heapId , u32 *dataSize )
{
  //仮
  return GFL_ARC_UTIL_LoadEx( ARCID_MUSICAL_PROGRAM_01 , 1 , FALSE , heapId , dataSize);
}
void* MUSICAL_DIST_SAVE_GetScriptData( MUSICAL_DIST_SAVE *distSave , const HEAPID heapId , u32 *dataSize )
{
  //仮
  return GFL_ARC_UTIL_LoadEx( ARCID_MUSICAL_PROGRAM_01 , 2 , FALSE , heapId , dataSize);
}
void* MUSICAL_DIST_SAVE_GetStreamingData( MUSICAL_DIST_SAVE *distSave , const HEAPID heapId , u32 *dataSize )
{
  //仮
  return GFL_ARC_UTIL_LoadEx( ARCID_MUSICAL_PROGRAM_01 , 3 , FALSE , heapId , dataSize);
}
