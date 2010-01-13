//
//============================================================================================
/**
 * @file	tr_ai_def.h
 * @brief	トレーナーAIプログラム用定義ファイル
 * @author	HisashiSogabe
 * @date	06.04.25
 */
//============================================================================================
#pragma once

//#define	AIWT				((AIWorkTbl *)&UserWork[0x16800])
//#define	AIWT2				((AIWorkTbl2 *)&UserWork[0x16a00])
//#define	AI_PUSH_ADRS		((PushAdrs *)&UserWork[0x16c00])	//4バイト*8+1

#define	LOSS_CALC_OFF	(0)		//ダメージロス計算のぶれなし
#define	LOSS_CALC_ON	(1)		//ダメージロス計算のぶれあり

//=========================================================================
//	AI_STATUSFLAG宣言
//=========================================================================

#define	AI_STATUSFLAG_END		    (0x01)			//AIシーケンスの終了
#define	AI_STATUSFLAG_ESCAPE	  (0x02)			//にげるを選択
#define	AI_STATUSFLAG_SAFARI	  (0x04)			//サファリゾーン特有のアクション
#define	AI_STATUSFLAG_FINISH	  (0x08)			//チェックする技が残っていても終了
#define	AI_STATUSFLAG_CONTINUE	(0x10)			//AI計算継続

#define	AI_STATUSFLAG_END_OFF		    (0x01^0xff)
#define	AI_STATUSFLAG_ESCAPE_OFF	  (0x01^0xff)
#define	AI_STATUSFLAG_SAFARI_OFF	  (0x04^0xff)
#define	AI_STATUSFLAG_CONTINUE_OFF	(0x10^0xff)			//AI計算継続

#define	AI_ENEMY_ESCAPE		  4
#define	AI_ENEMY_SAFARI		  5
#define	AI_ENEMY_RESHUFFLE	6

//tr_ai.s waza_ai.cで使用（マクロでsideに指定する値）
#define	CHECK_DEFENCE			    (0)
#define	CHECK_ATTACK			    (1)
#define	CHECK_DEFENCE_FRIEND	(2)
#define	CHECK_ATTACK_FRIEND		(3)

#define	CHECK_DEFENCE_TYPE1		(0)
#define	CHECK_ATTACK_TYPE1		(1)
#define	CHECK_DEFENCE_TYPE2		(2)
#define	CHECK_ATTACK_TYPE2		(3)
#define	CHECK_WAZA					  (4)
#define	CHECK_DEFENCE_FRIEND_TYPE1	(5)
#define	CHECK_ATTACK_FRIEND_TYPE1	  (6)
#define	CHECK_DEFENCE_FRIEND_TYPE2	(7)
#define	CHECK_ATTACK_FRIEND_TYPE2	  (8)


//COMP_POWERで使用
#define	COMP_POWER_NONE		(0)		//威力計算しない技
#define	COMP_POWER_NOTOP	(1)		//ダメージ量がトップじゃない
#define	COMP_POWER_TOP		(2)		//ダメージ量がトップ

//IF_FIRSTで使用
#define	IF_FIRST_ATTACK		(0)		//自分が早い
#define	IF_FIRST_DEFENCE	(1)		//相手が早い
#define	IF_FIRST_EQUAL		(2)		//同じ

//AI_CHECK_AISYOUで使用
//AI_CHECK_WAZA_AISYOUで使用
#define	AISYOU_4BAI			(BTL_TYPEAFF_400)	//4倍効果
#define	AISYOU_2BAI			(BTL_TYPEAFF_200)	//2倍効果
#define	AISYOU_1BAI			(BTL_TYPEAFF_100)	//1倍効果
#define	AISYOU_1_2BAI		(BTL_TYPEAFF_50)	//1/2倍効果
#define	AISYOU_1_4BAI		(BTL_TYPEAFF_25)	//1/4倍効果
#define	AISYOU_0BAI			(BTL_TYPEAFF_0)		//効果なし

//AI_IF_PARA_UNDERで使用
//AI_IF_PARA_OVERで使用
//AI_IF_PARA_EQUALで使用
//AI_IFN_PARA_EQUALで使用
#define	PARA_HP					  0x00			//体力
#define	PARA_POW				  ( BPP_ATTACK_RANK )     //攻撃力
#define	PARA_DEF				  ( BPP_DEFENCE_RANK )    //防御力
#define	PARA_AGI				  ( BPP_AGILITY_RANK )    //素早さ
#define	PARA_SPEPOW				( BPP_SP_ATTACK_RANK )  //特攻
#define	PARA_SPEDEF				( BPP_SP_DEFENCE_RANK )	//特防
#define	PARA_HIT				  ( BPP_HIT_RATIO )			  //命中率
#define	PARA_AVOID				( BPP_AVOID_RATIO )			//回避率

//AI_IF_POKE_CHECK_STATE,AI_IF_WAZA_CHECK_STATEで使用
#define	STATE_KANASIBARI		0x00			//かなしばり
#define	STATE_ENCORE			0x01			//アンコール

//AI_CHECK_WEATHERで使用
#define	WEATHER_NONE			  ( BTL_WEATHER_NONE )    //天候変化なし
#define	WEATHER_HARE			  (	BTL_WEATHER_SHINE )   //にほんばれ状態
#define	WEATHER_AME				  ( BTL_WEATHER_RAIN )    //あまごい状態
#define	WEATHER_SUNAARASHI	(	BTL_WEATHER_SAND )    //すなあらし状態
#define	WEATHER_ARARE			  ( BTL_WEATHER_SNOW )		//あられ状態
#define	WEATHER_HUKAIKIRI		( BTL_WEATHER_NONE )		//ふかいきり状態（現状存在しない）

//AI_IF_LEVELで使用
#define	LEVEL_ATTACK			0x00			//アタックのレベルが高い
#define	LEVEL_DEFENCE			0x01			//ディフェンスのレベルが高い
#define	LEVEL_EQUAL				0x02			//レベルが一緒

// AI_HAVE_TOKUSEI, AI_HAVE_WAZA 等で使用
#define HAVE_NO			0
#define HAVE_YES		1
#define HAVE_UNKNOWN	2

// AI_IF_TABLE_JUMP, AI_IFN_TABLE_JUMPで使用するテーブルの終端
#define TR_AI_TABLE_END ( 0xffffffff )
