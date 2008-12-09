
#include "common_def.h"
#include "ds_net.h"
#include "network/net_system.h"
#include "draw.h"
//#include "net_command.h"
//DSからの解析とか・返事とかを行う
MATHCRC16Table *crcTable;
PLAYER_STATE	plState_[MY_MAX_NODES];


//送受信関数用
typedef enum 
{
    FC_CMD_SELFDATA = GFL_NET_CMD_COMMAND_MAX,  //自機データ
    FC_CMD_REQUEST_DATA,    //データ要求
    FC_CMD_SELF_PROFILE,    //キャラの基本情報
    FC_CMD_COMMON_FLG,      //汎用フラグ
    FC_CMD_USERDATA,        //行動用ユーザーデータ
    FC_CMD_MAX,
}F_COMM_COMMAND_TYPE;

static void RecvTestA(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void RecvTestA(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{ NET_PRINT( "CallFunc A\n" ); }

static void FieldComm_Post_CharaData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);

static const NetRecvFuncTable FieldCommPostTable[FC_CMD_MAX] = {
	{FieldComm_Post_CharaData,			NULL},
	{RecvTestA,							NULL},
	{RecvTestA,							NULL},
	{RecvTestA,							NULL},
	{RecvTestA,							NULL},
};

static const GFLNetInitializeStruct netInitStruct =
{
	FieldCommPostTable,	//NetSamplePacketTbl,  // 受信関数テーブル
	FC_CMD_MAX,// 受信テーブル要素数
    NULL,    ///< ハードで接続した時に呼ばれる
    NULL,    ///< ネゴシエーション完了時にコール
    NULL,	// ユーザー同士が交換するデータのポインタ取得関数
	NULL,	// ユーザー同士が交換するデータのサイズ取得関数
	NULL,	// ビーコンデータ取得関数  
	NULL,	// ビーコンデータサイズ取得関数 
	NULL,	// ビーコンのサービスを比較して繋いで良いかどうか判断する
	NULL,   ///< 通信不能なエラーが起こった場合呼ばれる 通信を終了させる必要がある
    NULL,  ///< 通信不能エラーが発生。 電源切断する必要がある
	NULL,	// 通信切断時に呼ばれる関数
	NULL,	// オート接続で親になった場合
	0x346,	//ggid  DP=0x333,RANGER=0x178,WII=0x346
	0,  //元になるheapid
	0,  //通信用にcreateされるHEAPID
	0,  //wifi用にcreateされるHEAPID
	0,0,	// 通信アイコンXY位置
	MY_MAX_NODES,//_MAXNUM,	//最大接続人数
	MY_DS_LENGTH,//_MAXSIZE,	//最大送信バイト数
	4,//_BCON_GET_NUM,  // 最大ビーコン収集数
	TRUE,		// CRC計算
	FALSE,		// MP通信＝親子型通信モードかどうか
	FALSE,		//wifi通信を行うかどうか
	TRUE,		// 親が再度初期化した場合、つながらないようにする場合TRUE
	30//WB_NET_FIELDMOVE_SERVICEID	//GameServiceID
	
};

void	MPDS_Init(void)
{
	HEAPID heapID = 0;
	u8 i;
	
	for( i=0;i<MY_MAX_NODES;i++ )
	{
		MPDS_InitPlayerState(i);
	}
	
	crcTable = (MATHCRC16Table*)mpAlloc(sizeof(MATHCRC16Table));
	MATH_CRC16CCITTInitTable( crcTable );
	GFL_NET_Init( &netInitStruct,NULL,NULL);
	_commSystemInit( MY_DS_LENGTH , heapID );

}

const BOOL MPDS_PostShareData( const u8 *adr , const u8 aid )
{
}

void* GFL_HEAP_AllocClearMemory(int work, int size)
{
	void *adr = mpAlloc((u32)size);
	NETMemSet(adr,0,(u32)size);
	return adr;
}

//毎フレーム共有情報
typedef struct
{
	u16 posX_;
	u16 posZ_;
	s8  posY_;      //マックス不明なので。場合によってはなしでOK？
	u8  dir_;       //グリッドなので上下左右が0～3で入る
	u16 zoneID_;    //ここは通信用のIDとして変換して抑えられる
	u8  talkState_;
}FIELD_COMM_PLAYER_PACKET;

static void FieldComm_Post_CharaData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
	//自分のデータも一応セットしておく(同期用に使う
	const FIELD_COMM_PLAYER_PACKET *pkt = (FIELD_COMM_PLAYER_PACKET*)pData;
	u16 tempX,tempY,tempZ;
	const u8 dirTable[2][4]=
	{
		{ ANIM_UP_STOP,ANIM_DOWN_STOP,ANIM_LEFT_STOP,ANIM_RIGHT_STOP, },
		{ ANIM_UP_WALK,ANIM_DOWN_WALK,ANIM_LEFT_WALK,ANIM_RIGHT_WALK, },
	};
	PLAYER_STATE	*plData = MPDS_GetPlayerState( netID );
	
//	OSReport("POST[%d][%d][%d][%d][%d]\n",netID,pkt->posX_,pkt->posY_,pkt->posZ_,pkt->dir_);
EnterCriticalSection();
	
	plData->isValid_ = TRUE;
	
	//座標系が違うので注意！
	tempX = NETReadSwappedBytes16( &pkt->posX_ );
	tempY = NETReadSwappedBytes16( &pkt->posZ_ );
	tempZ = 0;
//	tempZ = pkt->posY_;

	if( tempX != plData->posX_ ||
		tempY != plData->posY_ ||
		tempZ != plData->posZ_ )
	{
		plData->newAnim_ = dirTable[1][pkt->dir_];
	}
	else
	{
		plData->newAnim_ = dirTable[0][pkt->dir_];
	}
	plData->posX_ = tempX;
	plData->posY_ = tempY;
	plData->posZ_ = tempZ;

LeaveCriticalSection();
}

PLAYER_STATE*	MPDS_GetPlayerState( const u8 idx )
{
	return &plState_[idx];
}

void		MPDS_InitPlayerState( const u8 idx )
{
	plState_[idx].isValid_ = FALSE;
	plState_[idx].oldAnim_ = ANIM_TYPE_MAX;
	plState_[idx].newAnim_ = ANIM_TYPE_MAX;
}
