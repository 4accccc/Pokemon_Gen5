//======================================================================
/**
 *
 * @file	ari_comm_func.c	
 * @brief	通信用機能郡
 * @author	ariizumi
 * @data	08.10.14
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "net/network_define.h"
#include "textprint.h"
#include "arc_def.h"
#include "system/main.h"

#include "ari_debug.h"
#include "ari_comm_func.h"
#include "ari_comm_def.h"
//======================================================================
//	define
//======================================================================
#define FIELD_COMM_PLAYERDATA_POST_BUFFER_NUM ( FIELD_COMM_MEMBER_MAX*5 )
#define FIELD_COMM_PLAYERDATA_POST_BUFFER_SIZE (sizeof(FIELD_COMM_PLAYER_DATA)*FIELD_COMM_PLAYERDATA_POST_BUFFER_NUM)

#define FIELD_COMM_CONV_DIR_TO_COMMDIR(v) (v/256)
#define FIELD_COMM_CONV_COMMDIR_TO_DIR(v) (v*256)
//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================
typedef struct 
{
	u32 posX:12;
	u32 posZ:12;
	u32 dir:8;
	u16 posY;
	u16 pad2;

	u32 testData[8];
}FIELD_COMM_PLAYER_DATA;
//======================================================================
//	FIELD_COMM_DATA_OLD
//======================================================================
struct _FIELD_COMM_DATA_OLD 
{
	u8		seqNo;

	HEAPID   heapID;
	BOOL isFinishInitSystem;
	BOOL isError;
	BOOL isDutyMemberData;
	BOOL isStartMode;

	FIELD_COMM_TYPE commType;
	FIELD_COMM_MODE commMode;

	//仲間データ
	u8			memberNum;
	FIELD_COMM_BEACON_OLD		*memberData;	//名前とID
	FIELD_COMM_PLAYER_DATA	*playerData;	//座標とか
	u8			isActiveUser[FIELD_COMM_MEMBER_MAX];	//ビットフラグで管理

	u8	playerDataPostBuffIdx;
	u8	*playerDataPostBuff;
	FIELD_COMM_PLAYER_DATA selfData;	//送信バッファ代わり

	GFL_NETHANDLE   *selfHandle;	//自身の通信ハンドル
};

//======================================================================
//	proto
//======================================================================
FIELD_COMM_DATA_OLD	*f_comm = NULL;

FIELD_COMM_DATA_OLD *FieldComm_InitData( u32 heapID );
BOOL	FieldComm_InitSystem();
void	FieldComm_TermSystem();
void	FieldComm_InitParent();
void	FieldComm_InitChild();

u8		FieldComm_UpdateSearchParent();
BOOL	FieldComm_GetSearchParentName( const u8 idx , STRBUF *name );
u8		FieldComm_GetMemberNum();
BOOL	FieldComm_GetMemberName( const u8 idx , STRBUF *name );
u8		FieldComm_GetSelfIndex();

BOOL	FieldComm_IsValidParentData( u8 idx );

//送受信関係
void	FieldComm_SendSelfData();
void	FieldComm_SendStartMode();
void	FieldComm_SendSelfPosition( const VecFx32 *pos , const u16 *dir);
void	FieldComm_PostPlayerPosition( const u8 id , VecFx32 *pos , u16 *dir);

//チェック関数
BOOL	FieldComm_IsFinish_InitSystem(); 
BOOL	FieldComm_IsFinish_ConnectParent();
BOOL	FieldComm_IsError();
BOOL	FieldComm_IsDutyMemberData();
void	FieldComm_ResetDutyMemberData();
BOOL	FieldComm_IsStartCommMode();
BOOL	FieldComm_IsActiveUser(const u8 idx);
void	FieldComm_ResetActiveUser(const u8 idx);
void	FieldComm_SetActiveUser(const u8 idx);

//各種コールバック
void	InitCommLib_EndCallback( void* pWork );
void	FieldComm_ErrorCallBack(GFL_NETHANDLE* pNet,int errNo, void* pWork);
void	FieldComm_DisconnectCallBack(void* pWork);
void*	FieldComm_GetBeaconData(void* pWork);
int		FieldComm_GetBeaconSize(void* pWork);
BOOL	FieldComm_CheckConnectService(GameServiceID GameServiceID1, 
					  GameServiceID GameServiceID2);

//受信用コールバック
void FieldComm_Post_FirstBeacon(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
void FieldComm_Post_StartMode(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
void FieldComm_Post_PlayerData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);

u8*	 FieldComm_Post_PlayerData_Buff( int netID, void* pWork , int size );

static const NetRecvFuncTable FieldCommPostTable[] = {
	{ FieldComm_Post_FirstBeacon , NULL },
	{ FieldComm_Post_StartMode   , NULL },
	{ FieldComm_Post_PlayerData  , FieldComm_Post_PlayerData_Buff},
};
//--------------------------------------------------------------
/**
 * 通信用データ初期化
 * @param	heapID	ヒープID
 * @retval	FIELD_COMM_DATA_OLD
 */
//--------------------------------------------------------------
FIELD_COMM_DATA_OLD *FieldComm_InitData( u32 heapID )
{
	u8 i=0;

	f_comm = GFL_HEAP_AllocClearMemory( heapID , sizeof( FIELD_COMM_DATA_OLD ) );
	
	f_comm->seqNo  = 0;
	f_comm->heapID = heapID;
	f_comm->commType = FCT_CHILD;
	f_comm->commMode = FCM_2_SINGLE;
	f_comm->isFinishInitSystem = FALSE;
	f_comm->isError = FALSE;
	f_comm->isDutyMemberData = FALSE;
	f_comm->isStartMode = FALSE;

	f_comm->memberNum = 0;
	f_comm->memberData = GFL_HEAP_AllocClearMemory( heapID , sizeof( FIELD_COMM_BEACON_OLD ) * FIELD_COMM_MEMBER_MAX );
	f_comm->playerData = GFL_HEAP_AllocClearMemory( heapID , sizeof( FIELD_COMM_PLAYER_DATA	) * FIELD_COMM_MEMBER_MAX );
	for( i=0;i<FIELD_COMM_MEMBER_MAX;i++ )
	{
		f_comm->memberData[i].id = FIELD_COMM_ID_INVALID;
		f_comm->isActiveUser[i] = FALSE;
	}
	f_comm->playerDataPostBuff = GFL_HEAP_AllocClearMemory( heapID , FIELD_COMM_PLAYERDATA_POST_BUFFER_SIZE );
	f_comm->playerDataPostBuffIdx = 0;

	return f_comm;
}

//--------------------------------------------------------------
/**
 * 通信ライブラリ開放
 * @param	heapID	ヒープID
 * @retval	FIELD_COMM_DATA_OLD
 */
//--------------------------------------------------------------
void	FieldComm_TermSystem()
{
	if( f_comm == NULL ){
		OS_TPrintf("FieldComm System is not init.\n");
		return;
	}
	GFL_HEAP_FreeMemory( f_comm->playerDataPostBuff);
	GFL_HEAP_FreeMemory( f_comm->playerData );
	GFL_HEAP_FreeMemory( f_comm->memberData );
	GFL_HEAP_FreeMemory( f_comm );
	f_comm = NULL;
}

//--------------------------------------------------------------
/**
 * 通信ライブラリ初期化
 * @param	heapID	ヒープID
 * @retval	FIELD_COMM_DATA_OLD
 */
//--------------------------------------------------------------
BOOL	FieldComm_InitSystem()
{
	GFLNetInitializeStruct aGFLNetInit = {
		FieldCommPostTable,	//NetSamplePacketTbl,  // 受信関数テーブル
		NELEMS(FieldCommPostTable), // 受信テーブル要素数
        NULL,    ///< ハードで接続した時に呼ばれる
        NULL,    ///< ネゴシエーション完了時にコール
        NULL,	// ユーザー同士が交換するデータのポインタ取得関数
		NULL,	// ユーザー同士が交換するデータのサイズ取得関数
		FieldComm_GetBeaconData,	// ビーコンデータ取得関数  
		FieldComm_GetBeaconSize,	// ビーコンデータサイズ取得関数 
		FieldComm_CheckConnectService,	// ビーコンのサービスを比較して繋いで良いかどうか判断する
		FieldComm_ErrorCallBack,   ///< 通信不能なエラーが起こった場合呼ばれる 通信を終了させる必要がある
        NULL,  ///< 通信不能エラーが発生。 電源切断する必要がある
		FieldComm_DisconnectCallBack,	// 通信切断時に呼ばれる関数
		NULL,	// オート接続で親になった場合
#if GFL_NET_WIFI
    NULL,     ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
    NULL, ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
    NULL,  ///< wifiフレンドリスト削除コールバック
    NULL,   ///< DWC形式の友達リスト	
    NULL,  ///< DWCのユーザデータ（自分のデータ）
    TRUE,        ///< デバック用サーバにつなぐかどうか
#endif  //GFL_NET_WIFI
		0x444,	//ggid  DP=0x333,RANGER=0x178,WII=0x346
		GFL_HEAPID_APP,  //元になるheapid
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

	//aGFLNetInit.baseHeapID = GFL_HEAPID_APP;
	//aGFLNetInit.netHeapID  = GFL_HEAPID_APP;
	//aGFLNetInit.wifiHeapID = GFL_HEAPID_APP;

  

	GFL_NET_Init( &aGFLNetInit , InitCommLib_EndCallback , NULL );
	return TRUE;
}

//--------------------------------------------------------------
/**
 *  親機接続初期化 
 */
//--------------------------------------------------------------
void	FieldComm_InitParent()
{
	f_comm->commType = FCT_PARENT;
	GFL_NET_InitServer();
}

//--------------------------------------------------------------
/**
 *  子機接続初期化 
 */
//--------------------------------------------------------------
void	FieldComm_InitChild()
{
	f_comm->commType = FCT_CHILD;
	GFL_NET_StartBeaconScan();
}

//--------------------------------------------------------------
/**
 *  親機探しアップデート 
 * @param	
 * @retval	int 親機候補数
 */
//--------------------------------------------------------------
u8	FieldComm_UpdateSearchParent()
{   
	int findNum = 0;

	while(TRUE){
		// TODO:データは変更されるかもしれないので、ここでローカルに保存しておくべきか？
		FIELD_COMM_BEACON_OLD *b_data = GFL_NET_GetBeaconData( findNum );
		if( b_data != NULL ){
		findNum++;
	}
	else break;
	}
	
	OS_TPrintf("ARI_COMM FindParent [%d]\n",findNum);
	return findNum;
}

//--------------------------------------------------------------
/**
 *  親機探し 名前取得 FieldComm_GetBeaconSize
 * @param	const u8 インデックス
 * @param	STRBUF* 格納場所
 * @retval	int 親機候補数
 */
//--------------------------------------------------------------
BOOL	FieldComm_GetSearchParentName( const u8 idx , STRBUF *name )
{
	const FIELD_COMM_BEACON_OLD *b_data = GFL_NET_GetBeaconData( idx );
	if( b_data == NULL )
	{
		OS_TPrintf("FieldComm Parent data not found!! idx[%d]\n",idx);
		return FALSE;
	}
	GFL_STR_SetStringCode( name , b_data->name );
	return TRUE;
}

//--------------------------------------------------------------
/**
 *  親機探し データの有効性取得 
 * @param	const u8 データインデックス
 * @retval	BOOL 有効・無効
 */
//--------------------------------------------------------------
BOOL	FieldComm_IsValidParentData( const u8 idx )
{
	const FIELD_COMM_BEACON_OLD *b_data = GFL_NET_GetBeaconData( idx );
	if( b_data == NULL ){ return FALSE; }
	else				{ return TRUE;  }
}

//--------------------------------------------------------------
/**
 * メンバー数取得
 * @param	
 * @retval	
 */
//--------------------------------------------------------------
u8	FieldComm_GetMemberNum()
{
	return f_comm->memberNum;
}

//--------------------------------------------------------------
/**
 * メンバー名取得
 * @param	u8  インデックス
 * @param	STRBUF　名前格納場所
 * @retval	BOOL	データの有効性
 */
//--------------------------------------------------------------
BOOL	FieldComm_GetMemberName( const u8 idx , STRBUF *name )
{
	if( f_comm->memberData[idx].id == FIELD_COMM_ID_INVALID )
	{
		OS_TPrintf("FieldComm Member data not found!! idx[%d]\n",idx);
		return FALSE;
	}
	GFL_STR_SetStringCode( name , f_comm->memberData[idx].name );
	return TRUE;
}

//自機番号の取得
u8		FieldComm_GetSelfIndex()
{
	return GFL_NET_GetNetID( f_comm->selfHandle )-1;
}



//--------------------------------------------------------------
/**
 *  親機探し 親機に接続 
 * @param	const u8 インデックス
 * @retval	int 親機候補数
 */
//--------------------------------------------------------------
void	FieldComm_ConnectParent( u8 idx )
{
	u8* macAddress = GFL_NET_GetBeaconMacAddress( idx );
	if( macAddress == NULL ){
		GF_ASSERT("macAddress is not found!!");
	}
	GFL_NET_ConnectToParent( macAddress );
}


//--------------------------------------------------------------
//送受信関係
//--------------------------------------------------------------
//自身のデータ(名前・ID)を送る。ビーコンの送信と同等のデータとする
void	FieldComm_SendSelfData()
{
	//とりあえずビーコンを送信
	void *data = FieldComm_GetBeaconData(NULL);
//	const BOOL ret = GFL_NET_SendData( f_comm->selfHandle , 
//	FC_CMD_FIRST_BEACON , FieldComm_GetBeaconSize() , data );
	const BOOL ret = GFL_NET_SendDataEx( f_comm->selfHandle , 
			GFL_NET_SENDID_ALLUSER , FC_CMD_FIRST_BEACON , FieldComm_GetBeaconSize(NULL) ,
			data , FALSE , TRUE , FALSE );
	if( ret == FALSE ){ OS_TPrintf("FieldComm Data send is failued!!\n"); }

}

//通信モードの開始 現状フラグのみ
void	FieldComm_SendStartMode()
{
	static const u8 dummy = 0;
	f_comm->selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
	{
		const BOOL ret = GFL_NET_SendDataEx( f_comm->selfHandle , 
				GFL_NET_SENDID_ALLUSER , FC_CMD_START_MODE , sizeof(u8) ,
				&dummy , FALSE , TRUE , FALSE );
		if( ret == FALSE ){ OS_TPrintf("FieldComm Data send is failued!!\n"); }
	}
}

//自分の座標の送信
void FieldComm_SendSelfPosition( const VecFx32 *pos , const u16 *dir )
{
	f_comm->selfData.posX = (u16)(FX_FX32_TO_F32(pos->x));
	f_comm->selfData.posZ = (u16)(FX_FX32_TO_F32(pos->z));
	f_comm->selfData.posY = (u16)(FX_FX32_TO_F32(pos->y));
	
	f_comm->selfData.dir = FIELD_COMM_CONV_DIR_TO_COMMDIR(*dir);

	{
		const BOOL ret = GFL_NET_SendDataEx( f_comm->selfHandle , 
				GFL_NET_SENDID_ALLUSER , FC_CMD_PLAYER_DATA , sizeof(FIELD_COMM_PLAYER_DATA) ,
				&f_comm->selfData , FALSE , TRUE , TRUE );
		if( ret == FALSE ){ OS_TPrintf("FieldComm PlayerData send is failued!!\n"); }
	}

}

//座標の取得
void FieldComm_PostPlayerPosition( const u8 id , VecFx32 *pos , u16 *dir)
{
	const FIELD_COMM_PLAYER_DATA *data = &f_comm->playerData[id];
	
	pos->x = FX32_CONST( data->posX );
	pos->y = FX32_CONST( data->posY );
	pos->z = FX32_CONST( data->posZ );
	*dir   = FIELD_COMM_CONV_COMMDIR_TO_DIR(data->dir);
}

//--------------------------------------------------------------
//	各種チェック関数
//--------------------------------------------------------------
BOOL	FieldComm_IsFinish_InitSystem()
{
	return f_comm->isFinishInitSystem;
}

//  親機への接続が完了し、通信ができる状態になったか？
BOOL	FieldComm_IsFinish_ConnectParent()
{
	switch( f_comm->seqNo )
	{
	case 0:
		if( GFL_NET_HANDLE_RequestNegotiation() == TRUE )
		{
			f_comm->seqNo++;
		}
		break;
	case 1:
		f_comm->selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
		if( GFL_NET_HANDLE_IsNegotiation( f_comm->selfHandle ) == TRUE )
		{
			f_comm->seqNo++;
		}
		break;
	case 2:
		if( GFL_NET_IsSendEnable( f_comm->selfHandle ) == TRUE )
		{
			f_comm->seqNo = 0;
			return TRUE;
		}
		break;
	}
	return FALSE;
}

BOOL	FieldComm_IsError()
{
	return f_comm->isError;
}

//メンバーデータに更新があったか？
BOOL	FieldComm_IsDutyMemberData()
{
	return f_comm->isDutyMemberData;
}
void	FieldComm_ResetDutyMemberData()
{
	f_comm->isDutyMemberData = FALSE;
}

//通信モードが開始しているか？
BOOL	FieldComm_IsStartCommMode()
{
	if( f_comm == NULL ){return FALSE;}
	return f_comm->isStartMode;
}

//ユーザーがアクティブ(データ更新があったか？)
BOOL	FieldComm_IsActiveUser(const u8 idx)
{
	return f_comm->isActiveUser[idx];
}
void	FieldComm_ResetActiveUser( const u8 idx )
{
	f_comm->isActiveUser[idx] = FALSE;
}
void	FieldComm_SetActiveUser( const u8 idx )
{
	f_comm->isActiveUser[idx] = TRUE;
}
//--------------------------------------------------------------
//	以下、各種コールバック
//--------------------------------------------------------------

//--------------------------------------------------------------
/**
 * 通信ライブラリ初期化完了用コールバック
 * @param	void	*pWork(*FIELC_COMM_DATA)
 * @retval	void
 */
//--------------------------------------------------------------
void	InitCommLib_EndCallback( void* pWork )
{
	f_comm->isFinishInitSystem = TRUE;
}

//エラー取得コールバック
void	FieldComm_ErrorCallBack(GFL_NETHANDLE* pNet,int errNo, void* pWork)
{
	OS_TPrintf("FieldComm Error!![%d]\n",errNo);
	f_comm->isError = TRUE;
}

//切断感知用コールバック
void	FieldComm_DisconnectCallBack(void *pWork)
{
	OS_TPrintf("FieldComm Disconnect!!\n");
	f_comm->isError = TRUE;
}

//--------------------------------------------------------------
/**
 *	各種ビーコン用コールバック関数 
 */
//--------------------------------------------------------------
void*	FieldComm_GetBeaconData(void* pWork)
{
	static FIELD_COMM_BEACON_OLD testData = {
		L"てすとよう" ,
		0x8000
	};

	OSOwnerInfo dsData;
	u8 i;
	OS_GetOwnerInfo( &dsData );
	
	testData.id = 0;
	for( i=0;i<5;i++ )
	{
		testData.name[i] = dsData.nickName[i];
	}
	testData.id = (testData.id+dsData.nickName[i])%FIELD_COMM_ID_MAX;
	//testData.id = FIELD_COMM_ID_INVALID;
	testData.name[5] = 0xFFFF;
	
	return (void*)&testData;
}
int	FieldComm_GetBeaconSize(void* pWork)
{
	return sizeof( FIELD_COMM_BEACON_OLD );
}
BOOL	FieldComm_CheckConnectService(GameServiceID GameServiceID1, 
									  GameServiceID GameServiceID2)
{
	if( GameServiceID1 == GameServiceID2 ){ return TRUE; }
	return FALSE;
}


//受信用コールバック
void FieldComm_Post_FirstBeacon(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
	//ここでは、親機がつながった子機の情報を収集するだけの物
	if( IS_PARENT ){
		const FIELD_COMM_BEACON_OLD *postData = (FIELD_COMM_BEACON_OLD*)pData;
		OS_TPrintf("FieldComm getData[FirstBeacon]\n");
		OS_TPrintf("          id[%d]\n",postData->id);
	
		GFL_STD_MemCopy( pData , &f_comm->memberData[f_comm->memberNum] , sizeof( FIELD_COMM_BEACON_OLD ) );
//		f_comm->memberData[f_comm->memberNum] = postData;
		f_comm->memberNum++;
		f_comm->isDutyMemberData = TRUE;
	}

}

void FieldComm_Post_StartMode(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
	OS_TPrintf("FieldComm getData[StartMode]\n");
	f_comm->isStartMode = TRUE;
}

void FieldComm_Post_PlayerData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
//	OS_TPrintf("FieldComm getData[PlayerData]\n");
	
	if( netID == 0 ) return;
		//netID=0はとりあえず考慮しない
	GFL_STD_MemCopy( pData , &f_comm->playerData[netID-1] , sizeof( FIELD_COMM_PLAYER_DATA ) );
	
	ARI_TPrintf("FieldComm PostData id[%d] pos[%d][%d][%d]\n",netID-1
			,f_comm->playerData[netID-1].posX
			,f_comm->playerData[netID-1].posY
			,f_comm->playerData[netID-1].posZ);
	FieldComm_SetActiveUser( netID-1 );
}

u8*	 FieldComm_Post_PlayerData_Buff( int netID, void* pWork , int size )
{
	u8* pPos = f_comm->playerDataPostBuff + f_comm->playerDataPostBuffIdx*sizeof( FIELD_COMM_PLAYER_DATA);	
	
//	ARI_TPrintf("FieldComm PostBuff id[%d] size[%d]->set[%d]\n",netID,size,f_comm->playerDataPostBuffIdx);

	f_comm->playerDataPostBuffIdx++;
	if( f_comm->playerDataPostBuffIdx >= FIELD_COMM_PLAYERDATA_POST_BUFFER_NUM ){
		f_comm->playerDataPostBuffIdx = 0;
	}

	return pPos;
}


