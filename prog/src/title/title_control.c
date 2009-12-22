//==============================================================================
/**
 * @file	title_control.c
 * @brief	タイトルのPROC制御
 * @author	matsuda
 * @date	2008.12.05(金)
 *
 * ファイル名と機能が一致していないような気もする
 */
//==============================================================================
#include <gflib.h>
#include <procsys.h>
#include "system\main.h"
#include "title/title.h"
#include "demo/command_demo.h"
#include "corporate.h"


//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT TitleControlProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT TitleControlProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT TitleControlProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

//==============================================================================
//	データ
//==============================================================================

const GFL_PROC_DATA TitleControlProcData = {
	TitleControlProcInit,
	TitleControlProcMain,
	TitleControlProcEnd,
};

#ifdef PM_DEBUG
static u32 CorpRet;										// 社名表示処理ワーク
#endif
static COMMANDDEMO_DATA	cdemo_data;		// デモ処理ワーク


//==============================================================================
//
//	
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   
 *
 * @param   proc		
 * @param   seq		
 * @param   pwk		
 * @param   mywk		
 *
 * @retval  
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT TitleControlProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT TitleControlProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
#ifdef PM_DEBUG
	switch( *seq  ){
	case 0:
		cdemo_data.mode = COMMANDDEMO_MODE_TEST;
		cdemo_data.skip = COMMANDDEMO_SKIP_DEBUG;
		cdemo_data.ret  = COMMANDDEMO_RET_NORMAL;
		GFL_PROC_SysCallProc( FS_OVERLAY_ID(title), &CorpProcData, &CorpRet );
		*seq = 1;
		break;

	case 1:
		if( CorpRet == CORPORATE_RET_NORMAL ){
			GFL_PROC_SysCallProc( FS_OVERLAY_ID(command_demo), &COMMANDDEMO_ProcData, &cdemo_data );
		}
		*seq = 2;
		break;

	case 2:
		if( cdemo_data.ret == COMMANDDEMO_RET_SKIP_DEBUG ){
			CorpRet = CORPORATE_RET_DEBUG;
		}
//		GFL_PROC_SysCallProc( FS_OVERLAY_ID(title), &TitleProcData, &CorpRet );
//		GFL_PROC_SysSetNextProc( FS_OVERLAY_ID(title), &TitleProcData, &CorpRet );
		return GFL_PROC_RES_FINISH;
	}
#else
	switch( *seq  ){
	case 0:
		cdemo_data.mode = COMMANDDEMO_MODE_TEST;
		cdemo_data.skip = COMMANDDEMO_SKIP_ON;
		cdemo_data.ret  = COMMANDDEMO_RET_NORMAL;
		GFL_PROC_SysCallProc( FS_OVERLAY_ID(title), &CorpProcData, NULL );
		*seq = 1;
		break;

	case 1:
		GFL_PROC_SysCallProc( FS_OVERLAY_ID(command_demo), &COMMANDDEMO_ProcData, &cdemo_data );
		*seq = 2;
		break;

	case 2:
//		GFL_PROC_SysCallProc( FS_OVERLAY_ID(title), &TitleProcData, NULL );
//		GFL_PROC_SysSetNextProc( FS_OVERLAY_ID(title), &TitleProcData, NULL );
		return GFL_PROC_RES_FINISH;
	}
#endif

	return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT TitleControlProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
#ifdef PM_DEBUG
	GFL_PROC_SysSetNextProc( FS_OVERLAY_ID(title), &TitleProcData, &CorpRet );
#else
	GFL_PROC_SysSetNextProc( FS_OVERLAY_ID(title), &TitleProcData, NULL );
#endif

	return GFL_PROC_RES_FINISH;
}
