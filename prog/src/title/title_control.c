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
//  enum定義
//==============================================================================
enum{
  TC_SEQ_CORPLOGO=0,
  TC_SEQ_GFLOGO,
  TC_SEQ_DEMOMOVIE,
  TC_SEQ_TITLE,
  
};
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
  case TC_SEQ_CORPLOGO:   // 社名表示
#ifdef DEBUG_ONLY_FOR_hudson
    if( HUDSON_IsSkipTitle() )
    {
      // HUDSONで実行した場合は直でフィールドへ
      CorpParam.ret = CORPORATE_RET_DEBUG;
      *seq = TC_SEQ_TITLE;
    }
    else
#endif
    {
      GFL_PROC_SysCallProc( FS_OVERLAY_ID(title), &CorpProcData, &CorpParam );
      *seq = TC_SEQ_GFLOGO; // ムービーに移行
    }
    break;

  case TC_SEQ_GFLOGO:   // ＧＦロゴ
    CorpParam.mode = ONLY_DEVELOP_VIEW;     // 2回目以降の権利者表示モード設定
    if( CorpParam.ret == CORPORATE_RET_NORMAL ){
      cdemo_data.mode = COMMANDDEMO_MODE_GF_LOGO;
      cdemo_data.skip = COMMANDDEMO_SKIP_DEBUG;
      cdemo_data.ret  = COMMANDDEMO_RET_NORMAL;
      GFL_PROC_SysCallProc( FS_OVERLAY_ID(command_demo), &COMMANDDEMO_ProcData, &cdemo_data );
      *seq = TC_SEQ_DEMOMOVIE;   // デモムービーへ
    }else{
      *seq = TC_SEQ_TITLE;   // タイトルへ
    }
    break;

  case TC_SEQ_DEMOMOVIE:   // デモムービー
    if( cdemo_data.ret != COMMANDDEMO_RET_SKIP_DEBUG ){
      cdemo_data.mode = COMMANDDEMO_MODE_OP_MOVIE2;
      cdemo_data.ret  = COMMANDDEMO_RET_NORMAL;
      GFL_PROC_SysCallProc( FS_OVERLAY_ID(command_demo), &COMMANDDEMO_ProcData, &cdemo_data );
    }
    *seq = TC_SEQ_TITLE;   // タイトルへ
    break;

  case TC_SEQ_TITLE:   // メインタイトル
    if( cdemo_data.ret == COMMANDDEMO_RET_SKIP_DEBUG ){
      CorpParam.ret = CORPORATE_RET_DEBUG;
    }
    GFL_PROC_SysCallProc( FS_OVERLAY_ID(title), &TitleProcData, &CorpParam.ret );
    *seq = TC_SEQ_CORPLOGO;
    break;
  }
#else
  switch( *seq  ){
  case TC_SEQ_CORPLOGO:   // 社名表示
    GFL_PROC_SysCallProc( FS_OVERLAY_ID(title), &CorpProcData, &CorpParam );
    *seq = 1;
    break;

  case TC_SEQ_GFLOGO:   // ＧＦロゴ
    CorpParam.mode = ONLY_DEVELOP_VIEW;     // 2回目以降の権利者表示モード設定
    cdemo_data.mode = COMMANDDEMO_MODE_GF_LOGO;
    cdemo_data.skip = COMMANDDEMO_SKIP_ON;
    cdemo_data.ret  = COMMANDDEMO_RET_NORMAL;
    GFL_PROC_SysCallProc( FS_OVERLAY_ID(command_demo), &COMMANDDEMO_ProcData, &cdemo_data );
    *seq = TC_SEQ_DEMOMOVIE;
    break;

  case TC_SEQ_DEMOMOVIE:   // デモムービー
    cdemo_data.mode = COMMANDDEMO_MODE_OP_MOVIE2;
    cdemo_data.ret  = COMMANDDEMO_RET_NORMAL;
    GFL_PROC_SysCallProc( FS_OVERLAY_ID(command_demo), &COMMANDDEMO_ProcData, &cdemo_data );
    *seq = TC_SEQ_TITLE;   // タイトルへ
    break;

  case TC_SEQ_TITLE:   // メインタイトル
    GFL_PROC_SysCallProc( FS_OVERLAY_ID(title), &TitleProcData, NULL );
    *seq = TC_SEQ_CORPLOGO;
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
