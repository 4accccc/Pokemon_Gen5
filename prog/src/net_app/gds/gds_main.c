//==============================================================================
/**
 * @file	gds_main.c
 * @brief	GDSモード：PROC制御
 * @author	matsuda
 * @date	2008.01.17(木)
 */
//==============================================================================
#include <gflib.h>
#include <dwc.h>
#include "system/main.h"
#include "net/dwc_rap.h"
#include "savedata\save_control.h"
#include "pm_define.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/poke_tool_def.h"
#include "savedata/gds_profile.h"

#include "message.naix"

#include "gds_rap.h"
#include "gds_rap_response.h"

#include "net_app/gds_main.h"

#include "net\network_define.h"
#include "sound/pm_sndsys.h"

#include "gds_test.h"
#include "system/net_err.h"


#ifdef PM_DEBUG
//#define DEBUG_GDS
#endif 

//==============================================================================
//	定数定義
//==============================================================================
#define MYDWC_HEAPSIZE		0x20000

///GDSプロック制御が使用するヒープサイズ
#define GDSPROC_HEAP_SIZE		(0x800)  //(MYDWC_HEAPSIZE + 0x8000)

//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT GdsMainProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GdsMainProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GdsMainProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

//==============================================================================
//	データ
//==============================================================================
///GDSメインプロセス定義データ
const GFL_PROC_DATA GdsMainProcData = {
	GdsMainProc_Init,
	GdsMainProc_Main,
	GdsMainProc_End,
};

//--------------------------------------------------------------
//  デバッグ
//--------------------------------------------------------------
FS_EXTERN_OVERLAY(gds_debug);
FS_EXTERN_OVERLAY(gds_comm);



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
static GFL_PROC_RESULT GdsMainProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GDSPROC_MAIN_WORK *gmw;
	
	//sys_HBlankIntrStop();	//HBlank割り込み停止

	GFL_DISP_GX_SetVisibleControlDirect(0);
	GFL_DISP_GXS_SetVisibleControlDirect(0);
	GX_SetVisiblePlane(0);
	GXS_SetVisiblePlane(0);
	GX_SetVisibleWnd(GX_WNDMASK_NONE);
	GXS_SetVisibleWnd(GX_WNDMASK_NONE);
	G2_BlendNone();
	G2S_BlendNone();

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_GDS_MAIN, GDSPROC_HEAP_SIZE );

	gmw = GFL_PROC_AllocWork(proc, sizeof(GDSPROC_MAIN_WORK), HEAPID_GDS_MAIN );
	GFL_STD_MemClear(gmw, sizeof(GDSPROC_MAIN_WORK));
	gmw->proc_param = pwk;
	
  gmw->proc_sys = GFL_PROC_LOCAL_boot(HEAPID_GDS_MAIN);

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
static GFL_PROC_RESULT GdsMainProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GDSPROC_MAIN_WORK * gmw  = mywk;
	enum{
		SEQ_INIT_DPW,
		
		SEQ_WIFI_CONNECT,
		SEQ_WIFI_CONNECT_MAIN,
		
		SEQ_BATTLE_RECORDER,
		SEQ_BATTLE_RECORDER_MAIN,
		
		SEQ_WIFI_CLEANUP,
		SEQ_WIFI_CLEANUP_MAIN,
		
		SEQ_END,
	};
  GFL_PROC_MAIN_STATUS proc_status;
  
  proc_status = GFL_PROC_LOCAL_Main(gmw->proc_sys);

  //エラー検知
  //gamesystem_main上のLOCAL_PROCで動くものしか検知してくれないので、
  //自分で動かす必要がある
  if( proc_status == GFL_PROC_MAIN_CHANGE || proc_status == GFL_PROC_MAIN_NULL )
  { 
    NetErr_DispCall(FALSE);
  }

	
	switch(*seq){
	case SEQ_INIT_DPW:	//通信ライブラリ初期化
		(*seq)++;
		break;
		
	case SEQ_WIFI_CONNECT:	//WIFI接続
	  {
      GFL_STD_MemClear( &gmw->login_param, sizeof(WIFILOGIN_PARAM) );
      gmw->login_param.gamedata = gmw->proc_param->gamedata;
      gmw->login_param.bg       = WIFILOGIN_BG_NORMAL;
      gmw->login_param.display  = WIFILOGIN_DISPLAY_UP;
      gmw->login_param.pSvl = &gmw->aSvl;
      gmw->login_param.nsid = WB_NET_GDS;


      if( gmw->br_param.result == BR_RESULT_NET_ERROR )
      { 
        gmw->login_param.mode = WIFILOGIN_MODE_ERROR;
      }
      else
      { 
        gmw->login_param.mode = WIFILOGIN_MODE_NORMAL;
      }
      GFL_PROC_LOCAL_CallProc(
        gmw->proc_sys, FS_OVERLAY_ID(wifi_login), &WiFiLogin_ProcData, &gmw->login_param);
    }
		(*seq)++;
		break;
	case SEQ_WIFI_CONNECT_MAIN:
    if(proc_status == GFL_PROC_MAIN_NULL){
      if(gmw->login_param.result == WIFILOGIN_RESULT_LOGIN){
				gmw->connect_success = TRUE;
				(*seq)++;
			}
			else{
				*seq = SEQ_END;
			}
		}
		break;
	
	case SEQ_BATTLE_RECORDER:	//バトルレコーダー(GDSモード)
		{
		#ifndef DEBUG_GDS
      GFL_STD_MemClear( &gmw->br_param, sizeof(BATTLERECORDER_PARAM) );
      gmw->br_param.mode        = gmw->proc_param->gds_mode;
      gmw->br_param.p_gamedata  = gmw->proc_param->gamedata;
      gmw->br_param.p_svl       = &gmw->aSvl;
  		GFL_PROC_LOCAL_CallProc(
        gmw->proc_sys, FS_OVERLAY_ID(battle_recorder), &BattleRecorder_ProcData, &gmw->br_param);
		#else
		  gmw->gds_test_parent.gamedata = gmw->proc_param->gamedata;
		  gmw->gds_test_parent.pSvl = &gmw->aSvl;
      GFL_OVERLAY_Load( FS_OVERLAY_ID(gds_comm) );
		  GFL_PROC_LOCAL_CallProc(
		    gmw->proc_sys, FS_OVERLAY_ID(gds_debug), &GdsTestProcData, &gmw->gds_test_parent);

		#endif
			(*seq)++;
		}
		break;
	case SEQ_BATTLE_RECORDER_MAIN:
    if(proc_status == GFL_PROC_MAIN_NULL){
#ifdef DEBUG_GDS
      GFL_OVERLAY_Unload( FS_OVERLAY_ID(gds_comm) );
#else
#endif
      //エラーならばWIFILOGINへ復帰
      if( gmw->br_param.result == BR_RESULT_NET_ERROR )
      { 
        *seq  = SEQ_WIFI_CONNECT;
      }
      else
      { 
        (*seq)++;
      }
		}
		break;
	
	case SEQ_WIFI_CLEANUP:		//WIFI切断
	  {
      GFL_STD_MemClear( &gmw->logout_param, sizeof(WIFILOGOUT_PARAM) );
      gmw->logout_param.gamedata = gmw->proc_param->gamedata;
      gmw->logout_param.bg = WIFILOGIN_BG_NORMAL;
      gmw->logout_param.display = WIFILOGIN_DISPLAY_UP;
      GFL_PROC_LOCAL_CallProc(
        gmw->proc_sys, FS_OVERLAY_ID(wifi_login), &WiFiLogout_ProcData, &gmw->logout_param);
    }
  	gmw->connect_success = FALSE;
		(*seq)++;
		break;
	case SEQ_WIFI_CLEANUP_MAIN:
    if(proc_status == GFL_PROC_MAIN_NULL){
			(*seq)++;
		}
		break;

	case SEQ_END:
		return GFL_PROC_RES_FINISH;
	}

	if(gmw->connect_success == TRUE){
		// 受信強度リンクを反映させる
		DWC_UpdateConnection();
		// Dpw_Tr_Main() だけは例外的にいつでも呼べる
	//	Dpw_Tr_Main();
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
static GFL_PROC_RESULT GdsMainProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GDSPROC_MAIN_WORK * gmw  = mywk;

  GFL_PROC_LOCAL_Exit(gmw->proc_sys);

	GFL_PROC_FreeWork( proc );				// PROCワーク開放

	GFL_HEAP_DeleteHeap( HEAPID_GDS_MAIN );

	return GFL_PROC_RES_FINISH;
}

