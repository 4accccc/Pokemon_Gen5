//==============================================================================
/**
 * @file    monolith_mission_select.c
 * @brief   モノリス：ミッション説明
 * @author  matsuda
 * @date    2009.11.20(金)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "monolith_common.h"
#include "arc_def.h"


//==============================================================================
//  構造体定義
//==============================================================================
///ミッション説明画面制御ワーク
typedef struct{
  u32 work;
}MISSION_SELECT_WORK;

//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT MissionSelectProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT MissionSelectProc_Main(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT MissionSelectProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk);


//==============================================================================
//  データ
//==============================================================================
///モノリスパワーROCデータ
const GFL_PROC_DATA MonolithAppProc_Down_MissionSelect = {
  MissionSelectProc_Init,
  MissionSelectProc_Main,
  MissionSelectProc_End,
};



//==============================================================================
//
//  
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   プロセス関数：初期化
 *
 * @param   proc		プロセスデータ
 * @param   seq			シーケンス
 *
 * @retval  処理状況
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT MissionSelectProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MISSION_SELECT_WORK *msw = mywk;
	ARCHANDLE *hdl;
  
  msw = GFL_PROC_AllocWork(proc, sizeof(MISSION_SELECT_WORK), HEAPID_MONOLITH);
  GFL_STD_MemClear(msw, sizeof(MISSION_SELECT_WORK));
  
	//ファイルオープン
	hdl = GFL_ARC_OpenDataHandle(ARCID_MONOLITH, HEAPID_MONOLITH);
  {
    ;
  }
	//ファイルクローズ
	GFL_ARC_CloseDataHandle(hdl);

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
/**
 * @brief   プロセス関数：メイン
 *
 * @param   proc		プロセスデータ
 * @param   seq			シーケンス
 *
 * @retval  処理状況
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT MissionSelectProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MISSION_SELECT_WORK *msw = mywk;

  switch(*seq){
  case 0:
    return GFL_PROC_RES_FINISH;
  }
  
  return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   プロセス関数：終了
 *
 * @param   proc		プロセスデータ
 * @param   seq			シーケンス
 *
 * @retval  処理状況
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT MissionSelectProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MISSION_SELECT_WORK *msw = mywk;
  
  GFL_PROC_FreeWork(proc);
  return GFL_PROC_RES_FINISH;
}
