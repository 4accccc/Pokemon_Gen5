//============================================================================
/**
 *@file		tr_ai_move.s
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
	IF_WAZASICK		CHECK_ATTACK,WAZASICK_BIND,MvPokeAISeq_end	// しめ技
	IF_WAZASICK		CHECK_ATTACK,WAZASICK_TOOSENBOU,MvPokeAISeq_end	// くろいまなざし
	GET_TOKUSEI		CHECK_DEFENCE
	IF_EQUAL		TOKUSYU_KAGEHUMI,MvPokeAISeq_end	// かげふみ
	GET_TOKUSEI		CHECK_ATTACK
	IF_EQUAL		TOKUSYU_HUYUU,MvPokeAISeq_Escape	// ふゆうは、ありじごくがきかない

//2010.6.19　追加
 	CHECK_TYPE	CHECK_ATTACK_TYPE1
	IF_EQUAL	POKETYPE_HIKOU,MvPokeAISeq_Escape	// ひこうタイプは、ありじごくがきかない
	CHECK_TYPE	CHECK_ATTACK_TYPE2
	IF_EQUAL	POKETYPE_HIKOU,MvPokeAISeq_Escape	// ひこうトタイプは、ありじごくがきかない

	GET_TOKUSEI		CHECK_DEFENCE
	IF_EQUAL		TOKUSYU_ARIZIGOKU,MvPokeAISeq_end	// ありじごく
	
MvPokeAISeq_Escape:
	ESCAPE

MvPokeAISeq_end:
	AIEND

