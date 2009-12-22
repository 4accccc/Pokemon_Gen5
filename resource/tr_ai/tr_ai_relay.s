//============================================================================
/**
 *@file		tr_ai_relay.s
 *@brief	トレーナーAI
 *@author	HisashiSogabe
 *@data		2009.12.15
 */
//============================================================================

	.text
	.include	"tr_ai_seq_def.h"

//========================================================
//	リレー型AI
//========================================================
RelayAISeq:
	IF_MIKATA_ATTACK	RelayAI_end		//対象が味方なら終了

	CHECK_BENCH_COUNT	CHECK_ATTACK
	IF_EQUAL	0,RelayAI_end			// 味方の控えがいない

	COMP_POWER	LOSS_CALC_OFF
	IFN_EQUAL	COMP_POWER_NONE,RelayAI_end	// 威力計算する技

	IF_HAVE_WAZA_SEQNO	CHECK_ATTACK,127,RelayAI_1		// バトンタッチ

	IF_RND_UNDER	80,RelayAI_end

RelayAI_1:
	IF_WAZANO	WAZANO_TURUGINOMAI,RelayAI_Turugi	// つるぎのまい
	IF_WAZANO	WAZANO_RYUUNOMAI,RelayAI_Turugi		// りゅうのまい
	IF_WAZANO	WAZANO_MEISOU,RelayAI_Turugi		// めいそう
	IF_WAZANO	WAZANO_WARUDAKUMI,RelayAI_Turugi	// わるだくみ2006.6.13

	IF_WAZA_SEQNO_JUMP	111, ReplayAI_Mamoru		// まもる等
	IF_WAZANO	WAZANO_BATONTATTI, RelayAI_Baton


	IF_RND_UNDER	20,RelayAI_end
	INCDEC		3

// つるぎのまい等の強化ワザ
RelayAI_Turugi:
	CHECK_TURN
	IF_EQUAL	0, AI_INC5
	IF_HP_UNDER	CHECK_ATTACK,60, AI_DEC10
	JUMP AI_INC1

// まもるでターン引き伸ばし
ReplayAI_Mamoru:
	CHECK_LAST_WAZA			CHECK_ATTACK
	IF_TABLE_JUMP		ReplayAI_MamoruTbl, AI_DEC2
	INCDEC	2
	AIEND

ReplayAI_MamoruTbl:
	.long	WAZANO_MAMORU, WAZANO_MIKIRI
	.long	0xffffffff

// バトンタッチ
RelayAI_Baton:
	//特性「たんじゅん」チェックいれる予定
	CHECK_TURN
	IF_EQUAL	0, AI_DEC2
	IF_PARA_OVER	CHECK_ATTACK, PARA_POW, 8, AI_INC3
	IF_PARA_OVER	CHECK_ATTACK, PARA_POW, 7, AI_INC2
	IF_PARA_OVER	CHECK_ATTACK, PARA_POW, 6, AI_INC1
	IF_PARA_OVER	CHECK_ATTACK, PARA_SPEPOW, 8, AI_INC3
	IF_PARA_OVER	CHECK_ATTACK, PARA_SPEPOW, 7, AI_INC2
	IF_PARA_OVER	CHECK_ATTACK, PARA_SPEPOW, 6, AI_INC1
	AIEND

RelayAI_end:
	AIEND

AI_DEC1:
	INCDEC		-1	
	AIEND
AI_DEC2:
	INCDEC		-2	
	AIEND
AI_DEC3:
	INCDEC		-3	
	AIEND
AI_DEC5:
	INCDEC		-5	
	AIEND
AI_DEC6:
	INCDEC		-6	
	AIEND
AI_DEC8:
	INCDEC		-8	
	AIEND
AI_DEC10:
	INCDEC		-10
	AIEND
AI_DEC12:
	INCDEC		-12
	AIEND
AI_DEC30:
	INCDEC		-30
	AIEND
AI_INC1:
	INCDEC		1	
	AIEND
AI_INC2:
	INCDEC		2	
	AIEND
AI_INC3:
	INCDEC		3	
	AIEND
AI_INC5:
	INCDEC		5	
	AIEND
AI_INC10:
	INCDEC		10	
	AIEND

