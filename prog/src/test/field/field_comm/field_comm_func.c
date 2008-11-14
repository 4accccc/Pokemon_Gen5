//======================================================================
/**
 * @file	field_comm_func.c
 * @brief	フィールド通信　通信機能部
 * @author	ariizumi
 * @data	08/11/13
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "net/network_define.h"

#include "gamesystem/playerwork.h"
#include "field_comm_func.h"
#include "field_comm_data.h"
#include "test/ariizumi/ari_debug.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================
//通信状態
typedef enum
{
	FIELD_COMM_MODE_NONE,	//通信なし
	FIELD_COMM_MODE_PARENT,	//親機(侵入待ち・侵入後
	FIELD_COMM_MODE_CHILD,	//子機(侵入後
	FIELD_COMM_MODE_SEARCH,	//探索中(侵入先探し中
}FIELD_COMM_MODE;

//送受信関数用
enum FIELD_COMM_COMMAND_TYPE
{
    FC_CMD_SELFDATA = GFL_NET_CMD_COMMAND_MAX,	//大容量データ(ボックスデータを想定
};

//======================================================================
//	typedef struct
//======================================================================
struct _FIELD_COMM_FUNC
{
	HEAPID	heapID_;
	FIELD_COMM_MODE commMode_;
	
	BOOL	isInitCommSystem_;

};

//======================================================================
//	proto
//======================================================================

FIELD_COMM_FUNC* FieldCommFunc_InitSystem( HEAPID heapID );
void	FieldCommFunc_TermSystem( FIELD_COMM_FUNC *commFunc );
void	FieldCommFunc_InitCommSystem( FIELD_COMM_FUNC *commFunc );
void	FieldCommFunc_TermCommSystem( FIELD_COMM_FUNC *commFunc );

void	FieldCommFunc_StartCommChild( FIELD_COMM_FUNC *commFunc );
void	FieldCommFunc_StartCommParent( FIELD_COMM_FUNC *commFunc );
void	FieldCommFunc_StartCommChangeover( FIELD_COMM_FUNC *commFunc );

//各種チェック関数
const BOOL FieldCommFunc_IsFinishInitCommSystem( FIELD_COMM_FUNC *commFunc );
const BOOL FieldCommFunc_IsFinishTermCommSystem( FIELD_COMM_FUNC *commFunc );
const int	FieldCommFunc_GetSelfIndex( FIELD_COMM_FUNC *commFunc );

//送受信関数
void	FieldCommFunc_Send_SelfData( FIELD_COMM_FUNC *commFunc );
void	FieldCommFunc_Post_SelfData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );

//各種コールバック
void	FieldCommFunc_FinishInitCallback( void* pWork );
void	FieldCommFunc_FinishTermCallback( void* pWork );
void*	FieldCommFunc_GetBeaconData(void);		// ビーコンデータ取得関数  
int		FieldCommFunc_GetBeaconSize(void);		// ビーコンデータサイズ取得関数 
BOOL	FieldCommFunc_CheckConnectService(GameServiceID GameServiceID1 , GameServiceID GameServiceID2 ); // ビーコンのサービスを比較して繋いで良いかどうか判断する
void	FieldCommFunc_ErrorCallBack(GFL_NETHANDLE* pNet,int errNo, void* pWork);		// 通信不能なエラーが起こった場合呼ばれる 切断するしかない
void	FieldCommFunc_DisconnectCallBack(void* pWork);	// 通信切断時に呼ばれる関数(終了時

static const NetRecvFuncTable FieldCommRecvTable[] = {
	{ FieldCommFunc_Post_SelfData ,NULL }
};


//--------------------------------------------------------------
//	システム初期化
//	この時点では通信は開始しません
//--------------------------------------------------------------
FIELD_COMM_FUNC* FieldCommFunc_InitSystem( HEAPID heapID )
{
	FIELD_COMM_FUNC	*commFunc;
	
	commFunc = GFL_HEAP_AllocMemory( heapID , sizeof(FIELD_COMM_FUNC) );
	commFunc->heapID_ = heapID;
	//commFunc->isInitCommSystem_ = FALSE;
	commFunc->isInitCommSystem_ = GFL_NET_IsInit();
	return commFunc;
}

//--------------------------------------------------------------
//	システム開放
//--------------------------------------------------------------
void	FieldCommFunc_TermSystem( FIELD_COMM_FUNC *commFunc )
{
	GFL_HEAP_FreeMemory( commFunc );
}

//--------------------------------------------------------------
//	通信開始
//--------------------------------------------------------------
void	FieldCommFunc_InitCommSystem( FIELD_COMM_FUNC *commFunc )
{
	GFLNetInitializeStruct aGFLNetInit = {
		FieldCommRecvTable,	//NetSamplePacketTbl,  // 受信関数テーブル
		NELEMS(FieldCommRecvTable), // 受信テーブル要素数
        NULL,    ///< ハードで接続した時に呼ばれる
        NULL,    ///< ネゴシエーション完了時にコール
        NULL,	// ユーザー同士が交換するデータのポインタ取得関数
		NULL,	// ユーザー同士が交換するデータのサイズ取得関数
		FieldCommFunc_GetBeaconData,		// ビーコンデータ取得関数  
		FieldCommFunc_GetBeaconSize,		// ビーコンデータサイズ取得関数 
		FieldCommFunc_CheckConnectService,	// ビーコンのサービスを比較して繋いで良いかどうか判断する
		FieldCommFunc_ErrorCallBack,		// 通信不能なエラーが起こった場合呼ばれる 切断するしかない
		FieldCommFunc_DisconnectCallBack,	// 通信切断時に呼ばれる関数(終了時
		NULL,	// オート接続で親になった場合
#if GFL_NET_WIFI
		NULL,NULL,NULL,NULL,
#endif //GFL_NET_WIFI
		0x444,	//ggid  DP=0x333,RANGER=0x178,WII=0x346
		GFL_HEAPID_APP,  //元になるheapid
		HEAPID_NETWORK,  //通信用にcreateされるHEAPID
		HEAPID_WIFI,  //wifi用にcreateされるHEAPID
		HEAPID_NETWORK,	//
		GFL_WICON_POSX,GFL_WICON_POSY,	// 通信アイコンXY位置
		4,//_MAXNUM,	//最大接続人数
		48,//_MAXSIZE,	//最大送信バイト数
		4,//_BCON_GET_NUM,  // 最大ビーコン収集数
		TRUE,		// CRC計算
		FALSE,		// MP通信＝親子型通信モードかどうか
		GFL_NET_TYPE_WIRELESS,		//通信タイプの指定
		TRUE,		// 親が再度初期化した場合、つながらないようにする場合TRUE
		WB_NET_FIELDMOVE_SERVICEID	//GameServiceID
	};

	GFL_NET_Init( &aGFLNetInit , FieldCommFunc_FinishInitCallback , (void*)commFunc );
}

//--------------------------------------------------------------
//	通信システム終了(通信の切断はしません
//--------------------------------------------------------------
void	FieldCommFunc_TermCommSystem( FIELD_COMM_FUNC *commFunc )
{
	GFL_NET_Exit( FieldCommFunc_FinishTermCallback );
}

//--------------------------------------------------------------
//	親機通信の開始
//--------------------------------------------------------------
void	FieldCommFunc_StartCommParent( FIELD_COMM_FUNC *commFunc )
{
	GFL_NET_InitServer();
}

//--------------------------------------------------------------
//	子機通信の開始
//--------------------------------------------------------------
void	FieldCommFunc_StartCommChild( FIELD_COMM_FUNC *commFunc )
{
	GFL_NET_StartBeaconScan();
}

//--------------------------------------------------------------
//	探索通信の開始(親子交互通信
//--------------------------------------------------------------
void	FieldCommFunc_StartCommChangeover( FIELD_COMM_FUNC *commFunc )
{
	GFL_NET_ChangeoverConnect(NULL);
}


//======================================================================
//	各種チェック関数
//======================================================================
//--------------------------------------------------------------
// 通信ライブラリ　初期化・開放用  
//--------------------------------------------------------------
const BOOL FieldCommFunc_IsFinishInitCommSystem( FIELD_COMM_FUNC *commFunc )
{
	return commFunc->isInitCommSystem_;
}
const BOOL FieldCommFunc_IsFinishTermCommSystem( FIELD_COMM_FUNC *commFunc )
{
	return !commFunc->isInitCommSystem_;
}

//--------------------------------------------------------------
// 自分のインデックス(netID)を取得  
//--------------------------------------------------------------
const int	FieldCommFunc_GetSelfIndex( FIELD_COMM_FUNC *commFunc )
{
	GFL_NETHANDLE *handle =GFL_NET_HANDLE_GetCurrentHandle();
	return GFL_NET_GetNetID( handle );
}

//======================================================================
//送受信関数
//======================================================================
typedef struct
{
	u16 posX_;
	u16 posZ_;
	u8	posY_;		//マックス不明なので。場合によってはなしでOK？
	u8	dir_;
	u16	zoneID_;	//ここは通信用のIDとして変換して抑えられる
}FIELD_COMM_PLAYER_PACKET;
//--------------------------------------------------------------
// 自分のデータ送信  
//--------------------------------------------------------------
void	FieldCommFunc_Send_SelfData( FIELD_COMM_FUNC *commFunc )
{
	FIELD_COMM_PLAYER_PACKET plPkt;
	PLAYER_WORK *plWork = NULL;
	const VecFx32 *pos;
	u16 dir;
	ZONEID zoneID;

	//データ収集
	plWork = FieldCommData_GetSelfData_PlayerWork();
	zoneID = PLAYERWORK_getZoneID( plWork );
	pos = PLAYERWORK_getPosition( plWork );
	dir = PLAYERWORK_getDirection( plWork );
	
	//パケットにセット
	plPkt.zoneID_ = zoneID;
	plPkt.posX_ = F32_CONST( pos->x );
	plPkt.posY_ = F32_CONST( pos->y );
	plPkt.posZ_ = F32_CONST( pos->z );
	//plPkt.dir_ = dir>>8;
	plPkt.dir_ = dir;

	ARI_TPrintf("SEND[ ][%d][%d][%d][%d]\n",plPkt.posX_,plPkt.posY_,plPkt.posZ_,dir);
	{
		GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
		const BOOL ret = GFL_NET_SendDataEx( selfHandle , GFL_NET_SENDID_ALLUSER , 
				FC_CMD_SELFDATA , sizeof(FIELD_COMM_PLAYER_PACKET) , 
				(void*)&plPkt , FALSE , TRUE , FALSE );
		if( ret == FALSE ){
			ARI_TPrintf("FieldComm SendSelfData is failue!\n");
		}
	}
}

//--------------------------------------------------------------
// 自分のデータ受信(用は他のプレイヤーの受信  
//--------------------------------------------------------------
void	FieldCommFunc_Post_SelfData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
	//自分のデータも一応セットしておく(同期用に使う
	PLAYER_WORK *plWork;
	const FIELD_COMM_PLAYER_PACKET *pkt = (FIELD_COMM_PLAYER_PACKET*)pData;
	VecFx32 pos;
	u16 dir;
	
	//パケットデータを戻す
	pos.x = FX32_CONST( pkt->posX_ );
	pos.y = FX32_CONST( pkt->posY_ );
	pos.z = FX32_CONST( pkt->posZ_ );
	//dir	= pkt->dir_<<8;
	dir = pkt->dir_;

	ARI_TPrintf("POST[%d][%d][%d][%d][%d]\n",netID,pkt->posX_,pkt->posY_,pkt->posZ_,dir);
	
	//set
	plWork = FieldCommData_GetCharaData_PlayerWork(netID);
	PLAYERWORK_setPosition( plWork , &pos );
	PLAYERWORK_setDirection( plWork , dir );
	PLAYERWORK_setZoneID( plWork , pkt->zoneID_ );
	FieldCommData_SetCharaData_IsValid( netID , TRUE );

	ARI_TPrintf("FieldComm PostSelfData[%d]\n",netID);
}

//======================================================================
//各種コールバック
//======================================================================
//--------------------------------------------------------------
// 通信ライブラリ　初期化・開放用  
//--------------------------------------------------------------
void	FieldCommFunc_FinishInitCallback( void* pWork )
{
	FIELD_COMM_FUNC *commFunc = (FIELD_COMM_FUNC*)pWork;
	commFunc->isInitCommSystem_ = TRUE;
}
void	FieldCommFunc_FinishTermCallback( void* pWork )
{
	FIELD_COMM_FUNC *commFunc = (FIELD_COMM_FUNC*)pWork;
	commFunc->isInitCommSystem_ = FALSE;
}

//--------------------------------------------------------------
// ビーコンデータ取得関数  
//--------------------------------------------------------------
void*	FieldCommFunc_GetBeaconData(void)
{
	static u8 dummyData[2] = {1,27};
	return (void*)&dummyData;
}

//--------------------------------------------------------------
// ビーコンデータサイズ取得関数 
//--------------------------------------------------------------
int		FieldCommFunc_GetBeaconSize(void)
{
	return 2;
}

//--------------------------------------------------------------
// ビーコンのサービスを比較して繋いで良いかどうか判断する
//--------------------------------------------------------------
BOOL	FieldCommFunc_CheckConnectService(GameServiceID GameServiceID1 , GameServiceID GameServiceID2 )
{
	return TRUE;
}

//--------------------------------------------------------------
// 通信不能なエラーが起こった場合呼ばれる 切断するしかない
//--------------------------------------------------------------
void	FieldCommFunc_ErrorCallBack(GFL_NETHANDLE* pNet,int errNo, void* pWork)
{
}

//--------------------------------------------------------------
// 通信切断時に呼ばれる関数(終了時
//--------------------------------------------------------------
void	FieldCommFunc_DisconnectCallBack(void* pWork)
{
}

