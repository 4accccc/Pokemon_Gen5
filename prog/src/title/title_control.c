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


//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT TitleControlProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT TitleControlProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT TitleControlProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

//==============================================================================
//	データ
//==============================================================================
extern const GFL_PROC_DATA CorpProcData;

const GFL_PROC_DATA TitleControlProcData = {
	TitleControlProcInit,
	TitleControlProcMain,
	TitleControlProcEnd,
};

static COMMANDDEMO_DATA	cdemo_data;


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
	switch( *seq  ){
	case 0:
		GFL_PROC_SysCallProc(FS_OVERLAY_ID(title), &CorpProcData, NULL);
		*seq = 1;
		break;
	case 1:
		GFL_PROC_SysCallProc( FS_OVERLAY_ID(command_demo), &COMMANDDEMO_ProcData, &cdemo_data );
		*seq = 2;
		break;
	case 2:
		GFL_PROC_SysCallProc(FS_OVERLAY_ID(title), &TitleProcData, NULL);
		return GFL_PROC_RES_FINISH;
	}
	return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT TitleControlProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	return GFL_PROC_RES_FINISH;
}
