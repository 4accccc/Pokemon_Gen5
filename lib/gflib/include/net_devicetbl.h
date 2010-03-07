#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
/**
 * @file	net_devicetbl.h
 * @brief	通信デバイスアクセス関数登録テーブル
 * @author	k.ohno
 * @date    2008.12.19
 */
//=============================================================================
#pragma once

/// @brief 送信完了コールバック
typedef BOOL (*PTRSendDataCallback)(BOOL result);
/// @brief コールバック関数の書式
typedef void (*PTRCommRecvLocalFunc)(u16 aid, u16 *data, u16 size);
/// @brief コールバック関数の書式
typedef void (*PTRChildConnectCallbackFunc)(u16 aid);
/// ビーコンを拾ったときに呼ばれるコールバック関数
typedef void (*PTRPARENTFIND_CALLBACK)(void);


/// 処理終了時に呼ばれるコールバック
typedef BOOL (*NetDevEndCallback)(BOOL bResult);
// 電源起動直後に呼ばれる
typedef void (*DevBootFunc)(HEAPID heapID, NetErrorFunc errorFunc);
/// オーバレイセット 接続クラスの初期化 メモリ確保関数
typedef BOOL (*DevInitFunc)(HEAPID heapID, GFL_NETSYS* pNet,NetDevEndCallback callback, void* pUserWork);
/// 接続  WIFI接続までやる
typedef BOOL (*DevStartFunc)(NetDevEndCallback callback);

typedef int (*DevMainFunc)(u16 bitmap); ///<  動作
typedef BOOL (*DevEndFunc)(BOOL bForce, NetDevEndCallback callback);  ///<   切断
typedef BOOL (*DevExitFunc)(NetDevEndCallback callback); ///<  メモリ開放オーバーレイ開放

typedef BOOL (*DevChildWorkInitFunc)(int NetID); ///<     子機エリアクリア
typedef BOOL (*DevParentWorkInitFunc)(void); ///<DevParentWork              親機エリアクリア

typedef BOOL (*DevMyBeaconSetFunc)(NetDevEndCallback callback); ///<DevMyBeaconSet             自分ビーコンを設定
typedef BOOL (*DevBeaconSearchFunc)(NetDevEndCallback callback); ///<DevBeaconSearch            自分のセット サーチ開始
typedef void* (*DevBeaconGetFunc)(int index); ///<DevBeaconGet               ビーコンを得る
typedef u8* (*DevBeaconGetMacFunc)(int index); ///<         ビーコンのMACを得る
typedef BOOL (*DevIsBeaconChangeFunc)(NetDevEndCallback callback); ///<DevIsBeaconChange          ビーコンが更新されたかどうかを得る
typedef BOOL (*DevResetBeaconChangeFlgFunc)(NetDevEndCallback callback); ///<DevResetBeaconChangeFlg    フラグをリセットする
typedef BOOL (*DevIsChangeOverTurnFunc)(NetDevEndCallback callback); ///<DevIsChangeOverTurn        親子切り替えしてよいかどうか(親機が一回でもビーコンを送信し終わったかどうか)

typedef BOOL (*DevSetChildConnectCallbackFunc)(PTRChildConnectCallbackFunc func);  ///< 子供がつないだ時のコールバックを指定する
typedef BOOL (*DevParentModeConnectFunc)(BOOL bChannelChange,NetDevEndCallback callback); ///<DevParentModeConnect       親開始
typedef BOOL (*DevChildModeConnectFunc)(BOOL bBconInit,NetDevEndCallback callback); ///<DevChildModeConnect        子供開始        ビーコンindex + macAddress  つながない場合-1
typedef BOOL (*DevChildModeMacConnectFunc)(BOOL bBconInit,void* pMacAddr,int index,NetDevEndCallback callback,PTRPARENTFIND_CALLBACK pCallback); ///<DevChildModeConnect        子供開始        ビーコンindex + macAddress  つながない場合-1
typedef BOOL (*DevChangeOverModeConnectFunc)(u8* keyStr,int numEntry, BOOL bParent, u32 timelimit,NetDevEndCallback callback); ///<DevChangeOverModeConnect   ランダム接続     つながない場合-1
typedef BOOL (*DevWifiConnectFunc)(int index, int maxnum, BOOL bVCT);
typedef BOOL (*DevModeDisconnectFunc)(BOOL bForce, NetDevEndCallback callback); ///<DevModeDisconnect          接続を終わる
typedef BOOL (*DevIsStepDSFunc)(void);  ///< データシェアリング方式送信出来る状態か
typedef BOOL (*DevStepDSFunc)(void* pSendData);  ///< データシェアリング方式送信
typedef void* (*DevGetSharedDataAdrFunc)(int index);  ///<データシェアリング方式データを得る

typedef BOOL (*DevSendDataFunc)(void* data,int size,int no,NetDevEndCallback callback); ///<DevSendData                送信関数
typedef BOOL (*DevRecvCallbackFunc)(PTRCommRecvLocalFunc recvCallback); ///<DevRecvCallback            受信コールバック

typedef BOOL (*DevIsStartFunc)(void); ///<DevIsStart                 通信接続してるかどうか
typedef BOOL (*DevIsConnectFunc)(void); ///<DevIsConnect               相手と接続してるかどうか
typedef BOOL (*DevIsEndFunc)(void); ///<DevIsEnd                   通信終了してよいかどうか
typedef BOOL (*DevIsIdleFunc)(void); ///<アイドル状態かどうか
typedef u32 (*DevGetBitmapFunc)(void); ///<DevGetBitmap               接続状態をBITMAPで返す
typedef u32 (*DevGetCurrentIDFunc)(void); ///<DevGetCurrentID            自分の接続IDを返す
typedef int (*DevGetIconLevelFunc)(void); ///<アイコンレベルを返す
typedef int (*DevGetErrorFunc)(void);  ///< エラーを得る
typedef void (*DevSetNoChildErrorFunc)(BOOL bOn);  ///< 子機がいない場合にエラーにするかどうかを設定
typedef BOOL (*DevIsConnectableFunc)(int index); ///< 相手がつないでよい状態かどうか (ほぼfriendWifi用)
typedef BOOL (*DevIsVChatFunc)(void);  ///< VChatかどうか
typedef BOOL (*DevIsNewPlayerFunc)(void);  ///< 接続してきたかどうかを返します
typedef void (*DevIrcMoveFunc)(void);   ///< IRC移動関数
typedef BOOL (*DevIsSendDataFunc)(void);  ///< データを送ってよいかどうか
typedef BOOL (*DevGetSendTurnFunc)(void);  ///< 送信可能ターンフラグを取得
typedef BOOL (*DevIsConnectSystemFunc)(void);  ///< 再接続中など関係なく、純粋に今、繋がっているか
typedef BOOL (*DevGetSendLockFlagFunc)(void); ///< 送信ロックフラグを取得
typedef void (*DevConnectWorkInitFunc)(void); ///< 初めての接続後のワーク設定
typedef void (*DevSetClientConnectFunc)(BOOL bEnable); ///< 子機がつながってよいかどうかハードレベルで調整
typedef BOOL (*DevCrossScanFunc)(void); ///< すれ違い通信開始
typedef void (*DevCrossScanChangeSpeedFunc)(int num); ///< すれ違い速度変更

typedef BOOL (*DevLobbyLoginFunc)(const void* cp_loginprofile); ///<Wi-Fi広場にログイン	DWC_LOBBY_Login
typedef void (*DevDebugSetRoomFunc)( u32 locktime, u32 random, u8 roomtype, u8 season ); ///<デバッグ用 部屋データ設定DWC_LOBBY_DEBUG_SetRoomData
typedef BOOL (*DevLobbyUpdateErrorCheckFunc)(void);	///<DWC_LOBBY_UpdateErrorCheck
typedef BOOL (*DevLobbyLoginWaitFunc)(void);	///<DWC_LOBBY_LoginWait
typedef void (*DevLobbyLogoutFunc)(void);	///<DWC_LOBBY_Logout
typedef BOOL (*DevLobbyLogoutWaitFunc)(void);	///<DWC_LOBBY_LogoutWait
typedef BOOL (*DevLobbyMgCheckRecruitFunc)( int type ); ///<DWC_LOBBY_MG_CheckRecruit
typedef BOOL (*DevLobbyMgStartRecruitFunc)( int type, u32 maxnum ); ///<DWC_LOBBY_MG_StartRecruit
typedef BOOL (*DevLobbyMgEntryFunc)( int type );  ///<DWC_LOBBY_MG_Entry
typedef BOOL (*DevLobbyMgForceEndFunc)( void ); ///<DWC_LOBBY_MG_ForceEnd
typedef void (*DevLobbyMgEndConnectFunc)( void ); ///<DWC_LOBBY_MG_EndConnect
typedef BOOL (*DevLobbyMgMyParentFunc)( void );  ///<DWC_LOBBY_MG_MyParent
typedef void (*DevLobbyMgEndRecruitFunc)( void );  ///<DWC_LOBBY_MG_EndRecruit


/// @brief デバイスアクセス用構造体
typedef struct{
  DevBootFunc DevBoot;
  DevInitFunc DevInit;                    ///< オーバレイセット 接続クラスの初期化 メモリ確保
  DevStartFunc DevStart;                   ///< 接続  WIFI接続までやる
  DevMainFunc DevMain;                    ///<動作
  DevEndFunc DevEnd;                     ///<切断
  DevExitFunc DevExit;                    ///<メモリ開放オーバーレイ開放

  DevChildWorkInitFunc DevChildWorkInit;           ///<子機エリアクリア
  DevParentWorkInitFunc DevParentWorkInit;              ///<親機エリアクリア

  DevMyBeaconSetFunc DevMyBeaconSet;             ///<自分ビーコンを設定
  DevBeaconSearchFunc DevBeaconSearch;            ///<自分のセット サーチ開始
  DevBeaconGetFunc DevBeaconGet;               ///<ビーコンを得る
  DevBeaconGetMacFunc DevBeaconGetMac;     ///<ビーコンのMACを得る
  DevIsBeaconChangeFunc DevIsBeaconChange;          ///<ビーコンが更新されたかどうかを得る
  DevResetBeaconChangeFlgFunc DevResetBeaconChangeFlg;    ///<フラグをリセットする
  DevIsChangeOverTurnFunc DevIsChangeOverTurn;        ///<親子切り替えしてよいかどうか(親機が一回でもビーコンを送信し終わったかどうか)

  DevSetChildConnectCallbackFunc DevSetChildConnectCallback; ///< 子供がつないだ時のコールバックを指定する
  DevParentModeConnectFunc DevParentModeConnect;       ///<親開始
  DevChildModeConnectFunc DevChildModeConnect;        ///<子供開始
  DevChildModeMacConnectFunc   DevChildModeMacConnect; ///<     子供開始      macAddress
  DevChangeOverModeConnectFunc DevChangeOverModeConnect;   ///<ランダム接続     つながない場合-1
  DevWifiConnectFunc DevWifiConnect;  ///<WIFI接続
  DevModeDisconnectFunc DevModeDisconnect;          ///<接続を終わる
  DevIsStepDSFunc DevIsStepDS;  ///< データシェアリング方式送信出来る状態か
  DevStepDSFunc DevStepDS;  ///< データシェアリング方式送信
  DevGetSharedDataAdrFunc DevGetSharedDataAdr;

  DevSendDataFunc DevSendData;                ///<送信関数
  DevRecvCallbackFunc DevRecvCallback;            ///<受信コールバック

  DevIsStartFunc DevIsStart;                 ///<通信接続してるかどうか
  DevIsConnectFunc DevIsConnect;               ///<相手と接続してるかどうか
  DevIsEndFunc DevIsEnd;                 ///<通信終了してよいかどうか
  DevIsIdleFunc DevIsIdle;   ///< アイドル状態かどうか
  DevGetBitmapFunc DevGetBitmap;               ///<接続状態をBITMAPで返す
  DevGetCurrentIDFunc DevGetCurrentID;            ///<自分の接続IDを返す
  DevGetIconLevelFunc DevGetIconLevel;  ///< アイコンレベルを返す
  DevGetErrorFunc DevGetError;  ///< エラーを得る
  DevSetNoChildErrorFunc DevSetNoChildError;  ///< 子機がいない場合にエラーにするかどうかを設定
  DevIsConnectableFunc DevIsConnectable;   ///< 相手がつないでよい状態かどうか (ほぼfriendWifi用)
  DevIsVChatFunc DevIsVChat;   ///< VChatかどうか
  DevIsNewPlayerFunc DevIsNewPlayer; ///< 接続してきたかどうかを返します
  DevIrcMoveFunc DevIrcMove;  ///< IRC移動関数
  DevIsSendDataFunc DevIsSendData; ///< データを送ってよいかどうか
  DevGetSendTurnFunc DevGetSendTurn; ///< 送信可能ターンフラグを取得
  DevIsConnectSystemFunc DevIsConnectSystem;  ///< 再接続中など関係なく、純粋に今、繋がっているか
  DevGetSendLockFlagFunc DevGetSendLockFlag;  ///< 送信ロックフラグを取得
  DevConnectWorkInitFunc DevConnectWorkInit;           ///<初めての接続後のワーク設定
  DevSetClientConnectFunc DevSetClientConnect; ///< 子機がつながってよいかどうかハードレベルで調整
  DevCrossScanFunc DevCrossScan; ///<  すれ違い通信開始
  DevCrossScanChangeSpeedFunc DevCrossScanChangeSpeed; ///<  すれ違い通信速度変更

#if 0
  //-- 以下、Wi-Fi広場専用
  DevLobbyLoginFunc DevLobbyLogin;		///<Wi-Fi広場にログイン	DWC_LOBBY_Login
  DevDebugSetRoomFunc DevDebugSetRoom;	///<デバッグ用 部屋データ設定 DWC_LOBBY_DEBUG_SetRoomData
  DevLobbyUpdateErrorCheckFunc DevLobbyUpdateErrorCheck;	///<DWCロビー更新 DWC_LOBBY_UpdateErrorCheck
  DevLobbyLoginWaitFunc DevLobbyLoginWait;	///<DWC_LOBBY_LoginWait
  DevLobbyLogoutFunc DevLobbyLogout;	///<DWC_LOBBY_Logout
  DevLobbyLogoutWaitFunc DevLobbyLogoutWait;	///<DWC_LOBBY_LogoutWait
  DevLobbyMgCheckRecruitFunc DevLobbyMgCheckRecruit;  ///<DWC_LOBBY_MG_CheckRecruit
  DevLobbyMgStartRecruitFunc DevLobbyMgStartRecruit;  ///<DWC_LOBBY_MG_StartRecruit
  DevLobbyMgEntryFunc DevLobbyMgEntry;  ///<DWC_LOBBY_MG_Entry
  DevLobbyMgForceEndFunc DevLobbyMgForceEnd;  ///<DWC_LOBBY_MG_ForceEnd
  DevLobbyMgEndConnectFunc DevLobbyMgEndConnect; ///<DWC_LOBBY_MG_EndConnect
  DevLobbyMgMyParentFunc DevLobbyMgMyParent;  ///<DWC_LOBBY_MG_MyParent
  DevLobbyMgEndRecruitFunc DevLobbyMgEndRecruit;  ///<DWC_LOBBY_MG_EndRecruit
#endif
  
} GFLNetDevTable;


/* 以下の関数はGFLIB外に作成する。オーバーレイとドライバーのロードを担当する
   net_dev.c のサンプルをそのまま持ってくればよい*/


///   ワイヤレスデバイスをロードする
extern GFLNetDevTable* NET_DeviceLoad(int deviceNo);
///   ワイヤレスデバイスをアンロードする
extern void NET_DeviceUnload(int deviceNo);



#ifdef __cplusplus
} /* extern "C" */
#endif
