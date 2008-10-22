//======================================================================
/**
 * @file	dlplay_comm_func.c	
 * @brief	DL通信 親子共通の通信関係
 * @author	ariizumi
 * @data	08/10/21
 */
//======================================================================
#include "gflib.h"
#include "system/gfl_use.h"
#include "net/network_define.h"
#include "system/main.h"


#include "dlplay_comm_func.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================

//======================================================================
//	DLPLAY_COMM_DATA
//======================================================================
struct _DLPLAY_COMM_DATA 
{
	u8		seqNo_;

	HEAPID   heapID_;
	BOOL isFinishInitSystem_;
	BOOL isError_;
	BOOL isStartMode_;

	GFL_NETHANDLE   *selfHandle_;	//自身の通信ハンドル
};

//======================================================================
//	proto
//======================================================================
DLPLAY_COMM_DATA* DLPlayComm_InitData( u32 heapID );
void	DLPlayComm_TermSystem( DLPLAY_COMM_DATA *d_comm );
BOOL	DLPlayComm_InitSystem( DLPLAY_COMM_DATA *d_comm);

void*	DLPlayComm_GetBeaconDataDummy(void);
int		DLPlayComm_GetBeaconSizeDummy(void);

//各種チェック関数
BOOL	DLPlayComm_IsFinish_InitSystem( DLPLAY_COMM_DATA *d_comm );

//各種コールバック
void	DLPlayComm_InitEndCallback( void* pWork );
void	DLPlayComm_ErrorCallBack(GFL_NETHANDLE* pNet,int errNo, void* pWork);
void	DLPlayComm_DisconnectCallBack(GFL_NETHANDLE* pNet);

static const NetRecvFuncTable DLPlayCommPostTable[] = {
	{ NULL , NULL },
};
//--------------------------------------------------------------
/**
 * 通信用データ初期化
 * @param	heapID	ヒープID
 * @retval	DLPLAY_COMM_DATA
 */
//--------------------------------------------------------------
DLPLAY_COMM_DATA* DLPlayComm_InitData( u32 heapID )
{
	u8 i=0;
	DLPLAY_COMM_DATA *d_comm;

	d_comm = GFL_HEAP_AllocClearMemory( heapID , sizeof( DLPLAY_COMM_DATA ) );
	
	d_comm->seqNo_  = 0;
	d_comm->heapID_ = heapID;
	d_comm->isFinishInitSystem_ = FALSE;
	d_comm->isError_ = FALSE;
	d_comm->isStartMode_ = FALSE;

	return d_comm;
}

//--------------------------------------------------------------
/**
 * 通信ライブラリ開放
 * @param	heapID	ヒープID
 * @retval	DLPLAY_COMM_DATA
 */
//--------------------------------------------------------------
void	DLPlayComm_TermSystem( DLPLAY_COMM_DATA *d_comm )
{
	if( d_comm == NULL ){
		OS_TPrintf("FieldComm System is not init.\n");
		return;
	}
	GFL_HEAP_FreeMemory( d_comm );
	d_comm = NULL;
}

//--------------------------------------------------------------
/**
 * 通信ライブラリ初期化
 * @param	heapID	ヒープID
 * @retval	DLPLAY_COMM_DATA
 */
//--------------------------------------------------------------
BOOL	DLPlayComm_InitSystem( DLPLAY_COMM_DATA *d_comm)
{
	GFLNetInitializeStruct aGFLNetInit = {
		DLPlayCommPostTable,	//NetSamplePacketTbl,  // 受信関数テーブル
		NELEMS(DLPlayCommPostTable), // 受信テーブル要素数
		NULL,	// ユーザー同士が交換するデータのポインタ取得関数
		NULL,	// ユーザー同士が交換するデータのサイズ取得関数
		DLPlayComm_GetBeaconDataDummy,	// ビーコンデータ取得関数  
		DLPlayComm_GetBeaconSizeDummy,	// ビーコンデータサイズ取得関数 
		NULL,	// ビーコンのサービスを比較して繋いで良いかどうか判断する
		DLPlayComm_ErrorCallBack,	// 通信不能なエラーが起こった場合呼ばれる 切断するしかない
		DLPlayComm_DisconnectCallBack,	// 通信切断時に呼ばれる関数
		NULL,	// オート接続で親になった場合
#if GFL_NET_WIFI
		NULL,NULL,NULL,NULL,
#endif //GFL_NET_WIFI
		0x3FFF21,//0x444,	//ggid  DP=0x333,RANGER=0x178,WII=0x346
		0,  //元になるheapid
		HEAPID_NETWORK,  //通信用にcreateされるHEAPID
		HEAPID_WIFI,  //wifi用にcreateされるHEAPID
		GFL_WICON_POSX,GFL_WICON_POSY,	// 通信アイコンXY位置
		4,//_MAXNUM,	//最大接続人数
		48,//_MAXSIZE,	//最大送信バイト数
		4,//_BCON_GET_NUM,  // 最大ビーコン収集数
		TRUE,		// CRC計算
		FALSE,		// MP通信＝親子型通信モードかどうか
		FALSE,		//wifi通信を行うかどうか
		TRUE,		// 親が再度初期化した場合、つながらないようにする場合TRUE
		1//WB_NET_FIELDMOVE_SERVICEID	//GameServiceID
	};

	aGFLNetInit.baseHeapID = d_comm->heapID_;
//	aGFLNetInit.netHeapID = d_comm->heapID_;


	GFL_NET_Init( &aGFLNetInit , DLPlayComm_InitEndCallback , (void*)d_comm );
	aGFLNetInit.netHeapID = d_comm->heapID_;
	return TRUE;
}


//--------------------------------------------------------------
/**
 *	各種ビーコン用コールバック関数 
 */
//--------------------------------------------------------------
void*	DLPlayComm_GetBeaconDataDummy(void)
{
	static u8 dummy[2];
	return (void*)&dummy;
}
int	DLPlayComm_GetBeaconSizeDummy(void)
{
	return sizeof(u8)*2;
}


//--------------------------------------------------------------
//	各種チェック関数
//--------------------------------------------------------------
BOOL	DLPlayComm_IsFinish_InitSystem( DLPLAY_COMM_DATA *d_comm )
{
	return d_comm->isFinishInitSystem_;
}




//--------------------------------------------------------------
//	以下、各種コールバック
//--------------------------------------------------------------

//--------------------------------------------------------------
/**
 * 通信ライブラリ初期化完了用コールバック
 * @param	vo#include "net/network_define.h"
#include "textprint.h"
#include "arc_def.h"
#include "system/main.h"

id	*pWork(*DLPLAY_COMM_DATA)
 * @retval	void
 */
//--------------------------------------------------------------
void	DLPlayComm_InitEndCallback( void* pWork )
{
	DLPLAY_COMM_DATA *d_comm = (DLPLAY_COMM_DATA*)pWork;
	d_comm->isFinishInitSystem_ = TRUE;
}

//エラー取得コールバック
void	DLPlayComm_ErrorCallBack(GFL_NETHANDLE* pNet,int errNo, void* pWork)
{
	DLPLAY_COMM_DATA *d_comm = (DLPLAY_COMM_DATA*)pWork;
	OS_TPrintf("FieldComm Error!![%d]\n",errNo);
	d_comm->isError_ = TRUE;
}

//切断感知用コールバック
void	DLPlayComm_DisconnectCallBack(GFL_NETHANDLE* pNet)
{
//	DLPLAY_COMM_DATA *d_comm = (DLPLAY_COMM_DATA*)pWork;
	OS_TPrintf("FieldComm Disconnect!!\n");
//	d_comm->isError_ = TRUE;
}




