//=============================================================================
/**
 * @file	net_command.h
 * @brief	データ共有を行う場合の通信システム
 *          使用する場合に書き込む必要があるものをまとめたヘッダー
 * @author	Katsumi Ohno
 * @date    2005.07.26
 */
//=============================================================================

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __NET_COMMAND_H__
#define __NET_COMMAND_H__


//==============================================================================
//	定義
//==============================================================================

//==============================================================================
//	型宣言
//==============================================================================

//#define   _SET(callfunc, getSize, name)       {callfunc,  getSize},

///-------------------------------------------------------------------------
///-------------------------------------------------------------------------

/// 汎用通信コマンドの定義
enum CommCommand_e {
  GFL_NET_CMD_NONE = 0,                ///< なにもしない
  GFL_NET_CMD_FREE = GFL_NET_CMD_NONE,                   ///< 空コマンド
  GFL_NET_CMD_COMMAND_MIN = 1,             ///< 最小値
  GFL_NET_CMD_EXIT_REQ = GFL_NET_CMD_COMMAND_MIN,            ///< 終了
  GFL_NET_CMD_EXIT,            ///< 自動終了
  GFL_NET_CMD_NEGOTIATION,  ///< 初期化時のネゴシエーション
  GFL_NET_CMD_NEGOTIATION_RETURN,  ///< ネゴシエーションの返事
  GFL_NET_CMD_DSMP_CHANGE,            ///< DSモード通信かMPモード通信かに切り替える許可を得る
  GFL_NET_CMD_TIMING_SYNC,            ///< 同期を取るコマンド  8
  GFL_NET_CMD_TIMING_SYNC_END,        ///< 同期が取れたことを返すコマンド
#if GFL_NET_WIFI
  GFL_NET_CMD_WIFI_EXIT,              ///< 子機が親機に終了するように送信
#endif
  //------------------------------------------------ここまで----------
  GFL_NET_CMD_COMMAND_MAX   // 終端--------------これは移動させないでください
};


//

//--------------------------------------------------------------
/**
 * @brief   コマンドテーブルの初期化  コマンドテーブルの変更
 * @param   cokind               コマンド体系番号 NetStartCommand_e
 * @param   pCommPacketLocal 呼び出しモジュール専用のコマンド体系テーブル
 * @param   listNum          コマンド数
 * @param   pWork            呼び出しモジュール専用のワークエリア
 * @retval  none
 */
//--------------------------------------------------------------
extern void GFL_NET_COMMAND_Init(const int cokind, const NetRecvFuncTable* pCommPacketLocal,int listNum,void* pWork);


//--------------------------------------------------------------
/**
 * @brief   コマンドテーブルの追加
 * @param   cokind          コマンド体系番号 NetStartCommand_e
 * @param   pCommPacketLocal 呼び出しモジュール専用のコマンド体系テーブル
 * @param   listNum          コマンド数
 * @param   pWork            呼び出しモジュール専用のワークエリア
 * @retval  none
 */
//--------------------------------------------------------------
extern void GFL_NET_AddCommandTable(const int cokind,const NetRecvFuncTable* pCommPacketLocal,int listNum,void* pWork);

//--------------------------------------------------------------
/**
 * @brief   コマンドテーブルの削除
 * @param   cokind          コマンド体系番号 NetStartCommand_e
 * @retval  none
 */
//--------------------------------------------------------------
extern void GFL_NET_DelCommandTable(const int cokind);

//--------------------------------------------------------------
/**
 * @brief   コマンドテーブルの開放処理
 *          ゲーム中に開放する場合は、必ず同期をあわせてから開放してください
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------
extern void GFL_NET_COMMAND_Exit( void );


// ここから下はライブラリ専用 GFI関数

//--------------------------------------------------------------
/**
 * @brief   テーブルに従い 受信コールバックを呼び出します
 * @param   netID           ネットワークID
 * @param   sendID          送信ID
 * @param   command         受信コマンド
 * @param   size            受信データサイズ
 * @param   pData           受信データ
 * @retval  none
 */
//--------------------------------------------------------------
extern void GFI_NET_COMMAND_CallBack(int netID, int sendID, int command, int size, void* pData,GFL_NETHANDLE* pNetHandle);

//--------------------------------------------------------------
/**
 * @brief   受信バッファを持っているかどうかの検査
 * @param   command         コマンド
 * @retval  持ってるならTRUE
 */
//--------------------------------------------------------------
extern BOOL GFI_NET_COMMAND_CreateBuffCheck(int command);


//--------------------------------------------------------------
/**
 * @brief   受信バッファを得る
 * @param   command         コマンド
 * @param   netID           送信者
 * @param   size         サイズ
 * @retval  受信バッファ 持っていないならばNULL
 */
//--------------------------------------------------------------

extern void* GFI_NET_COMMAND_CreateBuffStart(int command, int netID, int size);

//--------------------------------------------------------------
/**
 * @brief   コマンドを受け取る人かどうか
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

extern BOOL GFI_NET_COMMAND_RecvCheck(int recvID);


#endif// __NET_COMMAND_H__

#ifdef __cplusplus
} /* extern "C" */
#endif
