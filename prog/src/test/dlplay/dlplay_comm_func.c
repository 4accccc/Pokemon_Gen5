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
#include "dlplay_func.h"
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
    DC_CMD_LARGE_PACKET = GFL_NET_CMD_BOX_DOWNLOAD,	//大容量データ(ボックスデータを想定
	DC_CMD_CONNET_SIGN,		//子機が接続したときに送る
	DC_CMD_BOX_INDEX,		//最初に送るやつ。名前とポケモンNoのみ？
	DC_CMD_POST_INDEX,		//↑の受信完了確認
	DC_CMD_BOX_NUMBER,		//決定したボックスの番号を送る

	DC_CMD_SEND_FLG,		//汎用フラグ送信

	DC_CMD_ERROR,			//エラー発生
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
	BOOL isConnect_;
	BOOL isStartPostIndex_;
	BOOL isPostIndex_;	//親機が子機からデータをもらう
	BOOL isSendIndex_;	//子機側が親機からデータの受信完了をもらう
	BOOL isPostBoxData_;
	BOOL isSendBoxData_;
	BOOL isFinishSaveFirst_;	//相手が終わったか？
	BOOL isFinishSaveSecond_;

	BOOL permitLastSaveFirst_;	//最終セーブ許可
	BOOL permitLastSaveSecond_;
	
	GFL_NETHANDLE		*selfHandle_;	//自身の通信ハンドル

	DLPLAY_LARGE_PACKET packetBuff_;	//送受信共通のバッファ！取り扱い注意
	DLPLAY_BOX_INDEX	boxIndexBuff_;	//同上

	u8					selectBoxNumber_;
	u16					waitCount_;			//同期セーブ用待ち時間
	u8					postErrorState_;	//受け取ったエラー状態
#if DLPLAY_FUNC_USE_PRINT
	DLPLAY_MSG_SYS		*msgSys_;
#endif
};

//======================================================================
//	proto
//======================================================================
DLPLAY_COMM_DATA* DLPlayComm_InitData( u32 heapID );
void	DLPlayComm_TermData( DLPLAY_COMM_DATA *d_comm );
BOOL	DLPlayComm_InitSystem( DLPLAY_COMM_DATA *d_comm);
void	DLPlayComm_TermSystem( DLPLAY_COMM_DATA *d_comm );
#if DLPLAY_FUNC_USE_PRINT
void	DLPlayComm_SetMsgSys( DLPLAY_MSG_SYS *msgSys , DLPLAY_COMM_DATA *d_comm );
#endif

void	DLPlayComm_InitParent( DLPLAY_COMM_DATA *d_comm );
void	DLPlayComm_InitChild( DLPLAY_COMM_DATA *d_comm , u8 *macAddress );
BOOL	DLPlayComm_IsFinish_Negotiation( DLPLAY_COMM_DATA *d_comm );

void*	DLPlayComm_GetBeaconDataDummy(void* pWork);
int		DLPlayComm_GetBeaconSizeDummy(void* pWork);

//各種チェック関数
BOOL	DLPlayComm_IsFinish_InitSystem( DLPLAY_COMM_DATA *d_comm );
BOOL	DLPlayComm_IsFinish_TermSystem( DLPLAY_COMM_DATA *d_comm );
BOOL	DLPlayComm_IsConnect( DLPLAY_COMM_DATA *d_comm );
BOOL	DLPlayComm_IsStartPostIndex( DLPLAY_COMM_DATA *d_comm );
BOOL	DLPlayComm_IsPostIndex( DLPLAY_COMM_DATA *d_comm );
BOOL	DLPlayComm_IsPost_SendIndex( DLPLAY_COMM_DATA *d_comm );
BOOL	DLPlayComm_IsPostData( DLPLAY_COMM_DATA *d_comm );
BOOL	DLPlayComm_IsPost_SendData( DLPLAY_COMM_DATA *d_comm );
const	DLPLAY_CARD_TYPE DLPlayComm_GetCardType( DLPLAY_COMM_DATA *d_comm );
void	DLPlayComm_SetCardType( DLPLAY_COMM_DATA *d_comm , const DLPLAY_CARD_TYPE type );
const u8	DLPlayComm_GetSelectBoxNumber( DLPLAY_COMM_DATA *d_comm );
const u8	DLPlayComm_GetPostErrorState( DLPLAY_COMM_DATA *d_comm );
const u16	DLPlayComm_IsFinishSaveFlg( u8 flg , DLPLAY_COMM_DATA *d_comm );

void	DLPlayComm_InitEndCallback( void* pWork );
void	DLPlayComm_TermEndCallback( void* pWork );
void	DLPlayComm_ErrorCallBack(GFL_NETHANDLE* pNet,int errNo, void* pWork);
void	DLPlayComm_DisconnectCallBack( void* pWork);

//送受信用関数
//送
DLPLAY_LARGE_PACKET* DLPlayComm_GetLargePacketBuff( DLPLAY_COMM_DATA *d_comm );
void	DLPlayComm_Send_LargeData( DLPLAY_COMM_DATA *d_comm );
void	DLPlayComm_Send_ConnectSign( DLPLAY_COMM_DATA *d_comm );
DLPLAY_BOX_INDEX* DLPlayComm_GetBoxIndexBuff( DLPLAY_COMM_DATA *d_comm );
void	DLPlayComm_Send_BoxIndex( DLPLAY_COMM_DATA *d_comm );
void	DLPlayComm_Send_PostBoxIndex( DLPLAY_COMM_DATA *d_comm );
void	DLPlayComm_Send_BoxNumber( u8 idx , DLPLAY_COMM_DATA *d_comm );
void	DLPlayComm_Send_ErrorState( u8 type , DLPLAY_COMM_DATA *d_comm );
void	DLPlayComm_Send_CommonFlg( u8 flg , u16 value , DLPLAY_COMM_DATA *d_comm );

//受
void	DLPlayComm_Post_LargeData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
u8*		DLPlayComm_Post_LargeData_Buff( int netID, void* pWork , int size );
void	DLPlayComm_Post_ConnectSign(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
void	DLPlayComm_Post_BoxIndex(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
u8*		DLPlayComm_Post_BoxIndex_Buff( int netID, void* pWork , int size );
void	DLPlayComm_Post_PostBoxIndex( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
void	DLPlayComm_Post_BoxNumber( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
void	DLPlayComm_Post_ErrorState( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
void	DLPlayComm_Post_CommonFlg( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );

static const NetRecvFuncTable DLPlayCommPostTable[] = {
	{ DLPlayComm_Post_LargeData , DLPlayComm_Post_LargeData_Buff },	//ボックスデータ
	{ DLPlayComm_Post_ConnectSign , NULL },							//接続確認
	{ DLPlayComm_Post_BoxIndex , DLPlayComm_Post_BoxIndex_Buff },	//インデックスデータ
	{ DLPlayComm_Post_PostBoxIndex , NULL },	//↑受信確認
	{ DLPlayComm_Post_BoxNumber , NULL },		//親機決定ボックス番号
	{ DLPlayComm_Post_CommonFlg , NULL },		//セーブ完了フラグ汎用
	{ DLPlayComm_Post_ErrorState , NULL },		//エラー処理
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
	d_comm->isFinishInitSystem_	= FALSE;
	d_comm->isError_		= FALSE;
	d_comm->isStartMode_	= FALSE;
	d_comm->isConnect_		= FALSE;
	d_comm->isStartPostIndex_ = FALSE;
	d_comm->isPostIndex_	= FALSE;
	d_comm->isSendIndex_	= FALSE;
	d_comm->isPostBoxData_	= FALSE;
	d_comm->isSendBoxData_	= FALSE;
	d_comm->isFinishSaveFirst_ = FALSE;		//相手が終わったか？
	d_comm->isFinishSaveSecond_ = FALSE;
	d_comm->permitLastSaveFirst_ = FALSE;	//最終セーブ許可
	d_comm->permitLastSaveSecond_ = FALSE;
	
	d_comm->selectBoxNumber_ = SELECT_BOX_INVALID;
	d_comm->waitCount_		= 0;

	MI_CpuClearFast( d_comm->packetBuff_.pokeData_ , LARGEPACKET_POKE_SIZE );
	return d_comm;
}	

//--------------------------------------------------------------
/**
 * 通信ライブラリ開放
 * @param	heapID	ヒープID
 * @retval	DLPLAY_COMM_DATA
 */
//--------------------------------------------------------------
void	DLPlayComm_TermData( DLPLAY_COMM_DATA *d_comm )
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
        NULL,    ///< ハードで接続した時に呼ばれる
        NULL,    ///< ネゴシエーション完了時にコール
		NULL,	// ユーザー同士が交換するデータのポインタ取得関数
		NULL,	// ユーザー同士が交換するデータのサイズ取得関数
		DLPlayComm_GetBeaconDataDummy,	// ビーコンデータ取得関数  
		DLPlayComm_GetBeaconSizeDummy,	// ビーコンデータサイズ取得関数 
		NULL,	// ビーコンのサービスを比較して繋いで良いかどうか判断する
		DLPlayComm_ErrorCallBack,	// 通信不能なエラーが起こった場合呼ばれる 
        NULL,  //Fatalエラーが起こった場合
		DLPlayComm_DisconnectCallBack,	// 通信切断時に呼ばれる関数
		NULL,	// オート接続で親になった場合
#if GFL_NET_WIFI
    NULL,     ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
    NULL, ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
    NULL,  ///< wifiフレンドリスト削除コールバック
    NULL,   ///< DWC形式の友達リスト	
    NULL,  ///< DWCのユーザデータ（自分のデータ）
    0,   ///< DWCへのHEAPサイズ
    TRUE,        ///< デバック用サーバにつなぐかどうか
#endif  //GFL_NET_WIFI
    SYASHI_NETWORK_GGID,  //ggid  
		0,  //元になるheapid
		HEAPID_NETWORK,  //通信用にcreateされるHEAPID
		HEAPID_WIFI,  //wifi用にcreateされるHEAPID
		HEAPID_WIFI,  //赤外線用にcreateされるHEAPID
		GFL_WICON_POSX,GFL_WICON_POSY,	// 通信アイコンXY位置
		2,//_MAXNUM,	//最大接続人数
		48,//_MAXSIZE,	//最大送信バイト数
		2,//_BCON_GET_NUM,  // 最大ビーコン収集数
		TRUE,		// CRC計算
		FALSE,		// MP通信＝親子型通信モードかどうか
		GFL_NET_TYPE_WIRELESS,		//	NET通信タイプ ← wifi通信を行うかどうか
		FALSE,		// 親が再度初期化した場合、つながらないようにする場合TRUE
		WB_NET_BOX_DOWNLOAD_SERVICEID,//GameServiceID
#if GFL_NET_IRC
	IRC_TIMEOUT_STANDARD,	// 赤外線タイムアウト時間
#endif
    0,//MP親最大サイズ 512まで
    0,//dummy
	};

	aGFLNetInit.baseHeapID = d_comm->heapID_;
	GFL_NET_Init( &aGFLNetInit , DLPlayComm_InitEndCallback , (void*)d_comm );
	aGFLNetInit.netHeapID = d_comm->heapID_;
	return TRUE;
}
void	DLPlayComm_TermSystem( DLPLAY_COMM_DATA *d_comm )
{
	u8 dummy = 127;
	d_comm->selfHandle_ = GFL_NET_HANDLE_GetCurrentHandle();
	{
		const BOOL ret = GFL_NET_SendData( d_comm->selfHandle_ , GFL_NET_CMD_EXIT_REQ , 1 , &dummy );
		if( ret == FALSE ){ OS_TPrintf("DLPlayComm ConnectSign send is failued!!\n"); }
	}
}

#if DLPLAY_FUNC_USE_PRINT
void	DLPlayComm_SetMsgSys( DLPLAY_MSG_SYS *msgSys , DLPLAY_COMM_DATA *d_comm )
{
	d_comm->msgSys_ = msgSys;
}
#endif

//--------------------------------------------------------------
/**						
 *  親機接続初期化 
 */
//--------------------------------------------------------------
void	DLPlayComm_InitParent( DLPLAY_COMM_DATA *d_comm )
{
	GFL_NET_InitServer();
	d_comm->seqNo_ = 0;
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
BOOL	DLPlayComm_IsFinish_Negotiation( DLPLAY_COMM_DATA *d_comm )
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
 *	各種ビーコン用コールバック関数(ダミー
 */
//--------------------------------------------------------------
void*	DLPlayComm_GetBeaconDataDummy(void* pWork)
{
	static u8 dummy[2];
	return (void*)&dummy;
}
int	DLPlayComm_GetBeaconSizeDummy(void* pWork)
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
BOOL	DLPlayComm_IsFinish_TermSystem( DLPLAY_COMM_DATA *d_comm )
{
	return !d_comm->isFinishInitSystem_;
}
BOOL	DLPlayComm_IsConnect( DLPLAY_COMM_DATA *d_comm )
{
	d_comm->selfHandle_ = GFL_NET_HANDLE_GetCurrentHandle();
	if( GFL_NET_HANDLE_IsNegotiation( d_comm->selfHandle_ ) == TRUE )
	{
		return d_comm->isConnect_;
	}
	return FALSE;
}
BOOL	DLPlayComm_IsStartPostIndex( DLPLAY_COMM_DATA *d_comm )
{
	return d_comm->isStartPostIndex_;
}
BOOL	DLPlayComm_IsPostIndex( DLPLAY_COMM_DATA *d_comm )
{
	return d_comm->isPostIndex_;
}
BOOL	DLPlayComm_IsPost_SendIndex( DLPLAY_COMM_DATA *d_comm )
{
	return d_comm->isSendIndex_;
}
BOOL	DLPlayComm_IsPostData( DLPLAY_COMM_DATA *d_comm )
{
	return d_comm->isPostBoxData_;
}
BOOL	DLPlayComm_IsPost_SendData( DLPLAY_COMM_DATA *d_comm )
{
	return d_comm->isSendBoxData_;
}
const u8	DLPlayComm_GetSelectBoxNumber( DLPLAY_COMM_DATA *d_comm )
{
	return d_comm->selectBoxNumber_;
}
const u8	DLPlayComm_GetPostErrorState( DLPLAY_COMM_DATA *d_comm )
{
	return d_comm->postErrorState_;
}
//関数増えるのいやだったのでまとめた・・・
//フラグは送信と一緒のやつ
const u16	DLPlayComm_IsFinishSaveFlg( u8 flg , DLPLAY_COMM_DATA *d_comm )
{
	switch( flg )
	{
	case DC_FLG_FINISH_SAVE1_PARENT:	//親機第1セーブ完了
	case DC_FLG_FINISH_SAVE1_CHILD:		//子機第1セーブ完了
		return d_comm->isFinishSaveFirst_;
		break;
	case DC_FLG_PERMIT_LASTBIT_SAVE:	//最終ビットセーブ開始
		if( d_comm->permitLastSaveFirst_ == TRUE )
		{
			return d_comm->waitCount_;
		}
		return 0;
		break;
	case DC_FLG_FINISH_SAVE_ALL:
		return d_comm->isFinishSaveSecond_;
		break;
	}
	return 0;
}

//--------------------------------------------------------------
//	以下、各種コールバック
//--------------------------------------------------------------

//--------------------------------------------------------------
/**
 * 通信ライブラリ初期化完了用コールバック						
	
 * @param	vo#include
 * @retval	void
 */
//--------------------------------------------------------------
void	DLPlayComm_InitEndCallback( void* pWork )
{
	DLPLAY_COMM_DATA *d_comm = (DLPLAY_COMM_DATA*)pWork;
	d_comm->isFinishInitSystem_ = TRUE;
}
//終了待ちコールバック
void	DLPlayComm_TermEndCallback( void* pWork )
{
	DLPLAY_COMM_DATA *d_comm = (DLPLAY_COMM_DATA*)pWork;
	GF_ASSERT( d_comm->isFinishInitSystem_ == TRUE );
	d_comm->isFinishInitSystem_ = FALSE;
}


//エラー取得コールバック
void	DLPlayComm_ErrorCallBack(GFL_NETHANDLE* pNet,int errNo, void* pWork)
{
	DLPLAY_COMM_DATA *d_comm = (DLPLAY_COMM_DATA*)pWork;
	OS_TPrintf("FieldComm Error!![%d]\n",errNo);
	d_comm->isError_ = TRUE;
}
//切断感知用コールバック
void	DLPlayComm_DisconnectCallBack( void* pWork )
{
	DLPLAY_COMM_DATA *d_comm = (DLPLAY_COMM_DATA*)pWork;
	OS_TPrintf("FieldComm Disconnect!!\n");
//	GFL_NET_Exit( DLPlayComm_TermEndCallback );
	GF_ASSERT( d_comm->isFinishInitSystem_ == TRUE );
	d_comm->isFinishInitSystem_ = FALSE;
}

//--------------------------------------------------------------
//	送受信関数
//--------------------------------------------------------------
#if DLPLAY_FUNC_USE_PRINT
static OSTick tickWork = 0;
#endif
//--------------------------------------------------------------
//	LargePacket関係
//--------------------------------------------------------------
//Bufferを渡す(ここにデータを入れてから送信する
DLPLAY_LARGE_PACKET* DLPlayComm_GetLargePacketBuff( DLPLAY_COMM_DATA *d_comm )
{
	return &d_comm->packetBuff_;
}
void	DLPlayComm_Send_LargeData( DLPLAY_COMM_DATA *d_comm )
{
	//バッファのポインタを外に渡してセットしてもらうようにしたので、コピーは無し。
//	GFL_STD_MemCopy( (void*)data , (void*)&d_comm->packetBuff_ , sizeof( DLPLAY_LARGE_PACKET ) );
	{
		const BOOL ret = GFL_NET_SendDataEx( d_comm->selfHandle_ , 
				0/*親機*/ , DC_CMD_LARGE_PACKET , sizeof(DLPLAY_LARGE_PACKET) ,
				&d_comm->packetBuff_ , TRUE , TRUE , TRUE );
		if( ret == FALSE ){ OS_TPrintf("DLPlayComm LargeData send is failued!!\n"); }
	}
}

//--------------------------------------------------------------
//受信コールバック
//--------------------------------------------------------------
void DLPlayComm_Post_LargeData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
	DLPLAY_COMM_DATA *d_comm = (DLPLAY_COMM_DATA*)pWork;
	d_comm->isPostBoxData_ = TRUE;
#if DLPLAY_FUNC_USE_PRINT
	{
		OSTick postTick = OS_GetTick();
		char str[64];
		sprintf(str,"LargePacket finish post. tick[%d]",OS_TicksToMilliSeconds(postTick-tickWork));
		DLPlayFunc_PutString( str , d_comm->msgSys_ );
	}
#endif
}

u8*	 DLPlayComm_Post_LargeData_Buff( int netID, void* pWork , int size )
{
	{
		DLPLAY_COMM_DATA *d_comm = (DLPLAY_COMM_DATA*)pWork;
#if DLPLAY_FUNC_USE_PRINT
		char str[64];
		sprintf(str,"LargePacket start post size[%d]",size);
		DLPlayFunc_PutString( str , d_comm->msgSys_ );
		tickWork = OS_GetTick();
#endif
		return (u8*)&d_comm->packetBuff_ ;
	}
}

//--------------------------------------------------------------
//BoxIndex系
//--------------------------------------------------------------
DLPLAY_BOX_INDEX* DLPlayComm_GetBoxIndexBuff( DLPLAY_COMM_DATA *d_comm )
{
	return &d_comm->boxIndexBuff_;
}
void	DLPlayComm_Send_BoxIndex( DLPLAY_COMM_DATA *d_comm )
{
	{
		const BOOL ret = GFL_NET_SendDataEx( d_comm->selfHandle_ , 
				0/*親機*/ , DC_CMD_BOX_INDEX , sizeof(DLPLAY_BOX_INDEX) ,
				&d_comm->boxIndexBuff_ , TRUE , TRUE , TRUE );
		if( ret == FALSE ){ OS_TPrintf("DLPlayComm BoxIndexData send is failued!!\n"); }
	}
}

//--------------------------------------------------------------
//	受信コールバック
//--------------------------------------------------------------
//ボックスのインデックスデータ関係
void DLPlayComm_Post_BoxIndex(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
	DLPLAY_COMM_DATA *d_comm = (DLPLAY_COMM_DATA*)pWork;
	d_comm->isPostIndex_ = TRUE;
#if DLPLAY_FUNC_USE_PRINT
	{
	OSTick postTick = OS_GetTick();
	char str[64];
	sprintf(str,"BoxIndex finish post. tick[%d]",OS_TicksToMilliSeconds(postTick-tickWork));
	DLPlayFunc_PutString( str , d_comm->msgSys_ );
	}
#endif
	//受信確認を即送る
	DLPlayComm_Send_PostBoxIndex( d_comm );
}

u8*	 DLPlayComm_Post_BoxIndex_Buff( int netID, void* pWork , int size )
{
	DLPLAY_COMM_DATA *d_comm = (DLPLAY_COMM_DATA*)pWork;
#if DLPLAY_FUNC_USE_PRINT
	char str[64];
	sprintf(str,"BoxIndex start post size[%d]",size);
	tickWork = OS_GetTick();
	DLPlayFunc_PutString( str , d_comm->msgSys_ );
#endif
	d_comm->isStartPostIndex_ = TRUE;
	return (u8*)&d_comm->boxIndexBuff_ ;
}

//子機再接続時の合図
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
	DLPLAY_COMM_DATA *d_comm = (DLPLAY_COMM_DATA*)pWork;
	d_comm->isConnect_ = TRUE;
	ARI_TPrintf("DLPlayComm getData[ConnectSign]\n");
}

//ボックスインデックス受信確認用フラグ
void	DLPlayComm_Send_PostBoxIndex( DLPLAY_COMM_DATA *d_comm )
{
	u8 dummy = 127;
	d_comm->selfHandle_ = GFL_NET_HANDLE_GetCurrentHandle();
	{
		const BOOL ret = GFL_NET_SendData( d_comm->selfHandle_ , DC_CMD_POST_INDEX , 1 , &dummy );
		if( ret == FALSE ){ OS_TPrintf("DLPlayComm PostBoxData send is failued!!\n"); }
	}
}

void	DLPlayComm_Post_PostBoxIndex( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
	DLPLAY_COMM_DATA *d_comm = (DLPLAY_COMM_DATA*)pWork;
	d_comm->isSendIndex_ = TRUE;
	ARI_TPrintf("DLPlayComm getData[postBoxData]\n");
}

//選択ボックス番号送信用
void	DLPlayComm_Send_BoxNumber( u8 idx , DLPLAY_COMM_DATA *d_comm )
{
	d_comm->selfHandle_ = GFL_NET_HANDLE_GetCurrentHandle();
	{
		const BOOL ret = GFL_NET_SendData( d_comm->selfHandle_ , DC_CMD_BOX_NUMBER , 1 , &idx );
		if( ret == FALSE ){ OS_TPrintf("DLPlayComm SelectBoxIndex send is failued!!\n"); }
	}
}

void	DLPlayComm_Post_BoxNumber( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
	DLPLAY_COMM_DATA *d_comm = (DLPLAY_COMM_DATA*)pWork;
	d_comm->selectBoxNumber_ = *(u8*)pData;
	ARI_TPrintf("DLPlayComm getData[SelectBoxNumber:%d]\n",d_comm->selectBoxNumber_);
}

//エラー発生同期用
void	DLPlayComm_Send_ErrorState( u8 type , DLPLAY_COMM_DATA *d_comm )
{
	d_comm->selfHandle_ = GFL_NET_HANDLE_GetCurrentHandle();
	{
		const BOOL ret = GFL_NET_SendData( d_comm->selfHandle_ , DC_CMD_ERROR , 1 , &type );
		if( ret == FALSE ){ OS_TPrintf("DLPlayComm ErrorState send is failued!!\n"); }
	}
}

void	DLPlayComm_Post_ErrorState( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
	DLPLAY_COMM_DATA *d_comm = (DLPLAY_COMM_DATA*)pWork;
	d_comm->postErrorState_ = *(u8*)pData;
	ARI_TPrintf("DLPlayComm getData[ErrorState:%d]\n",d_comm->postErrorState_);
}

//汎用フラグ処理(u8数値つき
typedef struct
{
	u8 flg;
	u16 value;
}DLPLAY_COMM_FLG;
void	DLPlayComm_Send_CommonFlg( u8 flg , u16 value , DLPLAY_COMM_DATA *d_comm )
{
	DLPLAY_COMM_FLG sendData;
	sendData.flg = flg;
	sendData.value = value;
	d_comm->selfHandle_ = GFL_NET_HANDLE_GetCurrentHandle();
	{
		const BOOL ret = GFL_NET_SendData( d_comm->selfHandle_ , DC_CMD_SEND_FLG , 3 , &sendData );
		if( ret == FALSE ){ OS_TPrintf("DLPlayComm CommonFlg send is failued!!\n"); }
	}
}

void	DLPlayComm_Post_CommonFlg( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
	DLPLAY_COMM_DATA *d_comm = (DLPLAY_COMM_DATA*)pWork;
	const DLPLAY_COMM_FLG *sendData = (DLPLAY_COMM_FLG*)pData;
	ARI_TPrintf("DLPlayComm getData[CommonFlg:%d:%d]\n",sendData->flg,sendData->value);
	
	switch( sendData->flg )
	{	
	case DC_FLG_FINISH_SAVE1_PARENT:	//親機第1セーブ完了
	case DC_FLG_FINISH_SAVE1_CHILD:		//子機第1セーブ完了
		if( netID != GFL_NET_GetNetID( d_comm->selfHandle_ ) )
		{
			d_comm->isFinishSaveFirst_ = TRUE;
		}
		break;
	case DC_FLG_PERMIT_LASTBIT_SAVE:	//最終ビットセーブ開始
		//こいつは親→子しか無いので一緒に処理
		d_comm->permitLastSaveFirst_ = TRUE;
		d_comm->waitCount_ = sendData->value;
		break;
	case DC_FLG_FINISH_SAVE_ALL:
		//このフラグは親からしか来ないので、OK
		//if( netID != GFL_NET_GetNetID( d_comm->selfHandle_ ) )
		{
			d_comm->isFinishSaveSecond_ = TRUE;
		}
		break;
	case DC_FLG_POST_BOX_DATA:	//親→子ボックスデータ受信確認(セーブ開始合図
		d_comm->isSendBoxData_ = TRUE;
		break;
	}
}

