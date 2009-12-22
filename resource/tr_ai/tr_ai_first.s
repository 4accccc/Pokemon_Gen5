//============================================================================
/**
 *@file		tr_ai_first.s
 *@brief	トレーナーAI
 *@author	HisashiSogabe
 *@data		2009.12.15
 */
//============================================================================

	.text
	.include	"tr_ai_seq_def.h"

//========================================================
//	最初のターンに補助技を出す
//========================================================
FirstHojoAISeq:
	IF_MIKATA_ATTACK	FirstHojoAI_end			//対象が味方なら終了

	CHECK_TURN
	IFN_EQUAL	0,FirstHojoAI_end
	
	CHECK_WAZASEQNO
	IFN_TABLE_JUMP	FirstHojoAI_Table,FirstHojoAI_end

	IF_RND_UNDER	80,FirstHojoAI_end

	INCDEC		2
	
FirstHojoAI_end:
	AIEND

FirstHojoAI_Table:
	.long	10,11,12,13,14,15,16
	.long	18,19,20,21,22,23,24
	.long	30,35,54,47,49
	.long	50,51,52,53,54,55,56
	.long	58,59,60,61,62,63,64
	.long	65,66,67,79,84,108,109
	.long	118,213,187,156,165,166,167
	.long	181,192,199,205,206,208,211
	.long	213
	//2006.6.13
	.long	225,226,240,252,258,261
	.long	0xffffffff

