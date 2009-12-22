//============================================================================
/**
 *@file		tr_ai_gamble.s
 *@brief	トレーナーAI
 *@author	HisashiSogabe
 *@data		2009.12.15
 */
//============================================================================

	.text
	.include	"tr_ai_seq_def.h"

//========================================================
//	ギャンブルAI
//========================================================
GambleAISeq:
	IF_MIKATA_ATTACK	GambleAI_end			//対象が味方なら終了

	CHECK_WAZASEQNO
	IFN_TABLE_JUMP	GambleAI_Table,GambleAI_end

	IF_RND_UNDER	128,GambleAI_end

	INCDEC		2
GambleAI_end:
	AIEND

GambleAI_Table:
	.long	1,7,9,38,43,49,83,88,89,98
	.long	118,120,122,140,142,144,170,185,199
	//2006.6.13
	.long	219,226,227,230,241,248
	.long	0xffffffff

