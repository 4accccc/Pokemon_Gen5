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


#include "ari_comm_func.h"
#include "ari_comm_def.h"
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
//	FIELD_COMM_DATA
//======================================================================
struct _FIELD_COMM_DATA 
{
    u8	    seqNo;

    HEAPID   heapID;
    BOOL isFinishInitSystem;
    BOOL isError;
    BOOL isDutyMemberData;

    FIELD_COMM_TYPE commType;
    FIELD_COMM_MODE commMode;

    //仲間データ
    u8			memberNum;
    FIELD_COMM_BEACON	*memberData;

    GFL_NETHANDLE   *selfHandle;    //自身の通信ハンドル
};

//======================================================================
//	proto
//======================================================================
FIELD_COMM_DATA	*f_comm = NULL;

FIELD_COMM_DATA *FieldComm_InitData( u32 heapID );
BOOL	FieldComm_InitSystem();
void	FieldComm_TermSystem();
void	FieldComm_InitParent();
void	FieldComm_InitChild();

u8	FieldComm_UpdateSearchParent();
BOOL	FieldComm_GetSearchParentName( const u8 idx , STRBUF *name );
u8	FieldComm_GetMemberNum();
BOOL	FieldComm_GetMemberName( const u8 idx , STRBUF *name );

BOOL	FieldComm_IsValidParentData( u8 idx );

//送受信関係
void	FieldComm_SendSelfData();

//チェック関数
BOOL	FieldComm_IsFinish_InitSystem(); 
BOOL	FieldComm_IsFinish_ConnectParent();
BOOL	FieldComm_IsError();
BOOL	FieldComm_IsDutyMemberData();
void	FieldComm_ResetDutyMemberData();

//各種コールバック
void	InitCommLib_EndCallback( void* pWork );
void	FieldComm_ErrorCallBack(GFL_NETHANDLE* pNet,int errNo, void* pWork);
void	FieldComm_DisconnectCallBack(GFL_NETHANDLE* pNet);
void*	FieldComm_GetBeaconData(void);
int	FieldComm_GetBeaconSize(void);
BOOL	FieldComm_CheckConnectService(GameServiceID GameServiceID1, 
				      GameServiceID GameServiceID2);

//受信用コールバック
void FieldComm_Post_FirstBeacon(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
void FieldComm_Post_StartMode(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
void FieldComm_Post_PlayerData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);

static const NetRecvFuncTable FieldCommPostTable[] = {
    { FieldComm_Post_FirstBeacon , NULL },
    { FieldComm_Post_StartMode   , NULL },
    { FieldComm_Post_PlayerData  , NULL },
};
//--------------------------------------------------------------
/**
 * 通信用データ初期化
 * @param	heapID	ヒープID
 * @retval	FIELD_COMM_DATA
 */
//--------------------------------------------------------------
FIELD_COMM_DATA *FieldComm_InitData( u32 heapID )
{
    u8 i=0;

    f_comm = GFL_HEAP_AllocClearMemory( heapID , sizeof( FIELD_COMM_DATA ) );
    
    f_comm->seqNo  = 0;
    f_comm->heapID = heapID;
    f_comm->commType = FCT_CHILD;
    f_comm->commMode = FCM_2_SINGLE;
    f_comm->isFinishInitSystem = FALSE;
    f_comm->isError = FALSE;
    f_comm->isDutyMemberData = FALSE;

    f_comm->memberNum = 0;
    f_comm->memberData = GFL_HEAP_AllocClearMemory( heapID , sizeof( FIELD_COMM_BEACON ) * FIELD_COMM_MEMBER_MAX );
    for( i=0;i<FIELD_COMM_MEMBER_MAX;i++ )
    {
	f_comm->memberData[i].id = FIELD_COMM_ID_INVALID;
    }

    return f_comm;
}

//--------------------------------------------------------------
/**
 * 通信ライブラリ開放
 * @param	heapID	ヒープID
 * @retval	FIELD_COMM_DATA
 */
//--------------------------------------------------------------
void	FieldComm_TermSystem()
{
    if( f_comm == NULL ){
	OS_TPrintf("FieldComm System is not init.\n");
	return;
    }
    GFL_HEAP_FreeMemory( f_comm->memberData );
    GFL_HEAP_FreeMemory( f_comm );
    f_comm = NULL;
}

//--------------------------------------------------------------
/**
 * 通信ライブラリ初期化
 * @param	heapID	ヒープID
 * @retval	FIELD_COMM_DATA
 */
//--------------------------------------------------------------
BOOL	FieldComm_InitSystem()
{
    GFLNetInitializeStruct aGFLNetInit = {
  	FieldCommPostTable,	//NetSamplePacketTbl,  // 受信関数テーブル
	NELEMS(FieldCommPostTable), // 受信テーブル要素数
	NULL,	// ユーザー同士が交換するデータのポインタ取得関数
	NULL,	// ユーザー同士が交換するデータのサイズ取得関数
	FieldComm_GetBeaconData,	// ビーコンデータ取得関数  
	FieldComm_GetBeaconSize,	// ビーコンデータサイズ取得関数 
	FieldComm_CheckConnectService,	// ビーコンのサービスを比較して繋いで良いかどうか判断する
	FieldComm_ErrorCallBack,	// 通信不能なエラーが起こった場合呼ばれる 切断するしかない
	FieldComm_DisconnectCallBack,	// 通信切断時に呼ばれる関数
	NULL,	// オート接続で親になった場合
#if GFL_NET_WIFI
	NULL,NULL,NULL,NULL,
#endif //GFL_NET_WIFI
	0x444,	//ggid  DP=0x333,RANGER=0x178,WII=0x346
	0,  //下で設定してる //元になるheapid
	0,  //下で設定してる //通信用にcreateされるHEAPID
	0,  //下で設定してる //wifi用にGameServiceID GameServiceID1, GameServiceID GameServiceID2createされるHEAPID
	GFL_WICON_POSX,GFL_WICON_POSY,	// 通信アイコンXY位置
	4,//_MAXNUM,	//最大接続人数
	48,//_MAXSIZE,	//最大送信バイト数
	4,//_BCON_GET_NUM,  // 最大ビーコン収集数
	TRUE,	    // CRC計算
	FALSE,	    // MP通信＝親子型通信モードかどうか
	FALSE,	    //wifi通信を行うかどうか
	TRUE,	    // 親が再度初期化した場合、つながらないようにする場合TRUE
	1//WB_NET_FIELDMOVE_SERVICEID	//GameServiceID
    };

    aGFLNetInit.baseHeapID = f_comm->heapID;
    aGFLNetInit.netHeapID  = f_comm->heapID+1;
    aGFLNetInit.wifiHeapID = f_comm->heapID+2;

  

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
	FIELD_COMM_BEACON *b_data = GFL_NET_GetBeaconData( findNum );
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
    const FIELD_COMM_BEACON *b_data = GFL_NET_GetBeaconData( idx );
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
    const FIELD_COMM_BEACON *b_data = GFL_NET_GetBeaconData( idx );
    if( b_data == NULL ){ return FALSE; }
    else		{ return TRUE;  }
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
    void *data = FieldComm_GetBeaconData();
//  const BOOL ret = GFL_NET_SendData( f_comm->selfHandle , 
//		    FC_CMD_FIRST_BEACON , FieldComm_GetBeaconSize() , data );
    const BOOL ret = GFL_NET_SendDataEx( f_comm->selfHandle , 
		    GFL_NET_SENDID_ALLUSER , FC_CMD_FIRST_BEACON , FieldComm_GetBeaconSize() ,
		    data , FALSE , TRUE , FALSE );
    if( ret == FALSE ){ OS_TPrintf("FieldComm Data send is failued!!\n"); }

    #if 1
    {
    u8 i;
    FIELD_COMM_BEACON *beacon = (FIELD_COMM_BEACON*)data;
    OS_TPrintf("FieldComm SendSelfData Name");
    for(i=0;i<FIELD_COMM_NAME_LENGTH;i++)
	{OS_TPrintf("[%d]",beacon->name[i]);}
    OS_TPrintf(" id[%d]\n",beacon->id);
    }
    #endif
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
void	FieldComm_DisconnectCallBack(GFL_NETHANDLE* pNet)
{
    OS_TPrintf("FieldComm Disconnect!!\n");
    f_comm->isError = TRUE;
}

//--------------------------------------------------------------
/**
 *	各種ビーコン用コールバック関数 
 */
//--------------------------------------------------------------
void*	FieldComm_GetBeaconData(void)
{
    static FIELD_COMM_BEACON testData = {
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
int	FieldComm_GetBeaconSize(void)
{
    return sizeof( FIELD_COMM_BEACON );
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
        const FIELD_COMM_BEACON *postData = (FIELD_COMM_BEACON*)pData;
	OS_TPrintf("FieldComm getData[FirstBeacon]\n");
        OS_TPrintf("          id[%d]\n",postData->id);
    
	GFL_STD_MemCopy( pData , &f_comm->memberData[f_comm->memberNum] , sizeof( FIELD_COMM_BEACON ) );
//	f_comm->memberData[f_comm->memberNum] = postData;
	f_comm->memberNum++;
	f_comm->isDutyMemberData = TRUE;
    }

    #if 1
    {
    u8 i;
    FIELD_COMM_BEACON *beacon = (FIELD_COMM_BEACON*)pData;
    OS_TPrintf("          Name");
    for(i=0;i<FIELD_COMM_NAME_LENGTH;i++)
	{OS_TPrintf("[%d]",beacon->name[i]);}
    OS_TPrintf(" id[%d]\n",beacon->id);
    }
    #endif
}

void FieldComm_Post_StartMode(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    OS_TPrintf("FieldComm getData[StartMode]\n");
}

void FieldComm_Post_PlayerData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    OS_TPrintf("FieldComm getData[PlayerData]\n");
}



