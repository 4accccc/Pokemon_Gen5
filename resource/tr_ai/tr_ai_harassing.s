//============================================================================
/**
 *@file		tr_ai_harassing.s
 *@brief	トレーナーAI
 *@author	HisashiSogabe
 *@data		2009.12.15
 */
//============================================================================

	.text
	.include	"tr_ai_seq_def.h"

//========================================================
//	嫌がらせAI
//========================================================
HarassingAISeq:
	IF_MIKATA_ATTACK	HarassingAISeq_End		
	CHECK_WAZASEQNO
	IFN_TABLE_JUMP	HarassingAI_Table,HarassingAISeq_End
	IF_RND_UNDER	128,HarassingAISeq_End
	INCDEC	+2	

HarassingAISeq_End:
	AIEND

HarassingAI_Table:
	.long	1,18,19,23,24,49,58,59,60,62
	.long	66,67,84,90,100,112,118,120,165,166
	.long	167,173,187,188,192,197,199,205,213,232	
	.long	234,249,258,265	
	.long	0xffffffff

