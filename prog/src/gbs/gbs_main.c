//============================================================================================
/**
 * @file	  gbs_main.c
 * @brief	  グローバルバトルステーション
 * @author	k.ohno
 * @date	  09.08.27
 */
//============================================================================================
#include <gflib.h>
#include "arc_def.h"
#include "net/network_define.h"
#include "savedata/save_tbl.h"



//--------------------------------------------------------------------------------------------
/**
 * @brief	  グローバルバトルステーションプロセス：初期化
 * @param  	proc	プロセスへのポインタ
 * @param	  seq		シーケンス用ワークへのポインタ
 * @param	  pwk		親のワーク
 * @param	  mywk	自分のワーク
 * @return	PROC_RES_CONTINUE	動作継続中
 * @return	PROC_RES_FINISH		動作終了
 */
//--------------------------------------------------------------------------------------------
static GFL_PROC_RESULT GlobalBattleStationProc_Init(GFL_PROC *proc, int * seq, void *pwk, void *mywk )
{
	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	  グローバルバトルステーションプロセス：メイン
 * @param  	proc	プロセスへのポインタ
 * @param	  seq		シーケンス用ワークへのポインタ
 * @param	  pwk		親のワーク
 * @param	  mywk	自分のワーク
 * @return	PROC_RES_CONTINUE	動作継続中
 * @return	PROC_RES_FINISH		動作終了
 */
//--------------------------------------------------------------------------------------------
static GFL_PROC_RESULT GlobalBattleStationProc_Main(GFL_PROC *proc, int * seq, void *pwk, void *mywk )
{
	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	  グローバルバトルステーションプロセス：終了
 * @param  	proc	プロセスへのポインタ
 * @param	  seq		シーケンス用ワークへのポインタ
 * @param	  pwk		親のワーク
 * @param	  mywk	自分のワーク
 * @return	PROC_RES_CONTINUE	動作継続中
 * @return	PROC_RES_FINISH		動作終了
 */
//--------------------------------------------------------------------------------------------
static GFL_PROC_RESULT GlobalBattleStationProc_End(GFL_PROC *proc, int * seq, void *pwk, void *mywk )
{
	return GFL_PROC_RES_FINISH;
}






//------------------------------------------------------------------
///	プロセスデータ
//------------------------------------------------------------------
const GFL_PROC_DATA GlobalBattleStationProcData = {
	GlobalBattleStationProc_Init,
	GlobalBattleStationProc_Main,
	GlobalBattleStationProc_End,
};
