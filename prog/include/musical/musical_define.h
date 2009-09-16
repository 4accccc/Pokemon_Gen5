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
#define MUSICAL_ITEM_EQUIP_MAX (MUS_POKE_EQUIP_USER_MAX)	//顔＆頭・胴・右手・左手

//ミュージカルの演目名の文字数
#define MUSICAL_PROGRAM_NAME_MAX (26) //日本12・海外24＋EOM

//======================================================================
//	enum
//======================================================================

//ミュージカルポケモンの装備箇所
//データ上の装備箇所
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
//ユーザーが認識できる箇所。
typedef enum
{
	MUS_POKE_EQU_USER_EAR_R,	//耳(右耳)
	MUS_POKE_EQU_USER_EAR_L,	//耳(左耳)
	MUS_POKE_EQU_USER_HEAD,		//頭
	MUS_POKE_EQU_USER_FACE,		//顔
	MUS_POKE_EQU_USER_BODY,		//胴
	MUS_POKE_EQU_USER_HAND_R,	//手(右手)
	MUS_POKE_EQU_USER_HAND_L,	//手(左手)

	MUS_POKE_EQUIP_USER_MAX,
	
	MUS_POKE_EQU_USER_INVALID,	//無効値
}MUS_POKE_EQUIP_USER;

//ミュージカルのキャラの種類
typedef enum
{
  MUS_CHARA_PLAYER, //自分
  MUS_CHARA_COMM,   //通信相手
  MUS_CHARA_NPC,    //NPC

  MUS_CHARA_MAX,

  MUS_CHARA_INVALID,    //無効値
}MUSICAL_CHARA_TYPE;

//======================================================================
//	typedef struct
//======================================================================

typedef struct
{
	u16	itemNo;	//アイテムの種類
	s16	angle;	//アイテムの傾き
	u8  priority; //表示優先度
}MUSICAL_POKE_EQUIP;

typedef struct
{
  u16 monsno;
  u8  sex;
  u8  form;
  u8  rare;
}MUSICAL_MCSS_PARAM;

typedef struct
{
  MUSICAL_CHARA_TYPE charaType;
	POKEMON_PARAM *pokePara;
	MUSICAL_MCSS_PARAM mcssParam;
	MUSICAL_POKE_EQUIP equip[MUS_POKE_EQUIP_MAX];
	u16                point; //評価点
}MUSICAL_POKE_PARAM;

#endif MUSICAL_DEFINE_H__
