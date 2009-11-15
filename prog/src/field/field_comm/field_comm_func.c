//======================================================================
/**
 * @file  field_comm_func.c
 * @brief フィールド通信　通信機能部
 * @author  ariizumi
 * @data  08/11/13
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/net_err.h"
#include "net/network_define.h"

#include "infowin/infowin.h"
#include "gamesystem/playerwork.h"
#include "field_comm_main.h"
#include "field_comm_func.h"
#include "field_comm_data.h"
#include "field/field_comm/field_comm_sys.h"
#include "test/ariizumi/ari_debug.h"
#include "field/game_beacon_search.h"
#include "fieldmap/zone_id.h"

//======================================================================
//  define
//======================================================================
//通信のパケットバッファ数
#define F_COMM_PACKET_BUFF_NUM (10)

//======================================================================
//  enum
//======================================================================
//送受信関数用
typedef enum
{
    FC_CMD_SELFDATA = GFL_NET_CMD_FIELD,  //自機データ
  FC_CMD_REQUEST_DATA,  //データ要求
  FC_CMD_SELF_PROFILE,  //キャラの基本情報
  FC_CMD_COMMON_FLG,    //汎用フラグ
  FC_CMD_USERDATA,    //行動用ユーザーデータ
  FC_CMD_EXIT_REQ,    //切断リクエスト
}F_COMM_COMMAND_TYPE;

//パケットバッファの送信タイプ
typedef enum
{
  FCPBT_FLG,
  FCPBT_REQ_DATA,

  FCPBT_INVALID,
}F_COMM_PACKET_BUFF_TYPE;

///同期取り番号
enum{
  FCF_TIMING_EXIT = 20,
};

//======================================================================
//  typedef struct
//======================================================================
//毎フレーム共有情報
typedef struct
{
  u16 posX_;
  u16 posZ_;
  s8  posY_;    //マックス不明なので。場合によってはなしでOK？
  u8  dir_;   //グリッドなので上左下右が0～3で入る(方向自体は0x4000単位
  u16 zoneID_;  //ここは通信用のIDとして変換して抑えられる
  u8  talkState_;
  u8  palace_area;  ///<※check　field_commのシステムに乗っかっている現状、ここにパレスのデータも入れる。別に作成した場合、ここから消す
  u8  mission_no;
  u8  padding;
}FIELD_COMM_PLAYER_PACKET;

//再送信用パケット情報
typedef struct
{
  u8  type_;
  u8  id_;
  u16 value_;
  u16 value2_;
  u8  sendID_;
}FIELD_COMM_PACKET_BUFF_DATA;

struct _FIELD_COMM_FUNC
{
  HEAPID  heapID_;
  FIELD_COMM_MODE commMode_;
  u8  seqNo_;

  BOOL  isInitCommSystem_;

  //送受信サポート関係
  FIELD_COMM_PLAYER_PACKET plPkt_;

  F_COMM_SYNC_TYPE  sendSyncType_;
  F_COMM_SYNC_TYPE  postSyncType_[FIELD_COMM_MEMBER_MAX]; //同期通信用
  //再送信用関数
  u8  pktBuffStart_;
  u8  pktBuffEnd_;
  FIELD_COMM_PACKET_BUFF_DATA pktBuffData_[F_COMM_PACKET_BUFF_NUM];

  //会話関係
  u8    talkID_;    //会話対象
  u16    talkPosX_;
  u16    talkPosZ_;    //会話対象位置

  //行動選択系
  u8    selectAction_;    //親が選んだ行動
  BOOL  isActionReturn_;  //行動選択の返事が届いたか？
  BOOL  actionReturn_;    //行動選択の結果

  BOOL  isGetUserData_;
  
  GBS_BEACON send_beacon;   ///<送信ビーコンバッファ
};
//ビーコン
typedef struct
{
  u8  mode_:1;  //0:待機 1:探索
  u8  memberNum_:3; //人数

  u8  pad_:4;
}FIELD_COMM_BEACON;
//======================================================================
//  proto
//======================================================================

static  void FIELD_COMM_FUNC_UpdateSearchParent( FIELD_COMM_FUNC *commFunc );
static  u8  FIELD_COMM_FUNC_CompareBeacon( const FIELD_COMM_BEACON *firstBcn , const FIELD_COMM_BEACON *secondBcn );

//各種チェック関数
static const BOOL FIELD_COMM_FUNC_Send_RequestDataFunc( const u8 charaIdx , const F_COMM_REQUEST_TYPE reqType , const BOOL isReSend , FIELD_COMM_FUNC *commFunc );
static const BOOL FIELD_COMM_FUNC_Send_CommonFlgFunc( const F_COMM_COMMON_FLG flg , const u16 val , const u16 val2, const u8 sendID , BOOL isReSend , FIELD_COMM_FUNC *commFunc );

//各種コールバック
void  FIELD_COMM_FUNC_FinishInitCallback( void* pWork );
void  FIELD_COMM_FUNC_FinishTermCallback( void* pWork );
void* FIELD_COMM_FUNC_GetBeaconData(void* pWork);   // ビーコンデータ取得関数
static void* FIELD_COMM_FUNC_GetBeaconData_CommFunc(FIELD_COMM_FUNC *commFunc);
BOOL  FIELD_COMM_FUNC_CheckConnectService(GameServiceID GameServiceID1 , GameServiceID GameServiceID2 ); // ビーコンのサービスを比較して繋いで良いかどうか判断する
void  FIELD_COMM_FUNC_ErrorCallBack(GFL_NETHANDLE* pNet,int errNo, void* pWork);    // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
void  FIELD_COMM_FUNC_DisconnectCallBack(void* pWork);  // 通信切断時に呼ばれる関数(終了時

const BOOL  FIELD_COMM_FUNC_SendPacketBuff( FIELD_COMM_FUNC *commFunc );
void  FIELD_COMM_FUNC_SetPacketBuff( const F_COMM_PACKET_BUFF_TYPE type , const u8 id , const u16 value , const u16 val2, const u8 sendID , FIELD_COMM_FUNC *commFunc );
const BOOL  FIELD_COMM_FUNC_SendPacketBuff( FIELD_COMM_FUNC *commFunc );


void  FIELD_COMM_FUNC_Post_SelfData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
void  FIELD_COMM_FUNC_Post_RequestData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
void  FIELD_COMM_FUNC_Post_SelfProfile( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
void  FIELD_COMM_FUNC_Post_CommonFlg( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
void  FIELD_COMM_FUNC_Post_UserData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
static void  FIELD_COMM_FUNC_Post_ExitReq( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
u8*   FIELD_COMM_FUNC_Post_UserData_Buff( int netID, void* pWork , int size );

static const NetRecvFuncTable FieldCommRecvTable[] = {
  { FIELD_COMM_FUNC_Post_SelfData   ,NULL },
  { FIELD_COMM_FUNC_Post_RequestData  ,NULL },
  { FIELD_COMM_FUNC_Post_SelfProfile  ,NULL },
  { FIELD_COMM_FUNC_Post_CommonFlg  ,NULL },
  { FIELD_COMM_FUNC_Post_UserData   ,FIELD_COMM_FUNC_Post_UserData_Buff  },
  { FIELD_COMM_FUNC_Post_ExitReq    ,NULL },
};

//--------------------------------------------------------------
//  システム初期化
//  この時点では通信は開始しません
//--------------------------------------------------------------
FIELD_COMM_FUNC* FIELD_COMM_FUNC_InitSystem( HEAPID heapID )
{
  FIELD_COMM_FUNC *commFunc;
  int i;

  commFunc = GFL_HEAP_AllocClearMemory( heapID , sizeof(FIELD_COMM_FUNC) );
  commFunc->heapID_ = heapID;
  commFunc->commMode_ = FIELD_COMM_MODE_NONE;
  commFunc->sendSyncType_ = FCST_MAX;
  for( i=0;i<FIELD_COMM_MEMBER_MAX;i++ )
  {
    commFunc->postSyncType_[i] = FCST_MAX;
  }
  commFunc->pktBuffStart_ = 0;
  commFunc->pktBuffEnd_ = 0;
  for( i=0;i<F_COMM_PACKET_BUFF_NUM;i++ )
  {
    commFunc->pktBuffData_[i].type_ = FCPBT_INVALID;
  }

  FIELD_COMM_FUNC_DataReset(commFunc);
  return commFunc;

#if 0
  FIELD_COMM_FUNC *commFunc;

  commFunc = GFL_HEAP_AllocClearMemory( heapID , sizeof(FIELD_COMM_FUNC) );
  commFunc->heapID_ = heapID;
  commFunc->isInitCommSystem_ = GFL_NET_IsInit();
  //通信ステートのバックアップをチェック
  if( FIELD_COMM_DATA_IsExistSystem() == TRUE )
    commFunc->commMode_ = (FIELD_COMM_MODE)FIELD_COMM_DATA_GetFieldCommMode();
  else
    commFunc->commMode_ = FIELD_COMM_MODE_NONE;
  //通信が初期化されていたならアイコンをリロードする
  if( commFunc->isInitCommSystem_ == TRUE )
    GFL_NET_ReloadIcon();
  return commFunc;
#endif
}

void FIELD_COMM_FUNC_DataReset( FIELD_COMM_FUNC *commFunc )
{
  if( GFL_NET_IsInit() )
  {
    commFunc->isInitCommSystem_ = TRUE;
  }
  else
  {
    commFunc->isInitCommSystem_ = FALSE;
  }
  //通信が初期化されていたならアイコンをリロードする
  if( commFunc->isInitCommSystem_ == TRUE )
    GFL_NET_ReloadIcon();
}

//--------------------------------------------------------------
//  システム開放
//--------------------------------------------------------------
void  FIELD_COMM_FUNC_TermSystem( FIELD_COMM_FUNC *commFunc )
{
  GFL_HEAP_FreeMemory( commFunc );
}


static void FIELD_COMM_FUNC_Connect(void* pWork,int hardID)
{
    if(GFL_NET_IsParentMachine()==FALSE){
        COMM_FIELD_SYS_PTR commField = pWork;
        FIELD_COMM_FUNC *commFunc = FIELD_COMM_SYS_GetCommFuncWork(commField);
        commFunc->commMode_ = FIELD_COMM_MODE_TRY_CONNECT;
        commFunc->seqNo_ = 0;
        ARI_TPrintf("Connect!(Child)\n");
    }
}


//--------------------------------------------------------------
//  通信開始
//--------------------------------------------------------------
void * FIELD_COMM_FUNC_InitCommSystem( int *seq, void *pwk )
{
  static const GFLNetInitializeStruct aGFLNetInit = {
    FieldCommRecvTable, //NetSamplePacketTbl,  // 受信関数テーブル
    NELEMS(FieldCommRecvTable), // 受信テーブル要素数
        FIELD_COMM_FUNC_Connect,    ///< ハードで接続した時に呼ばれる
        NULL,    ///< ネゴシエーション完了時にコール
        NULL, // ユーザー同士が交換するデータのポインタ取得関数
    NULL, // ユーザー同士が交換するデータのサイズ取得関数
    FIELD_COMM_FUNC_GetBeaconData,    // ビーコンデータ取得関数
    GameBeacon_GetBeaconSize,    // ビーコンデータサイズ取得関数
    FIELD_COMM_FUNC_CheckConnectService,  // ビーコンのサービスを比較して繋いで良いかどうか判断する
    FIELD_COMM_FUNC_ErrorCallBack,    // 通信不能なエラーが起こった場合呼ばれる
        NULL,  //FatalError
        FIELD_COMM_FUNC_DisconnectCallBack, // 通信切断時に呼ばれる関数(終了時
    NULL, // オート接続で親になった場合
#if GFL_NET_WIFI
    NULL,     ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
    NULL, ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
    NULL,  ///< wifiフレンドリスト削除コールバック
    NULL,   ///< DWC形式の友達リスト
    NULL,  ///< DWCのユーザデータ（自分のデータ）
    0,   ///< DWCへのHEAPサイズ
    TRUE,        ///< デバック用サーバにつなぐかどうか
#endif  //GFL_NET_WIFI
    0x222,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
    GFL_HEAPID_APP,  //元になるheapid
    HEAPID_NETWORK,  //通信用にcreateされるHEAPID
    HEAPID_WIFI,  //wifi用にcreateされるHEAPID
    HEAPID_NETWORK, //
    GFL_WICON_POSX,GFL_WICON_POSY,  // 通信アイコンXY位置
    4,//_MAXNUM,  //最大接続人数
    48,//_MAXSIZE,  //最大送信バイト数
    4,//_BCON_GET_NUM,  // 最大ビーコン収集数
    TRUE,   // CRC計算
    FALSE,    // MP通信＝親子型通信モードかどうか
    GFL_NET_TYPE_WIRELESS,    //通信タイプの指定
    TRUE,   // 親が再度初期化した場合、つながらないようにする場合TRUE
    WB_NET_FIELDMOVE_SERVICEID, //GameServiceID
#if GFL_NET_IRC
  IRC_TIMEOUT_STANDARD, // 赤外線タイムアウト時間
#endif
    0,//MP親最大サイズ 512まで
    0,//dummy
  };

  COMM_FIELD_SYS_PTR commField;
  FIELD_INVALID_PARENT_WORK *invalid_parent = pwk;
  
  commField = FIELD_COMM_MAIN_CommFieldSysAlloc(GFL_HEAP_LOWID(HEAPID_APP_CONTROL), invalid_parent->game_comm);
  
  GFL_NET_Init( &aGFLNetInit , FIELD_COMM_FUNC_FinishInitCallback , commField );
  {
    FIELD_COMM_FUNC *commFunc = FIELD_COMM_SYS_GetCommFuncWork(commField);
    commFunc->commMode_ = FIELD_COMM_MODE_NONE;
  }
  
  return commField;
}

//==================================================================
/**
 * 通信システム初期化待ち
 * @param   pWork		
 * @retval  BOOL		TRUE:初期化完了　　FALSE:初期化待ち
 */
//==================================================================
BOOL  FIELD_COMM_FUNC_InitCommSystemWait( int *seq, void *pwk, void *pWork )
{
  COMM_FIELD_SYS_PTR commField = pWork;
  FIELD_COMM_FUNC *commFunc = FIELD_COMM_SYS_GetCommFuncWork(commField);
  FIELD_INVALID_PARENT_WORK *invalid_parent = pwk;
  
  switch(*seq){
  case 0:
    if(commFunc->isInitCommSystem_ == TRUE){
      if(invalid_parent->my_invasion == TRUE){
        OS_TPrintf("親として起動\n");
        FIELD_COMM_FUNC_StartCommWait(commFunc);
//        GFL_NET_ChangeoverConnect(NULL);
        commFunc->commMode_ = FIELD_COMM_MODE_WAIT;
      }
      else{
        OS_TPrintf("子として起動\n");
        GFL_NET_InitClientAndConnectToParent( invalid_parent->parent_macAddress );
        commFunc->commMode_ = FIELD_COMM_MODE_CONNECTING;
      }
      (*seq)++;
    }
    break;
  case 1:
    if( GFL_NET_HANDLE_RequestNegotiation() == TRUE ){
      OS_TPrintf("ネゴシエーション送信\n");
      (*seq)++;
    }
    break;
  case 2:
    {
      GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
      if( GFL_NET_HANDLE_IsNegotiation( selfHandle ) == TRUE ){
        {
          GAME_COMM_SYS_PTR game_comm = FIELD_COMM_SYS_GetGameCommSys(commField);
          MYSTATUS *myst = GAMEDATA_GetMyStatus(GameCommSys_GetGameData(game_comm));
          MYSTATUS *dest_myst = GAMEDATA_GetMyStatusPlayer(GameCommSys_GetGameData(game_comm), GFL_NET_SystemGetCurrentID());
          MyStatus_Copy(myst, dest_myst);
        }
        
        {
          FIELD_COMM_FUNC_Send_RequestData( GFL_NET_SENDID_ALLUSER , FCRT_PROFILE , commFunc );
        }
        
        commFunc->commMode_ = FIELD_COMM_MODE_CONNECT;
        OS_TPrintf("ネゴシエーション完了\n");
        return TRUE;
      }
    }
    break;
  }
  return FALSE;
}

//--------------------------------------------------------------
//  通信システム終了
//--------------------------------------------------------------
BOOL  FIELD_COMM_FUNC_TermCommSystem( int *seq, void *pwk, void *pWork )
{
  COMM_FIELD_SYS_PTR commField = pWork;
  FIELD_COMM_FUNC *commFunc = FIELD_COMM_SYS_GetCommFuncWork(commField);

  switch(*seq){
  case 0:
    if(GFL_NET_IsParentMachine() == TRUE){
      if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), FC_CMD_EXIT_REQ, 0, NULL) == TRUE){
        (*seq)++;
      }
    }
    else{
      (*seq)++;
    }
    break;
  case 1:
    GFL_NET_TimingSyncStart( GFL_NET_HANDLE_GetCurrentHandle() , FCF_TIMING_EXIT );
    (*seq)++;
    break;
  case 2:
    if(GFL_NET_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(), FCF_TIMING_EXIT) == TRUE){
      OS_TPrintf("切断同期完了\n");
      (*seq)++;
    }
    else{
      OS_TPrintf("切断同期待ち\n");
    }
    break;
  case 3:
    if(GFL_NET_IsParentMachine() == TRUE){
      if(GFL_NET_GetConnectNum() <= 1){ //親は自分一人になってから終了する
        (*seq)++;
      }
      else{
        OS_TPrintf("親：子の終了待ち 残り=%d\n", GFL_NET_GetConnectNum() - 1);
      }
    }
    else{
      (*seq)++;
    }
    break;
  case 4:
    GFL_NET_Exit( FIELD_COMM_FUNC_FinishTermCallback );
    return TRUE;
  }
  
  return FALSE;
}

//==================================================================
/**
 * 通信システム終了待ち確認
 * @param   pWork		
 * @retval  BOOL		TRUE:終了。　FALSE:終了待ち
 */
//==================================================================
BOOL  FIELD_COMM_FUNC_TermCommSystemWait( int *seq, void *pwk, void *pWork )
{
  COMM_FIELD_SYS_PTR commField = pWork;

  FIELD_COMM_FUNC *commFunc = FIELD_COMM_SYS_GetCommFuncWork(commField);
  if(commFunc->isInitCommSystem_ == FALSE){
    FIELD_COMM_MAIN_CommFieldSysFree(commField);
    GFL_HEAP_FreeMemory(pwk);
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
//  通信システム更新(ビーコンの待ちうけ
//--------------------------------------------------------------
void  FIELD_COMM_FUNC_UpdateSystem( int *seq, void *pwk, void *pWork )
{
  COMM_FIELD_SYS_PTR commField = pWork;
  FIELD_COMM_FUNC *commFunc = FIELD_COMM_SYS_GetCommFuncWork(commField);

  //待ち受け側でもビーコンをチェックしてみる
  //if( commFunc->commMode_ == FIELD_COMM_MODE_SEARCH )
  //  待ちうけ・探索中で親機ではない時
  if( commFunc->commMode_ == FIELD_COMM_MODE_SEARCH ||
    commFunc->commMode_ == FIELD_COMM_MODE_WAIT )
  {
    //子機と親機で処理を分ける←見分ける方法が無いので断念・・・
  //  if( GFL_NET_IsParentMachine() == FALSE )
    {
      //子機の場合はビーコンのチェック
////      FIELD_COMM_FUNC_UpdateSearchParent( commFunc );
    }
  //  else
    {
      if( GFL_NET_GetConnectNum() > 1 )
      {
        //親機の場合は子機が来たら接続状態に
        ARI_TPrintf("Connect!(Parent)\n");
        commFunc->commMode_ = FIELD_COMM_MODE_CONNECT;
        //commFunc->commMode_ = FIELD_COMM_MODE_TRY_CONNECT;
      }
    }
  }
  if( commFunc->commMode_ == FIELD_COMM_MODE_TRY_CONNECT )
  {
    switch( commFunc->seqNo_ )
    {
    case 0:
      if( GFL_NET_HANDLE_RequestNegotiation() == TRUE )
      {
        commFunc->seqNo_++;
      }
      break;
    case 1:
      {
        GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
        if( GFL_NET_HANDLE_IsNegotiation( selfHandle ) == TRUE )
        {
          commFunc->seqNo_++;
        }
      }
      break;
    case 2:
      {
        GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
        if( GFL_NET_IsSendEnable( selfHandle ) == TRUE )
        {
          commFunc->commMode_ = FIELD_COMM_MODE_CONNECT;
        }
      }
    }
  }

  if( commFunc->commMode_ == FIELD_COMM_MODE_CONNECT )
  {
    //バッファの送信処理
    FIELD_COMM_FUNC_SendPacketBuff( commFunc );
  }

#if 0 //DEB_ARI
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_Y )
  {
    commFunc->commMode_ = FIELD_COMM_MODE_NONE;

    FIELD_COMM_FUNC_TermCommSystem();
    commFunc->isInitCommSystem_ = FALSE;
    NetErr_ErrorSet();
    //FIELD_COMM_FUNC_Send_CommonFlg( FCCF_TEST , 0 , GFL_NET_SENDID_ALLUSER );
  }
#endif
}

//--------------------------------------------------------------
//  子機で親機を探している状態
//--------------------------------------------------------------
static  void FIELD_COMM_FUNC_UpdateSearchParent( FIELD_COMM_FUNC *commFunc )
{
  u8 bcnIdx = 0;
  int targetIdx = -1;
  FIELD_COMM_BEACON *bcnData;
  const FIELD_COMM_BEACON *selfBcn = FIELD_COMM_FUNC_GetBeaconData_CommFunc(commFunc);
  while( GFL_NET_GetBeaconData(bcnIdx) != NULL )
  {
    bcnData = GFL_NET_GetBeaconData( bcnIdx );
    if( selfBcn->mode_ == 1 || bcnData->mode_ == 1 )
    {
      //接続条件を満たした。
      if( targetIdx == -1 )
      {
        targetIdx = bcnIdx;
      }
      else
      {
        //すでに他のビーコンが接続候補にあるので比較
        const FIELD_COMM_BEACON *compBcn = GFL_NET_GetBeaconData(targetIdx);
        const u8 result = FIELD_COMM_FUNC_CompareBeacon( bcnData , compBcn );
        if( result == 1 )
        {
          targetIdx = bcnIdx;
        }
      }
    }
    bcnIdx++;
  }
  if( targetIdx != -1 && commFunc->commMode_ != FIELD_COMM_MODE_TRY_CONNECT)
  {
    //ビーコンがあった
    u8 *macAdr = GFL_NET_GetBeaconMacAddress(targetIdx);
    if( macAdr != NULL )
    {
      GFL_NET_ConnectToParent( macAdr );
      commFunc->commMode_ = FIELD_COMM_MODE_CONNECTING;
    //  commFunc->seqNo_ = 0;
    //  ARI_TPrintf("Connect!(Child)\n");
    }
  }

}

//--------------------------------------------------------------
//  ビーコンの比較(人数が多い・共に待ち受け中を優先する
//  @return 0:エラー 1:第１引数のビーコン 2:第２引数のビーコン
//--------------------------------------------------------------
static  u8  FIELD_COMM_FUNC_CompareBeacon( const FIELD_COMM_BEACON *firstBcn , const FIELD_COMM_BEACON *secondBcn )
{
  //人数マックスチェック
  if( firstBcn->memberNum_ == FIELD_COMM_MEMBER_MAX &&
    secondBcn->memberNum_ == FIELD_COMM_MEMBER_MAX )
    return 0;

  if( secondBcn->memberNum_ == FIELD_COMM_MEMBER_MAX ||
    firstBcn->memberNum_ > secondBcn->memberNum_ )
    return 1;
  if( firstBcn->memberNum_ == FIELD_COMM_MEMBER_MAX ||
    firstBcn->memberNum_ < secondBcn->memberNum_ )
    return 2;

  if( firstBcn->mode_ > secondBcn->mode_ )
    return 1;
  if( firstBcn->mode_ < secondBcn->mode_ )
    return 2;

  return (GFUser_GetPublicRand(2)+1);
}

//--------------------------------------------------------------
//  通信の開始(侵入受付状態
//--------------------------------------------------------------
void  FIELD_COMM_FUNC_StartCommWait( FIELD_COMM_FUNC *commFunc )
{
  if( commFunc->commMode_ == FIELD_COMM_MODE_NONE )
  {
    GFL_NET_Changeover(NULL);
  }
  commFunc->commMode_ = FIELD_COMM_MODE_WAIT;
}

//--------------------------------------------------------------
//  通信の開始(侵入先探索状態
//--------------------------------------------------------------
void  FIELD_COMM_FUNC_StartCommSearch( FIELD_COMM_FUNC *commFunc )
{
  if( commFunc->commMode_ == FIELD_COMM_MODE_NONE )
  {
    //GFL_NET_Changeover(NULL);
        GFL_NET_StartBeaconScan();
  }
  commFunc->commMode_ = FIELD_COMM_MODE_SEARCH;
}

//--------------------------------------------------------------
//  探索通信の開始(親子交互通信・未使用
//--------------------------------------------------------------
void  FIELD_COMM_FUNC_StartCommChangeover( FIELD_COMM_FUNC *commFunc )
{
  //GFL_NET_ChangeoverConnect(NULL);
}

//--------------------------------------------------------------
//  会話開始時に初期化するもの
//--------------------------------------------------------------
void  FIELD_COMM_FUNC_InitCommData_StartTalk( FIELD_COMM_FUNC *commFunc )
{
  commFunc->selectAction_ = FCAL_SELECT;
  commFunc->isActionReturn_ = FALSE;
  commFunc->actionReturn_ = FALSE;
}


//======================================================================
//  各種チェック関数
//======================================================================
//--------------------------------------------------------------
// 通信ライブラリ　初期化・開放用
//--------------------------------------------------------------
const BOOL FIELD_COMM_FUNC_IsFinishInitCommSystem( FIELD_COMM_FUNC *commFunc )
{
  if(commFunc == NULL){
    return FALSE;
  }
  return commFunc->isInitCommSystem_;
}
const BOOL FIELD_COMM_FUNC_IsFinishTermCommSystem( FIELD_COMM_FUNC *commFunc )
{
  if(commFunc == NULL){
    return TRUE;
  }
  return !commFunc->isInitCommSystem_;
}

//--------------------------------------------------------------
//  通信状態？を取得
//--------------------------------------------------------------
const FIELD_COMM_MODE FIELD_COMM_FUNC_GetCommMode( FIELD_COMM_FUNC *commFunc )
{
  return commFunc->commMode_;
}
//--------------------------------------------------------------
//  接続人数を取得
//--------------------------------------------------------------
const int FIELD_COMM_FUNC_GetMemberNum( void )
{
  /*
  u8 i;
  u8 num = 1;//自分の分
  for( i=0;i<FIELD_COMM_MEMBER_MAX;i++ )
  {
    if( FIELD_COMM_DATA_GetCharaData_State(i) != FCCS_NONE )
      num++;
  }
  return num;
  */
  return GFL_NET_GetConnectNum();
}

//--------------------------------------------------------------
// 自分のインデックス(netID)を取得
//--------------------------------------------------------------
const int FIELD_COMM_FUNC_GetSelfIndex( FIELD_COMM_FUNC *commFunc )
{
  GFL_NETHANDLE *handle =GFL_NET_HANDLE_GetCurrentHandle();
  return GFL_NET_GetNetID( handle );
}

//--------------------------------------------------------------
// 会話予約のデータ取得
//--------------------------------------------------------------
void FIELD_COMM_FUNC_GetTalkParterData_ID( u8 *ID , FIELD_COMM_FUNC *commFunc )
{
  *ID = commFunc->talkID_;
}
void FIELD_COMM_FUNC_GetTalkParterData_Pos( u16 *posX , u16 *posZ , FIELD_COMM_FUNC *commFunc )
{
  *posX = commFunc->talkPosX_;
  *posZ = commFunc->talkPosZ_;
}

//--------------------------------------------------------------
// 親の選んだ行動の取得  (FCAL_SELECT は未着
//--------------------------------------------------------------
const F_COMM_ACTION_LIST FIELD_COMM_FUNC_GetSelectAction( FIELD_COMM_FUNC *commFunc )
{
  return (F_COMM_ACTION_LIST)commFunc->selectAction_;
}
//--------------------------------------------------------------
// 子が行動をOKしたかの返事と着信確認
//--------------------------------------------------------------
const BOOL FIELD_COMM_FUNC_IsActionReturn( FIELD_COMM_FUNC *commFunc )
{
  return commFunc->isActionReturn_;
}
const BOOL FIELD_COMM_FUNC_GetActionReturn( FIELD_COMM_FUNC *commFunc )
{
  return commFunc->actionReturn_;
}

//--------------------------------------------------------------
// ユーザーデータ系
//--------------------------------------------------------------
void FIELD_COMM_FUNC_ResetGetUserData( FIELD_COMM_FUNC *commFunc )
{
  commFunc->isGetUserData_ = FALSE;
}
const BOOL FIELD_COMM_FUNC_IsGetUserData( FIELD_COMM_FUNC *commFunc )
{
  return commFunc->isGetUserData_;
}

//======================================================================
//送受信関数
//======================================================================
//--------------------------------------------------------------
// 自分のデータ送信
//--------------------------------------------------------------
const BOOL  FIELD_COMM_FUNC_Send_SelfData( FIELD_COMM_FUNC *commFunc, FIELD_COMM_DATA *commData, int palace_area, int mission_no )
{
  PLAYER_WORK *plWork = NULL;
  const VecFx32 *pos;
  u16 dir;
  ZONEID zoneID;
  u8  talkState;

  //データ収集
  plWork = FIELD_COMM_DATA_GetSelfData_PlayerWork(commData);
  zoneID = PLAYERWORK_getZoneID( plWork );
  pos = PLAYERWORK_getPosition( plWork );
  dir = PLAYERWORK_getDirection( plWork );
  talkState = FIELD_COMM_DATA_GetTalkState( commData, FCD_SELF_INDEX );
  //パケットにセット
  commFunc->plPkt_.zoneID_ = zoneID;
  commFunc->plPkt_.posX_ = F32_CONST( pos->x );
  commFunc->plPkt_.posY_ = (int)F32_CONST( pos->y );
  commFunc->plPkt_.posZ_ = F32_CONST( pos->z );
  commFunc->plPkt_.dir_ = dir >> 14;// / 0x4000;   //送信データを小さくする為、indexに変換
  commFunc->plPkt_.talkState_ = talkState;
  commFunc->plPkt_.palace_area = palace_area;
  commFunc->plPkt_.mission_no = mission_no;
  
//  ARI_TPrintf("SEND[ ][%d][%d][%d][%x]\n",commFunc->plPkt_.posX_,commFunc->plPkt_.posY_,commFunc->plPkt_.posZ_,dir);
  {
    GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
    const BOOL ret = GFL_NET_SendDataEx( selfHandle , GFL_NET_SENDID_ALLUSER ,
        FC_CMD_SELFDATA , sizeof(FIELD_COMM_PLAYER_PACKET) ,
        (void*)&commFunc->plPkt_ , FALSE , TRUE , TRUE );
    if( ret == FALSE ){
      //こいつは再送処理なしで良いかな・・・
      //ARI_TPrintf("FieldComm SendSelfData is failue!\n");
    }
    return ret;
  }
}

//--------------------------------------------------------------
// 自分のデータ受信(要は他のプレイヤーの受信
//--------------------------------------------------------------
void  FIELD_COMM_FUNC_Post_SelfData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  COMM_FIELD_SYS_PTR commField = pWork;
  FIELD_COMM_FUNC *commFunc = FIELD_COMM_SYS_GetCommFuncWork(commField);
  FIELD_COMM_DATA *commData = FIELD_COMM_SYS_GetCommDataWork(commField);
  GAME_COMM_SYS_PTR game_comm = FIELD_COMM_SYS_GetGameCommSys(commField);
  
  //自分のデータも一応セットしておく(同期用に使う
  const FIELD_COMM_PLAYER_PACKET *pkt = (FIELD_COMM_PLAYER_PACKET*)pData;
  VecFx32 pos;
  u16 dir;
  u8 mission_no;

  //パケットデータを戻す
  pos.x = FX32_CONST( pkt->posX_ );
  pos.y = FX32_CONST( pkt->posY_ );
  pos.z = FX32_CONST( pkt->posZ_ );
  dir = pkt->dir_ << 14;// * 0x4000;//通信キャラはインデックスの4方向なので

//  ARI_TPrintf("POST[%d][%d][%d][%d][%x]\n",netID,pkt->posX_,pkt->posY_,pkt->posZ_,dir);

  //set
  {
    PLAYER_WORK *plWork;
    
    plWork = FIELD_COMM_DATA_GetCharaData_PlayerWork(commData, netID);
    PLAYERWORK_setPosition( plWork , &pos );
    PLAYERWORK_setDirection( plWork , dir );
    PLAYERWORK_setZoneID( plWork, pkt->zoneID_ );
    PLAYERWORK_setPalaceArea( plWork, pkt->palace_area );
    FIELD_COMM_DATA_SetCharaData_IsValid( commData, netID , TRUE );
    FIELD_COMM_DATA_SetTalkState( commData, netID , pkt->talkState_ );
    
    if(netID == 0){
      mission_no = pkt->mission_no;
    }
    else{
      mission_no = GameCommStatus_GetPlayerStatus_MissionNo(game_comm, GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle()));
    }
    GameCommStatus_SetPlayerStatus(FIELD_COMM_SYS_GetGameCommSys(commField), 
      netID, pkt->zoneID_, pkt->palace_area, mission_no);
  }
  
  {//ゾーン違いによるアクター表示・非表示設定
    PLAYER_WORK *my_player = GAMEDATA_GetMyPlayerWork(GameCommSys_GetGameData(game_comm));
    ZONEID my_zone_id = PLAYERWORK_getZoneID( my_player );
    BOOL *vanish_flag = FIELD_COMM_SYS_GetCommActorVanishFlag(commField, netID);
    
    if(pkt->zoneID_ != my_zone_id){
      *vanish_flag = TRUE;  //違うゾーンにいるので非表示
    }
    else if(pkt->zoneID_ != ZONE_ID_PALACETEST 
        && pkt->palace_area != FIELD_COMM_SYS_GetInvalidNetID(commField)){
      //通常フィールドの同じゾーンに居ても、侵入先ROMが違うなら非表示
      *vanish_flag = TRUE;
    }
    else{
      *vanish_flag = FALSE;
    }
  }

//  ARI_TPrintf("FieldComm PostSelfData[%d]\n",netID);
}

//--------------------------------------------------------------
// 他のプレイヤーにデータを要求する
// Postの関数でそのまま"要求されたキャラだけに"データを送る
// @param charaIdx キャラ番号(=netID)
//--------------------------------------------------------------
const BOOL  FIELD_COMM_FUNC_Send_RequestData( const u8 charaIdx , const F_COMM_REQUEST_TYPE reqType , FIELD_COMM_FUNC *commFunc )
{
  //再送機能を実装したため、一段噛ませて引数の追加に対応
  return FIELD_COMM_FUNC_Send_RequestDataFunc( charaIdx , reqType , FALSE , commFunc );
}
static const BOOL FIELD_COMM_FUNC_Send_RequestDataFunc( const u8 charaIdx , const F_COMM_REQUEST_TYPE reqType , const BOOL isReSend , FIELD_COMM_FUNC *commFunc )
{
  if( isReSend == FALSE )
  {
    BOOL ret = FIELD_COMM_FUNC_SendPacketBuff( commFunc );
    if( ret == FALSE )
    {
      //送信失敗したらバッファにためて終わり
      FIELD_COMM_FUNC_SetPacketBuff( FCPBT_REQ_DATA , reqType , 0 , 0, charaIdx , commFunc );
      ARI_TPrintf("FieldComm SendRequestData is failue![Buffer]\n");
      return FALSE;
    }
  }
  {
    GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
    u8  type = reqType;
    const BOOL ret = GFL_NET_SendDataEx( selfHandle , charaIdx ,
        FC_CMD_REQUEST_DATA , 1 ,
        (void*)&type , FALSE , FALSE , FALSE );
      ARI_TPrintf("FieldComm Send RequestData[%d:%d].\n",charaIdx,type);
    if( ret == FALSE ){
      ARI_TPrintf("FieldComm SendRequestData is failue!\n");
      if( isReSend == FALSE )
      {
        //バッファにセット
        FIELD_COMM_FUNC_SetPacketBuff( FCPBT_REQ_DATA , reqType , 0 , 0, charaIdx , commFunc );
      }
    }
    return ret;
  }
}
void  FIELD_COMM_FUNC_Post_RequestData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  COMM_FIELD_SYS_PTR commField = pWork;
  const u8 type = *(u8*)pData;
  ARI_TPrintf("FieldComm PostReqData[%d:%d]\n",netID,type);
  switch( type )
  {
  case FCRT_PROFILE:
    FIELD_COMM_FUNC_Send_SelfProfile( netID , commField );
    break;
  default:
    OS_TPrintf("Invalid Type[%d]!\n!",type);
    GF_ASSERT( NULL );
    break;
  }
}

//--------------------------------------------------------------
// 自分のプロフィールの送信(最初に送る物 IDとかキャラの基本情報
// 要求された相手だけに送る
//--------------------------------------------------------------
typedef struct
{
  STRCODE name[PERSON_NAME_SIZE + EOM_SIZE];
  u16 ID_;
  u8  sex_:1;
  u8  regionCode_:7;
  u32 myst_id;
}FIELD_COMM_CHARA_PROFILE;
const BOOL  FIELD_COMM_FUNC_Send_SelfProfile( const int sendNetID , COMM_FIELD_SYS_PTR commField )
{
  GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
  FIELD_COMM_CHARA_PROFILE profile;
  STRBUF *namebuf;
  GAME_COMM_SYS_PTR game_comm = FIELD_COMM_SYS_GetGameCommSys(commField);
  MYSTATUS *myst = GAMEDATA_GetMyStatus(GameCommSys_GetGameData(game_comm));
  
  //FIXME:IDとかの正しい持って来る方法がわからないので仮処理
  namebuf = MyStatus_CreateNameString(myst, HEAPID_APP_CONTROL);
  GFL_STR_GetStringCode(namebuf, profile.name, PERSON_NAME_SIZE + EOM_SIZE);
  GFL_STR_DeleteBuffer(namebuf);
  profile.ID_ = 1000+GFL_NET_GetNetID( selfHandle );
  profile.sex_ = MyStatus_GetMySex(myst);
  profile.regionCode_ = 0;
  profile.myst_id = MyStatus_GetID(myst);
  {
    const BOOL ret = GFL_NET_SendDataEx( selfHandle , sendNetID ,
        FC_CMD_SELF_PROFILE , sizeof( FIELD_COMM_CHARA_PROFILE ) ,
        (void*)&profile , FALSE , FALSE , FALSE );
    ARI_TPrintf("FieldComm Send SelfProfile[%d:%d].\n",sendNetID,profile.ID_);
    if( ret == FALSE ){
      ARI_TPrintf("FieldComm Send SelfProfile is failue!\n");
    }
    return ret;
  }
}
void  FIELD_COMM_FUNC_Post_SelfProfile( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  COMM_FIELD_SYS_PTR commField = pWork;
  FIELD_COMM_DATA *commData = FIELD_COMM_SYS_GetCommDataWork(commField);
  GAME_COMM_SYS_PTR game_comm = FIELD_COMM_SYS_GetGameCommSys(commField);
  const FIELD_COMM_CHARA_PROFILE *prof = (FIELD_COMM_CHARA_PROFILE*)pData;
  PLAYER_WORK *plWork = FIELD_COMM_DATA_GetCharaData_PlayerWork( commData, netID );
  MYSTATUS *myst;
  
  ARI_TPrintf("FieldComm Post SelfProfile[%d:%d]\n",netID,prof->ID_);

  plWork->mystatus.id = prof->ID_;
  plWork->mystatus.sex = prof->sex_;
  plWork->mystatus.region_code = prof->regionCode_;
  MyStatus_SetMyName(&plWork->mystatus, prof->name);

  //gamedataの方にもセット
  myst = GAMEDATA_GetMyStatusPlayer(GameCommSys_GetGameData(game_comm), netID);
  MyStatus_SetMyName(myst, prof->name);
  MyStatus_SetID(myst, prof->myst_id);
  MyStatus_SetMySex(myst, prof->sex_);
  FIELD_COMM_SYS_SetRecvProfile(commField, netID);

  FIELD_COMM_DATA_SetCharaData_State( commData, netID , FCCS_EXIST_DATA );
}

//--------------------------------------------------------------
// 汎用フラグ
//--------------------------------------------------------------
typedef struct
{
  u8  flg_;
  u8  padding[3];
  union{
    struct{
      u16 work[2];
    };
    struct{
      u16 value_;
      u16 padding2;
    };
    struct{
      u16 talk_x;
      u16 talk_y;
    };
  };
}FIELD_COMM_COMMONFLG_PACKET;
const BOOL  FIELD_COMM_FUNC_Send_CommonFlg( FIELD_COMM_FUNC *commFunc, const F_COMM_COMMON_FLG flg , const u16 val , const u16 val2, const u8 sendID)
{
  //再送機能を実装したため、一段噛ませて引数の追加に対応
  return FIELD_COMM_FUNC_Send_CommonFlgFunc( flg , val , val2, sendID , FALSE , commFunc );
}
static const BOOL FIELD_COMM_FUNC_Send_CommonFlgFunc( const F_COMM_COMMON_FLG flg , const u16 val , const u16 val2, const u8 sendID , BOOL isReSend , FIELD_COMM_FUNC *commFunc )
{
  //まずバッファの処理を行う
  if( isReSend == FALSE )
  {
    BOOL ret = FIELD_COMM_FUNC_SendPacketBuff( commFunc );
    if( ret == FALSE )
    {
      //送信失敗したらバッファにためて終わり
      ARI_TPrintf("FieldComm Send commonFlg is failue[Buffer]!\n");
      FIELD_COMM_FUNC_SetPacketBuff( FCPBT_FLG , flg , val , val2, sendID , commFunc );
      return FALSE;
    }
  }
  {
    GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
    FIELD_COMM_COMMONFLG_PACKET pkt;
    pkt.flg_ = flg;
    pkt.work[0] = val;
    pkt.work[1] = val2;
    {
      const BOOL ret = GFL_NET_SendDataEx( selfHandle , sendID ,
          FC_CMD_COMMON_FLG , sizeof( FIELD_COMM_COMMONFLG_PACKET ) ,
          (void*)&pkt , FALSE , FALSE , FALSE );
      ARI_TPrintf("FieldComm Send commonFlg[%d:%d:%d].\n",sendID,flg,val);
      if( ret == FALSE ){
        ARI_TPrintf("FieldComm Send commonFlg is failue!\n");
        if( isReSend == FALSE )
        {
          FIELD_COMM_FUNC_SetPacketBuff( FCPBT_FLG , flg , val , val2, sendID , commFunc );
        }
      }
      return ret;
    }
  }
}
void  FIELD_COMM_FUNC_Post_CommonFlg( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  COMM_FIELD_SYS_PTR commField = pWork;
  FIELD_COMM_FUNC *commFunc = FIELD_COMM_SYS_GetCommFuncWork(commField);
  FIELD_COMM_DATA *commData = FIELD_COMM_SYS_GetCommDataWork(commField);
  const FIELD_COMM_COMMONFLG_PACKET *pkt = (FIELD_COMM_COMMONFLG_PACKET*)pData;
  const F_COMM_TALK_STATE talkState = FIELD_COMM_DATA_GetTalkState( commData, FCD_SELF_INDEX );
  ARI_TPrintf("FieldComm PostCommonFlg[%d:%d:%d]\n",netID,pkt->flg_,pkt->value_);
  switch( pkt->flg_ )
  {
  case FCCF_TALK_REQUEST:   //会話要求
    if( talkState == FCTS_NONE )
    {
      FIELD_COMM_DATA_SetTalkState( commData, FCD_SELF_INDEX , FCTS_RESERVE_TALK );
      commFunc->talkPosX_ = pkt->talk_x;
      commFunc->talkPosZ_ = pkt->talk_y;
      commFunc->talkID_ = netID;
    }
    else
    {
      FIELD_COMM_FUNC_Send_CommonFlg( commFunc, FCCF_TALK_UNPOSSIBLE , 0xf , 0, netID );
    }
    break;

  case FCCF_TALK_UNPOSSIBLE:  //要求に対して無理
    GF_ASSERT( talkState == FCTS_WAIT_TALK );
    FIELD_COMM_DATA_SetTalkState( commData, FCD_SELF_INDEX , FCTS_UNPOSSIBLE );
    break;

  case FCCF_TALK_ACCEPT:    //要求に対して許可
    GF_ASSERT( talkState == FCTS_WAIT_TALK );
    FIELD_COMM_DATA_SetTalkState( commData, FCD_SELF_INDEX , FCTS_ACCEPT );
    commFunc->talkID_ = netID;
    break;

  case FCCF_ACTION_SELECT:  //親が選んだ行動
    GF_ASSERT( pkt->value_ < FCAL_MAX );
    commFunc->selectAction_ = pkt->value_;
    break;

  case FCCF_ACTION_RETURN:  //親の選んだ行動に対する子の返事
    commFunc->isActionReturn_ = TRUE;
    commFunc->actionReturn_ = pkt->value_;
    break;

  case FCCF_SYNC_TYPE:
    commFunc->postSyncType_[netID] = pkt->value_;
    break;

  case FCCF_TEST:
    break;

  default:
    OS_TPrintf("Invalid flg[%d]!\n!",pkt->flg_);
    GF_ASSERT( NULL );
    break;
  }
}

//--------------------------------------------------------------
// 行動用ユーザーデータ
//--------------------------------------------------------------
const BOOL  FIELD_COMM_FUNC_Send_UserData( FIELD_COMM_DATA *commData, F_COMM_USERDATA_TYPE type , const u8 sendID )
{
  GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
  void* selfUserData = FIELD_COMM_DATA_GetSelfUserData( commData, type );
  const u32 dataSize = FIELD_COMM_DATA_GetUserDataSize( type );

  {
    const BOOL ret = GFL_NET_SendDataEx( selfHandle , sendID ,
        FC_CMD_USERDATA , dataSize ,
        selfUserData , FALSE , FALSE , TRUE );
    ARI_TPrintf("FieldComm Send UserData[%d:%d:%d].\n",sendID,type,dataSize);
    if( ret == FALSE ){
      ARI_TPrintf("FieldComm Send UserData is failue!\n");
    }
    return ret;
  }
}
void  FIELD_COMM_FUNC_Post_UserData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  COMM_FIELD_SYS_PTR commField = pWork;
  FIELD_COMM_FUNC *commFunc = FIELD_COMM_SYS_GetCommFuncWork(commField);
  commFunc->isGetUserData_ = TRUE;
  ARI_TPrintf("FieldComm PostUserData[%d:%d]\n",netID,size);

}
u8*   FIELD_COMM_FUNC_Post_UserData_Buff( int netID, void* pWork , int size )
{
  COMM_FIELD_SYS_PTR commField = pWork;
  FIELD_COMM_DATA *commData = FIELD_COMM_SYS_GetCommDataWork(commField);
  void* userData = FIELD_COMM_DATA_GetPartnerUserData( commData, FCUT_MAX );
  return (u8*)userData;
}

//--------------------------------------------------------------
/**
 * 切断リクエスト：受信
 *
 * @param   netID		
 * @param   size		
 * @param   pData		
 * @param   pWork		
 * @param   pNetHandle		
 */
//--------------------------------------------------------------
static void  FIELD_COMM_FUNC_Post_ExitReq( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  COMM_FIELD_SYS_PTR commField = pWork;
  FIELD_COMM_FUNC *commFunc = FIELD_COMM_SYS_GetCommFuncWork(commField);
  GAME_COMM_SYS_PTR game_comm = FIELD_COMM_SYS_GetGameCommSys(commField);
  
  OS_TPrintf("切断リクエスト受信\n");

  FIELD_COMM_SYS_SetExitReq(commField); //終了フラグをセット
#if 0
  if(GFL_NET_IsParentMachine() == FALSE){
    GameCommSys_ExitReq(game_comm);
  }
#endif
}

//======================================================================
//同期用コマンド
//======================================================================
void FIELD_COMM_FUNC_Send_SyncCommand( const F_COMM_SYNC_TYPE type , FIELD_COMM_FUNC *commFunc )
{
  commFunc->sendSyncType_ = type;
  FIELD_COMM_FUNC_Send_CommonFlg( commFunc, FCCF_SYNC_TYPE , type , 0, GFL_NET_SENDID_ALLUSER );
//  GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
//  GFL_NET_TimingSyncStart( selfHandle , type );
}
const BOOL FIELD_COMM_FUNC_CheckSyncFinish( const F_COMM_SYNC_TYPE type , const u8 checkBit , FIELD_COMM_FUNC *commFunc )
{
  u8 i;
  ARI_TPrintf("SYNC CHECK [%d:%x]",type ,checkBit);
  for( i=0;i<FIELD_COMM_MEMBER_MAX;i++ )
  {
    if( (1<<i)&checkBit )
    {
      ARI_TPrintf("[%d]",i);
      if( commFunc->sendSyncType_ != commFunc->postSyncType_[i] )
      {
        ARI_TPrintf("_FALSE_E\n");
        return FALSE;
      }
    }
  }

  commFunc->sendSyncType_ = FCST_MAX;
  for( i=0;i<FIELD_COMM_MEMBER_MAX;i++ )
  {
    if( (1<<i)&checkBit )
    {
      commFunc->postSyncType_[i] = FCST_MAX;
    }
  }
  ARI_TPrintf("_TRUE_E\n");
  return TRUE;
//  GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
//  return GFL_NET_IsTimingSync( selfHandle , type );
}

//--------------------------------------------------------------
// 自分と会話相手のIDのbitを取得
//--------------------------------------------------------------
const u8 FIELD_COMM_FUNC_GetBit_TalkMember( FIELD_COMM_FUNC *commFunc )
{
  GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
  const u8 selfID = GFL_NET_GetNetID( selfHandle );
  return (1<<selfID)|(1<<commFunc->talkID_);
}

//======================================================================
//通信バッファ用
//======================================================================
//--------------------------------------------------------------
// 送信バッファセット
//--------------------------------------------------------------
void  FIELD_COMM_FUNC_SetPacketBuff( const F_COMM_PACKET_BUFF_TYPE type , const u8 id , const u16 value , const u16 val2, const u8 sendID , FIELD_COMM_FUNC *commFunc )
{
  //元に戻すように取っておく
  u8 idx = commFunc->pktBuffEnd_;
  commFunc->pktBuffEnd_++;
  if( commFunc->pktBuffEnd_ >= F_COMM_PACKET_BUFF_NUM )
  {
    commFunc->pktBuffEnd_ -= F_COMM_PACKET_BUFF_NUM;
  }
  //バッファオーバーチェック
  if( commFunc->pktBuffStart_ == commFunc->pktBuffEnd_ )
  {
    GF_ASSERT_MSG(NULL,"Field comm packet buffer is over!!");
    //TODO:タイムアウト通信エラー扱いにしてしまう？
    commFunc->pktBuffEnd_ = idx;
    return;
  }

  commFunc->pktBuffData_[commFunc->pktBuffEnd_].type_ = type;
  commFunc->pktBuffData_[commFunc->pktBuffEnd_].id_ = id;
  commFunc->pktBuffData_[commFunc->pktBuffEnd_].value_ = value;
  commFunc->pktBuffData_[commFunc->pktBuffEnd_].value2_ = val2;
  commFunc->pktBuffData_[commFunc->pktBuffEnd_].sendID_ = sendID;
  ARI_TPrintf("FieldComm PktBuff Set[%d-%d]\n",commFunc->pktBuffStart_, commFunc->pktBuffEnd_);
}

//--------------------------------------------------------------
// 送信バッファ送信
//--------------------------------------------------------------
const BOOL  FIELD_COMM_FUNC_SendPacketBuff( FIELD_COMM_FUNC *commFunc )
{
  if( commFunc->pktBuffStart_ == commFunc->pktBuffEnd_ )
  {
    //バッファ無し
    return TRUE;
  }

  while( commFunc->pktBuffStart_ != commFunc->pktBuffEnd_ )
  {
    const FIELD_COMM_PACKET_BUFF_DATA *pktBuff = &commFunc->pktBuffData_[commFunc->pktBuffStart_];
    BOOL ret;
    switch( pktBuff->type_ )
    {
    case FCPBT_FLG:
      ret = FIELD_COMM_FUNC_Send_CommonFlgFunc( pktBuff->id_ , pktBuff->value_,pktBuff->value2_ , pktBuff->sendID_ , TRUE , commFunc );
      break;
    case FCPBT_REQ_DATA:
      ret = FIELD_COMM_FUNC_Send_RequestDataFunc( pktBuff->sendID_ , pktBuff->id_ , TRUE , commFunc );
      break;
    }
    if( ret == FALSE )
    {
      //送信失敗
      return FALSE;
    }
    else
    {
      //送信成功
      ARI_TPrintf("FieldComm PktBuff Send[%d-%d]\n",commFunc->pktBuffStart_, commFunc->pktBuffEnd_);
      commFunc->pktBuffStart_++;
      if( commFunc->pktBuffStart_ >= F_COMM_PACKET_BUFF_NUM )
      {
        commFunc->pktBuffStart_ -= F_COMM_PACKET_BUFF_NUM;
      }

    }
  }
  return TRUE;
}

//======================================================================
//各種コールバック
//======================================================================
//--------------------------------------------------------------
// 通信ライブラリ　初期化・開放用
//--------------------------------------------------------------
void  FIELD_COMM_FUNC_FinishInitCallback( void* pWork )
{
  COMM_FIELD_SYS_PTR commField = pWork;
  FIELD_COMM_FUNC *commFunc = FIELD_COMM_SYS_GetCommFuncWork(commField);
  
  commFunc->isInitCommSystem_ = TRUE;
}
void  FIELD_COMM_FUNC_FinishTermCallback( void* pWork )
{
  COMM_FIELD_SYS_PTR commField = pWork;
  FIELD_COMM_FUNC *commFunc = FIELD_COMM_SYS_GetCommFuncWork(commField);
  
  commFunc->isInitCommSystem_ = FALSE;
  OS_TPrintf("切断完了コールバック\n");
}

//--------------------------------------------------------------
// ビーコンデータ取得関数
//--------------------------------------------------------------
void* FIELD_COMM_FUNC_GetBeaconData(void* pWork)
{
  COMM_FIELD_SYS_PTR commField = pWork;
  FIELD_COMM_FUNC *commFunc = FIELD_COMM_SYS_GetCommFuncWork(commField);

  return FIELD_COMM_FUNC_GetBeaconData_CommFunc(commFunc);
}

static void* FIELD_COMM_FUNC_GetBeaconData_CommFunc(FIELD_COMM_FUNC *commFunc)
{
#if 0
  static FIELD_COMM_BEACON beacon;

  if( commFunc->commMode_ == FIELD_COMM_MODE_WAIT )
    beacon.mode_ = 0;
  else
    beacon.mode_ = 1;
  beacon.memberNum_ = GFL_NET_GetConnectNum(commFunc);

  return (void*)&beacon;
#else
  GBS_BEACON *beacon = &commFunc->send_beacon;
  
  beacon->gsid = WB_NET_FIELDMOVE_SERVICEID;
  beacon->member_num = GFL_NET_GetConnectNum();
  beacon->member_max = FIELD_COMM_MEMBER_MAX;
  beacon->error = GFL_NET_SystemGetErrorCode();
  return beacon;
#endif
}

//--------------------------------------------------------------
// ビーコンのサービスを比較して繋いで良いかどうか判断する
//--------------------------------------------------------------
BOOL  FIELD_COMM_FUNC_CheckConnectService(GameServiceID GameServiceID1 , GameServiceID GameServiceID2 )
{
  return (GameServiceID1==GameServiceID2);
}

//--------------------------------------------------------------
// 通信不能なエラーが起こった場合呼ばれる 切断するしかない
//--------------------------------------------------------------
void  FIELD_COMM_FUNC_ErrorCallBack(GFL_NETHANDLE* pNet,int errNo, void* pWork)
{
  COMM_FIELD_SYS_PTR commField = pWork;
  FIELD_COMM_FUNC *commFunc = FIELD_COMM_SYS_GetCommFuncWork(commField);

  NetErr_ErrorSet();
////  FIELD_COMM_FUNC_TermCommSystem();
  commFunc->isInitCommSystem_ = FALSE;
}

//--------------------------------------------------------------
// 通信切断時に呼ばれる関数(終了時
//--------------------------------------------------------------
void  FIELD_COMM_FUNC_DisconnectCallBack(void* pWork)
{
  OS_TPrintf("切断コールバック呼ばれた\n");
}

