//============================================================================================
/**
 * @file	musical_save.h
 * @brief	ミュージカル用セーブデータ
 * @author	Nobuhiko Ariizumi
 * @date	2009/04/08
 */
//============================================================================================

#ifndef MUSICAL_SAVE_H__
#define MUSICAL_SAVE_H__

#include "savedata/save_control.h"	//SAVE_CONTROL_WORK参照のため
#include "musical/musical_define.h"


//======================================================================
//	typedef struct
//======================================================================

typedef struct
{
	u8	pos;					//装備箇所(MUS_POKE_EQUIP_POS）
	MUSICAL_POKE_EQUIP data;	//アイテム番号と角度
}MUSICAL_EQUIP_ONE_SAVE;

typedef struct
{
	MUSICAL_EQUIP_ONE_SAVE equipData[MUSICAL_ITEM_EQUIP_MAX];
}MUSICAL_EQUIP_SAVE;

typedef struct _MUSICAL_SAVE MUSICAL_SAVE;


//======================================================================
//	proto
//======================================================================

//----------------------------------------------------------
//	セーブデータシステムが依存する関数
//----------------------------------------------------------
extern int MUSICAL_SAVE_GetWorkSize(void);
extern void MUSICAL_SAVE_InitWork(MUSICAL_SAVE * my);

//----------------------------------------------------------
//	データ操作のための関数
//----------------------------------------------------------


#endif //MUSICAL_SAVE_H__
