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
#if 0
DamageAISeq:
	IF_MIKATA_ATTACK	DamageAI_end			//対象が味方なら終了

	COMP_POWER	LOSS_CALC_OFF
	IFN_EQUAL	COMP_POWER_NONE,DamageAI_end	// 威力計算する技
	IF_RND_UNDER	100,DamageAI_end
	
	INCDEC		2

DamageAI_end:
	AIEND

#else

DamageAISeq:            //　レシラム　ゼクロム なら　最初にクロス技を出す
  CHECK_MONSNO	CHECK_ATTACK
	IF_EQUAL	MONSNO_RESIRAMU,DensetsuAI
	IF_EQUAL	MONSNO_ZEKUROMU,DensetsuAI

  JUMP  DensetsuAI_End

DensetsuAI:
	CHECK_TURN
	IFN_EQUAL	0,DensetsuAI_End

	IF_WAZANO	WAZANO_KUROSUSANDAA,DensetsuAI_Inc
	IF_WAZANO	WAZANO_KUROSUHUREIMU,DensetsuAI_Inc

  JUMP  DensetsuAI_End

DensetsuAI_Inc:
	INCDEC		10

DensetsuAI_End:
	AIEND

#endif

