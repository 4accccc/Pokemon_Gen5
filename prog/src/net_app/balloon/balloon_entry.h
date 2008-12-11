//==============================================================================
/**
 * @file	balloon_entry.h
 * @brief	風船割り：エントリー画面のヘッダ
 * @author	matsuda
 * @date	2007.11.06(火)
 */
//==============================================================================
#ifndef __BALLOON_ENTRY_H__
#define __BALLOON_ENTRY_H__


//==============================================================================
//	外部関数宣言
//==============================================================================
extern GFL_PROC_RESULT BalloonEntryProc_Init( GFL_PROC * proc, int * seq );
extern GFL_PROC_RESULT BalloonEntryProc_Main( GFL_PROC * proc, int * seq );
extern GFL_PROC_RESULT BalloonEntryProc_End(GFL_PROC *proc, int *seq);


#endif	//__BALLOON_ENTRY_H__

