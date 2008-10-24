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
#include "../ariizumi/ari_debug.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================
//送受信関数用
enum DLPLAY_COMM_COMMAND_TYPE
{
    DC_CMD_LARGE_PACKET = GFL_NET_CMD_COMMAND_MAX,	//大容量データ(ボックスデータを想定
	DC_CMD_CONNET_SIGN,		//子機が接続したときに送る
};
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

	DLPLAY_LARGE_PACKET packetBuff_;	//送受信共通のバッファ！取り扱い注意

};

//======================================================================
//	proto
//======================================================================
DLPLAY_COMM_DATA* DLPlayComm_InitData( u32 heapID );
void	DLPlayComm_TermSystem( DLPLAY_COMM_DATA *d_comm );
BOOL	DLPlayComm_InitSystem( DLPLAY_COMM_DATA *d_comm);

void	DLPlayComm_InitParent( DLPLAY_COMM_DATA *d_comm );
void	DLPlayComm_InitChild( DLPLAY_COMM_DATA *d_comm , u8 *macAddress );
BOOL	DLPlayComm_IsFinish_ConnectParent( DLPLAY_COMM_DATA *d_comm );

void*	DLPlayComm_GetBeaconDataDummy(void);
int		DLPlayComm_GetBeaconSizeDummy(void);

//各種チェック関数
BOOL	DLPlayComm_IsFinish_InitSystem( DLPLAY_COMM_DATA *d_comm );

//各種コールバック
void	DLPlayComm_InitEndCallback( void* pWork );
void	DLPlayComm_ErrorCallBack(GFL_NETHANDLE* pNet,int errNo, void* pWork);
void	DLPlayComm_DisconnectCallBack(GFL_NETHANDLE* pNet);

//送受信用関数
//送
void	DLPlayComm_Send_LargeData( DLPLAY_LARGE_PACKET *data , DLPLAY_COMM_DATA *d_comm );
void	DLPlayComm_Send_ConnectSign( DLPLAY_COMM_DATA *d_comm );
//受
void	DLPlayComm_Post_LargeData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
u8*		DLPlayComm_Post_LargeData_Buff( int netID, void* pWork , int size );
void	DLPlayComm_Post_ConnectSign(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);


static const NetRecvFuncTable DLPlayCommPostTable[] = {
	{ DLPlayComm_Post_LargeData , DLPlayComm_Post_LargeData_Buff },
	{ DLPlayComm_Post_ConnectSign , NULL },
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
#if 1
	{
		u8 i;
		for( i=0;i<TEST_DATA_LINE;i++ )
		{
			u16 j;
			for( j=0;j<TEST_DATA_NUM;j++ )
			{
				d_comm->packetBuff_.data_[i][j] =('A'+i);
			}
			d_comm->packetBuff_.data_[i][TEST_DATA_NUM-1] = '\0';
		}
	}
#endif
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
		0x444,	//ggid  DP=0x333,RANGER=0x178,WII=0x346
		0,  //元になるheapid
		HEAPID_NETWORK,  //通信用にcreateされるHEAPID
		HEAPID_WIFI,  //wifi用にcreateされるHEAPID
		GFL_WICON_POSX,GFL_WICON_POSY,	// 通信アイコンXY位置
		2,//_MAXNUM,	//最大接続人数
		48,//_MAXSIZE,	//最大送信バイト数
		2,//_BCON_GET_NUM,  // 最大ビーコン収集数
		TRUE,		// CRC計算
		FALSE,		// MP通信＝親子型通信モードかどうか
		FALSE,		//wifi通信を行うかどうか
		FALSE,		// 親が再度初期化した場合、つながらないようにする場合TRUE
		1//WB_NET_FIELDMOVE_SERVICEID	//GameServiceID
	};

	aGFLNetInit.baseHeapID = d_comm->heapID_;
//	aGFLNetInit.netHeapID = d_comm->heapID_;


	GFL_NET_Init( &aGFLNetInit , DLPlayComm_InitEndCallback , (void*)d_comm );
	aGFLNetInit.netHeapID = d_comm->heapID_;
	return TRUE;
}

//--------------------------------------------------------------
/**						if ( GFL_UI_KEY_GetTrg() == PAD_BUTTON_START )
	
 *  親機接続初期化 
 */
//--------------------------------------------------------------
void	DLPlayComm_InitParent( DLPLAY_COMM_DATA *d_comm )
{
//	d_comm->commType = FCT_PARENT;
	GFL_NET_InitServer();
}

//--------------------------------------------------------------
/**
 *	子機接続初期化(いきなり接続をする
 */
//--------------------------------------------------------------
void	DLPlayComm_InitChild( DLPLAY_COMM_DATA *d_comm , u8 *macAddress )
{
//	d_comm->commType = FCT_PARENT;
	d_comm->seqNo_ = 0;
	GFL_NET_InitClientAndConnectToParent( macAddress );
}

//  親機への接続が完了し、通信ができる状態になったか？
BOOL	DLPlayComm_IsFinish_ConnectParent( DLPLAY_COMM_DATA *d_comm )
{
	switch( d_comm->seqNo_ )
	{
	case 0:
		if( GFL_NET_HANDLE_RequestNegotiation() == TRUE )
		{
			d_comm->seqNo_++;
		}
		break;
	case 1:
		d_comm->selfHandle_ = GFL_NET_HANDLE_GetCurrentHandle();
		if( GFL_NET_HANDLE_IsNegotiation( d_comm->selfHandle_ ) == TRUE )
		{
			d_comm->seqNo_++;
		}
		break;
	case 2:
		if( GFL_NET_IsSendEnable( d_comm->selfHandle_ ) == TRUE )
		{
			d_comm->seqNo_ = 0;
			return TRUE;
		}
		break;
	}
	return FALSE;
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
 * 通信ライブラリ初期化完了用コールバック						if ( GFL_UI_KEY_GetTrg() == PAD_BUTTON_START )
	
 * @param	vo#include
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


//--------------------------------------------------------------
//	送受信関数
//--------------------------------------------------------------
void	DLPlayComm_Send_LargeData( DLPLAY_LARGE_PACKET *data , DLPLAY_COMM_DATA *d_comm )
{
	GFL_STD_MemCopy( (void*)data , (void*)&d_comm->packetBuff_ , sizeof( DLPLAY_LARGE_PACKET ) );
	{
		const BOOL ret = GFL_NET_SendDataEx( d_comm->selfHandle_ , 
				1/*親機*/ , DC_CMD_LARGE_PACKET , sizeof(DLPLAY_LARGE_PACKET) ,
				&d_comm->packetBuff_ , TRUE , TRUE , TRUE );
		if( ret == FALSE ){ OS_TPrintf("DLPlayComm LargeData send is failued!!\n"); }
	}
	
}

void DLPlayComm_Post_LargeData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
	DLPLAY_COMM_DATA *d_comm = (DLPLAY_COMM_DATA*)pWork;
	ARI_TPrintf("DLPlayComm getData[LargeData]\n");
	{
		u8 i;
		for( i=0;i<TEST_DATA_LINE;i++)
		{
			OS_TPrintf("%2d:%s\n",i,d_comm->packetBuff_.data_[i]);
		}
	}
}

u8*	 DLPlayComm_Post_LargeData_Buff( int netID, void* pWork , int size )
{
	//if( GFL_NET_IsParentMachine() == FALSE ){return (void*)&temp;}
	//else
	{
		DLPLAY_COMM_DATA *d_comm = (DLPLAY_COMM_DATA*)pWork;
		ARI_TPrintf("DLPlayComm PostBuff id[%d] size[%d]\n",netID,size);
		return (u8*)&d_comm->packetBuff_ ;
	}
}

void	DLPlayComm_Send_ConnectSign( DLPLAY_COMM_DATA *d_comm )
{
	u8 dummy = 127;
	d_comm->selfHandle_ = GFL_NET_HANDLE_GetCurrentHandle();
	{
		const BOOL ret = GFL_NET_SendData( d_comm->selfHandle_ , DC_CMD_CONNET_SIGN , 1 , &dummy );
		if( ret == FALSE ){ OS_TPrintf("DLPlayComm ConnectSign send is failued!!\n"); }
	}

}

void	DLPlayComm_Post_ConnectSign(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
	ARI_TPrintf("DLPlayComm getData[ConnectSign]\n");
}


