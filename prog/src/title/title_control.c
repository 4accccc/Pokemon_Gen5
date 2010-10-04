//==============================================================================
/**
 * @file  title_control.c
 * @brief タイトルのPROC制御
 * @author  matsuda
 * @date  2008.12.05(金)
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
//  プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT TitleControlProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT TitleControlProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT TitleControlProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

//==============================================================================
//  データ
//==============================================================================

const GFL_PROC_DATA TitleControlProcData = {
  TitleControlProcInit,
  TitleControlProcMain,
  TitleControlProcEnd,
};

static CORP_PARAM  CorpParam={0,0};   // 社名表示処理ワーク
static COMMANDDEMO_DATA cdemo_data;   // デモ処理データ


extern const GFL_PROC_DATA HaisinControlProcData;
FS_EXTERN_OVERLAY(matsuda_debug);


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

#include "debug/debug_hudson.h"
//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT TitleControlProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
#ifdef PM_DEBUG
  switch( *seq  ){
  case 0:   // 社名表示
#ifdef DEBUG_ONLY_FOR_hudson
    if( HUDSON_IsSkipTitle() )
    {
      // HUDSONで実行した場合は直でフィールドへ
      CorpParam.ret = CORPORATE_RET_DEBUG;
      *seq = 4;
    }
    else
#endif
    {
//      GFL_PROC_SysCallProc( FS_OVERLAY_ID(title), &CorpProcData, &CorpParam );
      GFL_PROC_SysCallProc( FS_OVERLAY_ID(matsuda_debug), &HaisinControlProcData, NULL );
      *seq = 1;
    }
    break;

  case 1:   // ＧＦロゴ
    CorpParam.mode = ONLY_DEVELOP_VIEW;     // 2回目以降の権利者表示モード設定
    if( CorpParam.ret == CORPORATE_RET_NORMAL ){
      cdemo_data.mode = COMMANDDEMO_MODE_GF_LOGO;
      cdemo_data.skip = COMMANDDEMO_SKIP_DEBUG;
      cdemo_data.ret  = COMMANDDEMO_RET_NORMAL;
      GFL_PROC_SysCallProc( FS_OVERLAY_ID(command_demo), &COMMANDDEMO_ProcData, &cdemo_data );
      *seq = 2;
    }else{
      *seq = 4;   // タイトルへ
    }
    break;

  case 2:		// デモ１
    if( cdemo_data.ret != COMMANDDEMO_RET_SKIP_DEBUG ){
      cdemo_data.mode = COMMANDDEMO_MODE_OP_MOVIE1;
      cdemo_data.ret  = COMMANDDEMO_RET_NORMAL;
      GFL_PROC_SysCallProc( FS_OVERLAY_ID(command_demo), &COMMANDDEMO_ProcData, &cdemo_data );
      *seq = 3;
    }else{
      *seq = 4;   // タイトルへ
    }
    break;

  case 3:		// デモ２
    if( cdemo_data.ret != COMMANDDEMO_RET_SKIP_DEBUG ){
      cdemo_data.mode = COMMANDDEMO_MODE_OP_MOVIE2;
      cdemo_data.ret  = COMMANDDEMO_RET_NORMAL;
      GFL_PROC_SysCallProc( FS_OVERLAY_ID(command_demo), &COMMANDDEMO_ProcData, &cdemo_data );
    }
    *seq = 4;   // タイトルへ
    break;

  case 4:   // メインタイトル
    if( cdemo_data.ret == COMMANDDEMO_RET_SKIP_DEBUG ){
      CorpParam.ret = CORPORATE_RET_DEBUG;
    }
    GFL_PROC_SysCallProc( FS_OVERLAY_ID(title), &TitleProcData, &CorpParam.ret );
    *seq = 0;
    break;
  }
#else
  switch( *seq  ){
  case 0:   // 社名表示
    GFL_PROC_SysCallProc( FS_OVERLAY_ID(title), &CorpProcData, &CorpParam );
    *seq = 1;
    break;

  case 1:   // ＧＦロゴ
		CorpParam.mode = ONLY_DEVELOP_VIEW;     // 2回目以降の権利者表示モード設定
    cdemo_data.mode = COMMANDDEMO_MODE_GF_LOGO;
    cdemo_data.skip = COMMANDDEMO_SKIP_ON;
    cdemo_data.ret  = COMMANDDEMO_RET_NORMAL;
    GFL_PROC_SysCallProc( FS_OVERLAY_ID(command_demo), &COMMANDDEMO_ProcData, &cdemo_data );
    *seq = 2;
    break;

  case 2:		// デモ１
		if( cdemo_data.ret == COMMANDDEMO_RET_NORMAL ){
			cdemo_data.mode = COMMANDDEMO_MODE_OP_MOVIE1;
			cdemo_data.ret  = COMMANDDEMO_RET_NORMAL;
			GFL_PROC_SysCallProc( FS_OVERLAY_ID(command_demo), &COMMANDDEMO_ProcData, &cdemo_data );
			*seq = 3;
		}else{
			*seq = 4;   // タイトルへ
		}
		break;

  case 3:		// デモ２
		cdemo_data.mode = COMMANDDEMO_MODE_OP_MOVIE2;
		cdemo_data.ret  = COMMANDDEMO_RET_NORMAL;
		GFL_PROC_SysCallProc( FS_OVERLAY_ID(command_demo), &COMMANDDEMO_ProcData, &cdemo_data );
		*seq = 4;   // タイトルへ
		break;

  case 4:   // メインタイトル
    GFL_PROC_SysCallProc( FS_OVERLAY_ID(title), &TitleProcData, NULL );
    *seq = 0;
    break;
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
  return GFL_PROC_RES_FINISH;
}
