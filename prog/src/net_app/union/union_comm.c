//==============================================================================
/**
 * @file    union_comm.c
 * @brief   ユニオンルーム：通信制御
 * @author  matsuda
 * @date    2009.07.02(木)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/game_comm.h"
#include "union_types.h"
#include "union_local.h"
#include "savedata/mystatus.h"
#include "net_app/union/union_main.h"
#include "system/net_err.h"
#include "net/network_define.h"
#include "union_comm_command.h"
#include "union_msg.h"


//==============================================================================
//  定数定義
//==============================================================================
///HEAPID_UNIONで確保するヒープサイズ
#define HEAP_SIZE_UNION               (0x6000)

///通信ステータス
enum{
  UNION_COMM_STATUS_NULL,         ///<何も動作していない
  UNION_COMM_STATUS_INIT_START,   ///<初期化完了
  UNION_COMM_STATUS_INIT,         ///<初期化完了
  UNION_COMM_STATUS_UPDATE,       ///<メイン更新実行中
  UNION_COMM_STATUS_EXIT_START,   ///<終了処理開始
  UNION_COMM_STATUS_EXIT,         ///<終了処理完了
};


//--------------------------------------------------------------
//  オーバーレイID
//--------------------------------------------------------------
FS_EXTERN_OVERLAY(union_room);


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void UnionComm_InitCallback(void *pWork);
static void	UnionComm_ExitCallback(void* pWork);
static void UnionComm_BeaconSearch(UNION_SYSTEM_PTR unisys);
static BOOL UnionBeacon_SetReceiveData(UNION_SYSTEM_PTR unisys, const UNION_BEACON *beacon, const u8 *beacon_mac_address);
static void* UnionComm_GetBeaconData(void* pWork);
static void UnionComm_SetBeaconParam(UNION_SYSTEM_PTR unisys, UNION_BEACON *beacon);
static int UnionComm_GetBeaconSize(void *pWork);
static BOOL UnionComm_CheckConnectService(GameServiceID GameServiceID1 , GameServiceID GameServiceID2 );
static void UnionComm_ErrorCallBack(GFL_NETHANDLE* pNet,int errNo, void* pWork);
static void UnionComm_DisconnectCallBack(void* pWork);


//==============================================================================
//  データ
//==============================================================================
static const GFLNetInitializeStruct aGFLNetInit = {
	Union_CommPacketTbl,  // 受信関数テーブル
	UNION_CMD_NUM,  			// 受信テーブル要素数
	NULL,		///< ハードで接続した時に呼ばれる
	NULL,		///< ネゴシエーション完了時にコール
	NULL,		// ユーザー同士が交換するデータのポインタ取得関数
	NULL,		// ユーザー同士が交換するデータのサイズ取得関数
	UnionComm_GetBeaconData,		// ビーコンデータ取得関数  
	UnionComm_GetBeaconSize,		// ビーコンデータサイズ取得関数 
	UnionComm_CheckConnectService,		// ビーコンのサービスを比較して繋いで良いかどうか判断する
	UnionComm_ErrorCallBack,		// 通信不能なエラーが起こった場合呼ばれる
	NULL,		//FatalError
	UnionComm_DisconnectCallBack,		// 通信切断時に呼ばれる関数(終了時
	NULL,		// オート接続で親になった場合
#if GFL_NET_WIFI
	NULL,		///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
	NULL,		///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
	NULL,		///< wifiフレンドリスト削除コールバック
	NULL,		///< DWC形式の友達リスト	
	NULL,		///< DWCのユーザデータ（自分のデータ）
	0,			///< DWCへのHEAPサイズ
	TRUE,		///< デバック用サーバにつなぐかどうか
#endif  //GFL_NET_WIFI
	0x122,		//ggid  DP=0x333,RANGER=0x178,WII=0x346
	GFL_HEAPID_APP,		//元になるheapid
	HEAPID_NETWORK + HEAPDIR_MASK,		//通信用にcreateされるHEAPID
	HEAPID_WIFI + HEAPDIR_MASK,		//wifi用にcreateされるHEAPID
	HEAPID_NETWORK + HEAPDIR_MASK,		//
	GFL_WICON_POSX,GFL_WICON_POSY,	// 通信アイコンXY位置
	UNION_CONNECT_PLAYER_NUM,      //_MAXNUM,	//最大接続人数
	48,     //_MAXSIZE,	//最大送信バイト数
	UNION_RECEIVE_BEACON_MAX,  // 最大ビーコン収集数
	TRUE,		// CRC計算
	FALSE,		// MP通信＝親子型通信モードかどうか
	GFL_NET_TYPE_WIRELESS,		//通信タイプの指定
	TRUE,		// 親が再度初期化した場合、つながらないようにする場合TRUE
	WB_NET_UNION,	//GameServiceID
#if GFL_NET_IRC
	IRC_TIMEOUT_STANDARD,	// 赤外線タイムアウト時間
#endif
};


//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * ユニオンシステム初期化
 *
 * @retval  UNION_SYSTEM_PTR		生成したユニオンシステムワークへのポインタ
 */
//==================================================================
static UNION_SYSTEM_PTR Union_InitSystem(UNION_PARENT_WORK *uniparent)
{
  UNION_SYSTEM_PTR unisys;
  
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_UNION, HEAP_SIZE_UNION );
  unisys = GFL_HEAP_AllocClearMemory(HEAPID_UNION, sizeof(UNION_SYSTEM));
  unisys->uniparent = uniparent;
  UnionMyComm_Init(&unisys->my_situation.mycomm);
  
  GFL_OVERLAY_Load( FS_OVERLAY_ID( union_room ) );
  
  return unisys;
}

//==================================================================
/**
 * ユニオンシステム終了
 *
 * @param   unisys		ユニオンシステムワークへのポインタ
 */
//==================================================================
static void Union_ExitSystem(UNION_SYSTEM_PTR unisys)
{
  GFL_HEAP_FreeMemory(unisys);
  GFL_HEAP_DeleteHeap(HEAPID_UNION);
  
  GFL_OVERLAY_Unload( FS_OVERLAY_ID( union_room ) );
}



//==============================================================================
//  game_comm
//==============================================================================
//==================================================================
/**
 * 初期化処理
 *
 * @retval  UNION_SYSTEM_PTR		ユニオンシステムへのポインタ
 */
//==================================================================
void * UnionComm_Init(int *seq, void *pwk)
{
	UNION_SYSTEM_PTR unisys;
	
	unisys = Union_InitSystem(pwk);
	
	unisys->comm_status = UNION_COMM_STATUS_INIT_START;
	GFL_NET_Init(&aGFLNetInit, UnionComm_InitCallback, unisys);

	return unisys;
}

//==================================================================
/**
 * 初期化完了確認
 *
 * @param   pWork		
 *
 * @retval  BOOL		TRUE:初期化完了     FALSE:初期化完了待ち
 */
//==================================================================
BOOL UnionComm_InitWait(int *seq, void *pwk, void *pWork)
{
  UNION_SYSTEM_PTR unisys = pWork;

  if(unisys->comm_status >= UNION_COMM_STATUS_INIT){
    GF_ASSERT(unisys->comm_status < UNION_COMM_STATUS_UPDATE);
    unisys->comm_status = UNION_COMM_STATUS_UPDATE;
//    GFL_NET_StartBeaconScan();
    GFL_NET_Changeover(NULL);
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 初期化完了コールバック
 *
 * @param   pWork		UNION_SYSTEM_PTR
 */
//--------------------------------------------------------------
static void UnionComm_InitCallback(void *pWork)
{
  UNION_SYSTEM_PTR unisys = pWork;
  
  unisys->comm_status = UNION_COMM_STATUS_INIT;
}

//==================================================================
/**
 * 終了処理
 *
 * @param   unisys		
 */
//==================================================================
BOOL UnionComm_Exit(int *seq, void *pwk, void *pWork)
{
  UNION_SYSTEM_PTR unisys = pWork;
  
  GF_ASSERT(unisys->comm_status == UNION_COMM_STATUS_UPDATE);
  if(unisys->comm_status < UNION_COMM_STATUS_UPDATE){
    return FALSE; //通常ありえないけど念のためUpdateになるまで待つようにする
  }
  
  unisys->comm_status = UNION_COMM_STATUS_EXIT_START;
	GFL_NET_Exit(UnionComm_ExitCallback);
	return TRUE;
}

//==================================================================
/**
 * 終了待ち
 *
 * @param   unisys		
 *
 * @retval  BOOL		TRUE:終了完了。　FALSE:終了待ち中
 */
//==================================================================
BOOL UnionComm_ExitWait(int *seq, void *pwk, void *pWork)
{
  UNION_SYSTEM_PTR unisys = pWork;

  if(unisys->comm_status == UNION_COMM_STATUS_EXIT){
    Union_ExitSystem(unisys);
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 終了完了コールバック
 *
 * @param   pWork		UNION_SYSTEM_PTR
 */
//--------------------------------------------------------------
static void	UnionComm_ExitCallback(void* pWork)
{
  UNION_SYSTEM_PTR unisys = pWork;
  
  OS_TPrintf("ExitCallback\n");
  unisys->comm_status = UNION_COMM_STATUS_EXIT;
}

//==================================================================
/**
 * 更新処理
 *
 * @param   pWork		UNION_SYSTEM_PTR
 */
//==================================================================
void UnionComm_Update(int *seq, void *pwk, void *pWork)
{
  UNION_SYSTEM_PTR unisys = pWork;
  
  UnionComm_BeaconSearch(unisys);   //ビーコンサーチ
}

//--------------------------------------------------------------
/**
 * ビーコンサーチ
 *
 * @param   unisys		
 */
//--------------------------------------------------------------
static void UnionComm_BeaconSearch(UNION_SYSTEM_PTR unisys)
{
  int i;
  UNION_BEACON *bcon_buff;
  const u8 *mac_address_ptr;
  
  for(i = 0; i < UNION_RECEIVE_BEACON_MAX; i++){
  	bcon_buff = GFL_NET_GetBeaconData(i);
  	if(bcon_buff != NULL){
      mac_address_ptr = GFL_NET_GetBeaconMacAddress(i);
      if(mac_address_ptr != NULL){
        UnionBeacon_SetReceiveData(unisys, bcon_buff, mac_address_ptr);
      }
  	}
  }
}

//--------------------------------------------------------------
/**
 * 受信ビーコンを受信用バッファにセットする
 *
 * @param   unisys		
 * @param   beacon		
 *
 * @retval  BOOL		TRUE:成功。　FALSE:失敗
 */
//--------------------------------------------------------------
static BOOL UnionBeacon_SetReceiveData(UNION_SYSTEM_PTR unisys, const UNION_BEACON *beacon, const u8 *beacon_mac_address)
{
  int i;
  UNION_BEACON_PC *dest;
  
  dest = unisys->receive_beacon;
  
  if(beacon->data_valid != UNION_BEACON_VALID){
    return FALSE; //有効なビーコンではない
  }
  
  //既に受信済みのビーコンデータか確認
  for(i = 0; i < UNION_RECEIVE_BEACON_MAX; i++){
    if(dest[i].beacon.data_valid == UNION_BEACON_VALID
        && GFL_STD_MemComp(beacon_mac_address, dest[i].mac_address, 6) == 0){
      GFL_STD_MemCopy(beacon, &dest[i].beacon, sizeof(UNION_BEACON));
      //一度に同じマシンからの連続受信の可能性を考えnewの場合はフラグを変えない
      if(dest[i].update_flag != UNION_BEACON_RECEIVE_NEW){
        dest[i].update_flag = UNION_BEACON_RECEIVE_UPDATE;
      }
      dest[i].life = UNION_CHAR_LIFE;   //ビーコンの更新があったので寿命を元に戻す
//      OS_TPrintf("life回復 %d\n", OS_GetVBlankCount());
      return TRUE;
    }
  }

  //新規のビーコンデータなので空き領域にセットする
  for(i = 0; i < UNION_RECEIVE_BEACON_MAX; i++){
    if(dest[i].beacon.data_valid != UNION_BEACON_VALID){
      GFL_STD_MemCopy(beacon, &dest[i].beacon, sizeof(UNION_BEACON));
      GFL_STD_MemCopy(beacon_mac_address, dest[i].mac_address, 6);
      dest[i].update_flag = UNION_BEACON_RECEIVE_NEW;
      dest[i].life = UNION_CHAR_LIFE;
      return TRUE;
    }
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * ビーコンデータ取得関数
 *
 * @param   pWork		UNION_SYSTEM_PTR
 *
 * @retval  void*		ビーコンデータへのポインタ
 */
//--------------------------------------------------------------
static void* UnionComm_GetBeaconData(void* pWork)
{
  UNION_SYSTEM_PTR unisys = pWork;
  
  UnionComm_SetBeaconParam(unisys, &unisys->my_beacon);
  return &unisys->my_beacon;
}

//==================================================================
/**
 * 送信するビーコンデータを作成する
 *
 * @param   beacon		ビーコンパラメータ代入先
 */
//==================================================================
static void UnionComm_SetBeaconParam(UNION_SYSTEM_PTR unisys, UNION_BEACON *beacon)
{
  UNION_MY_SITUATION *situ = &unisys->my_situation;
  
  GFL_STD_MemClear(beacon, sizeof(UNION_BEACON));
  
  if(situ->calling_pc != NULL){
    GFL_STD_MemCopy(situ->calling_pc->mac_address, beacon->connect_mac_address, 6);
  }
  
  beacon->pm_version = PM_VERSION;
  beacon->language = PM_LANG;
  beacon->union_status = situ->union_status;
  beacon->appeal_no = situ->appeal_no;
  
  MyStatus_CopyNameStrCode(unisys->uniparent->mystatus, beacon->name, PERSON_NAME_SIZE + EOM_SIZE);
  
  beacon->trainer_view = MyStatus_GetTrainerView( unisys->uniparent->mystatus );
  beacon->sex = MyStatus_GetMySex(unisys->uniparent->mystatus);
  
  
  //送信データ完成
  beacon->data_valid = UNION_BEACON_VALID;
}

//--------------------------------------------------------------
/**
 * ビーコンデータサイズ取得関数
 *
 * @param   pWork		UNION_SYSTEM_PTR
 *
 * @retval  int		ビーコンデータサイズ
 */
//--------------------------------------------------------------
static int UnionComm_GetBeaconSize(void *pWork)
{
  return sizeof(UNION_BEACON);
}

//--------------------------------------------------------------
/**
 * ビーコンのサービスを比較して繋いで良いかどうか判断する
 *
 * @param   GameServiceID1		サービスID１
 * @param   GameServiceID2		サービスID２
 *
 * @retval  BOOL		TRUE:接続OK　　FALSE:接続NG
 */
//--------------------------------------------------------------
static BOOL UnionComm_CheckConnectService(GameServiceID GameServiceID1 , GameServiceID GameServiceID2 )
{
  return (GameServiceID1==GameServiceID2);
}

//--------------------------------------------------------------
/**
 * 通信不能なエラーが起こった場合呼ばれる 切断するしかない
 *
 * @param   pNet		ネットハンドル
 * @param   errNo		エラー番号
 * @param   pWork		UNION_SYSTEM_PTR
 */
//--------------------------------------------------------------
static void UnionComm_ErrorCallBack(GFL_NETHANDLE* pNet,int errNo, void* pWork)
{
	UNION_SYSTEM_PTR unisys = pWork;

  NetErr_ErrorSet();
}

//--------------------------------------------------------------
/**
 * 通信切断時に呼ばれる関数(終了時
 *
 * @param   pWork		UNION_SYSTEM_PTR
 */
//--------------------------------------------------------------
static void UnionComm_DisconnectCallBack(void* pWork)
{
  OS_TPrintf("通信切断コールバック UnionComm_DisconnectCallBack\n");
}


//==============================================================================
//  通信リクエスト
//==============================================================================
//==================================================================
/**
 * 自分シチュエーションのセット
 *
 * @param   unisys		
 * @param   index		  シチュエーションパラメータのデータIndex
 * @param   work		  各Indexで必要とするワークへのポインタ
 */
//==================================================================
void UnionMySituation_SetParam(UNION_SYSTEM_PTR unisys, UNION_MYSITU_PARAM_IDX index, void *work)
{
  UNION_MY_SITUATION *situ = &unisys->my_situation;

  switch(index){
  case UNION_MYSITU_PARAM_IDX_CALLING_PC:
    {
      UNION_BEACON_PC *calling_pc = work;
      situ->calling_pc = calling_pc;
    }
    break;
  case UNION_MYSITU_PARAM_IDX_ANSWER_PC:
    {
      UNION_BEACON_PC *answer_pc = work;
      situ->answer_pc = answer_pc;
    }
    break;
  case UNION_MYSITU_PARAM_IDX_CONNECT_PC:
    {
      UNION_BEACON_PC *connect_pc = work;
      situ->connect_pc = connect_pc;
    }
    break;
  }
}

//==================================================================
/**
 * 自分シチュエーションのクリア
 *
 * @param   unisys		
 */
//==================================================================
void UnionMySituation_Clear(UNION_SYSTEM_PTR unisys)
{
  UNION_MY_SITUATION *situ = &unisys->my_situation;
  
  GFL_STD_MemClear(situ, sizeof(UNION_MY_SITUATION));
  UnionMyComm_Init(&situ->mycomm);
}

//==================================================================
/**
 * UNION_MY_COMM構造体を初期化
 *
 * @param   mycomm		
 */
//==================================================================
void UnionMyComm_Init(UNION_MY_COMM *mycomm)
{
  GFL_STD_MemClear(mycomm, sizeof(UNION_MY_COMM));
  mycomm->mainmenu_select = UNION_MSG_MENU_SELECT_NULL;
  mycomm->submenu_select = UNION_MSG_MENU_SELECT_NULL;
  mycomm->mainmenu_yesno_result = 0xff;
}
