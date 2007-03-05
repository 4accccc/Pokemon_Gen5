//============================================================================================
/**
 * @file	yt_net.c
 * @brief	DS版ヨッシーのたまご ネットワーク関連のプログラム
 * @author	ohno
 * @date	2007.03.05
 */
//============================================================================================


#include "gflib.h"
#include "procsys.h"
#include "ui.h"

#include "main.h"
#include "yt_common.h"

///< @brief 通信テーブル



#define _MAXNUM (2)     // 接続最大
#define _MAXSIZE (120)  // 送信最大サイズ

#if GFL_NET_WIFI //GFL_NET_WIFI WIFI通信テスト

// この二つのデータが「ともだちこーど」になります。本来はセーブする必要があります
static DWCUserData _testUserData;
static DWCFriendData _testFriendData[_BCON_GET_NUM];

// 通信初期化構造体  wifi用
GFLNetInitializeStruct aGFLNetInit = {
    _CommPacketTbl,  // 受信関数テーブル
    NELEMS(_CommPacketTbl), // 受信テーブル要素数
    NULL,   // ワークポインタ
    _netBeaconGetFunc,  // ビーコンデータ取得関数
    _netBeaconGetSizeFunc,  // ビーコンデータサイズ取得関数
    _netBeaconCompFunc,  // ビーコンのサービスを比較して繋いで良いかどうか判断する
    FatalError_Disp,  // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
    NULL,  // 通信切断時に呼ばれる関数
    NULL,  // wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
    NULL,  // wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
    &_testFriendData[0],  // DWC形式の友達リスト	
    &_testUserData,  // DWCのユーザデータ（自分のデータ）
    _netGetSSID,  // 親子接続時に認証する為のバイト列  
    1,  //gsid
    0,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
    GFL_HEAPID_APP,  //元になるheapid
    HEAPID_NETWORK,  //通信用にcreateされるHEAPID
    HEAPID_WIFI,  //wifi用にcreateされるHEAPID
    _MAXNUM,     // 最大接続人数
    _MAXSIZE,  //最大送信バイト数
    _BCON_GET_NUM,    // 最大ビーコン収集数
    FALSE,     // MP通信＝親子型通信モードかどうか
    TRUE,  //wifi通信を行うかどうか
    TRUE,     // 通信を開始するかどうか
};

#else  //GFL_NET_WIFI 普通のワイヤレス通信

// 通信初期化構造体  wifi用
GFLNetInitializeStruct aGFLNetInit = {
    _CommPacketTbl,  // 受信関数テーブル
    NELEMS(_CommPacketTbl), // 受信テーブル要素数
    NULL,   // ワークポインタ
    _netBeaconGetFunc,  // ビーコンデータ取得関数
    _netBeaconGetSizeFunc,  // ビーコンデータサイズ取得関数
    _netBeaconCompFunc,  // ビーコンのサービスを比較して繋いで良いかどうか判断する
    FatalError_Disp,  // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
    NULL,  // 通信切断時に呼ばれる関数
    _netGetSSID,  // 親子接続時に認証する為のバイト列  
    1,  //gsid
    0,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
    GFL_HEAPID_APP,  //元になるheapid
    HEAPID_NETWORK,  //通信用にcreateされるHEAPID
    HEAPID_WIFI,  //wifi用にcreateされるHEAPID
    _MAXNUM,     // 最大接続人数
    _MAXSIZE,  //最大送信バイト数
    _BCON_GET_NUM,    // 最大ビーコン収集数
    FALSE,     // MP通信＝親子型通信モードかどうか
    FALSE,  //wifi通信を行うかどうか
    TRUE,     // 通信を開始するかどうか
};

#endif //GFL_NET_WIFI

void TEST_NET_Init(void)
{

    GFL_NET_sysInit(&aGFLNetInit);

}






//----------------------------------------------------------------------------
/**
 *	@brief	
 *	@param	gp			ゲームパラメータポインタ
 *	@param	player_no	1P or 2P
 *	@param	type		キャラタイプ
 *	@param	line_no		落下ラインナンバー
 *
 *	@retval FALL_CHR_PARAM
 */
//-----------------------------------------------------------------------------
void YT_NetInit(void)
{
    GFL_NET_sysInit(&aGFLNetInit);
}

