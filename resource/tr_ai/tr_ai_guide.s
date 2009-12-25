//============================================================================
/**
 *@file		tr_ai_guide.s
 *@brief	トレーナーAI
 *@author	HisashiSogabe
 *@data		2009.12.15
 */
//============================================================================

	.text
	.include	"tr_ai_seq_def.h"

//========================================================
//	ガイドバトルAI
//========================================================

GuideAISeq:
	IF_HP_EQUAL	CHECK_DEFENCE,20,GuideAIHPHalf
	IF_HP_UNDER	CHECK_DEFENCE,20,GuideAIHPHalf
	AIEND

//相手のHPが半分以下なら「にげる」
GuideAIHPHalf:
	ESCAPE
	AIEND

