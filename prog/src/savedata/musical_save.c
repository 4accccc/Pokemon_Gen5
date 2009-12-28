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
	u8 i;
	GFL_STD_MemClear( musSave , sizeof( MUSICAL_SAVE ));
	MUSICAL_SAVE_ResetBefEquip(musSave);
	for( i=0;i<MUS_SAVE_FAN_NUM;i++ )
	{
    musSave->fanState[i].type = MCT_MAX;
  }
  musSave->programNumber = 0;
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
	for( i=0;i<MUSICAL_ITEM_EQUIP_MAX;i++ )
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
  OS_TPrintf("AddMusicalItem[%d][%d:%d][%d]\n",itemNo,arrIdx,bitIdx,musSave->itemBit[arrIdx]);
}

void MUSICAL_SAVE_ResetNewItem( MUSICAL_SAVE *musSave , const u8 itemNo )
{
  u8 i;
  for( i=0;i<MUS_SAVE_ITEM_BIT_MAX;i++ )
  {
    musSave->itemNewBit[i] = 0;
  }
}

const int MUSICAL_SAVE_GetItemNum( MUSICAL_SAVE *musSave )
{
  int num = 0;
  int i;
  for( i=0;i<MUS_SAVE_ITEM_MAX;i++ )
  {
    if( MUSICAL_SAVE_ChackHaveItem( musSave , i ) == TRUE )
    {
      num++;
    }
  }
  return num;
}

const BOOL MUSICAL_SAVE_IsCompleteItem( MUSICAL_SAVE *musSave )
{
  const int num = MUSICAL_SAVE_GetItemNum(musSave);
  if( num == MUSICAL_ITEM_MAX_REAL )
  {
    return TRUE;
  }
  return FALSE;
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

const MUSICAL_CONDITION_TYPE MUSICAL_SAVE_GetMaxBefConditionType( MUSICAL_SAVE *musSave )
{
  u8 i;
  u8 maxPoint = 0;
  u8 maxPointIdx = 0;
  
  for( i=0;i<MCT_MAX;i++ )
  {
    if( musSave->befCondition[i] > maxPoint )
    {
      maxPoint = musSave->befCondition[i];
      maxPointIdx = i;
    }
  }
  if( maxPoint == 0 )
  {
    //無装備
    return MCT_MAX;
  }
  return maxPointIdx;
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

const u8 MUSICAL_SAVE_GetProgramNumber( const MUSICAL_SAVE *musSave )
{
  return musSave->programNumber;
}

void MUSICAL_SAVE_SetProgramNumber( MUSICAL_SAVE *musSave , const u8 num )
{
  musSave->programNumber = num;
}

