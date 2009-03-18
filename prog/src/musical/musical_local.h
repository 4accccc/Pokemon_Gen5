
//============================================================================================
/**
 * @file	musical_local.h
 * @brief	ミュージカル内用定義
 * @author	ariizumi
 * @date	2008.11.14
 */
//============================================================================================

#ifndef	MUSICAL_LOCAL_H_
#define	MUSICAL_LOCAL_H_

//ミュージカルポケモンの装備箇所(パレット対応
typedef enum
{
	MUS_POKE_PLT_SHADOW		=0x04,	//影用座標
	MUS_POKE_PLT_ROTATE		=0x05,	//回転用座標
	
	MUS_POKE_PLT_HEAD		=0x08,	//頭
	MUS_POKE_PLT_EAR_R		=0x09,	//右耳
	MUS_POKE_PLT_EAR_L		=0x0A,	//左耳
	MUS_POKE_PLT_BODY		=0x0B,	//胴
	MUS_POKE_PLT_WAIST		=0x0C,	//腰
	MUS_POKE_PLT_HAND_R		=0x0D,	//右手
	MUS_POKE_PLT_HAND_L		=0x0E,	//左手
	MUS_POKE_PLT_TAIL		=0x0F,	//尻尾
}MUS_POKE_EQUIP_PLT;

//パレット番号から装備箇所へ変換
#define MUS_POKE_PLT_TO_POS(val) ((MUS_POKE_EQUIP_POS)(val-MUS_POKE_PLT_HEAD))
//グループ番号から回転へ変換(32等分)
//#define MUS_POKE_GRP_TO_ROT(val) (val*(360/32)*65536/360);
#define MUS_POKE_GRP_TO_ROT(val) (val*(65536/32))	//360を通分

#endif	//MUSICAL_LOCAL_H_
