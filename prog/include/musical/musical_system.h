//======================================================================
/**
 * @file	musical_system.h
 * @brief	ミュージカル用 汎用定義
 * @author	ariizumi
 * @data	09/02/10
 */
//======================================================================
#ifndef MUSICAL_SYSTEM_H__
#define MUSICAL_SYSTEM_H__
#include "poke_tool/poke_tool.h"

//======================================================================
//	define
//======================================================================
//ミュージカルの装備アイテムの最大個数
#define MUSICAL_ITEM_MAX (240)

//======================================================================
//	enum
//======================================================================

//ミュージカルポケモンの装備箇所
typedef enum
{
	MUS_POKE_EQU_EAR_R,		//右耳
	MUS_POKE_EQU_EAR_L,		//左耳
	MUS_POKE_EQU_HEAD,		//頭
	MUS_POKE_EQU_FACE,		//顔
	MUS_POKE_EQU_BODY,		//胴
	MUS_POKE_EQU_HAND_R,	//右手
	MUS_POKE_EQU_HAND_L,	//左手
	MUS_POKE_EQU_WAIST,		//腰
	MUS_POKE_EQU_TAIL,		//尻尾

	MUS_POKE_EQUIP_MAX,
	
	MUS_POKE_EQU_INVALID,	//無効値
}MUS_POKE_EQUIP_POS;

//======================================================================
//	typedef struct
//======================================================================

typedef struct
{
	u16	type;	//アイテムの種類
	u8	angle;	//アイテムの傾き
}MUSICAL_POKE_EQUIP;

typedef struct
{
	POKEMON_PARAM *pokePara;
	MUSICAL_POKE_EQUIP equip[MUS_POKE_EQUIP_MAX];
}MUSICAL_POKE_PARAM;


//ミュージカルの参加資格があるか調べる(仮)
BOOL	MUSICAL_SYSTEM_CheckEntryMusical( POKEMON_PARAM *pokePara );
//ミュージカル用パラメータの初期化
MUSICAL_POKE_PARAM* MUSICAL_SYSTEM_InitMusPoke( POKEMON_PARAM *pokePara , HEAPID heapId );

#endif MUSICAL_SYSTEM_H__
