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


//==============================================================================
//	定数定義
//==============================================================================
#define MYDWC_HEAPSIZE		0x20000

///GDSプロック制御が使用するヒープサイズ
#define GDSPROC_HEAP_SIZE		(0x800)  //(MYDWC_HEAPSIZE + 0x8000)

//==============================================================================
//	グローバル変数
//==============================================================================
#if GDS_FIX
static NNSFndHeapHandle _wtHeapHandle;
#endif

//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT GdsMainProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GdsMainProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GdsMainProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static void GdsMain_CommInitialize(GDSPROC_MAIN_WORK *gmw);
static void GdsMain_CommFree(GDSPROC_MAIN_WORK *gmw);
#if GDS_FIX
static void *AllocFunc( DWCAllocType name, u32   size, int align );
static void FreeFunc(DWCAllocType name, void* ptr,  u32 size);
#endif
static void _NetInitCallback( void *work );

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

	PMSND_PlayBGM(SEQ_BGM_WIN1);

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
		SEQ_INIT_DPW_WAIT,
		
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
	
	switch(*seq){
	case SEQ_INIT_DPW:	//通信ライブラリ初期化
	#if GDS_FIX
		GdsMain_CommInitialize(gmw);
	#endif
		*seq = SEQ_INIT_DPW_WAIT;
		break;
	case SEQ_INIT_DPW_WAIT:
		if(1){//gmw->net_init == TRUE){//if(CommIsVRAMDInitialize()){
		#if GDS_FIX
			_wtHeapHandle = gmw->heapHandle;
	  #endif
	  
			// wifiメモリ管理関数呼び出し
			//DWC_SetMemFunc( AllocFunc, FreeFunc );
			
			gmw->comm_initialize_ok = TRUE;
			(*seq)++;
		}
		break;
		
	case SEQ_WIFI_CONNECT:	//WIFI接続
	#if GDS_FIX
		GFL_PROC_LOCAL_CallProc(gmw->proc_sys, NO_OVERLAY_ID, &GdsConnectProcData, gmw);
	#else
	  {
      gmw->login_param.gamedata = gmw->proc_param->gamedata;
      gmw->login_param.bg       = WIFILOGIN_BG_NORMAL;
      gmw->login_param.display  = WIFILOGIN_DISPLAY_UP;
      gmw->login_param.pSvl = &gmw->aSVL;
      gmw->login_param.mode = WIFILOGIN_MODE_NORMAL;
      GFL_PROC_LOCAL_CallProc(
        gmw->proc_sys, FS_OVERLAY_ID(wifi_login), &WiFiLogin_ProcData, &gmw->login_param);
    }
	#endif
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
		#if 0 //※check 新バトルレコーダー用の呼び出しに変える 2009.11.09(月)
			const GFL_PROC_DATA *br_proc;
			
			br_proc = BattleRecoder_ProcDataGet(gmw->proc_param->gds_mode);
  		GFL_PROC_LOCAL_CallProc(gmw->proc_sys, NO_OVERLAY_ID, &br_proc, gmw->proc_param->fsys);
		#else
		  GFL_PROC_LOCAL_CallProc(
		    gmw->proc_sys, FS_OVERLAY_ID(gds_debug), &GdsTestProcData, gmw->proc_param->gamedata);
		#endif
			(*seq)++;
		}
		break;
	case SEQ_BATTLE_RECORDER_MAIN:
    if(proc_status == GFL_PROC_MAIN_NULL){
			(*seq)++;
		}
		break;
	
	case SEQ_WIFI_CLEANUP:		//WIFI切断
	#if GDS_FIX
		GFL_PROC_LOCAL_CallProc(gmw->proc_sys, NO_OVERLAY_ID, &GdsConnectProcData, gmw);
	#else
	  {
      gmw->logout_param.gamedata = gmw->proc_param->gamedata;
      gmw->logout_param.bg = WIFILOGIN_BG_NORMAL;
      gmw->logout_param.display = WIFILOGIN_DISPLAY_UP;
      GFL_PROC_LOCAL_CallProc(
        gmw->proc_sys, FS_OVERLAY_ID(wifi_login), &WiFiLogout_ProcData, &gmw->logout_param);
    }
	#endif
		(*seq)++;
		break;
	case SEQ_WIFI_CLEANUP_MAIN:
    if(proc_status == GFL_PROC_MAIN_NULL){
			gmw->connect_success = FALSE;
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

#if GDS_FIX
	GdsMain_CommFree(gmw);
#endif

	//GDSプロック呼び出しパラメータ解放
	GFL_HEAP_FreeMemory(gmw->proc_param);
	
	GFL_PROC_FreeWork( proc );				// PROCワーク開放

	GFL_HEAP_DeleteHeap( HEAPID_GDS_MAIN );

	return GFL_PROC_RES_FINISH;
}


//--------------------------------------------------------------
/**
 * @brief   通信ライブラリ関連の初期化処理
 *
 * @param   gmw		
 */
//--------------------------------------------------------------
static void GdsMain_CommInitialize(GDSPROC_MAIN_WORK *gmw)
{
	if(gmw->comm_initialize_ok == FALSE){
		OS_TPrintf("Comm初期化開始\n");
		
		//世界交換のWifi通信命令を使用するためオーバーレイを読み出す(dpw_tr.c等)
//		Overlay_Load(FS_OVERLAY_ID(worldtrade), OVERLAY_LOAD_NOT_SYNCHRONIZE);

#if GDS_FIX
		// DWCライブラリ（Wifi）に渡すためのワーク領域を確保
		gmw->heapPtr    = GFL_HEAP_AllocMemory(HEAPID_GDS_MAIN, MYDWC_HEAPSIZE + 32);
		gmw->heapHandle = NNS_FndCreateExpHeap( (void *)( ((u32)gmw->heapPtr + 31) / 32 * 32 ), MYDWC_HEAPSIZE);
#endif

	#if 0
		//DWCオーバーレイ読み込み
		DwcOverlayStart();
		DpwCommonOverlayStart();
		// イクニューモン転送
		CommVRAMDInitialize();
  #else
    {
      //WIFIのオーバーレイを起動させるだけなので、
      //ほぼデータなし
      static const GFLNetInitializeStruct net_init =
      {
        NULL,
        0,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
#if GFL_NET_WIFI
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        MYDWC_HEAPSIZE,
        TRUE,         //デバッグ用サーバにつなぐかどうか
#endif  //GFL_NET_WIFI
        SYASHI_NETWORK_GGID,  //ggid  
        GFL_HEAPID_APP,
        HEAPID_NETWORK,
        HEAPID_WIFI,
        HEAPID_IRC,
        GFL_WICON_POSX,GFL_WICON_POSY,        // 通信アイコンXY位置
        2,     // 最大接続人数
        32,  //最大送信バイト数
        4,    // 最大ビーコン収集数
        TRUE,
        FALSE,
        GFL_NET_TYPE_WIFI_GTS,
        TRUE,
        WB_NET_GDS,
#if GFL_NET_IRC
        IRC_TIMEOUT_STANDARD,
#endif  //GFL_NET_IRC
        0,//MP親最大サイズ 512まで
        0,//dummy
      };
      GFL_NET_Init( &net_init, _NetInitCallback, gmw );
    }
  #endif
  
		OS_TPrintf("Comm初期化終了\n");
	}
}

//--------------------------------------------------------------
/**
 * @brief   通信ライブラリ関連の解放処理
 *
 * @param   gmw		
 */
//--------------------------------------------------------------
static void GdsMain_CommFree(GDSPROC_MAIN_WORK *gmw)
{
	if(gmw->comm_initialize_ok == TRUE){
		OS_TPrintf("Comm解放開始\n");
		
#if GDS_FIX
		NNS_FndDestroyExpHeap(gmw->heapHandle);
		GFL_HEAP_FreeMemory( gmw->heapPtr );
#endif
	#if 0
		DpwCommonOverlayEnd();
		DwcOverlayEnd();

		// イクニューモン解放
		CommVRAMDFinalize();
  #else
    GFL_NET_Exit(NULL); //WiFiのオーバーレイ解放
    gmw->net_init = FALSE;
  #endif
  
//		Overlay_UnloadID(FS_OVERLAY_ID(worldtrade));
		
		gmw->comm_initialize_ok = FALSE;

		OS_TPrintf("Comm解放完了\n");
	}
}

//----------------------------------------------------------------------------
/**
 *  @brief  netの初期化終了コールバック
 *
 *  @param  void *wk_adrs   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void _NetInitCallback( void *work )
{
  GDSPROC_MAIN_WORK *gmw = work;
  gmw->net_init = TRUE;
}

#if GDS_FIX
/*---------------------------------------------------------------------------*
  メモリ確保関数
 *---------------------------------------------------------------------------*/
static void *AllocFunc( DWCAllocType name, u32   size, int align )
{
#pragma unused( name )
    void * ptr;
    OSIntrMode old;
    old = OS_DisableInterrupts();
    ptr = NNS_FndAllocFromExpHeapEx( _wtHeapHandle, size, align );
    OS_RestoreInterrupts( old );
    if(ptr == NULL){
	}
	
    return ptr;
}

/*---------------------------------------------------------------------------*
  メモリ開放関数
 *---------------------------------------------------------------------------*/
static void FreeFunc(DWCAllocType name, void* ptr,  u32 size)
{
#pragma unused( name, size )
    OSIntrMode old;

    if ( !ptr ) return;
    old = OS_DisableInterrupts();
    NNS_FndFreeToExpHeap( _wtHeapHandle, ptr );
    OS_RestoreInterrupts( old );
}
#endif

