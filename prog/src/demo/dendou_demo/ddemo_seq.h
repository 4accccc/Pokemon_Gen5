//============================================================================================
/**
 * @file		ddemo_seq.h
 * @brief		殿堂入りデモ画面
 * @author	Hiroyuki Nakamura
 * @date		10.04.06
 *
 *	モジュール名：DDEMOSEQ
 */
//============================================================================================
#pragma	once


enum {
	MAINSEQ_INIT = 0,
	MAINSEQ_RELEASE,
	MAINSEQ_WIPE,
	MAINSEQ_WAIT,

	MAINSEQ_1ST_INIT,
	MAINSEQ_1ST_MAIN,
	MAINSEQ_1ST_EXIT,

	MAINSEQ_2ND_INIT,
	MAINSEQ_2ND_MAIN,
	MAINSEQ_2ND_EXIT,

	MAINSEQ_END
};


extern BOOL DDEMOSEQ_MainSeq( DDEMOMAIN_WORK * wk );
