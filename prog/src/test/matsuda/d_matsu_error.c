//==============================================================================
/**
 * @file	d_matsu_error.c
 * @brief	通信エラー画面テスト
 * @author	matsuda
 * @date	2008.11.14(金)
 */
//==============================================================================
#include <gflib.h>
#include <procsys.h>
#include <tcbl.h>
#include "system\main.h"
#include <backup_system.h>
//#include "savedata/contest_savedata.h"
#include "savedata/save_tbl.h"
#include "savedata/save_control.h"

#include "net\network_define.h"

//#include "test/goto/comm_error.h"


//==============================================================================
//	構造体定義
//==============================================================================
typedef struct {

	u16		seq;
	HEAPID	heapID;
	int debug_mode;
	
	//セーブ関連
	GFL_SAVEDATA *sv_normal;	///<ノーマルセーブデータへのポインタ
	
	//通信
	BOOL connect_ok;
	int send_key;
	
	u8 huge_data[0x1000];
	u8 receive_huge_data[2][0x1000];
	volatile u16 cs;
}D_MATSU_WORK;


//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static BOOL DebugMatsuda_WiressTest(D_MATSU_WORK *wk);
static void* _netBeaconGetFunc(void* pWork);
static int _netBeaconGetSizeFunc(void* pWork);
static BOOL _netBeaconCompFunc(GameServiceID myNo,GameServiceID beaconNo);
static void _initCallBack(void* pWork);
static void _connectCallBack(void* pWork);
static void _endCallBack(void* pWork);
static void _RecvMoveData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _RecvHugeData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _RecvKeyData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static u8 * _RecvHugeBuffer(int netID, void* pWork, int size);



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
static GFL_PROC_RESULT DebugMatsudaMainProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	static const GFL_DISP_VRAM vramBank = {
		GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
		GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
		GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
		GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
		GX_VRAM_OBJ_128_B,				// メイン2DエンジンのOBJ
		GX_VRAM_OBJEXTPLTT_0_F,			// メイン2DエンジンのOBJ拡張パレット
		GX_VRAM_SUB_OBJ_16_I,			// サブ2DエンジンのOBJ
		GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
		GX_VRAM_TEX_NONE,				// テクスチャイメージスロット
		GX_VRAM_TEXPLTT_0123_E,			// テクスチャパレットスロット
		GX_OBJVRAMMODE_CHAR_1D_128K,	// メインOBJマッピングモード
		GX_OBJVRAMMODE_CHAR_1D_32K,		// サブOBJマッピングモード
	};

	D_MATSU_WORK* wk;

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MATSUDA_DEBUG, 0x70000 );
	wk = GFL_PROC_AllocWork( proc, sizeof(D_MATSU_WORK), HEAPID_MATSUDA_DEBUG );
	MI_CpuClear8(wk, sizeof(D_MATSU_WORK));
	wk->heapID = HEAPID_MATSUDA_DEBUG;
	{
		int i;
		volatile u16 cs = 0;
		u8 maca[6];
		OS_GetMacAddress(maca);
		for(i = 0; i < 6; i++){
			OS_TPrintf("mac addrs = %x %x %x %x %x %x\n", maca[0], maca[1], maca[2], maca[3], maca[4], maca[5]);
		}
		for(i = 0; i < 0x1000; i++){
			if(i < 0x5000){
				wk->huge_data[i] = maca[1];
			}
			else{
				wk->huge_data[i] = maca[2];
			}
			cs += wk->huge_data[i];
			if(i % 0x100 == 0 || i == 0x1000-1 || i == 0x1000-2){
				OS_TPrintf("cs = %x, huge %x %x\n", cs, i, wk->huge_data[i]);
			}
		}
		wk->cs = cs;
		OS_TPrintf("巨大データチェックサム = 0x%x\n", wk->cs);
	}
	
//	GFL_ARC_Init( arcFiles, NELEMS(arcFiles) );	gfl_use.cで1回だけ初期化に変更

	GFL_DISP_SetBank( &vramBank );

	// 各種効果レジスタ初期化
	G2_BlendNone();

	// BGsystem初期化
	GFL_BG_Init( wk->heapID );
	GFL_BMPWIN_Init( wk->heapID );

	//ＢＧモード設定
	{
		static const GFL_BG_SYS_HEADER sysHeader = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_3, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &sysHeader );
	}

	// 個別フレーム設定
	{
		static const GFL_BG_BGCNT_HEADER bgcntText = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x5800, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};

		GFL_BG_SetBGControl( GFL_BG_FRAME0_M,   &bgcntText,   GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( GFL_BG_FRAME0_S,   &bgcntText,   GFL_BG_MODE_TEXT );

		GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_ON );
		GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME2_M,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME3_M,   VISIBLE_OFF );

//		GFL_BG_SetClearCharacter( GFL_BG_FRAME0_M, 0x20, 0x22, wk->heapID );
		GFL_BG_FillCharacter( GFL_BG_FRAME0_M, 0xff, 1, 0 );

//		void GFL_BG_FillScreen( u8 frmnum, u16 dat, u8 px, u8 py, u8 sx, u8 sy, u8 mode )
		GFL_BG_FillScreen( GFL_BG_FRAME0_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );

		GFL_BG_FillCharacter( GFL_BG_FRAME0_S, 0x00, 1, 0 );
		GFL_BG_FillScreen( GFL_BG_FRAME0_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
	}

	// 上下画面設定
	GX_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );

	GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );

	return GFL_PROC_RES_FINISH;
}
//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugMatsudaMainProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	D_MATSU_WORK* wk = mywk;
	BOOL ret = 0;
	
	wk->debug_mode = 0;
	switch(wk->debug_mode){
	case 0:		//ワイヤレステスト
		ret = DebugMatsuda_WiressTest(wk);
		break;
	case 1:		//赤外線テスト
		ret = DebugMatsuda_WiressTest(wk);
		break;
	default:
		ret = TRUE;
	}
	
	if(ret == TRUE){
		return GFL_PROC_RES_FINISH;
	}

	return GFL_PROC_RES_CONTINUE;
}
//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugMatsudaMainProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GFL_BG_Exit();
	GFL_BMPWIN_Exit();

	GFL_PROC_FreeWork(proc);
	GFL_HEAP_DeleteHeap(HEAPID_MATSUDA_DEBUG);
	
	return GFL_PROC_RES_FINISH;
}



//==============================================================================
//	
//==============================================================================
static const NetRecvFuncTable _CommPacketTbl[] = {
//    {_RecvMoveData,        GFL_NET_COMMAND_SIZE( 0 ), NULL},    ///NET_CMD_MOVE
//    {_RecvHugeData,        GFL_NET_COMMAND_SIZE( 0 ), NULL},    ///NET_CMD_HUGE
//    {_RecvKeyData,         GFL_NET_COMMAND_SIZE( 4 ), NULL},    ///NET_CMD_KEY
    {_RecvMoveData,         NULL},    ///NET_CMD_MOVE
    {_RecvHugeData,         _RecvHugeBuffer},    ///NET_CMD_HUGE
    {_RecvKeyData,          NULL},    ///NET_CMD_KEY
};

enum{
	NET_CMD_MOVE = GFL_NET_CMD_DEBUG_MATSUDA,
	NET_CMD_HUGE,
	NET_CMD_KEY,
};

#define _MAXNUM   2//(4)         // 最大接続人数
#define _MAXSIZE  (GFL_NET_IRC_SEND_MAX)	//(32)        // 最大送信バイト数
#define _BCON_GET_NUM (16)    // 最大ビーコン収集数


static const GFLNetInitializeStruct aGFLNetInit = {
    _CommPacketTbl,  // 受信関数テーブル
    NELEMS(_CommPacketTbl), // 受信テーブル要素数
    NULL,    ///< ハードで接続した時に呼ばれる
    NULL,    ///< ネゴシエーション完了時にコール
    NULL,   // ユーザー同士が交換するデータのポインタ取得関数
    NULL,   // ユーザー同士が交換するデータのサイズ取得関数
    _netBeaconGetFunc,  // ビーコンデータ取得関数
    _netBeaconGetSizeFunc,  // ビーコンデータサイズ取得関数
    _netBeaconCompFunc,  // ビーコンのサービスを比較して繋いで良いかどうか判断する
    NULL,            // 普通のエラーが起こった場合 通信終了
    FatalError_Disp,  // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
    _endCallBack,  // 通信切断時に呼ばれる関数
    NULL,  // オート接続で親になった場合
#if GFL_NET_WIFI
    NULL,     ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
    NULL, ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
    NULL,  ///< wifiフレンドリスト削除コールバック
    NULL,   ///< DWC形式の友達リスト	
    NULL,  ///< DWCのユーザデータ（自分のデータ）
    0,   ///< DWCへのHEAPサイズ
    TRUE,        ///< デバック用サーバにつなぐかどうか
#endif  //GFL_NET_WIFI
    0x532,//0x444,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
    GFL_HEAPID_APP,  //元になるheapid
    HEAPID_NETWORK,  //通信用にcreateされるHEAPID
    HEAPID_WIFI,  //wifi用にcreateされるHEAPID
    HEAPID_IRC,   //※check　赤外線通信用にcreateされるHEAPID
    GFL_WICON_POSX, GFL_WICON_POSY,        // 通信アイコンXY位置
    _MAXNUM,     // 最大接続人数
    _MAXSIZE,  //最大送信バイト数
    _BCON_GET_NUM,    // 最大ビーコン収集数
    TRUE,     // CRC計算
    FALSE,     // MP通信＝親子型通信モードかどうか
    GFL_NET_TYPE_IRC_WIRELESS,//GFL_NET_TYPE_WIRELESS,//GFL_NET_TYPE_IRC,  //wifi通信を行うかどうか
    TRUE,     // 親が再度初期化した場合、つながらないようにする場合TRUE
    WB_NET_DEBUG_MATSUDA_SERVICEID,  //GameServiceID
#if GFL_NET_IRC
	IRC_TIMEOUT_STANDARD,	// 赤外線タイムアウト時間
#endif
    0,//MP親最大サイズ 512まで
    0,//dummy
};

//--------------------------------------------------------------
/**
 * @brief   ワイヤレス通信テスト
 *
 * @param   wk		
 *
 * @retval  TRUE:終了。　FALSE:処理継続中
 */
//--------------------------------------------------------------
static BOOL DebugMatsuda_WiressTest(D_MATSU_WORK *wk)
{
	BOOL ret;
	
	GF_ASSERT(wk);

	if(aGFLNetInit.bNetType == GFL_NET_TYPE_IRC && wk->seq > 3){
		static u8 shut_print = 0;
//		if(IRC_IsConnect() == FALSE && shut_print == 0){
//			OS_TPrintf("赤外線切断\n");
//			shut_print++;
//		}
	}
	
	switch( wk->seq ){
	case 0:
		CommErrorSys_Setup();	//ゲームメインの最初に一度だけ呼ばれる
		CommErrorSys_Init(GFL_BG_FRAME0_M);	//各アプリのInitで呼ばれる
		wk->seq++;
		break;
	case 1:
		if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_A){
			CommErrorSys_Call();
		}
		break;
	
	case 100:
		{
			GFLNetInitializeStruct net_ini_data;
			
			net_ini_data = aGFLNetInit;
			GFL_NET_Init(&net_ini_data, _initCallBack, wk);	//通信初期化
		}
		wk->seq++;
		break;
	case 101:
		if(GFL_NET_IsInit() == TRUE){	//初期化終了待ち
			wk->seq++;
		}
		break;
	case 102:
		GFL_NET_ChangeoverConnect(_connectCallBack); // 自動接続
		wk->seq++;
		break;
	case 103:
		//自動接続待ち
		if(wk->connect_ok == TRUE){
			OS_TPrintf("接続した\n");
			wk->seq++;
		}
		break;
	case 104:		//タイミングコマンド発行
//		wk->seq = 6;
//		break;
		GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle() ,15, WB_NET_DEBUG_MATSUDA_SERVICEID);
		wk->seq++;
		break;
	case 105:
		if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),15, WB_NET_DEBUG_MATSUDA_SERVICEID) == TRUE){
			OS_TPrintf("タイミング取り成功\n");
			OS_TPrintf("接続人数 = %d\n", GFL_NET_GetConnectNum());
			wk->seq++;
		}
		break;
	
	case 106:	//キーを送信しあう
		ret = -1;
		wk->send_key = GFL_UI_KEY_GetTrg();
		if(wk->send_key & PAD_KEY_UP){
			ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), NET_CMD_KEY, 4, &wk->send_key);
		}
		if(wk->send_key & PAD_KEY_DOWN){
			ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), NET_CMD_KEY, 4, &wk->send_key);
		}
		if(wk->send_key & PAD_KEY_LEFT){
			ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), NET_CMD_KEY, 4, &wk->send_key);
		}
		if(wk->send_key & PAD_KEY_RIGHT){
			ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), NET_CMD_KEY, 4, &wk->send_key);
		}
		
		if(wk->send_key & PAD_BUTTON_B){
			ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), NET_CMD_HUGE, 0x1000, &wk->huge_data);
		}
		
		if(ret == FALSE){
			OS_TPrintf("送信失敗\n");
		}
		
		if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT){
			wk->seq++;
		}
		break;
	case 107:	//通信終了
		if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), GFL_NET_CMD_EXIT_REQ, 0, NULL)){
			//GFL_NET_Exit(_endCallBack);	//通信終了
			wk->seq++;
		}
		break;
	case 108:	//通信終了待ち
		if(wk->connect_ok == FALSE){
			wk->seq++;
		}
		break;
	default:
		return TRUE;
	}
	
	return FALSE;
}



typedef struct{
    int gameNo;   ///< ゲーム種類
} _testBeaconStruct;

static _testBeaconStruct _testBeacon = { WB_NET_DEBUG_MATSUDA_SERVICEID };

//--------------------------------------------------------------
/**
 * @brief   ビーコンデータ取得関数
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------

static void* _netBeaconGetFunc(void* pWork)
{
	return &_testBeacon;
}

///< ビーコンデータサイズ取得関数
static int _netBeaconGetSizeFunc(void* pWork)
{
	return sizeof(_testBeacon);
}

///< ビーコンデータ取得関数
static BOOL _netBeaconCompFunc(GameServiceID myNo,GameServiceID beaconNo)
{
    if(myNo != beaconNo){
        return FALSE;
    }
    return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   初期化完了コールバック
 * @param   pCtl    デバッグワーク
 * @retval  none
 */
//--------------------------------------------------------------
static void _initCallBack(void* pWork)
{
	OS_TPrintf("初期化完了コールバックが呼び出された\n");
	return;
}

//==============================================================================
//	
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   接続完了コールバック
 * @param   pCtl    デバッグワーク
 * @retval  none
 */
//--------------------------------------------------------------
static void _connectCallBack(void* pWork)
{
	D_MATSU_WORK *wk = pWork;
	
    OS_TPrintf("ネゴシエーション完了\n");
    wk->connect_ok = TRUE;
}

static void _endCallBack(void* pWork)
{
	D_MATSU_WORK *wk = pWork;

    NET_PRINT("endCallBack終了\n");
    wk->connect_ok = FALSE;
}


//==============================================================================
//	
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   移動コマンド受信関数
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------

static void _RecvMoveData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
}

//--------------------------------------------------------------
/**
 * @brief   巨大データコマンド受信関数
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------

static void _RecvHugeData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
	D_MATSU_WORK *wk = pWork;
	int i;
	const u8 *data = pData;
	u16 cs = 0;
	
	OS_TPrintf("巨大データ受信 netid = %d, size = 0x%x\n", netID, size);
	for(i = 0; i < size; i++){
		cs += data[i];
		//OS_TPrintf("%d, ", data[i]);
	//	if(i % 32 == 0){
	//		OS_TPrintf("\n");
	//	}
	}
	OS_TPrintf("巨大データ受信チェックサム=0x%x\n", cs);
}

static u8 * _RecvHugeBuffer(int netID, void* pWork, int size)
{
	D_MATSU_WORK *wk = pWork;
	return wk->receive_huge_data[netID - 1];
}


//--------------------------------------------------------------
/**
 * @brief   キー情報コマンド受信関数
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _RecvKeyData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
	int key_data;
	
	if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
		return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
	}
	
	key_data = *((int *)pData);
	OS_TPrintf("データ受信：netID=%d, size=%d, data=%d\n", netID, size, key_data);
	if(key_data & PAD_KEY_UP){
		OS_TPrintf("受信：上\n");
	}
	if(key_data & PAD_KEY_DOWN){
		OS_TPrintf("受信：下\n");
	}
	if(key_data & PAD_KEY_LEFT){
		OS_TPrintf("受信：左\n");
	}
	if(key_data & PAD_KEY_RIGHT){
		OS_TPrintf("受信：右\n");
	}
}
















//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA DebugMatsudaErrorProcData = {
	DebugMatsudaMainProcInit,
	DebugMatsudaMainProcMain,
	DebugMatsudaMainProcEnd,
};


