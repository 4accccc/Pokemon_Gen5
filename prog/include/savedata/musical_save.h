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
	u16	itemNo;	      //アイテムの種類
	s16	angle;	      //アイテムの傾き
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
extern void MUSICAL_SAVE_InitWork(MUSICAL_SAVE * musSave);

//----------------------------------------------------------
//	データ操作のための関数
//----------------------------------------------------------
extern MUSICAL_SAVE* MUSICAL_SAVE_GetMusicalSave( SAVE_CONTROL_WORK *sv );

extern void MUSICAL_SAVE_ResetBefEquip( MUSICAL_SAVE *musSave );
extern MUSICAL_EQUIP_SAVE* MUSICAL_SAVE_GetBefEquipData( MUSICAL_SAVE *musSave );

#endif //MUSICAL_SAVE_H__
