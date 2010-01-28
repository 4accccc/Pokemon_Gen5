//==============================================================================
/**
 * @brief   ミュージカルショット：サブヘッダ
 */
//==============================================================================
#ifndef __GT_GDS_MUSICAL_SUB_H__


//==============================================================================
//  定数定義
//==============================================================================
//ミュージカルポケモンの装備箇所識別用
//ユーザーが認識できる箇所。
typedef enum
{
	MUS_POKE_EQU_USER_EAR_R,	//耳(右耳)
	MUS_POKE_EQU_USER_EAR_L,	//耳(左耳)
	MUS_POKE_EQU_USER_HEAD,		//頭
	MUS_POKE_EQU_USER_FACE,		//顔
	MUS_POKE_EQU_USER_BODY,		//胴
	MUS_POKE_EQU_USER_WAIST,	//腰
	MUS_POKE_EQU_USER_HAND_R,	//手(右手)
	MUS_POKE_EQU_USER_HAND_L,	//手(左手)

	MUS_POKE_EQUIP_USER_MAX,
	
	MUS_POKE_EQU_USER_INVALID,	//無効値
}MUS_POKE_EQUIP_USER;

///ミュージカルのトレーナーネーム長
#define MUSICAL_TRAINER_NAME_LEN      (7+1) //プレイヤー名＋EOM

//ミュージカルに参加可能な人数
#define MUSICAL_POKE_MAX (4)

//ミュージカルで装備できるアイテムの個数
#define MUSICAL_ITEM_EQUIP_MAX (MUS_POKE_EQUIP_USER_MAX)	//顔＆頭・胴・右手・左手

//ミュージカルの演目名の文字数
#define MUSICAL_PROGRAM_NAME_MAX (36+EOM_SIZE) //日本18・海外36＋EOM


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
}MUSICAL_SHOT_POKE_EQUIP;

//--------------------------------------------------------------
//  ポケモン１体のデータ　84byte
//--------------------------------------------------------------
typedef struct
{
  u16 monsno; //ポケモン番号
  u16  sex :2; //性別
  u16  rare:1; //レアフラグ
  u16  form:5; //フォルム番号
  u16  pad :8;
  
  STRCODE trainerName[MUSICAL_TRAINER_NAME_LEN];  //トレーナー名
  MUSICAL_SHOT_POKE_EQUIP equip[MUSICAL_ITEM_EQUIP_MAX];  //装備グッズデータ(７個
  
}MUSICAL_SHOT_POKE;

//--------------------------------------------------------------
//  ミュージカルショット　416byte
//--------------------------------------------------------------
typedef struct
{
  u32 bgNo   :5;  //背景番号
  u32 spotBit:4;  //スポットライト対象(bit)(トップだったポケモン
  u32 year   :7;  //年
  u32 month  :5;  //月  これが０だったら無効データとみなす
  u32 day    :6;  //日
  u32 player :2;  //自分の番号
  u32 musVer :3;  //ミュージカルバージョン

  MUSICAL_SHOT_POKE shotPoke[MUSICAL_POKE_MAX]; //ポケモンデータ(４体
  STRCODE title[MUSICAL_PROGRAM_NAME_MAX];  //ミュージカルタイトル(日本18・海外36＋EOM
  u8 pmVersion;
  u8 pmLang;
}MUSICAL_SHOT_DATA;

#endif  //__GT_GDS_MUSICAL_SUB_H__
