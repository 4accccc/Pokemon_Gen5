//============================================================================
/**
 *@file		tr_ai_other.s
 *@brief	トレーナーAI
 *@author	HisashiSogabe
 *@data		2009.12.15
 */
//============================================================================

	.text
	.include	"tr_ai_seq_def.h"

//========================================================
//	移動ポケモンAI
//========================================================
//移動ポケモンは１００％逃げる
MvPokeAISeq:
	IF_COND2		CHECK_ATTACK,CONDITION2_SHIME,MvPokeAISeq_end	// しめ技
	IF_COND2		CHECK_ATTACK,CONDITION2_KUROIMANAZASHI,MvPokeAISeq_end	// くろいまなざし
	GET_TOKUSEI		CHECK_DEFENCE
	IF_EQUAL		TOKUSYU_KAGEHUMI,MvPokeAISeq_end	// かげふみ
	GET_TOKUSEI		CHECK_ATTACK
	IF_EQUAL		TOKUSYU_HUYUU,MvPokeAISeq_Escape	// ふゆうは、ありじごくがきかない
	GET_TOKUSEI		CHECK_DEFENCE
	IF_EQUAL		TOKUSYU_ARIZIGOKU,MvPokeAISeq_end	// ありじごく
	
MvPokeAISeq_Escape:
	ESCAPE

MvPokeAISeq_end:
	AIEND

//========================================================
//	サファリゾーンAI
//========================================================

SafariAISeq:
	SAFARI_ESCAPE_JUMP	SafariAIEscape
	SAFARI
SafariAIEscape:
	ESCAPE

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

DummyAISeq:
	AIEND

