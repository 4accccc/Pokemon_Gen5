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
	GET_TOKUSEI		CHECK_DEFENCE
	IF_EQUAL		TOKUSYU_ARIZIGOKU,MvPokeAISeq_end	// ありじごく
	
MvPokeAISeq_Escape:
	ESCAPE

MvPokeAISeq_end:
	AIEND

