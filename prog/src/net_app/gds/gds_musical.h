//==============================================================================
/**
 *
 * ミュージカルショット
 *
 */
//==============================================================================
#ifndef __GT_GDS_MUSICAL_H__
#define __GT_GDS_MUSICAL_H__

#include "gds_define.h"
#include "gds_profile_local.h"


//==============================================================================
//  定数定義
//==============================================================================
//ミュージカルポケモンの装備箇所識別用
//ユーザーが認識できる箇所。
typedef enum
{
	GT_MUS_POKE_EQU_USER_EAR_R,	//耳(右耳)
	GT_MUS_POKE_EQU_USER_EAR_L,	//耳(左耳)
	GT_MUS_POKE_EQU_USER_HEAD,		//頭
	GT_MUS_POKE_EQU_USER_FACE,		//顔
	GT_MUS_POKE_EQU_USER_BODY,		//胴
	GT_MUS_POKE_EQU_USER_HAND_R,	//手(右手)
	GT_MUS_POKE_EQU_USER_HAND_L,	//手(左手)

	GT_MUS_POKE_EQUIP_USER_MAX,
	
	GT_MUS_POKE_EQU_USER_INVALID,	//無効値
}GT_MUS_POKE_EQUIP_USER;

///ミュージカルのトレーナーネーム長
#define GT_MUSICAL_TRAINER_NAME_LEN      (7+1) //プレイヤー名＋EOM

//ミュージカルに参加可能な人数
#define GT_MUSICAL_POKE_MAX (4)

//ミュージカルで装備できるアイテムの個数
#define GT_MUSICAL_ITEM_EQUIP_MAX (GT_MUS_POKE_EQUIP_USER_MAX)	//顔＆頭・胴・右手・左手

//ミュージカルの演目名の文字数
#define GT_MUSICAL_PROGRAM_NAME_MAX (36+EOM_SIZE) //日本18・海外36＋EOM


//==============================================================================
//  構造体定義
//==============================================================================
//--------------------------------------------------------------
//  装備グッズデータ    8byte
//--------------------------------------------------------------
typedef struct
{
  u16 itemNo;   //グッズ番号
  s16 angle;    //角度
  u8  equipPos; //装備箇所
  u8  pad[3];
}GT_MUSICAL_SHOT_POKE_EQUIP;

//--------------------------------------------------------------
//  ポケモン１体のデータ　76byte
//--------------------------------------------------------------
typedef struct
{
  u16 monsno; //ポケモン番号
  u16  sex :2; //性別
  u16  rare:1; //レアフラグ
  u16  form:5; //フォルム番号
  u16  pad :8;
  
  GT_STRCODE trainerName[GT_MUSICAL_TRAINER_NAME_LEN];  //トレーナー名
  GT_MUSICAL_SHOT_POKE_EQUIP equip[GT_MUSICAL_ITEM_EQUIP_MAX];  //装備グッズデータ(７個
  
}GT_MUSICAL_SHOT_POKE;

//--------------------------------------------------------------
//  ミュージカルショット　384byte
//--------------------------------------------------------------
typedef struct
{
  u32 bgNo   :5;  //背景番号
  u32 spotBit:4;  //スポットライト対象(bit)
  u32 year   :7;  //年
  u32 month  :5;  //月  これが０だったら無効データとみなす
  u32 day    :6;  //日
  u32 pad    :5;
  
  GT_STRCODE title[GT_MUSICAL_PROGRAM_NAME_MAX];  //ミュージカルタイトル(日本18・海外36＋EOM
  GT_MUSICAL_SHOT_POKE shotPoke[GT_MUSICAL_POKE_MAX]; //ポケモンデータ(４体
}GT_MUSICAL_SHOT_DATA;


//--------------------------------------------------------------
/**
 *	送信データ：ミュージカルショット
 *		512byte
 */
//--------------------------------------------------------------
typedef struct{
	GT_GDS_PROFILE profile;
	GT_MUSICAL_SHOT_DATA mus_shot;
}GT_MUSICAL_SHOT_SEND;

//--------------------------------------------------------------
/**
 *	受信データ：ミュージカルショット
 *		512byte
 */
//--------------------------------------------------------------
typedef struct{
	GT_GDS_PROFILE profile;
	GT_MUSICAL_SHOT_DATA mus_shot;
}GT_MUSICAL_SHOT_RECV;


#endif  //__GT_GDS_MUSICAL_H__
