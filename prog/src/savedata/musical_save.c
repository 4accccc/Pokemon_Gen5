//============================================================================================
/**
 * @file	musical_save.c
 * @brief	ミュージカル用セーブデータ
 * @author	Nobuhiko Ariizumi
 * @date	2009/04/08
 */
//============================================================================================
#include <gflib.h>

#include "savedata/musical_save.h"
#include "savedata/save_tbl.h"

#include "musical_save_local.h"

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

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
int MUSICAL_SAVE_GetWorkSize(void)
{
	return sizeof( MUSICAL_SAVE );
}

void MUSICAL_SAVE_InitWork(MUSICAL_SAVE *musSave)
{
	MUSICAL_SAVE_ResetBefEquip(musSave);
	GFL_STD_MemClear( &musSave->musicalShotData , sizeof( MUSICAL_SHOT_DATA ));
}

//----------------------------------------------------------
//	データ操作のための関数
//----------------------------------------------------------
MUSICAL_SAVE* MUSICAL_SAVE_GetMusicalSave( SAVE_CONTROL_WORK *sv )
{
	return (MUSICAL_SAVE*)SaveControl_DataPtrGet( sv , GMDATA_ID_MUSICAL );
}

void MUSICAL_SAVE_ResetBefEquip( MUSICAL_SAVE *musSave )
{
	u8 i;
	for( i=0;i<MUS_POKE_EQUIP_MAX;i++ )
	{
		musSave->befEquip.equipData[i].pos = MUS_POKE_EQU_INVALID;
	}
}

MUSICAL_EQUIP_SAVE* MUSICAL_SAVE_GetBefEquipData( MUSICAL_SAVE *musSave )
{
	return &musSave->befEquip;
}

//----------------------------------------------------------
//	データ操作のための関数
//----------------------------------------------------------
BOOL MUSICAL_SAVE_IsValidMusicalShotData( MUSICAL_SAVE *musSave )
{
  if( musSave->musicalShotData.month == 0 )
  {
    return FALSE;
  }
  return TRUE;
}

MUSICAL_SHOT_DATA* MUSICAL_SAVE_GetMusicalShotData( MUSICAL_SAVE *musSave )
{
  return &musSave->musicalShotData;
}
