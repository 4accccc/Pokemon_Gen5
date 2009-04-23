//======================================================================
/**
 * @file	musical_system.h
 * @brief	ミュージカル用 汎用定義
 * @author	ariizumi
 * @data	09/02/10
 */
//======================================================================
#ifndef MUSICAL_DEFINE_H__
#define MUSICAL_DEFINE_H__
#include "poke_tool/poke_tool.h"

//======================================================================
//	define
//======================================================================
//ミュージカルの装備アイテムの最大個数(最大512で考えておく
#define MUSICAL_ITEM_MAX (250)
#define MUSICAL_ITEM_INVALID (511)
//ミュージカルに参加可能な人数
#define MUSICAL_POKE_MAX (4)

//ミュージカルで装備できるアイテムの個数
#define MUSICAL_ITEM_EQUIP_MAX (4)	//顔＆頭・胴・右手・左手

//======================================================================
//	enum
//======================================================================

//ミュージカルポケモンの装備箇所
typedef enum
{
	MUS_POKE_EQU_EAR_R,		//右耳
	MUS_POKE_EQU_EAR_L,		//左耳
	MUS_POKE_EQU_HEAD,		//頭
	MUS_POKE_EQU_EYE,		//目・鼻
	MUS_POKE_EQU_FACE,		//顔
	MUS_POKE_EQU_BODY,		//胴
	MUS_POKE_EQU_WAIST,		//腰
	MUS_POKE_EQU_HAND_R,	//右手
	MUS_POKE_EQU_HAND_L,	//左手

	MUS_POKE_EQUIP_MAX,
	
	MUS_POKE_EQU_INVALID,	//無効値
}MUS_POKE_EQUIP_POS;

//ミュージカルポケモンの装備箇所識別用
//アイテムデータに設定される。装備できる場所の種類
typedef enum
{
	MUS_POKE_EQU_TYPE_EAR,		//耳(右耳・左耳)
	MUS_POKE_EQU_TYPE_HEAD,		//頭
	MUS_POKE_EQU_TYPE_EYE,		//目
	MUS_POKE_EQU_TYPE_FACE,		//顔
	MUS_POKE_EQU_TYPE_BODY,		//胴
	MUS_POKE_EQU_TYPE_WAIST,	//腰
	MUS_POKE_EQU_TYPE_HAND,		//手(右手・左手)

	MUS_POKE_EQU_IS_BACK,			//背面装備判定

	MUS_POKE_EQUIP_TYPE_MAX,
	
	MUS_POKE_EQU_TYPE_INVALID,	//無効値
}MUS_POKE_EQUIP_TYPE;


//======================================================================
//	typedef struct
//======================================================================

typedef struct
{
	u16	itemNo;	//アイテムの種類
	s16	angle;	//アイテムの傾き
}MUSICAL_POKE_EQUIP;

typedef struct
{
	POKEMON_PARAM *pokePara;
	MUSICAL_POKE_EQUIP equip[MUS_POKE_EQUIP_MAX];
}MUSICAL_POKE_PARAM;

#endif MUSICAL_DEFINE_H__
