//==============================================================================
/**
 * @file	title.h
 * @brief	タイトル画面のヘッダ
 * @author	matsuda
 * @date	2008.12.03(水)
 */
//==============================================================================
#ifndef __TITLE_H__
#define __TITLE_H__

//==============================================================================
//	外部関数宣言
//==============================================================================
extern GFL_PROC_RESULT TitleProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
extern GFL_PROC_RESULT TitleProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
extern GFL_PROC_RESULT TitleProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk );


#endif	//__TITLE_H__
