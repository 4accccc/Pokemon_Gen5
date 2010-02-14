//============================================================================================
/**
 * @file	worldtrade_status.c
 * @brief	世界交換ポケモンステータス画面呼び出し・復帰
 * @author	Akito Mori
 * @date	06.05.10
 */
//============================================================================================
#include <gflib.h>
#include <dwc.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "arc_def.h"
#include <dpw_tr.h>
#include "print/wordset.h"
#include "message.naix"
#include "system/wipe.h"
#include "system/bmp_menu.h"
#include "sound/pm_sndsys.h"
#include "savedata/wifilist.h"
#include "savedata/worldtrade_data.h"
#include "poke_tool/pokeparty.h"
#include "savedata/box_savedata.h"
#include "item/itemsym.h"

#include "net_app/worldtrade.h"
#include "worldtrade_local.h"

#include "app/p_status.h"

#include "savedata/mystatus.h"


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
/*** 関数プロトタイプ ***/
//static void InitWork( WORLDTRADE_WORK *wk );
//static void FreeWork( WORLDTRADE_WORK *wk );



// ステータス画面で表示するフラグの列（ビットテーブルにすればいいのに…）
/*
static const u8 StatusPageTable[]={
	PST_PAGE_INFO_MEMO,		// 「ポケモンじょうほう」「トレーナーメモ」
	PST_PAGE_PARAM_B_SKILL,	// 「ポケモンのうりょく」「わざ説明」
	PST_PAGE_RIBBON,		// 「きねんリボン」
	PST_PAGE_RET,			// 「もどる」
	PST_PAGE_MAX
};
*/
//============================================================================================
//	プロセス関数
//============================================================================================
FS_EXTERN_OVERLAY( poke_status );

//==============================================================================
/**
 * @brief   世界交換入り口画面初期化
 *
 * @param   wk		
 * @param   seq		
 *
 * @retval  int		
 */
//==============================================================================
int WorldTrade_Status_Init(WORLDTRADE_WORK *wk, int seq)
{
	PSTATUS_DATA	 *statusParam;						///< スタータス呼び出し用パラメータ

	WorldTrade_ExitSystem( wk );

	MORI_PRINT( "heap remain RAM = %d\n", GFL_HEAP_GetHeapFreeSize( HEAPID_WORLDTRADE ));
	
  wk->sub_proc_wk = GFL_HEAP_AllocMemory( HEAPID_WORLDTRADE, sizeof(PSTATUS_DATA) );
  GFL_STD_MemClear( wk->sub_proc_wk, sizeof(PSTATUS_DATA) );
  statusParam = wk->sub_proc_wk;

	statusParam->ppd  = WorldTrade_GetPokePtr(wk->param->myparty, wk->param->mybox, wk->BoxTrayNo, wk->BoxCursorPos);
	statusParam->cfg	 = wk->param->config;
	statusParam->game_data	 = GAMESYSTEM_GetGameData( wk->param->gamesys );

	statusParam->player_name	= MyStatus_GetMyName( wk->param->mystatus );
	statusParam->player_id		= MyStatus_GetID( wk->param->mystatus );
	statusParam->player_sex	=  MyStatus_GetMySex( wk->param->mystatus );

	statusParam->ppt  =PST_PP_TYPE_POKEPARAM;
	statusParam->max  = 1;
	statusParam->pos  = 0;
	statusParam->mode = PST_MODE_NO_WAZACHG;	// 技入れ替え禁止にする
	statusParam->waza = 0;
  statusParam->page = PPT_INFO;
	statusParam->zukan_mode = wk->param->zukanmode;

	GAMESYSTEM_CallProc( wk->param->gamesys,
		FS_OVERLAY_ID(poke_status), &PokeStatus_ProcData, statusParam );

	wk->subprocflag = 1;

	return SEQ_FADEIN;
}
//==============================================================================
/**
 * @brief   世界交換入り口画面メイン
 *
 * @param   wk		
 * @param   seq		
 *
 * @retval  int		
 */
//==============================================================================
int WorldTrade_Status_Main(WORLDTRADE_WORK *wk, int seq)
{
	int ret=SEQ_MAIN;

		if(1){	
			WorldTrade_SubProcessChange( wk, WORLDTRADE_MYBOX, wk->sub_process_mode );
			ret = SEQ_FADEOUT;
		}

	return ret;
}


//==============================================================================
/**
 * @brief   世界交換入り口画面終了
 *
 * @param   wk		
 * @param   seq		
 *
 * @retval  int		
 */
//==============================================================================
int WorldTrade_Status_End(WORLDTRADE_WORK *wk, int seq)
{
  GFL_HEAP_FreeMemory( wk->sub_proc_wk );

	WorldTrade_InitSystem( wk );


	// ボックス画面に戻る
	WorldTrade_SubProcessUpdate( wk );

	return SEQ_INIT;
}
