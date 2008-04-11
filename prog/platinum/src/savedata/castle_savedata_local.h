//==============================================================================================
/**
 * @file	castle_savedata_local.h
 * @brief	「バトルキャッスル」ヘッダー
 * @author	Satoshi Nohara
 * @date	2007.07.03
 */
//==============================================================================================
#ifndef	__CASTLE_SAVEDATA_LOCAL_H__
#define	__CASTLE_SAVEDATA_LOCAL_H__

#include "..\frontier\castle_def.h"

//============================================================================================
//
//	プレイデータ
//
//============================================================================================
struct _CASTLEDATA{
	u8	type		:3;									//シングル、ダブル、マルチ、wifiマルチ
	u8	save_flag	:1;									//セーブ済みかどうかフラグ
	u8	dmy99		:4;
	u8	round;											//今何人目？
	u8	dmy89;

	//参加している手持ちポケモンの位置情報
	u8 mine_poke_pos[3];

	//敵トレーナーデータ
	u16 tr_index[CASTLE_LAP_MULTI_ENEMY_MAX];			//トレーナーindexテーブル[7*2]

	//手持ちポケモン
	u16	hp[CASTLE_MINE_POKE_MAX];						//HP
	u16	pp[CASTLE_MINE_POKE_MAX][4];					//技4つのPP
	u8	condition[CASTLE_MINE_POKE_MAX];				//毒、麻痺などの状態
	u16 item[CASTLE_MINE_POKE_MAX];						//道具

	//敵ポケモンデータが被らないように保存
	u16 enemy_poke_index[CASTLE_LAP_MULTI_ENEMY_MAX];	//ポケモンindexテーブル

	u16 dummy;											//4byte境界
};


//============================================================================================
//
//	成績データ
//
//============================================================================================
struct _CASTLESCORE{
	//↓WIFI以外のバトルタイプで使用している
	u8	clear_flag;										//7連勝(クリア)したかフラグ(ビット)

	u8	guide_flag;										//説明受けたか
	u8	dmy[2];
};


#endif	__CASTLE_SAVEDATA_LOCAL_H__


