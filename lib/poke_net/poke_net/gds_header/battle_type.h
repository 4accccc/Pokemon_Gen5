#ifndef __POKE_GDS_BATTLETYPE_DEF_H_
#define __POKE_GDS_BATTLETYPE_DEF_H_

//戦闘種別フラグ（戦闘システム内の判別で使用していますBattleParamのfight_typeの指定には使わないでください）
#define	FIGHT_TYPE_1vs1			(0x00000000)		///<1vs1
#define	FIGHT_TYPE_YASEI		(0x00000000)		///<野生戦1vs1
#define	FIGHT_TYPE_TRAINER		(0x00000001)		///<トレーナー戦
#define	FIGHT_TYPE_2vs2			(0x00000002)		///<2vs2
#define	FIGHT_TYPE_SIO			(0x00000004)		///<通信対戦
#define	FIGHT_TYPE_MULTI		(0x00000008)		///<マルチ対戦
#define	FIGHT_TYPE_TAG			(0x00000010)		///<タッグ
#define	FIGHT_TYPE_SAFARI		(0x00000020)		///<サファリゾーン
#define	FIGHT_TYPE_AI			(0x00000040)		///<AIマルチ
#define	FIGHT_TYPE_TOWER		(0x00000080)		///<バトルタワー
#define	FIGHT_TYPE_MOVE			(0x00000100)		///<移動ポケモン
#define	FIGHT_TYPE_POKE_PARK	(0x00000200)		///<ポケパーク
#define	FIGHT_TYPE_GET_DEMO		(0x00000400)		///<捕獲デモ

#define	FIGHT_TYPE_DEBUG		(0x80000000)		///<デバッグ戦闘

//戦闘種別フラグ（必要なフラグをOR済みのものBattleParamのfight_type指定にはこちらを使ってください）
#define	FIGHT_TYPE_1vs1_YASEI				(FIGHT_TYPE_1vs1|FIGHT_TYPE_YASEI)						///<1vs1野生戦
#define	FIGHT_TYPE_2vs2_YASEI				(FIGHT_TYPE_2vs2|FIGHT_TYPE_MULTI|FIGHT_TYPE_AI)		///<2vs2野生戦
#define	FIGHT_TYPE_1vs1_TRAINER				(FIGHT_TYPE_1vs1|FIGHT_TYPE_TRAINER)					///<1vs1トレーナー戦
#define	FIGHT_TYPE_2vs2_TRAINER				(FIGHT_TYPE_2vs2|FIGHT_TYPE_TRAINER)					///<2vs2トレーナー戦
#define	FIGHT_TYPE_1vs1_SIO					(FIGHT_TYPE_SIO|FIGHT_TYPE_TRAINER)						///<1vs1通信トレーナー戦
#define	FIGHT_TYPE_2vs2_SIO					(FIGHT_TYPE_1vs1_SIO|FIGHT_TYPE_2vs2)					///<2vs2通信トレーナー戦
#define	FIGHT_TYPE_MULTI_SIO				(FIGHT_TYPE_2vs2_SIO|FIGHT_TYPE_MULTI)					///<2vs2通信トレーナー戦
#define	FIGHT_TYPE_AI_MULTI					(FIGHT_TYPE_2vs2_TRAINER|FIGHT_TYPE_MULTI|FIGHT_TYPE_AI)///<2vs2AIマルチ戦
#define	FIGHT_TYPE_BATTLE_TOWER_1vs1		(FIGHT_TYPE_1vs1_TRAINER|FIGHT_TYPE_TOWER)				///<1vs1バトルタワー戦
#define	FIGHT_TYPE_BATTLE_TOWER_2vs2		(FIGHT_TYPE_2vs2_TRAINER|FIGHT_TYPE_TOWER)				///<2vs2バトルタワー戦
#define	FIGHT_TYPE_BATTLE_TOWER_AI_MULTI	(FIGHT_TYPE_AI_MULTI|FIGHT_TYPE_TOWER)					///<バトルタワーAIマルチ戦
#define	FIGHT_TYPE_BATTLE_TOWER_SIO_MULTI	(FIGHT_TYPE_MULTI_SIO|FIGHT_TYPE_TOWER)					///<バトルタワー通信マルチ戦
#define	FIGHT_TYPE_TAG_BATTLE				(FIGHT_TYPE_2vs2_TRAINER|FIGHT_TYPE_TAG)				///<タッグバトル戦


#endif // __POKE_GDS_BATTLETYPE_DEF_H_
