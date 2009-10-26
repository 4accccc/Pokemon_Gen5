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
//	前回装備のための関数
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
//	ミュージカルショットのための関数
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

//----------------------------------------------------------
//	ミュージカルアイテム関係
//----------------------------------------------------------
const BOOL MUSICAL_SAVE_ChackHaveItem( MUSICAL_SAVE *musSave , const u8 itemNo )
{
  const u8 arrIdx = itemNo / 8;
  const u8 bitIdx = itemNo % 8;
  return ( musSave->itemBit[arrIdx] & (1<<bitIdx) );
}

const BOOL MUSICAL_SAVE_ChackNewItem( MUSICAL_SAVE *musSave , const u8 itemNo )
{
  const u8 arrIdx = itemNo / 8;
  const u8 bitIdx = itemNo % 8;
  return ( musSave->itemNewBit[arrIdx] & (1<<bitIdx) );
}

void MUSICAL_SAVE_AddItem( MUSICAL_SAVE *musSave , const u8 itemNo )
{
  const u8 arrIdx = itemNo / 8;
  const u8 bitIdx = itemNo % 8;
  
  musSave->itemBit[arrIdx] |= (1<<bitIdx);
  musSave->itemNewBit[arrIdx] |= (1<<bitIdx);
}

void MUSICAL_SAVE_ResetNewItem( MUSICAL_SAVE *musSave , const u8 itemNo )
{
  u8 i;
  for( i=0;i<MUS_SAVE_ITEM_BIT_MAX;i++ )
  {
    musSave->itemNewBit[i] = 0;
  }
}

//----------------------------------------------------------
//	ファン関係
//----------------------------------------------------------
MUSICAL_FAN_STATE* MUSICAL_SAVE_GetFanState( MUSICAL_SAVE *musSave , const u8 idx )
{
  return &musSave->fanState[idx];
}

//----------------------------------------------------------
//	その他数値
//----------------------------------------------------------
const u16 MUSICAL_SAVE_GetEntryNum( const MUSICAL_SAVE *musSave )
{
  return musSave->entryNum;
}

void MUSICAL_SAVE_AddEntryNum( MUSICAL_SAVE *musSave )
{
  if( musSave->entryNum < MUS_SAVE_ENTRY_NUM_MAX )
  {
    musSave->entryNum++;
  }
}

const u16 MUSICAL_SAVE_GetTopNum( const MUSICAL_SAVE *musSave )
{
  return musSave->topNum;
}

void MUSICAL_SAVE_AddTopNum( MUSICAL_SAVE *musSave )
{
  if( musSave->topNum < MUS_SAVE_ENTRY_NUM_MAX )
  {
    musSave->topNum++;
  }
}

const u8 MUSICAL_SAVE_GetBefCondition( MUSICAL_SAVE *musSave , const MUSICAL_CONDITION_TYPE conType )
{
  return musSave->befCondition[conType];
}

void MUSICAL_SAVE_SetBefCondition( MUSICAL_SAVE *musSave , const MUSICAL_CONDITION_TYPE conType , const u8 value )
{
  musSave->befCondition[conType] = value;
}

const u8 MUSICAL_SAVE_GetBefPoint( const MUSICAL_SAVE *musSave )
{
  return musSave->befPoint;
}

void MUSICAL_SAVE_SetBefPoint( MUSICAL_SAVE *musSave , const u8 point )
{
  musSave->befPoint = point;
}

