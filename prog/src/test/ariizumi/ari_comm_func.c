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
    HEAPID   heapID;
    BOOL isFinishInitSystem;

    FIELD_COMM_TYPE commType;
    FIELD_COMM_MODE commMode;
};

//======================================================================
//	proto
//======================================================================
FIELD_COMM_DATA	*f_comm = NULL;

FIELD_COMM_DATA *FieldComm_InitData( u32 heapID );
BOOL	FieldComm_InitSystem();
void	FieldComm_InitParent();
void	FieldComm_InitChild();

int	fieldComm_UpdateSearchParent();

//チェック関数
BOOL	FieldComm_IsFinish_InitSystem(); 

//各種コールバック
void	InitCommLib_EndCallback( void* pWork );
void*	FieldComm_GetBeaconData(void);
int	FieldComm_GetBeaconSize(void);
BOOL	FieldComm_CheckConnectService(GameServiceID GameServiceID1, 
				      GameServiceID GameServiceID2);

//--------------------------------------------------------------
/**
 * 通信用データ初期化
 * @param	heapID	ヒープID
 * @retval	FIELD_COMM_DATA
 */
//--------------------------------------------------------------
FIELD_COMM_DATA *FieldComm_InitData( u32 heapID )
{
    f_comm = GFL_HEAP_AllocClearMemory( heapID , sizeof( FIELD_COMM_DATA ) );

    f_comm->heapID = heapID;
    f_comm->commType = FCT_CHILD;
    f_comm->commMode = FCM_2_SINGLE;
    f_comm->isFinishInitSystem = FALSE;

    return f_comm;
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
  	NULL,	//NetSamplePacketTbl,  // 受信関数テーブル
	1,	//NELEMS(_CommPacketTbl), // 受信テーブル要素数
	NULL,	// ユーザー同士が交換するデータのポインタ取得関数
	NULL,	// ユーザー同士が交換するデータのサイズ取得関数
	FieldComm_GetBeaconData,	// ビーコンデータ取得関数  
	FieldComm_GetBeaconSize,	// ビーコンデータサイズ取得関数 
	FieldComm_CheckConnectService,	// ビーコンのサービスを比較して繋いで良いかどうか判断する
	NULL,	//FatalError_Disp,  // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
	NULL,	// 通信切断時に呼ばれる関数
	NULL,	// オート接続で親になった場合
	0x666,	//ggid  DP=0x333,RANGER=0x178,WII=0x346
	0,	    //元になるheapid
	0,   //通信用にcreateされるHEAPID
	0,   //wifi用にGameServiceID GameServiceID1, GameServiceID GameServiceID2createされるHEAPID
	GFL_WICON_POSX,GFL_WICON_POSY,	// 通信アイコンXY位置
	4,//_MAXNUM,	//最大接続人数
	32,//_MAXSIZE,	//最大送信バイト数
	4,//_BCON_GET_NUM,  // 最大ビーコン収集数
	TRUE,	    // CRC計算
	FALSE,	    // MP通信＝親子型通信モードかどうか
	FALSE,	    //wifi通信を行うかどうか
	TRUE,	    // 親が再度初期化した場合、つながらないようにする場合TRUE
	1//WB_NET_FIELDMOVE_SERVICEID	//GameServiceID
    };

    aGFLNetInit.baseHeapID = f_comm->heapID;
    aGFLNetInit.netHeapID = f_comm->heapID+1;
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
int	FieldComm_UpdateSearchParent()
{   
    int findNum = 0;

    while(TRUE){
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
//	各種チェック関数
//--------------------------------------------------------------
BOOL	FieldComm_IsFinish_InitSystem()
{
    return f_comm->isFinishInitSystem;
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



//--------------------------------------------------------------
/**
 *	各種ビーコン用コールバック関数 
 */
//--------------------------------------------------------------
void*	FieldComm_GetBeaconData(void)
{
    static FIELD_COMM_BEACON testData = {
	{ 0,1,2,3,4,5 } ,
	0x8000
    };
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
