//======================================================================
/**
 * @file  musical_shot_sys.h
 * @brief ミュージカルの記念写真Proc
 * @author  ariizumi
 * @data  09/09/14
 */
//======================================================================
#pragma once

#include "musical/musical_define.h"
#include "buflen.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================

typedef struct
{
  u16 itemNo;
  s16 angle;
  u8  equipPos;
}
MUSICAL_SHOT_POKE_EQUIP;

typedef struct
{
  u16 monsno;
  u8  sex :2;
  u8  rare:1;
  u8  form:5;
  
  STRCODE trainerName[BUFLEN_PERSON_NAME];
  MUSICAL_SHOT_POKE_EQUIP equip[MUSICAL_ITEM_EQUIP_MAX];
  
}MUSICAL_SHOT_POKE;

typedef struct
{
  u8 bgNo;    //背景番号
  u32 spotBit:4; //スポットライト対象(bit)
  u32 year   :7;
  u32 month  :5;
  u32 day    :6;
  
  STRCODE title[MUSICAL_PROGRAM_NAME_MAX];
  MUSICAL_SHOT_POKE shotPoke[MUSICAL_POKE_MAX];
}MUSICAL_SHOT_DATA;

typedef struct
{
  MUSICAL_SHOT_DATA *musShotData;
}MUS_SHOT_INIT_WORK;

//======================================================================
//	proto
//======================================================================

extern GFL_PROC_DATA MusicalShot_ProcData;

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
