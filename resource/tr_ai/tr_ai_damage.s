//============================================================================
/**
 *@file		tr_ai_damage.s
 *@brief	トレーナーAI
 *@author	HisashiSogabe
 *@data		2009.12.15
 */
//============================================================================

	.text
	.include	"tr_ai_seq_def.h"

//========================================================
//	ダメージ技重視AI
//========================================================
DamageAISeq:
	IF_MIKATA_ATTACK	DamageAI_end			//対象が味方なら終了

	COMP_POWER	LOSS_CALC_OFF
	IFN_EQUAL	COMP_POWER_NONE,DamageAI_end	// 威力計算する技
	IF_RND_UNDER	100,DamageAI_end
	
	INCDEC		2

DamageAI_end:
	AIEND

